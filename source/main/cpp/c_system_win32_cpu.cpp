#include "cbase/c_target.h"

#ifdef TARGET_PC

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
#    include <intrin.h>
#    include <string>
#    include <thread>

#    include "cbase/c_debug.h"
#    include "cbase/c_memory.h"
#    include "csystem/c_system.h"

namespace ncore
{
    namespace ncpu
    {

        static void IntToString(char*& str, char const* strEnd, int32_t num)
        {
            // first move forward by the number of digits
            int32_t tempNum = num;
            while (tempNum != 0)
            {
                tempNum /= 10;
                str++;
            }

            // now write the digits to the string in reverse order
            tempNum = num;
            while (tempNum != 0)
            {
                *--str = '0' + (tempNum % 10);
                tempNum /= 10;
            }
        }

        static void AppendString(char*& str, char const* strEnd, const char* strToAppend)
        {
            while (*strToAppend != '\0' && str < strEnd)
            {
                *str++ = *strToAppend++;
            }
        }

        // From https://en.wikipedia.org/wiki/CPUID
        enum EFeature
        {
            FEATURE_PCLMULQDQ,
            FEATURE_DTES64,
            FEATURE_MONITOR,
            FEATURE_DS_CPL,
            FEATURE_VMX,
            FEATURE_SMX,
            FEATURE_EIST,
            FEATURE_TM2,
            FEATURE_CNXT_ID,
            FEATURE_SDBG,
            FEATURE_FMA,
            FEATURE_CMPXCHG16B,
            FEATURE_XTPR,
            FEATURE_PDCM,
            FEATURE_PCID,
            FEATURE_DCA,
            FEATURE_X2APIC,
            FEATURE_MOVBE,
            FEATURE_POPCNT,
            FEATURE_TSC_DEADLINE,
            FEATURE_AESNI,
            FEATURE_XSAVE,
            FEATURE_OSXSAVE,
            FEATURE_F16C,
            FEATURE_RDRAND,
            FEATURE_FPU,
            FEATURE_VME,
            FEATURE_DE,
            FEATURE_PSE,
            FEATURE_TSC,
            FEATURE_MSR,
            FEATURE_PAE,
            FEATURE_MCE,
            FEATURE_CX8,
            FEATURE_APIC,
            FEATURE_SEP,
            FEATURE_MTRR,
            FEATURE_PGE,
            FEATURE_MCA,
            FEATURE_CMOV,
            FEATURE_PAT,
            FEATURE_PSE_36,
            FEATURE_PSN,
            FEATURE_CLFSH,
            FEATURE_DS,
            FEATURE_ACPI,
            FEATURE_FXSR,
            FEATURE_SS,
            FEATURE_HTT,
            FEATURE_TM,
            FEATURE_PBE,
            FEATURE_FSGSBASE,
            FEATURE_SGX,
            FEATURE_BMI1,
            FEATURE_HLE,
            FEATURE_BMI2,
            FEATURE_RTM,
            FEATURE_RDT_M,
            FEATURE_MPX,
            FEATURE_RDT_A,
            FEATURE_RDSEED,
            FEATURE_ADX,
            FEATURE_SMAP,
            FEATURE_IPT,
            FEATURE_SHA,
            FEATURE_DIGITALTEMP,
            FEATURE_TURBOBOOST,
            FEATURE_ARAT,
            FEATURE_PLN,
            FEATURE_ECMD,
            FEATURE_PTM,
            FEATURE_MMX,
            FEATURE_SSE,
            FEATURE_SSE2,
            FEATURE_SSE3,
            FEATURE_SSSE3,
            FEATURE_SSE41,
            FEATURE_SSE42,
            FEATURE_AVX,
            FEATURE_AVX2,
            FEATURE_COUNT,
        };

        struct cache_descr
        {
            u8          mId;
            const char* mDescr;
        };

