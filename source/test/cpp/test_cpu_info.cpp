#include "cbase/c_target.h"
#include "cbase/c_allocator.h"

#include "csystem/c_system.h"

#include "cunittest/cunittest.h"

namespace ncore
{
    extern void GetCpuInfo();
}

UNITTEST_SUITE_BEGIN(cpu_info)
{
	UNITTEST_FIXTURE(info)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check) 
		{ 
			ncore::GetCpuInfo();
		}
	}
}
UNITTEST_SUITE_END