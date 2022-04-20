#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define to32(c) (((c)<<24)|((c)<<16)|((c)<<8)|(c))
#define MAXPATHLEN 256

#define IMAGE_PATH "NetConnect:Images/"
#define PROGRAM_PATH "NetConnect:Programs/"

#define ID_NTCN	MAKE_ID('N','T','C','N')	/* `NetConnect' data chunk. */
#define ID_AICN	MAKE_ID('A','I','C','N')	/* An active icon (struct Icon; in IconBar) */
#define ID_IICN	MAKE_ID('I','I','C','N')	/* An inactive icon (struct Icon; in IconBank) */
#define ID_ROWS	MAKE_ID('R','O','W','S')
#define ID_WINT	MAKE_ID('W','I','N','T')	/* window type */
#define ID_BTTY	MAKE_ID('B','T','T','Y')	/* how buttons are displayed */

#define ID_MENU	MAKE_ID('M','E','N','U')	/* A menu entry */
#define ID_NODE	MAKE_ID('N','O','D','E')	/* A new node in listtree	*/
#define ID_END		MAKE_ID('E','N','D','0')	/* End tag. */

#define NUM_STOPS (sizeof(Stops) / (2 * sizeof(ULONG)))


#define MUISERIALNR_NETCONNECT 1
#define TAGBASE_NETCONNECT (TAG_USER | (MUISERIALNR_NETCONNECT << 16))

#define ID_REBUILD										(TAGBASE_NETCONNECT | 0x0fff)

#define MUIA_NetConnect_Icon							(TAGBASE_NETCONNECT | 0x1000)
#define MUIA_NetConnect_List							(TAGBASE_NETCONNECT | 0x1001)
#define MUIA_NetConnect_Originator					(TAGBASE_NETCONNECT | 0x1002)

#define MUIM_IconBarPrefs_LoadIcons			(TAGBASE_NETCONNECT | 0x1010)
#define MUIM_IconBarPrefs_Reset				(TAGBASE_NETCONNECT | 0x1011)
#define MUIM_IconBarPrefs_Rows				(TAGBASE_NETCONNECT | 0x1012)
#define MUIM_IconBarPrefs_NewIcon			(TAGBASE_NETCONNECT | 0x1013)
#define MUIM_IconBarPrefs_DeleteIcon		(TAGBASE_NETCONNECT | 0x1014)
#define MUIM_IconBarPrefs_EditIcon			(TAGBASE_NETCONNECT | 0x1015)
#define MUIM_IconBarPrefs_EditIcon_Finish	(TAGBASE_NETCONNECT | 0x1016)
#define MUIM_IconBarPrefs_List_Active		(TAGBASE_NETCONNECT | 0x1017)

struct MUIP_IconBarPrefs_EditIcon_Finish	{ ULONG MethodID; struct Icon *icon; Object *list; LONG use; };
struct MUIP_IconBarPrefs_SetStates			{ ULONG MethodID; LONG level; };

#define MUIM_EditIcon_Editor_Active		(TAGBASE_NETCONNECT | 0x1020)
#define MUIM_EditIcon_ChangeLine			(TAGBASE_NETCONNECT | 0x1021)
#define MUIM_EditIcon_Type_Active		(TAGBASE_NETCONNECT | 0x1022)
#define MUIM_EditIcon_Program_Active	(TAGBASE_NETCONNECT | 0x1023)
#define MUIM_EditIcon_Sound_Active		(TAGBASE_NETCONNECT | 0x1024)
#define MUIM_EditIcon_PlaySound			(TAGBASE_NETCONNECT | 0x1025)

#define MUIM_IconBar_LoadButtons				(TAGBASE_NETCONNECT | 0x1020)
#define MUIM_IconBar_IconBarPrefs			(TAGBASE_NETCONNECT | 0x1021)
#define MUIM_IconBar_IconBarPrefs_Finish	(TAGBASE_NETCONNECT | 0x1022)
#define MUIM_IconBar_MenuPrefs				(TAGBASE_NETCONNECT | 0x1023)
#define MUIM_IconBar_MenuPrefs_Finish		(TAGBASE_NETCONNECT | 0x1024)
#define MUIM_IconBar_AmiTCPPrefs				(TAGBASE_NETCONNECT | 0x1025)
#define MUIM_IconBar_About						(TAGBASE_NETCONNECT | 0x1026)
#define MUIM_IconBar_About_Finish			(TAGBASE_NETCONNECT | 0x1027)

struct MUIP_IconBar_IconBarPrefs_Finish	{ ULONG MethodID; Object *window; LONG level; };
struct MUIP_IconBar_MenuPrefs_Finish		{ ULONG MethodID; Object *window; LONG level; };
struct MUIP_IconBar_About_Finish				{ ULONG MethodID; Object *window; LONG level; };

#define MUIM_Button_Action						(TAGBASE_NETCONNECT | 0x1030)

#define MUIM_MenuPrefs_NewEntry				(TAGBASE_NETCONNECT | 0x1040)
#define MUIM_MenuPrefs_Listtree_Active		(TAGBASE_NETCONNECT | 0x1041)
#define MUIM_MenuPrefs_LoadMenus				(TAGBASE_NETCONNECT | 0x1042)


struct IconBar_Data
{
	Object *MN_Strip;
	Object *GR_Buttons;

	Object *LI_IconBank;		/* list of icon in the bank (struct Icon) */
	Object *LI_IconBar;		/* list of active icon (struct Icon) */
};

struct MenuPrefs_Data
{
	Object *LV_Menus;
	Object *LT_Menus;
	Object *STR_Name;
	Object *BT_New;
	Object *BT_Delete;

	Object *STR_Shortcut;
	Object *LV_Commands;
	Object *LI_Commands;
	Object *STR_Command;
	Object *BT_NewCommand;
	Object *BT_DeleteCommand;
	Object *CY_Type;

	Object *BT_Save;
	Object *BT_Use;
	Object *BT_Cancel;
};

struct IconBarPrefs_Data
{
	Object *MN_Strip;
	Object *LV_InactiveIcons;
	Object *LI_InactiveIcons;
	Object *BT_New;
	Object *BT_Edit;
	Object *BT_Delete;
	Object *LV_ActiveIcons;
	Object *LI_ActiveIcons;
	Object *SL_Rows;
	Object *CY_ButtonType;
	Object *CY_WindowType;

	Object *BT_Save;
	Object *BT_Use;
	Object *BT_Cancel;
};

struct EditIcon_Data
{
	Object *GR_Register;

	Object *GR_Button;
	Object *STR_Name;
	Object *PA_Program;
	Object *STR_Hotkey;
	Object *PA_Image;
	Object *PA_Sound;
	Object *BT_PlaySound;
	Object *CY_Type;
	Object *SL_Volume;

	Object *GR_Script;
	Object *LV_Editor;
	Object *LI_Editor;
	Object *STR_Line;
	Object *BT_New;
	Object *BT_Delete;
	Object *BT_Clear;

	Object *BT_Okay;
	Object *BT_Cancel;
};

struct Editor_Data
{
	Object *dummy;
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
	Object *edit_window;
};
