STRPTR GetStr(STRPTR idstr);

/// amiga.c
ULONG __stdargs DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...);
SAVEDS ASM VOID DestructFunc(REG(a2) APTR pool, REG(a1) APTR ptr);
LONG xget(Object *obj, ULONG attribute);
Object *MakeKeyLabel2(STRPTR label, STRPTR control_char);
Object *MakeKeyLLabel2(STRPTR label, STRPTR control_char);
Object *MakeButton(STRPTR string);
Object *MakeKeyString(STRPTR string, LONG len, STRPTR control_char);
Object *MakeKeyCycle(STRPTR *array, STRPTR control_char);
Object *MakeKeySlider(LONG min, LONG max, LONG level, STRPTR control_char);
Object *MakePopAsl(Object *string, STRPTR title, BOOL drawers_only);
Object *MakeKeyCheckMark(BOOL selected, STRPTR control_char);
VOID init_icon(struct Icon *icon);
BOOL find_list(struct DockPrefs_Data *data, Object **list, struct Icon **icon);
int find_max(STRPTR file);
SAVEDS ASM LONG AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x);
SAVEDS ASM LONG Editor_AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x);
BOOL editor_load(STRPTR file, Object *editor);
BOOL editor_save(STRPTR file, Object *editor);
BOOL editor_checksave(STRPTR file, Object *editor);
VOID play_sound(STRPTR file, LONG volume);
BOOL CopyFile(STRPTR infile, STRPTR outfile);
char *getfilename(Object *win, STRPTR title, STRPTR file, BOOL save);
STRPTR update_string(STRPTR old, STRPTR source);
///

/// mui.c
SAVEDS ASM ULONG About_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG ProgramList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
SAVEDS ASM ULONG MenuPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG PagerList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
SAVEDS ASM ULONG Editor_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
SAVEDS ASM ULONG EditIcon_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
SAVEDS ASM ULONG IconList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
SAVEDS ASM ULONG DockPrefs_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
SAVEDS ASM ULONG MainWindow_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);

///

