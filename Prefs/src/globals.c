/// includes
#include "/includes.h"

#define USE_SCRIPT_COMMANDS
#define USE_EVENT_COMMANDS
#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "protos.h"
#include "mui/grouppager_mcc.h"

///
/// define images
#define USE_LOGO_HEADER
#define USE_LOGO_BODY
#define USE_LOGO_COLORS
#include "images/logo.h"

#define USE_DATABASES_HEADER
#define USE_DATABASES_BODY
#define USE_DATABASES_COLORS
#include "images/databases.h"

#define USE_DIALER_HEADER
#define USE_DIALER_BODY
#define USE_DIALER_COLORS
#include "images/dialer.h"

#define USE_MODEM_HEADER
#define USE_MODEM_BODY
#define USE_MODEM_COLORS
#include "images/modem.h"

#define USE_PROVIDER_HEADER
#define USE_PROVIDER_BODY
#define USE_PROVIDER_COLORS
#include "images/provider.h"

#define USE_USER_HEADER
#define USE_USER_BODY
#define USE_USER_COLORS
#include "images/user.h"

#define USE_INFORMATION_HEADER
#define USE_INFORMATION_BODY
#define USE_INFORMATION_COLORS
#include "images/information.h"

///

/// Libraries
struct   Library      *MUIMasterBase = NULL;
struct   Library      *GenesisBase   = NULL;
struct   Library      *UserGroupBase = NULL;
#ifdef DEMO
struct   Library      *BattClockBase = NULL;
#endif
struct   Catalog      *cat           = NULL; /* pointer to our locale catalog */

///

/// other data
struct Process *proc = NULL;
struct StackSwapStruct StackSwapper;

char config_file[MAXPATHLEN];
BOOL changed_passwd;
BOOL changed_group;
BOOL changed_hosts;
BOOL changed_protocols;
BOOL changed_services;
BOOL changed_inetaccess;
BOOL changed_inetd;
BOOL changed_networks;
BOOL changed_rpc;

BOOL root_authenticated = FALSE;

///

/// Menu
struct NewMenu MainWindowMenu[] =
{
   { NM_TITLE, (STRPTR)MSG_MENU_PROJECT   , 0               , 0, 0, (APTR)0            },
   { NM_ITEM , (STRPTR)MSG_MENU_ABOUT     , MSG_CC_ABOUT    , 0, 0, (APTR)MEN_ABOUT    },
   { NM_ITEM , (STRPTR)MSG_MENU_ABOUTMUI  , 0               , 0, 0, (APTR)MEN_ABOUT_MUI},
   { NM_ITEM , (STRPTR)NM_BARLABEL        , 0               , 0, 0, (APTR)0            },
   { NM_ITEM , (STRPTR)MSG_MENU_ICONIFY   , MSG_CC_ICONIFY  , 0, 0, (APTR)MEN_ICONIFY  },
   { NM_ITEM , (STRPTR)NM_BARLABEL        , 0               , 0, 0, (APTR)0            },
   { NM_ITEM , (STRPTR)MSG_MENU_QUIT      , MSG_CC_QUIT     , 0, 0, (APTR)MEN_QUIT     },

   { NM_TITLE, (STRPTR)MSG_MENU_SETTINGS  , 0               , 0, 0, (APTR)0            },
   { NM_ITEM , (STRPTR)MSG_MENU_LOAD      , MSG_CC_LOAD     , 0, 0, (APTR)MEN_LOAD     },
   { NM_ITEM , (STRPTR)MSG_MENU_IMPORT    , MSG_CC_IMPORT   , 0, 0, (APTR)MEN_IMPORT   },
   { NM_ITEM , (STRPTR)MSG_MENU_SAVE      , MSG_CC_SAVE     , 0, 0, (APTR)MEN_SAVE     },
   { NM_ITEM , (STRPTR)MSG_MENU_SAVEAS    , MSG_CC_SAVEAS   , 0, 0, (APTR)MEN_SAVEAS   },
   { NM_ITEM , (STRPTR)NM_BARLABEL        , 0               , 0, 0, (APTR)0            },
   { NM_ITEM , (STRPTR)MSG_MENU_MUI       , MSG_CC_MUI      , 0, 0, (APTR)MEN_MUI      },

   { NM_END  , NULL                       , 0               , 0, 0, (APTR)0            },

};

///

/// MUI Class pointers

struct MUI_CustomClass  *CL_About;
struct MUI_CustomClass  *CL_Databases;
struct MUI_CustomClass  *CL_Dialer;
struct MUI_CustomClass  *CL_MainWindow;
struct MUI_CustomClass  *CL_Modem;
struct MUI_CustomClass  *CL_PasswdReq;
struct MUI_CustomClass  *CL_User;
struct MUI_CustomClass  *CL_Provider;
struct MUI_CustomClass  *CL_ProviderWindow;
struct MUI_CustomClass  *CL_UserWindow;
struct MUI_CustomClass  *CL_IfaceWindow;

///
/// MUI stuff
Object *app                = NULL;  /* our MUI application                 */
Object *win                = NULL;  /* a global pointer to our main window */
Object *group              = NULL;

struct Hook sorthook   = { {NULL, NULL}, (VOID *)sortfunc  , NULL, NULL};
struct Hook des_hook   = { {NULL, NULL}, (VOID *)des_func  , NULL, NULL};
struct Hook strobjhook = { {NULL, NULL}, (VOID *)strobjfunc, NULL, NULL};
struct Hook objstrhook = { {NULL, NULL}, (VOID *)objstrfunc, NULL, NULL};

///

