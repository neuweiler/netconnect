/// Includes
#include "/includes.h"

#include "Strings.h"
#define USE_SCRIPT_COMMANDS
#define USE_EVENT_COMMANDS
#include "/Genesis.h"
#include "mui.h"
#include "protos.h"

///

/// Libraries
struct   Library  *MUIMasterBase    = NULL;
struct   Library  *TimerBase        = NULL;
struct   Library  *LockSocketBase   = NULL;
struct   Library  *OwnDevUnitBase   = NULL;
struct   Library  *GenesisBase      = NULL;
struct   Library  *IconBase         = NULL;
struct   Library  *GenesisLoggerBase= NULL;
struct   Library  *NetConnectBase   = NULL;
struct   Library  *BattClockBase    = NULL;
struct   Library  *VUPBase          = NULL;

///

/// Other data
struct Config Config;

struct Process *proc;
struct StackSwapStruct StackSwapper;

struct Catalog       *cat           = NULL;   /* pointer to our locale catalog */
struct MsgPort       *MainPort      = NULL;   /* port to comunicate with mainProcess */
struct timerequest   *TimeReq       = NULL;
struct MsgPort       *TimePort      = NULL;

struct MinList args_ifaces_online; /* see amiga.c/parse_arguments() */
ULONG  LogNotifySignal = -1, ConfigNotifySignal = -1;
struct NotifyRequest log_nr, config_nr;
ULONG sigs = NULL;
struct CommandLineInterface   *LocalCLI = NULL;
BPTR                          OldCLI = NULL;
int waitstack;
struct User *current_user = NULL;
APTR vuphandle;

const char AmiTCP_PortName[] = "AMITCP";
char config_file[MAXPATHLEN];
char connectspeed[41];

BOOL dialup = 0;

int h_errno;

///

/// MainMenu
struct NewMenu MainMenu[] =
{
   { NM_TITLE  , MSG_MENU_PROJECT   , 0               , 0, 0, (APTR)0               },
   { NM_ITEM   , MSG_MENU_REPORT    , MSG_CC_REPORT   , 0, 0, (APTR)MEN_REPORT      },
   { NM_ITEM   , MSG_MENU_NETSTATUS , MSG_CC_NETSTATUS, 0, 0, (APTR)MEN_NETINFO     },
   { NM_ITEM   , NM_BARLABEL        , 0               , 0, 0, 0                     },
   { NM_ITEM   , MSG_MENU_ABOUT     , MSG_CC_ABOUT    , 0, 0, (APTR)MEN_ABOUT       },
   { NM_ITEM   , MSG_MENU_ABOUTMUI  , 0               , 0, 0, (APTR)MEN_ABOUT_MUI   },
   { NM_ITEM   , NM_BARLABEL        , 0               , 0, 0, 0                     },
   { NM_ITEM   , MSG_MENU_QUIT      , MSG_CC_QUIT     , 0, 0, (APTR)MEN_QUIT        },

   { NM_TITLE  , MSG_MENU_SETTINGS  , 0               , 0, 0, (APTR)0               },
   { NM_ITEM   , MSG_MENU_DISPLAY        , 0                     , 0, 0, (APTR)MEN_DISPLAY               },
   { NM_SUB    , MSG_MENU_SHOW_TIMEONLINE, MSG_CC_SHOW_TIMEONLINE, CHECKIT|MENUTOGGLE|CHECKED, 0, (APTR)MEN_TIMEONLINE  },
   { NM_SUB    , MSG_MENU_SHOW_LEDS      , MSG_CC_SHOW_LEDS      , CHECKIT|MENUTOGGLE|CHECKED, 0, (APTR)MEN_LEDS        },
   { NM_SUB    , NM_BARLABEL             , 0                     , 0, 0, 0                     },
   { NM_SUB    , MSG_MENU_SHOW_CONNECT   , MSG_CC_SHOW_CONNECT   , CHECKIT|MENUTOGGLE|CHECKED, 0, (APTR)MEN_CONNECT     },
   { NM_SUB    , NM_BARLABEL             , 0                     , 0, 0, 0                     },
   { NM_SUB    , MSG_MENU_SHOW_INTERFACE , MSG_CC_SHOW_INTERFACE , CHECKIT|MENUTOGGLE|CHECKED, 0, (APTR)MEN_INTERFACE   },
   { NM_SUB    , MSG_MENU_SHOW_USER      , MSG_CC_SHOW_USER      , CHECKIT|MENUTOGGLE|CHECKED, 0, (APTR)MEN_USER        },
   { NM_SUB    , NM_BARLABEL             , 0                     , 0, 0, 0                     },
   { NM_SUB    , MSG_MENU_SHOW_LOG       , MSG_CC_SHOW_LOG       , CHECKIT|MENUTOGGLE|CHECKED, 0, (APTR)MEN_LOG         },
   { NM_SUB    , MSG_MENU_SHOW_BUTTONS   , MSG_CC_SHOW_BUTTONS   , CHECKIT|MENUTOGGLE|CHECKED, 0, (APTR)MEN_BUTTONS     },
   { NM_SUB    , NM_BARLABEL             , 0                     , 0, 0, 0                     },
   { NM_SUB    , MSG_MENU_SHOW_STATUS    , MSG_CC_SHOW_STATUS    , CHECKIT|MENUTOGGLE|CHECKED, 0, (APTR)MEN_STATUS      },
   { NM_SUB    , MSG_MENU_SHOW_SERIAL    , MSG_CC_SHOW_SERIAL    , CHECKIT|MENUTOGGLE|CHECKED, 0, (APTR)MEN_SERIAL      },
   { NM_ITEM   , NM_BARLABEL             , 0                     , 0, 0, 0                     },
   { NM_ITEM   , MSG_MENU_GENESIS   , MSG_CC_GENESIS  , 0, 0, (APTR)MEN_GENESIS     },
   { NM_ITEM   , MSG_MENU_MUI       , MSG_CC_MUI      , 0, 0, (APTR)MEN_MUI         },