        cache_descr CacheLookup[] = {{0x1, "Instruction TLB: 4 KByte pages, 4-way set associative, 32 entries"},
                                     {0x2, "Instruction TLB: 4 MByte pages, fully associative, 2 entries"},
                                     {0x3, "Data TLB: 4 KByte pages, 4-way set associative, 64 entries"},
                                     {0x4, "Data TLB: 4 MByte pages, 4-way set associative, 8 entries"},
                                     {0x5, "Data TLB1: 4 MByte pages, 4-way set associative, 32 entries"},
                                     {0x6, "1st-level instruction cache: 8 KBytes, 4-way set associative, 32 byte line size"},
                                     {0x8, "1st-level instruction cache: 16 KBytes, 4-way set associative, 32 byte line size"},
                                     {0x9, "1st-level instruction cache: 32KBytes, 4-way set associative, 64 byte line size"},
                                     {0xA, "1st-level data cache: 8 KBytes, 2-way set associative, 32 byte line size"},
                                     {0xB, "Instruction TLB: 4 MByte pages, 4-way set associative, 4 entries"},
                                     {0xC, "1st-level data cache: 16 KBytes, 4-way set associative, 32 byte line size"},
                                     {0xD, "1st-level data cache: 16 KBytes, 4-way set associative, 64 byte line size"},
                                     {0xE, "1st-level data cache: 24 KBytes, 6-way set associative, 64 byte line size"},
                                     {0x1D, "2nd-level cache: 128 KBytes, 2-way set associative, 64 byte line size"},
                                     {0x21, "2nd-level cache: 256 KBytes, 8-way set associative, 64 byte line size"},
                                     {0x22, "3rd-level cache: 512 KBytes, 4-way set associative, 64 byte line size, 2 lines per sector"},
                                     {0x23, "3rd-level cache: 1 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector"},
                                     {0x24, "2nd-level cache: 1 MBytes, 16-way set associative, 64 byte line size"},
                                     {0x25, "3rd-level cache: 2 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector"},
                                     {0x29, "3rd-level cache: 4 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector"},
                                     {0x2C, "1st-level data cache: 32 KBytes, 8-way set associative, 64 byte line size"},
                                     {0x30, "1st-level instruction cache: 32 KBytes, 8-way set associative, 64 byte line size"},
                                     {0x40, "No 2nd-level cache or, if processor contains a valid 2nd-level cache, no 3rd-level cache"},
                                     {0x41, "2nd-level cache: 128 KBytes, 4-way set associative, 32 byte line size"},
                                     {0x42, "2nd-level cache: 256 KBytes, 4-way set associative, 32 byte line size"},
                                     {0x43, "2nd-level cache: 512 KBytes, 4-way set associative, 32 byte line size"},
                                     {0x44, "2nd-level cache: 1 MByte, 4-way set associative, 32 byte line size"},
                                     {0x45, "2nd-level cache: 2 MByte, 4-way set associative, 32 byte line size"},
                                     {0x46, "3rd-level cache: 4 MByte, 4-way set associative, 64 byte line size"},
                                     {0x47, "3rd-level cache: 8 MByte, 8-way set associative, 64 byte line size"},
                                     {0x48, "2nd-level cache: 3MByte, 12-way set associative, 64 byte line size"},
                                     {0x49, "3rd-level cache: 4MB, 16-way set associative, 64-byte line size (Intel Xeon processor MP, Family 0FH, Model06H); 2nd - level cache : 4 MByte, 16 - way set associative, 64 byte line size"},
                                     {0x4A, "3rd-level cache: 6MByte, 12-way set associative, 64 byte line size"},
                                     {0x4B, "3rd-level cache: 8MByte, 16-way set associative, 64 byte line size"},
                                     {0x4C, "3rd-level cache: 12MByte, 12-way set associative, 64 byte line size"},
                                     {0x4D, "3rd-level cache: 16MByte, 16-way set associative, 64 byte line size"},
                                     {0x4E, "2nd-level cache: 6MByte, 24-way set associative, 64 byte line size"},
                                     {0x4F, "Instruction TLB: 4 KByte pages, 32 entries"},
                                     {0x50, "Instruction TLB: 4 KByte and 2-MByte or 4-MByte pages, 64 entries"},
                                     {0x51, "Instruction TLB: 4 KByte and 2-MByte or 4-MByte pages, 128 entries"},
                                     {0x52, "Instruction TLB: 4 KByte and 2-MByte or 4-MByte pages, 256 entries"},
                                     {0x55, "Instruction TLB: 2-MByte or 4-MByte pages, fully associative, 7 entries"},
                                     {0x56, "Data TLB0: 4 MByte pages, 4-way set associative, 16 entries"},
                                     {0x57, "Data TLB0: 4 KByte pages, 4-way associative, 16 entries"},
                                     {0x59, "Data TLB0: 4 KByte pages, fully associative, 16 entries"},
                                     {0x5A, "Data TLB0: 2 MByte or 4 MByte pages, 4-way set associative, 32 entries"},
                                     {0x5B, "Data TLB: 4 KByte and 4 MByte pages, 64 entries"},
                                     {0x5C, "Data TLB: 4 KByte and 4 MByte pages,128 entries"},
                                     {0x5D, "Data TLB: 4 KByte and 4 MByte pages,256 entries"},
                                     {0x60, "1st-level data cache: 16 KByte, 8-way set associative, 64 byte line size"},
                                     {0x61, "Instruction TLB: 4 KByte pages, fully associative, 48 entries"},
                                     {0x63, "Data TLB: 2 MByte or 4 MByte pages, 4-way set associative, 32 entries and a separate array with 1 GBytepages, 4 - way set associative, 4 entries"},
                                     {0x64, "Data TLB: 4 KByte pages, 4-way set associative, 512 entries"},
                                     {0x66, "1st-level data cache: 8 KByte, 4-way set associative, 64 byte line size"},
                                     {0x67, "1st-level data cache: 16 KByte, 4-way set associative, 64 byte line size"},
                                     {0x68, "1st-level data cache: 32 KByte, 4-way set associative, 64 byte line size"},
                                     {0x6A, "uTLB: 4 KByte pages, 8-way set associative, 64 entries"},
                                     {0x6B, "DTLB: 4 KByte pages, 8-way set associative, 256 entries"},
                                     {0x6C, "DTLB: 2M/4M pages, 8-way set associative, 128 entries"},
                                     {0x6D, "DTLB: 1 GByte pages, fully associative, 16 entries"},
                                     {0x70, "Trace cache: 12 K-uop, 8-way set associative"},
                                     {0x71, "Trace cache: 16 K-uop, 8-way set associative"},
                                     {0x72, "Trace cache: 32 K-uop, 8-way set associative"},
                                     {0x76, "Instruction TLB: 2M/4M pages, fully associative, 8 entries "},
                                     {0x78, "2nd-level cache: 1 MByte, 4-way set associative, 64byte line size"},
                                     {0x79, "2nd-level cache: 128 KByte, 8-way set associative, 64 byte line size, 2 lines per sector"},
                                     {0x7A, "2nd-level cache: 256 KByte, 8-way set associative, 64 byte line size, 2 lines per sector"},
                                     {0x7B, "2nd-level cache: 512 KByte, 8-way set associative, 64 byte line size, 2 lines per sector"},
                                     {0x7C, "2nd-level cache: 1 MByte, 8-way set associative, 64 byte line size, 2 lines per sector"},
                                     {0x7D, "2nd-level cache: 2 MByte, 8-way set associative, 64byte line size"},
                                     {0x7F, "2nd-level cache: 512 KByte, 2-way set associative, 64-byte line size"},
                                     {0x80, "2nd-level cache: 512 KByte, 8-way set associative, 64-byte line size"},
                                     {0x82, "2nd-level cache: 256 KByte, 8-way set associative, 32 byte line size"},
                                     {0x83, "2nd-level cache: 512 KByte, 8-way set associative, 32 byte line size"},
                                     {0x84, "2nd-level cache: 1 MByte, 8-way set associative, 32 byte line size"},
                                     {0x85, "2nd-level cache: 2 MByte, 8-way set associative, 32 byte line size"},
                                     {0x86, "2nd-level cache: 512 KByte, 4-way set associative, 64 byte line size"},
                                     {0x87, "2nd-level cache: 1 MByte, 8-way set associative, 64 byte line size"},
                                     {0xA0, "DTLB: 4k pages, fully associative, 32 entries"},
                                     {0xB0, "Instruction TLB: 4 KByte pages, 4-way set associative, 128 entries"},
                                     {0xB1, "Instruction TLB: 2M pages, 4-way, 8 entries or 4M pages, 4-way, 4 entries"},
                                     {0xB2, "Instruction TLB: 4KByte pages, 4-way set associative, 64 entries"},
                                     {0xB3, "Data TLB: 4 KByte pages, 4-way set associative, 128 entries"},
                                     {0xB4, "Data TLB1: 4 KByte pages, 4-way associative, 256 entries"},
                                     {0xB5, "Instruction TLB: 4KByte pages, 8-way set associative, 64 entries"},
                                     {0xB6, "Instruction TLB: 4KByte pages, 8-way set associative, 128 entries"},
                                     {0xBA, "Data TLB1: 4 KByte pages, 4-way associative, 64 entries"},
                                     {0xC0, "Data TLB: 4 KByte and 4 MByte pages, 4-way associative, 8 entries"},
                                     {0xC1, "Shared 2nd-Level TLB: 4 KByte/2MByte pages, 8-way associative, 1024 entries"},
                                     {0xC2, "DTLB: 4 KByte/2 MByte pages, 4-way associative, 16 entries"},
                                     {0xC3, "Shared 2nd-Level TLB: 4 KByte /2 MByte pages, 6-way associative, 1536 entries. Also 1GBbyte pages, 4-way, 16 entries."},
                                     {0xC4, "DTLB: 2M/4M Byte pages, 4-way associative, 32 entries"},
                                     {0xCA, "Shared 2nd-Level TLB: 4 KByte pages, 4-way associative, 512 entries"},
                                     {0xD0, "3rd-level cache: 512 KByte, 4-way set associative, 64 byte line size"},
                                     {0xD1, "3rd-level cache: 1 MByte, 4-way set associative, 64 byte line size"},
                                     {0xD2, "3rd-level cache: 2 MByte, 4-way set associative, 64 byte line size"},
                                     {0xD6, "3rd-level cache: 1 MByte, 8-way set associative, 64 byte line size"},
                                     {0xD7, "3rd-level cache: 2 MByte, 8-way set associative, 64 byte line size"},
                                     {0xD8, "3rd-level cache: 4 MByte, 8-way set associative, 64 byte line size"},
                                     {0xDC, "3rd-level cache: 1.5 MByte, 12-way set associative, 64 byte line size"},
                                     {0xDD, "3rd-level cache: 3 MByte, 12-way set associative, 64 byte line size"},
                                     {0xDE, "3rd-level cache: 6 MByte, 12-way set associative, 64 byte line size"},
                                     {0xE2, "3rd-level cache: 2 MByte, 16-way set associative, 64 byte line size"},
                                     {0xE3, "3rd-level cache: 4 MByte, 16-way set associative, 64 byte line size"},
                                     {0xE4, "3rd-level cache: 8 MByte, 16-way set associative, 64 byte line size"},
                                     {0xEA, "3rd-level cache: 12MByte, 24-way set associative, 64 byte line size"},
                                     {0xEB, "3rd-level cache: 18MByte, 24-way set associative, 64 byte line size"},
                                     {0xEC, "3rd-level cache: 24MByte, 24-way set associative, 64 byte line size"},
                                     {0xF0, "64-Byte prefetching"},
                                     {0xF1, "128-Byte prefetching"}};

