#define MUIM_MenuPrefs_NewEntry                 (TAGBASE_NETCONNECTPREFS | 0x1050)
#define MUIM_MenuPrefs_MenuList_Active          (TAGBASE_NETCONNECTPREFS | 0x1051)
#define MUIM_MenuPrefs_MenuList_ChangeLine      (TAGBASE_NETCONNECTPREFS | 0x1052)
#define MUIM_MenuPrefs_NewProgram               (TAGBASE_NETCONNECTPREFS | 0x1053)
#define MUIM_MenuPrefs_ProgramList_Active       (TAGBASE_NETCONNECTPREFS | 0x1054)
#define MUIM_MenuPrefs_ProgramList_ChangeLine   (TAGBASE_NETCONNECTPREFS | 0x1055)
#define MUIM_MenuPrefs_Asynch_Active            (TAGBASE_NETCONNECTPREFS | 0x1056)
#define MUIM_MenuPrefs_Type_Active              (TAGBASE_NETCONNECTPREFS | 0x1057)
#define MUIM_MenuPrefs_GetProgramList           (TAGBASE_NETCONNECTPREFS | 0x1058)

struct MenuPrefs_Data
{
   Object *LV_Menus;
   Object *LI_Menus;
   Object *BT_New;
   Object *BT_Delete;
   Object *STR_Name;

   Object *LV_Programs;
   Object *LI_Programs;
   Object *BT_NewProgram;
   Object *BT_DeleteProgram;
   Object *CY_Asynch;
   Object *CY_Type;
   Object *PA_Program;
   Object *STR_Program;
};

