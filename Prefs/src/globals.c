#include "AmiTCPPrefs.h"
#include "locale/Strings.h"
#include "mui.h"
#include "rev.h"

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
#ifdef DEMO
struct	Library					*BattClockBase	= NULL;
#endif

Object *app						= NULL;	/* our MUI application						*/
Object *win						= NULL;	/* a global pointer to our main window */
Object *group					= NULL;

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

struct Process		*ThisProcess;
struct WBStartup	*WBenchMsg;
BPTR					WBenchLock;

enum { MEN_ABOUT = 1, MEN_QUIT, MEN_MUI , MEN_HELP_AMITCP };
struct NewMenu AmiTCPPrefsMenu[] =
{
	{ NM_TITLE, (STRPTR)MSG_MENU_PROJECT	, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM ,	(STRPTR)MSG_MENU_ABOUT		,"?", 0, 0, (APTR)MEN_ABOUT	},
	{ NM_ITEM , (STRPTR)NM_BARLABEL			, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM ,	(STRPTR)MSG_MENU_QUIT		,"Q", 0, 0, (APTR)MEN_QUIT		},

	{ NM_TITLE, (STRPTR)MSG_MENU_SETTINGS	, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM , (STRPTR)MSG_MENU_MUI			,"M", 0, 0, (APTR)MEN_MUI		},

	{ NM_TITLE , (STRPTR)MSG_MENU_HELP				, 0, 0, 0, (APTR)0},
	{ NM_ITEM  , (STRPTR)MSG_MENU_HELP_AMITCP		, 0, 0, 0, (APTR)MEN_HELP_AMITCP		},

	{ NM_END  , NULL								, 0 , 0, 0, (APTR)0				},

};
