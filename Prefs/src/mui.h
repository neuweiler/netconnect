#define MUISERIALNR_CONFIG 1
#define TAGBASE_CONFIG (TAG_USER | (MUISERIALNR_CONFIG << 16))

#define MUIM_AmiTCPPrefs_Finish						(TAGBASE_CONFIG | 0x1000)
#define MUIM_AmiTCPPrefs_InitGroups					(TAGBASE_CONFIG | 0x1001)
#define MUIM_AmiTCPPrefs_SetPage						(TAGBASE_CONFIG | 0x1002)
#define MUIM_AmiTCPPrefs_ImportProvider			(TAGBASE_CONFIG | 0x1003)
#define MUIM_AmiTCPPrefs_ExportProvider			(TAGBASE_CONFIG | 0x1004)
#define MUIM_AmiTCPPrefs_LoadProvider				(TAGBASE_CONFIG | 0x1005)
#define MUIM_AmiTCPPrefs_LoadPrefs					(TAGBASE_CONFIG | 0x1006)
#define MUIM_AmiTCPPrefs_About						(TAGBASE_CONFIG | 0x1007)
#define MUIM_AmiTCPPrefs_About_Finish				(TAGBASE_CONFIG | 0x1008)
#define MUIM_AmiTCPPrefs_Expert						(TAGBASE_CONFIG | 0x1009)

#define MUIM_Provider_PopString_Close				(TAGBASE_CONFIG | 0x1010)
#define MUIM_Provider_PopList_Update				(TAGBASE_CONFIG | 0x1011)
#define MUIM_Provider_ChangeAction					(TAGBASE_CONFIG | 0x1012)
#define MUIM_Provider_Interface_Active				(TAGBASE_CONFIG | 0x1013)
#define MUIM_Provider_Authentication_Active		(TAGBASE_CONFIG | 0x1014)
#define MUIM_Provider_Reset							(TAGBASE_CONFIG | 0x1015)

#define MUIM_User_UserStartnetList_Active			(TAGBASE_CONFIG | 0x1020)
#define MUIM_User_ChangeLine							(TAGBASE_CONFIG | 0x1021)
#define MUIM_User_UserStopnetList_Active			(TAGBASE_CONFIG | 0x1022)
#define MUIM_User_StopnetChangeLine					(TAGBASE_CONFIG | 0x1023)

#define MUIM_Modem_PopString_Close					(TAGBASE_CONFIG | 0x1030)

#define MUIM_Users_SetUserStates						(TAGBASE_CONFIG | 0x1040)
#define MUIM_Users_SetGroupStates					(TAGBASE_CONFIG | 0x1041)
#define MUIM_Users_GetGroupStates					(TAGBASE_CONFIG | 0x1042)
#define MUIM_Users_Modification						(TAGBASE_CONFIG | 0x1043)

#define MUIM_Databases_SetStates						(TAGBASE_CONFIG | 0x1060)
#define MUIM_Databases_Modification					(TAGBASE_CONFIG | 0x1061)

#define MUIV_Provider_PopString_Country	1
#define MUIV_Provider_PopString_Provider	2
#define MUIV_Provider_PopString_PoP			3

#define MUIV_Modem_PopString_BaudRate		1
#define MUIV_Modem_PopString_DialPrefix	2
#define MUIV_Modem_PopString_Device			3
#define MUIV_Modem_PopString_Modem			4

#define MUIV_Users_Modification_NewUser				1
#define MUIV_Users_Modification_User					2
#define MUIV_Users_Modification_FullName				3
#define MUIV_Users_Modification_HomeDir				4
#define MUIV_Users_Modification_Shell					5
#define MUIV_Users_Modification_UserID					6
#define MUIV_Users_Modification_GroupID				7
#define MUIV_Users_Modification_Disable				8
#define MUIV_Users_Modification_ChangePassword		9
#define MUIV_Users_Modification_RemovePassword		10
#define MUIV_Users_Modification_NewGroup				11
#define MUIV_Users_Modification_Group					12
#define MUIV_Users_Modification_GroupNumber			13
#define MUIV_Users_Modification_RemoveGroupMember	14
#define MUIV_Users_Modification_MembersActive		15

#define MUIV_Databases_SetStates_Protocols			0
#define MUIV_Databases_SetStates_Services				1
#define MUIV_Databases_SetStates_Inetd					2
#define MUIV_Databases_SetStates_Hosts					3	
#define MUIV_Databases_SetStates_Networks				4
#define MUIV_Databases_SetStates_Rpcs					5

