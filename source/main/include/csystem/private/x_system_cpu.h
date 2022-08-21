#ifndef __CCORE_SYSTEM_CPU_H__
#define __CCORE_SYSTEM_CPU_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

namespace ncore
{
	class cpu_info_t
	{
		static void			initialize();
		
		static s32			getPhysicalProcessors ();
		static s32			getLogicalProcessorsPerPhysical ();

	};
}

#endif	///< __CCORE_SYSTEM_CPU_H__
