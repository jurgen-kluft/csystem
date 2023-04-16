#include "cbase/c_base.h"
#include "cbase/c_allocator.h"

#if defined(TARGET_PC)

#    include "cbase/c_debug.h"
#    include "cbase/c_memory.h"
#    include "cbase/c_console.h"

#    include "csystem/c_system.h"

#    define WIN32_LEAN_AND_MEAN
#    define NOGDI
#    define NOMB
#    define NOKANJI

#    include <winsock2.h>
#    include <windows.h>
#    include <Psapi.h>
#    include <winnt.h>
#    include <mmsystem.h>
#    include <stdlib.h>
#    include <string.h>
#    include <tchar.h>
#    include <Assert.h>
#    include <Dbghelp.h>

#    include <Iphlpapi.h>
#    pragma comment(lib, "iphlpapi.lib")
#    pragma comment(lib, "DbgHelp.lib")

#    include "cbase/c_integer.h"

namespace ncore
{
    enum ESystemPC
    {
        MAX_EXE_PATH       = 256,
        MAX_STACK_NAME_LEN = 1024
    };

    static bool is64BitOS() { return sizeof(void*) == 8; }

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
            , m_pContext(NULL)
        {
        }

        alloc_t*     m_allocator;
        char         m_szAppTitle[128];
        char         m_szExePath[MAX_EXE_PATH + 1];
        char         m_szNickName[128];
        u64          m_uMemHeapTotalSize;
        ELanguage    m_uLanguage;
        bool         m_isCircleButtonBack;
        EConsoleType m_console_type;
        EMediaType   m_media_type;
        s32          m_codeSegmentSize;
        s32          m_bssSegmentSize;
        s32          m_dataSegmentSize;
        s32          m_imageSize;
        u64          m_mainThreadStackSize;

        DCORE_CLASS_PLACEMENT_NEW_DELETE
    };

    void InitializeExecutableInfo(s32& codeSegmentSize, s32& bssSegmentSize, s32& dataSegmentSize, s32& imageSize, s32& mainThreadStackSize)
    {
        HMODULE cb[64];

        DWORD cbNeeded;
        EnumProcessModules(GetCurrentProcess(), cb, sizeof(cb), &cbNeeded);

        HMODULE                  hModule = cb[0];
        MEMORY_BASIC_INFORMATION mbi;

        s32 codeSegmentSize     = 0;
        s32 bssSegmentSize      = 0;
        s32 dataSegmentSize     = 0;
        s32 imageSize           = 0;
        s32 mainThreadStackSize = 0;

        if (!VirtualQuery(hModule, &mbi, sizeof(mbi)))
        {
            return;
        }

        ::GetModuleFileNameExA(GetCurrentProcess(), hModule, c->m_szExePath, MAX_EXE_PATH);

        PVOID hMod = (PVOID)mbi.AllocationBase;

        if (0 != hMod)
        {
            PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;
            PIMAGE_NT_HEADERS pNtHdr  = (PIMAGE_NT_HEADERS)((u8*)hMod + pDosHdr->e_lfanew);

#    ifdef TARGET_32BIT
            PIMAGE_OPTIONAL_HEADER32 pOptionalHdr = &pNtHdr->OptionalHeader;
#    else
            PIMAGE_OPTIONAL_HEADER64 pOptionalHdr = &pNtHdr->OptionalHeader;
#    endif

            codeSegmentSize     = pOptionalHdr->SizeOfCode;
            bssSegmentSize      = pOptionalHdr->SizeOfUninitializedData;
            dataSegmentSize     = pOptionalHdr->SizeOfInitializedData;
            imageSize           = pOptionalHdr->SizeOfImage;
            mainThreadStackSize = pOptionalHdr->SizeOfStackReserve;
        }
    }

