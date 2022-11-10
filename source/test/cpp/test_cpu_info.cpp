#include "cbase/c_target.h"
#include "xsystem/x_system.h"
#include "xsystem/private/x_system_win32.h"
#include "cunittest/cunittest.h"

extern ncore::x_iallocator* gTestAllocator;
extern ncore::xsystem::xctxt* gCtxt;

UNITTEST_SUITE_BEGIN(cpu_info)
{
	UNITTEST_FIXTURE(info)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			

		}
	}
}
UNITTEST_SUITE_END