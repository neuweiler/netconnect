#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define MAXPATHLEN 256


#define NUM_STOPS (sizeof(Stops) / (2 * sizeof(ULONG)))


#define MUISERIALNR_CONFIG 1
#define TAGBASE_CONFIG (TAG_USER | (MUISERIALNR_CONFIG << 16))

#define MUIM_AmiTCPPrefs_Finish				(TAGBASE_CONFIG | 0x1000)
#define MUIM_AmiTCPPrefs_LoadConfig			(TAGBASE_CONFIG | 0x1001)
#define MUIM_ServerPrefs_PopString_Close	(TAGBASE_CONFIG | 0x1010)
#define MUIM_ServerPrefs_PopList_Update	(TAGBASE_CONFIG | 0x1011)


#define MUIV_ServerPrefs_PopString_Country	1
#define MUIV_ServerPrefs_PopString_Provider	2
#define MUIV_ServerPrefs_PopString_PoP			3


struct MUIP_AmiTCPPrefs_Finish				{ ULONG MethodID; LONG level; };
struct MUIP_AmiTCPPrefs_LoadConfig			{ ULONG MethodID; STRPTR file; };
struct MUIP_ServerPrefs_PopString_Close	{ ULONG MethodID; LONG flags; };
struct MUIP_ServerPrefs_PopList_Update		{ ULONG MethodID; STRPTR path; LONG flags; };


struct AmiTCPPrefs_Data
{
	Object *GR_Register;
	Object *GR_User;
	Object *GR_Server;
	Object *GR_Interface;
	Object *GR_Misc;
	Object *GR_DialScript;
	Object *GR_UserStartnet;

	Object *BT_Save;
	Object *BT_Use;
	Object *BT_Cancel;
};

struct UserPrefs_Data
{
	Object *RA_Connection;
	Object *RA_Interface;
	Object *CH_BOOTP;

	Object *STR_UserName;
	Object *STR_NodeName;
	Object *STR_RealName;
	Object *STR_Organisation;
	Object *STR_IP_Address;
	Object *STR_Password;
};

struct ServerPrefs_Data
{
	Object *PO_Country;
	Object *LV_Country;
	Object *PO_Provider;
	Object *LV_Provider;
	Object *PO_PoP;
	Object *LV_PoP;
	Object *STR_Phone;
	Object *STR_NameServer1;
	Object *STR_MailServer;
	Object *STR_NameServer2;
	Object *STR_NewsServer;
	Object *STR_Netmask;
	Object *STR_IRCServer;
	Object *STR_Gateway;
	Object *STR_DomainName;
};

struct InterfacePrefs_Data
{
	Object *CH_Carrier;
	Object *CH_7Wire;
	Object *CH_OwnDevUnit;
	Object *CH_Shared;
	Object *CY_Compression;
	Object *SL_MTU;
	Object *STR_Options;
};

struct MiscPrefs_Data
{
	Object *PA_MailIn;
	Object *PA_MailOut;
	Object *PA_FileIn;
	Object *PA_FileOut;

	Object *PA_SerialDriver;
	Object *STR_ModemInit;
	Object *STR_SerialUnit;
	Object *PO_DialPrefix;
	Object *LV_DialPrefix;
	Object *PO_BaudRate;
	Object *LV_BaudRate;
};

struct DialscriptPrefs_Data
{
	Object *GR_ScriptEditor;
};

struct UserStartnetPrefs_Data
{
	Object *GR_Editor;
};


struct pc_Data
{
	STRPTR Buffer;		/* buffer holding the file (internal use only) */
	LONG Size;			/* variable holding the size of the buffer (internal use only) */
	STRPTR Current;	/* pointer to the current position (internal use only) */

	STRPTR Argument;	/* pointer to the argument */
	STRPTR Contents;	/* pointer to its contents */
};
