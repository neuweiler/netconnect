STRPTR GetStr(STRPTR idstr);
VOID ParseEnd(struct pc_Data *pc_data);
ULONG __stdargs DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...);
LONG xget(Object *obj, ULONG attribute);
SAVEDS ASM LONG sortfunc(REG(a1) STRPTR str1, REG(a2) STRPTR str2);
SAVEDS ASM LONG strobjfunc(REG(a2) Object *list, REG(a1) Object *str);
SAVEDS ASM LONG txtobjfunc(REG(a2) Object *list, REG(a1) Object *str);
Object *MakeKeyLabel1(STRPTR label, STRPTR control_char);
Object *MakeKeyLabel2(STRPTR label, STRPTR control_char);
Object *MakeButton(STRPTR string);
Object *MakeKeyString(STRPTR string, LONG len, STRPTR control_char);
Object *MakeKeyCycle(STRPTR *array, STRPTR control_char);
Object *MakeKeySlider(LONG min, LONG max, LONG level, STRPTR control_char);
Object *MakePopAsl(Object *string, STRPTR title, BOOL drawers_only);
LONG get_file_size(STRPTR file);
STRPTR LoadFile(STRPTR file);
BOOL editor_load(STRPTR file, Object *editor);
BOOL editor_save(STRPTR file, Object *editor);
BOOL ParseConfig(STRPTR file, struct pc_Data *pc_data);
BOOL ParseNext(struct pc_Data *pc_data);
BOOL ParseNextLine(struct pc_Data *pc_data);
VOID ParseEnd(struct pc_Data *pc_data);
BOOL CopyFile(STRPTR infile, STRPTR outfile);

SAVEDS ASM ULONG About_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG InfoWindow_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG Provider_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG User_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG Modem_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG Paths_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);

// StackCall.asm

LONG __stdargs StackCall(LONG *Success, LONG StackSize, LONG ArgCount, LONG (* __stdargs Function)(...), ...);
LONG StackSize(struct Task *Task);