        static cache_descr* CacheLookup_Find(u8 id)
        {
            for (int i = 0; i < sizeof(CacheLookup) / sizeof(cache_descr); i++)
            {
                if (CacheLookup[i].mId == id)
                    return &CacheLookup[i];
            }
            return NULL;
        }

        class cpu_info_t
        {
        public:
            void     init(char*& str, char const* strEnd); // str -> vendor
            void     getBrand(char*& str, char const* strEnd);
            int8_t   getStepping();
            int8_t   getModel();
            int8_t   getFamily();
            void     getProcessorType(char*& str, char const* strEnd);
            uint16_t getExtendedModel();
            uint32_t getExtendedFamily();
            void     getCacheTopology(char*& str, const char* strEnd);
            bool     hasFeature(EFeature feature) const;

        private:
            void CPUIDExtended();

            void getCPUID(s32* regs, int32_t eax, int32_t ecx);
            void getCacheInfo(char*& str, const char* strEnd);
            int  MaxInputBasicCPUID;
            int  MaxInputExtendedCPUID;
        };

        struct feature_t
        {
            const char* name;
            bool        extended;
            int         function;
            int         subleaf;
            int         reg;
            int         bit;
        };

        // convert all the cpu_info_t feature functions into a data array of features
        static feature_t features[FEATURE_COUNT];
        static void      init_features(feature_t* f)
        {
            f[FEATURE_PCLMULQDQ]    = {"PCLMULQDQ", false, 1, 0, 2, (0x1 << 1)};
            f[FEATURE_DTES64]       = {"DTES64", false, 1, 0, 2, (0x1 << 2)};
            f[FEATURE_MONITOR]      = {"MONITOR", false, 1, 0, 2, (0x1 << 3)};
            f[FEATURE_DS_CPL]       = {"DS_CPL", false, 1, 0, 2, (0x1 << 4)};
            f[FEATURE_VMX]          = {"VMX", false, 1, 0, 2, (0x1 << 5)};
            f[FEATURE_SMX]          = {"SMX", false, 1, 0, 2, (0x1 << 6)};
            f[FEATURE_EIST]         = {"EIST", false, 1, 0, 2, (0x1 << 7)};
            f[FEATURE_TM2]          = {"TM2", false, 1, 0, 2, (0x1 << 8)};
            f[FEATURE_CNXT_ID]      = {"CNXT_ID", false, 1, 0, 2, (0x1 << 10)};
            f[FEATURE_SDBG]         = {"SDBG", false, 1, 0, 2, (0x1 << 11)};
            f[FEATURE_FMA]          = {"FMA", false, 1, 0, 2, (0x1 << 12)};
            f[FEATURE_CMPXCHG16B]   = {"CMPXCHG16B", false, 1, 0, 2, (0x1 << 13)};
            f[FEATURE_XTPR]         = {"xTPR", false, 1, 0, 2, (0x1 << 14)};
            f[FEATURE_PDCM]         = {"PDCM", false, 1, 0, 2, (0x1 << 15)};
            f[FEATURE_PCID]         = {"PCID", false, 1, 0, 2, (0x1 << 17)};
            f[FEATURE_DCA]          = {"DCA", false, 1, 0, 2, (0x1 << 18)};
            f[FEATURE_X2APIC]       = {"x2APIC", false, 1, 0, 2, (0x1 << 21)};
            f[FEATURE_MOVBE]        = {"MOVBE", false, 1, 0, 2, (0x1 << 22)};
            f[FEATURE_POPCNT]       = {"POPCNT", false, 1, 0, 2, (0x1 << 23)};
            f[FEATURE_TSC_DEADLINE] = {"TSC_Deadline", false, 1, 0, 2, (0x1 << 24)};
            f[FEATURE_AESNI]        = {"AESNI", false, 1, 0, 2, (0x1 << 25)};
            f[FEATURE_XSAVE]        = {"XSAVE", false, 1, 0, 2, (0x1 << 26)};
            f[FEATURE_OSXSAVE]      = {"OSXSAVE", false, 1, 0, 2, (0x1 << 27)};
            f[FEATURE_F16C]         = {"F16C", false, 1, 0, 2, (0x1 << 29)};
            f[FEATURE_RDRAND]       = {"RDRAND", false, 1, 0, 2, (0x1 << 30)};
            /* ISA extensions listed in edx register */
            f[FEATURE_FPU]    = {"FPU", false, 1, 0, 3, 0x1};
            f[FEATURE_VME]    = {"VME", false, 1, 0, 3, (0x1 << 1)};
            f[FEATURE_DE]     = {"DE", false, 1, 0, 3, (0x1 << 2)};
            f[FEATURE_PSE]    = {"PSE", false, 1, 0, 3, (0x1 << 3)};
            f[FEATURE_TSC]    = {"TSC", false, 1, 0, 3, (0x1 << 4)};
            f[FEATURE_MSR]    = {"MSR", false, 1, 0, 3, (0x1 << 5)};
            f[FEATURE_PAE]    = {"PAE", false, 1, 0, 3, (0x1 << 6)};
            f[FEATURE_MCE]    = {"MCE", false, 1, 0, 3, (0x1 << 7)};
            f[FEATURE_CX8]    = {"CX8", false, 1, 0, 3, (0x1 << 8)};
            f[FEATURE_APIC]   = {"APIC", false, 1, 0, 3, (0x1 << 9)};
            f[FEATURE_SEP]    = {"SEP", false, 1, 0, 3, (0x1 << 11)};
            f[FEATURE_MTRR]   = {"MTRR", false, 1, 0, 3, (0x1 << 12)};
            f[FEATURE_PGE]    = {"PGE", false, 1, 0, 3, (0x1 << 13)};
            f[FEATURE_MCA]    = {"MCA", false, 1, 0, 3, (0x1 << 14)};
            f[FEATURE_CMOV]   = {"CMOV", false, 1, 0, 3, (0x1 << 15)};
            f[FEATURE_PAT]    = {"PAT", false, 1, 0, 3, (0x1 << 16)};
            f[FEATURE_PSE_36] = {"PSE_36", false, 1, 0, 3, (0x1 << 17)};
            f[FEATURE_PSN]    = {"PSN", false, 1, 0, 3, (0x1 << 18)};
            f[FEATURE_CLFSH]  = {"CLFSH", false, 1, 0, 3, (0x1 << 19)};
            f[FEATURE_DS]     = {"DS", false, 1, 0, 3, (0x1 << 21)};
            f[FEATURE_ACPI]   = {"ACPI", false, 1, 0, 3, (0x1 << 22)};
            f[FEATURE_FXSR]   = {"FXSR", false, 1, 0, 3, (0x1 << 24)};
            f[FEATURE_SS]     = {"SS", false, 1, 0, 3, (0x1 << 27)};
            f[FEATURE_HTT]    = {"HTT", false, 1, 0, 3, (0x1 << 28)};
            f[FEATURE_TM]     = {"TM", false, 1, 0, 3, (0x1 << 29)};
            f[FEATURE_PBE]    = {"PBE", false, 1, 0, 3, (0x1 << 31)},
            /* Extended Features */
                f[FEATURE_FSGSBASE] = {"FSGSBASE", true, 7, 0, 1, (0x1 << 0)};
            f[FEATURE_SGX]          = {"SGX", true, 7, 0, 1, (0x1 << 2)};
            f[FEATURE_BMI1]         = {"BMI1", true, 7, 0, 1, (0x1 << 3)};
            f[FEATURE_HLE]          = {"HLE", true, 7, 0, 1, (0x1 << 4)};
            f[FEATURE_BMI2]         = {"BMI2", true, 7, 0, 1, (0x1 << 8)};
            f[FEATURE_RTM]          = {"RTM", true, 7, 0, 1, (0x1 << 11)};
            f[FEATURE_RDT_M]        = {"RDT_M", true, 7, 0, 1, (0x1 << 12)};
            f[FEATURE_MPX]          = {"MPX", true, 7, 0, 1, (0x1 << 14)};
            f[FEATURE_RDT_A]        = {"RDT_A", true, 7, 0, 1, (0x1 << 15)};
            f[FEATURE_RDSEED]       = {"RDSEED", true, 7, 0, 1, (0x1 << 18)};
            f[FEATURE_ADX]          = {"ADX", true, 7, 0, 1, (0x1 << 19)};
            f[FEATURE_SMAP]         = {"SMAP", true, 7, 0, 1, (0x1 << 20)};
            f[FEATURE_IPT]          = {"IPT", true, 7, 0, 1, (0x1 << 25)};
            f[FEATURE_SHA]          = {"SHA", true, 7, 0, 1, (0x1 << 29)};

            /* Thermal and Power Management */
            f[FEATURE_DIGITALTEMP] = {"DigitalTemp", false, 6, 0, 0, (0x1 << 0)};
            f[FEATURE_TURBOBOOST]  = {"TurboBoost", false, 6, 0, 0, (0x1 << 1)};
            f[FEATURE_ARAT]        = {"ARAT", false, 6, 0, 0, (0x1 << 2)};
            f[FEATURE_PLN]         = {"PLN", false, 6, 0, 0, (0x1 << 4)};
            f[FEATURE_ECMD]        = {"ECMD", false, 6, 0, 0, (0x1 << 5)};
            f[FEATURE_PTM]         = {"PTM", false, 6, 0, 0, (0x1 << 6)};

            f[FEATURE_MMX]   = {"MMX", false, 1, 0, 3, (0x1 << 23)};
            f[FEATURE_SSE]   = {"SSE", false, 1, 0, 3, (0x1 << 25)};
            f[FEATURE_SSE2]  = {"SSE2", false, 1, 0, 3, (0x1 << 26)};
            f[FEATURE_SSE3]  = {"SSE3", false, 1, 0, 2, (0x1 << 0)};
            f[FEATURE_SSSE3] = {"SSSE3", false, 1, 0, 2, (0x1 << 9)};
            f[FEATURE_SSE41] = {"SSE41", false, 1, 0, 2, (0x1 << 19)};
            f[FEATURE_SSE42] = {"SSE42", false, 1, 0, 2, (0x1 << 20)};

            f[FEATURE_AVX]  = {"AVX", false, 1, 0, 2, (0x1 << 28)};
            f[FEATURE_AVX2] = {"AVX2", true, 7, 0, 1, (0x1 << 5)};
        }

