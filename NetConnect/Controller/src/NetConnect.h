#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define to32(c) (((c)<<24)|((c)<<16)|((c)<<8)|(c))
#define MAXPATHLEN 256
#define CMDLINELEN 4096

#define PATH "NetConnect:"
#define IMAGE_PATH PATH"Images/"
#define PROGRAM_PATH PATH"Programs/"

#define ID_NTCN	MAKE_ID('N','T','C','N')	/* `NetConnect' data chunk. */
#define ID_AICN	MAKE_ID('A','I','C','N')	/* An active icon (struct Icon; in IconBar) */
#define ID_IICN	MAKE_ID('I','I','C','N')	/* An inactive icon (struct Icon; in IconBank) */
#define ID_COLS	MAKE_ID('C','O','L','S')
#define ID_WINT	MAKE_ID('W','I','N','T')	/* window type */
#define ID_BTTY	MAKE_ID('B','T','T','Y')	/* how buttons are displayed */
#define ID_MENU	MAKE_ID('M','E','N','U')	/* A menu entry */
#define ID_CMND	MAKE_ID('C','M','N','D')	/* A command entry */

#define NUM_STOPS (sizeof(Stops) / (2 * sizeof(ULONG)))


#define MUISERIALNR_NETCONNECT 1
#define TAGBASE_NETCONNECT (TAG_USER | (MUISERIALNR_NETCONNECT << 16))

#define ID_REBUILD										(TAGBASE_NETCONNECT | 0x0fff)

#define MUIA_NetConnect_Icon							(TAGBASE_NETCONNECT | 0x1000)
#define MUIA_NetConnect_List							(TAGBASE_NETCONNECT | 0x1001)
#define MUIA_NetConnect_Originator					(TAGBASE_NETCONNECT | 0x1002)

#define MUIM_IconBarPrefs_LoadIcons			(TAGBASE_NETCONNECT | 0x1010)
#define MUIM_IconBarPrefs_Reset				(TAGBASE_NETCONNECT | 0x1011)
//#define MUIM_IconBarPrefs_Rows				(TAGBASE_NETCONNECT | 0x1012)
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

#define MUIM_MenuPrefs_NewEntry						(TAGBASE_NETCONNECT | 0x1040)
#define MUIM_MenuPrefs_MenuList_Active				(TAGBASE_NETCONNECT | 0x1041)
#define MUIM_MenuPrefs_MenuList_ChangeLine		(TAGBASE_NETCONNECT | 0x1042)
#define MUIM_MenuPrefs_LoadMenus						(TAGBASE_NETCONNECT | 0x1043)
#define MUIM_MenuPrefs_NewProgram					(TAGBASE_NETCONNECT | 0x1044)
#define MUIM_MenuPrefs_ProgramList_Active			(TAGBASE_NETCONNECT | 0x1045)
#define MUIM_MenuPrefs_ProgramList_ChangeLine	(TAGBASE_NETCONNECT | 0x1046)
#define MUIM_MenuPrefs_Asynch_Active				(TAGBASE_NETCONNECT | 0x1047)
#define MUIM_MenuPrefs_Type_Active					(TAGBASE_NETCONNECT | 0x1048)
#define MUIM_MenuPrefs_GetProgramList				(TAGBASE_NETCONNECT | 0x1049)
#define MUIM_MenuPrefs_TriggerMenu					(TAGBASE_NETCONNECT | 0x104a)


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
	Object *LI_Menus;
	Object *STR_Name;
	Object *BT_New;
	Object *BT_Delete;

	Object *LV_Programs;
	Object *LI_Programs;
	Object *PA_Program;
	Object *BT_NewProgram;
	Object *BT_DeleteProgram;
	Object *CY_Asynch;
	Object *CY_Type;

	Object *BT_Save;
	Object *BT_Use;
	Object *BT_Cancel;
};

struct ProgramList_Data
{
	Object *Originator;
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
	Object *SL_Columns;
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

	Object *GR_Advanced;
	Object *STR_Stack;
	Object *SL_Priority;
	Object *PA_CurrentDir;
	Object *PA_OutputFile;
	Object *STR_PublicScreen;
	Object *CH_WBArgs;
	Object *CH_ToFront;

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


enum { TYPE_CLI, TYPE_WORKBENCH, TYPE_SCRIPT, TYPE_AREXX };

#define PRG_Arguments	1
#define PRG_ToFront		2

struct Program
{
	char File[MAXPATHLEN];
	LONG Type;
	BOOL Asynch;

	LONG Stack;
	LONG Priority;
	char CurrentDir[MAXPATHLEN];
	char OutputFile[MAXPATHLEN];
	char PublicScreen[81];
	LONG Flags;			// PRG_Arguments = pass wb-icons as argument		PRG_ToFront = Put pubscreen to front
};

struct Icon
{
	char Name[81];
	char Hotkey[81];
	char ImageFile[MAXPATHLEN];
	char Sound[MAXPATHLEN];
	LONG Volume;
	struct Program Program;

	Object *bodychunk;
	Object *list;
	UBYTE *body;
	struct BitMapHeader *bmhd;
	ULONG *cols;					/* pointer to color array (MUST be ULONG !!) */
	CxObj *cx_filter;
	Object *edit_window;
	struct DiskObject *disk_object;
};


struct MenuEntry
{
	char Name[81];
	Object *LI_Programs;
	struct AppMenuItem *AppMenuItem;
	LONG Id;
};


struct Dock
{
	char Name[41];
	char WindowTitle[41];
	char Hotkey[41];
	LONG Columns;
	Object *LI_Buttons;

	LONG Type;
	LONG Flags;		// Activated, Popup, Backdrop, Frontmost, Arguments (Pass selected wb-icons as arguments)
};
