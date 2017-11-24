#include "xbase/x_target.h"

#ifdef TARGET_PC
#include "xbase/x_debug.h"
#include "xbase/x_console.h"
#include "xbase/x_string_ascii.h"

//==============================================================================
// INCLUDES
//==============================================================================
#include "xsystem/x_system.h"

#include <Windows.h>
#include <Iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	namespace xsystem
	{
		static ELanguage		sLanguage = LANGUAGE_DEFAULT;
		static char				sGameTitle[128];
		static const char*		sExePath = NULL;

		void					init					( )
		{
		}

		void 					update					( )
		{
		}

		void					shutdown				( )
		{
		}

		void					printPlatformName		( )
		{
			xconsole::writeLine("Win32");
		}

		void					printConsoleType		( )
		{
			xconsole::writeLine("Retail");
		}

		void					printMediaType			( )
		{
			xconsole::writeLine("HD");
		}

		void					printCurrentLanguage	( )
		{
			xconsole::writeLine(gToString(sLanguage));
		}

		void					printExecutableInfo		( )
		{
		}

		void					printCpuInfo			( )
		{
		}

		void					printMemoryInfo			( )
		{
		}

		EConsoleType			getConsoleType			( )
		{
			return CONSOLE_RETAIL;
		}

		EMediaType				getMediaType			( )
		{
			return MEDIA_HDD;
		}

		// Return the MAC address of this PC
		nodeid_t getNodeId()
		{
			nodeid_t id;

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
				for (s32 i=0; i<6; ++i)
					id.mID[i] = pAdapterInfo->Address[i];

				pAdapterInfo = pAdapterInfo->Next;		// Progress through linked list
			} while (pAdapterInfo!=NULL);				// Terminate if last adapter

			return id;
		}


		ELanguage				getLanguage				( )
		{
			return sLanguage;
		}

		const char*				getLanguageStr			( )
		{
			return gToString(sLanguage);
		}

		void					setLanguage				( ELanguage language )
		{
			sLanguage = language;
		}

		void					getNickname				( char* szBuffer, u32 szBufferMaxLen )
		{
			x_strcpy(szBuffer, szBufferMaxLen, "Desktop");
		}

		bool					isCircleButtonBack		( )
		{
			return false;
		}

		void					setGameTitle			( const char* title )
		{
			x_strcpy(sGameTitle, sizeof(sGameTitle)-1, title);
		}

		const char*				getGameTitle			( )
		{
			return sGameTitle;
		}

		const char*				getExePath				( )
		{
			return sExePath;
		}

		s32						getNumCores				( )
		{
			return 1;
		}

		s32						getNumHwThreadsPerCore	( )
		{
			return 1;
		}

		u64						getCoreClockFrequency	( )
		{
			return X_CONSTANT_U64(2 * 1024 * 1024 * 1024);
		}

		const char*				getPlatformStr			( )
		{
			return "Win32";
		}

		const char*				getBuildConfigStr		( )
		{
#if defined(TARGET_DEBUG)
			return "Debug";
#elif defined(TARGET_RELEASE)
			return "Release";
#elif defined(TARGET_FINAL  )
			return "Final";
#endif
		}

		const char*				getBuildModeStr			( )
		{
#if defined(TARGET_DEVKIT)
			return "Dev";
#elif defined(TARGET_CLIENT)
			return "Client";
#elif defined(TARGET_RETAIL)
			return "Retail";
#endif
		}

		s32						getCodeSegmentSize		( )
		{
			return 0;
		}

		s32						getBssSegmentSize		( )
		{
			return 0;
		}

		s32						getDataSegmentSize		( )
		{
			return 0;
		}

		s32						getMainThreadStackSize	( )
		{
			return 0;
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
