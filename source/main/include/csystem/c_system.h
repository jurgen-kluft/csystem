#ifndef __CSYSTEM_H__
#define __CSYSTEM_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    class alloc_t;

    class system_t
    {
    public:
        struct instance_t;

        static void      setSystem(system_t* instance);
        static system_t* getSystem();

        enum ELanguage
        {
            LANGUAGE_INVALID    = -1,
            LANGUAGE_ENGLISH    = 0,
            LANGUAGE_FRENCH     = 1,
            LANGUAGE_ITALIAN    = 2,
            LANGUAGE_GERMAN     = 3,
            LANGUAGE_SPANISH    = 4,
            LANGUAGE_GREEK      = 5,
            LANGUAGE_ENGLISH_US = 6,
            LANGUAGE_FRENCH_US  = 7,
            LANGUAGE_PORTUGUESE = 8,
            LANGUAGE_BRAZILIAN  = 9, ///< Brazilian Portuguese
            LANGUAGE_JAPANESE   = 10,
            LANGUAGE_KOREAN     = 11, ///< Korean
            LANGUAGE_RUSSIAN    = 12, ///< Russian
            LANGUAGE_DUTCH      = 13,
            LANGUAGE_CHINESE_T  = 14,
            LANGUAGE_CHINESE_S  = 15,
            LANGUAGE_FINNISH    = 16,
            LANGUAGE_SWEDISH    = 17,
            LANGUAGE_DANISH     = 18,
            LANGUAGE_NORWEGIAN  = 19,
            LANGUAGE_POLISH     = 20,

            LANGUAGE_COUNT,

            LANGUAGE_DEFAULT = LANGUAGE_ENGLISH,
            LANGUAGE_MAIN    = LANGUAGE_DEFAULT
        };

        enum EConsoleType
        {
            CONSOLE_DESKTOP,
            CONSOLE_DEVKIT,
            CONSOLE_TESTKIT,
            CONSOLE_RETAIL,
        };

        enum EMediaType
        {
            MEDIA_LOCAL   = 0x01,
            MEDIA_NETWORK = 0x02,
            MEDIA_CD      = 0x04,
            MEDIA_DVD     = 0x08,
            MEDIA_BLURAY  = 0x10,
            MEDIA_SD      = 0x20,
            MEDIA_HDD     = 0x40,
            MEDIA_SSD     = 0x80
        };

        enum EDataSource
        {
            DATA_FROM_HOST     = 0x00,
            DATA_FROM_DVD      = 0x10,
            DATA_FROM_NETWORK  = 0x20,
            DATA_FROM_HARDDISK = 0x40,
            DATA_FROM_SSD      = 0x80,
            DATA_FROM_MASK     = 0xFF,
        };

        enum EEnvMode
        {
            ENVIRONMENT_OPEN        = 0,
            ENVIRONMENT_DEBUG       = 1,
            ENVIRONMENT_DEVELOPMENT = 2,
            ENVIRONMENT_QA          = 3,
            ENVIRONMENT_RETAIL      = 4,
        };

        void init(alloc_t*);
        void update();
        void shutdown();

        bool isLittleEndian() const;
        bool isBigEndian() const;
        bool is64BitOS() const;

        EConsoleType getConsoleType() const;
        EMediaType   getMediaType() const;
        EDataSource  getDataSource() const;
        EEnvMode     getEnvMode() const;

        ELanguage   getLanguage() const;
        const char* getLanguageString(ELanguage) const;
        void        setLanguage(ELanguage language);

        u64 getTotalMemorySize() const;
        u64 getCurrentSystemMemory() const;

        const char* getUnixEndLine();
        const char* getWindowsEndLine();
        const char* getEndLine();

        void setUserName(const char* szUserName);
        void getUserName(char* szBuffer, u32 szBufferMaxLen) const;

        bool isCircleButtonBack() const;

        void        setAppTitle(const char* title) const;
        const char* getAppTitle() const;
        const char* getExePath() const;

        bool hasVirtualMemory() const;

        s32 getNumCores() const;
        s32 getNumHwThreadsPerCore() const;
        u64 getCoreClockFrequency() const;

        const char* getPlatformName() const;    // Name of platform
        const char* getBuildConfigName() const; // Debug, Release, Final
        const char* getBuildModeName() const;   // Dev, Client, Retail
        const char* getConsoleTypeName() const; //
        const char* getMediaTypeName() const;   //
        const char* getLanguageName() const;

        u64 getCodeSegmentSize() const;
        u64 getBssSegmentSize() const;
        u64 getDataSegmentSize() const;
        u64 getMainThreadStackSize() const;

        instance_t* m_instance;
    };
}; // namespace ncore

#endif // __CSYSTEM_H__
