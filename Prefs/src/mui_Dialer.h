#define MUIM_Dialer_PopString_AddPhone          (TAGBASE_PREFS | 0x1030)
#define MUIM_Dialer_LineModified                (TAGBASE_PREFS | 0x1031)
#define MUIM_Dialer_ScriptActive                (TAGBASE_PREFS | 0x1032)

struct MUIP_Dialer_PopString_AddPhone  { ULONG MethodID; LONG doit; };

struct Dialer_Data
{
   Object *GR_Script;
   Object *LV_Script;
   Object *LI_Script;
   Object *BT_Add;
   Object *BT_Remove;
   Object *CY_Command;
   Object *STR_String;
   Object *PO_Phone;
   Object *STR_Phone;
   Object *STR_AddPhone;
   Object *BT_AddPhone;
   Object *BT_CancelPhone;

   Object *GR_Events;
   Object *PA_Online;
   Object *STR_Online;
   Object *CY_WinOnline;
   Object *PA_OnlineFailed;
   Object *STR_OnlineFailed;
   Object *CY_WinOnlineFailed;
   Object *PA_OfflineActive;
   Object *STR_OfflineActive;
   Object *CY_WinOfflineActive;
   Object *PA_OfflinePassive;
   Object *STR_OfflinePassive;
   Object *CY_WinOfflinePassive;
   Object *PA_Startup;
   Object *STR_Startup;
   Object *CY_WinStartup;
   Object *PA_Shutdown;
   Object *STR_Shutdown;

   Object *GR_Misc;
   Object *CY_AutoLogin;
   Object *CH_GoOnline;
   Object *CH_QuickReconnect;
   Object *CH_SynClock;
   Object *CH_ShowStatus;
   Object *CH_ShowSpeed;
   Object *CH_ShowOnline;
   Object *CH_ShowButtons;

};

