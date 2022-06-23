/// amiga.c
ULONG __stdargs DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...);
LONG xget(Object *obj, ULONG attribute);
//int BrokerFunc();
STRPTR GetStr(STRPTR idstr);
VOID play_sound(STRPTR file, LONG volume);
//ULONG BuildCommandLine(char *buf, struct Program *program, BPTR curdir, struct AppMessage *msg);
//BOOL StartCLIProgram(struct Program *program, struct AppMessage *msg);
//BOOL StartWBProgram(struct Program *program, struct AppMessage *msg);
//VOID StartProgram(struct Program *program, struct AppMessage *msg);
//VOID init_icon(struct Icon *icon);
STRPTR update_string(STRPTR old, STRPTR source);
BOOL load_config(VOID);

///
/// main.c
ULONG check_date(VOID);

///
/// mui.c
ULONG About_Dispatcher();
ULONG Button_Dispatcher();
ULONG Dock_Dispatcher();
ULONG MenuList_Dispatcher();

///
/// fixpath.c
VOID __regargs DeleteCLI(struct CommandLineInterface *CLI);
struct CommandLineInterface * __regargs CloneCLI(struct Message *Message);

///
