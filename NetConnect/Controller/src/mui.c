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

		DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE ,
			MUIV_Notify_Application, 5, MUIM_Application_PushMethod,
			win, 2, MUIM_IconBar_About_Finish, obj);
		DoMethod(data->BT_Button, MUIM_Notify, MUIA_Pressed, FALSE ,
			MUIV_Notify_Application, 5, MUIM_Application_PushMethod,
			win, 2, MUIM_IconBar_About_Finish, obj);
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
/* Icon List class                                                          */
/****************************************************************************/

/*
 * Decide what to do when MUI tells us an
 * object is beeing dragged into the list.
 */

ULONG IconList_DragQuery(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
	if(msg->obj == obj)										/* If somebody tried to drag ourselves onto ourselves, we let our superclass (the list class) handle the necessary actions.*/
		return(DoSuperMethodA(cl, obj, msg));
	else
	{
		 if(msg->obj == (Object *)muiUserData(obj))	/* If our predefined source object (the other list) wants us to become active, we politely accept it. */
			return(MUIV_DragQuery_Accept);
		else
			return(MUIV_DragQuery_Refuse);				/* Everything else is beeing rejected */
	}
}


/*
 * An object was dropped over the list.
 * Do the necessary actions (move from
 * one list to the other).
 */

ULONG IconList_DragDrop(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
	if(msg->obj == obj)										/* the source is the same as the destination => */
		return(DoSuperMethodA(cl, obj, msg));			/*  let MUI handle the moving of objects within the list */
	else
	{
		struct Icon *icon;
		LONG dropmark;
		LONG pos, state;

		/* make the lists silent while the entries are beeing moved */
		set(obj		, MUIA_List_Quiet, TRUE);
		set(msg->obj, MUIA_List_Quiet, TRUE);

		/* copy the entries to the dest. by iterating through the source's selected entries */
		pos = MUIV_List_NextSelected_Start;				/* copy all selected entries and the active entry before we delete anything */
		dropmark = xget(obj, MUIA_List_DropMark);		/* find out where to include the entries in the destination */
		FOREVER
		{
			DoMethod(msg->obj, MUIM_List_NextSelected, &pos);
			if(pos == MUIV_List_NextSelected_End)
				break;

			DoMethod(msg->obj, MUIM_List_GetEntry, pos, &icon);
			DoMethod(obj, MUIM_List_InsertSingle, icon, dropmark++);
		}

		/* delete entries in the source */
		pos	= MUIV_List_GetEntry_Active;				/* we have to delete the "active" entry first (because its state is not "selected") */
		state	= 1;												/* because we use the active entry first we have to fake the selection state */
		FOREVER
		{
			DoMethod(msg->obj, MUIM_List_GetEntry, pos, &icon);
			if(!icon)
				break;

			if(state)											/* is the current entry selected */
			{
				icon->bodychunk	= NULL;						/* zero these pointers so they don't get cleared */
				icon->list			= NULL;						/* by PrefsIconList_Desctruct */
				icon->body			= NULL;
				icon->cols			= NULL;
				icon->bmhd			= NULL;
				DoMethod(msg->obj, MUIM_List_Remove, pos);
				pos = 0;											/* start over again because we lost an entry */
			}
			else
				pos++;											/* otherwise go to the next entry */

			DoMethod(msg->obj, MUIM_List_Select, pos, MUIV_List_Select_Ask, &state);	/* is it selected ? */
		}

		/* The above 2-way method is necessary because we can't delete an object in the list
		**	while we scan it for selected items. So deletion is done in the second part but
		**	we have to find out which entries are selected in a different way to get the
		**	correct entries deleted => MUIM_List_Select is used but this we we haven't got
		**	the active entry (the last one we clicked on) included so we have to delete the
		**	MUIV_List_GetEntry_Active entry first and then scan the list with
		**	MUIV_List_Select_Ask. A bit complicated but it works and if I were you I wouldn't
		** change it :)
		*/


		/*
		** make the insterted object the active and make the source listviews
		** active object inactive to give some more visual feedback to the user.
		*/
		get(obj, MUIA_List_InsertPosition, &dropmark);
		set(obj, MUIA_List_Active			, dropmark);
		set(msg->obj, MUIA_List_Active	, MUIV_List_Active_Off);

		/* and now make the changes visible */
		set(obj		, MUIA_List_Quiet, FALSE);
		set(msg->obj, MUIA_List_Quiet, FALSE);

		return(NULL);
	}
}


SAVEDS ASM struct Icon *IconList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Icon *src)
{
	struct Icon *new;

	if((new = (struct Icon *)AllocVec(sizeof(struct Icon), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct Icon));
	return(new);
}

SAVEDS ASM VOID IconList_DestructFunc(REG(a2) APTR pool, REG(a1) struct Icon *icon)
{
	if(icon)
	{
		if(icon->bodychunk)
			MUI_DisposeObject(icon->bodychunk);
		if(icon->body)
			FreeVec(icon->body);
		if(icon->cols)
			FreeVec(icon->cols);
		if(icon->bmhd)
			FreeVec(icon->bmhd);
		FreeVec(icon);
	}
}

SAVEDS ASM LONG IconList_DisplayFunc(REG(a0) struct Hook *hook, REG(a2) char **array, REG(a1) struct Icon *icon)
{
	if(icon)
	{
		static char buf[MAXPATHLEN];

		sprintf(buf,"\33O[%08lx]", icon->list);	/* show the image */
		*array++ = buf;
		*array   = icon->Name;							/* show the name */
	}
	return(NULL);
}

ULONG IconList_New(struct IClass *cl,Object *obj,struct opSet *msg)
{
	static const struct Hook IconList_DisplayHook	= { { 0,0 }, (VOID *)IconList_DisplayFunc		, NULL, NULL };
	static const struct Hook IconList_ConstructHook	= { { 0,0 }, (VOID *)IconList_ConstructFunc	, NULL, NULL };
	static const struct Hook IconList_DestructHook	= { { 0,0 }, (VOID *)IconList_DestructFunc	, NULL, NULL };
	LONG max;

	max = find_max() + 6;
	if(max > 200)
		max = 200;

	obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Frame					, MUIV_Frame_InputList,
		MUIA_List_DisplayHook	, &IconList_DisplayHook,
		MUIA_List_ConstructHook	, &IconList_ConstructHook,
		MUIA_List_DestructHook	, &IconList_DestructHook,
		MUIA_List_Format			, ",",
		MUIA_List_MinLineHeight	, max,
		MUIA_List_AutoVisible	, TRUE,
		MUIA_List_DragSortable	, TRUE,
		TAG_MORE, msg->ops_AttrList);

	return((ULONG)obj);
}

ULONG IconList_Cleanup(struct IClass *cl, Object *obj, Msg msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl,obj);
	struct Icon *icon;
	LONG pos;

	if(!DoSuperMethodA(cl, obj, msg))
		return(FALSE);

	/* Free the images which were created in the
	** setup. Zero the variables !! So they're not
	** used twice since there are two lists and
	** CreateImage was issued only on one.
	*/
	pos = 0;
	FOREVER
	{
		DoMethod(data->LI_ActiveIcons, MUIM_List_GetEntry, pos++, &icon);
		if(!icon)
			break;
		if(icon->list)
			DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
		icon->list = NULL;
	}
	pos = 0;
	FOREVER
	{
		DoMethod(data->LI_InactiveIcons, MUIM_List_GetEntry, pos++, &icon);
		if(!icon)
			break;
		if(icon->list)
			DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
		icon->list = NULL;
	}
	return(DoSuperMethodA(cl, obj, msg));
}

