#include "globals.c"
#include "protos.h"

/****************************************************************************/
/* About class                                                              */
/****************************************************************************/

ULONG About_New(struct IClass *cl, Object *obj, Msg msg)
{
	struct About_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title, GetStr(MSG_LA_About),
		MUIA_Window_ID   , MAKE_ID('A','B','O','U'),
		WindowContents, VGroup,
			MUIA_Background, MUII_RequesterBack,
			Child, HGroup,
				TextFrame,
				MUIA_Background, MUII_GroupBack,
				Child, HVSpace,
				Child, BodychunkObject,
					MUIA_FixWidth             , LOGO_WIDTH ,
					MUIA_FixHeight            , LOGO_HEIGHT,
					MUIA_Bitmap_Width         , LOGO_WIDTH ,
					MUIA_Bitmap_Height        , LOGO_HEIGHT,
					MUIA_Bodychunk_Depth      , LOGO_DEPTH ,
					MUIA_Bodychunk_Body       , (UBYTE *)logo_body,
					MUIA_Bodychunk_Compression, LOGO_COMPRESSION,
					MUIA_Bodychunk_Masking    , LOGO_MASKING,
					MUIA_Bitmap_SourceColors  , (ULONG *)logo_colors,
					MUIA_Bitmap_Transparent   , 0,
				End,
				Child, HVSpace,
			End,
			Child, ScrollgroupObject,
				MUIA_CycleChain, 1,
				MUIA_Background, MUII_ReadListBack,
				MUIA_Scrollgroup_FreeHoriz, FALSE,
				MUIA_Scrollgroup_Contents, VirtgroupObject,
					ReadListFrame,
					Child, TextObject,
						MUIA_Text_Contents, GetStr(MSG_TX_About1),
					End,
					Child, MUI_MakeObject(MUIO_HBar, 2),
					Child, TextObject,
						MUIA_Text_Contents, GetStr(MSG_TX_About2),
					End,
				End,
			End,
			Child, HGroup,
				Child, HSpace(0),
				Child, tmp.BT_Button = MakeButton(MSG_BT_Okay),
				Child, HSpace(0),
			End,
		End))
	{
		struct About_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		set(obj, MUIA_Window_ActiveObject, tmp.BT_Button);

		DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE ,	MUIV_Notify_Application, 5, MUIM_Application_PushMethod, win, 2, MUIM_IconBar_About_Finish, obj);
		DoMethod(data->BT_Button, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, win, 2, MUIM_IconBar_About_Finish, obj);
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG About_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW : return(About_New(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Program List Class  (List)                                               */
/****************************************************************************/

SAVEDS ASM struct Program *ProgramList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Program *src)
{
	struct Program *new;

	if(new = (struct Program *)AllocVec(sizeof(struct Program), MEMF_ANY | MEMF_CLEAR))
	{
		new->Asynch = 1;
		if(src)
			memcpy(new, src, sizeof(struct Program));
	}
	return(new);
}

SAVEDS ASM VOID ProgramList_DestructFunc(REG(a2) APTR pool, REG(a1) struct Program *program)
{
	if(program)
		FreeVec(program);
}

static const struct Hook ProgramList_ConstructHook	= { { 0,0 }, (VOID *)ProgramList_ConstructFunc	, NULL, NULL };
static const struct Hook ProgramList_DestructHook	= { { 0,0 }, (VOID *)ProgramList_DestructFunc	, NULL, NULL };

ULONG ProgramList_DragQuery(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
	if(msg->obj == obj)
		return(DoSuperMethodA(cl, obj, msg));
	else
		return(MUIV_DragQuery_Refuse);
}

ULONG ProgramList_DragDrop(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
	struct ProgramList_Data *data = INST_DATA(cl, obj);

	if(msg->obj == obj)
	{
		ULONG ret;

		ret = DoSuperMethodA(cl, obj, msg);		// go to move the entry first ! :)
		if(data->Originator)
			DoMethod(data->Originator, MUIM_MenuPrefs_GetProgramList);
		return(ret);
	}

	return(NULL);
}

ULONG ProgramList_New(struct IClass *cl,Object *obj,struct opSet *msg)
{
	obj = (Object *)DoSuperNew(cl, obj,
		InputListFrame,
		MUIA_List_AutoVisible	, TRUE,
		MUIA_List_DragSortable	, TRUE,
		MUIA_List_ConstructHook	, &ProgramList_ConstructHook,
		MUIA_List_DestructHook	, &ProgramList_DestructHook,
		TAG_MORE, msg->ops_AttrList);

	return((ULONG)obj);
}

SAVEDS ASM ULONG ProgramList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg)
{
	switch(msg->MethodID)
	{
		case OM_NEW				: return(ProgramList_New 	 	  	(cl, obj, (APTR)msg));
		case MUIM_DragQuery	: return(ProgramList_DragQuery	(cl, obj, (APTR)msg));
		case MUIM_DragDrop	: return(ProgramList_DragDrop		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Menu Prefs class (Window)                                                */
/****************************************************************************/

SAVEDS ASM struct MenuEntry *MenuList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct MenuEntry *src)
{
	struct MenuEntry *new;

	if(new = (struct MenuEntry *)AllocVec(sizeof(struct MenuEntry), MEMF_ANY | MEMF_CLEAR))
	{
		if(src)
			memcpy(new, src, sizeof(struct MenuEntry));

		if(new->LI_Programs = ListObject,
			MUIA_List_ConstructHook	, &ProgramList_ConstructHook,
			MUIA_List_DestructHook	, &ProgramList_DestructHook,
			End)
		{
			new->AppMenuItem = AddAppMenuItem(new->Id, NULL, new->Name, appmenu_port, TAG_END);
		}
		else
		{
			FreeVec(new);
			new = NULL;
		}
	}
	return(new);
}

SAVEDS ASM VOID MenuList_DestructFunc(REG(a2) APTR pool, REG(a1) struct MenuEntry *menu)
{
	if(menu)
	{
		if(menu->AppMenuItem)
			RemoveAppMenuItem(menu->AppMenuItem);
		if(menu->LI_Programs)
			MUI_DisposeObject(menu->LI_Programs);
		FreeVec(menu);
	}
}

ULONG MenuPrefs_TriggerMenu(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);
	struct AppMessage *app_msg;
	struct MenuEntry *menu;

	while(app_msg = (struct AppMessage *)GetMsg(appmenu_port))
	{
		DoMethod(data->LI_Menus, MUIM_List_GetEntry, app_msg->am_ID, &menu);
		if(menu && strcmp(menu->Name, "~~~~~~~~~~"))
		{
			int pos = 0;
			struct Program *program;

			FOREVER
			{
				DoMethod(menu->LI_Programs, MUIM_List_GetEntry, pos++, &program);
				if(!program)
					break;
				StartProgram(program, app_msg);
			}
		}
		ReplyMsg(app_msg);
	}
	return(NULL);
}

ULONG MenuPrefs_LoadMenus(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);
	BOOL success = FALSE;
	struct IFFHandle	*Handle;
	struct ContextNode	*Chunk;
	struct MenuEntry *menu, *menu_ptr;
	struct Program *program;
	LONG menu_id;

	if(menu = (struct MenuEntry *)AllocVec(sizeof(struct MenuEntry), MEMF_ANY))
	{
		if(program = (struct Program *)AllocVec(sizeof(struct Program), MEMF_ANY))
		{
			if(Handle = AllocIFF())
			{
				if(Handle->iff_Stream = Open("ENV:NetConnectPrefs.menus", MODE_OLDFILE))
				{
					InitIFFasDOS(Handle);
					if(!(OpenIFF(Handle, IFFF_READ)))
					{
						if(!(StopChunks(Handle, Stops, NUM_STOPS)))
						{
							set(data->LI_Menus, MUIA_List_Quiet,TRUE);
							DoMethod(data->LI_Menus, MUIM_List_Clear);
							menu_id = NULL;
							while(!ParseIFF(Handle, IFFPARSE_SCAN))
							{
								Chunk = CurrentChunk(Handle);

								if(Chunk->cn_ID == ID_MENU)
								{
									if(ReadChunkBytes(Handle, menu, MIN(sizeof(struct MenuEntry), Chunk->cn_Size)) == MIN(sizeof(struct MenuEntry), Chunk->cn_Size))
									{
										menu->Id = menu_id++;
										DoMethod(data->LI_Menus, MUIM_List_InsertSingle, menu, MUIV_List_Insert_Bottom);
									}
								}
								if(Chunk->cn_ID == ID_CMND)
								{
									if(ReadChunkBytes(Handle, program, MIN(sizeof(struct Program), Chunk->cn_Size)) == MIN(sizeof(struct Program), Chunk->cn_Size))
									{
										DoMethod(data->LI_Menus, MUIM_List_GetEntry, xget(data->LI_Menus, MUIA_List_Entries) - 1, &menu_ptr);
										if(menu_ptr)
											DoMethod(menu_ptr->LI_Programs, MUIM_List_InsertSingle, program, MUIV_List_Insert_Bottom);
									}
								}
							}
							set(data->LI_Menus, MUIA_List_Quiet, FALSE);
							success = TRUE;
						}
						CloseIFF(Handle);
					}
					Close(Handle->iff_Stream);
				}
				FreeIFF(Handle);
			}
			FreeVec(program);
		}
		FreeVec(menu);
	}
	return(success);
}


