#define MUIM_MainWindow_About                  (TAGBASE_GENESIS | 0x1001)
#define MUIM_MainWindow_DisposeWindow          (TAGBASE_GENESIS | 0x1002)
#define MUIM_MainWindow_NetInfo                (TAGBASE_GENESIS | 0x1003)
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

struct MUIP_MainWindow_DisposeWindow      { ULONG MethodID; Object *window; };
struct MUIP_MainWindow_MUIRequest         { ULONG MethodID; STRPTR buttons; STRPTR message; };
struct MUIP_MainWindow_OnOffline          { ULONG MethodID; LONG online; };
struct MUIP_MainWindow_SendPing           { ULONG MethodID; STRPTR hostname; };
struct MUIP_MainWindow_ChangeProvider     { ULONG MethodID; STRPTR name; LONG do_online; };
struct MUIP_MainWindow_ChangeUser         { ULONG MethodID; STRPTR name; STRPTR password; };

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

   Object *GR_TimeLamps;
   Object *GR_Online;
   Object *TX_Online;
   Object *GR_Lamps;
   Object *GR_Status;
   Object *GR_Led[32];

   Object *GR_Speed;
   Object *TX_Speed;

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

   Object *GR_Log;
   Object *LV_Log;
   Object *LI_Log;

   Object *GR_Buttons;
   Object *BT_Online;
   Object *BT_Offline;
};

