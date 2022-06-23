/// amiga.c
LONG xget(Object *obj,ULONG attribute);
char *xgetstr(Object *obj);
ULONG DoSuperNew(struct IClass *cl,Object *obj,ULONG tag1,...);
Object *MakeButton(STRPTR string);
Object *MakeText(STRPTR string);
Object *MakeFloatText(STRPTR string);
Object *MakeString(STRPTR string, LONG len);
Object *MakePopAsl(Object *string, STRPTR title, BOOL drawers_only);
SAVEDS ASM VOID desfunc(register __a2 APTR pool, register __a1 APTR *entry);
SAVEDS ASM LONG sortfunc(register __a1 STRPTR str1, register __a2 STRPTR str2);
SAVEDS ASM LONG strobjfunc(register __a2 Object *list, register __a1 Object *str);
SAVEDS ASM LONG txtobjfunc(register __a2 Object *list, register __a1 Object *txt);
SAVEDS ASM VOID objstrfunc(register __a2 Object *list,register __a1 Object *str);
SAVEDS ASM VOID objtxtfunc(register __a2 Object *list,register __a1 Object *txt);
STRPTR GetStr(STRPTR idstr);
LONG GetEnvDOS(STRPTR name, STRPTR buffer, LONG max_len);
STRPTR extract_arg(STRPTR string, STRPTR buffer, LONG len, char sep);
VOID EscapeString(STRPTR buffer, STRPTR str);
BOOL have_ppp_frame(UBYTE *data, ULONG count);
BOOL load_config(STRPTR file, struct Config *config);
BOOL save_config(STRPTR file, struct ISP *isp, struct Interface *iface, struct Config *config);
VOID print_config(BPTR fh, struct ISP *isp, struct Interface *iface, struct Config *config);
VOID clear_list(struct MinList *list);
BOOL launch_amitcp(VOID);
LONG amirexx_do_command(const char *fmt, ...);
ULONG DoMainMethod(Object *obj, LONG MethodID, APTR data1, APTR data2, APTR data3);
VOID syslog_AmiTCP(ULONG level, const STRPTR format, ...);
struct ServerEntry *find_server_by_name(struct MinList *list, STRPTR name);
struct ServerEntry *add_server(struct MinList *list, STRPTR name);


///
/// main.c
VOID About(VOID);
VOID HandleMainMethod(struct MsgPort *port);

///
/// mui?.c
SAVEDS ASM ULONG MainWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG ModemDetect_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG ModemProtocol_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG ModemWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Online_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Welcome_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG SerialSana_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG SerialModem_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG ModemStrings_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG UserInfo_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Protocol_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG LoginScript_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Finished_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Sana2_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG SanaConfig_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Request_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG About_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);

///
/// mui_LoginScript.c
VOID add_sl(LONG command, STRPTR contents);

///
/// bootpconfig.c
BOOL go_online(VOID);

///
/// serial.c
VOID serial_stopread(VOID);
VOID serial_startread(STRPTR data, LONG len);
VOID serial_send(STRPTR cmd, LONG len);
int serial_waitfor(STRPTR string1, STRPTR string2, STRPTR string3, int secs);
BOOL serial_carrier(VOID);
BOOL serial_dsr(VOID);
VOID serial_clear(VOID);
VOID serial_hangup(VOID);
BOOL serial_create(STRPTR device, LONG unit);
VOID serial_delete(VOID);

///
/// StackCall.asm

LONG __stdargs StackCall(LONG *Success, LONG StackSize, LONG ArgCount, LONG (* __stdargs Function)(...), ...);
LONG StackSize(struct Task *Task);
///

