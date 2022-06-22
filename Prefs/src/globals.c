#include "AmiTCPPrefs.h"
#include "locale/Strings.h"
#include "mui.h"
#include "rev.h"

#define USE_LOGO_HEADER
#define USE_LOGO_BODY
#define USE_LOGO_COLORS
#include "images/logo.bh"

#define USE_INFORMATION_HEADER
#define USE_INFORMATION_BODY
#define USE_INFORMATION_COLORS
#include "images/information.bh"

#define USE_DATABASES_HEADER
#define USE_DATABASES_BODY
#define USE_DATABASES_COLORS
#include "images/databases.bh"

#define USE_GROUPS_HEADER
#define USE_GROUPS_BODY
#define USE_GROUPS_COLORS
#include "images/groups.bh"

#define USE_MODEM_HEADER
#define USE_MODEM_BODY
#define USE_MODEM_COLORS
#include "images/modem.bh"

#define USE_PATHS_HEADER
#define USE_PATHS_BODY
#define USE_PATHS_COLORS
#include "images/paths.bh"

#define USE_PROVIDER_HEADER
#define USE_PROVIDER_BODY
#define USE_PROVIDER_COLORS
#include "images/provider.bh"

#define USE_USER_HEADER
#define USE_USER_BODY
#define USE_USER_COLORS
#include "images/user.bh"

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
struct MUI_CustomClass  *CL_Users;
struct MUI_CustomClass  *CL_Databases;
struct MUI_CustomClass  *CL_Events;
struct MUI_CustomClass	*CL_About;
struct MUI_CustomClass	*CL_MemberList;
struct MUI_CustomClass	*CL_GroupIDString;
struct MUI_CustomClass	*CL_PagerList;
struct MUI_CustomClass	*CL_InfoWindow;

struct Process		*ThisProcess;
struct WBStartup	*WBenchMsg;
BPTR					WBenchLock;
BOOL	expert = FALSE;

enum { MEN_ABOUT = 1, MEN_QUIT, MEN_EXPERT, MEN_IMPORT, MEN_EXPORT, MEN_MUI , MEN_HELP_AMITCP };
struct NewMenu AmiTCPPrefsMenu[] =
{
	{ NM_TITLE, (STRPTR)MSG_MENU_PROJECT	, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM ,	(STRPTR)MSG_MENU_ABOUT		,"?", 0, 0, (APTR)MEN_ABOUT	},
	{ NM_ITEM , (STRPTR)NM_BARLABEL			, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM ,	(STRPTR)MSG_MENU_QUIT		,"Q", 0, 0, (APTR)MEN_QUIT		},

	{ NM_TITLE, (STRPTR)MSG_MENU_SETTINGS	, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM , (STRPTR)MSG_MENU_EXPERT		,"X", CHECKIT|MENUTOGGLE, 0, (APTR)MEN_EXPERT	},
	{ NM_ITEM , (STRPTR)NM_BARLABEL			, 0 , 0, 0, (APTR)0				},
#ifdef DEMO
	{ NM_ITEM , (STRPTR)MSG_MENU_IMPORT		,"I", NM_ITEMDISABLED, 0, (APTR)MEN_IMPORT	},
#else
	{ NM_ITEM , (STRPTR)MSG_MENU_IMPORT		,"I", 0, 0, (APTR)MEN_IMPORT	},
#endif
	{ NM_ITEM , (STRPTR)MSG_MENU_EXPORT		,"E", 0, 0, (APTR)MEN_EXPORT	},
	{ NM_ITEM , (STRPTR)NM_BARLABEL			, 0 , 0, 0, (APTR)0				},
	{ NM_ITEM , (STRPTR)MSG_MENU_MUI			,"M", 0, 0, (APTR)MEN_MUI		},

	{ NM_TITLE , (STRPTR)MSG_MENU_HELP				, 0, 0, 0, (APTR)0},
	{ NM_ITEM  , (STRPTR)MSG_MENU_HELP_AMITCP		, 0, 0, 0, (APTR)MEN_HELP_AMITCP		},

	{ NM_END  , NULL								, 0 , 0, 0, (APTR)0				},

};

SAVEDS ASM VOID des_func(REG(a2) APTR pool, REG(a1) APTR ptr);
SAVEDS ASM LONG sortfunc(REG(a1) STRPTR str1, REG(a2) STRPTR str2);
SAVEDS ASM LONG strobjfunc(REG(a2) Object *list, REG(a1) Object *str);
SAVEDS ASM LONG txtobjfunc(REG(a2) Object *list, REG(a1) Object *str);

static struct Hook sorthook = { {NULL, NULL}, (VOID *)sortfunc, NULL, NULL};
static struct Hook des_hook = { {NULL, NULL}, (VOID *)des_func, NULL, NULL};
static struct Hook strobjhook = { {NULL, NULL}, (VOID *)strobjfunc, NULL, NULL};
static struct Hook txtobjhook = { {NULL, NULL}, (VOID *)txtobjfunc, NULL, NULL};
