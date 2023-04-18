#include "cbase/c_target.h"
#include "cbase/c_debug.h"

#include "csystem/c_system.h"

namespace ncore
{
    static system_t* sSystem = nullptr;

    void             system_t::setSystem(system_t* system) { sSystem = system; }
    system_t*        system_t::getSystem() { return sSystem; }

    bool system_t::is64BitOS() const
    {
        return sizeof(void*) == 8;
    }

    bool system_t::isLittleEndian() const
    {
        char16_t test = 0x0102;
        return ((char*)&test)[0] == 0x02;
    }

    bool system_t::isBigEndian() const
    {
        return !isLittleEndian();
    }

    static const char* sLanguageStr[] = {
        "English", "French",  "Italian", "German",    "Spanish", "Greek", "English US", "French US", "Portuguese", "Brazilian", "Japanese", "Chinese", "Korean", "Russian", "Dutch", "Chinese traditional", "Chinese simplified",
        "Finnish", "Swedish", "Danish",  "Norwegian", "Polish",
    };

    const char* system_t::getLanguageString(ELanguage language) const { return (sLanguageStr[language]); }

    const char* system_t::getUnixEndLine() { return "\n"; }
    const char* system_t::getWindowsEndLine() { return "\r\n"; }

    const char* system_t::getBuildConfigName() const
    {
#    if defined(TARGET_DEBUG)
        return "Debug";
#    elif defined(TARGET_RELEASE)
        return "Release";
#    elif defined(TARGET_FINAL)
        return "Final";
#    endif
    }

    const char* system_t::getBuildModeName() const
    {
#    if defined(TARGET_DEBUG)
        return "Debug";
#    elif defined(TARGET_DEV)
        return "Dev";
#    elif defined(TARGET_CLIENT)
        return "Client";
#    elif defined(TARGET_RETAIL)
        return "Retail";
#    endif
    }

}; // namespace ncore
