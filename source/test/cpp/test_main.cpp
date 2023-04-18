#include "cbase/c_base.h"
#include "cbase/c_allocator.h"
#include "cbase/c_console.h"
#include "csystem/c_system.h"

#include "cunittest/cunittest.h"

UNITTEST_SUITE_LIST(cUnitTest);
UNITTEST_SUITE_DECLARE(cUnitTest, cpu_info);
UNITTEST_SUITE_DECLARE(cUnitTest, exe_info);
UNITTEST_SUITE_DECLARE(cUnitTest, basic_info);

#include "test_main.cxx"