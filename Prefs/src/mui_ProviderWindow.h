#define MUIM_ProviderWindow_Init                      (TAGBASE_PREFS | 0x1020)
#define MUIM_ProviderWindow_CopyData                  (TAGBASE_PREFS | 0x1021)
#define MUIM_ProviderWindow_PopString_AddPhone        (TAGBASE_PREFS | 0x1022)
#define MUIM_ProviderWindow_Modification              (TAGBASE_PREFS | 0x1023)
#define MUIM_ProviderWindow_NameserversActive         (TAGBASE_PREFS | 0x1024)
#define MUIM_ProviderWindow_DomainnamesActive         (TAGBASE_PREFS | 0x1025)
#define MUIM_ProviderWindow_ScriptActive              (TAGBASE_PREFS | 0x1026)
#define MUIM_ProviderWindow_IfacesActive              (TAGBASE_PREFS | 0x1027)
#define MUIM_ProviderWindow_EditIface                 (TAGBASE_PREFS | 0x1028)
#define MUIM_ProviderWindow_EditIfaceFinish           (TAGBASE_PREFS | 0x1029)

#define MUIV_ProviderWindow_Modification_AddInterface    1
#define MUIV_ProviderWindow_Modification_AddLine         2
#define MUIV_ProviderWindow_Modification_ScriptLine      3
#define MUIV_ProviderWindow_Modification_AddNameServer   4
#define MUIV_ProviderWindow_Modification_NameServer      5
#define MUIV_ProviderWindow_Modification_AddDomainName   6
#define MUIV_ProviderWindow_Modification_DomainName      7

struct MUIP_ProviderWindow_PopString_AddPhone  { ULONG MethodID; LONG doit; };
struct MUIP_ProviderWindow_Init                { ULONG MethodID; struct ISP *isp; };
struct MUIP_ProviderWindow_Modification        { ULONG MethodID; LONG what; };
struct MUIP_ProviderWindow_EditIfaceFinish     { ULONG MethodID; Object *win; LONG ok; };

struct ProviderWindow_Data
{
   struct ISP *isp;

   Object *GR_ISP;
   Object *STR_Name;
   Object *STR_Comment;
   Object *STR_Login;
   Object *STR_Password;

   Object *GR_TCPIP;
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
   Object *CY_Resolv;
   Object *STR_HostName;
   Object *CY_HostName;
   Object *CH_DontQueryHostname;

   Object *GR_Interface;
   Object *LV_Interfaces;
   Object *LI_Interfaces;
   Object *BT_AddIface;
   Object *BT_DeleteIface;
   Object *BT_EditIface;

   Object *GR_Server;
   Object *CH_GetTime;
   Object *CH_SaveTime;
   Object *STR_TimeServer;
   Object *CH_BOOTP;
   Object *STR_BootpServer;

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

   Object *BT_Okay;
   Object *BT_Cancel;
};

