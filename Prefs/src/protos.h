/// amiga.c
STRPTR GetStr(STRPTR idstr);
ULONG __stdargs DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...);
LONG xget(Object *obj, ULONG attribute);
Object *MakeKeyLabel1(STRPTR label, STRPTR control_char);
Object *MakeKeyLabel2(STRPTR label, STRPTR control_char);
Object *MakeButton(STRPTR string);
Object *MakeKeyString(STRPTR string, LONG len, STRPTR control_char);
Object *MakeKeyCycle(STRPTR *array, STRPTR control_char);
Object *MakeKeySlider(LONG min, LONG max, LONG level, STRPTR control_char);
Object *MakeKeyCheckMark(BOOL state, STRPTR control_char);
Object *MakePopAsl(Object *string, STRPTR title, BOOL drawers_only);
LONG get_file_size(STRPTR file);
BOOL ParseConfig(STRPTR file, struct pc_Data *pc_data);
BOOL ParseNext(struct pc_Data *pc_data);
BOOL ParseNextLine(struct pc_Data *pc_data);
VOID ParseEnd(struct pc_Data *pc_data);
STRPTR extract_arg(STRPTR string, STRPTR buffer, LONG len, char sep);
char *getfilename(Object *win, STRPTR title, STRPTR file, BOOL save);
STRPTR get_configcontents(BOOL ppp);

///
/// mui?.c
SAVEDS ASM ULONG PasswdReq_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG MainWindow_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG About_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG Provider_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG User_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG Dialer_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG Paths_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG Databases_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG Modem_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);

///
/// StackCall.asm
LONG __stdargs StackCall(LONG *Success, LONG StackSize, LONG ArgCount, LONG (* __stdargs Function)(...), ...);
LONG StackSize(struct Task *Task);

///

