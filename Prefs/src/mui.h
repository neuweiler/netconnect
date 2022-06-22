#define MUISERIALNR_CONFIG 1
#define TAGBASE_CONFIG (TAG_USER | (MUISERIALNR_CONFIG << 16))

#define VVSpace MUI_NewObject(MUIC_Rectangle, MUIA_FixWidth, 1, TAG_DONE)

/// MUIM & MUIV
#define MUIM_MainWindow_Finish                  (TAGBASE_CONFIG | 0x1000)
#define MUIM_MainWindow_InitGroups              (TAGBASE_CONFIG | 0x1001)
#define MUIM_MainWindow_LoadConfig              (TAGBASE_CONFIG | 0x1002)
#define MUIM_MainWindow_SaveConfig              (TAGBASE_CONFIG | 0x1003)
#define MUIM_MainWindow_About                   (TAGBASE_CONFIG | 0x1004)
#define MUIM_MainWindow_AboutFinish             (TAGBASE_CONFIG | 0x1005)

#define MUIM_Provider_Reset                     (TAGBASE_CONFIG | 0x1010)
#define MUIM_Provider_Sana2Cycle                (TAGBASE_CONFIG | 0x1011)
#define MUIM_Provider_PopString_Close           (TAGBASE_CONFIG | 0x1012)

#define MUIM_Modem_PopString_Close              (TAGBASE_CONFIG | 0x1020)
#define MUIM_Modem_UpdateProtocolList           (TAGBASE_CONFIG | 0x1021)

#define MUIM_Dialer_PopString_AddPhone          (TAGBASE_CONFIG | 0x1030)
#define MUIM_Dialer_LineModified                (TAGBASE_CONFIG | 0x1031)
#define MUIM_Dialer_ScriptActive                (TAGBASE_CONFIG | 0x1032)

#define MUIM_User_InsertFile                    (TAGBASE_CONFIG | 0x1040)
#define MUIM_User_Popstring_UserName            (TAGBASE_CONFIG | 0x1041)

#define MUIM_Databases_SetStates                (TAGBASE_CONFIG | 0x1050)
#define MUIM_Databases_Modification             (TAGBASE_CONFIG | 0x1051)

#define MUIA_AmiTCP_Originator                  (TAGBASE_CONFIG | 0x1060)
#define MUIM_AmiTCP_Finish                      (TAGBASE_CONFIG | 0x1061)


#define MUIV_Modem_PopString_Modem        1
#define MUIV_Modem_PopString_Protocol     2

#define MUIV_Provider_PopString_IfaceName  1

#define MUIV_Databases_SetStates_Users             0
#define MUIV_Databases_SetStates_Groups            1
#define MUIV_Databases_SetStates_Protocols         2
#define MUIV_Databases_SetStates_Services          3
#define MUIV_Databases_SetStates_Inetd             4
#define MUIV_Databases_SetStates_Hosts             5
#define MUIV_Databases_SetStates_Networks          6
#define MUIV_Databases_SetStates_Rpcs              7

