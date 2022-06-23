#define MUIM_MainWindow_GetPageData            (TAGBASE_WIZARD | 0x1010)
#define MUIM_MainWindow_SetPage                (TAGBASE_WIZARD | 0x1011)
#define MUIM_MainWindow_NextPage               (TAGBASE_WIZARD | 0x1012)
#define MUIM_MainWindow_BackPage               (TAGBASE_WIZARD | 0x1013)
#define MUIM_MainWindow_About                  (TAGBASE_WIZARD | 0x1014)
#define MUIM_MainWindow_Quit                   (TAGBASE_WIZARD | 0x1015)
#define MUIM_MainWindow_MUIRequest             (TAGBASE_WIZARD | 0x1016)
#define MUIM_MainWindow_Request                (TAGBASE_WIZARD | 0x1017)
#define MUIM_MainWindow_DisposeWindow          (TAGBASE_WIZARD | 0x1018)

struct MUIP_MainWindow_DisposeWindow         { ULONG MethodID; Object *window; };
struct MUIP_MainWindow_MUIRequest            { ULONG MethodID; STRPTR buttons; STRPTR message; };
struct MUIP_MainWindow_Request               { ULONG MethodID; STRPTR text; STRPTR buffer; struct Process *proc; };

struct MainWindow_Data
{
   LONG Page;

   Object *MN_Strip;
   Object *GR_Picture;
   Object *BC_Picture;
   Object *GR_Pager;
   Object *GR_Welcome;
   Object *GR_SerialSana;
   Object *GR_SerialModem;
   Object *GR_ModemStrings;
   Object *GR_UserInfo;
   Object *GR_Protocol;
   Object *GR_LoginScript;
   Object *GR_Finished;
   Object *GR_Sana2;
   Object *GR_SanaConfig;

   Object *BT_Back;
   Object *BT_Next;
   Object *BT_Abort;
};

