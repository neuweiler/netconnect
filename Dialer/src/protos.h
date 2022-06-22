/// amiga.c
LONG xget(Object *obj,ULONG attribute);
ULONG DoSuperNew(struct IClass *cl,Object *obj,ULONG tag1,...);
Object *MakeButton(STRPTR string);
Object *MakeText(STRPTR string);
VOID SAVEDS des_func(register __a2 APTR pool, register __a1 APTR ptr);
STRPTR GetStr(STRPTR idstr);
BOOL SetEnvDOS(STRPTR name, STRPTR string, LONG len, BOOL save);
LONG GetEnvDOS(STRPTR name, STRPTR buffer, LONG max_len);
BOOL ParseConfig(STRPTR file, struct pc_Data *pc_data);
BOOL ParseNext(struct pc_Data *pc_data);
BOOL ParseNextLine(struct pc_Data *pc_data);
VOID ParseEnd(struct pc_Data *pc_data);
VOID EscapeString(STRPTR buffer, STRPTR str);
BOOL launch_amitcp(VOID);
LONG amirexx_do_command(const char *fmt, ...);
VOID clear_config(struct config *conf);
VOID exec_script(struct MinList *list);
ULONG DoMainMethod(Object *obj, LONG MethodID, APTR data1, APTR data2, APTR data3);

///
/// mui
SAVEDS ULONG MainWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ULONG Online_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);

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