        bool cpu_info_t::hasFeature(EFeature feature) const
        {
            const feature_t f = features[feature];
            s32             regs[4];
            if (f.extended)
            {
                __cpuidex(regs, f.function, f.subleaf);
                return regs[f.reg] & f.bit;
            }
            else
            {
                __cpuid(regs, f.function);
                return regs[f.reg] & f.bit;
            }
        }

#    define CHECK_BIT(var, pos) ((var) & (1<<(pos))
#    define MASK_LOWBYTE(var) ((var)&0xff)

        void cpu_info_t::init(char*& str, char const* strEnd)
        {
            init_features(features);

            s32 regs[4];
            __cpuid(regs, 0);
            MaxInputBasicCPUID = regs[0]; // number of standard CPUID leafs

            // per Intel documentation vendor name stored in ebx, edx, ecx -- need to switch order in array
            int32_t temp = regs[3]; // edx
            regs[3]      = regs[2];
            regs[2]      = temp;

            char vendor[128];
            int  count = 0;
            for (int i = 1; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    vendor[count++] = (char)regs[i]; // get the lowest order byte
                    regs[i]         = regs[i] >> 8;
                }
            }
            vendor[count] = '\0';

            AppendString(str, strEnd, vendor);
            AppendString(str, strEnd, " ");

