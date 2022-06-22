#define MUISERIALNR_PREFS 1
#define TAGBASE_PREFS (TAG_USER | (MUISERIALNR_PREFS << 16))

#define VVSpace MUI_NewObject(MUIC_Rectangle, MUIA_FixWidth, 1, TAG_DONE)

enum { MEN_ABOUT = 1, MEN_ABOUT_MUI, MEN_ICONIFY, MEN_QUIT, MEN_LOAD, MEN_IMPORT, MEN_SAVE, MEN_SAVEAS, MEN_MUI };

#define MUIA_Genesis_Originator                  (TAGBASE_PREFS | 0x1000)
#define MUIM_Genesis_Finish                      (TAGBASE_PREFS | 0x1001)

struct MUIP_Genesis_Finish              { ULONG MethodID; Object *obj; LONG status; };

/*
Genesis        : 1000
MainWindow     : 1010
ProviderWindow : 1020
User           : 1030
Provider       : 1040
DataBase       : 1050
Modem          : 1060
Dialer         : 1070
Ifaces         : 1080
UserWindow     : 1090
*/

