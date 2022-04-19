#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define to32(c) (((c)<<24)|((c)<<16)|((c)<<8)|(c))
#define MAXPATHLEN 256


#define ID_NTCN	MAKE_ID('N','T','C','N')	/* `NetConnect' data chunk. */
#define ID_AICN	MAKE_ID('A','I','C','N')	/* An active icon (struct Icon; in IconBar) */
#define ID_IICN	MAKE_ID('I','I','C','N')	/* An inactive icon (struct Icon; in IconBank) */
#define ID_ROWS	MAKE_ID('R','O','W','S')

#define NUM_STOPS (sizeof(Stops) / (2 * sizeof(ULONG)))


#define MUISERIALNR_NETCONNECT 1
#define TAGBASE_NETCONNECT (TAG_USER | (MUISERIALNR_NETCONNECT << 16))

#define MUIA_NetConnect_Icon							(TAGBASE_NETCONNECT | 0x1000)

#define MUIM_IconBarPrefs_LoadIcons			(TAGBASE_NETCONNECT | 0x1010)
#define MUIM_IconBarPrefs_Reset				(TAGBASE_NETCONNECT | 0x1011)
#define MUIM_IconBarPrefs_NewIcon			(TAGBASE_NETCONNECT | 0x1012)
#define MUIM_IconBarPrefs_Rows				(TAGBASE_NETCONNECT | 0x1013)
#define MUIM_IconBarPrefs_ModifyIcon		(TAGBASE_NETCONNECT | 0x1014)
#define MUIM_IconBarPrefs_SetStates			(TAGBASE_NETCONNECT | 0x1015)

#define MUIV_IconBarPrefs_ModifyIcon_Remove			1
#define MUIV_IconBarPrefs_ModifyIcon_Name				2
#define MUIV_IconBarPrefs_ModifyIcon_Type				3
#define MUIV_IconBarPrefs_ModifyIcon_Program			4
#define MUIV_IconBarPrefs_ModifyIcon_Hotkey			5
#define MUIV_IconBarPrefs_ModifyIcon_Image			6
#define MUIV_IconBarPrefs_ModifyIcon_Sound			7
#define MUIV_IconBarPrefs_ModifyIcon_PlaySound		8
#define MUIV_IconBarPrefs_ModifyIcon_Volume			9
#define MUIV_IconBarPrefs_ModifyIcon_LoadScript		10
#define MUIV_IconBarPrefs_ModifyIcon_SaveScript		11
#define MUIV_IconBarPrefs_ModifyIcon_ClearScript	12

struct MUIP_IconBarPrefs_ModifyIcon			{ ULONG MethodID; LONG flags; };
struct MUIP_IconBarPrefs_SetStates			{ ULONG MethodID; LONG level; };


#define MUIM_IconBar_LoadButtons				(TAGBASE_NETCONNECT | 0x1020)
#define MUIM_IconBar_IconBarPrefs			(TAGBASE_NETCONNECT | 0x1021)
#define MUIM_IconBar_IconBarPrefs_Finish	(TAGBASE_NETCONNECT | 0x1022)
#define MUIM_IconBar_AmiTCPPrefs				(TAGBASE_NETCONNECT | 0x1023)
#define MUIM_IconBar_About						(TAGBASE_NETCONNECT | 0x1024)
#define MUIM_IconBar_About_Finish			(TAGBASE_NETCONNECT | 0x1025)

struct MUIP_IconBar_IconBarPrefs_Finish	{ ULONG MethodID; Object *window; LONG level; };
struct MUIP_IconBar_About_Finish				{ ULONG MethodID; Object *window; };

#define MUIM_Button_Action						(TAGBASE_NETCONNECT | 0x1030)



struct IconBar_Data
{
	Object *MN_Strip;
	Object *GR_Buttons;

	Object *LI_IconBank;		/* list of icon in the bank (struct Icon) */
	Object *LI_IconBar;		/* list of active icon (struct Icon) */
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
	Object *SL_Rows;
	Object *STR_Name;
	Object *PA_Program;
	Object *STR_Hotkey;
	Object *PA_Image;
	Object *PA_Sound;
	Object *BT_PlaySound;
	Object *CY_Type;
	Object *SL_Volume;
	Object *GR_Script;
	Object *GR_Editor;
	Object *TF_Editor;
	Object *SB_Editor;
	Object *BT_LoadScript;
	Object *BT_SaveScript;
	Object *BT_ClearScript;

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

struct About_Data
{
	Object *BT_Button;
};


struct Icon
{
	char Name[81];
	char Program[MAXPATHLEN];
	char Hotkey[81];
	char ImageFile[MAXPATHLEN];
	char Sound[MAXPATHLEN];
	LONG Volume;
	LONG Type;

	Object *bodychunk;
	Object *list;
	UBYTE *body;
	struct BitMapHeader *bmhd;
	ULONG *cols;					/* pointer to color array (MUST be ULONG !!) */
	CxObj *cx_filter;
};
