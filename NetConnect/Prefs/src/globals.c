#include "Config.h"
#include "ConfigStrings.h"

struct	Library					*IntuitionBase	= NULL;
struct	Library					*MUIMasterBase	= NULL;
struct	Library					*UtilityBase	= NULL;
struct	Library					*LocaleBase		= NULL;
struct	Library					*IFFParseBase	= NULL; /* this one's for the config-file */
struct	Library					*DataTypesBase	= NULL;
struct	Catalog					*cat				= NULL; /* pointer to our locale catalog */

Object *app						= NULL;	/* our MUI application						*/
Object *win						= NULL;	/* a global pointer to our main window */

struct MUI_CustomClass	*CL_AmiTCPPrefs;
struct MUI_CustomClass  *CL_UserPrefs;
struct MUI_CustomClass  *CL_ServerPrefs;
struct MUI_CustomClass  *CL_InterfacePrefs;
struct MUI_CustomClass  *CL_MiscPrefs;
struct MUI_CustomClass  *CL_DialscriptPrefs;
struct MUI_CustomClass  *CL_UserStartnetPrefs;

STRPTR STR_GR_Register[7];
STRPTR STR_RA_Connection[3];		/* array which holds the strings for static/dynamic radio buttons in AmiTCP prefs	*/
STRPTR STR_RA_Interface[3];
STRPTR STR_CY_Compression[4];
STRPTR ARR_BaudRates[] = { "9600", "14400", "19200", "38400", "57600", "76800", "115200", "230400", "345600", "460800" , NULL };
STRPTR ARR_DialPrefix[] = { "ATDT", "ATDP", "ATD0w", "ATD0,", NULL };