#define MUIV_Databases_Modification_NewUser           1
#define MUIV_Databases_Modification_Login             2
#define MUIV_Databases_Modification_FullName          3
#define MUIV_Databases_Modification_HomeDir           4
#define MUIV_Databases_Modification_Shell             5
#define MUIV_Databases_Modification_UserID            6
#define MUIV_Databases_Modification_GroupID           7
#define MUIV_Databases_Modification_Disable           8
#define MUIV_Databases_Modification_ChangePassword    9
#define MUIV_Databases_Modification_RemovePassword    10
#define MUIV_Databases_Modification_NewGroup          11
#define MUIV_Databases_Modification_GroupName         12
#define MUIV_Databases_Modification_GroupNumber       13
#define MUIV_Databases_Modification_GroupMembers      14
#define MUIV_Databases_Modification_ProtocolName      15
#define MUIV_Databases_Modification_ProtocolID        16
#define MUIV_Databases_Modification_ProtocolAliases   17
#define MUIV_Databases_Modification_ServiceName       18
#define MUIV_Databases_Modification_ServicePort       19
#define MUIV_Databases_Modification_ServiceAliases    20
#define MUIV_Databases_Modification_ServiceProtocol   21
#define MUIV_Databases_Modification_InetdService      22
#define MUIV_Databases_Modification_InetdUser         23
#define MUIV_Databases_Modification_InetdServer       24
#define MUIV_Databases_Modification_InetdArgs         25
#define MUIV_Databases_Modification_InetdActive       26
#define MUIV_Databases_Modification_InetdProtocol     27
#define MUIV_Databases_Modification_InetdSocket       28
#define MUIV_Databases_Modification_InetdWait         29
#define MUIV_Databases_Modification_HostAddr          30
#define MUIV_Databases_Modification_HostName          31
#define MUIV_Databases_Modification_HostAliases       32
#define MUIV_Databases_Modification_NetworkName       33
#define MUIV_Databases_Modification_NetworkID         34
#define MUIV_Databases_Modification_NetworkAliases    35
#define MUIV_Databases_Modification_RpcName           36
#define MUIV_Databases_Modification_RpcID             37
#define MUIV_Databases_Modification_RpcAliases        38
#define MUIV_Databases_Modification_NewProtocol       39
#define MUIV_Databases_Modification_NewService        40
#define MUIV_Databases_Modification_NewInetd          41
#define MUIV_Databases_Modification_NewHost           42
#define MUIV_Databases_Modification_NewNetwork        43
#define MUIV_Databases_Modification_NewRpc            44
#define MUIV_Databases_Modification_RemoveProtocol    45
#define MUIV_Databases_Modification_RemoveService     46
#define MUIV_Databases_Modification_RemoveInetd       47
#define MUIV_Databases_Modification_RemoveHost        48
#define MUIV_Databases_Modification_RemoveNetwork     49
#define MUIV_Databases_Modification_RemoveRpc         50

struct MUIP_MainWindow_Finish          { ULONG MethodID; LONG level; };
struct MUIP_MainWindow_AboutFinish     { ULONG MethodID; Object *window; };
struct MUIP_MainWindow_LoadConfig      { ULONG MethodID; STRPTR file; };
struct MUIP_MainWindow_SaveConfig      { ULONG MethodID; STRPTR file; };
struct MUIP_Modem_PopString_Close      { ULONG MethodID; LONG flags; };
struct MUIP_Provider_PopString_Close   { ULONG MethodID; LONG flags; };
struct MUIP_Dialer_PopString_AddPhone  { ULONG MethodID; LONG doit; };
struct MUIP_User_InsertFile            { ULONG MethodID; LONG stopnet; };
struct MUIP_Databases_SetStates        { ULONG MethodID; LONG page; };
struct MUIP_Databases_Modification     { ULONG MethodID; LONG what; };
struct MUIP_AmiTCP_Finish              { ULONG MethodID; Object *obj; LONG status; };

///

/// struct About_Data
struct About_Data
{
   Object *BT_Button;
};

///
/// struct MainWindow_Data
struct MainWindow_Data
{
   Object *MN_Strip;

   Object *GR_Pager;

   Object *BT_Save;
   Object *BT_Cancel;

   Object *GR_Info;
   Object *GR_Provider;
   Object *GR_User;
   Object *GR_Modem;
   Object *GR_Dialer;
   Object *GR_Databases;
};

///
/// struct Provider_Data
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

///
/// struct User_Data
struct User_Data
{
   Object *GR_User;
   Object *STR_LoginName;
   Object *PO_UserName;
   Object *STR_UserName;
   Object *LV_UserName;
   Object *CY_UserName;
   Object *STR_RealName;
   Object *STR_Password;
   Object *STR_EMail;
   Object *STR_Organisation;

   Object *GR_UserStartnet;
   Object *TI_UserStartnet;
   Object *BT_StartInsert;
   Object *BT_StartClear;

   Object *GR_UserStopnet;
   Object *TI_UserStopnet;
   Object *BT_StopInsert;
   Object *BT_StopClear;
};

