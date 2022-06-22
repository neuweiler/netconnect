#define MUISERIALNR_GENESIS 1
#define TAGBASE_GENESIS (TAG_USER | (MUISERIALNR_GENESIS << 16))

#define VVSpace MUI_NewObject(MUIC_Rectangle, MUIA_FixWidth, 1, TAG_DONE)
enum { MEN_ABOUT=1, MEN_ABOUT_MUI, MEN_QUIT, MEN_MUI, MEN_GENESIS };
enum { ID_DOUBLESTART=40 };

#define MUIV_Background_Red      "2:cccccccc,60606060,60606060"
#define MUIV_Background_Orange   "2:cccccccc,a0a0a0a0,70707070"
#define MUIV_Background_Yellow   "2:cccccccc,cccccccc,70707070"
#define MUIV_Background_Green    "2:70707070,cccccccc,70707070"

/*
MainWindow: 1000
Online    : 1010
Genesis   : 1020
IfaceReq  : 1030
*/

/// MainMessage
struct MainMessage
{
   struct Message msg;

   Object *obj;
   LONG MethodID;
   APTR data1;
   APTR data2;
   APTR data3;
   ULONG result;
};

///
/// LogEntry
struct LogEntry
{
   char time[31];
   char type[21];
   char info[81];
};

///

/*** MUIM, MUIA, MUIV ***/
/// MainWindow
#define MUIM_MainWindow_About                  (TAGBASE_GENESIS | 0x1001)
#define MUIM_MainWindow_DisposeWindow          (TAGBASE_GENESIS | 0x1002)
#define MUIM_MainWindow_Trigger                (TAGBASE_GENESIS | 0x1003)
#define MUIM_MainWindow_Quit                   (TAGBASE_GENESIS | 0x1004)
#define MUIM_MainWindow_OnOffline              (TAGBASE_GENESIS | 0x1005)
#define MUIM_MainWindow_PutOnline              (TAGBASE_GENESIS | 0x1006)
#define MUIM_MainWindow_PutOffline             (TAGBASE_GENESIS | 0x1007)
#define MUIM_MainWindow_SetStates              (TAGBASE_GENESIS | 0x1008)
#define MUIM_MainWindow_TimeTrigger            (TAGBASE_GENESIS | 0x1009)
#define MUIM_MainWindow_LoadConfig             (TAGBASE_GENESIS | 0x100a)
#define MUIM_MainWindow_MUIRequest             (TAGBASE_GENESIS | 0x100b)
#define MUIM_MainWindow_UpdateLog              (TAGBASE_GENESIS | 0x100c)
#define MUIM_MainWindow_ChangeProvider         (TAGBASE_GENESIS | 0x100d)
#define MUIM_MainWindow_ChangeUser             (TAGBASE_GENESIS | 0x100e)
#define MUIM_MainWindow_GenesisPrefs           (TAGBASE_GENESIS | 0x100f)

///
/// Online
#define MUIM_Online_GoOnline                    (TAGBASE_GENESIS | 0x1010)
#define MUIM_Online_Abort                       (TAGBASE_GENESIS | 0x1011)

///
/// Genesis
#define MUIM_Genesis_Handshake                   (TAGBASE_GENESIS | 0x1020)
#define MUIM_Genesis_Get                         (TAGBASE_GENESIS | 0x1021)

///

/*** MUIP ***     ***** DO NOT USE BOOL *****/
/// MainWindow
struct MUIP_MainWindow_DisposeWindow      { ULONG MethodID; Object *window; };
struct MUIP_MainWindow_MUIRequest         { ULONG MethodID; STRPTR buttons; STRPTR message; };
struct MUIP_MainWindow_OnOffline          { ULONG MethodID; LONG online; };
struct MUIP_MainWindow_SendPing           { ULONG MethodID; STRPTR hostname; };
///
/// Online

///

/*** Object structures ***/
/// MainWindow
struct MainWindow_Data
{
   struct ISP isp;
   struct MUI_InputHandlerNode online_ihn;
   ULONG online;  /* timeval when timer started, also indicator if ihn is added or not !!! */
   struct timeval time;
   char time_str[21];
   LONG log_pos;
   int nr_leds;   /* how many led objects there are */

   Object *MN_Strip;

   Object *GR_Status;
   Object *GR_Led[32];
   Object *GR_Config;
   Object *PO_Provider;
   Object *BT_Provider;
   Object *TX_Provider;
   Object *LV_Providers;
   Object *LI_Providers;
   Object *BO_ProviderUser;
   Object *PO_User;
   Object *BT_User;
   Object *TX_User;
   Object *LV_Users;
   Object *LI_Users;
   Object *GR_Speed;
   Object *TX_Speed;
   Object *GR_Online;
   Object *TX_Online;
   Object *GR_Log;
   Object *LV_Log;
   Object *LI_Log;

   Object *GR_Buttons;
   Object *BT_Online;
   Object *BT_Offline;
};

///
/// Online
struct Online_Data
{
   struct Process *TCPHandler;
   BOOL abort;

   Object *TX_Info;
   Object *BU_Busy;
   Object *GR_Terminal;
   Object *TR_Terminal;
   Object *SB_Terminal;
   Object *BT_Abort;
};

///

