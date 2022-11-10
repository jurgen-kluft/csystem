#include "cbase/c_target.h"

#ifdef TARGET_PC

//==============================================================================
// INCLUDES
//==============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOMB
#define NOKANJI

#include <winsock2.h>
#include <windows.h>
#include <Psapi.h>
#include <winnt.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <Assert.h>
#include <Dbghelp.h>
#include <intrin.h> 
#include <array>
#include <vector>
#include <string>
#include <thread>

#include "cbase/c_debug.h"
#include "cbase/c_runes.h"
#include "cbase/c_memory.h"

#include "csystem/c_system.h"
#include "csystem/private/c_system_cpu.h"

//==============================================================================
// xCore namespace
//==============================================================================
namespace ncore
{
	namespace ncpu
	{
		struct cache_descr
		{
			u8				mId;
			const char*		mDescr;
		};

		cache_descr CacheLookup[] =
		{
			{ 0x1, "Instruction TLB: 4 KByte pages, 4-way set associative, 32 entries" },
			{ 0x2, "Instruction TLB: 4 MByte pages, fully associative, 2 entries" },
			{ 0x3, "Data TLB: 4 KByte pages, 4-way set associative, 64 entries" },
			{ 0x4, "Data TLB: 4 MByte pages, 4-way set associative, 8 entries" },
			{ 0x5, "Data TLB1: 4 MByte pages, 4-way set associative, 32 entries" },
			{ 0x6, "1st-level instruction cache: 8 KBytes, 4-way set associative, 32 byte line size" },
			{ 0x8, "1st-level instruction cache: 16 KBytes, 4-way set associative, 32 byte line size" },
			{ 0x9, "1st-level instruction cache: 32KBytes, 4-way set associative, 64 byte line size" },
			{ 0xA, "1st-level data cache: 8 KBytes, 2-way set associative, 32 byte line size" },
			{ 0xB, "Instruction TLB: 4 MByte pages, 4-way set associative, 4 entries" },
			{ 0xC, "1st-level data cache: 16 KBytes, 4-way set associative, 32 byte line size" },
			{ 0xD, "1st-level data cache: 16 KBytes, 4-way set associative, 64 byte line size" },
			{ 0xE, "1st-level data cache: 24 KBytes, 6-way set associative, 64 byte line size" },
			{ 0x1D, "2nd-level cache: 128 KBytes, 2-way set associative, 64 byte line size" },
			{ 0x21, "2nd-level cache: 256 KBytes, 8-way set associative, 64 byte line size" },
			{ 0x22, "3rd-level cache: 512 KBytes, 4-way set associative, 64 byte line size, 2 lines per sector" },
			{ 0x23, "3rd-level cache: 1 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector" },
			{ 0x24, "2nd-level cache: 1 MBytes, 16-way set associative, 64 byte line size" },
			{ 0x25, "3rd-level cache: 2 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector" },
			{ 0x29, "3rd-level cache: 4 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector" },
			{ 0x2C, "1st-level data cache: 32 KBytes, 8-way set associative, 64 byte line size" },
			{ 0x30, "1st-level instruction cache: 32 KBytes, 8-way set associative, 64 byte line size" },
			{ 0x40, "No 2nd-level cache or, if processor contains a valid 2nd-level cache, no 3rd-level cache" },
			{ 0x41, "2nd-level cache: 128 KBytes, 4-way set associative, 32 byte line size" },
			{ 0x42, "2nd-level cache: 256 KBytes, 4-way set associative, 32 byte line size" },
			{ 0x43, "2nd-level cache: 512 KBytes, 4-way set associative, 32 byte line size" },
			{ 0x44, "2nd-level cache: 1 MByte, 4-way set associative, 32 byte line size" },
			{ 0x45, "2nd-level cache: 2 MByte, 4-way set associative, 32 byte line size" },
			{ 0x46, "3rd-level cache: 4 MByte, 4-way set associative, 64 byte line size" },
			{ 0x47, "3rd-level cache: 8 MByte, 8-way set associative, 64 byte line size" },
			{ 0x48, "2nd-level cache: 3MByte, 12-way set associative, 64 byte line size" },
			{ 0x49, "3rd-level cache: 4MB, 16-way set associative, 64-byte line size (Intel Xeon processor MP, Family 0FH, Model06H); 2nd - level cache : 4 MByte, 16 - way set associative, 64 byte line size" },
			{ 0x4A, "3rd-level cache: 6MByte, 12-way set associative, 64 byte line size" },
			{ 0x4B, "3rd-level cache: 8MByte, 16-way set associative, 64 byte line size" },
			{ 0x4C, "3rd-level cache: 12MByte, 12-way set associative, 64 byte line size" },
			{ 0x4D, "3rd-level cache: 16MByte, 16-way set associative, 64 byte line size" },
			{ 0x4E, "2nd-level cache: 6MByte, 24-way set associative, 64 byte line size" },
			{ 0x4F, "Instruction TLB: 4 KByte pages, 32 entries" },
			{ 0x50, "Instruction TLB: 4 KByte and 2-MByte or 4-MByte pages, 64 entries" },
			{ 0x51, "Instruction TLB: 4 KByte and 2-MByte or 4-MByte pages, 128 entries" },
			{ 0x52, "Instruction TLB: 4 KByte and 2-MByte or 4-MByte pages, 256 entries" },
			{ 0x55, "Instruction TLB: 2-MByte or 4-MByte pages, fully associative, 7 entries" },
			{ 0x56, "Data TLB0: 4 MByte pages, 4-way set associative, 16 entries" },
			{ 0x57, "Data TLB0: 4 KByte pages, 4-way associative, 16 entries" },
			{ 0x59, "Data TLB0: 4 KByte pages, fully associative, 16 entries" },
			{ 0x5A, "Data TLB0: 2 MByte or 4 MByte pages, 4-way set associative, 32 entries" },
			{ 0x5B, "Data TLB: 4 KByte and 4 MByte pages, 64 entries" },
			{ 0x5C, "Data TLB: 4 KByte and 4 MByte pages,128 entries" },
			{ 0x5D, "Data TLB: 4 KByte and 4 MByte pages,256 entries" },
			{ 0x60, "1st-level data cache: 16 KByte, 8-way set associative, 64 byte line size" },
			{ 0x61, "Instruction TLB: 4 KByte pages, fully associative, 48 entries" },
			{ 0x63, "Data TLB: 2 MByte or 4 MByte pages, 4-way set associative, 32 entries and a separate array with 1 GBytepages, 4 - way set associative, 4 entries" },
			{ 0x64, "Data TLB: 4 KByte pages, 4-way set associative, 512 entries" },
			{ 0x66, "1st-level data cache: 8 KByte, 4-way set associative, 64 byte line size" },
			{ 0x67, "1st-level data cache: 16 KByte, 4-way set associative, 64 byte line size" },
			{ 0x68, "1st-level data cache: 32 KByte, 4-way set associative, 64 byte line size" },
			{ 0x6A, "uTLB: 4 KByte pages, 8-way set associative, 64 entries" },
			{ 0x6B, "DTLB: 4 KByte pages, 8-way set associative, 256 entries" },
			{ 0x6C, "DTLB: 2M/4M pages, 8-way set associative, 128 entries" },
			{ 0x6D, "DTLB: 1 GByte pages, fully associative, 16 entries" },
			{ 0x70, "Trace cache: 12 K-uop, 8-way set associative" },
			{ 0x71, "Trace cache: 16 K-uop, 8-way set associative" },
			{ 0x72, "Trace cache: 32 K-uop, 8-way set associative" },
			{ 0x76, "Instruction TLB: 2M/4M pages, fully associative, 8 entries " },
			{ 0x78, "2nd-level cache: 1 MByte, 4-way set associative, 64byte line size" },
			{ 0x79, "2nd-level cache: 128 KByte, 8-way set associative, 64 byte line size, 2 lines per sector" },
			{ 0x7A, "2nd-level cache: 256 KByte, 8-way set associative, 64 byte line size, 2 lines per sector" },
			{ 0x7B, "2nd-level cache: 512 KByte, 8-way set associative, 64 byte line size, 2 lines per sector" },
			{ 0x7C, "2nd-level cache: 1 MByte, 8-way set associative, 64 byte line size, 2 lines per sector" },
			{ 0x7D, "2nd-level cache: 2 MByte, 8-way set associative, 64byte line size" },
			{ 0x7F, "2nd-level cache: 512 KByte, 2-way set associative, 64-byte line size" },
			{ 0x80, "2nd-level cache: 512 KByte, 8-way set associative, 64-byte line size" },
			{ 0x82, "2nd-level cache: 256 KByte, 8-way set associative, 32 byte line size" },
			{ 0x83, "2nd-level cache: 512 KByte, 8-way set associative, 32 byte line size" },
			{ 0x84, "2nd-level cache: 1 MByte, 8-way set associative, 32 byte line size" },
			{ 0x85, "2nd-level cache: 2 MByte, 8-way set associative, 32 byte line size" },
			{ 0x86, "2nd-level cache: 512 KByte, 4-way set associative, 64 byte line size" },
			{ 0x87, "2nd-level cache: 1 MByte, 8-way set associative, 64 byte line size" },
			{ 0xA0, "DTLB: 4k pages, fully associative, 32 entries" },
			{ 0xB0, "Instruction TLB: 4 KByte pages, 4-way set associative, 128 entries" },
			{ 0xB1, "Instruction TLB: 2M pages, 4-way, 8 entries or 4M pages, 4-way, 4 entries" },
			{ 0xB2, "Instruction TLB: 4KByte pages, 4-way set associative, 64 entries" },
			{ 0xB3, "Data TLB: 4 KByte pages, 4-way set associative, 128 entries" },
			{ 0xB4, "Data TLB1: 4 KByte pages, 4-way associative, 256 entries" },
			{ 0xB5, "Instruction TLB: 4KByte pages, 8-way set associative, 64 entries" },
			{ 0xB6, "Instruction TLB: 4KByte pages, 8-way set associative, 128 entries" },
			{ 0xBA, "Data TLB1: 4 KByte pages, 4-way associative, 64 entries" },
			{ 0xC0, "Data TLB: 4 KByte and 4 MByte pages, 4-way associative, 8 entries" },
			{ 0xC1, "Shared 2nd-Level TLB: 4 KByte/2MByte pages, 8-way associative, 1024 entries" },
			{ 0xC2, "DTLB: 4 KByte/2 MByte pages, 4-way associative, 16 entries" },
			{ 0xC3, "Shared 2nd-Level TLB: 4 KByte /2 MByte pages, 6-way associative, 1536 entries. Also 1GBbyte pages, 4-way, 16 entries." },
			{ 0xC4, "DTLB: 2M/4M Byte pages, 4-way associative, 32 entries" },
			{ 0xCA, "Shared 2nd-Level TLB: 4 KByte pages, 4-way associative, 512 entries" },
			{ 0xD0, "3rd-level cache: 512 KByte, 4-way set associative, 64 byte line size" },
			{ 0xD1, "3rd-level cache: 1 MByte, 4-way set associative, 64 byte line size" },
			{ 0xD2, "3rd-level cache: 2 MByte, 4-way set associative, 64 byte line size" },
			{ 0xD6, "3rd-level cache: 1 MByte, 8-way set associative, 64 byte line size" },
			{ 0xD7, "3rd-level cache: 2 MByte, 8-way set associative, 64 byte line size" },
			{ 0xD8, "3rd-level cache: 4 MByte, 8-way set associative, 64 byte line size" },
			{ 0xDC, "3rd-level cache: 1.5 MByte, 12-way set associative, 64 byte line size" },
			{ 0xDD, "3rd-level cache: 3 MByte, 12-way set associative, 64 byte line size" },
			{ 0xDE, "3rd-level cache: 6 MByte, 12-way set associative, 64 byte line size" },
			{ 0xE2, "3rd-level cache: 2 MByte, 16-way set associative, 64 byte line size" },
			{ 0xE3, "3rd-level cache: 4 MByte, 16-way set associative, 64 byte line size" },
			{ 0xE4, "3rd-level cache: 8 MByte, 16-way set associative, 64 byte line size" },
			{ 0xEA, "3rd-level cache: 12MByte, 24-way set associative, 64 byte line size" },
			{ 0xEB, "3rd-level cache: 18MByte, 24-way set associative, 64 byte line size" },
			{ 0xEC, "3rd-level cache: 24MByte, 24-way set associative, 64 byte line size" },
			{ 0xF0, "64-Byte prefetching" },
			{ 0xF1, "128-Byte prefetching" }
		};

