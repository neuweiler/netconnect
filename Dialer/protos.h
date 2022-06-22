/// amiga.c
LONG xget(Object *obj,ULONG attribute);
char *xgetstr(Object *obj);
BOOL xgetbool(Object *obj);
ULONG __stdargs DoSuperNew(struct IClass *cl,Object *obj,ULONG tag1,...);
Object *MakeButton(STRPTR string);
Object *MakeText(STRPTR string);
Object *MakeKeyString(STRPTR string, LONG len, STRPTR c);
Object *MakeKeyLabel(STRPTR label, STRPTR control_char);
Object *MakeKeyCycle(STRPTR *array, STRPTR control_char);
Object *MakeCheckMark(BOOL selected);
Object *MakePopAsl(Object *string, STRPTR title, BOOL drawers_only);
SAVEDS ASM VOID desfunc(REG(a2) APTR pool, REG(a1) APTR *entry);
SAVEDS ASM LONG sortfunc(REG(a1) STRPTR str1, REG(a2) STRPTR str2);
SAVEDS ASM LONG strobjfunc(REG(a2) Object *list, REG(a1) Object *str);
SAVEDS ASM LONG txtobjfunc(REG(a2) Object *list, REG(a1) Object *txt);
STRPTR GetStr(STRPTR idstr);
BOOL SetEnvDOS(STRPTR name, STRPTR string, LONG len, BOOL save);
LONG GetEnvDOS(STRPTR name, STRPTR buffer, LONG max_len);
STRPTR extract_arg(STRPTR string, STRPTR buffer, LONG len, char sep);
BOOL ParseConfig(STRPTR file, struct pc_Data *pc_data);
BOOL ParseNext(struct pc_Data *pc_data);
BOOL ParseNextLine(struct pc_Data *pc_data);
VOID ParseEnd(struct pc_Data *pc_data);
VOID EscapeString(STRPTR buffer, STRPTR str);
BOOL launch_async(STRPTR file);
BOOL launch_amitcp(VOID);
LONG amirexx_do_command(const char *fmt, ...);
VOID clear_config(struct config *conf);
VOID exec_script(struct MinList *list);
ULONG DoMainMethod(Object *obj, LONG MethodID, APTR data1, APTR data2, APTR data3);

///
/// mui
SAVEDS ASM ULONG MainWindow_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG Online_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);

///
/// serial.c
VOID serial_stopread(VOID);
VOID serial_startread(STRPTR data, LONG len);
VOID serial_send(STRPTR cmd, LONG len);
BOOL serial_waitfor(STRPTR string, int secs);
BOOL serial_carrier(VOID);
VOID serial_hangup(VOID);
BOOL serial_create(VOID);
VOID serial_delete(VOID);

///

/// StackCall.asm

LONG __stdargs StackCall(LONG *Success, LONG StackSize, LONG ArgCount, LONG (* __stdargs Function)(...), ...);
LONG StackSize(struct Task *Task);
///
/// fixpath.c

VOID __regargs DeleteCLI(struct CommandLineInterface *CLI);
struct CommandLineInterface * __regargs CloneCLI(struct Message *Message);
///