#define MUIV_Databases_Modification_ProtocolName		0
#define MUIV_Databases_Modification_ProtocolID			2
#define MUIV_Databases_Modification_ProtocolAliases	3
#define MUIV_Databases_Modification_ServiceName			4
#define MUIV_Databases_Modification_ServicePort			5
#define MUIV_Databases_Modification_ServiceAliases		6
#define MUIV_Databases_Modification_ServiceProtocol	7
#define MUIV_Databases_Modification_InetdService		8
#define MUIV_Databases_Modification_InetdUser			9
#define MUIV_Databases_Modification_InetdServer			10
#define MUIV_Databases_Modification_InetdArgs			11
#define MUIV_Databases_Modification_InetdActive			12
#define MUIV_Databases_Modification_InetdProtocol		13
#define MUIV_Databases_Modification_InetdSocket			14
#define MUIV_Databases_Modification_InetdWait			15
#define MUIV_Databases_Modification_HostAddr				16
#define MUIV_Databases_Modification_HostName				17
#define MUIV_Databases_Modification_HostAliases			18
#define MUIV_Databases_Modification_NetworkName			19
#define MUIV_Databases_Modification_NetworkID			20
#define MUIV_Databases_Modification_NetworkAliases		21
#define MUIV_Databases_Modification_RpcName				22
#define MUIV_Databases_Modification_RpcID					23
#define MUIV_Databases_Modification_RpcAliases			24
#define MUIV_Databases_Modification_NewProtocol			25
#define MUIV_Databases_Modification_NewService			26
#define MUIV_Databases_Modification_NewInetd				27
#define MUIV_Databases_Modification_NewHost				28
#define MUIV_Databases_Modification_NewNetwork			29
#define MUIV_Databases_Modification_NewRpc				30
#define MUIV_Databases_Modification_RemoveProtocol		31
#define MUIV_Databases_Modification_RemoveService		32
#define MUIV_Databases_Modification_RemoveInetd			33
#define MUIV_Databases_Modification_RemoveHost			34
#define MUIV_Databases_Modification_RemoveNetwork		35
#define MUIV_Databases_Modification_RemoveRpc			36

struct MUIP_AmiTCPPrefs_Finish			{ ULONG MethodID; LONG level; };
struct MUIP_AmiTCPPrefs_LoadProvider	{ ULONG MethodID; STRPTR file; STRPTR pop; };
struct MUIP_AmiTCPPrefs_About_Finish	{ ULONG MethodID; Object *window; LONG level;};
struct MUIP_Provider_PopString_Close	{ ULONG MethodID; LONG flags; };
struct MUIP_Provider_PopList_Update		{ ULONG MethodID; STRPTR path; LONG flags; };
struct MUIP_Provider_ChangeAction		{ ULONG MethodID; LONG which; };
struct MUIP_Modem_PopString_Close		{ ULONG MethodID; LONG flags; };
struct MUIP_Users_Modification			{ ULONG MethodID; LONG flags; };
struct MUIP_Databases_SetStates			{ ULONG MethodID; LONG page; };
struct MUIP_Databases_Modification		{ ULONG MethodID; LONG what; };


struct About_Data
{
	Object *BT_Button;
};

struct AmiTCPPrefs_Data
{
	Object *MN_Strip;

	Object *GR_Pager;
	Object *LV_Pager;
	Object *LI_Pager;
	Object *GR_Active;

	Object *BT_Save;
	Object *BT_Use;
	Object *BT_Cancel;

	Object *GR_Info;
	Object *GR_Provider;
	Object *GR_User;
	Object *GR_Modem;
	Object *GR_Paths;
	Object *GR_Users;
	Object *GR_Databases;
	Object *GR_Events;
	Object *GR_InfoWindow;
};

struct Provider_Data
{
	Object *GR_Register;

	Object *PO_Country;
	Object *TX_Country;
	Object *LV_Country;
	Object *PO_Provider;
	Object *TX_Provider;
	Object *LV_Provider;
	Object *PO_PoP;
	Object *TX_PoP;
	Object *LV_PoP;
	Object *STR_Phone;
	Object *CH_ProviderInfo;

	Object *CY_Address;
	Object *CY_Interface;
	Object *CH_BOOTP;
	Object *SL_MTU;
	Object *CY_Authentication;
	Object *CY_Header;

	Object *STR_HostName;
	Object *STR_IP_Address;
	Object *STR_NameServer1;
	Object *STR_NameServer2;
	Object *STR_DomainName;
	Object *STR_MailServer;
	Object *STR_POPServer;
	Object *STR_NewsServer;
	Object *STR_WWWServer;
	Object *STR_FTPServer;
	Object *STR_IRCServer;
	Object *STR_IRCPort;
	Object *STR_ProxyServer;
	Object *STR_ProxyPort;
	Object *STR_TimeServer;

	Object *GR_LoginScript;
	Object *CY_Action[8];
	Object *STR_Line[8];
	Object *CH_CR[8];
};

struct User_Data
{
	Object *GR_Register;

	Object *STR_LoginName;
	Object *STR_RealName;
	Object *STR_Password;
	Object *STR_EMail;
	Object *STR_Organisation;

	Object *GR_UserStartnet;
	Object *LV_UserStartnet;
	Object *LI_UserStartnet;
	Object *STR_Line;
	Object *BT_New;
	Object *BT_Remove;