            CPUIDExtended();
        }

        void cpu_info_t::CPUIDExtended()
        {
            s32 regs[4];

            int32_t extended = 0x80000000;
            __cpuid(regs, extended);
            MaxInputExtendedCPUID = regs[0];

            for (int k = extended + 1; k < MaxInputExtendedCPUID; k++)
            {
                __cpuid(regs, k);
            }
        }

        int8_t cpu_info_t::getStepping()
        {
            s32 regs[4];
            __cpuid(regs, 0x1);
            return regs[0] & 0xf;
        }

        int8_t cpu_info_t::getModel()
        {
            s32 regs[4];
            __cpuid(regs, 0x1);
            int8_t mod = (regs[0] & 0xf0) >> 4;
            return mod;
        }

        int8_t cpu_info_t::getFamily()
        {
            s32 regs[4];
            __cpuid(regs, 0x1);
            return (regs[0] & 0xf00) >> 8;
        }

        uint16_t cpu_info_t::getExtendedModel()
        {
            int8_t family = getFamily();
            if (family == 0x6 || family == 0xf)
            {
                s32 regs[4];
                __cpuid(regs, 0x1);
                uint32_t extMod       = (regs[0] & 0xf0000) >> 12;
                uint16_t displayModel = extMod + getModel();
                while ((displayModel & 0x1) == 0)
                    displayModel >>= 1;
                return displayModel;
            }
            else
                return getModel();
        }

