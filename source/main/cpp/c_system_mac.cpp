#include "cbase/c_base.h"
#include "cbase/c_allocator.h"

#if defined(TARGET_MAC)

#    include "cbase/c_debug.h"
#    include "cbase/c_memory.h"
#    include "cbase/c_console.h"

#    include "csystem/c_system.h"

#    include <sys/sysctl.h>
#    include <sys/types.h>
#    include <sys/sysctl.h>
#    include <mach/mach.h>
#    include <mach/mach_time.h>
#    include <mach/vm_prot.h>
#    include <mach/machine.h>
#    include <mach-o/dyld.h>
#    include <mach-o/nlist.h>
#    include <mach-o/loader.h>
#    include <mach-o/swap.h>
#    include <mach-o/arch.h>
#    include <mach-o/fat.h>
#    include <mach-o/dyld_images.h>

#    include <math.h>
#    include <pthread.h>

namespace ncore
{
    static bool is64BitOS() { return sizeof(void*) == 8; }

    static bool isBigEndian()
    {
        u16 test = 0x0102;
        return ((u8*)&test)[0] == 0x01;
    }

    static const char* getOSVersion()
    {
        static char result[1024];
        size_t      size = sizeof(result);
        if (sysctlbyname("kern.osrelease", result, &size, nullptr, 0) == 0)
            return result;

        return "<apple>";
    }

    static bool getCPUInfo(int& logical, int& physical)
    {
        logical             = 0;
        physical            = 0;
        size_t logical_size = sizeof(logical);
        if (sysctlbyname("hw.logicalcpu", &logical, &logical_size, nullptr, 0) != 0)
            logical = -1;

        int    physical      = 0;
        size_t physical_size = sizeof(physical);
        if (sysctlbyname("hw.physicalcpu", &physical, &physical_size, nullptr, 0) != 0)
            physical = -1;

        return logical > physical; // hyper threading ?
    }

    static s64 getCpuClockSpeed()
    {
        s64    frequency = 0;
        size_t size      = sizeof(frequency);
        if (sysctlbyname("hw.cpufrequency", &frequency, &size, nullptr, 0) == 0)
            return frequency;

        return -1;
    }

    static mach_header* getmainmodule()
    {
        // get the main module
        const mach_header* header = _dyld_get_image_header(0);
        if (header->magic != MH_MAGIC)
            return nullptr;

        // get the load commands
        const load_command* loadCommands = (const load_command*)(header + 1);
        for (u32 i = 0; i < header->ncmds; ++i)
        {
            // get the current load command
            const load_command* loadCommand = loadCommands;

            // get the next load command
            loadCommands = (const load_command*)((const u8*)loadCommands + loadCommand->cmdsize);

            // check if this is a segment command
            if (loadCommand->cmd == LC_SEGMENT)
            {
                // get the segment command
                const segment_command* segmentCommand = (const segment_command*)loadCommand;

                // check if this is the __TEXT segment
                if (strcmp(segmentCommand->segname, "__TEXT") == 0)
                {
                    // get the mach header
                    return (mach_header*)((const u8*)header + segmentCommand->fileoff);
                }
            }
        }

        return nullptr;
    }

    static void InitializeExecutableInfo(s32& codeSegmentSize, s32& bssSegmentSize, s32& dataSegmentSize, s32& imageSize, s32& mainThreadStackSize)
    {
        // get the code, bss and data segment sizes as well as the size of the full image.
        // also get the size of the main thread stack

        // get the mach header
        const mach_header* header = (const mach_header*)getmainmodule();
        if (header->magic != MH_MAGIC)
            return;

        // get the load commands
        const load_command* loadCommands = (const load_command*)(header + 1);
        for (u32 i = 0; i < header->ncmds; ++i)
        {
            // get the current load command
            const load_command* loadCommand = loadCommands;

            // get the next load command
            loadCommands = (const load_command*)((const u8*)loadCommands + loadCommand->cmdsize);

            // check if this is a segment load command
            if (loadCommand->cmd == LC_SEGMENT)
            {
                // get the segment load command
                const segment_command* segmentCommand = (const segment_command*)loadCommand;

                // check if this is the code segment
                if (strcmp(segmentCommand->segname, "__TEXT") == 0)
                {
                    codeSegmentSize = segmentCommand->vmsize;
                }

                // check if this is the bss segment
                if (strcmp(segmentCommand->segname, "__BSS") == 0)
                {
                    bssSegmentSize = segmentCommand->vmsize;
                }

                // check if this is the data segment
                if (strcmp(segmentCommand->segname, "__DATA") == 0)
                {
                    dataSegmentSize = segmentCommand->vmsize;
                }
            }
        }

        // get the size of the full image
        imageSize = header->sizeofcmds + sizeof(mach_header);

        // get the size of the main thread stack
        mainThreadStackSize = 0;

    }

