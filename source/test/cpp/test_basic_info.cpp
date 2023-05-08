#include "ccore/c_target.h"
#include "csystem/c_system.h"
#include "csystem/test_allocator.h"

#include "cunittest/cunittest.h"

UNITTEST_SUITE_BEGIN(basic_info)
{
	UNITTEST_FIXTURE(console)
	{
        UNITTEST_ALLOCATOR;
		ncore::system_t system;
		UNITTEST_FIXTURE_SETUP() { system.init(Allocator); }
		UNITTEST_FIXTURE_TEARDOWN() { system.shutdown(); }

		UNITTEST_TEST(check)
		{
			ncore::system_t::EConsoleType consoleType = system.getConsoleType();
#ifdef TARGET_PC
			CHECK_TRUE(consoleType == ncore::system_t::EConsoleType::CONSOLE_DESKTOP);
#endif
			
		}
	}


	UNITTEST_FIXTURE(media)
	{
        UNITTEST_ALLOCATOR;
		ncore::system_t system;
		UNITTEST_FIXTURE_SETUP() { system.init(Allocator); }
		UNITTEST_FIXTURE_TEARDOWN() { system.shutdown(); }

		UNITTEST_TEST(check)
		{
			ncore::system_t::EMediaType mediaType = system.getMediaType();
#ifdef TARGET_PC
			CHECK_TRUE(mediaType == ncore::system_t::EMediaType::MEDIA_HDD);
#endif

		}
	}

	UNITTEST_FIXTURE(language)
	{
        UNITTEST_ALLOCATOR;
		ncore::system_t system;
		UNITTEST_FIXTURE_SETUP() { system.init(Allocator); }
		UNITTEST_FIXTURE_TEARDOWN() { system.shutdown(); }

		UNITTEST_TEST(check)
		{
			system.setLanguage(ncore::system_t::ELanguage::LANGUAGE_ENGLISH);
			ncore::system_t::ELanguage lan = system.getLanguage();
			CHECK_TRUE(lan == ncore::system_t::ELanguage::LANGUAGE_ENGLISH);

		}
	}

	UNITTEST_FIXTURE(otherinfo)
	{
        UNITTEST_ALLOCATOR;
		ncore::system_t system;
		UNITTEST_FIXTURE_SETUP() { system.init(Allocator); }
		UNITTEST_FIXTURE_TEARDOWN() { system.shutdown(); }

		UNITTEST_TEST(check)
		{
			const char* path = system.getExePath();
			CHECK_TRUE(path != nullptr);

			bool isLittleEndian = system.isLittleEndian();
			CHECK_TRUE(isLittleEndian);

			bool isBigEndian = system.isBigEndian();
			CHECK_TRUE(!isBigEndian);

			bool is64Bit = system.is64BitOS();
			CHECK_TRUE(is64Bit);

			//ncore::u32 maxSystemMemory = ncore::system_t::getMaxSystemMemory();
			//CHECK_TRUE(maxSystemMemory >= 0);

			//ncore::u32 currentSystemMemory = ncore::system_t::getCurrentSystemMemory();
			//CHECK_TRUE(currentSystemMemory>= 0);

		}
	}


}
UNITTEST_SUITE_END