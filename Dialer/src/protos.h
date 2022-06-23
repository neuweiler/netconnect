/// amiga.c
LONG xget(Object *obj,ULONG attribute);
ULONG DoSuperNew(struct IClass *cl,Object *obj,ULONG tag1,...);
Object *MakeButton(STRPTR string);
Object *MakeText(STRPTR string, BOOL set_min);
STRPTR GetStr(STRPTR idstr);
BOOL SetEnvDOS(STRPTR name, STRPTR string, LONG len, BOOL save);
LONG GetEnvDOS(STRPTR name, STRPTR buffer, LONG max_len);
LONG amirexx_do_command(const char *fmt, ...);
VOID EscapeString(STRPTR buffer, STRPTR str);
STRPTR extract_arg(STRPTR string, STRPTR buffer, LONG len, char sep);
BOOL parse_arguments(VOID);
VOID clear_config(struct Config *conf);
VOID clear_isp(struct ISP *isp);
VOID clear_list(struct MinList *list);
VOID iterate_ifacelist(struct MinList *list, int set_mode);
BOOL is_one_online(struct MinList *list);
struct Interface *find_iface_by_name(struct MinList *list, STRPTR name);
struct ServerEntry *find_server_by_name(struct MinList *list, STRPTR name);
struct Modem *find_modem_by_id(struct MinList *list, int id);
struct ServerEntry *add_server(struct MinList *list, STRPTR name, BOOL top);
VOID adjust_default_gateway(struct Library *SocketBase, struct MinList *list);
BOOL get_daytime(struct Library *SocketBase, STRPTR host, STRPTR buf);
BOOL run_async(STRPTR file);
BOOL run_wb(STRPTR cmd);
struct ScriptLine *add_script_line(struct MinList *list, int command, STRPTR contents, int userdata);
VOID exec_command(STRPTR command, int how);
VOID exec_event(struct MinList *list, int event);
BOOL launch_amitcp(VOID);
VOID launch_dialin(struct Interface *iface);
VOID syslog_AmiTCP(struct Library *SocketBase, ULONG level, const STRPTR format, ...);
BOOL safe_put_to_port(struct Message *message, STRPTR portname);
VOID decrypt(STRPTR in, STRPTR out);
ULONG DoMainMethod(Object *obj, LONG MethodID, APTR data1, APTR data2, APTR data3);
SAVEDS ASM VOID des_func(register __a2 APTR pool, register __a1 APTR ptr);
LONG   get_file_size(STRPTR file);

///
/// main.c
VOID HandleMainMethod(struct MsgPort *port);
#ifdef DEMO
ULONG check_date(VOID);
#endif

///
/// dialin.c
SAVEDS ASM VOID DialinHandler(register __a0 STRPTR args, register __d0 LONG arg_len);

///
/// mui
SAVEDS ASM ULONG MainWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Online_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG IfaceReq_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Led_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG About_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG NetInfo_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM APTR user_rxfunc(register __a0 struct Hook *hook, register __a2 Object *appl, register __a1 ULONG *arg);
SAVEDS ASM APTR online_rxfunc(register __a0 struct Hook *hook, register __a2 Object *appl, register __a1 ULONG *arg);
SAVEDS ASM APTR offline_rxfunc(register __a0 struct Hook *hook, register __a2 Object *appl, register __a1 ULONG *arg);
SAVEDS ASM APTR isonline_rxfunc(register __a0 struct Hook *hook, register __a2 Object *appl, register __a1 ULONG *arg);
SAVEDS ASM APTR window_rxfunc(register __a0 struct Hook *hook, register __a2 Object *appl, register __a1 ULONG *arg);

///
/// serial.c
VOID serial_stopread(struct Modem *modem);
VOID serial_startread(struct Modem *modem, STRPTR data, LONG len);
VOID serial_send(struct Modem *modem, STRPTR cmd, LONG len);
BOOL serial_waitfor(struct Modem *modem, STRPTR string, int secs);
BOOL serial_readln(struct Modem *modem, STRPTR buffer, ULONG buf_len, int secs, BOOL echo);
BOOL serial_carrier(struct Modem *modem);
BOOL serial_dsr(struct Modem *modem);
VOID serial_hangup(struct Modem *modem, struct Library *SocketBase);
VOID serial_clear(struct Modem *modem);
BOOL serial_create(struct Modem *modem, struct Library *SocketBase);
VOID serial_delete(struct Modem *modem);

///
BOOL config_lo0(struct Library *SocketBase);
BOOL is_inaddr_any(struct Library *SocketBase, STRPTR addr);

/// fixpath.c

VOID __regargs DeleteCLI(struct CommandLineInterface *CLI);
struct CommandLineInterface * __regargs CloneCLI(struct Message *Message);
///

