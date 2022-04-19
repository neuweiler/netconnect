#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define to32(c) (((c)<<24)|((c)<<16)|((c)<<8)|(c))
#define MAXPATHLEN 256


#define ID_NTCN	MAKE_ID('N','T','C','N')	/* `NetConnect' data chunk. */
#define ID_AICN	MAKE_ID('A','I','C','N')	/* An active icon (struct Icon; in IconBar) */
#define ID_IICN	MAKE_ID('I','I','C','N')	/* An inactive icon (struct Icon; in IconBank) */

#define NUM_STOPS (sizeof(Stops) / (2 * sizeof(ULONG)))


#define MUISERIALNR_NETCONNECT 1
#define TAGBASE_NETCONNECT (TAG_USER | (MUISERIALNR_NETCONNECT << 16))

#define MUIA_NetConnect_Icon							(TAGBASE_NETCONNECT | 0x1000)

#define MUIM_IconBarPrefs_LoadIcons			(TAGBASE_NETCONNECT | 0x1010)
#define MUIM_IconBarPrefs_Reset				(TAGBASE_NETCONNECT | 0x1011)
#define MUIM_IconBarPrefs_NewIcon			(TAGBASE_NETCONNECT | 0x1012)
#define MUIM_IconBarPrefs_ModifyIcon		(TAGBASE_NETCONNECT | 0x1013)
#define MUIM_IconBarPrefs_SetStates			(TAGBASE_NETCONNECT | 0x1014)

#define MUIV_IconBarPrefs_ModifyIcon_Remove		1
#define MUIV_IconBarPrefs_ModifyIcon_Name			2
#define MUIV_IconBarPrefs_ModifyIcon_Program		3
#define MUIV_IconBarPrefs_ModifyIcon_Image		4
#define MUIV_IconBarPrefs_ModifyIcon_Sound		5
#define MUIV_IconBarPrefs_ModifyIcon_Volume		6
#define MUIV_IconBarPrefs_ModifyIcon_Advanced	7

struct MUIP_IconBarPrefs_ModifyIcon			{ ULONG MethodID; LONG flag; };
struct MUIP_IconBarPrefs_SetStates			{ ULONG MethodID; LONG level; };


#define MUIM_IconBar_Finish					(TAGBASE_NETCONNECT | 0x1020)
#define MUIM_IconBar_LoadButtons				(TAGBASE_NETCONNECT | 0x1021)
#define MUIM_IconBar_IconBarPrefs			(TAGBASE_NETCONNECT | 0x1022)
#define MUIM_IconBar_IconBarPrefs_Finish	(TAGBASE_NETCONNECT | 0x1023)
#define MUIM_IconBar_AmiTCPPrefs				(TAGBASE_NETCONNECT | 0x1024)

struct MUIP_IconBar_IconBarPrefs_Finish		{ ULONG MethodID; LONG level; };


#define MUIM_AmiTCPPrefs_Finish				(TAGBASE_NETCONNECT | 0x1030)

struct MUIP_AmiTCPPrefs_Finish				{ ULONG MethodID; LONG level; };


struct IconBar_Data
{
	Object *MN_Strip;
	Object *GR_Buttons;

	Object *LI_IconBank;		/* list of icon in the bank (struct Icon) */
	Object *LI_IconBar;		/* list of active icon (struct Icon) */
	LONG Columns;				/* number of columns the IconBar window should contain */
	LONG Icons;					/* number of icons in the IconBar */
};

struct IconBarPrefs_Data
{
	Object *MN_Strip;
	Object *LV_InactiveIcons;
	Object *LI_InactiveIcons;
	Object *BT_New;
	Object *BT_Remove;
	Object *LV_ActiveIcons;
	Object *LI_ActiveIcons;
	Object *GR_Button;
	Object *SL_Columns;
	Object *PO_Name;
	Object *LV_Name;
	Object *PA_Program;
	Object *PA_Image;
	Object *PA_Sound;
	Object *CH_Advanced;
	Object *SL_Volume;
	Object *GR_Script;
	Object *GR_Editor;
	Object *TF_Editor;
	Object *PA_FindProgram;
	Object *BT_Add;
	Object *PA_SaveScript;

	Object *BT_Save;
	Object *BT_Use;
	Object *BT_Cancel;
};

struct IconList_Data
{
	LONG dummy;
};

struct Button_Data
{
	struct Icon *icon;
};

struct AmiTCPPrefs_Data
{
	Object *GR_Register;
	Object *GR_User;
	Object *GR_Server;
	Object *GR_Misc;
	Object *GR_DialScript;

	Object *BT_Save;
	Object *BT_Use;
	Object *BT_Cancel;
};

struct UserPrefs_Data
{
	Object *RA_Connection;

	Object *STR_UserName;
	Object *STR_NodeName;
	Object *STR_RealName;
	Object *STR_DomainName;
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
};

struct MiscPrefs_Data
{
	Object *PA_MailIn;
	Object *PA_MailOut;
	Object *PA_FilesIn;
	Object *PA_FilesOut;

	Object *PA_SerialDriver;
	Object *STR_ModemInit;
	Object *STR_SerialUnit;
	Object *STR_DialString;
	Object *PO_BaudRate;
	Object *LV_BaudRate;
};


struct Icon
{
	char Name[81];
	char Program[MAXPATHLEN];
	char ImageFile[MAXPATHLEN];
	char Sound[MAXPATHLEN];
	LONG Volume;
	BOOL Advanced;
	char Script[MAXPATHLEN];

	Object *bodychunk;
	Object *list;
	UBYTE *body;
};
