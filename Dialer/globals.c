/// Includes & Defines
#include "rev.h"
#include "locale/Strings.h"
#include "/AmiTCP.h"
#include "mui.h"
#include "protos.h"

///

/// Libraries
struct   ExecBase             *SysBase       = NULL;
struct   DosLibrary           *DOSBase       = NULL;
struct   Library              *IntuitionBase = NULL;
struct   Library              *MUIMasterBase = NULL;
struct   Library              *UtilityBase   = NULL;
struct   Library              *LocaleBase    = NULL;
struct   Library              *TimerBase     = NULL;
struct   Library              *SocketBase    = NULL;
struct   RxsLib               *RexxSysBase   = NULL;
#ifdef DEMO
struct   Library              *BattClockBase = NULL;
#endif

///
/// Other Pointers
struct   Catalog              *cat           = NULL; /* pointer to our locale catalog */
struct   Process              *process       = NULL;

struct WBStartup     *WBenchMsg;
BPTR                 WBenchLock = NULL;
struct CommandLineInterface   *LocalCLI = NULL;
STATIC BPTR                   OldCLI = NULL;
ULONG sigs = NULL;

///
/// Other data
struct config Config;

struct MinList dialscript;
struct MinList user_startnet;
struct MinList user_stopnet;

struct MsgPort       *MainPort  = NULL;   /* port to comunicate with mainProcess */
struct IOExtSer      *SerReq    = NULL;   /* Serial IORequest */
struct MsgPort       *SerPort   = NULL;   /* Serial reply port */
struct timerequest   *TimeReq   = NULL;
struct MsgPort       *TimePort  = NULL;

ULONG  NotifySignal = -1;
struct NotifyRequest nr;

const char AmiTCP_PortName[] = "AMITCP";

int dialing_try = 0;
int dial_number = 0;

int h_errno;

///

/// MainMenu
enum { MEN_ABOUT=1, MEN_ABOUT_MUI, MEN_QUIT, MEN_MUI };
struct NewMenu MainMenu[] =
{
   { NM_TITLE  , MSG_MENU_PROJECT   , 0                  , 0, 0, (APTR)0               },
   { NM_ITEM   , MSG_MENU_ABOUT     , "  ?", 0, 0, (APTR)MEN_ABOUT       },
   { NM_ITEM   , NM_BARLABEL        , 0                  , 0, 0, 0                     },
   { NM_ITEM   , MSG_MENU_MUI       , 0                  , 0, 0, (APTR)MEN_ABOUT_MUI   },
   { NM_ITEM   , NM_BARLABEL        , 0                  , 0, 0, 0                     },
   { NM_ITEM   , MSG_MENU_QUIT      , "  Q" , 0, 0, (APTR)MEN_QUIT        },

   { NM_TITLE  , MSG_MENU_SETTINGS  , 0                  , 0, 0, (APTR)0               },
   { NM_ITEM   , MSG_MENU_MUI       , "  M"  , 0, 0, (APTR)MEN_MUI         },

   { NM_END    , NULL               , 0                  , 0, 0, (APTR)0               },
};
///

/// MUI Class Pointers
struct MUI_CustomClass  *CL_MainWindow          = NULL;
struct MUI_CustomClass  *CL_Online              = NULL;

///
/// MUI stuff
Object   *app        = NULL;
Object   *win        = NULL;
Object   *status_win = NULL;

struct Hook deshook           = {{NULL, NULL}, (VOID *)desfunc          , NULL, NULL};
struct Hook sorthook          = {{NULL, NULL}, (VOID *)sortfunc         , NULL, NULL};
struct Hook strobjhook        = {{NULL, NULL}, (VOID *)strobjfunc       , NULL, NULL};
///

