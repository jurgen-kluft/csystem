#include "xbase/x_base.h"
#include "xbase/x_allocator.h"

#if defined(TARGET_PC)

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_debug.h"
#include "xbase/x_string_ascii.h"
#include "xbase/x_memory_std.h"

#include "xsystem/x_system.h"

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

#include <Iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "DbgHelp.lib")

#include "xbase/x_integer.h"


//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	namespace xsystem
	{

		enum ESystemPC
		{
			MAX_EXE_PATH = 256,
			MAX_STACK_NAME_LEN = 1024
		};

		struct xctxt
		{
			xctxt(x_iallocator* a)
				: m_allocator(a)
				, m_uMemHeapTotalSize(16ULL * 1024 * 1024 * 1024)
				, m_uLanguage(LANGUAGE_ENGLISH)
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

			x_iallocator*					m_allocator;
			char							m_szAppTitle[128];
			char							m_szExePath[MAX_EXE_PATH];
			char							m_szNickName[128];
			u64								m_uMemHeapTotalSize;
			ELanguage						m_uLanguage;
			LPTOP_LEVEL_EXCEPTION_FILTER	m_pPreviousFilter;

			EConsoleType					m_console_type;
			EMediaType						m_media_type;

			s32								m_codeSegmentSize;
			s32								m_bssSegmentSize;
			s32								m_dataSegmentSize;
			s32								m_imageSize;
			u64								m_mainThreadStackSize;
			
			CONTEXT							m_xContext;
			CONTEXT*						m_pContext;

			XCORE_CLASS_PLACEMENT_NEW_DELETE
		};
		static xctxt*						sCtxt = NULL;

		static void							InitializeExecutableInfo(xctxt* c);
		static void							GenerateExceptionReport(xctxt* c, PEXCEPTION_POINTERS pExceptionInfo);
		static LONG WINAPI					UnhandledExceptionFilter(xctxt* c, PEXCEPTION_POINTERS pExceptionInfo);

		static char*						GetExceptionString			( DWORD dwCode );
		static BOOL							GetLogicalAddress			( PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset );

		static BOOL CALLBACK				ReadProcessMemoryProc64		( HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead );


		//==============================================================================
		//Executable info
		//==============================================================================

		//---------------------------------------------------------------------------------------------------------------------
		void InitializeExecutableInfo(xctxt* c)
		{
			HMODULE hModule = 0;

			EnumProcessModules(GetCurrentProcess(),&hModule,sizeof(hModule),0);

			MEMORY_BASIC_INFORMATION mbi;

			if ( !VirtualQuery( hModule, &mbi, sizeof(mbi) ) )
			{
				return;
			}

			::GetModuleFileNameExA(GetCurrentProcess(), hModule, c->m_szExePath, sizeof(c->m_szExePath));

			PVOID hMod = (PVOID)mbi.AllocationBase;

			if (0 != hMod)
			{
				PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;
				PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)((u8*)hMod + pDosHdr->e_lfanew);

#ifdef TARGET_32BIT
				PIMAGE_OPTIONAL_HEADER32 pOptionalHdr = &pNtHdr->OptionalHeader;
#else
				PIMAGE_OPTIONAL_HEADER64 pOptionalHdr = &pNtHdr->OptionalHeader;
#endif

				c->m_codeSegmentSize		=	pOptionalHdr->SizeOfCode;
				c->m_bssSegmentSize		=	pOptionalHdr->SizeOfUninitializedData;
				c->m_dataSegmentSize		=	pOptionalHdr->SizeOfInitializedData;
				c->m_imageSize			=	pOptionalHdr->SizeOfImage;
				c->m_mainThreadStackSize	=	pOptionalHdr->SizeOfStackReserve;
			}
		}


		//---------------------------------------------------------------------------------------------------------------------
		u64	getCodeSegmentSize(xctxt* c)
		{
			return c->m_codeSegmentSize;
		}

		//---------------------------------------------------------------------------------------------------------------------

		u64	getBssSegmentSize(xctxt* c)
		{
			return c->m_bssSegmentSize;
		}

		//---------------------------------------------------------------------------------------------------------------------

		u64	getDataSegmentSize(xctxt* c)
		{
			return c->m_dataSegmentSize;
		}

		//---------------------------------------------------------------------------------------------------------------------

		u64	getMainThreadStackSize(xctxt* c)
		{
			return c->m_mainThreadStackSize;
		}

		//---------------------------------------------------------------------------------------------------------------------

		const char*				getExePath(xctxt* c)
		{
			return c->m_szExePath;
		}

		const char*				getNickname(xctxt* c)
		{
			return c->m_szNickName;
		}

		void					setGameTitle(xctxt* c, const char* title)
		{
			ascii::copy(c->m_szAppTitle, c->m_szAppTitle + sizeof(c->m_szAppTitle) - 1, title, NULL);
		}

		const char*				getGameTitle(xctxt* c)
		{
			return c->m_szAppTitle;
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

		BOOL GetLogicalAddress( PVOID addr, PTSTR szModule, DWORD len, DWORD& section, u64& offset )
		{
			MEMORY_BASIC_INFORMATION mbi;

			if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
			{
				return FALSE;
			}

			PVOID hMod = (PVOID)mbi.AllocationBase;

			if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
			{
				return FALSE;
			}

			// Point to the DOS header in memory
			PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

			// From the DOS header, find the NT (PE) header
			PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)((u8*)hMod + pDosHdr->e_lfanew);

			PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

			u64 rva = ((u8*)addr - (u8*)hMod); // RVA is offset from module load address

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
				*lpNumberOfBytesRead	= (DWORD)xRead;
			}

			return boOK;
		}

		void GenerateExceptionReport( PEXCEPTION_POINTERS pExceptionInfo )
		{
#if 0
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
#endif
		}


		LONG WINAPI UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo )
		{
			GenerateExceptionReport( pExceptionInfo );

			if (sCtxt->m_pPreviousFilter )
			{
				return sCtxt->m_pPreviousFilter( pExceptionInfo );
			}
			else
			{
				return EXCEPTION_EXECUTE_HANDLER;
			}
		}

		void				init(x_iallocator* a, xctxt*& c)
		{
			// Allocator context
			void * ctxt_mem = a->allocate(sizeof(xctxt), sizeof(void*));
			c = new (ctxt_mem) xctxt(a);
			sCtxt = c;

			if(!IsDebuggerPresent())
			{
				// Install the unhandled exception filter function
				c->m_pPreviousFilter = SetUnhandledExceptionFilter(UnhandledExceptionFilter);
			}

			//------------------------
			// Dvd/Host mapping
			//------------------------
#if 0
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
#endif

			//InitializeCpuInfo();
			InitializeExecutableInfo(c);
		}

		//---------------------------------------------------------------------------------------------------------------------

		void				update					(xctxt* c)
		{

		}

		//---------------------------------------------------------------------------------------------------------------------

		void				shutdown				(xctxt* c)
		{
			if(!IsDebuggerPresent())
			{
				// Uninstall the unhandled exception filter function
				SetUnhandledExceptionFilter( c->m_pPreviousFilter );
			}

			c->m_allocator->deallocate(c);
		}

		//---------------------------------------------------------------------------------------------------------------------
		
		void				fatalError				( void )
		{
			exit(-1);
		}

		// Return the MAC address of this PC
		MAC_t					getMAC(xctxt*)
		{
			MAC_t id;

			IP_ADAPTER_INFO AdapterInfo[16];			// Allocate information for up to 16 NICs
			DWORD dwBufLen = sizeof(AdapterInfo);		// Save the memory size of buffer

			DWORD dwStatus = GetAdaptersInfo(			// Call GetAdapterInfo
				AdapterInfo,							// [out] buffer to receive data
				&dwBufLen);								// [in] size of receive data buffer
			ASSERT(dwStatus == ERROR_SUCCESS);			// Verify return value is valid, no buffer overflow

			PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
			do
			{
				pAdapterInfo->Address;					// The MAC address

														// Take the first 6 bytes
				for (s32 i = 0; i<6; ++i)
					id.mID[i] = pAdapterInfo->Address[i];

				pAdapterInfo = pAdapterInfo->Next;		// Progress through linked list
			} while (pAdapterInfo != NULL);				// Terminate if last adapter

			return id;
		}

		bool					isCircleButtonBack()
		{
			return false;
		}

		s32						getNumCores(xctxt* c)
		{
			return 1;
		}

		s32						getNumHwThreadsPerCore(xctxt* c)
		{
			return 1;
		}

		u64						getCoreClockFrequency(xctxt* c)
		{
			return X_CONSTANT_U64(2 * 1024 * 1024 * 1024);
		}

		const char*				getPlatformName(xctxt* c)
		{
			return "Win32";
		}

		const char*				getBuildConfigName(xctxt* c)
		{
#if defined(TARGET_DEBUG)
			return "Debug";
#elif defined(TARGET_RELEASE)
			return "Release";
#elif defined(TARGET_FINAL  )
			return "Final";
#endif
		}

		const char*				getBuildModeName(xctxt* c)
		{
#if defined(TARGET_DEBUG)
			return "Debug";
#elif defined(TARGET_DEV)
			return "Dev";
#elif defined(TARGET_CLIENT)
			return "Client";
#elif defined(TARGET_RETAIL)
			return "Retail";
#endif
		}


		//---------------------------------------------------------------------------------------------------------------------

		EConsoleType		getConsoleType			(xctxt* c)
		{
			return c->m_console_type;
		}

		//---------------------------------------------------------------------------------------------------------------------

		EMediaType			getMediaType			(xctxt* c)
		{
			return c->m_media_type;
		}

		//---------------------------------------------------------------------------------------------------------------------

		ELanguage			getLanguage					(xctxt* c)
		{
			return c->m_uLanguage;//LANGUAGE_ENGLISH;
		}

		//---------------------------------------------------------------------------------------------------------------------

		void				setLanguage					(xctxt* c, ELanguage language)
		{
			c->m_uLanguage = language;
		}

		const char*			getLanguageStr(xctxt* c)
		{
			return gToString(c->m_uLanguage);
		}


		//---------------------------------------------------------------------------------------------------------------------

		u64					getTotalMemorySize			(xctxt* c)
		{
			return c->m_uMemHeapTotalSize;
		}

		//---------------------------------------------------------------------------------------------------------------------

		u64					getCurrentSystemMemory		(xctxt* c)
		{
			return c->m_uMemHeapTotalSize;
		}

		//---------------------------------------------------------------------------------------------------------------------

		void				sleep						( u64 uMicroSec )
		{
			SleepEx((u32)(uMicroSec/(u64)1000), FALSE);
		}

		//---------------------------------------------------------------------------------------------------------------------

		void				createStackTrace			(xctxt* c, u32 uStartIndex, u64* pTrace, u32& ruDepth, u32 uMaxDepth)
		{
			RtlCaptureContext(&c->m_xContext);

			STACKFRAME64	StackFrame;

			memset(&StackFrame, 0, sizeof(StackFrame));
			CONTEXT& xContext = c->m_xContext;

#if defined(TARGET_32BIT)
			ADDRESS64	AddrPC		= { xContext.Eip, 0, AddrModeFlat };
			ADDRESS64	AddrFrame	= { xContext.Esp, 0, AddrModeFlat };
			ADDRESS64	AddrStack	= { xContext.Esp, 0, AddrModeFlat };
#else
			ADDRESS64	AddrPC		= { xContext.Rip, 0, AddrModeFlat };
			ADDRESS64	AddrFrame	= { xContext.Rsp, 0, AddrModeFlat };
			ADDRESS64	AddrStack	= { xContext.Rsp, 0, AddrModeFlat };
#endif
			StackFrame.AddrPC		= AddrPC;
			StackFrame.AddrFrame	= AddrFrame;
			StackFrame.AddrStack	= AddrStack;

			u32	uDepth			= 0;
			ruDepth				= 0;

			while(1)
			{
				BOOL boOK = StackWalk64(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &StackFrame, c->m_pContext, ReadProcessMemoryProc64, SymFunctionTableAccess64, SymGetModuleBase64, NULL);

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


		void				dumpStackTrace				(xctxt* ctx, u64* pTrace, u32 uDepth, const char* sExePath )
		{
#if 0
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

				boOK = SymGetLineFromAddr64(GetCurrentProcess(), pTrace[uI], (DWORD*)&dwDisplacement, &xLine);

				if(boOK)
				{
					xconsole::writeLine("%s(%d) : %s (%s)", x_va_list(x_va(xLine.FileName), x_va((u32)xLine.LineNumber), x_va(szName), x_va(szFullName)));
				}
			}

			SymCleanup(GetCurrentProcess());
#endif
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
