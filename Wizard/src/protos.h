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
VOID close_serial(VOID);
BOOL open_serial(STRPTR device_name, LONG unit);
VOID __regargs StartSerialRead(register APTR Data, register ULONG Length);
VOID send_serial(STRPTR cmd, LONG len);
BOOL serial_carrier(VOID);
VOID FlushSerialRead(VOID);
VOID StopSerialRead(VOID);
BOOL have_ppp_frame(UBYTE *data, ULONG count);
BOOL save_config(STRPTR file);
VOID print_config(BPTR fh);

///
/// main.c
VOID About(VOID);

///
/// mui1.c
SAVEDS ASM ULONG MainWindow_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG ModemDetect_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG ModemProtocol_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG ModemWindow_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG Online_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);

///
/// StackCall.asm

LONG __stdargs StackCall(LONG *Success, LONG StackSize, LONG ArgCount, LONG (* __stdargs Function)(...), ...);
LONG StackSize(struct Task *Task);
///
/// fixpath.c

VOID __regargs DeleteCLI(struct CommandLineInterface *CLI);
struct CommandLineInterface * __regargs CloneCLI(struct Message *Message);
///
/// bootpconfig.c
BOOL go_online(VOID);

///