	Object *GR_UserStopnet;
	Object *LV_UserStopnet;
	Object *LI_UserStopnet;
	Object *STR_StopnetLine;
	Object *BT_StopnetNew;
	Object *BT_StopnetRemove;
};

struct Modem_Data
{
	Object *GR_Register;

	Object *LV_Modem;
	Object *PO_Modem;
	Object *TX_Modem;
	Object *PO_DialPrefix;
	Object *STR_DialPrefix;
	Object *LV_DialPrefix;
	Object *STR_ModemInit;

	Object *PO_SerialDriver;
	Object *STR_SerialDriver;
	Object *LV_Devices;
	Object *STR_SerialUnit;
	Object *PO_BaudRate;
	Object *STR_BaudRate;
	Object *LV_BaudRate;
	Object *SL_RedialAttempts;
	Object *SL_RedialDelay;
	Object *CH_Carrier;
	Object *CH_7Wire;
	Object *CH_OwnDevUnit;
};

struct Paths_Data
{
	Object *PA_MailIn;
	Object *STR_MailIn;
	Object *PA_MailOut;
	Object *STR_MailOut;
	Object *PA_NewsIn;
	Object *STR_NewsIn;
	Object *PA_NewsOut;
	Object *STR_NewsOut;
	Object *PA_FileIn;
	Object *STR_FileIn;
	Object *PA_FileOut;
	Object *STR_FileOut;
};

struct MemberList_Data
{
	Object *Originator;
};

struct Users_Data
{
	Object *LV_User;
	Object *LI_User;
	Object *STR_User;
	Object *BT_NewUser;
	Object *BT_RemoveUser;
	Object *BT_ChangePassword;
	Object *BT_RemovePassword;
	Object *CH_Disabled;
	Object *STR_UserID;
	Object *STR_GroupID;
	Object *STR_Name;
	Object *PA_HomeDir;
	Object *STR_Shell;

	Object *LV_Groups;
	Object *LI_Groups;
	Object *BT_NewGroup;
	Object *BT_RemoveGroup;
	Object *STR_Group;
	Object *STR_GroupNumber;
	Object *LV_GroupMembers;
	Object *LI_GroupMembers;
	Object *BT_RemoveGroupMember;
};

struct InfoWindow_Data
{
	Object *LV_Info;
	Object *LI_Info;
};

struct Events_Data
{
	ULONG dummy;
};

struct Databases_Data
{
	Object *GR_Register;

	Object *LV_Protocols;
	Object *LI_Protocols;
	Object *BT_NewProtocol;
	Object *BT_RemoveProtocol;
	Object *STR_ProtocolName;
	Object *STR_ProtocolID;
	Object *STR_ProtocolAliases;

	Object *LV_Services;
	Object *LI_Services;
	Object *BT_NewService;
	Object *BT_RemoveService;
	Object *STR_ServiceName;
	Object *STR_ServicePort;
	Object *STR_ServiceAliases;
	Object *LV_ServiceProtocol;
	Object *LI_ServiceProtocol;

	Object *LV_Inetd;
	Object *LI_Inetd;
	Object *BT_NewInetd;
	Object *BT_RemoveInetd;
	Object *STR_InetdService;
	Object *STR_InetdUser;
	Object *PA_InetdServer;
	Object *STR_InetdArgs;
	Object *LV_InetdProtocol;
	Object *LI_InetdProtocol;
	Object *CY_InetdSocket;
	Object *CY_InetdWait;
	Object *CH_InetdActive;

	Object *LV_Hosts;
	Object *LI_Hosts;
	Object *BT_NewHost;
	Object *BT_RemoveHost;
	Object *STR_HostAddr;
	Object *STR_HostName;
	Object *STR_HostAliases;

	Object *LV_Networks;
	Object *LI_Networks;
	Object *BT_NewNetwork;
	Object *BT_RemoveNetwork;
	Object *STR_NetworkName;
	Object *STR_NetworkID;
	Object *STR_NetworkAliases;

	Object *LV_Rpcs;
	Object *LI_Rpcs;
	Object *BT_NewRpc;
	Object *BT_RemoveRpc;
	Object *STR_RpcName;
	Object *STR_RpcID;
	Object *STR_RpcAliases;

	Object *TX_Info;
};

struct PagerList_Data
{
	Object *o_information;
	Object *o_provider;
	Object *o_user;
	Object *o_modem;
	Object *o_groups;
	Object *o_databases;
	Object *o_paths;

	Object *i_information;
	Object *i_provider;
	Object *i_user;
	Object *i_modem;
	Object *i_groups;
	Object *i_databases;
	Object *i_paths;

	STRPTR information;
	STRPTR provider;
	STRPTR user;
	STRPTR modem;
	STRPTR groups;
	STRPTR databases;
	STRPTR paths;

	struct Hook DisplayHook;
};
