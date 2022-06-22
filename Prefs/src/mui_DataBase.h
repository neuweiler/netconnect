#define MUIM_Databases_SetStates                (TAGBASE_PREFS | 0x1050)
#define MUIM_Databases_Modification             (TAGBASE_PREFS | 0x1051)

#define MUIV_Databases_SetStates_Users             0
#define MUIV_Databases_SetStates_Groups            1
#define MUIV_Databases_SetStates_Protocols         2
#define MUIV_Databases_SetStates_Services          3
#define MUIV_Databases_SetStates_InetAccess        4
#define MUIV_Databases_SetStates_Inetd             5
#define MUIV_Databases_SetStates_Hosts             6
#define MUIV_Databases_SetStates_Networks          7
#define MUIV_Databases_SetStates_Rpcs              8

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
#define MUIV_Databases_Modification_InetAccessService 22
#define MUIV_Databases_Modification_InetAccessHost    23
#define MUIV_Databases_Modification_InetAccess        24
#define MUIV_Databases_Modification_InetAccessLog     25
#define MUIV_Databases_Modification_InetdService      26
#define MUIV_Databases_Modification_InetdUser         27
#define MUIV_Databases_Modification_InetdServer       28
#define MUIV_Databases_Modification_InetdArgs         29
#define MUIV_Databases_Modification_InetdActive       30
#define MUIV_Databases_Modification_InetdProtocol     31
#define MUIV_Databases_Modification_InetdSocket       32
#define MUIV_Databases_Modification_InetdWait         33
#define MUIV_Databases_Modification_HostAddr          34
#define MUIV_Databases_Modification_HostName          35
#define MUIV_Databases_Modification_HostAliases       36
#define MUIV_Databases_Modification_NetworkName       37
#define MUIV_Databases_Modification_NetworkID         38
#define MUIV_Databases_Modification_NetworkAliases    39
#define MUIV_Databases_Modification_RpcName           40
#define MUIV_Databases_Modification_RpcID             41
#define MUIV_Databases_Modification_RpcAliases        42
#define MUIV_Databases_Modification_NewProtocol       43
#define MUIV_Databases_Modification_NewService        44
#define MUIV_Databases_Modification_NewInetAccess     45
#define MUIV_Databases_Modification_NewInetd          46
#define MUIV_Databases_Modification_NewHost           47
#define MUIV_Databases_Modification_NewNetwork        48
#define MUIV_Databases_Modification_NewRpc            49

struct MUIP_Databases_SetStates        { ULONG MethodID; LONG page; };
struct MUIP_Databases_Modification     { ULONG MethodID; LONG what; };

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

   Object *GR_InetAccess;
   Object *LV_InetAccess;
   Object *LI_InetAccess;
   Object *BT_NewInetAccess;
   Object *BT_RemoveInetAccess;
   Object *STR_InetAccessService;
   Object *STR_InetAccessHost;
   Object *CY_InetAccess;
   Object *CH_InetAccessLog;

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

