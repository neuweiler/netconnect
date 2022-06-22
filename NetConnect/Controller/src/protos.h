/// amiga.c
ULONG __stdargs DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...);
LONG xget(Object *obj, ULONG attribute);
SAVEDS ASM int BrokerFunc(REG(a1) CxMsg *msg);
STRPTR GetStr(STRPTR idstr);
VOID play_sound(STRPTR file, LONG volume);
ULONG BuildCommandLine(char *buf, struct Program *program, BPTR curdir, struct AppMessage *msg);
BOOL StartCLIProgram(struct Program *program, struct AppMessage *msg);
BOOL StartWBProgram(struct Program *program, struct AppMessage *msg);
VOID StartProgram(struct Program *program, struct AppMessage *msg);
VOID init_icon(struct Icon *icon);
STRPTR update_string(STRPTR old, STRPTR source);
BOOL load_config(VOID);

///
/// main.c
ULONG check_date(VOID);

///
/// mui.c
SAVEDS ASM ULONG About_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Button_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Dock_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG MenuList_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);

///
/// fixpath.c
VOID __regargs DeleteCLI(struct CommandLineInterface *CLI);
struct CommandLineInterface * __regargs CloneCLI(struct Message *Message);

///
