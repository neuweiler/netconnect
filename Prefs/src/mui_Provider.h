#define MUIM_Provider_Reset                     (TAGBASE_PREFS | 0x1010)
#define MUIM_Provider_Sana2Cycle                (TAGBASE_PREFS | 0x1011)
#define MUIM_Provider_PopString_Close           (TAGBASE_PREFS | 0x1012)

#define MUIV_Provider_PopString_IfaceName  1

struct MUIP_Provider_PopString_Close   { ULONG MethodID; LONG flags; };

struct Provider_Data
{
   Object *GR_TCPIP;
   Object *STR_DomainName;
   Object *STR_NameServer1;
   Object *STR_NameServer2;
   Object *CY_Resolv;
   Object *STR_IP_Address;
   Object *CY_Address;
   Object *STR_HostName;

   Object *GR_Sana2;
   Object *CY_Sana2Device;
   Object *PA_Sana2Device;
   Object *STR_Sana2Device;
   Object *STR_Sana2Unit;
   Object *PA_Sana2ConfigFile;
   Object *STR_Sana2ConfigFile;
   Object *TI_Sana2ConfigContents;

   Object *GR_Interface;
   Object *PO_IfaceName;
   Object *STR_IfaceName;
   Object *LV_IfaceNames;
   Object *STR_IfaceConfigParams;
   Object *STR_MTU;
   Object *CY_Ping;
   Object *SL_Ping;
   Object *CH_BOOTP;

   Object *GR_Server;
   Object *STR_MailServer;
   Object *STR_POPServer;
   Object *STR_NewsServer;
   Object *STR_TimeServer;
   Object *STR_IRCServer;
   Object *STR_IRCPort;
   Object *STR_ProxyServer;
   Object *STR_ProxyPort;
};

