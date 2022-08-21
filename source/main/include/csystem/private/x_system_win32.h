#ifndef __CCORE_SYSTEM_WIN32_H__
#define __CCORE_SYSTEM_WIN32_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "csystem/c_system.h"

namespace ncore
{
	namespace nsystem
	{
		void					parseArgs				(ctxt_t* ctx, int argc, const char** argv);

		u64						getTotalMemorySize		(ctxt_t* ctx);
		u64						getCurrentSystemMemory	(ctxt_t* c);
		u64						getDefaultStackSize		(ctxt_t* ctx);

		void					createStackTrace		(ctxt_t* ctx, u32 uStartIndex, u64* pTrace, u32& ruDepth, u32 uMaxDepth, void* pContext = NULL );
		void					dumpStackTrace			(ctxt_t* ctx, u64* pTrace, u32 uDepth, const char* szExePath );
	};

};

#endif	// __CCORE_SYSTEM_WIN32_H__