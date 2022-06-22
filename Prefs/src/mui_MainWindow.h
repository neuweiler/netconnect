#define MUIM_MainWindow_Finish                  (TAGBASE_PREFS | 0x1010)
#define MUIM_MainWindow_InitGroups              (TAGBASE_PREFS | 0x1011)
#define MUIM_MainWindow_LoadConfig              (TAGBASE_PREFS | 0x1012)
#define MUIM_MainWindow_SaveConfig              (TAGBASE_PREFS | 0x1013)
#define MUIM_MainWindow_About                   (TAGBASE_PREFS | 0x1014)
#define MUIM_MainWindow_AboutFinish             (TAGBASE_PREFS | 0x1015)
#define MUIM_MainWindow_MenuLoad                (TAGBASE_PREFS | 0x1016)
#define MUIM_MainWindow_MenuImport              (TAGBASE_PREFS | 0x1017)
#define MUIM_MainWindow_MenuSaveAs              (TAGBASE_PREFS | 0x1018)
#define MUIM_MainWindow_ClearConfig             (TAGBASE_PREFS | 0x1019)
#define MUIM_MainWindow_LoadDatabases           (TAGBASE_PREFS | 0x101a)

struct MUIP_MainWindow_Finish          { ULONG MethodID; LONG level; };
struct MUIP_MainWindow_AboutFinish     { ULONG MethodID; Object *window; };
struct MUIP_MainWindow_LoadConfig      { ULONG MethodID; STRPTR file; };
struct MUIP_MainWindow_SaveConfig      { ULONG MethodID; STRPTR file; };

struct MainWindow_Data
{
   Object *MN_Strip;

   Object *LV_Pager;
   Object *LI_Pager;
   Object *GR_Pager;

   Object *BT_Save;
   Object *BT_Cancel;

   Object *GR_Info;
   Object *GR_Provider;
   Object *GR_User;
   Object *GR_Modem;
   Object *GR_Dialer;
   Object *GR_Databases;
};