SAVEDS ASM ULONG IconList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg)
{
	switch(msg->MethodID)
	{
		case OM_NEW				: return(IconList_New  	  	(cl, obj, (APTR)msg));
		case MUIM_Cleanup		: return(IconList_Cleanup	(cl, obj, (APTR)msg));
		case MUIM_DragQuery	: return(IconList_DragQuery(cl, obj, (APTR)msg));
		case MUIM_DragDrop	: return(IconList_DragDrop	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Icon Bar Prefs class                                                     */
/****************************************************************************/

ULONG IconBarPrefs_LoadIcons(struct IClass *cl, Object *obj, Msg msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon;
	struct IFFHandle	*Handle;
	struct ContextNode *cn;
	BOOL anything = FALSE;
	LONG number;

	set(obj, MUIA_Window_Sleep, TRUE);
	set(data->LI_ActiveIcons	, MUIA_List_Quiet, TRUE);
	set(data->LI_InactiveIcons	, MUIA_List_Quiet, TRUE);

	if(icon = AllocVec(sizeof(struct Icon), MEMF_ANY | MEMF_CLEAR))
	{
		if(Handle = AllocIFF())
		{
			if(Handle->iff_Stream = Open("ENV:NetConnectPrefs", MODE_OLDFILE))
			{
				InitIFFasDOS(Handle);
				if(!(OpenIFF(Handle, IFFF_READ)))
				{
					if(!(StopChunks(Handle, Stops, NUM_STOPS)))
					{
						while(!ParseIFF(Handle, IFFPARSE_SCAN))
						{
							cn = CurrentChunk(Handle);

							if(cn->cn_ID == ID_ROWS)
							{
								ReadChunkBytes(Handle, &number, MIN(sizeof(LONG), cn->cn_Size));
								setslider(data->SL_Rows, number);
							}

							if(cn->cn_ID == ID_BTTY)
							{
								ReadChunkBytes(Handle, &number, MIN(sizeof(LONG), cn->cn_Size));
								setcycle(data->CY_ButtonType, number);
							}

							if(cn->cn_ID == ID_WINT)
							{
								ReadChunkBytes(Handle, &number, MIN(sizeof(LONG), cn->cn_Size));
								setcycle(data->CY_WindowType, number);
								set(data->CY_WindowType, MUIA_UserData, number);
							}

							if(cn->cn_ID == ID_AICN || cn->cn_ID == ID_IICN)
							{
								if(ReadChunkBytes(Handle, icon, MIN(sizeof(struct Icon), cn->cn_Size)) == MIN(sizeof(struct Icon), cn->cn_Size))
								{
									init_icon(icon, data->LI_ActiveIcons);
									DoMethod((cn->cn_ID == ID_AICN ? data->LI_ActiveIcons : data->LI_InactiveIcons), MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
									anything = TRUE;
								}
							}
						}
					}
					CloseIFF(Handle);
				}
				Close(Handle->iff_Stream);
			}
			FreeIFF(Handle);
		}
		FreeVec(icon);
	}
	set(data->LI_ActiveIcons, MUIA_List_Quiet, FALSE);
	set(data->LI_InactiveIcons, MUIA_List_Quiet, FALSE);

	number = xget(data->SL_Rows, MUIA_Numeric_Value);
	set(data->SL_Rows, MUIA_Slider_Max, (xget(data->LI_ActiveIcons, MUIA_List_Entries) ? xget(data->LI_ActiveIcons, MUIA_List_Entries) : 1));
	setslider(data->SL_Rows, number);
	set(obj, MUIA_Window_Sleep, FALSE);

	if(!anything)
		DoMethod(obj, MUIM_IconBarPrefs_Reset);

	return(NULL);
}

ULONG IconBarPrefs_Reset(struct IClass *cl, Object *obj, Msg msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon;
	LONG pos;

	set(obj, MUIA_Window_Sleep, TRUE);
	set(data->LI_ActiveIcons	, MUIA_List_Quiet, TRUE);
	set(data->LI_InactiveIcons	, MUIA_List_Quiet, TRUE);

	pos = 0;
	FOREVER
	{
		DoMethod(data->LI_ActiveIcons, MUIM_List_GetEntry, pos++, &icon);
		if(!icon)
			break;
		if(icon->list)
			DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
		icon->list = NULL;
	}
	pos = 0;
	FOREVER
	{
		DoMethod(data->LI_InactiveIcons, MUIM_List_GetEntry, pos++, &icon);
		if(!icon)
			break;
		if(icon->list)
			DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
		icon->list = NULL;
	}
	DoMethod(data->LI_ActiveIcons		, MUIM_List_Clear);
	DoMethod(data->LI_InactiveIcons	, MUIM_List_Clear);

	if(icon = AllocVec(sizeof(struct Icon), MEMF_ANY | MEMF_CLEAR))
	{
		icon->Volume = 64;

		strcpy(icon->Name, "WWW");
		strcpy(icon->ImageFile, IMAGE_PATH"WWW");
		strcpy(icon->Program, PROGRAM_PATH"Voyager");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Telnet");
		strcpy(icon->ImageFile, IMAGE_PATH"Telnet");
		strcpy(icon->Program, PROGRAM_PATH"Telnet");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "FTP");
		strcpy(icon->ImageFile, IMAGE_PATH"FTP");
		strcpy(icon->Program, PROGRAM_PATH"mFTP");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Ping");
		strcpy(icon->ImageFile, IMAGE_PATH"Ping");
		strcpy(icon->Program, PROGRAM_PATH"Ping");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Traceroute");
		strcpy(icon->ImageFile, IMAGE_PATH"Traceroute");
		strcpy(icon->Program, PROGRAM_PATH"Traceroute");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Finger");
		strcpy(icon->ImageFile, IMAGE_PATH"Finger");
		strcpy(icon->Program, PROGRAM_PATH"Finger");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "IRC");
		strcpy(icon->ImageFile, IMAGE_PATH"IRC");
		strcpy(icon->Program, PROGRAM_PATH"Irc");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Mail");
		strcpy(icon->ImageFile, IMAGE_PATH"Mail");
		strcpy(icon->Program, PROGRAM_PATH"Mail");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "News");
		strcpy(icon->ImageFile, IMAGE_PATH"News");
		strcpy(icon->Program, PROGRAM_PATH"News");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Search");
		strcpy(icon->ImageFile, IMAGE_PATH"Search");
		strcpy(icon->Program, PROGRAM_PATH"Search");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Docs");
		strcpy(icon->ImageFile, IMAGE_PATH"Docs");
		strcpy(icon->Program, PROGRAM_PATH"Docs");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Misc");
		strcpy(icon->ImageFile, IMAGE_PATH"Misc");
		strcpy(icon->Program, PROGRAM_PATH"Misc");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Start/Stop");
		strcpy(icon->ImageFile, IMAGE_PATH"Start");
		strcpy(icon->Program	, "AmiTCP:bin/startnet");
		icon->Type	= 2;
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Top);

		FreeVec(icon);
	}

	set(data->LI_ActiveIcons	, MUIA_List_Quiet, FALSE);
	set(data->LI_InactiveIcons	, MUIA_List_Quiet, FALSE);

	pos = xget(data->SL_Rows, MUIA_Numeric_Value);
	set(data->SL_Rows, MUIA_Slider_Max, (xget(data->LI_ActiveIcons, MUIA_List_Entries) ? xget(data->LI_ActiveIcons, MUIA_List_Entries) : 1));
	setslider(data->SL_Rows, pos);
	set(obj, MUIA_Window_Sleep, FALSE);

	return(NULL);
}

BOOL find_list(struct IconBarPrefs_Data *data, Object **list, struct Icon **icon)
{
	struct Icon *tmp_icon;

	DoMethod(data->LI_ActiveIcons, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &tmp_icon);
	if(tmp_icon)
	{
		*icon = tmp_icon;
		*list = data->LI_ActiveIcons;
		return(TRUE);
	}
	else
	{
		DoMethod(data->LI_InactiveIcons, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &tmp_icon);
		if(tmp_icon)
		{
			*icon = tmp_icon;
			*list = data->LI_InactiveIcons;
			return(TRUE);
		}
	}
	return(FALSE);
}

ULONG IconBarPrefs_EditIcon(struct IClass *cl, Object *obj, Msg msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);

	struct Icon *icon;
	Object *list;

	if(find_list(data, &list, &icon))
	{
		if(icon->edit_window)
		{
			DoMethod(icon->edit_window, MUIM_Window_ToFront);
		}
		else
		{
			set(app, MUIA_Application_Sleep, TRUE);
			if(icon->edit_window = (Object *)NewObject(CL_EditIcon->mcc_Class, NULL,
				MUIA_NetConnect_Icon, icon,
				MUIA_NetConnect_List, list,
				MUIA_NetConnect_Originator, obj,
				TAG_DONE))
			{
				DoMethod(app, OM_ADDMEMBER, icon->edit_window);
				set(icon->edit_window, MUIA_Window_Open, TRUE);
			}
			set(app, MUIA_Application_Sleep, FALSE);
		}
	}

	return(NULL);
}

ULONG IconBarPrefs_EditIcon_Finish(struct IClass *cl, Object *obj, struct MUIP_IconBarPrefs_EditIcon_Finish *msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon = msg->icon;
	struct EditIcon_Data *ei_data = INST_DATA(CL_EditIcon->mcc_Class, icon->edit_window);
	Object *list = msg->list;

	editor_checksave(icon->Program, ei_data->LI_Editor);

	if(msg->use)
	{
		strcpy(icon->Name		, (STRPTR)xget(ei_data->STR_Name		, MUIA_String_Contents));
		strcpy(icon->Program	, (STRPTR)xget(ei_data->PA_Program	, MUIA_String_Contents));
		strcpy(icon->Hotkey	, (STRPTR)xget(ei_data->STR_Hotkey	, MUIA_String_Contents));
		strcpy(icon->Sound	, (STRPTR)xget(ei_data->PA_Sound		, MUIA_String_Contents));
		icon->Type		= xget(ei_data->CY_Type		, MUIA_Cycle_Active);
		icon->Volume	= xget(ei_data->SL_Volume	, MUIA_Numeric_Value);

// the following MUST to be at the end because after that the contents of "icon" are no longer valid !!!

		if(strcmp(icon->ImageFile, (STRPTR)xget(ei_data->PA_Image, MUIA_String_Contents)))
		{
			struct Icon *new_icon;
			LONG position = MUIV_List_Insert_Active;

			if(xget(list, MUIA_List_Entries))
			{
				DoMethod(list, MUIM_List_GetEntry, xget(list, MUIA_List_Entries) - 1, &new_icon);
				if(new_icon == icon)
					position = MUIV_List_Insert_Bottom;
			}

			if(new_icon = AllocVec(sizeof(struct Icon), MEMF_ANY))
			{
				nnset(list, MUIA_List_Quiet, TRUE);
				memcpy(new_icon, icon, sizeof(struct Icon));
				strcpy(new_icon->ImageFile, (STRPTR)xget(ei_data->PA_Image, MUIA_String_Contents));

				if(icon->list)
					DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
				icon->list = NULL;
				DoMethod(list, MUIM_List_Remove, MUIV_List_Remove_Active);

				init_icon(new_icon, data->LI_ActiveIcons);
				DoMethod(list, MUIM_List_InsertSingle, new_icon, position);
				set(list, MUIA_List_Active, xget(list, MUIA_List_InsertPosition));
				nnset(list, MUIA_List_Quiet, FALSE);

				FreeVec(new_icon);
			}
			set(obj, MUIA_Window_Sleep, FALSE);
		}
		else
			DoMethod(list, MUIM_List_Redraw, MUIV_List_Redraw_Active);
	}

	set(icon->edit_window, MUIA_Window_Open, FALSE);
	DoMethod(app, OM_REMMEMBER, icon->edit_window);
	MUI_DisposeObject(icon->edit_window);
	icon->edit_window = NULL;

	return(NULL);
}

ULONG IconBarPrefs_NewIcon(struct IClass *cl, Object *obj, Msg msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon;

	if(icon = AllocVec(sizeof(struct Icon), MEMF_ANY | MEMF_CLEAR))
	{
		strcpy(icon->Name, GetStr(MSG_TX_New));
		strcpy(icon->Program, PROGRAM_PATH);
		strcpy(icon->ImageFile, IMAGE_PATH);
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		set(data->LI_InactiveIcons, MUIA_List_Active, xget(data->LI_InactiveIcons, MUIA_List_InsertPosition));
		DoMethod(obj, MUIM_IconBarPrefs_EditIcon);
		FreeVec(icon);
	}

	return(NULL);
}

