STRPTR GetStr(STRPTR idstr);

/// amiga.c
ULONG __stdargs DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...);
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
int find_max(STRPTR file);
BOOL editor_load(STRPTR file, Object *editor);
BOOL editor_save(STRPTR file, Object *editor);
BOOL editor_checksave(STRPTR file, Object *editor);
VOID play_sound(STRPTR file, LONG volume);
BOOL CopyFile(STRPTR infile, STRPTR outfile);
char *getfilename(Object *win, STRPTR title, STRPTR file, BOOL save);
STRPTR update_string(STRPTR old, STRPTR source);

/// mui.c
#ifdef __SASC
SAVEDS ASM ULONG About_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG ProgramList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
SAVEDS ASM ULONG MenuPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg);
SAVEDS ASM ULONG PagerList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
SAVEDS ASM ULONG Editor_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
SAVEDS ASM ULONG EditIcon_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
SAVEDS ASM ULONG IconList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
SAVEDS ASM ULONG DockPrefs_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
SAVEDS ASM ULONG MainWindow_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
#else /* gcc */
ULONG About_Dispatcher();
ULONG ProgramList_Dispatcher();
ULONG MenuPrefs_Dispatcher();
ULONG PagerList_Dispatcher();
ULONG Editor_Dispatcher();
ULONG EditIcon_Dispatcher();
ULONG IconList_Dispatcher();
ULONG DockPrefs_Dispatcher();
ULONG MainWindow_Dispatcher();
#endif
