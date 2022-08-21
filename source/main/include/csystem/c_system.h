#ifndef __CSYSTEM_H__
#define __CSYSTEM_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

namespace ncore
{
	class alloc_t;

	namespace nsystem
	{
		struct ctxt_t;

		enum ELanguage
		{
			LANGUAGE_INVALID		= -1,
			LANGUAGE_ENGLISH		= 0,
			LANGUAGE_FRENCH			= 1,
			LANGUAGE_ITALIAN		= 2,
			LANGUAGE_GERMAN			= 3,
			LANGUAGE_SPANISH		= 4,
			LANGUAGE_GREEK			= 5,
			LANGUAGE_ENGLISH_US		= 6,
			LANGUAGE_FRENCH_US		= 7,
			LANGUAGE_PORTUGUESE		= 8,
			LANGUAGE_BRAZILIAN		= 9,			///< Brazilian Portuguese
			LANGUAGE_JAPANESE		= 10,
			LANGUAGE_KOREAN         = 11,			///< Korean
			LANGUAGE_RUSSIAN        = 12,			///< Russian
			LANGUAGE_DUTCH			= 13,
			LANGUAGE_CHINESE_T		= 14,
			LANGUAGE_CHINESE_S		= 15,
			LANGUAGE_FINNISH		= 16,
			LANGUAGE_SWEDISH		= 17,
			LANGUAGE_DANISH			= 18,
			LANGUAGE_NORWEGIAN		= 19,
			LANGUAGE_POLISH			= 20,

			LANGUAGE_COUNT,

			LANGUAGE_DEFAULT		= LANGUAGE_ENGLISH,
			LANGUAGE_MAIN			= LANGUAGE_DEFAULT
		};
		extern const char*	gToString(ELanguage);

		enum EConsoleType
		{
			CONSOLE_DESKTOP,
			CONSOLE_DEVKIT,
			CONSOLE_TESTKIT,
			CONSOLE_RETAIL,
		};

		enum EMediaType
		{
			MEDIA_LOCAL   = 0x01,
			MEDIA_NETWORK = 0x02,
			MEDIA_CD = 0x04,
			MEDIA_DVD = 0x08,
			MEDIA_BLURAY = 0x10,
			MEDIA_SD = 0x20, 
			MEDIA_HDD = 0x40,
			MEDIA_SSD = 0x80
		};

		enum EDataSource
		{
			DATA_FROM_HOST = 0x00,
			DATA_FROM_DVD = 0x10,
			DATA_FROM_NETWORK = 0x20,
			DATA_FROM_HARDDISK = 0x40,
			DATA_FROM_SSD = 0x80,
			DATA_FROM_MASK = 0xFF,
		};

		enum EEnvMode
		{
			ENVIRONMENT_OPEN = 0,
			ENVIRONMENT_DEBUG = 1,
			ENVIRONMENT_DEVELOPMENT = 2,
			ENVIRONMENT_QA = 3,
			ENVIRONMENT_RETAIL = 4,
		};

		struct MAC_t
		{
			u8	mID[6];
		};

		void					init					(alloc_t*, ctxt_t*& );
		void 					update					(ctxt_t* );
		void					shutdown				(ctxt_t* );

		void					printCpuInfo			(ctxt_t* );
		void					printMemoryInfo			(ctxt_t* );

		EConsoleType			getConsoleType			(ctxt_t*);
		EMediaType				getMediaType			(ctxt_t*);
		EDataSource				getDataSource			(ctxt_t*);
		EEnvMode				getEnvMode				(ctxt_t*);

		MAC_t					getMAC					(ctxt_t*);

		ELanguage				getLanguage				(ctxt_t*);
		void					setLanguage				(ctxt_t*, ELanguage language );

		void					getNickname				(ctxt_t*, char* szBuffer, u32 szBufferMaxLen );
		bool					isCircleButtonBack		(ctxt_t*);

		void					setAppTitle				(ctxt_t*, const char* title );
		const char*				getAppTitle				(ctxt_t* );
		const char*				getExePath				(ctxt_t* );

		bool					hasVirtualMemory		(ctxt_t* );

		s32						getNumCores				(ctxt_t*);
		s32						getNumHwThreadsPerCore	(ctxt_t*);
		u64						getCoreClockFrequency	(ctxt_t*);

		const char*				getPlatformName			(ctxt_t*);		// Name of platform
		const char*				getBuildConfigName		(ctxt_t*);		// Debug, Release, Final
		const char*				getBuildModeName		(ctxt_t*);		// Dev, Client, Retail
		const char*				getConsoleTypeName		(ctxt_t*);		// 
		const char*				getMediaTypeName		(ctxt_t*);		// 
		const char*				getLanguageName			(ctxt_t*);

		u64						getCodeSegmentSize		(ctxt_t*);
		u64						getBssSegmentSize		(ctxt_t*);
		u64						getDataSegmentSize		(ctxt_t*);
		u64						getMainThreadStackSize	(ctxt_t*);
	};
};

#if defined TARGET_PC && defined PLATFORM_32BIT
	#include "csystem/private/c_system_win32.h"
#elif defined TARGET_PC && defined PLATFORM_64BIT
	#include "csystem/private/c_system_win64.h"
#elif defined TARGET_OSX
	#include "csystem/private/c_system_osx.h"
#else
	#error "Current platform is not supported!" 
#endif

#endif	// __CSYSTEM_H__