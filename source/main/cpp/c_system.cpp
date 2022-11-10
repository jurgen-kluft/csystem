#include "cbase/c_target.h"


//==============================================================================
// INCLUDES
//==============================================================================

#include "cbase/c_debug.h"
#include "cbase/c_string_ascii.h"

#include "xsystem/x_system.h"


//==============================================================================
// xCore namespace
//==============================================================================
namespace ncore
{
	namespace xsystem
	{
		const char* sLanguageStr[] = 
		{
			"English",
			"French",
			"Italian",
			"German",
			"Spanish",
			"Greek",
			"English US",
			"French US",
			"Portuguese",
			"Brazilian",
			"Japanese",
			"Chinese",
			"Korean",
			"Russian",
			"Dutch",
			"Chinese traditional",
			"Chinese simplified",
			"Finnish",
			"Swedish",
			"Danish",
			"Norwegian",
			"Polish",
		};

		const char*		gToString(ELanguage language)
		{
			return (sLanguageStr[language]);
		}

	};

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};


