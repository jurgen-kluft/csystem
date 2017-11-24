// x_system_cpu.h - System CPU information
#ifndef __XCORE_SYSTEM_CPU_H__
#define __XCORE_SYSTEM_CPU_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

namespace xcore
{
	class xicpu_info;

	class xcpu_info
	{
	public:
						xcpu_info() : mCpuInfo(NULL)	{ }

		void			initialize();
		
		void			print();

		u64				getCPUSpeedInkHz() const;
		u64				getCPUSpeedInMHz() const;
		s32				getPhysicalProcessors () const;
		s32				getLogicalProcessorsPerPhysical () const;
	private:
		xicpu_info*		mCpuInfo;
	};
}

#endif	///< __XCORE_SYSTEM_CPU_H__
