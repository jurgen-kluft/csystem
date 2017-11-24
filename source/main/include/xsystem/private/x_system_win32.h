// x_system_win32.h - System
#ifndef __XCORE_SYSTEM_WIN32_H__
#define __XCORE_SYSTEM_WIN32_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
// xCore namespace
//==============================================================================

namespace xcore
{
	class xstring_const;

	namespace xsystem
	{
		enum ESystemFlags
		{
			MEM_16MB			= 0x00000010,
			MEM_32MB			= 0x00000020,
			MEM_64MB			= 0x00000040,
			MEM_128MB			= 0x00000080,
			MEM_256MB			= 0x00000100,
			MEM_512MB			= 0x00000200,
			MEM_1024MB			= 0x00000400,
			MEM_MASK			= 0x000007F0,
			MEM_SYSTEM			= 0x00001000,

			MEM_256KB_STACK		= 0x00000000,
			MEM_512KB_STACK		= 0x00010000,
			MEM_STACK_MASK		= 0x000F0000,

			DATA_FROM_HOST		= 0x00000000,
			DATA_FROM_DVD		= 0x10000000,
			DATA_FROM_MASK		= 0xF0000000,

			MODE_DEFAULT		= 0,
			MODE_DEV			= MEM_SYSTEM | MEM_32MB | MEM_512KB_STACK | DATA_FROM_HOST,
			MODE_MASK			= MEM_MASK   | MEM_STACK_MASK  | DATA_FROM_MASK,
		};

		extern ESystemFlags		ParseSystemFlags		( ESystemFlags systemflags, int argc, const char** argv );
		inline ESystemFlags		SetSystemFlag			( ESystemFlags systemflags, ESystemFlags mask, ESystemFlags flag )		{ return (xcore::xsystem::ESystemFlags)((systemflags&mask)|flag); }

		extern void 			Initialise				( const char* szExePath, ESystemFlags flags );

		extern void				CreateStackTrace		( u32 uStartIndex, u64* pTrace, u32& ruDepth, u32 uMaxDepth, void* pContext = NULL );
		extern void				DumpStackTrace			( u64* pTrace, u32 uDepth, const char* szExePath );
	};

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

#endif	// __XCORE_SYSTEM_WIN32_H__