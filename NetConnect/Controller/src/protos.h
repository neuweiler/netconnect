/// amiga.c
ULONG __stdargs DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...);
LONG xget(Object *obj, ULONG attribute);
//int BrokerFunc();
STRPTR GetStr(STRPTR idstr);
VOID play_sound(STRPTR file, LONG volume);
STRPTR update_string(STRPTR old, STRPTR source);
BOOL load_config(VOID);

///
/// main.c
ULONG check_date(VOID);

///
/// mui.c
#ifdef __SASC
SAVEDS ASM ULONG About_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG Button_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG Dock_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG MenuList_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
#else /* gcc */
ULONG About_Dispatcher();
ULONG Button_Dispatcher();
ULONG Dock_Dispatcher();
ULONG MenuList_Dispatcher();
#endif

///
/// fixpath.c
VOID __regargs DeleteCLI(struct CommandLineInterface *CLI);
struct CommandLineInterface * __regargs CloneCLI(struct Message *Message);

///
