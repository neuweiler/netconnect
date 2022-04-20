#include "AmiTCPPrefs.h"
#include "AmiTCPPrefsStrings.h"

#define USE_LOGO_HEADER
#define USE_LOGO_BODY
#define USE_LOGO_COLORS
#include "logo.bh"

struct	ExecBase					*SysBase			= NULL;
struct	DosLibrary				*DOSBase			= NULL;
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
struct MUI_CustomClass  *CL_User;
struct MUI_CustomClass  *CL_Provider;
struct MUI_CustomClass  *CL_Modem;
struct MUI_CustomClass  *CL_Paths;
//struct MUI_CustomClass  *CL_Users;
struct MUI_CustomClass	*CL_About;
struct MUI_CustomClass	*CL_MemberList;
struct MUI_CustomClass	*CL_GroupIDString;
struct MUI_CustomClass	*CL_InfoWindow;

STRPTR STR_GR_ProviderRegister[5];
STRPTR STR_GR_UserRegister[3];
STRPTR STR_GR_ModemRegister[3];

STRPTR STR_RA_Connection[3];		/* array which holds the strings for static/dynamic radio buttons in AmiTCP prefs	*/
STRPTR STR_RA_Interface[3];
STRPTR STR_CY_Compression[4];
STRPTR STR_CY_Authentication[4];
STRPTR STR_CY_Header[4];
STRPTR ARR_BaudRates[] = { "9600", "14400", "19200", "38400", "57600", "76800", "115200", "230400", "345600", "460800" , NULL };
STRPTR ARR_DialPrefix[] = { "ATDT", "ATDP", "ATD0w", "ATD0,", NULL };
STRPTR ARR_Pages[] = { "Info", "Provider", "User", "Modem", "Paths" /*, "Users"*/, NULL };
STRPTR ARR_DialScript_AddLine[] = { "/* Dialscript */", "YourLogin=\"\"", "YourPassword=\"\"", "ShowConsole", "call CommandState", "call Dial", "WaitFor", "SendLn", "SendLn YourLogin", "SendLn YourPassword", "Set WaitForTimeout", "Set InterCharDelay", NULL };

struct Process		*ThisProcess;
struct WBStartup	*WBenchMsg;
BPTR					WBenchLock;


enum { MEN_ABOUT = 1, MEN_QUIT, MEN_MUI };
struct NewMenu AmiTCPPrefsMenu[] =
{
	{ NM_TITLE, (STRPTR)MSG_MENU_PROJECT	, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM ,	(STRPTR)MSG_MENU_ABOUT		,"?", 0, 0, (APTR)MEN_ABOUT	},
	{ NM_ITEM , (STRPTR)NM_BARLABEL			, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM ,	(STRPTR)MSG_MENU_QUIT		,"Q", 0, 0, (APTR)MEN_QUIT		},

	{ NM_TITLE, (STRPTR)MSG_MENU_SETTINGS	, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM , (STRPTR)MSG_MENU_MUI			,"M", 0, 0, (APTR)MEN_MUI		},

	{ NM_END  , NULL								, 0 , 0, 0, (APTR)0				},

};
