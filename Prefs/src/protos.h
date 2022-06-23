/// amiga.c
STRPTR GetStr(STRPTR idstr);
ULONG DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...);
LONG xget(Object *obj, ULONG attribute);
SAVEDS ASM LONG sortfunc(register __a1 STRPTR str1, register __a2 STRPTR str2);
SAVEDS ASM VOID des_func(register __a2 APTR pool, register __a1 APTR ptr);
SAVEDS ASM LONG strobjfunc(register __a2 Object *list, register __a1 Object *str);
SAVEDS ASM VOID objstrfunc(register __a2 Object *list,register __a1 Object *str);
Object *MakeKeyLabel1(STRPTR label, STRPTR control_char);
Object *MakeKeyLabel2(STRPTR label, STRPTR control_char);
Object *MakeButton(STRPTR string);
Object *MakeKeyString(STRPTR string, LONG len, STRPTR control_char);
Object *MakeKeyCycle(STRPTR *array, STRPTR control_char);
Object *MakeKeySlider(LONG min, LONG max, LONG level, STRPTR control_char);
Object *MakeKeyCheckMark(BOOL state, STRPTR control_char);
Object *MakePopAsl(Object *string, STRPTR title, BOOL drawers_only);
STRPTR extract_arg(STRPTR string, STRPTR buffer, LONG len, char sep);
char *getfilename(Object *win, STRPTR title, STRPTR file, BOOL save);
VOID clear_list(struct MinList *list);
STRPTR realloc_copy(STRPTR *old, STRPTR src);
VOID encrypt(STRPTR in, STRPTR out);
VOID decrypt(STRPTR in, STRPTR out);

///
/// mui?.c
SAVEDS ASM ULONG PasswdReq_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG MainWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG About_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Provider_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG ProviderWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG User_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Dialer_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Paths_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Databases_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG Modem_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG IfaceWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);
SAVEDS ASM ULONG UserWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg);

///