		static cache_descr*	CacheLookup_Find(u8 id)
		{

			return NULL;
		}

		class CPUInfo
		{
		public:
			void initCPUID();


			std::string getVendor() { return vendor; }
			std::string getBrand();
			int8_t getStepping();
			int8_t getModel();
			int8_t getFamily();
			std::string getProcessorType();
			uint16_t getExtendedModel();
			uint32_t getExtendedFamily();

			/* ISA extensions listed in ecx register */
			bool SSE3(void) { __cpuid(registers.data(), 1); return registers[2] & 0x1; }
			bool PCLMULQDQ(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 1); }
			bool DTES64(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 2); }
			bool MONITOR(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 3); }
			bool DS_CPL(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 4); }
			bool VMX(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 5); }
			bool SMX(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 6); }
			bool EIST(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 7); }
			bool TM2(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 8); }
			bool SSSE3(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 9); }
			bool CNXT_ID(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 10); }
			bool SDBG(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 11); }
			bool FMA(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 12); }
			bool CMPXCHG16B(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 13); }
			bool xTPR(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 14); }
			bool PDCM(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 15); }
			bool PCID(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 17); }
			bool DCA(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 18); }
			bool SSE41(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 19); }
			bool SSE42(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 20); }
			bool x2APIC(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 21); }
			bool MOVBE(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 22); }
			bool POPCNT(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 23); }
			bool TSC_Deadline(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 24); }
			bool AESNI(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 25); }
			bool XSAVE(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 26); }
			bool OSXSAVE(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 27); }
			bool AVX(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 28); }
			bool F16C(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 29); }
			bool RDRAND(void) { __cpuid(registers.data(), 1); return registers[2] & (0x1 << 30); }

			/* ISA extensions listed in edx register */
			bool FPU(void) { __cpuid(registers.data(), 1); return registers[3] & 0x1; }
			bool VME(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 1); }
			bool DE(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 2); }
			bool PSE(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 3); }
			bool TSC(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 4); }
			bool MSR(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 5); }
			bool PAE(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 6); }
			bool MCE(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 7); }
			bool CX8(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 8); }
			bool APIC(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 9); }
			bool SEP(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 11); }
			bool MTRR(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 12); }
			bool PGE(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 13); }
			bool MCA(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 14); }
			bool CMOV(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 15); }
			bool PAT(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 16); }
			bool PSE_36(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 17); }
			bool PSN(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 18); }
			bool CLFSH(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 19); }
			bool DS(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 21); }
			bool ACPI(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 22); }
			bool MMX(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 23); }
			bool FXSR(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 24); }
			bool SSE(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 25); }
			bool SSE2(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 26); }
			bool SS(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 27); }
			bool HTT(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 28); }
			bool TM(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 29); }
			bool PBE(void) { __cpuid(registers.data(), 1); return registers[3] & (0x1 << 31); }

			/* Extended Features */
			bool FSGSBASE(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 0); }
			bool SGX(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 2); }
			bool BMI1(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 3); }
			bool HLE(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 4); }
			bool AVX2(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 5); }
			bool BMI2(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 8); }
			bool RTM(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 11); }
			bool RDT_M(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 12); }
			bool MPX(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 14); }
			bool RDT_A(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 15); }
			bool RDSEED(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 18); }
			bool ADX(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 19); }
			bool SMAP(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 20); }
			bool IPT(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 25); }
			bool SHA(void) { __cpuidex(registers.data(), 7, 0); return registers[1] & (0x1 << 29); }



			/* Thermal and Power Management */
			bool DigitalTemp(void) { __cpuid(registers.data(), 6); return registers[0] & (0x1 << 0); }
			bool TurboBoost(void) { __cpuid(registers.data(), 6); return registers[0] & (0x1 << 1); }
			bool ARAT(void) { __cpuid(registers.data(), 6); return registers[0] & (0x1 << 2); }
			bool PLN(void) { __cpuid(registers.data(), 6); return registers[0] & (0x1 << 4); }
			bool ECMD(void) { __cpuid(registers.data(), 6); return registers[0] & (0x1 << 5); }
			bool PTM(void) { __cpuid(registers.data(), 6); return registers[0] & (0x1 << 6); }



			std::string getCacheTopology();


		private:
			void CPUIDExtended();

			std::array<int32_t, 4> getCPUID(std::array<int32_t, 4> regs, int32_t eax, int32_t ecx);
			std::string getCacheInfo();
			int MaxInputBasicCPUID;
			int MaxInputExtendedCPUID;

			std::array<int32_t, 4> registers; //will hold data from eax, ebx, ecx, edx

			std::string vendor;
			std::string brand;
		};


