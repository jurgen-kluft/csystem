#include "cbase/c_target.h"
#include "csystem/c_system.h"
#include "csystem/private/c_system_win32.h"

#include "cunittest/cunittest.h"

extern ncore::csystem::xctxt* gCtxt;

UNITTEST_SUITE_BEGIN(basic_info)
{
	UNITTEST_FIXTURE(console)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			ncore::csystem::EConsoleType consoleType = ncore::csystem::getConsoleType(gCtxt);
#ifdef TARGET_PC
			CHECK_TRUE(consoleType == ncore::csystem::EConsoleType::CONSOLE_DESKTOP);
#endif
			
		}
	}


	UNITTEST_FIXTURE(media)
	{
		UNITTEST_FIXTURE_SETUP() { }
		UNITTEST_FIXTURE_TEARDOWN() { }

		UNITTEST_TEST(check)
		{
			ncore::csystem::EMediaType mediaType = ncore::csystem::getMediaType(gCtxt);
#ifdef TARGET_PC
			CHECK_TRUE(mediaType == ncore::csystem::EMediaType::MEDIA_HDD);
#endif

		}
	}

	UNITTEST_FIXTURE(language)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			ncore::csystem::setLanguage(gCtxt, ncore::csystem::ELanguage::LANGUAGE_ENGLISH);//ncore::csystem::GetLanguage();
			ncore::csystem::ELanguage lan = ncore::csystem::getLanguage(gCtxt);
			CHECK_TRUE(lan == ncore::csystem::ELanguage::LANGUAGE_ENGLISH);

		}
	}

	UNITTEST_FIXTURE(otherinfo)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			const char* path = ncore::csystem::getExePath(gCtxt);
			CHECK_TRUE(path != NULL);

			//ncore::u32 maxSystemMemory = ncore::csystem::getMaxSystemMemory();
			//CHECK_TRUE(maxSystemMemory >= 0);

			//ncore::u32 currentSystemMemory = ncore::csystem::getCurrentSystemMemory();
			//CHECK_TRUE(currentSystemMemory>= 0);

		}
	}


}
UNITTEST_SUITE_END