ULONG IconBarPrefs_DeleteIcon(struct IClass *cl, Object *obj, Msg msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon;
	Object *list;
	LONG pos;

	if(find_list(data, &list, &icon))
	{
		if(icon && list)
		{
			if(icon->list)
				DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
			icon->list = NULL;
			DoMethod(list, MUIM_List_Remove, MUIV_List_Remove_Active);
			pos = xget(data->SL_Rows, MUIA_Numeric_Value);
			set(data->SL_Rows, MUIA_Slider_Max, (xget(data->LI_ActiveIcons, MUIA_List_Entries) > 0 ? xget(data->LI_ActiveIcons, MUIA_List_Entries) : 1));
			setslider(data->SL_Rows, pos);
		}
	}

	return(NULL);
}

ULONG IconBarPrefs_List_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon;
	Object *list;
	LONG max;

	max = xget(data->LI_ActiveIcons, MUIA_List_Entries);
	if(max != xget(data->SL_Rows, MUIA_Slider_Max))
	{
		LONG pos;

		pos = xget(data->SL_Rows, MUIA_Numeric_Value);
		set(data->SL_Rows, MUIA_Slider_Max, (max > 0 ? max : 1));
		setslider(data->SL_Rows, pos);
	}

	if(find_list(data, &list, &icon))
	{
		set(data->BT_Delete, MUIA_Disabled, FALSE);
		set(data->BT_Edit, MUIA_Disabled, FALSE);
	}
	else
	{
		set(data->BT_Delete, MUIA_Disabled, TRUE);
		set(data->BT_Edit, MUIA_Disabled, TRUE);
	}

	return(NULL);
}

ULONG IconBarPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct IconBarPrefs_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title		, GetStr(MSG_WI_IconBarPrefs),
		MUIA_Window_ID			, MAKE_ID('I','P','R','F'),
