#define MUIM_LoginScript_Dial                   (TAGBASE_WIZARD | 0x1080)
#define MUIM_LoginScript_GoOnline               (TAGBASE_WIZARD | 0x1081)
#define MUIM_LoginScript_HangUp                 (TAGBASE_WIZARD | 0x1082)
#define MUIM_LoginScript_SendLogin              (TAGBASE_WIZARD | 0x1083)
#define MUIM_LoginScript_SendPassword           (TAGBASE_WIZARD | 0x1084)
#define MUIM_LoginScript_SendBreak              (TAGBASE_WIZARD | 0x1085)
#define MUIM_LoginScript_DisposeWindow          (TAGBASE_WIZARD | 0x1086)
#define MUIM_LoginScript_KeyboardInput          (TAGBASE_WIZARD | 0x1087)
#define MUIM_LoginScript_SerialInput            (TAGBASE_WIZARD | 0x1088)

struct MUIP_LoginScript_Dial                    { ULONG MethodID; LONG restart; };

struct LoginScript_Data
{
   struct MUI_InputHandlerNode ihnode;
   BOOL ihnode_added;

   Object *TR_Terminal;
   Object *SB_Terminal;
   Object *BT_Dial;
   Object *BT_GoOnline;
   Object *BT_HangUp;
   Object *BT_SendLogin;
   Object *BT_SendPassword;
   Object *BT_SendBreak;
};
