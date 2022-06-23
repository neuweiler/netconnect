#define MUISERIALNR_PREFS 1
#define TAGBASE_PREFS (TAG_USER | (MUISERIALNR_PREFS << 16))

#define VVSpace MUI_NewObject(MUIC_Rectangle, MUIA_FixWidth, 1, TAG_DONE)
#define setinteger(obj, i)    set(obj, MUIA_String_Integer, i)

enum { MEN_ABOUT = 1, MEN_ABOUT_MUI, MEN_ICONIFY, MEN_QUIT, MEN_LOAD, MEN_IMPORT, MEN_SAVE, MEN_SAVEAS, MEN_MUI };

#define MUIA_Genesis_Originator                  (TAGBASE_PREFS | 0x1000)
#define MUIM_Genesis_Finish                      (TAGBASE_PREFS | 0x1001)

struct MUIP_Genesis_Finish              { ULONG MethodID; Object *obj; LONG status; };

struct McpNode
{
   struct MinNode mcp_node;
   Object *mcp_object;
};

/*
Genesis        : 1000
MainWindow     : 1010
About          : 1020
DataBase       : 1030
IfaceWindow    : 1040
Interfaces     : 1050
Modems         : 1060
ModemWindow    : 1070
Options,       : 1080
PasswdReq      : 1090
User           : 10a0
UserWindow     : 10b0
*/