#    define CASE_EXCEPTION(x) \
        case EXCEPTION_##x: return __T(#x);

    char* GetExceptionString(DWORD dwCode)
    {
        switch (dwCode)
        {
            CASE_EXCEPTION(ACCESS_VIOLATION)
            CASE_EXCEPTION(DATATYPE_MISALIGNMENT)
            CASE_EXCEPTION(BREAKPOINT)
            CASE_EXCEPTION(SINGLE_STEP)
            CASE_EXCEPTION(ARRAY_BOUNDS_EXCEEDED)
            CASE_EXCEPTION(FLT_DENORMAL_OPERAND)
            CASE_EXCEPTION(FLT_DIVIDE_BY_ZERO)
            CASE_EXCEPTION(FLT_INEXACT_RESULT)
            CASE_EXCEPTION(FLT_INVALID_OPERATION)
            CASE_EXCEPTION(FLT_OVERFLOW)
            CASE_EXCEPTION(FLT_STACK_CHECK)
            CASE_EXCEPTION(FLT_UNDERFLOW)
            CASE_EXCEPTION(INT_DIVIDE_BY_ZERO)
            CASE_EXCEPTION(INT_OVERFLOW)
            CASE_EXCEPTION(PRIV_INSTRUCTION)
            CASE_EXCEPTION(IN_PAGE_ERROR)
            CASE_EXCEPTION(ILLEGAL_INSTRUCTION)
            CASE_EXCEPTION(NONCONTINUABLE_EXCEPTION)
            CASE_EXCEPTION(STACK_OVERFLOW)
            CASE_EXCEPTION(INVALID_DISPOSITION)
            CASE_EXCEPTION(GUARD_PAGE)
            CASE_EXCEPTION(INVALID_HANDLE)
        }

        // If not one of the "known" exceptions, try to get the string from NTDLL.DLL's message table.

        static TCHAR szBuffer[512] = {0};

        FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandle(__T("NTDLL.DLL")), dwCode, 0, szBuffer, sizeof(szBuffer), 0);
        return szBuffer;
    }

