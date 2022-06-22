/// Includes & Defines
#include "rev.h"
#include "locale/Strings.h"
#include "/AmiTCP.h"
#include "mui.h"
#include "protos.h"

#define SIG_SER   (1L << ReadPortSER->mp_SigBit)

///

/// Libraries
struct   ExecBase             *SysBase       = NULL;
struct   DosLibrary           *DOSBase       = NULL;
struct   Library              *IntuitionBase = NULL;
struct   Library              *MUIMasterBase = NULL;
struct   Library              *UtilityBase   = NULL;
struct   Library              *LocaleBase    = NULL;
struct   Library              *SocketBase    = NULL;
struct   Library              *IfConfigBase  = NULL;
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

int dialing_try = 0;
int dial_number = 0;
int addr_assign = 0, dst_assign = 0, dns_assign = 0, domainname_assign = 0;

int h_errno;

char ip[21], dest[21], dns1[21], dns2[21], mask[21];

///
/// Devices, Ports, Requests
struct   MsgPort        *WritePortSER  = NULL;
struct   MsgPort        *ReadPortSER   = NULL;
struct   IOExtSer       *WriteSER      = NULL;
struct   IOExtSer       *ReadSER       = NULL;
char serial_in[1025];
BOOL ReadQueued = FALSE;   // is something queued in serial read
char serial_buffer[81], keyboard_buffer[81];
char serial_buffer_old1[81];
char serial_buffer_old2[81];
WORD ser_buf_pos, key_buf_pos;
BOOL keyboard_input = FALSE;

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
struct MUI_CustomClass  *CL_ModemDetect         = NULL;
struct MUI_CustomClass  *CL_ModemProtocol       = NULL;
struct MUI_CustomClass  *CL_ModemWindow         = NULL;
struct MUI_CustomClass  *CL_Online              = NULL;

///
/// MUI stuff
Object   *app     = NULL;
Object   *win     = NULL;
Object   *group   = NULL;
Object   *li_script = NULL;

struct Hook deshook           = {{NULL, NULL}, (VOID *)desfunc          , NULL, NULL};
struct Hook sorthook          = {{NULL, NULL}, (VOID *)sortfunc         , NULL, NULL};
struct Hook strobjhook        = {{NULL, NULL}, (VOID *)strobjfunc       , NULL, NULL};
struct Hook txtobjhook        = {{NULL, NULL}, (VOID *)txtobjfunc       , NULL, NULL};
///

