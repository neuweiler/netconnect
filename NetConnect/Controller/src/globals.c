/// includes
#include "/includes.h"

#include "/NetConnect.h"
#include "/locale/Strings.h"
#include "mui.h"
#include "protos.h"

#define USE_DEFAULT_ICON_HEADER
#define USE_DEFAULT_ICON_BODY
#define USE_DEFAULT_ICON_COLORS
#include "/images/default_icon.h"

///

/// Libraries
struct   Library              *MUIMasterBase = NULL;
struct   Library              *IFFParseBase  = NULL;
struct   Library              *DataTypesBase = NULL;
struct   Library              *CxBase        = NULL;
struct   Library              *WorkbenchBase = NULL;
struct   Library              *IconBase      = NULL;
struct   Library              *NetConnectBase= NULL;
struct   Library              *DiskfontBase  = NULL;
#ifdef DEMO
struct   Library              *BattClockBase = NULL;
#endif

///

/// other data
struct   Catalog              *cat           = NULL;
struct StackSwapStruct StackSwapper;
Object *SoundObject = NULL;
struct MsgPort                *appmenu_port;
struct Process                *proc;
struct CommandLineInterface   *LocalCLI = NULL;
BPTR                   OldCLI = NULL;
ULONG NotifySignal = -1;
struct NotifyRequest nr={DEFAULT_CONFIGFILE, NULL, 0, NRF_SEND_SIGNAL};
BOOL running;

///
/// DockMenu
struct NewMenu DockMenu[] =
{
   { NM_TITLE, (STRPTR)MSG_MENU_PROJECT   , 0 , 0, 0, (APTR)0              },
   { NM_ITEM , (STRPTR)MSG_MENU_ABOUT     ,"?", 0, 0, (APTR)MEN_ABOUT      },
   { NM_ITEM , (STRPTR)MSG_MENU_HELP      ,"H", 0, 0, (APTR)MEN_HELP       },
   { NM_ITEM , (STRPTR)NM_BARLABEL        , 0 , 0, 0, (APTR)0              },
   { NM_ITEM , (STRPTR)MSG_MENU_QUIT      ,"Q", 0, 0, (APTR)MEN_QUIT       },

   { NM_TITLE, (STRPTR)MSG_MENU_SETTINGS  , 0 , 0, 0, (APTR)0              },
   { NM_ITEM , (STRPTR)MSG_MENU_CONTROLLER,"C", 0, 0, (APTR)MEN_CONTROLLER },
   { NM_ITEM , (STRPTR)MSG_MENU_GENESIS   ,"G", 0, 0, (APTR)MEN_GENESIS    },
   { NM_ITEM , (STRPTR)NM_BARLABEL        , 0 , 0, 0, (APTR)0              },
   { NM_ITEM , (STRPTR)MSG_MENU_MUI       ,"M", 0, 0, (APTR)MEN_MUI        },

   { NM_END  , NULL                       , 0 , 0, 0, (APTR)0              },

};

///
/// arexx_list
struct MUI_Command arexx_list[] =
{
   {"refresh"  , MC_TEMPLATE_ID  , ID_REFRESH, NULL},
   {NULL       , NULL            , 0, NULL}
};

///

/// BrokerFunc
SAVEDS ASM int BrokerFunc(REG(a1) CxMsg *msg);
struct Hook BrokerHook = { { 0,0 }, (VOID *)BrokerFunc, NULL, NULL };

///
/// MUI Class Pointers
struct MUI_CustomClass  *CL_Dock       = NULL;
struct MUI_CustomClass  *CL_Button     = NULL;
struct MUI_CustomClass  *CL_About      = NULL;
struct MUI_CustomClass  *CL_MenuList   = NULL;

///
/// MUI stuff
Object *app                = NULL;
Object *group              = NULL;  /* an invisible group in the invisible main window :) */
Object *menu_list          = NULL;
struct MUI_InputHandlerNode   ihnode;

///

