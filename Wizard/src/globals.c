/// Includes & Defines
#include "rev.h"
#include "/locale/SetupAmiTCP.h"
#include "SetupAmiTCP.h"
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
/// Devices, Ports, Requests
struct   MsgPort        *WritePortSER  = NULL;
struct   MsgPort        *ReadPortSER   = NULL;
struct   IOExtSer       *WriteSER      = NULL;
struct   IOExtSer       *ReadSER       = NULL;
char serial_in[1025];
BOOL ReadQueued = FALSE;   // is something queued in serial read
char serial_buffer[81];
char serial_buffer_old1[81];
char serial_buffer_old2[81];
WORD ser_buf_pos;
int last_input;      // 0: from console, 1: from serial
int checking_modem  = 0;

///
/// MainMenu
enum { MEN_ABOUT=1, MEN_ABOUT_MUI, MEN_QUIT, MEN_MUI };
struct NewMenu MainMenu[] =
{
	{ NM_TITLE  , MSG_MEN_PROJECT    , 0                  , 0, 0, (APTR)0               },
	{ NM_ITEM   , MSG_MEN_ABOUT      , MSG_MEN_SHORT_ABOUT, 0, 0, (APTR)MEN_ABOUT       },
	{ NM_ITEM   , NM_BARLABEL        , 0                  , 0, 0, 0                     },
	{ NM_ITEM   , MSG_MEN_ABOUT_MUI  , 0                  , 0, 0, (APTR)MEN_ABOUT_MUI   },
	{ NM_ITEM   , NM_BARLABEL        , 0                  , 0, 0, 0                     },
	{ NM_ITEM   , MSG_MEN_QUIT       , MSG_MEN_SHORT_QUIT , 0, 0, (APTR)MEN_QUIT        },

	{ NM_TITLE  , MSG_MEN_SETTINGS   , 0                  , 0, 0, (APTR)0               },
	{ NM_ITEM   , MSG_MEN_MUI        , MSG_MEN_SHORT_MUI  , 0, 0, (APTR)MEN_MUI         },

	{ NM_END    , NULL               , 0                  , 0, 0, (APTR)0               },
};
///

/// MUI Class Pointers
struct MUI_CustomClass  *CL_MainWindow          = NULL;
struct MUI_CustomClass  *CL_InfoWindow          = NULL;
struct MUI_CustomClass  *CL_InfoText            = NULL;
///
/// MUI stuff
Object   *app     = NULL;
Object   *win     = NULL;
Object   *group   = NULL;
Object   *LT_Providers = NULL;      // for the OpenFunc
Object   *info_win= NULL;

struct Hook deshook           = {{NULL, NULL}, (VOID *)desfunc          , NULL, NULL};
struct Hook sorthook          = {{NULL, NULL}, (VOID *)sortfunc         , NULL, NULL};
struct Hook strobjhook        = {{NULL, NULL}, (VOID *)strobjfunc       , NULL, NULL};
///

