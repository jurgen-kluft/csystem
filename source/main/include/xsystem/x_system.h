// x_system.h - System
#ifndef __XSYSTEM_H__
#define __XSYSTEM_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_types.h"

//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	namespace xsystem
	{


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
			CONSOLE_DEVKIT,
			CONSOLE_TESTKIT,
			CONSOLE_RETAIL,
		};

		enum EMediaType
		{
			MEDIA_HOST,
			MEDIA_DISC,
			MEDIA_MS,
			MEDIA_HDD,
		};

		struct nodeid_t
		{
			xbyte	mID[6];
		};

		void					init					( );
		void 					update					( );
		void					shutdown				( );

		void					printPlatformName		( );
		void					printConsoleType		( );
		void					printMediaType			( );
		void					printCurrentLanguage	( );
		void					printExecutableInfo		( );
		void					printCpuInfo			( );
		void					printMemoryInfo			( );

		EConsoleType			getConsoleType			( );
		EMediaType				getMediaType			( );
		nodeid_t				getNodeId				( );

		ELanguage				getLanguage				( );
		const char*				getLanguageStr			( );
		void					setLanguage				( ELanguage language );

		void					getNickname				( char* szBuffer, u32 szBufferMaxLen );
		bool					isCircleButtonBack		( );

		void					setGameTitle			( const char* title );
		const char*				getGameTitle			( );
		const char*				getExePath				( );

		s32						getNumCores				( );
		s32						getNumHwThreadsPerCore	( );
		u64						getCoreClockFrequency	( );

		const char*				getPlatformStr			( );		///< Name of platform
		const char*				getBuildConfigStr		( );		///< Debug, Release, Final
		const char*				getBuildModeStr			( );		///< Dev, Client, Retail
		s32						getCodeSegmentSize		( );
		s32						getBssSegmentSize		( );
		s32						getDataSegmentSize		( );
		s32						getMainThreadStackSize	( );
	};

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

//==============================================================================
// Platforms
//==============================================================================

//==============================================================================
// Win32
//==============================================================================
#if defined TARGET_PC && defined X_TARGET_32BIT
	#include "xsystem/private/x_system_win32.h"

//==============================================================================
// Win64
//==============================================================================
#elif defined TARGET_PC && defined X_TARGET_64BIT
	#include "xsystem/private/x_system_win64.h"

//==============================================================================
// OSX
//==============================================================================
#elif defined TARGET_OSX
	#include "xsystem/private/x_system_osx.h"

#else
	#error "Current platform is not supported!" 
#endif


#endif	// __XSYSTEM_H__