        void cpu_info_t::getCPUID(s32* regs, int32_t eax, int32_t ecx) { __cpuidex(regs, eax, ecx); }

        uint32_t cpu_info_t::getExtendedFamily()
        {
            if (getFamily() != 0xf)
                return getFamily();
            else
            {
                s32 regs[4];
                getCPUID(regs, 0x1, 0x0);
                uint32_t displayModel = regs[0] & 0xff00000 + getFamily();
                while ((displayModel & 0x1) == 0)
                    displayModel >>= 1;
                return displayModel;
            }
        }

        void cpu_info_t::getProcessorType(char*& str, char const* strEnd)
        {
            s32 regs[4];
            getCPUID(regs, 0x1, 0x0);
            int8_t type = (regs[0] & 0x3000) >> 0xC;
            if (type == 0b00)
                AppendString(str, strEnd, "Original OEM Processor ");
            else if (type == 0b01)
                AppendString(str, strEnd, "Intel OverDrive Processor ");
            else if (type == 0b10)
                AppendString(str, strEnd, "Dual Processor ");
            else
                AppendString(str, strEnd, "Intel Reserved ");
        }

        void cpu_info_t::getBrand(char*& str, char const* strEnd)
        {
            s32 regs[4];
            __cpuid(regs, 0x80000000);
            if (regs[0] < 0x80000004)
                return;

            const int CPUID_Leaves = 3;
            const int regsNeeded   = 4;
            int       encodedBrand[CPUID_Leaves][regsNeeded];

            for (int i = 0; i <= 2; i++)
            {
                __cpuid(regs, i + 0x80000002);

                for (int j = 0; j < regsNeeded; j++)
                    encodedBrand[i][j] = regs[j];
            }

            // outside loop for 3 exteneded CPUID leaves
            char brand[128];
            int  count = 0;
            for (int ii = 0; ii < 3; ii++)
            {
                // loop for the 4 regs at each leaf
                for (int jj = 0; jj < 4; jj++)
                {
                    // inner loop for 4 bytes in each register
                    for (int kk = 0; kk < 4; kk++)
                    {
                        if (encodedBrand[ii][jj] == 0x20202020)
                            continue; // skip over excessive empty space

                        brand[count++] = MASK_LOWBYTE(encodedBrand[ii][jj]);
                        encodedBrand[ii][jj] >>= 8;
                    }
                }
            }
            brand[count] = '\0';
            AppendString(str, strEnd, brand);
            AppendString(str, strEnd, " ");
        }

