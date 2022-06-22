/// includes
#include "/AmiTCP.h"
#include "/locale/Strings.h"
#include "mui.h"
#include "rev.h"
#include "//mcc_grouppager/grouppager_mcc.h"

///
/// define images
#define USE_LOGO_HEADER
#define USE_LOGO_BODY
#define USE_LOGO_COLORS
#include "/images/logo.bh"

#define USE_DATABASES_HEADER
#define USE_DATABASES_BODY
#define USE_DATABASES_COLORS
#include "/images/databases.bh"

#define USE_DIALER_HEADER
#define USE_DIALER_BODY
#define USE_DIALER_COLORS
#include "/images/dialer.bh"

#define USE_MODEM_HEADER
#define USE_MODEM_BODY
#define USE_MODEM_COLORS
#include "/images/modem.bh"

#define USE_PROVIDER_HEADER
#define USE_PROVIDER_BODY
#define USE_PROVIDER_COLORS
#include "/images/provider.bh"

#define USE_USER_HEADER
#define USE_USER_BODY
#define USE_USER_COLORS
#include "/images/user.bh"

#define USE_INFORMATION_HEADER
#define USE_INFORMATION_BODY
#define USE_INFORMATION_COLORS
#include "/images/information.bh"

///

/// Libraries
struct   ExecBase             *SysBase       = NULL;
struct   DosLibrary           *DOSBase       = NULL;
struct   Library              *IntuitionBase = NULL;
struct   Library              *MUIMasterBase = NULL;
struct   Library              *UtilityBase   = NULL;
struct   Library              *LocaleBase    = NULL;
struct   Library              *IFFParseBase  = NULL; /* this one's for the config-file */
struct   Library              *DataTypesBase = NULL;
struct   Catalog              *cat           = NULL; /* pointer to our locale catalog */
#ifdef DEMO
struct   Library              *BattClockBase = NULL;
#endif

///

/// global vars
char config_file[MAXPATHLEN];
BOOL changed_passwd;
BOOL changed_group;
BOOL changed_hosts;
BOOL changed_protocols;
BOOL changed_services;
BOOL changed_inetd;
BOOL changed_networks;
BOOL changed_rpc;

///

/// MUI pointers
Object *app                = NULL;  /* our MUI application                 */
Object *win                = NULL;  /* a global pointer to our main window */
Object *group              = NULL;

struct MUI_CustomClass  *CL_MainWindow;
struct MUI_CustomClass  *CL_User;
struct MUI_CustomClass  *CL_Provider;
struct MUI_CustomClass  *CL_Dialer;
struct MUI_CustomClass  *CL_Users;
struct MUI_CustomClass  *CL_Databases;
struct MUI_CustomClass  *CL_Modem;
struct MUI_CustomClass  *CL_About;
struct MUI_CustomClass  *CL_PasswdReq;

///
/// misc pointers & variables
struct Process    *ThisProcess;
struct WBStartup  *WBenchMsg;
BPTR              WBenchLock;

///

/// Menu
enum { MEN_ABOUT = 1, MEN_ABOUT_MUI, MEN_ICONIFY, MEN_QUIT, MEN_LOAD, MEN_SAVE, MEN_SAVEAS, MEN_MUI };
struct NewMenu MainWindowMenu[] =
{
   { NM_TITLE, (STRPTR)MSG_MENU_PROJECT   , 0 , 0, 0, (APTR)0           },
   { NM_ITEM , (STRPTR)MSG_MENU_ABOUT     ,"?", 0, 0, (APTR)MEN_ABOUT   },
   { NM_ITEM , (STRPTR)"  About MUI"      , 0, 0, 0,  (APTR)MEN_ABOUT_MUI },
   { NM_ITEM , (STRPTR)NM_BARLABEL        , 0 , 0, 0, (APTR)0           },
   { NM_ITEM , (STRPTR)"  Iconify"        ,"I", 0, 0, (APTR)MEN_ICONIFY },
   { NM_ITEM , (STRPTR)NM_BARLABEL        , 0 , 0, 0, (APTR)0           },
   { NM_ITEM , (STRPTR)MSG_MENU_QUIT      ,"Q", 0, 0, (APTR)MEN_QUIT    },

   { NM_TITLE, (STRPTR)MSG_MENU_SETTINGS  , 0 , 0, 0, (APTR)0           },
   { NM_ITEM , (STRPTR)"  Load"           ,"L", 0, 0, (APTR)MEN_LOAD    },
   { NM_ITEM , (STRPTR)"  Save"           ,"S", 0, 0, (APTR)MEN_SAVE    },
   { NM_ITEM , (STRPTR)"  Save as..."     ,"W", 0, 0, (APTR)MEN_SAVEAS  },
   { NM_ITEM , (STRPTR)NM_BARLABEL        , 0 , 0, 0, (APTR)0           },
   { NM_ITEM , (STRPTR)MSG_MENU_MUI       ,"M", 0, 0, (APTR)MEN_MUI     },

   { NM_END  , NULL                       , 0 , 0, 0, (APTR)0           },

};

///

/// hooks
SAVEDS ASM VOID des_func(REG(a2) APTR pool, REG(a1) APTR ptr);
SAVEDS ASM LONG sortfunc(REG(a1) STRPTR str1, REG(a2) STRPTR str2);
SAVEDS ASM LONG strobjfunc(REG(a2) Object *list, REG(a1) Object *str);
SAVEDS ASM VOID objstrfunc(REG(a2) Object *list, REG(a1) Object *str);

static struct Hook sorthook = { {NULL, NULL}, (VOID *)sortfunc, NULL, NULL};
static struct Hook des_hook = { {NULL, NULL}, (VOID *)des_func, NULL, NULL};
static struct Hook strobjhook = { {NULL, NULL}, (VOID *)strobjfunc, NULL, NULL};
static struct Hook objstrhook = { {NULL, NULL}, (VOID *)objstrfunc, NULL, NULL};

///

