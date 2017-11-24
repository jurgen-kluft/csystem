#include "xbase/x_target.h"

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

#include "xbase/x_debug.h"
#include "xbase/x_string_ascii.h"
#include "xbase/x_memory_std.h"

#include "xsystem/x_system.h"


//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	namespace xsystem
	{
		ESystemFlags		ParseSystemFlags(ESystemFlags systemflags, int argc, const char** argv)
		{
			bool data_from_dvd = false;
			bool mem_32mb   = false;
			bool mem_64mb   = false;
			bool mem_128mb  = false;
			bool mem_256mb  = false;
			bool mem_512mb  = false;
			bool mem_1024mb = false;
			bool stack_512kb = false;
			bool dev = false;
			for (s32 i=0; i<argc; ++i)
			{
				data_from_dvd = data_from_dvd | (x_strcmp(argv[i], "-host")==0);
				mem_32mb    = mem_32mb    | (x_strcmp(argv[i], "-mem32")==0);
				mem_64mb    = mem_64mb    | (x_strcmp(argv[i], "-mem64")==0);
				mem_128mb   = mem_128mb   | (x_strcmp(argv[i], "-mem128")==0);
				mem_256mb   = mem_256mb   | (x_strcmp(argv[i], "-mem256")==0);
				mem_512mb   = mem_512mb   | (x_strcmp(argv[i], "-mem512")==0);
				mem_1024mb  = mem_1024mb  | (x_strcmp(argv[i], "-mem1024")==0);
				stack_512kb = stack_512kb | (x_strcmp(argv[i], "-stack512")==0);
				dev         = dev         | (x_strcmp(argv[i], "-dev")==0);
			}

			systemflags = dev           ? SetSystemFlag(systemflags, xcore::xsystem::MODE_MASK     , xcore::xsystem::MODE_DEV       ) : systemflags;
			systemflags = data_from_dvd ? SetSystemFlag(systemflags, xcore::xsystem::DATA_FROM_MASK, xcore::xsystem::DATA_FROM_DVD  ) : systemflags;
			systemflags = mem_32mb      ? SetSystemFlag(systemflags, xcore::xsystem::MEM_MASK      , xcore::xsystem::MEM_32MB       ) : systemflags;
			systemflags = mem_64mb      ? SetSystemFlag(systemflags, xcore::xsystem::MEM_MASK      , xcore::xsystem::MEM_64MB       ) : systemflags;
			systemflags = mem_128mb     ? SetSystemFlag(systemflags, xcore::xsystem::MEM_MASK      , xcore::xsystem::MEM_128MB      ) : systemflags;
			systemflags = mem_256mb     ? SetSystemFlag(systemflags, xcore::xsystem::MEM_MASK      , xcore::xsystem::MEM_256MB      ) : systemflags;
			systemflags = mem_512mb     ? SetSystemFlag(systemflags, xcore::xsystem::MEM_MASK      , xcore::xsystem::MEM_512MB      ) : systemflags;
			systemflags = mem_1024mb    ? SetSystemFlag(systemflags, xcore::xsystem::MEM_MASK      , xcore::xsystem::MEM_1024MB     ) : systemflags;
			systemflags = stack_512kb   ? SetSystemFlag(systemflags, xcore::xsystem::MEM_STACK_MASK, xcore::xsystem::MEM_512KB_STACK) : systemflags;

			return systemflags;
		}

		enum ESystemPC
		{
			MAX_EXE_PATH = 256,
			MAX_STACK_NAME_LEN = 1024
		};

		static char							m_szExePath[MAX_EXE_PATH];

		static bool							s_bGrowMemory						= true;
		static u32							s_uMemHeapTotalSize					= 0;
		static void*						s_pMemHeapBase						= NULL;
		static ELanguage					m_uLanguage							= LANGUAGE_ENGLISH;

		static LPTOP_LEVEL_EXCEPTION_FILTER	m_pPreviousFilter;

		static char*						GetExceptionString			( DWORD dwCode );
		static BOOL							GetLogicalAddress			( PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset );

		static BOOL CALLBACK				ReadProcessMemoryProc64		( HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead );

		static void							GenerateExceptionReport		( PEXCEPTION_POINTERS pExceptionInfo );
		static LONG WINAPI					UnhandledExceptionFilter	( PEXCEPTION_POINTERS pExceptionInfo );

		//==============================================================================
		//Executable info
		//==============================================================================

		static s32	codeSegmentSize;
		static s32	bssSegmentSize;
		static s32	dataSegmentSize;
		static s32	imageSize;
		static s32	mainThreadStackSize;

		//---------------------------------------------------------------------------------------------------------------------
		void InitializeExecutableInfo()
		{
			HMODULE hModule = 0;

			EnumProcessModules(GetCurrentProcess(),&hModule,sizeof(hModule),0);

			MEMORY_BASIC_INFORMATION mbi;

			if ( !VirtualQuery( hModule, &mbi, sizeof(mbi) ) )
			{
				return;
			}

			::GetModuleFileNameExA(GetCurrentProcess(), hModule, m_szExePath, sizeof(m_szExePath));

			DWORD hMod = (DWORD)mbi.AllocationBase;

			if (0 != hMod)
			{
				PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;
				PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

				PIMAGE_OPTIONAL_HEADER32 pOptionalHdr = &pNtHdr->OptionalHeader;

				codeSegmentSize		=	pOptionalHdr->SizeOfCode;
				bssSegmentSize		=	pOptionalHdr->SizeOfUninitializedData;
				dataSegmentSize		=	pOptionalHdr->SizeOfInitializedData;
				imageSize			=	pOptionalHdr->SizeOfImage;
				mainThreadStackSize	=	pOptionalHdr->SizeOfStackReserve;

			}

		}


		//---------------------------------------------------------------------------------------------------------------------
		s32	GetCodeSegmentSize()
		{
			return codeSegmentSize;
		}

		//---------------------------------------------------------------------------------------------------------------------

		s32	GetBssSegmentSize()
		{
			return bssSegmentSize;
		}

		//---------------------------------------------------------------------------------------------------------------------

		s32	GetDataSegmentSize()
		{
			return dataSegmentSize;
		}

		//---------------------------------------------------------------------------------------------------------------------

		s32	GetMainThreadStackSize()
		{
			return mainThreadStackSize;
		}

		//---------------------------------------------------------------------------------------------------------------------

		const char* GetExePath()
		{
			return m_szExePath;
		}








		//----------------
		// Private Methods
		//----------------

		#define CASE_EXCEPTION( x ) case EXCEPTION_##x: return __T(#x);

		char* GetExceptionString( DWORD dwCode )
		{
			switch ( dwCode )
			{
				CASE_EXCEPTION( ACCESS_VIOLATION )
				CASE_EXCEPTION( DATATYPE_MISALIGNMENT )
				CASE_EXCEPTION( BREAKPOINT )
				CASE_EXCEPTION( SINGLE_STEP )
				CASE_EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
				CASE_EXCEPTION( FLT_DENORMAL_OPERAND )
				CASE_EXCEPTION( FLT_DIVIDE_BY_ZERO )
				CASE_EXCEPTION( FLT_INEXACT_RESULT )
				CASE_EXCEPTION( FLT_INVALID_OPERATION )
				CASE_EXCEPTION( FLT_OVERFLOW )
				CASE_EXCEPTION( FLT_STACK_CHECK )
				CASE_EXCEPTION( FLT_UNDERFLOW )
				CASE_EXCEPTION( INT_DIVIDE_BY_ZERO )
				CASE_EXCEPTION( INT_OVERFLOW )
				CASE_EXCEPTION( PRIV_INSTRUCTION )
				CASE_EXCEPTION( IN_PAGE_ERROR )
				CASE_EXCEPTION( ILLEGAL_INSTRUCTION )
				CASE_EXCEPTION( NONCONTINUABLE_EXCEPTION )
				CASE_EXCEPTION( STACK_OVERFLOW )
				CASE_EXCEPTION( INVALID_DISPOSITION )
				CASE_EXCEPTION( GUARD_PAGE )
				CASE_EXCEPTION( INVALID_HANDLE )
			}

			// If not one of the "known" exceptions, try to get the string from NTDLL.DLL's message table.

			static TCHAR szBuffer[512] = { 0 };

			FormatMessage(  FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandle( __T("NTDLL.DLL") ), dwCode, 0, szBuffer, sizeof( szBuffer ), 0 );
			return szBuffer;
		}

		#undef CASE_EXCEPTION

		BOOL GetLogicalAddress( PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset )
		{
			MEMORY_BASIC_INFORMATION mbi;

			if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
			{
				return FALSE;
			}

			DWORD hMod = (DWORD)mbi.AllocationBase;

			if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
			{
				return FALSE;
			}

			// Point to the DOS header in memory
			PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

			// From the DOS header, find the NT (PE) header
			PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

			PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

			DWORD rva = (DWORD)addr - hMod; // RVA is offset from module load address

			// Iterate through the section table, looking for the one that encompasses
			// the linear address.
			for (   unsigned i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++, pSection++ )
			{
				DWORD sectionStart = pSection->VirtualAddress;
				DWORD sectionEnd = sectionStart + x_intu::max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

				// Is the address in this section???
				if ( (rva >= sectionStart) && (rva <= sectionEnd) )
				{
					// Yes, address is in the section.  Calculate section and offset,
					// and store in the "section" & "offset" params, which were
					// passed by reference.
					section = i+1;
					offset = rva - sectionStart;
					return TRUE;
				}
			}

			return FALSE;   // Should never get here!
		}

		BOOL CALLBACK ReadProcessMemoryProc64(HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead)
		{
			SIZE_T	xRead;
			BOOL	boOK = ReadProcessMemory(hProcess, (void*)lpBaseAddress, lpBuffer, nSize, &xRead);

			if(boOK)
			{
				*lpNumberOfBytesRead	= xRead;
			}

			return boOK;
		}

		void GenerateExceptionReport( PEXCEPTION_POINTERS pExceptionInfo )
		{
			// Start out with a banner
			xconsole::writeLine( "//=====================================================" );

			PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

			// First print information about the type of fault
			xconsole::writeLine("Exception: code %08X %s", x_va_list(x_va((u32)pExceptionRecord->ExceptionCode), x_va(GetExceptionString(pExceptionRecord->ExceptionCode))) );

			// Now print information about where the fault occured
			TCHAR szFaultingModule[MAX_PATH];
			DWORD section, offset;
			GetLogicalAddress(	pExceptionRecord->ExceptionAddress, szFaultingModule, sizeof( szFaultingModule ), section, offset );

			xconsole::writeLine( "Fault address:  %08X %02X:%08X %s", x_va_list(x_va((u32)pExceptionRecord->ExceptionAddress), x_va((u32)section), x_va((u32)offset), x_va(szFaultingModule)) );

			PCONTEXT pCtx = pExceptionInfo->ContextRecord;

			// Show the registers
#ifdef _M_IX86  // Intel Only!
			xconsole::writeLine( "\nRegisters:" );

			xconsole::writeLine( "EAX:%08X", x_va_list(x_va((u32)pCtx->Eax)) );
			xconsole::writeLine( "EBX:%08X", x_va_list(x_va((u32)pCtx->Ebx)) );
			xconsole::writeLine( "ECX:%08X", x_va_list(x_va((u32)pCtx->Ecx)) );
			xconsole::writeLine( "EDX:%08X", x_va_list(x_va((u32)pCtx->Edx)) );
			xconsole::writeLine( "ESI:%08X", x_va_list(x_va((u32)pCtx->Esi)) );
			xconsole::writeLine( "EDI:%08X", x_va_list(x_va((u32)pCtx->Edi)) );

			xconsole::writeLine( "CS:EIP:%04X:%08X", x_va_list(x_va((u32)pCtx->SegCs), x_va((u32)pCtx->Eip)) );
			xconsole::writeLine( "SS:ESP:%04X:%08X  EBP:%08X", x_va_list(x_va((u32)pCtx->SegSs), x_va((u32)pCtx->Esp), x_va((u32)pCtx->Ebp)) );
			xconsole::writeLine( "DS:%04X  ES:%04X  FS:%04X  GS:%04X", x_va_list(x_va((u32)pCtx->SegDs), x_va((u32)pCtx->SegEs), x_va((u32)pCtx->SegFs), x_va((u32)pCtx->SegGs)) );
			xconsole::writeLine( "Flags:%08X", x_va_list(x_va((u32)pCtx->EFlags)) );
#endif

			u32	uDepth;
			u64	uStack[256];
			CreateStackTrace(0, uStack, uDepth, 256, pCtx );
			DumpStackTrace(uStack, uDepth, 0);

			xconsole::writeLine();
		}

		LONG WINAPI UnhandledExceptionFilter( PEXCEPTION_POINTERS pExceptionInfo )
		{
			GenerateExceptionReport( pExceptionInfo );

			if ( m_pPreviousFilter )
			{
				return m_pPreviousFilter( pExceptionInfo );
			}
			else
			{
				return EXCEPTION_EXECUTE_HANDLER;
			}
		}

		//---------------
		// Public Methods
		//---------------
		//---------------------------------------------------------------------------------------------------------------------

		//==============================================================================
		// Memory 
		//==============================================================================

		static void*	AllocSystemMemory			( u32 uSize )
		{
			void* heap = HeapAlloc(GetProcessHeap(), 0, uSize);
			if(heap == NULL)
			{
				ASSERTS(false, "xcore::xsystem::AllocSystemMemory Error\n");
			}
			return heap;
		}

		//---------------------------------------------------------------------------------------------------------------------

		static void		FreeSystemMemory			( void* pData )
		{
			BOOL boOK = HeapFree(GetProcessHeap(), 0, pData);
			if(!boOK)
			{
				ASSERTS(false, "xcore::xsystem::FreeSystemMemory Error\n");
			}
		}

		static void*	sWin32AllocFunc(xsize_t size)
		{
			return ::_aligned_malloc(size, X_MEMALIGN);
		}
		static void		sWin32FreeFunc(void* ptr)
		{
			::_aligned_free(ptr);
		}

		void				Initialise				(const char* szExePath, ESystemFlags systemflags)
		{
			xconsole::writeLine("INFO xcore: " TARGET_FULL_DESCR_STR );

			x_strcpy(m_szExePath, MAX_EXE_PATH, szExePath);

			if(!IsDebuggerPresent())
			{
				// Install the unhandled exception filter function
				m_pPreviousFilter = SetUnhandledExceptionFilter(UnhandledExceptionFilter);
			}

			s_uMemHeapTotalSize = (systemflags&MEM_MASK) * 1024*1024;
			s_bGrowMemory = ((systemflags&MEM_SYSTEM)==MEM_SYSTEM);

			//xmem_main_heap::_initialize();
			u32 systemMemorySize = s_uMemHeapTotalSize - 128;
			s_pMemHeapBase = AllocSystemMemory(systemMemorySize);
// 			xmem_main_heap::_manage(s_pMemHeapBase, systemMemorySize);
// 			if (s_bGrowMemory)
// 				xmem_main_heap::_set_sys_calls(sWin32AllocFunc, sWin32FreeFunc);

			//------------------------
			// Dvd/Host mapping
			//------------------------
			if ((systemflags&DATA_FROM_MASK) == DATA_FROM_DVD)
			{
// 				xfilesystem::xalias dataFromDvd("data", "curdir");
// 				xfilesystem::AddAlias(dataFromDvd);
			}
			else if ((systemflags&DATA_FROM_MASK) == DATA_FROM_HOST)
			{
// 				xfilesystem::xalias dataFromHost("data", "curdir");
// 				xfilesystem::AddAlias(dataFromHost);
			}

			//InitializeCpuInfo();
			InitializeExecutableInfo();

		}

		//---------------------------------------------------------------------------------------------------------------------

		void				Update					( void )
		{

		}

		//---------------------------------------------------------------------------------------------------------------------

		void				Shutdown				(void)
		{
// 			xmem_main_heap::_destroy();
// 			xmem_main_heap::_set_sys_calls(NULL, NULL);
			FreeSystemMemory(s_pMemHeapBase);

			if(!IsDebuggerPresent())
			{
				// Uninstall the unhandled exception filter function
				SetUnhandledExceptionFilter( m_pPreviousFilter );
			}
		}

		//---------------------------------------------------------------------------------------------------------------------
		
		void				FatalError				( void )
		{
			exit(-1);
		}

		//---------------------------------------------------------------------------------------------------------------------

		EConsoleType		GetConsoleType			( )
		{
			return CONSOLE_DEVKIT;
		}

		//---------------------------------------------------------------------------------------------------------------------

		EMediaType			GetMediaType			( )
		{
			return MEDIA_HOST;
		}

		//---------------------------------------------------------------------------------------------------------------------

		ELanguage			GetLanguage					( void )
		{
			return m_uLanguage;//LANGUAGE_ENGLISH;
		}

		//---------------------------------------------------------------------------------------------------------------------

		void				SetLanguage					( ELanguage language)
		{
			m_uLanguage = language;
		}

		//---------------------------------------------------------------------------------------------------------------------

		u32					GetMaxSystemMemory			( void )
		{
			return s_uMemHeapTotalSize;
		}

		//---------------------------------------------------------------------------------------------------------------------

		u32					GetCurrentSystemMemory		( void )
		{
			return s_uMemHeapTotalSize;
		}

		//---------------------------------------------------------------------------------------------------------------------

		void*				GetSystemMemoryBase			( void )
		{
			return s_pMemHeapBase;
		}

		//---------------------------------------------------------------------------------------------------------------------

		void				Sleep						( u64 uMicroSec )
		{
			SleepEx((u32)(uMicroSec/(u64)1000), FALSE);
		}

		//---------------------------------------------------------------------------------------------------------------------

		void				CreateStackTrace			( u32 uStartIndex, u64* pTrace, u32& ruDepth, u32 uMaxDepth, void* pContext )
		{
			static CONTEXT xContext;
			if (pContext == NULL)
			{
				memset(&xContext, 0, sizeof(CONTEXT)); 

				xContext.ContextFlags = CONTEXT_FULL;
				__asm    call x
				__asm x: pop eax
				__asm    mov xContext.Eip, eax
				__asm    mov xContext.Ebp, ebp
				__asm    mov xContext.Esp, esp

				pContext	= &xContext;
			}

			STACKFRAME64	StackFrame;

			memset(&StackFrame, 0, sizeof(StackFrame));

			ADDRESS64	AddrPC		= { xContext.Eip, 0, AddrModeFlat };
			ADDRESS64	AddrFrame	= { xContext.Ebp, 0, AddrModeFlat };
			ADDRESS64	AddrStack	= { xContext.Esp, 0, AddrModeFlat };

			StackFrame.AddrPC		= AddrPC;
			StackFrame.AddrFrame	= AddrFrame;
			StackFrame.AddrStack	= AddrStack;

			u32	uDepth			= 0;
			ruDepth				= 0;

			while(1)
			{
				BOOL boOK = StackWalk64(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &StackFrame, pContext, ReadProcessMemoryProc64, SymFunctionTableAccess64, SymGetModuleBase64, NULL);

				if(!boOK || StackFrame.AddrPC.Offset == 0)
				{
					break;
				}
				else
				{
					if(uDepth >= uStartIndex)
					{
						pTrace[ruDepth]	= StackFrame.AddrPC.Offset;
						ruDepth++;

						if(ruDepth >= uMaxDepth)
						{
							break;
						}
					}

					uDepth++;
				}
			}
		}


		void				DumpStackTrace				( u64* pTrace, u32 uDepth, const char* sExePath )
		{
			BOOL boOK = SymInitialize(GetCurrentProcess(), NULL, TRUE);

			for(u32 uI = 0; uI < uDepth; uI++)
			{
				IMAGEHLP_SYMBOL64*	pSym = (IMAGEHLP_SYMBOL64 *) malloc(sizeof(IMAGEHLP_SYMBOL64) + MAX_STACK_NAME_LEN);
				IMAGEHLP_LINE64		xLine;

				memset(&xLine, 0, sizeof(xLine));
				xLine.SizeOfStruct = sizeof(xLine);

				memset(pSym, 0, sizeof(IMAGEHLP_SYMBOL64) + MAX_STACK_NAME_LEN);
				pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
				pSym->MaxNameLength = MAX_STACK_NAME_LEN;

				DWORD64	dwDisplacement = 0;
				boOK = SymGetSymFromAddr64(GetCurrentProcess(), pTrace[uI], &dwDisplacement, pSym);

				char	szName[MAX_STACK_NAME_LEN];
				char	szFullName[MAX_STACK_NAME_LEN];

				x_strcpy(szName, MAX_STACK_NAME_LEN, "");
				x_strcpy(szFullName, MAX_STACK_NAME_LEN, "");

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

				boOK = SymGetLineFromAddr64(GetCurrentProcess(), pTrace[uI], (DWORD*)&dwDisplacement, &xLine);

				if(boOK)
				{
					xconsole::writeLine("%s(%d) : %s (%s)", x_va_list(x_va(xLine.FileName), x_va((u32)xLine.LineNumber), x_va(szName), x_va(szFullName)));
				}
			}

			SymCleanup(GetCurrentProcess());
		}



		//------------
		// End of file
		//------------
	};

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

#endif // TARGET_PC