        void cpu_info_t::getCacheTopology(char*& str, char const* strEnd)
        {
            s32 regs[4];
            __cpuid(regs, 2);
            uint32_t validCacheData = 0x80000000; // bit 31 of the register will be 0 iff there is a valid 1 byte cache descriptor

            uint8_t highestByte   = 0;
            uint8_t topMiddleByte = 0;
            uint8_t botMiddleByte = 0;
            uint8_t bottomByte    = 0;

            for (int i = 0; i < 4; i++)
            {
                if (!CHECK_BIT(regs[i], 31)))
                    {
                        if (i != 0)
                            bottomByte = MASK_LOWBYTE(regs[i]);
                        regs[i] >>= 8;
                        botMiddleByte = MASK_LOWBYTE(regs[i]);
                        regs[i] >>= 8;
                        topMiddleByte = MASK_LOWBYTE(regs[i]);
                        regs[i] >>= 8;
                        highestByte = MASK_LOWBYTE(regs[i]);
                    }

                uint8_t cacheCodes[] = {highestByte, topMiddleByte, botMiddleByte, bottomByte};
                for (int j = 0; j < 4; j++)
                {
                    if (cacheCodes[j] == 0xFF)
                    {
                        getCacheInfo(str, strEnd);
                    }
                    else if (cacheCodes[j] != 0 && CacheLookup_Find(cacheCodes[j]) != NULL)
                    {
                        const char* cacheDescriptor = CacheLookup_Find(cacheCodes[j])->mDescr;
                        AppendString(str, strEnd, cacheDescriptor);
                        AppendString(str, strEnd, "\n");
                    }
                }
            }
        }

        void cpu_info_t::getCacheInfo(char*& str, const char* strEnd)
        {
            s32 regs[4];
            regs[0]   = 0xffffffff;
            int count = 0;

            struct Cache /* Struct organized largest element to smallest element to minimize alignment padding */
            {
                int32_t  numSets;
                uint32_t CacheSize;
                int16_t  lineSize;
                int16_t  linePartitions;
                int16_t  lineAssociativity;
                int8_t   level;
                int8_t   type;
                bool     selfInit;
                bool     FullyAssoc;
                bool     WriteBack;
                bool     Inclusive;
                bool     Mapping;
            };

            while (regs[0] != 0x0)
            {
                __cpuidex(regs, 0x4, count);
                Cache cache;
                cache.type = regs[0] & 0x1F;
                if (cache.type == 0)
                    break; // no more cache levels
                cache.level             = ((regs[0] & 0xE0) >> 5);
                cache.selfInit          = regs[0] & 0x100;
                cache.FullyAssoc        = regs[0] & 0x200;
                cache.lineSize          = (regs[1] & 0xFFF) + 1;
                cache.linePartitions    = ((regs[1] & 0x3FF000) >> 12) + 1;
                cache.lineAssociativity = ((regs[1] & 0xFFC00000) >> 22) + 1;
                cache.numSets           = (regs[2] & 0xFFFFFFFF) + 1;
                cache.WriteBack         = regs[3] & 0x1;
                cache.Inclusive         = regs[3] & 0x2;
                cache.Mapping           = regs[3] & 0x4;

                cache.CacheSize  = cache.lineAssociativity * cache.lineSize * cache.linePartitions * cache.numSets;
                int16_t numCores = ((regs[1] & 0xFFC00000) >> 22) + 1; // includes hyperthreaded cores
                while (numCores % 2 != 0)
                    numCores++; // round-up to nearest power of 2 --> likely ok solution if cores disabled in BIOS
                if (hasFeature(FEATURE_HTT))
                    numCores >>= 1; // if hyperthreading, number of physical cores is half -- possible issue on AMD

                int8_t cachepostfixCount = 0;
                while (cache.CacheSize >= 1024) // find out which is the best size abbreviation
                {
                    cache.CacheSize >>= 10;
                    cachepostfixCount++;
                }

                const char* cachePostFixes[] = {"bytes", "KB", "MB", "GB"};
                const char* postfix          = cachePostFixes[cachepostfixCount > 3 ? 0 : cachepostfixCount];

                const char* types[] = {"Data Cache ", "Instruction Cache ", "Unified Cache ", " "};
                const char* type    = types[(cache.type - 1) & 0x3];

                if (cache.type < 3)
                {
                    IntToString(str, strEnd, numCores);
                    AppendString(str, strEnd, "x ");
                }

                AppendString(str, strEnd, "L");
                IntToString(str, strEnd, cache.level);
                AppendString(str, strEnd, " ");
                AppendString(str, strEnd, type);
                IntToString(str, strEnd, cache.CacheSize);
                AppendString(str, strEnd, postfix);
                if (cache.FullyAssoc)
                {
                    AppendString(str, strEnd, "Fully-Associative");
                }
                else
                {
                    IntToString(str, strEnd, cache.lineAssociativity);
                    AppendString(str, strEnd, "-Way\n");
                }

                count++;
            }
        }

        // s32  cpu_info_t::getPhysicalProcessors() { return std::thread::hardware_concurrency(); }
        // s32  cpu_info_t::getLogicalProcessorsPerPhysical() { return 1; }

    } // namespace ncpu

    void GetCpuInfo()
    {
        char cpuInfo[1024];
        cpuInfo[0]                   = '\0';
        cpuInfo[sizeof(cpuInfo) - 1] = '\0';
        char* str                    = cpuInfo;

        ncpu::cpu_info_t info;
        info.init(str, cpuInfo + 1024 - 1);
        info.getBrand(str, cpuInfo + 1024 - 1);
        info.getProcessorType(str, cpuInfo + 1024 - 1);
        info.getCacheTopology(str, cpuInfo + 1024 - 1);
    }
    //---------------------------------------------------------------------------------------------------------------------

} // namespace ncore

#endif // TARGET_PC
