/// Includes
#include "/includes.h"
#pragma header

#define USE_SCRIPT_COMMANDS 1

#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "protos.h"

///

/// Libraries
struct   Library   *MUIMasterBase = NULL;
struct   Library   *GenesisBase   = NULL;
struct   Library   *SocketBase    = NULL;

#ifdef DEMO
struct   Library   *BattClockBase = NULL;
#endif

///
/// Other data
struct Config  Config;
struct ISP     ISP;
struct Interface Iface;

struct Process *proc;
struct StackSwapStruct StackSwapper;

struct Catalog       *cat           = NULL;   /* pointer to our locale catalog */
struct IOExtSer      *SerReadReq    = NULL;   /* Serial IORequest */
struct MsgPort       *SerReadPort   = NULL;   /* Serial reply port */
struct IOExtSer      *SerWriteReq   = NULL;   /* Serial IORequest */
struct MsgPort       *SerWritePort  = NULL;   /* Serial reply port */
struct MsgPort       *MainPort  = NULL;   /* port to comunicate with mainProcess */

ULONG sigs = NULL;
const char AmiTCP_PortName[] = "AMITCP";

int addr_assign = 0, dst_assign = 0, dns_assign = 0, domainname_assign = 0;
char sana2configtext[1024], configparams[MAXPATHLEN];

int h_errno;
int dialing_try, dial_number;

char serial_in[10];
char serial_buffer[1024], keyboard_buffer[81];
char serial_buffer_old1[81];
char serial_buffer_old2[81];
WORD ser_buf_pos, key_buf_pos;
BOOL keyboard_input = FALSE;
BOOL use_loginscript, use_modem, no_picture, dialup = FALSE;

ULONG *colors[NUM_PAGES];
UBYTE *bodies[NUM_PAGES];

///
/// MainMenu
struct NewMenu MainMenu[] =
{
   { NM_TITLE  , MSG_MENU_PROJECT   , 0                  , 0, 0, (APTR)0               },
   { NM_ITEM   , MSG_MENU_ABOUT     , "  ?", 0, 0, (APTR)MEN_ABOUT       },
   { NM_ITEM   , NM_BARLABEL        , 0                  , 0, 0, 0                     },
   { NM_ITEM   , MSG_MENU_ABOUT_MUI , 0                  , 0, 0, (APTR)MEN_ABOUT_MUI   },
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
struct MUI_CustomClass  *CL_Request             = NULL;

struct MUI_CustomClass  *CL_Finished            = NULL;
struct MUI_CustomClass  *CL_Protocol            = NULL;
struct MUI_CustomClass  *CL_LoginScript         = NULL;
struct MUI_CustomClass  *CL_ModemStrings        = NULL;
struct MUI_CustomClass  *CL_Sana2               = NULL;
struct MUI_CustomClass  *CL_SanaConfig          = NULL;
struct MUI_CustomClass  *CL_SerialModem         = NULL;
struct MUI_CustomClass  *CL_SerialSana          = NULL;
struct MUI_CustomClass  *CL_UserInfo            = NULL;
struct MUI_CustomClass  *CL_Welcome             = NULL;

///
/// MUI stuff
Object   *app     = NULL;
Object   *win     = NULL;
Object   *status_win = NULL;

struct Hook deshook           = {{NULL, NULL}, (VOID *)desfunc          , NULL, NULL};
struct Hook sorthook          = {{NULL, NULL}, (VOID *)sortfunc         , NULL, NULL};
struct Hook strobjhook        = {{NULL, NULL}, (VOID *)strobjfunc       , NULL, NULL};
struct Hook txtobjhook        = {{NULL, NULL}, (VOID *)txtobjfunc       , NULL, NULL};
struct Hook objstrhook        = {{NULL, NULL}, (VOID *)objstrfunc       , NULL, NULL};
struct Hook objtxthook        = {{NULL, NULL}, (VOID *)objtxtfunc       , NULL, NULL};
///