///
/// struct Dialer_Data
struct Dialer_Data
{
   Object *GR_Script;
   Object *LV_Script;
   Object *LI_Script;
   Object *BT_Add;
   Object *BT_Remove;
   Object *PO_Line;
   Object *STR_Line;
   Object *LV_Lines;
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

///
/// struct Modem_Data
struct Modem_Data
{
   Object *GR_Modem;
   Object *PO_Modem;
   Object *LV_Modems;
   Object *STR_Modem;
   Object *PO_ModemInit;
   Object *LV_Protocols;
   Object *STR_ModemInit;
   Object *PO_DialPrefix;
   Object *STR_DialPrefix;
   Object *LV_DialPrefix;
   Object *STR_DialSuffix;

   Object *GR_Serial;
   Object *PO_SerialDriver;
   Object *STR_SerialDriver;
   Object *LV_Devices;
   Object *STR_SerialUnit;
   Object *PO_BaudRate;
   Object *STR_BaudRate;
   Object *LV_BaudRate;
   Object *STR_SerBufLen;
   Object *SL_RedialAttempts;
   Object *SL_RedialDelay;
   Object *CH_Carrier;
   Object *CH_7Wire;
};

///
/// struct Databases_Data
struct Databases_Data
{
   Object *GR_Passwd;
   Object *LV_Users;
   Object *LI_Users;
   Object *BT_NewUser;
   Object *BT_RemoveUser;
   Object *STR_Login;
   Object *BT_ChangePassword;
   Object *BT_RemovePassword;
   Object *CH_Disabled;
   Object *STR_Name;
   Object *STR_UserID;
   Object *STR_GroupID;
   Object *PA_HomeDir;
   Object *STR_HomeDir;
   Object *STR_Shell;

   Object *GR_Groups;
   Object *LV_Groups;
   Object *LI_Groups;
   Object *BT_NewGroup;
   Object *BT_RemoveGroup;
   Object *STR_GroupName;
   Object *STR_GroupNumber;
   Object *STR_GroupMembers;

   Object *GR_Protocols;
   Object *LV_Protocols;
   Object *LI_Protocols;
   Object *BT_NewProtocol;
   Object *BT_RemoveProtocol;
   Object *STR_ProtocolName;
   Object *STR_ProtocolID;
   Object *STR_ProtocolAliases;

   Object *GR_Services;
   Object *LV_Services;
   Object *LI_Services;
   Object *BT_NewService;
   Object *BT_RemoveService;
   Object *STR_ServiceName;
   Object *STR_ServicePort;
   Object *STR_ServiceAliases;
   Object *LV_ServiceProtocol;
   Object *LI_ServiceProtocol;

   Object *GR_Inetd;
   Object *LV_Inetd;
   Object *LI_Inetd;
   Object *BT_NewInetd;
   Object *BT_RemoveInetd;
   Object *STR_InetdService;
   Object *STR_InetdUser;
   Object *PA_InetdServer;
   Object *STR_InetdServer;
   Object *STR_InetdArgs;
   Object *LV_InetdProtocol;
   Object *LI_InetdProtocol;
   Object *CY_InetdSocket;
   Object *CY_InetdWait;
   Object *CH_InetdActive;

   Object *GR_Hosts;
   Object *LV_Hosts;
   Object *LI_Hosts;
   Object *BT_NewHost;
   Object *BT_RemoveHost;
   Object *STR_HostAddr;
   Object *STR_HostName;
   Object *STR_HostAliases;

   Object *GR_Networks;
   Object *LV_Networks;
   Object *LI_Networks;
   Object *BT_NewNetwork;
   Object *BT_RemoveNetwork;
   Object *STR_NetworkName;
   Object *STR_NetworkID;
   Object *STR_NetworkAliases;

   Object *GR_Rpc;
   Object *LV_Rpcs;
   Object *LI_Rpcs;
   Object *BT_NewRpc;
   Object *BT_RemoveRpc;
   Object *STR_RpcName;
   Object *STR_RpcID;
   Object *STR_RpcAliases;
};

///
/// struct PasswdReq_Data
struct PasswdReq_Data
{
   Object *STR_pw1;
   Object *STR_pw2;
   Object *BT_Okay;
   Object *BT_Cancel;
};

///