//		MUIA_Window_AppWindow, TRUE,
		MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, IconBarPrefsMenu, 0),
		WindowContents		, VGroup,
			MUIA_HelpNode, "WI_IconBarPrefs",
			MUIA_Frame, MUIV_Frame_Group,
			Child, HGroup,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_InactiveIcons = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_FrameTitle				, GetStr(MSG_LV_ButtonBankTitle),
						MUIA_Listview_MultiSelect	, MUIV_Listview_MultiSelect_Default,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_DoubleClick	, TRUE,
						MUIA_Listview_List			, tmp.LI_InactiveIcons = NewObject(CL_IconList->mcc_Class, NULL, TAG_DONE),
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_New		= MakeButton(MSG_BT_New),
						Child, tmp.BT_Edit	= MakeButton("  _Edit"),
						Child, tmp.BT_Delete = MakeButton("  _Delete"),
					End,
				End,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_ActiveIcons = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_FrameTitle				, GetStr(MSG_LV_IconBarTitle),
						MUIA_Listview_MultiSelect	, MUIV_Listview_MultiSelect_Default,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_DoubleClick	, TRUE,
						MUIA_Listview_List			, tmp.LI_ActiveIcons = NewObject(CL_IconList->mcc_Class, NULL, TAG_DONE),
					End,
					Child, HGroup,
						Child, MakeKeyLabel2(MSG_LA_Rows, MSG_CC_Rows),
						Child, tmp.SL_Rows = MakeKeySlider(1, 10, 1, MSG_CC_Rows),
					End,
				End,
			End,
			Child, ColGroup(2),
				Child, Label("Show buttons as :"),
				Child, tmp.CY_ButtonType = Cycle(ARR_ButtonTypes),
				Child, Label("Window Type :"),
				Child, tmp.CY_WindowType = Cycle(ARR_WindowTypes),
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
		struct IconBarPrefs_Data *data = INST_DATA(cl, obj);

		*data = tmp;

		set(tmp.BT_Delete, MUIA_Disabled, TRUE);
		set(tmp.BT_Edit, MUIA_Disabled, TRUE);

		set(tmp.LI_InactiveIcons, MUIA_UserData, tmp.LI_ActiveIcons);	/* show the lists from whom they have to accept drag requests */
		set(tmp.LI_ActiveIcons, MUIA_UserData, tmp.LI_InactiveIcons);

		set(tmp.LV_InactiveIcons, MUIA_ShortHelp, GetStr(MSG_HELP_InactiveIcons));
		set(tmp.LV_ActiveIcons	, MUIA_ShortHelp, GetStr(MSG_HELP_ActiveIcons));
		set(tmp.BT_New				, MUIA_ShortHelp, GetStr(MSG_HELP_New));
		set(tmp.BT_Delete			, MUIA_ShortHelp, GetStr(MSG_HELP_Remove));
		set(tmp.SL_Rows			, MUIA_ShortHelp, GetStr(MSG_HELP_Rows));
		set(tmp.BT_Save			, MUIA_ShortHelp, GetStr(MSG_HELP_Save));
		set(tmp.BT_Use				, MUIA_ShortHelp, GetStr(MSG_HELP_Use));
		set(tmp.BT_Cancel			, MUIA_ShortHelp, GetStr(MSG_HELP_Cancel));

		DoMethod(tmp.LV_ActiveIcons	, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, tmp.LV_InactiveIcons	, 3, MUIM_NoNotifySet, MUIA_List_Active, MUIV_List_Active_Off);
		DoMethod(tmp.LV_InactiveIcons	, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, tmp.LV_ActiveIcons		, 3, MUIM_NoNotifySet, MUIA_List_Active, MUIV_List_Active_Off);
		DoMethod(tmp.LV_ActiveIcons	, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 1, MUIM_IconBarPrefs_List_Active);
		DoMethod(tmp.LV_InactiveIcons	, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 1, MUIM_IconBarPrefs_List_Active);
		DoMethod(tmp.LV_ActiveIcons	, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime, obj, 1, MUIM_IconBarPrefs_EditIcon);
		DoMethod(tmp.LV_InactiveIcons	, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime, obj, 1, MUIM_IconBarPrefs_EditIcon);
		DoMethod(tmp.BT_New				, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 1, MUIM_IconBarPrefs_NewIcon);
		DoMethod(tmp.BT_Delete			, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 1, MUIM_IconBarPrefs_DeleteIcon);
		DoMethod(tmp.BT_Edit				, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 1, MUIM_IconBarPrefs_EditIcon);

		DoMethod(obj						, MUIM_Notify, MUIA_Window_CloseRequest, TRUE			, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_IconBarPrefs_Finish, obj, 0);
		DoMethod(tmp.BT_Cancel			, MUIM_Notify, MUIA_Pressed			, FALSE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_IconBarPrefs_Finish, obj, 0);
		DoMethod(tmp.BT_Use				, MUIM_Notify, MUIA_Pressed			, FALSE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_IconBarPrefs_Finish, obj, 1);
		DoMethod(tmp.BT_Save				, MUIM_Notify, MUIA_Pressed			, FALSE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_IconBarPrefs_Finish, obj, 2);

		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_RESET)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 2, MUIM_IconBarPrefs_Reset, 0);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_MUI2)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG IconBarPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(IconBarPrefs_New					(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_LoadIcons			: return(IconBarPrefs_LoadIcons			(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_Reset				: return(IconBarPrefs_Reset				(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_NewIcon			: return(IconBarPrefs_NewIcon				(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_DeleteIcon		: return(IconBarPrefs_DeleteIcon			(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_EditIcon			: return(IconBarPrefs_EditIcon			(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_EditIcon_Finish	: return(IconBarPrefs_EditIcon_Finish	(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_List_Active		: return(IconBarPrefs_List_Active		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Editor Class  (List)                                                     */
/****************************************************************************/

ULONG Editor_DragQuery(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
	if(msg->obj == obj)
		return(DoSuperMethodA(cl, obj, msg));
	else
		return(MUIV_DragQuery_Refuse);
}

ULONG Editor_DragDrop(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
	if(msg->obj == obj)
	{
		set(obj, MUIA_UserData, 1);
		return(DoSuperMethodA(cl, obj, msg));
	}

	return(NULL);
}


ULONG Editor_New(struct IClass *cl,Object *obj,struct opSet *msg)
{
	obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Frame					, MUIV_Frame_InputList,
		MUIA_List_AutoVisible	, TRUE,
		MUIA_List_DragSortable	, TRUE,
		MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
		MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
		TAG_MORE, msg->ops_AttrList);

	return((ULONG)obj);
}

SAVEDS ASM ULONG Editor_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg)
{
	switch(msg->MethodID)
	{
		case OM_NEW				: return(Editor_New  	  	(cl, obj, (APTR)msg));
		case MUIM_DragQuery	: return(Editor_DragQuery	(cl, obj, (APTR)msg));
		case MUIM_DragDrop	: return(Editor_DragDrop	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Edit Icon Class  (WINDOW                                                 */
/****************************************************************************/

ULONG EditIcon_Editor_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct EditIcon_Data *data = INST_DATA(cl, obj);
	STRPTR ptr;

	DoMethod(data->LI_Editor, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
	if(ptr)
	{
		set(data->STR_Line, MUIA_Disabled, FALSE);
		set(data->BT_Delete, MUIA_Disabled, FALSE);
		setstring(data->STR_Line, ptr);
	}
	else
	{
		set(data->STR_Line, MUIA_Disabled, TRUE);
		set(data->BT_Delete, MUIA_Disabled, TRUE);
		setstring(data->STR_Line, "");
	}

	return(NULL);
}

ULONG EditIcon_ChangeLine(struct IClass *cl, Object *obj, Msg msg)
{
	struct EditIcon_Data *data = INST_DATA(cl, obj);
	LONG i;

	i = xget(data->LI_Editor, MUIA_List_Active);
	if(i != MUIV_List_Active_Off)
	{
		set(data->LI_Editor, MUIA_List_Quiet, TRUE);
		DoMethod(data->LI_Editor, MUIM_List_InsertSingle, xget(data->STR_Line, MUIA_String_Contents), i + 1);
		DoMethod(data->LI_Editor, MUIM_List_Remove, i);
		set(data->LI_Editor, MUIA_List_Quiet, FALSE);
		set((Object *)xget(data->STR_Line, MUIA_WindowObject), MUIA_Window_ActiveObject, data->STR_Line);
		set(data->LI_Editor, MUIA_UserData, 1);
	}

	return(NULL);
}


ULONG EditIcon_Type_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct EditIcon_Data *data = INST_DATA(cl, obj);
	int type;

	type = xget(data->CY_Type, MUIA_Cycle_Active);
	if(type == 2 || type == 3)
	{
		set(data->GR_Script, MUIA_Disabled, FALSE);
		editor_load((STRPTR)xget(data->PA_Program, MUIA_String_Contents), data->LI_Editor);
	}
	else
	{
		DoMethod(data->LI_Editor, MUIM_List_Clear);
		setstring(data->STR_Line, "");
		set(data->LI_Editor, MUIA_UserData, NULL);
		set(data->GR_Script, MUIA_Disabled, TRUE);
	}
	return(NULL);
}

ULONG EditIcon_Sound_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct EditIcon_Data *data = INST_DATA(cl, obj);
	BPTR lock;
	STRPTR ptr;

	lock = Lock((STRPTR)xget(data->PA_Sound, MUIA_String_Contents), ACCESS_READ);
	ptr = (STRPTR)xget(data->PA_Sound, MUIA_String_Contents);
	if(lock && ptr && *ptr)
	{
		set(data->BT_PlaySound, MUIA_Disabled, FALSE);
		set(data->SL_Volume, MUIA_Disabled, FALSE);
	}
	else
	{
		set(data->BT_PlaySound, MUIA_Disabled, TRUE);
		set(data->SL_Volume, MUIA_Disabled, TRUE);
	}

	if(lock)
		UnLock(lock);

	return(NULL);
}

ULONG EditIcon_Program_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct EditIcon_Data *data = INST_DATA(cl, obj);
	BPTR lock;
	STRPTR ptr;

	lock = Lock((STRPTR)xget(data->PA_Program, MUIA_String_Contents), ACCESS_READ);
	ptr = (STRPTR)xget(data->PA_Program, MUIA_String_Contents);
	if(lock && ptr && *ptr)
	{
		set(data->STR_Hotkey, MUIA_Disabled, FALSE);
	}
	else
	{
		set(data->STR_Hotkey, MUIA_Disabled, TRUE);
	}
	DoMethod(obj, MUIM_EditIcon_Type_Active);

	if(lock)
		UnLock(lock);

	return(NULL);
}

ULONG EditIcon_PlaySound(struct IClass *cl, Object *obj, Msg msg)
{
	struct EditIcon_Data *data = INST_DATA(cl, obj);

	play_sound((STRPTR)xget(data->PA_Sound, MUIA_String_Contents), xget(data->SL_Volume, MUIA_Numeric_Value));
	return(NULL);
}

ULONG EditIcon_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct EditIcon_Data tmp;
	Object *list, *originator;
	struct Icon *icon;

	STR_GR_Register[0] = GetStr(MSG_GR_ButtonConrtolTitle);
	STR_GR_Register[1] = GetStr(MSG_GR_ScriptEditTitle);
	STR_GR_Register[2] = NULL;

	icon = (struct Icon *)GetTagData(MUIA_NetConnect_Icon, (ULONG)"", msg->ops_AttrList);
	list = (Object *)GetTagData(MUIA_NetConnect_List, (ULONG)"", msg->ops_AttrList);
	originator = (Object *)GetTagData(MUIA_NetConnect_Originator, (ULONG)"", msg->ops_AttrList);

	if(!icon || !list)
		return(NULL);

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title		, "Edit Icon",
		MUIA_Window_ID			, MAKE_ID('I','E','D','I'),
		MUIA_Window_AppWindow, TRUE,
		WindowContents		, VGroup,
			Child, tmp.GR_Register = RegisterObject,
				MUIA_Background, MUII_RegisterBack,
				MUIA_Register_Titles, STR_GR_Register,
				MUIA_CycleChain, 1,
				Child, tmp.GR_Button = ColGroup(2),
					MUIA_HelpNode	, "GR_IconConrtol",
					Child, MakeKeyLabel2(MSG_LA_Name, MSG_CC_Name),
					Child, tmp.STR_Name = MakeKeyString(icon->Name, 80, MSG_CC_Name),
					Child, MakeKeyLabel2(MSG_LA_Hotkey, MSG_CC_Hotkey),
					Child, tmp.STR_Hotkey = MakeKeyString(icon->Hotkey, 80, MSG_CC_Hotkey),
					Child, MakeKeyLabel2(MSG_LA_ProgramType, MSG_CC_ProgramType),
					Child, tmp.CY_Type = MakeKeyCycle(ARR_ProgramTypes, MSG_CC_ProgramType),
					Child, MakeKeyLabel2(MSG_LA_Program, MSG_CC_Program),
					Child, tmp.PA_Program = MakePopAsl(MakeKeyString(icon->Program, MAXPATHLEN, MSG_CC_Program), MSG_LA_Program, FALSE),
					Child, MakeKeyLabel2(MSG_LA_Image, MSG_CC_Image),
					Child, tmp.PA_Image = MakePopAsl(MakeKeyString(icon->ImageFile, MAXPATHLEN, MSG_CC_Image), MSG_LA_Image, FALSE),
					Child, MakeKeyLabel2(MSG_LA_Sound, MSG_CC_Sound),
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.PA_Sound = MakePopAsl(MakeKeyString(icon->Sound, MAXPATHLEN, MSG_CC_Sound), MSG_LA_Sound, FALSE),
						Child, tmp.BT_PlaySound = ImageObject,
							ImageButtonFrame,
							MUIA_CycleChain		, 1,
							MUIA_InputMode			, MUIV_InputMode_RelVerify,
							MUIA_Image_Spec		, MUII_TapePlay,
							MUIA_Image_FreeVert	, TRUE,
							MUIA_Background		, MUII_ButtonBack,
						End,
					End,
					Child, MakeKeyLabel2(MSG_LA_Volume, MSG_CC_Volume),
					Child, tmp.SL_Volume = MakeKeySlider(0, 64, icon->Volume, MSG_CC_Volume),
				End,
				Child, tmp.GR_Script = VGroup,
					MUIA_Disabled		, TRUE,
					MUIA_HelpNode		, "GR_ScriptEdit",
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_Editor = ListviewObject,
						MUIA_CycleChain, 1,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_Editor = NewObject(CL_Editor->mcc_Class, NULL, TAG_DONE),
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_New		= MakeButton("  _New"),
						Child, tmp.BT_Delete	= MakeButton("  _Delete"),
						Child, tmp.BT_Clear	= MakeButton("  C_lear"),
					End,
					Child, tmp.STR_Line = MakeKeyString("", MAXPATHLEN, "   "),
				End,
			End,
			Child, HGroup,
				MUIA_Group_SameSize	, TRUE,
				Child, tmp.BT_Okay	= MakeButton("  _Okay"),
				Child, HSpace(0),
				Child, tmp.BT_Cancel	= MakeButton(MSG_BT_Cancel),
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct EditIcon_Data *data = INST_DATA(cl, obj);
		static const struct Hook AppMsgHook = { {NULL, NULL}, (VOID *)AppMsgFunc, NULL, NULL };
		static const struct Hook Editor_AppMsgHook = { {NULL, NULL}, (VOID *)Editor_AppMsgFunc, NULL, NULL };

		*data = tmp;


		set(tmp.STR_Name			, MUIA_ShortHelp, GetStr(MSG_HELP_Name));
		set(tmp.CY_Type			, MUIA_ShortHelp, GetStr(MSG_HELP_Type));
		set(tmp.PA_Program		, MUIA_ShortHelp, GetStr(MSG_HELP_Program));
		set(tmp.STR_Hotkey		, MUIA_ShortHelp, GetStr(MSG_HELP_Hotkey));
		set(tmp.PA_Image			, MUIA_ShortHelp, GetStr(MSG_HELP_Image));
		set(tmp.PA_Sound			, MUIA_ShortHelp, GetStr(MSG_HELP_Sound));
		set(tmp.BT_PlaySound		, MUIA_ShortHelp, GetStr(MSG_HELP_PlaySound));
		set(tmp.SL_Volume			, MUIA_ShortHelp, GetStr(MSG_HELP_Volume));
		set(tmp.LV_Editor			, MUIA_ShortHelp, GetStr(MSG_HELP_Editor));
		set(tmp.BT_Clear			, MUIA_ShortHelp, GetStr(MSG_HELP_ClearScript));

		set(tmp.STR_Line, MUIA_String_AttachedList, tmp.LV_Editor);
		set(tmp.STR_Line, MUIA_CycleChain, 1);
		set(tmp.STR_Line, MUIA_Disabled, TRUE);
		set(tmp.BT_Delete, MUIA_Disabled, TRUE);
		set(tmp.STR_Hotkey, MUIA_Disabled, (CxBase ? FALSE : TRUE));

		DoMethod(tmp.LI_Editor, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, obj, 1, MUIM_EditIcon_Editor_Active);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LI_Editor, 3, MUIM_List_InsertSingle, "", MUIV_List_Insert_Bottom);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LI_Editor, 3, MUIM_Set, MUIA_List_Active, MUIV_List_Active_Bottom);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, tmp.STR_Line);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LI_Editor, 3, MUIM_Set, MUIA_UserData, 1);
		DoMethod(tmp.BT_Delete, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LI_Editor, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.BT_Delete, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LI_Editor, 3, MUIM_Set, MUIA_UserData, 1);
		DoMethod(tmp.BT_Clear, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LI_Editor, 1, MUIM_List_Clear);
		DoMethod(tmp.BT_Clear, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LI_Editor, 3, MUIM_Set, MUIA_UserData, 1);
		DoMethod(tmp.STR_Line, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 1, MUIM_EditIcon_ChangeLine);

		DoMethod(tmp.STR_Name			, MUIM_Notify, MUIA_AppMessage	, MUIV_EveryTime		, tmp.STR_Name		, 3, MUIM_CallHook	, &AppMsgHook			, MUIV_TriggerValue);
		DoMethod(tmp.PA_Program			, MUIM_Notify, MUIA_AppMessage	, MUIV_EveryTime		, tmp.PA_Program	, 3, MUIM_CallHook	, &AppMsgHook			, MUIV_TriggerValue);
		DoMethod(tmp.PA_Image			, MUIM_Notify, MUIA_AppMessage	, MUIV_EveryTime		, tmp.PA_Image		, 3, MUIM_CallHook	, &AppMsgHook			, MUIV_TriggerValue);
		DoMethod(tmp.PA_Sound			, MUIM_Notify, MUIA_AppMessage	, MUIV_EveryTime		, tmp.PA_Sound		, 3, MUIM_CallHook	, &AppMsgHook			, MUIV_TriggerValue);
		DoMethod(tmp.LV_Editor			, MUIM_Notify, MUIA_AppMessage	, MUIV_EveryTime		, tmp.LI_Editor	, 3, MUIM_CallHook	, &Editor_AppMsgHook	, MUIV_TriggerValue);

		DoMethod(tmp.CY_Type				, MUIM_Notify, MUIA_Cycle_Active	, MUIV_EveryTime		, obj, 1, MUIM_EditIcon_Type_Active);
		DoMethod(tmp.PA_Program			, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime	, obj, 1, MUIM_EditIcon_Program_Active);
		DoMethod(tmp.PA_Sound			, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime	, obj, 1, MUIM_EditIcon_Sound_Active);
		DoMethod(tmp.BT_PlaySound		, MUIM_Notify, MUIA_Pressed		, FALSE					, obj, 1, MUIM_EditIcon_PlaySound);

		DoMethod(obj						, MUIM_Notify, MUIA_Window_CloseRequest, TRUE			, MUIV_Notify_Application, 8, MUIM_Application_PushMethod, originator, 5, MUIM_IconBarPrefs_EditIcon_Finish, icon, list, 0);
		DoMethod(tmp.BT_Cancel			, MUIM_Notify, MUIA_Pressed			, FALSE				, MUIV_Notify_Application, 8, MUIM_Application_PushMethod, originator, 5, MUIM_IconBarPrefs_EditIcon_Finish, icon, list, 0);
		DoMethod(tmp.BT_Okay				, MUIM_Notify, MUIA_Pressed			, FALSE				, MUIV_Notify_Application, 8, MUIM_Application_PushMethod, originator, 5, MUIM_IconBarPrefs_EditIcon_Finish, icon, list, 1);

// set these here so the methods get triggered
		set(tmp.CY_Type, MUIA_Cycle_Active, icon->Type);
		DoMethod(obj, MUIM_EditIcon_Program_Active);
		DoMethod(obj, MUIM_EditIcon_Sound_Active);
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG EditIcon_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW								: return(EditIcon_New				(cl, obj, (APTR)msg));
		case MUIM_EditIcon_Editor_Active	: return(EditIcon_Editor_Active	(cl, obj, (APTR)msg));
		case MUIM_EditIcon_ChangeLine		: return(EditIcon_ChangeLine		(cl, obj, (APTR)msg));
		case MUIM_EditIcon_Type_Active	: return(EditIcon_Type_Active		(cl, obj, (APTR)msg));
		case MUIM_EditIcon_Sound_Active	: return(EditIcon_Sound_Active	(cl, obj, (APTR)msg));
		case MUIM_EditIcon_Program_Active: return(EditIcon_Program_Active	(cl, obj, (APTR)msg));
		case MUIM_EditIcon_PlaySound		: return(EditIcon_PlaySound		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}


/****************************************************************************/
/* Menu Prefs class (Window)                                                */
/****************************************************************************/

VOID load_menulist(struct IFFHandle *Handle, Object *obj, struct MUIS_Listtree_TreeNode *list, STRPTR menu)
{
	struct ContextNode	*Chunk;

	while(!ParseIFF(Handle,IFFPARSE_SCAN))
	{
		Chunk = CurrentChunk(Handle);
	
		if(Chunk->cn_ID == ID_END)
			break;

		if(Chunk->cn_ID == ID_MENU)
		{
			WORD Size = MIN(40, Chunk->cn_Size);
			char name[41];

			if(ReadChunkBytes(Handle, name, Size) == Size)
				DoMethod(obj, MUIM_Listtree_Insert, name, NULL, list, MUIV_Listtree_Insert_PrevNode_Tail, 0);
			else
				break;
		}
		if(Chunk->cn_ID == ID_NODE)
		{
			WORD Size = MIN(80, Chunk->cn_Size);
			char buf[41];

			if(ReadChunkBytes(Handle, buf, Size) == Size)
			{
				struct MUIS_Listtree_TreeNode *tn;

				if(tn = (struct MUIS_Listtree_TreeNode *)DoMethod(obj, MUIM_Listtree_Insert, buf, NULL, list, MUIV_Listtree_Insert_PrevNode_Tail, TNF_LIST))
					load_menulist(Handle, obj, tn, buf);
			}
			else
				break;
		}
	}
}

ULONG MenuPrefs_LoadMenus(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);
	BOOL success = FALSE;
	struct IFFHandle	*Handle;
	struct ContextNode	*Chunk;
	char name[41];

	if(Handle = AllocIFF())
	{
		if(Handle->iff_Stream = Open("ENV:NetConnectPrefs.menus", MODE_OLDFILE))
		{
			InitIFFasDOS(Handle);
			if(!(OpenIFF(Handle, IFFF_READ)))
			{
				if(!(StopChunks(Handle, Stops, NUM_STOPS)))
				{
					set(obj,MUIA_Listtree_Quiet,TRUE);

					while(!ParseIFF(Handle, IFFPARSE_SCAN))
					{
						Chunk = CurrentChunk(Handle);

						if(Chunk->cn_ID == ID_NODE)
							load_menulist(Handle, data->LT_Menus, MUIV_Listtree_GetEntry_ListNode_Root, name);
					}
					set(obj,MUIA_Listtree_Quiet,FALSE);
				}
	
				CloseIFF(Handle);
			}
			Close(Handle->iff_Stream);
		}
		FreeIFF(Handle);
	}
	return(success);
}


ULONG MenuPrefs_NewEntry(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);

	switch(MUI_Request(app, obj, 0, 0, "Menu|Group|BarLabel", "Create what ?"))
	{
		case 0:
			DoMethod(data->LT_Menus, MUIM_Listtree_Insert, "~~~~~~~~~~", NULL, MUIV_Listtree_Insert_ListNode_Root, MUIV_Listtree_Insert_PrevNode_Tail, MUIV_Listtree_Insert_Flags_Active);
			break;
		case 1:
			DoMethod(data->LT_Menus, MUIM_Listtree_Insert, "NewMenu", NULL, MUIV_Listtree_Insert_ListNode_Root, MUIV_Listtree_Insert_PrevNode_Tail, MUIV_Listtree_Insert_Flags_Active);
			break;
		case 2:
			DoMethod(data->LT_Menus, MUIM_Listtree_Insert, "NewGroup", NULL, MUIV_Listtree_Insert_ListNode_Root, MUIV_Listtree_Insert_PrevNode_Tail, MUIV_Listtree_Insert_Flags_Active | TNF_LIST);
			break;
	}

	return(NULL);
}

ULONG MenuPrefs_Listtree_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct MenuPrefs_Data *data = INST_DATA(cl, obj);
	struct MUIS_Listtree_TreeNode *tn;

	tn = (struct MUIS_Listtree_TreeNode *)DoMethod(data->LT_Menus, MUIM_Listtree_GetEntry, MUIV_Listtree_GetEntry_ListNode_Active, MUIV_Listtree_GetEntry_Position_Active, NULL);
	if(tn)
	{
		set(data->STR_Name, MUIA_Disabled, FALSE);
		set(data->BT_Delete, MUIA_Disabled, FALSE);
		setstring(data->STR_Name, tn->tn_Name);
	}
	else
	{
		set(data->STR_Name, MUIA_Disabled, TRUE);
		set(data->BT_Delete, MUIA_Disabled, TRUE);
		setstring(data->STR_Name, "");
	}

	return(NULL);
}

ULONG MenuPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct MenuPrefs_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title		, "Menu Setup",
		MUIA_Window_ID			, MAKE_ID('M','P','R','F'),
		MUIA_Window_AppWindow, TRUE,
		WindowContents		, VGroup,
			MUIA_HelpNode, "WI_MenuPrefs",
			GroupFrame,
			Child, HGroup,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_Menus = ListviewObject,
						MUIA_FrameTitle, "Menu Entries",
						MUIA_CycleChain			, 1,
						MUIA_Listview_Input		, TRUE,
						MUIA_Listview_DragType	, 1,
						MUIA_Listview_List		, tmp.LT_Menus = ListtreeObject,
							InputListFrame,
							MUIA_Listtree_ConstructHook, MUIV_Listtree_ConstructHook_String,
							MUIA_Listtree_DestructHook	, MUIV_Listtree_DestructHook_String,
						End,
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_New = MakeButton("  _New"),
						Child, tmp.BT_Delete = MakeButton("  _Delete"),
					End,
					Child, tmp.STR_Name = String("", 40),
				End,
				Child, VGroup,
					Child, VGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.LV_Commands = ListviewObject,
							MUIA_FrameTitle, "Commands",
							MUIA_Listview_DragType		, 1,
							MUIA_Listview_List			, tmp.LI_Commands = ListObject,
								InputListFrame,
								MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
								MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
								MUIA_List_DragSortable	, TRUE,
							End,
						End,
						Child, HGroup,
							MUIA_Group_Spacing, 0,
							Child, tmp.BT_NewCommand = MakeButton("  N_ew"),
							Child, tmp.BT_DeleteCommand = MakeButton("  De_lete"),
							Child, tmp.CY_Type = Cycle(ARR_ProgramTypes),
						End,
						Child, tmp.STR_Command = String("", 40),
					End,
					Child, ColGroup(2),
						Child, MakeKeyLabel2("  Shortcut", "  h"),
						Child, MakeKeyString("   ", 1, "  h"),
					End,
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

		*data = tmp;

		set(tmp.BT_Delete, MUIA_Disabled, TRUE);
		set(tmp.STR_Name, MUIA_Disabled, TRUE);

		DoMethod(tmp.LT_Menus			, MUIM_Notify, MUIA_Listtree_Active	, MUIV_EveryTime	, obj, 1, MUIM_MenuPrefs_Listtree_Active);
		DoMethod(tmp.BT_New				, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 1, MUIM_MenuPrefs_NewEntry);
		DoMethod(tmp.BT_New				, MUIM_Notify, MUIA_Pressed, FALSE, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, tmp.STR_Name);
		DoMethod(tmp.BT_Delete			, MUIM_Notify, MUIA_Pressed			, FALSE				, tmp.LT_Menus, 4, MUIM_Listtree_Remove, NULL, MUIV_Listtree_Remove_TreeNode_Active, 0);
		DoMethod(tmp.STR_Name			, MUIM_Notify, MUIA_String_Acknowledge	, MUIV_EveryTime	, tmp.LT_Menus, 4, MUIM_Listtree_Rename, MUIV_Listtree_Rename_TreeNode_Active, MUIV_TriggerValue, 0);

		DoMethod(obj						, MUIM_Notify, MUIA_Window_CloseRequest, TRUE			, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_MenuPrefs_Finish, obj, 0);
		DoMethod(tmp.BT_Cancel			, MUIM_Notify, MUIA_Pressed			, FALSE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_MenuPrefs_Finish, obj, 0);
		DoMethod(tmp.BT_Use				, MUIM_Notify, MUIA_Pressed			, FALSE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_MenuPrefs_Finish, obj, 1);
		DoMethod(tmp.BT_Save				, MUIM_Notify, MUIA_Pressed			, FALSE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_MenuPrefs_Finish, obj, 2);
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG MenuPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW									: return(MenuPrefs_New					(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_LoadMenus			: return(MenuPrefs_LoadMenus			(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_NewEntry			: return(MenuPrefs_NewEntry			(cl, obj, (APTR)msg));
		case MUIM_MenuPrefs_Listtree_Active	: return(MenuPrefs_Listtree_Active	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Button class                                                             */
/****************************************************************************/

ULONG Button_Action(struct IClass *cl, Object *obj, Msg msg)
{
	struct Button_Data *data = INST_DATA(cl, obj);
	char command[MAXPATHLEN];

	play_sound(data->icon->Sound, data->icon->Volume);

	switch(data->icon->Type)
	{
		case 2:
			sprintf(command, "Execute %ls", data->icon->Program);
			break;
		case 3:
			sprintf(command, "SYS:Rexxc/rx %ls", data->icon->Program);
			break;
		default:
			strcpy(command, data->icon->Program);
			break;
	}

	if(data->icon->Type == 1)
	{
		BPTR lock;
		struct WBStartMsg msg;
		struct MsgPort *mp,*hp;

		if(mp = CreateMsgPort())
		{
			lock = CurrentDir(NULL);

			msg.wbsm_Msg.mn_Node.ln_Pri= 0;
			msg.wbsm_Msg.mn_ReplyPort	= mp;
			msg.wbsm_DirLock				= lock;
			msg.wbsm_Stack					= 4096;
			msg.wbsm_Prio					= 0;
			msg.wbsm_NumArgs				= 0;
			msg.wbsm_ArgList				= NULL;
			msg.wbsm_Name					= command;

			Forbid();
			if(hp = FindPort(WBS_PORTNAME))
				PutMsg(hp, (struct Message *)&msg);
			Permit();

			if(!hp)
			{
				BPTR ifh = Open("NIL:",MODE_NEWFILE);
				BPTR ofh = Open("NIL:",MODE_OLDFILE);

				if(SystemTags(WBS_LOADNAME,
					SYS_Input		, ifh,
					SYS_Output		, ofh,
					SYS_Asynch		, TRUE,
					SYS_UserShell	, TRUE,
					NP_ConsoleTask	, NULL,
					NP_WindowPtr	, NULL,
					TAG_DONE) != -1)
				{
					int i;

					for (i = 0; i < 10; i++)
					{
						Forbid();
						if(hp = FindPort(WBS_PORTNAME))
							PutMsg(hp, (struct Message *)&msg);
						Permit();

						if(hp)
							break;

						Delay(25);
					}
				}
				else
				{
					Close(ifh);
					Close(ofh);
				}
			}

			if(hp)
			{
				WaitPort(mp);
				GetMsg(mp);
			}
			else
				MUI_Request(app, win, 0, 0, GetStr(MSG_BT_Okay), GetStr(MSG_LA_NoWBStartHandler));

			CurrentDir(lock);
			DeleteMsgPort(mp);
		}
	}
	else
	{
		SystemTags(command,
			SYS_Input		, NULL,
			SYS_Output		, NULL,
			SYS_Asynch		, TRUE,
			NP_CloseInput	, FALSE,
			NP_CloseOutput	, FALSE,
			TAG_DONE);
	}

	if(data->icon->Type == 2)
	{
		STRPTR ptr;

		if(ptr = FilePart(data->icon->Program))
		{
			if(!stricmp(ptr, "startnet") || !stricmp(ptr, "stopnet"))
			{
				set(win, MUIA_Window_Sleep, TRUE);
				Delay(200);
				set(win, MUIA_Window_Sleep, FALSE);
				DoMethod(app, MUIM_Application_PushMethod, win, 1, MUIM_IconBar_LoadButtons);
			}
		}
	}

	return(NULL);
}

ULONG Button_Dispose(struct IClass *cl, Object *obj, Msg msg)
{
	struct Button_Data *data = INST_DATA(cl, obj);

	if(data->icon->body)					/* this one gets allocated in create_bodychunk */
		FreeVec(data->icon->body);

	if(data->icon->cols)
		FreeVec(data->icon->cols);

	if(data->icon->bmhd)
		FreeVec(data->icon->bmhd);

	if(data->icon->cx_filter && CxBase)
		DeleteCxObjAll(data->icon->cx_filter);

	if(data->icon)
		FreeVec(data->icon);

	return(DoSuperMethodA(cl, obj, msg));
}

ULONG Button_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	static const struct Hook Button_AppMsgHook = { {NULL, NULL}, (VOID *)Button_AppMsgFunc, NULL, NULL };
	struct Button_Data tmp;
	struct Icon *icon;

	if(icon = (struct Icon *)GetTagData(MUIA_NetConnect_Icon, (ULONG)"", msg->ops_AttrList))
	{
		if(obj = (Object *)DoSuperNew(cl, obj,
			ButtonFrame,
			MUIA_Group_Spacing, 0,
			MUIA_InnerBottom	, 0,
			MUIA_InnerLeft		, 0,
			MUIA_InnerTop		, 0,
			MUIA_InnerRight	, 0,
			MUIA_CycleChain	, 1,
			MUIA_Background	, MUII_ButtonBack,
			MUIA_InputMode		, MUIV_InputMode_RelVerify,
			TAG_MORE, msg->ops_AttrList))
		{
			struct Button_Data *data = INST_DATA(cl, obj);

			if(tmp.icon = AllocVec(sizeof(struct Icon), MEMF_ANY))
			{
				memcpy(tmp.icon, icon, sizeof(struct Icon));

				set(obj, MUIA_ShortHelp, tmp.icon->Name);
				DoMethod(obj, MUIM_Notify, MUIA_Pressed		, FALSE				, obj, 1, MUIM_Button_Action);
				DoMethod(obj, MUIM_Notify, MUIA_AppMessage	, MUIV_EveryTime	, obj, 3, MUIM_CallHook, &Button_AppMsgHook, MUIV_TriggerValue);

				tmp.icon->cx_filter = NULL;
				if(*tmp.icon->Hotkey && CxBase && xget(app, MUIA_Application_Broker) && xget(app, MUIA_Application_BrokerPort))
				{
					if(tmp.icon->cx_filter = CxFilter(tmp.icon->Hotkey))
					{
						BOOL success = FALSE;
						CxObj *sender;
						CxObj *translator;

						AttachCxObj((CxObj *)xget(app, MUIA_Application_Broker), tmp.icon->cx_filter);
						if(sender = CxSender(xget(app, MUIA_Application_BrokerPort), obj))
						{
							AttachCxObj(tmp.icon->cx_filter, sender);
							if(translator = CxTranslate(NULL))
							{
								AttachCxObj(tmp.icon->cx_filter, translator);
								if(!(CxObjError(tmp.icon->cx_filter)))
									success = TRUE;
							}
						}
						if(!success)
						{
							DeleteCxObjAll(tmp.icon->cx_filter);
							tmp.icon->cx_filter = NULL;
						}
					}
				}
			}
			*data = tmp;
		}
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG Button_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW					: return(Button_New		(cl, obj, (APTR)msg));
		case OM_DISPOSE			: return(Button_Dispose	(cl, obj, (APTR)msg));
		case MUIM_Button_Action	: return(Button_Action	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}


/****************************************************************************/
/* IconBar class                                                            */
/****************************************************************************/

ULONG IconBar_LoadButtons(struct IClass *cl, Object *obj, struct MUIP_IconBar_LoadButtons *msg)
{
	struct IconBar_Data *data = INST_DATA(cl,obj);
	Object *button, *group, *root;
	struct Icon *icon;
	BOOL anything = FALSE;
	struct IFFHandle *Handle;
	struct ContextNode *cn;
	LONG button_type = 0, rows = 1, window_type = 0;

	if(group = RowGroup(rows),
		MUIA_Group_Spacing, 0,
		MUIA_InnerBottom	, 0,
		MUIA_InnerLeft		, 0,
		MUIA_InnerTop		, 0,
		MUIA_InnerRight	, 0,
		End)
	{
		DoMethod(group, MUIM_Group_InitChange);
		if(icon = AllocVec(sizeof(struct Icon), MEMF_ANY))
		{
			if(Handle = AllocIFF())
			{
				if(Handle->iff_Stream = Open("ENV:NetConnectPrefs", MODE_OLDFILE))
				{
					InitIFFasDOS(Handle);
					if(!(OpenIFF(Handle, IFFF_READ)))
					{
						if(!(StopChunks(Handle, Stops, NUM_STOPS)))
						{
							while(!ParseIFF(Handle, IFFPARSE_SCAN))
							{
								cn = CurrentChunk(Handle);

								if(cn->cn_ID == ID_ROWS)
								{
									ReadChunkBytes(Handle, &rows, MIN(sizeof(LONG), cn->cn_Size));
									set(group, MUIA_Group_Rows, rows);
								}

								if(cn->cn_ID == ID_BTTY)
									ReadChunkBytes(Handle, &button_type, MIN(sizeof(LONG), cn->cn_Size));

								if(cn->cn_ID == ID_WINT)
									ReadChunkBytes(Handle, &window_type, MIN(sizeof(LONG), cn->cn_Size));

								if(cn->cn_ID == ID_AICN)
								{
									if(ReadChunkBytes(Handle, icon, MIN(sizeof(struct Icon), cn->cn_Size)) == MIN(sizeof(struct Icon), cn->cn_Size))
									{
										icon->body		= NULL;
										icon->list		= NULL;
										icon->cols		= NULL;
										icon->bmhd		= NULL;
										icon->edit_window = NULL;
										if(icon->Type == 2)
										{
											STRPTR ptr;

											if(ptr = FilePart(icon->Program))
											{
												if(!stricmp(ptr, "startnet") || !stricmp(ptr, "stopnet"))
												{
													struct Library *lib;

													if(lib = OpenLibrary("bsdsocket.library", NULL))
													{
														strcpy(ptr, "stopnet");

														if(ptr = FilePart(icon->ImageFile))
														{
															*ptr = NULL;
															AddPart(icon->ImageFile, "Stop", MAXPATHLEN - strlen(icon->ImageFile));
														}
														CloseLibrary(lib);
													}
													else
													{
														strcpy(ptr, "startnet");

														if(ptr = FilePart(icon->ImageFile))
														{
															*ptr = NULL;
															AddPart(icon->ImageFile, "Start", MAXPATHLEN - strlen(icon->ImageFile));
														}
													}
												}
											}
										}
										switch(button_type)
										{
											case 1:
												button = NewObject(CL_Button->mcc_Class, NULL,
													MUIA_NetConnect_Icon, icon,
													Child, create_bodychunk(icon, FALSE),
													TAG_DONE);
												break;
											case 2:
												button = NewObject(CL_Button->mcc_Class, NULL,
													MUIA_NetConnect_Icon, icon,
													Child, TextObject,
														MUIA_Text_PreParse, "\33c",
														MUIA_Text_Contents, icon->Name,
													End,
													TAG_DONE);
												break;
											default:
												button = NewObject(CL_Button->mcc_Class, NULL,
													MUIA_NetConnect_Icon, icon,
													Child, create_bodychunk(icon, FALSE),
													Child, TextObject,
														MUIA_Font, MUIV_Font_Tiny,
														MUIA_Text_PreParse, "\33c",
														MUIA_Text_Contents, icon->Name,
													End,
													TAG_DONE);
												break;
										}
										if(button)
										{
											DoMethod(group, OM_ADDMEMBER, button);
											anything = TRUE;
										}
									}
								}
							}
						}
						CloseIFF(Handle);
					}
					Close(Handle->iff_Stream);
				}
				FreeIFF(Handle);
			}
			if(!anything)
			{
				struct Library *lib;

				/* there's nothing in both lists => create at least the start/stop icon */

				if(lib = OpenLibrary("bsdsocket.library", NULL))
				{
					strcpy(icon->ImageFile, IMAGE_PATH"Stop");
					strcpy(icon->Program	, "AmiTCP:bin/stopnet");

					CloseLibrary(lib);
				}
				else
				{
					strcpy(icon->ImageFile, IMAGE_PATH"Start");
					strcpy(icon->Program	, "AmiTCP:bin/startnet");
				}

				strcpy(icon->Name		, "Start/Stop");
				icon->Type	= 2;

				if(button = NewObject(CL_Button->mcc_Class, NULL,
					MUIA_NetConnect_Icon, icon,
					Child, create_bodychunk(icon, FALSE),
					Child, TextObject,
						MUIA_Font, MUIV_Font_Tiny,
						MUIA_Text_PreParse, "\33c",
						MUIA_Text_Contents, icon->Name,
					End,
					TAG_DONE))
				{
					DoMethod(group, OM_ADDMEMBER, button);
				}
			}
			FreeVec(icon);
		}
		DoMethod(group, MUIM_Group_ExitChange);
		if(root = (Object *)xget(win, MUIA_Window_RootObject))
		{
			DoMethod(root, MUIM_Group_InitChange);

			DoMethod(root, OM_REMMEMBER, data->GR_Buttons);
			MUI_DisposeObject(data->GR_Buttons);
			data->GR_Buttons = group;
			DoMethod(root, OM_ADDMEMBER, group);

			DoMethod(root, MUIM_Group_ExitChange);
		}
	}
	return(NULL);
}


ULONG IconBar_IconBarPrefs(struct IClass *cl, Object *obj, Msg msg)
{
	Object *window;

	set(app, MUIA_Application_Sleep, TRUE);
	if(window = (Object *)NewObject(CL_IconBarPrefs->mcc_Class, NULL, TAG_DONE))
	{
		DoMethod(app, OM_ADDMEMBER, window);

		set(window, MUIA_Window_Open, TRUE);
		set(app, MUIA_Application_Sleep, FALSE);
		set(win, MUIA_Window_Sleep, TRUE);
	}
	else
		set(app, MUIA_Application_Sleep, FALSE);

	if(window)
		DoMethod(window, MUIM_IconBarPrefs_LoadIcons);

	return(NULL);
}

ULONG IconBar_IconBarPrefs_Finish(struct IClass *cl, Object *obj, struct MUIP_IconBar_IconBarPrefs_Finish *msg)
{
	Object *window = msg->window;
	struct IconBarPrefs_Data *data = INST_DATA(CL_IconBarPrefs->mcc_Class, window);
	struct Icon *icon;
	struct IFFHandle	*Handle;
	LONG i, button_type, window_type, rows, pos;

	button_type = xget(data->CY_ButtonType, MUIA_Cycle_Active);
	window_type = xget(data->CY_WindowType, MUIA_Cycle_Active);
	rows = xget(data->SL_Rows, MUIA_Numeric_Value);

// close the EditIcon Window and if msg->level, use the changed arguments
	pos = 0;
	FOREVER
	{
		DoMethod(data->LI_ActiveIcons, MUIM_List_GetEntry, pos++, &icon);
		if(icon)
		{
			if(icon->edit_window)
				DoMethod(window, MUIM_IconBarPrefs_EditIcon_Finish, icon, data->LI_ActiveIcons, msg->level);
		}
		else
			break;
	}
	pos = 0;
	FOREVER
	{
		DoMethod(data->LI_InactiveIcons, MUIM_List_GetEntry, pos++, &icon);
		if(icon)
		{
			if(icon->edit_window)
				DoMethod(window, MUIM_IconBarPrefs_EditIcon_Finish, icon, data->LI_InactiveIcons, msg->level);
		}
		else
			break;
	}


	i = msg->level;
	while(i > 0)
	{
		if(Handle = AllocIFF())
		{
			if(Handle->iff_Stream = Open((i == 2 ? "ENVARC:NetConnectPrefs" : "ENV:NetConnectPrefs"), MODE_NEWFILE))
			{
				InitIFFasDOS(Handle);
				if(!(OpenIFF(Handle, IFFF_WRITE)))
				{
					if(!(PushChunk(Handle, ID_NTCN, ID_FORM, IFFSIZE_UNKNOWN)))
					{
						if(!PushChunk(Handle, ID_NTCN, ID_BTTY, IFFSIZE_UNKNOWN))
							if(WriteChunkBytes(Handle, &button_type, sizeof(LONG)) == sizeof(LONG))
								PopChunk(Handle);
						if(!PushChunk(Handle, ID_NTCN, ID_WINT, IFFSIZE_UNKNOWN))
							if(WriteChunkBytes(Handle, &window_type, sizeof(LONG)) == sizeof(LONG))
								PopChunk(Handle);
						if(!PushChunk(Handle, ID_NTCN, ID_ROWS, IFFSIZE_UNKNOWN))
							if(WriteChunkBytes(Handle, &rows, sizeof(LONG)) == sizeof(LONG))
								PopChunk(Handle);

						pos = 0;
						FOREVER
						{
							DoMethod(data->LI_ActiveIcons, MUIM_List_GetEntry, pos++, &icon);
							if(!icon)
								break;
							if(PushChunk(Handle, ID_NTCN, ID_AICN, IFFSIZE_UNKNOWN))
								break;
							if(WriteChunkBytes(Handle, icon, sizeof(struct Icon)) != sizeof(struct Icon))
								break;
							if(PopChunk(Handle))
								break;
						}
						pos = 0;
						FOREVER
						{
							DoMethod(data->LI_InactiveIcons, MUIM_List_GetEntry, pos++, &icon);
							if(!icon)
								break;
							if(PushChunk(Handle, ID_NTCN, ID_IICN, IFFSIZE_UNKNOWN))
								break;
							if(WriteChunkBytes(Handle, icon, sizeof(struct Icon)) != sizeof(struct Icon))
								break;
							if(PopChunk(Handle))
								break;
						}
						PopChunk(Handle);
					}
					CloseIFF(Handle);
				}
				Close(Handle->iff_Stream);
			}
			FreeIFF(Handle);
		}
		i--;
	}

	if(msg->level)
	{
		if(xget(data->CY_WindowType, MUIA_Cycle_Active) == xget(data->CY_WindowType, MUIA_UserData))
			DoMethod(win, MUIM_IconBar_LoadButtons);
		else
			DoMethod(app, MUIM_Application_ReturnID, ID_REBUILD);
	}

	set(window, MUIA_Window_Open, FALSE);
	set(win, MUIA_Window_Sleep, FALSE);
	DoMethod(app, OM_REMMEMBER, window);
	MUI_DisposeObject(window);

	return(NULL);
}

ULONG IconBar_MenuPrefs(struct IClass *cl, Object *obj, Msg msg)
{
	Object *window;

	set(app, MUIA_Application_Sleep, TRUE);
	if(window = (Object *)NewObject(CL_MenuPrefs->mcc_Class, NULL, TAG_DONE))
	{
		DoMethod(app, OM_ADDMEMBER, window);

		set(window, MUIA_Window_Open, TRUE);
		set(app, MUIA_Application_Sleep, FALSE);
		set(win, MUIA_Window_Sleep, TRUE);
	}
	else
		set(app, MUIA_Application_Sleep, FALSE);

	if(window)
		DoMethod(window, MUIM_MenuPrefs_LoadMenus);

	return(NULL);
}

VOID save_menulist(struct IFFHandle *Handle, Object *obj, struct MUIS_Listtree_TreeNode *list)
{
	struct MUIS_Listtree_TreeNode *tn;
	LONG pos = 0;

	if(PushChunk(Handle, ID_NTCN, ID_NODE, IFFSIZE_UNKNOWN))
		return;
	if(list == MUIV_Listtree_GetEntry_ListNode_Root)
		WriteChunkBytes(Handle, "ROOT", 5);
	else
		WriteChunkBytes(Handle, list->tn_Name, strlen(list->tn_Name) + 1);
	if(PopChunk(Handle))
		return;

	FOREVER
	{
		if(tn = (struct MUIS_Listtree_TreeNode *)DoMethod(obj, MUIM_Listtree_GetEntry, list, pos, MUIV_Listtree_GetEntry_Flags_SameLevel))
		{
			if(tn->tn_Flags & TNF_LIST)
			{
				save_menulist(Handle, obj, tn);
			}
			else
			{
				if(!(PushChunk(Handle, ID_NTCN, ID_MENU, IFFSIZE_UNKNOWN)))
				{
					if(WriteChunkBytes(Handle, tn->tn_Name, strlen(tn->tn_Name) + 1) != strlen(tn->tn_Name) + 1)
						break;
					if(PopChunk(Handle))
						break;
				}
				else
					break;
			}
		}
		else
			break;

		pos++;
	}

	if(!(PushChunk(Handle, ID_NTCN, ID_END, IFFSIZE_UNKNOWN)))
		PopChunk(Handle);
}

ULONG IconBar_MenuPrefs_Finish(struct IClass *cl, Object *obj, struct MUIP_IconBar_MenuPrefs_Finish *msg)
{
	Object *window = msg->window;
	struct MenuPrefs_Data *data = INST_DATA(CL_MenuPrefs->mcc_Class, window);
	struct IFFHandle	*Handle;
	int i = msg->level;

	while(i > 0)
	{
		if(Handle = AllocIFF())
		{
			if(Handle->iff_Stream = Open((i == 2 ? "ENVARC:NetConnectPrefs.menus" : "ENV:NetConnectPrefs.menus"), MODE_NEWFILE))
			{
				InitIFFasDOS(Handle);
				if(!(OpenIFF(Handle, IFFF_WRITE)))
				{
					if(!(PushChunk(Handle, ID_NTCN, ID_FORM, IFFSIZE_UNKNOWN)))
					{
						save_menulist(Handle, data->LT_Menus, MUIV_Listtree_GetEntry_ListNode_Root);
						PopChunk(Handle);
					}
					CloseIFF(Handle);
				}
				Close(Handle->iff_Stream);
			}
			FreeIFF(Handle);
		}
		i--;
	}

	set(window, MUIA_Window_Open, FALSE);
	set(win, MUIA_Window_Sleep, FALSE);
	DoMethod(app, OM_REMMEMBER, window);
	MUI_DisposeObject(window);

	return(NULL);
}


ULONG IconBar_AmiTCPPrefs(struct IClass *cl, Object *obj, Msg msg)
{
	SystemTags("NetConnect:AmiTCPConfig",
		SYS_Input		, NULL,
		SYS_Output		, NULL,
		SYS_Asynch		, TRUE,
		NP_CloseInput	, FALSE,
		NP_CloseOutput	, FALSE,
		TAG_DONE);

	return(NULL);
}

ULONG IconBar_About(struct IClass *cl, Object *obj, Msg msg)
{
	Object *req;

	set(app, MUIA_Application_Sleep, TRUE);
	if(req = (APTR)NewObject(CL_About->mcc_Class, NULL, TAG_DONE))
	{
		DoMethod(app, OM_ADDMEMBER, req);
		set(req, MUIA_Window_Open, TRUE);
		set(app, MUIA_Application_Sleep, FALSE);
		set(win, MUIA_Window_Sleep, TRUE);
	}
	else
		set(app, MUIA_Application_Sleep, FALSE);

	return(NULL);
}

ULONG IconBar_About_Finish(struct IClass *cl, Object *obj, struct MUIP_IconBar_About_Finish *msg)
{
	Object *window = msg->window;

	set(window, MUIA_Window_Open, FALSE);
	set(win, MUIA_Window_Sleep, FALSE);
	DoMethod(app, OM_REMMEMBER, window);
	MUI_DisposeObject(window);

	return(NULL);
}

ULONG IconBar_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct IconBar_Data tmp;
	struct IFFHandle	*Handle;
	struct ContextNode *cn;
	LONG window_type, rows;

	if(Handle = AllocIFF())
	{
		if(Handle->iff_Stream = Open("ENV:NetConnectPrefs", MODE_OLDFILE))
		{
			InitIFFasDOS(Handle);
			if(!(OpenIFF(Handle, IFFF_READ)))
			{
				if(!(StopChunks(Handle, Stops, NUM_STOPS)))
				{
					while(!ParseIFF(Handle, IFFPARSE_SCAN))
					{
						cn = CurrentChunk(Handle);
						if(cn->cn_ID == ID_ROWS)
							ReadChunkBytes(Handle, &rows, MIN(sizeof(LONG), cn->cn_Size));
						if(cn->cn_ID == ID_WINT)
							ReadChunkBytes(Handle, &window_type, MIN(sizeof(LONG), cn->cn_Size));
					}
				}
				CloseIFF(Handle);
			}
			Close(Handle->iff_Stream);
		}
		FreeIFF(Handle);
	}

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_ID				, MAKE_ID('I','C','O','N'),
		MUIA_Window_Menustrip	, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, IconBarMenu,0),
		MUIA_Window_AppWindow	, TRUE,
		MUIA_Window_Title			, (window_type != 1 ? GetStr(MSG_WI_IconBar) : NULL),
		MUIA_Window_Borderless	, window_type,
		MUIA_Window_DepthGadget	, !window_type,
		MUIA_Window_DragBar		, (window_type != 1),
		MUIA_Window_CloseGadget	, !window_type,
		MUIA_Window_SizeGadget	, !window_type,
		WindowContents			, VGroup,
			MUIA_Group_Spacing, 0,
			MUIA_InnerBottom	, 0,
			MUIA_InnerLeft		, 0,
			MUIA_InnerTop		, 0,
			MUIA_InnerRight	, 0,
			Child, tmp.GR_Buttons = ColGroup(rows),
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct IconBar_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
			MUIV_Notify_Application, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_ABOUT)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 1, MUIM_IconBar_About);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_QUIT)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			MUIV_Notify_Application, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_ICONBAR)	, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 1, MUIM_IconBar_IconBarPrefs);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_MENUS)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 1, MUIM_IconBar_MenuPrefs);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_AMITCP)	, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 1, MUIM_IconBar_AmiTCPPrefs);
		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_MUI)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG IconBar_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(IconBar_New						(cl, obj, (APTR)msg));
		case MUIM_IconBar_LoadButtons				: return(IconBar_LoadButtons			(cl, obj, (APTR)msg));
		case MUIM_IconBar_IconBarPrefs			: return(IconBar_IconBarPrefs			(cl, obj, (APTR)msg));
		case MUIM_IconBar_IconBarPrefs_Finish	: return(IconBar_IconBarPrefs_Finish(cl, obj, (APTR)msg));
		case MUIM_IconBar_MenuPrefs				: return(IconBar_MenuPrefs				(cl, obj, (APTR)msg));
		case MUIM_IconBar_MenuPrefs_Finish		: return(IconBar_MenuPrefs_Finish	(cl, obj, (APTR)msg));
		case MUIM_IconBar_AmiTCPPrefs				: return(IconBar_AmiTCPPrefs			(cl, obj, (APTR)msg));
		case MUIM_IconBar_About						: return(IconBar_About					(cl, obj, (APTR)msg));
		case MUIM_IconBar_About_Finish			: return(IconBar_About_Finish			(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}




/*
 * close our custom classes
 */

VOID exit_classes(VOID)
{
	if(CL_MenuPrefs)		MUI_DeleteCustomClass(CL_MenuPrefs);
	if(CL_IconList)		MUI_DeleteCustomClass(CL_IconList);
	if(CL_IconBarPrefs)	MUI_DeleteCustomClass(CL_IconBarPrefs);
	if(CL_EditIcon)		MUI_DeleteCustomClass(CL_EditIcon);
	if(CL_Editor)			MUI_DeleteCustomClass(CL_Editor);

	if(CL_About)			MUI_DeleteCustomClass(CL_About);
	if(CL_Button)			MUI_DeleteCustomClass(CL_Button);
	if(CL_IconBar)			MUI_DeleteCustomClass(CL_IconBar);

	CL_IconBar			= CL_Button		=
	CL_IconBarPrefs	= CL_IconList	= 
	CL_About				= CL_EditIcon	=
	CL_Editor			= CL_MenuPrefs = NULL;
}


/*
 * initialize our custom classes
 */

BOOL init_classes(VOID)
{
	CL_Button			= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct Button_Data)			, Button_Dispatcher);
	CL_IconBar			= MUI_CreateCustomClass(NULL, MUIC_Window	, NULL, sizeof(struct IconBar_Data)			, IconBar_Dispatcher);
	CL_About				= MUI_CreateCustomClass(NULL, MUIC_Window	, NULL, sizeof(struct About_Data)			, About_Dispatcher);

	CL_Editor			= MUI_CreateCustomClass(NULL, MUIC_List	, NULL, sizeof(struct Editor_Data)			, Editor_Dispatcher);
	CL_EditIcon			= MUI_CreateCustomClass(NULL, MUIC_Window	, NULL, sizeof(struct EditIcon_Data)		, EditIcon_Dispatcher);
	CL_IconList			= MUI_CreateCustomClass(NULL, MUIC_List	, NULL, sizeof(struct IconList_Data)		, IconList_Dispatcher);
	CL_IconBarPrefs	= MUI_CreateCustomClass(NULL, MUIC_Window	, NULL, sizeof(struct IconBarPrefs_Data)	, IconBarPrefs_Dispatcher);
	CL_MenuPrefs		= MUI_CreateCustomClass(NULL, MUIC_Window	, NULL, sizeof(struct MenuPrefs_Data)		, MenuPrefs_Dispatcher);

	if(CL_IconBar			&& CL_Button	&& CL_About		&&
		CL_IconBarPrefs	&& CL_IconList && CL_EditIcon	&&
		CL_Editor			&& CL_MenuPrefs)
		return(TRUE);

	exit_classes();
	return(FALSE);
}