   { NM_END    , NULL                    , 0                     , 0, 0, (APTR)0               },
};
///
/// InetInfoMenu
struct NewMenu NetInfoMenu[] =
{
   { NM_TITLE  , MSG_MENU_PROJECT   , 0               , 0, 0, (APTR)0               },
   { NM_ITEM   , MSG_MENU_REPORT    , MSG_CC_REPORT   , 0, 0, (APTR)MEN_REPORT      },
   { NM_ITEM   , MSG_MENU_MAINWINDOW, MSG_CC_MAINWINDOW, 0, 0, (APTR)MEN_MAINWINDOW },
   { NM_ITEM   , NM_BARLABEL        , 0               , 0, 0, 0                     },
   { NM_ITEM   , MSG_MENU_ABOUT     , MSG_CC_ABOUT    , 0, 0, (APTR)MEN_ABOUT       },
   { NM_ITEM   , MSG_MENU_ABOUTMUI  , 0               , 0, 0, (APTR)MEN_ABOUT_MUI   },
   { NM_ITEM   , NM_BARLABEL        , 0               , 0, 0, 0                     },
   { NM_ITEM   , MSG_MENU_QUIT      , MSG_CC_QUIT     , 0, 0, (APTR)MEN_QUIT        },

   { NM_TITLE  , MSG_MENU_SETTINGS  , 0               , 0, 0, (APTR)0               },
   { NM_ITEM   , MSG_MENU_GENESIS   , MSG_CC_GENESIS  , 0, 0, (APTR)MEN_GENESIS     },
   { NM_ITEM   , MSG_MENU_MUI       , MSG_CC_MUI      , 0, 0, (APTR)MEN_MUI         },

   { NM_END    , NULL                    , 0                     , 0, 0, (APTR)0               },
};
///
/// AREXX stuff

struct Hook user_rxhook       = { {NULL, NULL}, (VOID *)user_rxfunc        , NULL, NULL};
struct Hook online_rxhook     = { {NULL, NULL}, (VOID *)online_rxfunc      , NULL, NULL};
struct Hook offline_rxhook    = { {NULL, NULL}, (VOID *)offline_rxfunc     , NULL, NULL};
struct Hook isonline_rxhook   = { {NULL, NULL}, (VOID *)isonline_rxfunc    , NULL, NULL};
struct Hook window_rxhook     = { {NULL, NULL}, (VOID *)window_rxfunc      , NULL, NULL};

struct MUI_Command arexx_list[] =
{
   {"user"              , "NAME,PW=PASSWORD/K"     , 2, &user_rxhook          },
   {"online"            , "IF=IFACENAMES/M,ALL/S"  , 2, &online_rxhook        },
   {"offline"           , "IF=IFACENAMES/M,ALL/S"  , 2, &offline_rxhook       },
   {"isonline"          , "IF=IFACENAME,ANY/S"     , 2, &isonline_rxhook      },
   {"window"            , "OPEN/S,CLOSE/S"         , 2, &window_rxhook        },
   {NULL                , NULL                     , 0, NULL                  }
};

///

/// MUI Class Pointers
struct MUI_CustomClass  *CL_MainWindow          = NULL;
struct MUI_CustomClass  *CL_Online              = NULL;
struct MUI_CustomClass  *CL_Led                 = NULL;
struct MUI_CustomClass  *CL_About               = NULL;
struct MUI_CustomClass  *CL_NetInfo             = NULL;

///
/// MUI stuff
Object   *app        = NULL;
Object   *win        = NULL;
Object   *status_win = NULL;
Object   *netinfo_win= NULL;

///