#define CHECK_BIT(var,pos) ((var) & (1<<(pos))
#define MASK_LOWBYTE(var)  ((var) & 0xff)


		void CPUInfo::initCPUID()
		{

			int32_t* startOfArray = registers.data(); //get the starting address of the array
			__cpuid(startOfArray, 0);
			MaxInputBasicCPUID = registers[0]; //number of standard CPUID leafs

			char vend[0x18] = { 0 }; //initialize a char array for the max length of the vendor string
			int count = 0;

			//per Intel documentation vendor name stored in ebx, edx, ecx -- need to switch order in array
			int32_t temp = registers[3]; //edx
			registers[3] = registers[2];
			registers[2] = temp;


			for (int i = 1; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					vend[count] = (char)registers[i]; // get the lowest order byte
					registers[i] = registers[i] >> 8;
					count++;
				}
			}
			vendor = vend;

			CPUIDExtended();
			startOfArray = nullptr; //clean dangling pointer
		}


		void CPUInfo::CPUIDExtended()
		{
			std::array<int32_t, 4> extendedRegisters;
			int32_t* startOfArray = extendedRegisters.data();

			int32_t extended = 0x80000000;
			__cpuid(startOfArray, extended);
			MaxInputExtendedCPUID = extendedRegisters[0];

			for (int k = extended + 1; k<MaxInputExtendedCPUID; k++)
			{
				__cpuid(startOfArray, k);
			}
		}


		int8_t CPUInfo::getStepping()
		{
			__cpuid(registers.data(), 0x1);
			return registers[0] & 0xf;
		}

		int8_t CPUInfo::getModel()
		{
			__cpuid(registers.data(), 0x1);
			int8_t mod = (registers[0] & 0xf0) >> 4;
			return mod;
		}

		int8_t CPUInfo::getFamily()
		{
			__cpuid(registers.data(), 0x1);
			return (registers[0] & 0xf00) >> 8;
		}

		uint16_t CPUInfo::getExtendedModel()
		{
			int8_t family = getFamily();
			if (family == 0x6 || family == 0xf)
			{
				__cpuid(registers.data(), 0x1);
				uint32_t extMod = (registers[0] & 0xf0000) >> 12;
				uint16_t displayModel = extMod + getModel();
				while ((displayModel & 0x1) == 0)
					displayModel >>= 1;
				return displayModel;
			}
			else return getModel();
		}

		std::array<int32_t, 4> CPUInfo::getCPUID(std::array<int32_t, 4> regs, int32_t eax, int32_t ecx)
		{
			__cpuidex(regs.data(), eax, ecx);
			return regs;
		}


		uint32_t CPUInfo::getExtendedFamily()
		{
			if (getFamily() != 0xf)
				return getFamily();
			else
			{
				(getCPUID(registers, 0x1, 0x0));
				uint32_t displayModel = registers[0] & 0xff00000 + getFamily();
				while ((displayModel & 0x1) == 0)
					displayModel >>= 1;
				return displayModel;
			}
		}

		std::string CPUInfo::getProcessorType()
		{
			getCPUID(registers, 0x1, 0x0);
			int8_t type = (registers[0] & 0x3000) >> 0xC;
			if (type == 0b00) return "Original OEM Processor";
			else if (type == 0b01) return "Intel OverDrive Processor";
			else if (type == 0b10) return "Dual Processor";
			return "Intel Reserved";
		}


		std::string CPUInfo::getBrand()
		{
			__cpuid(registers.data(), 0x80000000);
			if (registers[0] < 0x80000004) return NULL;

			const int CPUID_Leaves = 3;
			const int registersNeeded = 4;
			int encodedBrand[CPUID_Leaves][registersNeeded];

			for (int i = 0; i <= 2; i++)
			{
				__cpuid(registers.data(), i + 0x80000002);

				for (int j = 0; j<registersNeeded; j++)
					encodedBrand[i][j] = registers[j];
			}

			std::string brand = "";

			//outside loop for 3 exteneded CPUID leaves
			for (int ii = 0; ii < 3; ii++)
			{
				//loop for the 4 registers at each leaf
				for (int jj = 0; jj < 4; jj++)
				{
					//inner loop for 4 bytes in each register
					for (int kk = 0; kk<4; kk++)
					{
						if (encodedBrand[ii][jj] == 0x20202020) continue; //skip over excessive empty space
						brand += MASK_LOWBYTE(encodedBrand[ii][jj]);
						encodedBrand[ii][jj] >>= 8;
					}

				}
			}
			return brand;

		}

		std::string CPUInfo::getCacheTopology()
		{
			__cpuid(registers.data(), 2);
			uint32_t validCacheData = 0x80000000; //bit 31 of the register will be 0 iff there is a valid 1 byte cache descriptor

			uint8_t highestByte = 0;
			uint8_t topMiddleByte = 0;
			uint8_t botMiddleByte = 0;
			uint8_t bottomByte = 0;
			std::string cacheTopology;
			std::string cacheDescriptor = "";
			for (int i = 0; i<4; i++)
			{

				if (!CHECK_BIT(registers[i], 31)))
		{
			if (i != 0) bottomByte = MASK_LOWBYTE(registers[i]);
			registers[i] >>= 8;
			botMiddleByte = MASK_LOWBYTE(registers[i]);
			registers[i] >>= 8;
			topMiddleByte = MASK_LOWBYTE(registers[i]);
			registers[i] >>= 8;
			highestByte = MASK_LOWBYTE(registers[i]);
		}

				std::array<uint8_t, 4> cacheCodes = { highestByte ,topMiddleByte, botMiddleByte, bottomByte };
				for (int j = 0; j < 4; j++)
				{
					if (cacheCodes[j] == 0xFF)
						cacheTopology += getCacheInfo();
					else if (cacheCodes[j] != 0 && CacheLookup_Find(cacheCodes[j]) != NULL)
					{
						cacheDescriptor = CacheLookup_Find(cacheCodes[j])->mDescr;
						cacheTopology += cacheDescriptor;
						cacheTopology += "\n";
					}

				}
			}
			return cacheTopology;

		}


		std::string CPUInfo::getCacheInfo()
		{

			registers[0] = 0xffffffff;
			int count = 0;

			struct Cache /* Struct organized largest element to smallest element to minimize alignment padding */
			{
				int32_t numSets;
				uint32_t CacheSize;
				int16_t lineSize;
				int16_t linePartitions;
				int16_t lineAssociativity;
				int8_t level;
				int8_t type;
				bool selfInit;
				bool FullyAssoc;
				bool WriteBack;
				bool Inclusive;
				bool Mapping;
			};
			std::string cacheDescription = "";

			while (registers[0] != 0x0)
			{
				__cpuidex(registers.data(), 0x4, count);
				Cache cache;
				cache.type = registers[0] & 0x1F;
				if (cache.type == 0) break; //no more cache levels
				cache.level = ((registers[0] & 0xE0) >> 5);
				cache.selfInit = registers[0] & 0x100;
				cache.FullyAssoc = registers[0] & 0x200;
				cache.lineSize = (registers[1] & 0xFFF) + 1;
				cache.linePartitions = ((registers[1] & 0x3FF000) >> 12) + 1;
				cache.lineAssociativity = ((registers[1] & 0xFFC00000) >> 22) + 1;
				cache.numSets = (registers[2] & 0xFFFFFFFF) + 1;
				cache.WriteBack = registers[3] & 0x1;
				cache.Inclusive = registers[3] & 0x2;
				cache.Mapping = registers[3] & 0x4;

				cache.CacheSize = cache.lineAssociativity * cache.lineSize * cache.linePartitions * cache.numSets;
				int16_t numCores = ((registers[1] & 0xFFC00000) >> 22) + 1; //includes hyperthreaded cores
				while (numCores % 2 != 0) numCores++; //round-up to nearest power of 2 --> likely ok solution if cores disabled in BIOS
				if (HTT()) numCores >>= 1; //if hyperthreading, number of physical cores is half -- possible issue on AMD

				std::string postfix;
				int8_t cachepostfixCount = 0;

				while (cache.CacheSize >= 1024) //find out which is the best size abbreviation
				{
					cache.CacheSize >>= 10;
					cachepostfixCount++;
				}

				switch (cachepostfixCount)
				{
				case 1:
					postfix = "KB ";
					break;
				case 2:
					postfix = "MB ";
					break;
				case 3: //unlikely to happen for a long time
					postfix = "GB ";
					break;
				default: postfix = " bytes ";
				}

				std::string type;
				switch (cache.type)
				{
				case 1:
					type = "Data Cache ";
					break;
				case 2:
					type = "Instruction Cache ";
					break;
				case 3:
					type = "Unified Cache ";
					break;
				default: type = ""; //Error
				}

				cacheDescription += (cache.type < 3 ? std::to_string(numCores) + "x " : "") + "L" + std::to_string(cache.level) + " " + type
					+ std::to_string(cache.CacheSize) + postfix + (cache.FullyAssoc ? " Fully-Associative" : std::to_string(cache.lineAssociativity) + "-Way\n");

				count++;
			}
			return cacheDescription;
		}
	}
	//---------------------------------------------------------------------------------------------------------------------

	void			ncpu::initialize()
	{
	}

	s32				ncpu::getPhysicalProcessors ()
	{
		return std::thread::hardware_concurrency();
	}

	s32				ncpu::getLogicalProcessorsPerPhysical ()
	{
		return 1;
	}

};

#endif // TARGET_PC
