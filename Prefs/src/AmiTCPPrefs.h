#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define MAXPATHLEN 256


#define MUISERIALNR_CONFIG 1
#define TAGBASE_CONFIG (TAG_USER | (MUISERIALNR_CONFIG << 16))

#define MUIM_AmiTCPPrefs_Finish			(TAGBASE_CONFIG | 0x1000)
#define MUIM_AmiTCPPrefs_InitGroups		(TAGBASE_CONFIG | 0x1001)
#define MUIM_AmiTCPPrefs_SetPage			(TAGBASE_CONFIG | 0x1002)
#define MUIM_AmiTCPPrefs_LoadConfig		(TAGBASE_CONFIG | 0x1003)
#define MUIM_AmiTCPPrefs_About			(TAGBASE_CONFIG | 0x1004)
#define MUIM_AmiTCPPrefs_About_Finish	(TAGBASE_CONFIG | 0x1005)
#define MUIM_Provider_PopString_Close	(TAGBASE_CONFIG | 0x1010)
#define MUIM_Provider_PopList_Update	(TAGBASE_CONFIG | 0x1011)
#define MUIM_Provider_DialScriptList_Active	(TAGBASE_CONFIG | 0x1012)
#define MUIM_Provider_ChangeLine			(TAGBASE_CONFIG | 0x1013)
#define MUIM_Provider_DialScriptPopString_Close	(TAGBASE_CONFIG | 0x1014)
#define MUIM_Provider_Interface_Active	(TAGBASE_CONFIG | 0x1015)
#define MUIM_Provider_Authentication_Active	(TAGBASE_CONFIG | 0x1016)
#define MUIM_User_UserStartnetList_Active	(TAGBASE_CONFIG | 0x1020)
#define MUIM_User_ChangeLine				(TAGBASE_CONFIG | 0x1021)
#define MUIM_User_ChangeDialScript		(TAGBASE_CONFIG | 0x1022)
#define MUIM_Modem_ModemList_Active		(TAGBASE_CONFIG | 0x1030)
#define MUIM_Modem_PopString_Close		(TAGBASE_CONFIG | 0x1031)
#define MUIM_Users_SetUserStates			(TAGBASE_CONFIG | 0x1040)
#define MUIM_Users_SetGroupStates		(TAGBASE_CONFIG | 0x1041)
#define MUIM_Users_Modification			(TAGBASE_CONFIG | 0x1042)
#define MUIM_InfoWindow_LoadFile			(TAGBASE_CONFIG | 0x1050)

#define MUIV_Provider_PopString_Country	1
#define MUIV_Provider_PopString_Provider	2
#define MUIV_Provider_PopString_PoP			3
#define MUIV_Modem_PopString_BaudRate		1
#define MUIV_Modem_PopString_DialPrefix	2
#define MUIV_Users_Modification_NewUser				1
#define MUIV_Users_Modification_RemoveUser			2
#define MUIV_Users_Modification_User					3
#define MUIV_Users_Modification_LoginName				4
#define MUIV_Users_Modification_HomeDir				5
#define MUIV_Users_Modification_Shell					6
#define MUIV_Users_Modification_UserID					7
#define MUIV_Users_Modification_GroupID				8
#define MUIV_Users_Modification_Disable				9
#define MUIV_Users_Modification_ChangePassword		10
#define MUIV_Users_Modification_RemovePassword		11
#define MUIV_Users_Modification_NewGroup				12
#define MUIV_Users_Modification_RemoveGroup			13
#define MUIV_Users_Modification_Group					14
#define MUIV_Users_Modification_GroupNumber			15
#define MUIV_Users_Modification_GroupMembers			16
#define MUIV_Users_Modification_RemoveGroupMember	17


