/// includes
#include "/includes.h"

#define USE_SCRIPT_COMMANDS
#define USE_EVENT_COMMANDS
#define USE_EXEC_TYPES
#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "protos.h"
#include "grouppager_mcc.h"

///
/// define images
#define USE_LOGO_HEADER
#define USE_LOGO_BODY
#define USE_LOGO_COLORS
#include "images/logo.h"

#define USE_DATABASE_HEADER
#define USE_DATABASE_BODY
#define USE_DATABASE_COLORS
#include "images/database.h"

#define USE_OPTIONS_HEADER
#define USE_OPTIONS_BODY
#define USE_OPTIONS_COLORS
#include "images/options.h"

#define USE_MODEMS_HEADER
#define USE_MODEMS_BODY
#define USE_MODEMS_COLORS
#include "images/modems.h"

#define USE_INTERFACES_HEADER
#define USE_INTERFACES_BODY
#define USE_INTERFACES_COLORS
#include "images/interfaces.h"

#define USE_DEFAULT_HEADER
#define USE_DEFAULT_BODY
#define USE_DEFAULT_COLORS
#include "images/default.h"

#ifdef INTERNAL_USER
#define USE_USER_HEADER
#define USE_USER_BODY
#define USE_USER_COLORS
#include "images/user.h"
#endif

#define USE_INFORMATION_HEADER
#define USE_INFORMATION_BODY
#define USE_INFORMATION_COLORS
#include "images/information.h"

///

/// Libraries
struct   Library      *MUIMasterBase = NULL;
struct   Library      *GenesisBase   = NULL;
struct   Library      *UserGroupBase = NULL;
struct   Library      *NetConnectBase= NULL;
struct   Catalog      *cat           = NULL; /* pointer to our locale catalog */

///

/// other data
struct Process *proc = NULL;
struct StackSwapStruct StackSwapper;

char config_file[MAXPATHLEN];
#ifdef INTERNAL_USER
BOOL changed_passwd;
#endif
BOOL changed_group, changed_hosts, changed_protocols, changed_services, changed_inetaccess,
     changed_inetd, changed_networks, changed_rpc;
BOOL root_authenticated = FALSE;
struct User *current_user = NULL;
struct MinList McpList;

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

struct MUI_CustomClass *CL_MainWindow;
struct MUI_CustomClass *CL_About;
struct MUI_CustomClass *CL_Interfaces;
struct MUI_CustomClass *CL_IfaceWindow;
struct MUI_CustomClass *CL_Options;
struct MUI_CustomClass *CL_Modems;
struct MUI_CustomClass *CL_ModemWindow;
struct MUI_CustomClass *CL_Database;
struct MUI_CustomClass *CL_LogLevel;
#ifdef INTERNAL_USER
struct MUI_CustomClass *CL_User;
struct MUI_CustomClass *CL_UserWindow;
#endif

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