ULONG MenuPrefs_NewEntry(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);
	struct MenuEntry *menu;

	if(menu = (struct MenuEntry *)AllocVec(sizeof(struct MenuEntry), MEMF_ANY | MEMF_CLEAR))
	{
		switch(MUI_Request(app, obj, 0, 0, "Menu|BarLabel", "Create what ?"))
		{
			case 0:
				strcpy(menu->Name, "~~~~~~~~~~");
				break;
			case 1:
				strcpy(menu->Name, "New Menu");
				break;
		}
		DoMethod(data->LI_Menus, MUIM_List_InsertSingle, menu, MUIV_List_Insert_Bottom);
		set(data->LI_Menus, MUIA_List_Active, MUIV_List_Active_Bottom);
		set(obj, MUIA_Window_ActiveObject, data->STR_Name);

		FreeVec(menu);
	}

	return(NULL);
}

ULONG MenuPrefs_MenuList_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);
	struct MenuEntry *menu;
	struct Program *program;
	int i;

	DoMethod(data->LI_Programs, MUIM_List_Clear);
	DoMethod(data->LI_Menus, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &menu);
	if(menu && strcmp(menu->Name, "~~~~~~~~~~"))
	{
		set(data->STR_Name		, MUIA_Disabled, FALSE);
		set(data->BT_Delete		, MUIA_Disabled, FALSE);
		set(data->LV_Programs	, MUIA_Disabled, FALSE);
		set(data->BT_NewProgram	, MUIA_Disabled, FALSE);
		setstring(data->STR_Name, menu->Name);

		i = 0;
		FOREVER
		{
			DoMethod(menu->LI_Programs, MUIM_List_GetEntry, i++, &program);
			if(!program)
				break;
			DoMethod(data->LI_Programs, MUIM_List_InsertSingle, program, MUIV_List_Insert_Bottom);
		}
	}
	else
	{
		set(data->STR_Name		, MUIA_Disabled, TRUE);
		set(data->BT_Delete		, MUIA_Disabled, !(menu));
		set(data->LV_Programs	, MUIA_Disabled, TRUE);
		set(data->BT_NewProgram	, MUIA_Disabled, TRUE);
		setstring(data->STR_Name, (menu ? menu->Name : ""));
	}

	return(NULL);
}

