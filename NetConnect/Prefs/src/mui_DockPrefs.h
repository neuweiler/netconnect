#define MUIA_DockPrefs_MinLineHeight   (TAGBASE_NETCONNECTPREFS | 0x1020)
#define MUIM_DockPrefs_GetDock         (TAGBASE_NETCONNECTPREFS | 0x1021)
#define MUIM_DockPrefs_SetDock         (TAGBASE_NETCONNECTPREFS | 0x1022)
#define MUIM_DockPrefs_NewIcon         (TAGBASE_NETCONNECTPREFS | 0x1023)
#define MUIM_DockPrefs_DeleteIcon      (TAGBASE_NETCONNECTPREFS | 0x1024)
#define MUIM_DockPrefs_CopyIcon        (TAGBASE_NETCONNECTPREFS | 0x1025)
#define MUIM_DockPrefs_EditIcon        (TAGBASE_NETCONNECTPREFS | 0x1026)
#define MUIM_DockPrefs_EditIcon_Finish (TAGBASE_NETCONNECTPREFS | 0x1027)
#define MUIM_DockPrefs_List_Active     (TAGBASE_NETCONNECTPREFS | 0x1028)
#define MUIM_DockPrefs_Name_Active     (TAGBASE_NETCONNECTPREFS | 0x1029)
#define MUIM_DockPrefs_Type_Active     (TAGBASE_NETCONNECTPREFS | 0x102a)

struct MUIP_DockPrefs_EditIcon_Finish  { ULONG MethodID; struct Icon *icon; Object *list; LONG use; };
struct MUIP_DockPrefs_SetDock          { ULONG MethodID; LONG entry; };

struct DockPrefs_Data
{
   Object *LV_ActiveIcons;
   Object *LI_ActiveIcons;
   Object *BT_Edit;
   Object *BT_Delete;
   Object *LV_InactiveIcons;
   Object *LI_InactiveIcons;
   Object *BT_New;
   Object *BT_Copy;
   Object *STR_Name;
   Object *STR_Hotkey;
   Object *SL_Rows;
   Object *PA_Font;
   Object *CY_ButtonType;
   Object *CH_Activate;
   Object *CH_PopUp;
   Object *CH_Backdrop;
   Object *CH_Frontmost;
   Object *CH_Borderless;
   Object *CH_DragBar;

   Object *LI_Docks;
   struct Dock *dock;   // pointer to dock that is currently selected
};