    static s64 getRamTotal()
    {
        s64    memsize = 0;
        size_t size    = sizeof(memsize);
        if (sysctlbyname("hw.memsize", &memsize, &size, nullptr, 0) == 0)
            return memsize;

        return -1;
    }

    static s64 getRamFree()
    {
        mach_port_t host_port = mach_host_self();
        if (host_port == MACH_PORT_NULL)
            return -1;

        vm_size_t page_size = 0;
        host_page_size(host_port, &page_size);

        vm_statistics_data_t   vmstat;
        mach_msg_type_number_t count      = HOST_VM_INFO_COUNT;
        kern_return_t          kernReturn = host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vmstat, &count);
        if (kernReturn != KERN_SUCCESS)
            return -1;

        [[maybe_unused]] s64 used_mem = (vmstat.active_count + vmstat.inactive_count + vmstat.wire_count) * page_size;
        s64                  free_mem = vmstat.free_count * page_size;
        return free_mem;
    }

    const char* system_t::getEndLine() { return getUnixEndLine(); }

    struct system_t::instance_t
    {
        instance_t(alloc_t* a)
            : m_allocator(a)
            , m_uMemHeapTotalSize(16ULL * 1024 * 1024 * 1024)
            , m_uLanguage(LANGUAGE_ENGLISH)
            , m_isCircleButtonBack(false)
            , m_console_type(CONSOLE_DESKTOP)
            , m_media_type(MEDIA_HDD)
            , m_codeSegmentSize(0)
            , m_bssSegmentSize(0)
            , m_dataSegmentSize(0)
            , m_imageSize(0)
            , m_mainThreadStackSize(0)
        {
        }

        alloc_t*     m_allocator;
        char         m_szAppTitle[128];
        char         m_szExePath[2048];
        char         m_szNickName[128];
        u64          m_uMemHeapTotalSize;
        u64          m_uMemHeapFreeSize;
        ELanguage    m_uLanguage;
        bool         m_isCircleButtonBack;
        EConsoleType m_console_type;
        EMediaType   m_media_type;
        s32          m_codeSegmentSize;
        s32          m_bssSegmentSize;
        s32          m_dataSegmentSize;
        s32          m_imageSize;
        const char*  m_OSVersion;
        bool         m_is64Bit;
        s32          m_logicalProcessors;
        s32          m_physicalProcessors;
        bool         m_hasHyperThreading;
        u64          m_cpuClockFrequency;
        u64          m_mainThreadStackSize;

        DCORE_CLASS_PLACEMENT_NEW_DELETE
    };

    void system_t::init(alloc_t* allocator)
    {
        void* instance_mem      = allocator->allocate(sizeof(instance_t), sizeof(void*));
        m_instance              = new (instance_mem) instance_t(allocator);
        m_instance->m_allocator = allocator;

        nmem::memset(m_instance->m_szAppTitle, 0, sizeof(m_instance->m_szAppTitle));
        nmem::memset(m_instance->m_szExePath, 0, sizeof(m_instance->m_szExePath));
        nmem::memset(m_instance->m_szNickName, 0, sizeof(m_instance->m_szNickName));

        m_instance->m_uMemHeapTotalSize = getRamTotal();
        m_instance->m_uMemHeapFreeSize  = getRamFree();

        m_instance->m_uLanguage = LANGUAGE_ENGLISH;

        m_instance->m_isCircleButtonBack = false;

        m_instance->m_console_type = CONSOLE_DESKTOP;
        m_instance->m_media_type   = MEDIA_HDD;

        m_instance->m_codeSegmentSize = 0;
        m_instance->m_bssSegmentSize  = 0;
        m_instance->m_dataSegmentSize = 0;
        m_instance->m_imageSize       = 0;

        m_instance->m_OSVersion         = getOSVersion();
        m_instance->m_is64Bit           = is64BitOS();
        m_instance->m_hasHyperThreading = getCPUInfo(m_instance->m_logicalProcessors, m_instance->m_physicalProcessors);
        m_instance->m_cpuClockFrequency = getCpuClockSpeed();
    }

    void system_t::update() { m_instance->m_uMemHeapFreeSize = getRamFree(); }

    void system_t::shutdown()
    {
        m_instance->~instance_t();
        m_instance = nullptr;
    }

} // namespace ncore

#endif