ULONG MenuPrefs_GetProgramList(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);
	struct MenuEntry *menu;
	struct Program *program;
	int i;

	DoMethod(data->LI_Menus, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &menu);
	if(menu)
	{
		DoMethod(menu->LI_Programs, MUIM_List_Clear);
		i = 0;
		FOREVER
		{
			DoMethod(data->LI_Programs, MUIM_List_GetEntry, i++, &program);
			if(!program)
				break;
			DoMethod(menu->LI_Programs, MUIM_List_InsertSingle, program, MUIV_List_Insert_Bottom);
		}
	}
	return(NULL);
}

ULONG MenuPrefs_MenuList_ChangeLine(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);
	struct MenuEntry *menu;

	DoMethod(data->LI_Menus, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &menu);
	if(menu)
	{
		strcpy(menu->Name, (STRPTR)xget(data->STR_Name, MUIA_String_Contents));
		DoMethod(data->LI_Menus, MUIM_List_Redraw, MUIV_List_Redraw_Active);
		set((Object *)xget(data->STR_Name, MUIA_WindowObject), MUIA_Window_ActiveObject, data->STR_Name);
	}

	return(NULL);
}

ULONG MenuPrefs_NewProgram(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);

	DoMethod(data->LI_Programs, MUIM_List_InsertSingle, NULL, MUIV_List_Insert_Bottom);
	set(data->LI_Programs, MUIA_List_Active, MUIV_List_Active_Bottom);
	set(obj, MUIA_Window_ActiveObject, data->PA_Program);

	DoMethod(obj, MUIM_MenuPrefs_GetProgramList);

	return(NULL);
}

