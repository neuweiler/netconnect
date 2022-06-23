#define MUIM_IfaceWindow_CopyData              (TAGBASE_PREFS | 0x1040)
#define MUIM_IfaceWindow_Init                  (TAGBASE_PREFS | 0x1041)
#define MUIM_IfaceWindow_PopString_Close       (TAGBASE_PREFS | 0x1042)
#define MUIM_IfaceWindow_SanaActive            (TAGBASE_PREFS | 0x1043)
#define MUIM_IfaceWindow_SetEventStates        (TAGBASE_PREFS | 0x1044)
#define MUIM_IfaceWindow_Modification          (TAGBASE_PREFS | 0x1045)
#define MUIM_IfaceWindow_EventActive           (TAGBASE_PREFS | 0x1046)
#define MUIM_IfaceWindow_ModemActive           (TAGBASE_PREFS | 0x1047)
#define MUIM_IfaceWindow_LoginScriptActive     (TAGBASE_PREFS | 0x1048)
#define MUIM_IfaceWindow_PopString_AddPhone    (TAGBASE_PREFS | 0x1049)
#define MUIM_IfaceWindow_NameserversActive     (TAGBASE_PREFS | 0x104a)
#define MUIM_IfaceWindow_DomainnamesActive     (TAGBASE_PREFS | 0x104b)

#define MUIV_IfaceWindow_PopString_IfaceName     1

#define MUIV_IfaceWindow_Modification_AddEvent      1
#define MUIV_IfaceWindow_Modification_RemoveEvent   2
#define MUIV_IfaceWindow_Modification_Event         3
#define MUIV_IfaceWindow_Modification_ExecType      4
#define MUIV_IfaceWindow_Modification_AddScriptLine 5
#define MUIV_IfaceWindow_Modification_ScriptLine    6
#define MUIV_IfaceWindow_Modification_NewModem      7
#define MUIV_IfaceWindow_Modification_EditModem     8
#define MUIV_IfaceWindow_Modification_DeleteModem   9
#define MUIV_IfaceWindow_Modification_AddNameServer 10
#define MUIV_IfaceWindow_Modification_NameServer    11
#define MUIV_IfaceWindow_Modification_AddDomainName 12
#define MUIV_IfaceWindow_Modification_DomainName    13

struct MUIP_IfaceWindow_Init                { ULONG MethodID; struct Interface *iface; };
struct MUIP_IfaceWindow_PopString_Close     { ULONG MethodID; LONG flags; };
struct MUIP_IfaceWindow_Modification        { ULONG MethodID; LONG what; };
struct MUIP_IfaceWindow_PopString_AddPhone  { ULONG MethodID; LONG doit; };

struct IfaceWindow_Data
{
   struct Interface *iface;

   Object *CY_Sana2Device;
   Object *PA_Sana2Device;
   Object *STR_Sana2Device;
   Object *STR_Sana2Unit;
   Object *PA_Sana2ConfigFile;
   Object *STR_Sana2ConfigFile;
   Object *TI_Sana2ConfigText;

   Object *PO_Name;
   Object *STR_Name;
   Object *LV_IfaceNames;
   Object *STR_Comment;
   Object *STR_ConfigParams;
   Object *STR_MTU;
   Object *STR_Address;
   Object *CY_Address;
   Object *STR_Netmask;
   Object *CY_Netmask;
   Object *STR_Dest;
   Object *CY_Dest;
   Object *STR_Gateway;
   Object *CY_Gateway;

   Object *LV_Modems;
   Object *LI_Modems;
   Object *BT_NewModem;
   Object *BT_DeleteModem;
   Object *BT_EditModem;
   Object *LV_LoginScript;
   Object *LI_LoginScript;
   Object *BT_AddScriptLine;
   Object *BT_RemoveScriptLine;
   Object *CY_ScriptCommand;
   Object *STR_ScriptContents;
   Object *PO_Phone;
   Object *STR_Phone;
   Object *STR_AddPhone;
   Object *BT_AddPhone;
   Object *BT_CancelPhone;
   Object *STR_Login;
   Object *STR_Password;

   Object *CH_UseBOOTP;
   Object *SL_KeepAlive;
   Object *CH_DialIn;
   Object *CH_AutoOnline;
   Object *CH_UseDNS;
   Object *CH_UseDomainName;
   Object *CH_UseHostName;
   Object *CH_GetTime;
   Object *CH_SaveTime;
   Object *STR_TimeServer;

   Object *CY_EventPage;
   Object *GR_EventPages;
   Object *LV_Online;
   Object *LI_Online;
   Object *LV_OnlineFail;
   Object *LI_OnlineFail;
   Object *LV_OfflineActive;
   Object *LI_OfflineActive;
   Object *LV_OfflinePassive;
   Object *LI_OfflinePassive;
   Object *LI_Active;
   Object *BT_AddEvent;
   Object *BT_RemoveEvent;
   Object *PA_Event;
   Object *STR_Event;
   Object *CY_ExecType;

   Object *CH_CarrierDetect;
   Object *SL_ConnectTimeout;
   Object *STR_Callback;
   Object *CH_Callback;
   Object *CH_MPPCompression;
   Object *CH_VJCompression;
   Object *CH_BSDCompression;
   Object *CH_DeflateCompression;
   Object *CY_EOF;

   Object *LV_NameServers;
   Object *LI_NameServers;
   Object *BT_AddNameServer;
   Object *BT_RemoveNameServer;
   Object *STR_NameServer;
   Object *LV_DomainNames;
   Object *LI_DomainNames;
   Object *BT_AddDomainName;
   Object *BT_RemoveDomainName;
   Object *STR_DomainName;
   Object *STR_HostName;

   Object *BT_Okay;
   Object *BT_Cancel;
};

