#define MUIM_MainWindow_Finish                  (TAGBASE_PREFS | 0x1000)
#define MUIM_MainWindow_InitGroups              (TAGBASE_PREFS | 0x1001)
#define MUIM_MainWindow_LoadConfig              (TAGBASE_PREFS | 0x1002)
#define MUIM_MainWindow_SaveConfig              (TAGBASE_PREFS | 0x1003)
#define MUIM_MainWindow_About                   (TAGBASE_PREFS | 0x1004)
#define MUIM_MainWindow_AboutFinish             (TAGBASE_PREFS | 0x1005)

struct MUIP_MainWindow_Finish          { ULONG MethodID; LONG level; };
struct MUIP_MainWindow_AboutFinish     { ULONG MethodID; Object *window; };
struct MUIP_MainWindow_LoadConfig      { ULONG MethodID; STRPTR file; };
struct MUIP_MainWindow_SaveConfig      { ULONG MethodID; STRPTR file; };

struct MainWindow_Data
{
   Object *MN_Strip;

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

