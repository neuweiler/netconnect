#define MUIM_MainWindow_GetPageData            (TAGBASE_WIZARD | 0x1010)
#define MUIM_MainWindow_SetPage                (TAGBASE_WIZARD | 0x1011)
#define MUIM_MainWindow_NextPage               (TAGBASE_WIZARD | 0x1012)
#define MUIM_MainWindow_BackPage               (TAGBASE_WIZARD | 0x1013)
#define MUIM_MainWindow_About                  (TAGBASE_WIZARD | 0x1014)
#define MUIM_MainWindow_Abort                  (TAGBASE_WIZARD | 0x1015)
#define MUIM_MainWindow_MUIRequest             (TAGBASE_WIZARD | 0x1016)
#define MUIM_MainWindow_DisposeWindow          (TAGBASE_WIZARD | 0x1017)

struct MUIP_MainWindow_DisposeWindow         { ULONG MethodID; Object *window; };
struct MUIP_MainWindow_MUIRequest            { ULONG MethodID; STRPTR buttons; STRPTR message; };

struct MainWindow_Data
{
   LONG Page;

   Object *MN_Strip;

   Object *GR_Pager;
   Object *GR_Active;

   Object *BT_Back;
   Object *BT_Next;
   Object *BT_Abort;
};