#    undef CASE_EXCEPTION

    BOOL GetLogicalAddress(PVOID addr, PTSTR szModule, DWORD len, DWORD& section, u64& offset)
    {
        MEMORY_BASIC_INFORMATION mbi;

        if (!VirtualQuery(addr, &mbi, sizeof(mbi)))
        {
            return FALSE;
        }

        PVOID hMod = (PVOID)mbi.AllocationBase;

        if (!GetModuleFileName((HMODULE)hMod, szModule, len))
        {
            return FALSE;
        }

        // Point to the DOS header in memory
        PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

        // From the DOS header, find the NT (PE) header
        PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)((u8*)hMod + pDosHdr->e_lfanew);

        PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHdr);

        u64 rva = ((u8*)addr - (u8*)hMod); // RVA is offset from module load address

        // Iterate through the section table, looking for the one that encompasses
        // the linear address.
        for (unsigned i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++, pSection++)
        {
            DWORD sectionStart = pSection->VirtualAddress;
            DWORD sectionEnd   = sectionStart + x_intu::max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

            // Is the address in this section???
            if ((rva >= sectionStart) && (rva <= sectionEnd))
            {
                // Yes, address is in the section.  Calculate section and offset,
                // and store in the "section" & "offset" params, which were
                // passed by reference.
                section = i + 1;
                offset  = rva - sectionStart;
                return TRUE;
            }
        }

        return FALSE; // Should never get here!
    }

    BOOL CALLBACK ReadProcessMemoryProc64(HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead)
    {
        SIZE_T xRead;
        BOOL   boOK = ReadProcessMemory(hProcess, (void*)lpBaseAddress, lpBuffer, nSize, &xRead);

        if (boOK)
        {
            *lpNumberOfBytesRead = (DWORD)xRead;
        }

        return boOK;
    }

    void GenerateExceptionReport(PEXCEPTION_POINTERS pExceptionInfo)
    {
#    if 0
			// Start out with a banner
			console->writeLine( "//=====================================================" );

			PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

			// First print information about the type of fault
			console->writeLine("Exception: code %08X %s", va_list_t(va_t((u32)pExceptionRecord->ExceptionCode), va_t(GetExceptionString(pExceptionRecord->ExceptionCode))) );

			// Now print information about where the fault occured
			TCHAR szFaultingModule[MAX_PATH];
			DWORD section, offset;
			GetLogicalAddress(	pExceptionRecord->ExceptionAddress, szFaultingModule, sizeof( szFaultingModule ), section, offset );

			console->writeLine( "Fault address:  %08X %02X:%08X %s", va_list_t(va_t((u32)pExceptionRecord->ExceptionAddress), va_t((u32)section), va_t((u32)offset), va_t(szFaultingModule)) );

			PCONTEXT pCtx = pExceptionInfo->ContextRecord;

			// Show the registers
#        ifdef _M_IX86 // Intel Only!
			console->writeLine( "\nRegisters:" );

			console->writeLine( "EAX:%08X", va_list_t(va_t((u32)pCtx->Eax)) );
			console->writeLine( "EBX:%08X", va_list_t(va_t((u32)pCtx->Ebx)) );
			console->writeLine( "ECX:%08X", va_list_t(va_t((u32)pCtx->Ecx)) );
			console->writeLine( "EDX:%08X", va_list_t(va_t((u32)pCtx->Edx)) );
			console->writeLine( "ESI:%08X", va_list_t(va_t((u32)pCtx->Esi)) );
			console->writeLine( "EDI:%08X", va_list_t(va_t((u32)pCtx->Edi)) );

			console->writeLine( "CS:EIP:%04X:%08X", va_list_t(va_t((u32)pCtx->SegCs), va_t((u32)pCtx->Eip)) );
			console->writeLine( "SS:ESP:%04X:%08X  EBP:%08X", va_list_t(va_t((u32)pCtx->SegSs), va_t((u32)pCtx->Esp), va_t((u32)pCtx->Ebp)) );
			console->writeLine( "DS:%04X  ES:%04X  FS:%04X  GS:%04X", va_list_t(va_t((u32)pCtx->SegDs), va_t((u32)pCtx->SegEs), va_t((u32)pCtx->SegFs), va_t((u32)pCtx->SegGs)) );
			console->writeLine( "Flags:%08X", va_list_t(va_t((u32)pCtx->EFlags)) );
#        endif

			u32	uDepth;
			u64	uStack[256];
			CreateStackTrace(0, uStack, uDepth, 256, pCtx );
			DumpStackTrace(uStack, uDepth, 0);

			console->writeLine();
#    endif
    }

    LONG WINAPI UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
    {
        GenerateExceptionReport(pExceptionInfo);

        if (system_t::getSystem()->m_instance->m_pPreviousFilter)
        {
            return system_t::getSystem()->m_instance->m_pPreviousFilter(pExceptionInfo);
        }
        else
        {
            return EXCEPTION_EXECUTE_HANDLER;
        }
    }

    void createStackTrace(u32 uStartIndex, u64* pTrace, u32& ruDepth, u32 uMaxDepth)
    {
        RtlCaptureContext(&m_instance->m_xContext);

        STACKFRAME64 StackFrame;

        memset(&StackFrame, 0, sizeof(StackFrame));
        CONTEXT& xContext = m_instance->m_xContext;

#    if defined(TARGET_32BIT)
        ADDRESS64 AddrPC    = {xContext.Eip, 0, AddrModeFlat};
        ADDRESS64 AddrFrame = {xContext.Esp, 0, AddrModeFlat};
        ADDRESS64 AddrStack = {xContext.Esp, 0, AddrModeFlat};
#    else
        ADDRESS64 AddrPC = {xContext.Rip, 0, AddrModeFlat};
        ADDRESS64 AddrFrame = {xContext.Rsp, 0, AddrModeFlat};
        ADDRESS64 AddrStack = {xContext.Rsp, 0, AddrModeFlat};
#    endif
        StackFrame.AddrPC    = AddrPC;
        StackFrame.AddrFrame = AddrFrame;
        StackFrame.AddrStack = AddrStack;

        u32 uDepth = 0;
        ruDepth    = 0;

        while (1)
        {
            BOOL boOK = StackWalk64(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &StackFrame, m_instance->m_pContext, ReadProcessMemoryProc64, SymFunctionTableAccess64, SymGetModuleBase64, NULL);

            if (!boOK || StackFrame.AddrPC.Offset == 0)
            {
                break;
            }
            else
            {
                if (uDepth >= uStartIndex)
                {
                    pTrace[ruDepth] = StackFrame.AddrPC.Offset;
                    ruDepth++;

                    if (ruDepth >= uMaxDepth)
                    {
                        break;
                    }
                }

                uDepth++;
            }
        }
    }

    void dumpStackTrace(ctxt_t* ctx, u64* pTrace, u32 uDepth, const char* sExePath)
    {
#    if 0
			BOOL boOK = SymInitialize(GetCurrentProcess(), NULL, TRUE);

			for(u32 uI = 0; uI < uDepth; uI++)
			{
				IMAGEHLP_SYMBOL64*	pSym = (IMAGEHLP_SYMBOL64 *) malloc(sizeof(IMAGEHLP_SYMBOL64) + MAX_STACK_NAME_LEN);
				IMAGEHLP_LINE64		line;

				memset(&line, 0, sizeof(line));
				line.SizeOfStruct = sizeof(line);

				memset(pSym, 0, sizeof(IMAGEHLP_SYMBOL64) + MAX_STACK_NAME_LEN);
				pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
				pSym->MaxNameLength = MAX_STACK_NAME_LEN;

				DWORD64	dwDisplacement = 0;
				boOK = SymGetSymFromAddr64(GetCurrentProcess(), pTrace[uI], &dwDisplacement, pSym);

				char	szName[MAX_STACK_NAME_LEN];
				char	szFullName[MAX_STACK_NAME_LEN];

				ascii::copy(szName, szName + MAX_STACK_NAME_LEN - 1, "", NULL);
				ascii::copy(szFullName, szFullName + MAX_STACK_NAME_LEN -1, "", NULL);

				if(!boOK)
				{
					free(pSym);
					break;
				}
				else
				{
					UnDecorateSymbolName( pSym->Name, szName, MAX_STACK_NAME_LEN, UNDNAME_NAME_ONLY );
					UnDecorateSymbolName( pSym->Name, szFullName, MAX_STACK_NAME_LEN, UNDNAME_COMPLETE );

					free(pSym);
				}

				boOK = SymGetLineFromAddr64(GetCurrentProcess(), pTrace[uI], (DWORD*)&dwDisplacement, &line);

				if(boOK)
				{
					console->writeLine("%s(%d) : %s (%s)", va_list_t(va_t(line.FileName), va_t((u32)line.LineNumber), va_t(szName), va_t(szFullName)));
				}
			}

			SymCleanup(GetCurrentProcess());
#    endif
    }

    void system_t::init(alloc_t* a)
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

        if (!IsDebuggerPresent())
        {
            // Install the unhandled exception filter function
            m_instance->m_pPreviousFilter = SetUnhandledExceptionFilter(UnhandledExceptionFilter);
        }

        InitializeExecutableInfo();
    }

    void system_t::update() {}

    void system_t::shutdown()
    {
        if (!IsDebuggerPresent())
        {
            // Uninstall the unhandled exception filter function
            SetUnhandledExceptionFilter(m_instance->m_pPreviousFilter);
        }

        m_instance->m_allocator->deallocate(m_instance);
    }

    //---------------------------------------------------------------------------------------------------------------------

    void fatalError(void) { exit(-1); }

    bool         system_t::isCircleButtonBack() const { return m_instance->m_isCircleButtonBack; }
    s32          system_t::getNumCores() const { return m_instance->m_num_cores; }
    s32          system_t::getNumHwThreadsPerCore() const { return m_instance->m_num_hwthreads_per_core; }
    u64          system_t::getCoreClockFrequency() const { return m_instance->m_core_clock_frequency; }
    const char*  system_t::getPlatformName() const { return "Win32"; }
    EConsoleType system_t::getConsoleType() { return m_instance->m_console_type; }
    EMediaType   system_t::getMediaType() { return m_instance->m_media_type; }
    ELanguage    system_t::getLanguage() { return m_instance->m_uLanguage; }
    void         system_t::setLanguage(ELanguage language) { m_instance->m_uLanguage = language; }
    u64          system_t::getTotalMemorySize() { return m_instance->m_uMemHeapTotalSize; }
    u64          system_t::getCurrentSystemMemory() { return m_instance->m_uMemHeapTotalSize; }

}; // namespace ncore

#endif // TARGET_PC