ULONG MenuPrefs_ProgramList_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);
	struct Program *program;

	DoMethod(data->LI_Programs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &program);
	if(program)
	{
		set(data->PA_Program			, MUIA_Disabled, FALSE);
		set(data->BT_DeleteProgram	, MUIA_Disabled, FALSE);
		set(data->CY_Asynch			, MUIA_Disabled, (program->Type == TYPE_WORKBENCH));
		set(data->CY_Type				, MUIA_Disabled, FALSE);

		setstring(data->PA_Program	, program->File);
		setcycle(data->CY_Asynch	, program->Asynch);
		setcycle(data->CY_Type		, program->Type);
	}
	else
	{
		set(data->PA_Program			, MUIA_Disabled, TRUE);
		set(data->BT_DeleteProgram	, MUIA_Disabled, TRUE);
		set(data->CY_Asynch			, MUIA_Disabled, TRUE);
		set(data->CY_Type				, MUIA_Disabled, TRUE);

		setstring(data->PA_Program	, "");
	}
	return(NULL);
}

ULONG MenuPrefs_ProgramList_ChangeLine(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);
	struct Program *program;

	DoMethod(data->LI_Programs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &program);
	if(program)
	{
		strcpy(program->File, (STRPTR)xget(data->PA_Program, MUIA_String_Contents));
		DoMethod(data->LI_Programs, MUIM_List_Redraw, MUIV_List_Redraw_Active);
		set((Object *)xget(data->PA_Program, MUIA_WindowObject), MUIA_Window_ActiveObject, data->PA_Program);

		DoMethod(obj, MUIM_MenuPrefs_GetProgramList);
	}
	return(NULL);
}

ULONG MenuPrefs_Asynch_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);
	struct Program *program;

	DoMethod(data->LI_Programs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &program);
	if(program)
	{
		program->Asynch = xget(data->CY_Asynch, MUIA_Cycle_Active);
		DoMethod(obj, MUIM_MenuPrefs_GetProgramList);
	}
	return(NULL);
}

ULONG MenuPrefs_Type_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);
	struct Program *program;

	DoMethod(data->LI_Programs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &program);
	if(program)
	{
		program->Type = xget(data->CY_Type, MUIA_Cycle_Active);
		if(program->Type == TYPE_WORKBENCH)
			setcycle(data->CY_Asynch, 1);
		set(data->CY_Asynch, MUIA_Disabled, (program->Type == TYPE_WORKBENCH));
		DoMethod(obj, MUIM_MenuPrefs_GetProgramList);
	}
	return(NULL);
}