struct MUIP_AmiTCPPrefs_Finish			{ ULONG MethodID; LONG level; };
struct MUIP_AmiTCPPrefs_LoadConfig		{ ULONG MethodID; STRPTR path; };
struct MUIP_AmiTCPPrefs_About_Finish	{ ULONG MethodID; Object *window; };
struct MUIP_Provider_PopString_Close	{ ULONG MethodID; LONG flags; };
struct MUIP_Provider_PopList_Update		{ ULONG MethodID; STRPTR path; LONG flags; };
struct MUIP_Modem_PopString_Close		{ ULONG MethodID; LONG flags; };
struct MUIP_Users_Modification			{ ULONG MethodID; LONG flags; };
struct MUIP_InfoWindow_LoadFile			{ ULONG MethodID; STRPTR file; };

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

	Object *GR_Temp;
	Object *GR_Info;
	Object *GR_Provider;
	Object *GR_User;
	Object *GR_Modem;
	Object *GR_Paths;
	Object *GR_Users;
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

	Object *RA_Connection;
	Object *RA_Interface;
	Object *CH_BOOTP;
	Object *SL_MTU;
	Object *CY_Authentication;
	Object *CY_Header;
	Object *STR_HostID;
	Object *STR_YourID;
	Object *STR_Password;

	Object *STR_NameServer1;
	Object *STR_NameServer2;
	Object *STR_DomainName;
	Object *STR_MailServer;
	Object *STR_POPServer;
	Object *STR_NewsServer;
	Object *STR_WWWServer;
	Object *STR_FTPServer;

	Object *LV_DialScript;
	Object *LI_DialScript;
	Object *PO_Line;
	Object *STR_Line;
	Object *LV_Line;
	Object *BT_New;
	Object *BT_Delete;
	Object *BT_Clear;
};

struct User_Data
{
	Object *GR_Register;

	Object *STR_LoginName;
	Object *STR_RealName;
	Object *STR_Password;
	Object *STR_EMail;
	Object *STR_Organisation;
	Object *STR_HostName;
	Object *STR_IP_Address;

	Object *LV_UserStartnet;
	Object *LI_UserStartnet;
	Object *STR_Line;
	Object *BT_New;
	Object *BT_Delete;
	Object *BT_Clear;
};

struct Modem_Data
{
	Object *GR_Register;

	Object *LV_Modem;
	Object *LI_Modem;
	Object *TX_Modem;
	Object *STR_ModemInit;
	Object *PO_DialPrefix;
	Object *STR_DialPrefix;
	Object *LV_DialPrefix;
	Object *STR_DialSuffix;

	Object *PA_SerialDriver;
	Object *STR_SerialDriver;
	Object *STR_SerialUnit;
	Object *PO_BaudRate;
	Object *STR_BaudRate;
	Object *LV_BaudRate;
	Object *SL_RedialAttempts;
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

struct Users_Data
{
	Object *LV_User;
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
	Object *BT_NewGroup;
	Object *BT_RemoveGroup;
	Object *STR_Group;
	Object *STR_GroupNumber;
	Object *LV_GroupMembers;
	Object *BT_RemoveGroupMember;
};

struct InfoWindow_Data
{
	Object *LV_Info;
	Object *LI_Info;
};



struct pc_Data
{
	STRPTR Buffer;		/* buffer holding the file (internal use only) */
	LONG Size;			/* variable holding the size of the buffer (internal use only) */
	STRPTR Current;	/* pointer to the current position (internal use only) */

	STRPTR Argument;	/* pointer to the argument */
	STRPTR Contents;	/* pointer to its contents */
};


struct Modem
{
	char Name[80];
	char InitString[80];
};

struct User
{
	char Login[41];
	char Password[41];
	LONG UserID;
	LONG GroupID;
	char Name[81];
	char HomeDir[MAXPATHLEN];
	char Shell[81];
	BOOL Disabled;			// I can't overwrite the password immediately when "disable" is selected because it might get deselected again
};

struct Group
{
	char Name[41];
	char Password[21];
	LONG ID;
	char Members[401];
};

struct InfoLine
{
	char Label[41];
	char Contents[81];
};

struct PoP
{
	char Name[81];
	char Phone[81];
};
