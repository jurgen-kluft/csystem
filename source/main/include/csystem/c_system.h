#ifndef __CSYSTEM_H__
#define __CSYSTEM_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

namespace ncore
{
	class x_iallocator;

	namespace xsystem
	{
		struct xctxt;

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
			xbyte	mID[6];
		};

		void					init					(x_iallocator*, xctxt*& );
		void 					update					(xctxt* );
		void					shutdown				(xctxt* );

		void					printCpuInfo			(xctxt* );
		void					printMemoryInfo			(xctxt* );

		EConsoleType			getConsoleType			(xctxt*);
		EMediaType				getMediaType			(xctxt*);
		EDataSource				getDataSource			(xctxt*);
		EEnvMode				getEnvMode				(xctxt*);

		MAC_t					getMAC					(xctxt*);

		ELanguage				getLanguage				(xctxt*);
		void					setLanguage				(xctxt*, ELanguage language );

		void					getNickname				(xctxt*, char* szBuffer, u32 szBufferMaxLen );
		bool					isCircleButtonBack		(xctxt*);

		void					setAppTitle				(xctxt*, const char* title );
		const char*				getAppTitle				(xctxt* );
		const char*				getExePath				(xctxt* );

		bool					hasVirtualMemory		(xctxt* );

		s32						getNumCores				(xctxt*);
		s32						getNumHwThreadsPerCore	(xctxt*);
		u64						getCoreClockFrequency	(xctxt*);

		const char*				getPlatformName			(xctxt*);		// Name of platform
		const char*				getBuildConfigName		(xctxt*);		// Debug, Release, Final
		const char*				getBuildModeName		(xctxt*);		// Dev, Client, Retail
		const char*				getConsoleTypeName		(xctxt*);		// 
		const char*				getMediaTypeName		(xctxt*);		// 
		const char*				getLanguageName			(xctxt*);

		u64						getCodeSegmentSize		(xctxt*);
		u64						getBssSegmentSize		(xctxt*);
		u64						getDataSegmentSize		(xctxt*);
		u64						getMainThreadStackSize	(xctxt*);
	};

};

//==============================================================================
// Platforms
//==============================================================================

//==============================================================================
// Win32
//==============================================================================
#if defined TARGET_PC && defined PLATFORM_32BIT
	#include "xsystem/private/x_system_win32.h"

//==============================================================================
// Win64
//==============================================================================
#elif defined TARGET_PC && defined PLATFORM_64BIT
	#include "xsystem/private/x_system_win64.h"

//==============================================================================
// OSX
//==============================================================================
#elif defined TARGET_OSX
	#include "xsystem/private/x_system_osx.h"

#else
	#error "Current platform is not supported!" 
#endif


#endif	// __CSYSTEM_H__