ULONG MenuPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	static const struct Hook MenuList_ConstructHook	= { { 0,0 }, (VOID *)MenuList_ConstructFunc	, NULL, NULL };
	static const struct Hook MenuList_DestructHook	= { { 0,0 }, (VOID *)MenuList_DestructFunc	, NULL, NULL };
	struct MenuPrefs_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title		, "Menu Setup",
		MUIA_Window_ID			, MAKE_ID('M','P','R','F'),
		MUIA_Window_AppWindow, TRUE,
		WindowContents		, VGroup,
			GroupFrame,
			Child, HGroup,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_Menus = ListviewObject,
						MUIA_FrameTitle, "Menu Entries",
						MUIA_CycleChain			, 1,
						MUIA_Listview_Input		, TRUE,
						MUIA_Listview_DragType	, 1,
						MUIA_Listview_List		, tmp.LI_Menus = ListObject,
							InputListFrame,
							MUIA_List_ConstructHook	, &MenuList_ConstructHook,
							MUIA_List_DestructHook	, &MenuList_DestructHook,
							MUIA_List_DragSortable	, TRUE,
						End,
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_New = MakeButton("  _New"),
						Child, tmp.BT_Delete = MakeButton("  _Delete"),
					End,
					Child, tmp.STR_Name = String("", 80),
				End,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_Programs = ListviewObject,
						MUIA_FrameTitle, "Programs",
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_Programs = NewObject(CL_ProgramList->mcc_Class, NULL, TAG_DONE),
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_NewProgram = MakeButton("  N_ew"),
						Child, tmp.BT_DeleteProgram = MakeButton("  De_lete"),
						Child, tmp.CY_Asynch = Cycle(ARR_Asynch),
					End,
					Child, tmp.CY_Type = Cycle(ARR_ProgramTypes),
					Child, tmp.PA_Program = MakePopAsl(String("", MAXPATHLEN), MSG_LA_Image, FALSE),
				End,
			End,
			Child, MUI_MakeObject(MUIO_HBar, 2),
			Child, HGroup,
				MUIA_Group_SameSize	, TRUE,
				Child, tmp.BT_Save	= MakeButton(MSG_BT_Save),
				Child, HSpace(0),
				Child, tmp.BT_Use		= MakeButton(MSG_BT_Use),
				Child, HSpace(0),
				Child, tmp.BT_Cancel	= MakeButton(MSG_BT_Cancel),
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct MenuPrefs_Data *data = INST_DATA(cl, obj);
		struct ProgramList_Data *cl_data = INST_DATA(CL_ProgramList->mcc_Class, tmp.LI_Programs);

		*data = tmp;

		cl_data->Originator = obj;

		set(tmp.BT_Delete			, MUIA_Disabled, TRUE);
		set(tmp.STR_Name			, MUIA_Disabled, TRUE);
		set(tmp.LV_Programs		, MUIA_Disabled, TRUE);
		set(tmp.BT_NewProgram	, MUIA_Disabled, TRUE);
		set(tmp.BT_DeleteProgram, MUIA_Disabled, TRUE);
		set(tmp.CY_Asynch			, MUIA_Disabled, TRUE);
		set(tmp.CY_Type			, MUIA_Disabled, TRUE);
		set(tmp.PA_Program		, MUIA_Disabled, TRUE);
		set(tmp.STR_Name			, MUIA_String_AttachedList, tmp.LV_Menus);
		set(tmp.PA_Program		, MUIA_String_AttachedList, tmp.LV_Programs);

		DoMethod(tmp.LI_Menus			, MUIM_Notify, MUIA_List_Active			, MUIV_EveryTime	, obj					, 1, MUIM_MenuPrefs_MenuList_Active);
		DoMethod(tmp.BT_New				, MUIM_Notify, MUIA_Pressed				, FALSE				, obj					, 1, MUIM_MenuPrefs_NewEntry);
		DoMethod(tmp.BT_Delete			, MUIM_Notify, MUIA_Pressed				, FALSE				, tmp.LI_Menus		, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.STR_Name			, MUIM_Notify, MUIA_String_Acknowledge	, MUIV_EveryTime	, obj					, 1, MUIM_MenuPrefs_MenuList_ChangeLine);

		DoMethod(tmp.LI_Programs		, MUIM_Notify, MUIA_List_Active			, MUIV_EveryTime	, obj					, 1, MUIM_MenuPrefs_ProgramList_Active);
		DoMethod(tmp.BT_NewProgram		, MUIM_Notify, MUIA_Pressed				, FALSE				, obj					, 1, MUIM_MenuPrefs_NewProgram);
		DoMethod(tmp.BT_DeleteProgram	, MUIM_Notify, MUIA_Pressed				, FALSE				, tmp.LI_Programs	, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.BT_DeleteProgram	, MUIM_Notify, MUIA_Pressed				, FALSE				, obj					, 1, MUIM_MenuPrefs_GetProgramList);
		DoMethod(tmp.PA_Program			, MUIM_Notify, MUIA_String_Acknowledge	, MUIV_EveryTime	, obj					, 1, MUIM_MenuPrefs_ProgramList_ChangeLine);
		DoMethod(tmp.CY_Asynch			, MUIM_Notify, MUIA_Cycle_Active			, MUIV_EveryTime	, obj					, 1, MUIM_MenuPrefs_Asynch_Active);
		DoMethod(tmp.CY_Type				, MUIM_Notify, MUIA_Cycle_Active			, MUIV_EveryTime	, obj					, 1, MUIM_MenuPrefs_Type_Active);

		DoMethod(obj						, MUIM_Notify, MUIA_Window_CloseRequest, TRUE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_MenuPrefs_Finish, obj, 0);
		DoMethod(tmp.BT_Cancel			, MUIM_Notify, MUIA_Pressed				, FALSE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_MenuPrefs_Finish, obj, 0);
		DoMethod(tmp.BT_Use				, MUIM_Notify, MUIA_Pressed				, FALSE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_MenuPrefs_Finish, obj, 1);
		DoMethod(tmp.BT_Save				, MUIM_Notify, MUIA_Pressed				, FALSE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_MenuPrefs_Finish, obj, 2);
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG MenuPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW												: return(MenuPrefs_New							(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_TriggerMenu					: return(MenuPrefs_TriggerMenu				(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_LoadMenus						: return(MenuPrefs_LoadMenus					(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_NewEntry						: return(MenuPrefs_NewEntry					(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_MenuList_Active				: return(MenuPrefs_MenuList_Active			(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_MenuList_ChangeLine		: return(MenuPrefs_MenuList_ChangeLine		(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_NewProgram					: return(MenuPrefs_NewProgram					(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_ProgramList_Active			: return(MenuPrefs_ProgramList_Active		(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_ProgramList_ChangeLine	: return(MenuPrefs_ProgramList_ChangeLine	(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_Asynch_Active				: return(MenuPrefs_Asynch_Active				(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_Type_Active					: return(MenuPrefs_Type_Active				(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_GetProgramList				: return(MenuPrefs_GetProgramList			(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}
