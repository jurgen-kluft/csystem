// x_system_win32.h - System
#ifndef __XCORE_SYSTEM_WIN32_H__
#define __XCORE_SYSTEM_WIN32_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xsystem/x_system.h"

namespace ncore
{
	class x_iallocator;

	namespace xsystem
	{
		void					parseArgs				(xctxt* ctx, int argc, const char** argv);

		u64						getTotalMemorySize		(xctxt* ctx);
		u64						getCurrentSystemMemory	(xctxt* c);
		u64						getDefaultStackSize		(xctxt* ctx);

		void					createStackTrace		(xctxt* ctx, u32 uStartIndex, u64* pTrace, u32& ruDepth, u32 uMaxDepth, void* pContext = NULL );
		void					dumpStackTrace			(xctxt* ctx, u64* pTrace, u32 uDepth, const char* szExePath );
	};

};

#endif	// __XCORE_SYSTEM_WIN32_H__