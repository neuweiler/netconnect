#include "globals.c"

extern STRPTR GetStr(STRPTR idstr);
extern Class *TextFieldClass;

#define IMAGE_PATH "NetConnect:Images/"
#define PROGRAM_PATH "NetConnect:Programs/"


SAVEDS ASM int BrokerFunc(REG(a1) CxMsg *msg)
{
	if(CxMsgType(msg) == CXM_IEVENT)
		DoMethod((Object *)CxMsgID(msg), MUIM_Button_Action);
	return(NULL);
}

ULONG __stdargs DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...)
{
	return(DoSuperMethod(cl, obj, OM_NEW, &tag1, NULL));
}

LONG xget(Object *obj, ULONG attribute)
{
	LONG x;
	get(obj, attribute, &x);
	return(x);
}

Object *MakeKeyLabel2(STRPTR label, STRPTR control_char)
{
	return(KeyLabel(GetStr(label), *GetStr(control_char)));
}

Object *MakeButton(STRPTR string)
{
	Object *obj = SimpleButton(GetStr(string));
	if(obj)
		set(obj, MUIA_CycleChain, 1);
	return(obj);
}

Object *MakeKeyString(STRPTR string, LONG len, STRPTR control_char)
{
	Object *obj = KeyString(GetStr(string), len, *(GetStr(control_char)));
	if(obj)
		set(obj, MUIA_CycleChain, 1);
	return(obj);
}

Object *MakeKeyCycle(STRPTR *array, STRPTR control_char)
{
	Object *obj = KeyCycle(array, *(GetStr(control_char)));
	if(obj)
		set(obj, MUIA_CycleChain, 1);
	return(obj);
}

Object *MakeKeySlider(LONG min, LONG max, LONG level, STRPTR control_char)
{
	Object *obj = KeySlider(min, max, level, *(GetStr(control_char)));
	if(obj)
		set(obj, MUIA_CycleChain, 1);
	return(obj);
}

Object *MakePopAsl(Object *string, STRPTR title, BOOL drawers_only)
{
	Object *obj = PopaslObject,
		MUIA_Popstring_String, string,
		MUIA_Popstring_Button, PopButton((drawers_only ? MUII_PopDrawer : MUII_PopFile)),
		MUIA_Popasl_Type		, ASL_FileRequest,
		ASLFR_TitleText		, GetStr(title),
		ASLFR_DrawersOnly		, drawers_only,
	End;
	if(obj)
		set(obj, MUIA_CycleChain, 1);
	return(obj);
}

BOOL load_icon(struct Icon *icon)
{
	struct IFFHandle *Handle;
	struct ContextNode *cn;
	struct StoredProperty *sp;
	int size, i;
	UBYTE *src;
	BOOL success = FALSE;

	if(Handle = AllocIFF())
	{
		if(Handle->iff_Stream = Open(icon->ImageFile, MODE_OLDFILE))
		{
			InitIFFasDOS(Handle);
			if(!OpenIFF(Handle, IFFF_READ))
			{
				if(!ParseIFF(Handle, IFFPARSE_STEP))
				{
					if((cn = CurrentChunk(Handle)) && (cn->cn_ID == ID_FORM))
					{
						if(cn->cn_Type == ID_ILBM)
						{
							if(!PropChunk(Handle, ID_ILBM, ID_BMHD) &&
								!PropChunk(Handle, ID_ILBM, ID_CMAP) &&
								!StopChunk(Handle, ID_ILBM, ID_BODY) &&
								!StopOnExit(Handle, ID_ILBM, ID_FORM) &&
								!ParseIFF(Handle, IFFPARSE_SCAN))
							{
								if(sp = FindProp(Handle, ID_ILBM, ID_CMAP))
								{
									src = sp->sp_Data;
									if(icon->cols = AllocVec(sizeof(ULONG) * sp->sp_Size, MEMF_ANY))
									{
										for (i = 0; i < sp->sp_Size; i++)
											icon->cols[i] = to32(src[i]);
									}
								}

								if(sp = FindProp(Handle, ID_ILBM, ID_BMHD))
								{
									size = CurrentChunk(Handle)->cn_Size;
									if(icon->bmhd = AllocVec(sizeof(struct BitMapHeader), MEMF_ANY))
									{
										memcpy(icon->bmhd, sp->sp_Data, sizeof(struct BitMapHeader));

										if(icon->body = AllocVec(size, MEMF_ANY))
										{
											if(ReadChunkBytes(Handle, icon->body, size) == size)
												success = TRUE;
										}
									}
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

	if(!success)
	{
		if(icon->cols)
			FreeVec(icon->cols);
		icon->cols = NULL;
		if(icon->bmhd)
			FreeVec(icon->bmhd);
		icon->bmhd = NULL;
		if(icon->body)
			FreeVec(icon->body);
		icon->body = NULL;
	}

	return(success);
}

Object *create_bodychunk(struct Icon *icon)
{
	Object *bodychunk = NULL;

	if(load_icon(icon))
	{
		bodychunk = BodychunkObject,
			ButtonFrame,
			MUIA_Bitmap_SourceColors	, icon->cols,
			MUIA_Bitmap_Width				, icon->bmhd->bmh_Width,
			MUIA_Bitmap_Height			, icon->bmhd->bmh_Height,
			MUIA_FixWidth					, icon->bmhd->bmh_Width,
			MUIA_FixHeight					, icon->bmhd->bmh_Height,
			MUIA_Background				, MUII_ButtonBack,
			MUIA_InputMode					, MUIV_InputMode_RelVerify,
			MUIA_Bodychunk_Depth			, icon->bmhd->bmh_Depth,
			MUIA_Bodychunk_Body			, icon->body,
			MUIA_Bodychunk_Compression	, icon->bmhd->bmh_Compression,
			MUIA_Bodychunk_Masking		, icon->bmhd->bmh_Masking,
			MUIA_Bitmap_Transparent		, 0,
			End;
	}
	if(!bodychunk)
	{
		icon->body = NULL;
		icon->bmhd = NULL;
		icon->cols = NULL;
		bodychunk = BodychunkObject,
			ButtonFrame,
			MUIA_Bitmap_SourceColors	, (ULONG *)default_icon_colors,
			MUIA_Bitmap_Width				, DEFAULT_ICON_WIDTH ,
			MUIA_Bitmap_Height			, DEFAULT_ICON_HEIGHT,
			MUIA_FixWidth					, DEFAULT_ICON_WIDTH ,
			MUIA_FixHeight					, DEFAULT_ICON_HEIGHT,
			MUIA_Background				, MUII_ButtonBack,
			MUIA_InputMode					, MUIV_InputMode_RelVerify,
			MUIA_Bodychunk_Depth			, DEFAULT_ICON_DEPTH ,
			MUIA_Bodychunk_Body			, (UBYTE *)default_icon_body,
			MUIA_Bodychunk_Compression	, DEFAULT_ICON_COMPRESSION,
			MUIA_Bodychunk_Masking		, DEFAULT_ICON_MASKING,
			MUIA_Bitmap_Transparent		, 0,
		End;
	}

	return(bodychunk);
}

VOID init_icon(struct Icon *icon, Object *list)
{
	icon->body		= NULL;
	icon->list		= NULL;
	icon->cols		= NULL;
	icon->bmhd		= NULL;
	if(icon->bodychunk = create_bodychunk(icon))
	{
		if(list)
			icon->list = (APTR)DoMethod(list, MUIM_List_CreateImage, icon->bodychunk, 0);
	}
}

LONG get_file_size(STRPTR file)
{
	struct FileInfoBlock *fib;
	BPTR lock;
	LONG size = 0;

	if(lock = Lock(file, ACCESS_READ))
	{
		if(fib = AllocDosObject(DOS_FIB, NULL))
		{
			if(Examine(lock, fib))
				size = (fib->fib_DirEntryType > 0 ? -1 : fib->fib_Size);

			FreeDosObject(DOS_FIB, fib);
		}
		UnLock(lock);
	}
	return(size);
}


STRPTR LoadFile(STRPTR file)
{
	LONG size;
	STRPTR buf = NULL;
	BPTR fh;
	BOOL success = FALSE;

	if((size = get_file_size(file)) > 0)
	{
		if(buf = AllocVec(size + 1, MEMF_ANY))
		{
			if(fh = Open(file, MODE_OLDFILE))
			{
				if(Read(fh, buf, size) == size)
					success = TRUE;
				Close(fh);
			}
			buf[size] = NULL;		// We need buffers which are terminated by a zero (for the textfield)
		}
	}

	if(!success && buf)
	{
		FreeVec(buf);
		buf = NULL;
	}

	return(buf);
}

int find_max(VOID)
{
	struct Icon *icon;
	struct BitMapHeader *bmhd;
	struct IFFHandle *Handle1, *Handle2;
	struct ContextNode *cn1, *cn2;
	struct StoredProperty *sp;
	int max_height = 0;

	if(icon = AllocVec(sizeof(struct Icon), MEMF_ANY | MEMF_CLEAR))
	{
		if(Handle1 = AllocIFF())
		{
			if(Handle1->iff_Stream = Open("ENV:NetConnectPrefs", MODE_OLDFILE))
			{
				InitIFFasDOS(Handle1);
				if(!(OpenIFF(Handle1, IFFF_READ)))
				{
					if(!(StopChunks(Handle1, Stops, NUM_STOPS)))
					{
						while(!ParseIFF(Handle1, IFFPARSE_SCAN))
						{
							cn1 = CurrentChunk(Handle1);
							if(cn1->cn_ID == ID_AICN || cn1->cn_ID == ID_IICN)
							{
								if(ReadChunkBytes(Handle1, icon, MIN(sizeof(struct Icon), cn1->cn_Size)) == MIN(sizeof(struct Icon), cn1->cn_Size))
								{
									if(Handle2=AllocIFF())
									{
										if(Handle2->iff_Stream = Open(icon->ImageFile, MODE_OLDFILE))
										{
											InitIFFasDOS(Handle2);
											if(!OpenIFF(Handle2, IFFF_READ))
											{
												if(!ParseIFF(Handle2, IFFPARSE_STEP))
												{
													if((cn2 = CurrentChunk(Handle2)) && (cn2->cn_ID == ID_FORM))
													{
														if(cn2->cn_Type == ID_ILBM)
														{
															if(!PropChunk(Handle2, ID_ILBM, ID_BMHD) &&
																!PropChunk(Handle2, ID_ILBM, ID_CMAP) &&
																!StopChunk(Handle2, ID_ILBM, ID_BODY) &&
																!StopOnExit(Handle2, ID_ILBM, ID_FORM) &&
																!ParseIFF(Handle2, IFFPARSE_SCAN))
															{
																if(sp = FindProp(Handle2, ID_ILBM, ID_BMHD))
																{
																	bmhd = (struct BitMapHeader *)sp->sp_Data;
																	max_height = MAX(max_height, bmhd->bmh_Height);
																}
															}
														}
													}
												}
												CloseIFF(Handle2);
											}
											Close(Handle2->iff_Stream);
										}
										FreeIFF(Handle2);
									}
								}
							}
						}
					}
					CloseIFF(Handle1);
				}
				Close(Handle1->iff_Stream);
			}
			FreeIFF(Handle1);
		}
		FreeVec(icon);
	}

	if(max_height)
		return(max_height);
	else
		return(40);
}


SAVEDS ASM LONG AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x)
{
	struct WBArg *ap;
	struct AppMessage *amsg = *x;
	static char buf[MAXPATHLEN];

	ap = amsg->am_ArgList;
	if(amsg->am_NumArgs)
	{
		NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
		AddPart(buf, ap->wa_Name, MAXPATHLEN);
		setstring(obj, buf);
	}

	return(NULL);
}

SAVEDS ASM LONG TF_AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x)
{
	struct WBArg *ap;
	struct AppMessage *amsg = *x;
	static char buf[MAXPATHLEN];

	ap = amsg->am_ArgList;
	if(amsg->am_NumArgs)
	{
		NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
		AddPart(buf, ap->wa_Name, MAXPATHLEN);
		set(obj, TEXTFIELD_InsertText, buf);
	}

	return(NULL);
}

BOOL editor_load(STRPTR file, Object *editor)
{
	STRPTR buf;
	BOOL success = FALSE;

	if(*file)
	{
		if(buf = LoadFile(file))
		{
			set(editor, TEXTFIELD_Text, buf);
			set(editor, TEXTFIELD_Modified, NULL);
			FreeVec(buf);
			success = TRUE;
		}
	}
	return(success);
}

BOOL editor_save(STRPTR file, Object *editor)
{
	struct Window *window;
	struct Gadget *gadget;
	STRPTR text;
	ULONG size;
	BPTR fh;
	BOOL do_it = TRUE, success = FALSE;

	window = (struct Window *)xget(editor, MUIA_Window);
	gadget = (struct Gadget *)xget(editor, MUIA_Boopsi_Object);
	if(window && gadget && *file)
	{
		// Set to readonly mode so I can grab the text from the gadget
		SetGadgetAttrs(gadget, window, NULL, TEXTFIELD_ReadOnly, TRUE, TAG_DONE);
		size = xget((Object *)gadget, TEXTFIELD_Size);
		text = (STRPTR)xget((Object *)gadget, TEXTFIELD_Text);
		if(text && size)
		{
			if(get_file_size(file))
				do_it = MUI_Request(app, (Object *)xget(editor, MUIA_WindowObject), NULL, NULL, GetStr(MSG_BT_OverwriteAbort), GetStr(MSG_LA_FileExists));

			if(do_it)
			{
				if(fh = Open(file, MODE_NEWFILE))
				{
					Write(fh, text, size);
					Close(fh);
					set(editor, TEXTFIELD_Modified, NULL);
					success = TRUE;
				}
			}
		}
		SetGadgetAttrs(gadget, window, NULL, TEXTFIELD_ReadOnly, FALSE, TAG_DONE);
	}
	return(success);
}

BOOL editor_checksave(STRPTR file, Object *editor)
{
	BOOL success = FALSE;

	if(xget(editor, TEXTFIELD_Modified))
	{
		if(*file)
		{
			if(MUI_Request(app, (Object *)xget(editor, MUIA_WindowObject), 0, 0, GetStr(MSG_BT_SaveDiscardChanges), GetStr(MSG_LA_ScriptModified)))
				success = editor_save(file, editor);
		}
	}
	return(success);
}

VOID play_sound(STRPTR file, LONG volume)
{
	if(*file && DataTypesBase)
	{
		if(SoundObject)
			DisposeDTObject(SoundObject);

		if(SoundObject = NewDTObject(file,
			DTA_SourceType	, DTST_FILE,
			DTA_GroupID		, GID_SOUND,
			SDTA_Volume		, volume,
			SDTA_Cycles		, 1,
		TAG_DONE))
		{
			DoMethod(SoundObject, DTM_TRIGGER, NULL, STM_PLAY, NULL);
		}
	}
}

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
		if(!anything)
		{
			strcpy(icon->Name			, "Start / Stop");
			strcpy(icon->ImageFile	, IMAGE_PATH"Start");
			strcpy(icon->Program		, "AmiTCP:bin/startnet");
			icon->Type	= 2;

			init_icon(icon, data->LI_ActiveIcons);
			DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		}
		FreeVec(icon);
	}
	set(data->LI_ActiveIcons, MUIA_List_Quiet, FALSE);
	set(data->LI_InactiveIcons, MUIA_List_Quiet, FALSE);

	set(data->SL_Rows, MUIA_Slider_Max, (xget(data->LI_ActiveIcons, MUIA_List_Entries) ? xget(data->LI_ActiveIcons, MUIA_List_Entries) : 1));
	setslider(data->SL_Rows, Rows);
	set(obj, MUIA_Window_Sleep, FALSE);

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
		strcpy(icon->Program, PROGRAM_PATH"IBrowse");
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
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Docs");
		strcpy(icon->ImageFile, IMAGE_PATH"Docs");
		strcpy(icon->Program, PROGRAM_PATH"Docs");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Misc");
		strcpy(icon->ImageFile, IMAGE_PATH"Misc");
		strcpy(icon->Program, PROGRAM_PATH"Misc");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Start / Stop");
		strcpy(icon->ImageFile, IMAGE_PATH"Start");
		strcpy(icon->Program	, "AmiTCP:bin/startnet");
		icon->Type	= 2;
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Top);

		FreeVec(icon);
	}

	set(data->LI_ActiveIcons	, MUIA_List_Quiet, FALSE);
	set(data->LI_InactiveIcons	, MUIA_List_Quiet, FALSE);

	set(data->SL_Rows, MUIA_Slider_Max, (xget(data->LI_ActiveIcons, MUIA_List_Entries) ? xget(data->LI_ActiveIcons, MUIA_List_Entries) : 1));
	setslider(data->SL_Rows, Rows);
	set(obj, MUIA_Window_Sleep, FALSE);

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
		set(data->LI_ActiveIcons, MUIA_List_Active, MUIV_List_Active_Off);

		FreeVec(icon);
	}

	return(NULL);
}

ULONG IconBarPrefs_Rows(struct IClass *cl, Object *obj, Msg msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);

	Rows = xget(data->SL_Rows, MUIA_Numeric_Value);
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

ULONG IconBarPrefs_ModifyIcon(struct IClass *cl, Object *obj, struct MUIP_IconBarPrefs_ModifyIcon *msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon;
	Object *list;

	if(find_list(data, &list, &icon))
	{
		switch(msg->flags)
		{
			case MUIV_IconBarPrefs_ModifyIcon_Remove:
				if(icon->list)
					DoMethod(data->LI_ActiveIcons, MUIM_List_DeleteImage, icon->list);
				icon->list = NULL;
				DoMethod(list, MUIM_List_Remove, MUIV_List_Remove_Active);
				set(data->SL_Rows, MUIA_Slider_Max, (xget(data->LI_ActiveIcons, MUIA_List_Entries) ? xget(data->LI_ActiveIcons, MUIA_List_Entries) : 1));
				setslider(data->SL_Rows, Rows);
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Name:
				strcpy(icon->Name, (STRPTR)xget(data->STR_Name, MUIA_String_Contents));
				DoMethod(list, MUIM_List_Redraw, MUIV_List_Redraw_Active);
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Type:
				icon->Type = xget(data->CY_Type, MUIA_Cycle_Active);
				if(icon->Type == 2 || icon->Type == 3)
				{
					set(data->GR_Script, MUIA_Disabled, FALSE);
					editor_load(icon->Program, data->TF_Editor);
				}
				else
				{
					set(data->TF_Editor, TEXTFIELD_Text, "");
					set(data->TF_Editor, TEXTFIELD_Modified, NULL);
					set(data->GR_Script, MUIA_Disabled, TRUE);
				}
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Program:
				strcpy(icon->Program, (STRPTR)xget(data->PA_Program, MUIA_String_Contents));
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Hotkey:
				strcpy(icon->Hotkey, (STRPTR)xget(data->STR_Hotkey, MUIA_String_Contents));
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Image:
				{
					struct Icon *new_icon;
					LONG position = MUIV_List_Insert_Active;

					set(obj, MUIA_Window_Sleep, TRUE);
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
						strcpy(new_icon->ImageFile, (STRPTR)xget(data->PA_Image, MUIA_String_Contents));

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
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Sound:
				strcpy(icon->Sound, (STRPTR)xget(data->PA_Sound, MUIA_String_Contents));
				break;

			case MUIV_IconBarPrefs_ModifyIcon_PlaySound:
				play_sound(icon->Sound, icon->Volume);
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Volume:
				icon->Volume = xget(data->SL_Volume, MUIA_Numeric_Value);
				break;

			case MUIV_IconBarPrefs_ModifyIcon_LoadScript:
				editor_load(icon->Program, data->TF_Editor);
				break;

			case MUIV_IconBarPrefs_ModifyIcon_SaveScript:
				editor_save(icon->Program, data->TF_Editor);
				break;

			case MUIV_IconBarPrefs_ModifyIcon_ClearScript:
				set(data->TF_Editor, TEXTFIELD_Text, "");
				break;
		}
	}
	return(NULL);
}

ULONG IconBarPrefs_SetStates(struct IClass *cl, Object *obj, struct MUIP_IconBarPrefs_SetStates *msg)
{
	struct IconBarPrefs_Data *data = INST_DATA(cl, obj);
	struct Icon *icon;
	LONG i;

	i = xget(data->CY_Type, MUIA_Cycle_Active);
	if(i == 2 || i == 3)
		editor_checksave((STRPTR)xget(data->PA_Program, MUIA_String_Contents), data->TF_Editor);
	set(data->TF_Editor, TEXTFIELD_Text, "");
	set(data->TF_Editor, TEXTFIELD_Modified, NULL);

	set(data->SL_Rows, MUIA_Slider_Max, (xget(data->LI_ActiveIcons, MUIA_List_Entries) ? xget(data->LI_ActiveIcons, MUIA_List_Entries) : 1));
	nnset((msg->level ? data->LI_InactiveIcons : data->LI_ActiveIcons), MUIA_List_Active, MUIV_List_Active_Off);
	DoMethod((msg->level ? data->LI_InactiveIcons : data->LI_ActiveIcons), MUIM_List_Select, MUIV_List_Select_All, MUIV_List_Select_Off, NULL);

	DoMethod((msg->level ? data->LI_ActiveIcons : data->LI_InactiveIcons), MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &icon);
	if(icon)
	{
		set(data->BT_Remove, MUIA_Disabled, FALSE);
		set(data->GR_Button, MUIA_Disabled, FALSE);

		nnset(data->STR_Name		, MUIA_String_Contents	, icon->Name);
		nnset(data->CY_Type		, MUIA_Cycle_Active		, icon->Type);
		nnset(data->PA_Program	, MUIA_String_Contents	, icon->Program);
		nnset(data->STR_Hotkey	, MUIA_String_Contents	, icon->Hotkey);
		nnset(data->PA_Image		, MUIA_String_Contents	, icon->ImageFile);
		nnset(data->PA_Sound		, MUIA_String_Contents	, icon->Sound);
		nnset(data->SL_Volume	, MUIA_Numeric_Value		, icon->Volume);

		if(icon->Type == 2 || icon->Type == 3)
		{
			set(data->GR_Script, MUIA_Disabled, FALSE);
			editor_load(icon->Program, data->TF_Editor);
		}
		else
			set(data->GR_Script, MUIA_Disabled, TRUE);
	}
	else
	{
		set(data->BT_Remove, MUIA_Disabled, TRUE);
		set(data->GR_Script, MUIA_Disabled, TRUE);
		set(data->GR_Button, MUIA_Disabled, TRUE);

		nnset(data->STR_Name		, MUIA_String_Contents	, "");
		nnset(data->CY_Type		, MUIA_Cycle_Active		, 0);
		nnset(data->PA_Program	, MUIA_String_Contents	, "");
		nnset(data->STR_Hotkey	, MUIA_String_Contents	, "");
		nnset(data->PA_Image		, MUIA_String_Contents	, "");
		nnset(data->PA_Sound		, MUIA_String_Contents	, "");
		nnset(data->SL_Volume	, MUIA_Numeric_Value		, 0);
	}
	if(!CxBase)
		set(data->STR_Hotkey, MUIA_Disabled, TRUE);

	return(NULL);
}

ULONG IconBarPrefs_Dispose(struct IClass *cl, Object *obj, struct opSet *msg)
{
	if(undo_handle)
		CloseClipboard(undo_handle);
	undo_handle = NULL;

	if(clip_handle)
		CloseClipboard(clip_handle);
	clip_handle = NULL;

	return(DoSuperMethodA(cl, obj, msg));
}

ULONG IconBarPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct IconBarPrefs_Data tmp;

	if(!clip_handle)
		clip_handle = OpenClipboard(0);
	if(!undo_handle)
		undo_handle = OpenClipboard(42);

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title		, GetStr(MSG_WI_IconBarPrefs),
		MUIA_Window_ID			, MAKE_ID('I','P','R','F'),
		MUIA_Window_AppWindow, TRUE,
		MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, IconBarPrefsMenu, 0),
		WindowContents		, VGroup,
			MUIA_HelpNode, "WI_IconBarPrefs",
			MUIA_Frame, MUIV_Frame_Group,
			Child, HGroup,
				MUIA_HelpNode, "GR_IconBarPrefs_top",
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_InactiveIcons = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_FrameTitle				, GetStr(MSG_LV_ButtonBankTitle),
						MUIA_Listview_MultiSelect	, MUIV_Listview_MultiSelect_Default,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_InactiveIcons = NewObject(CL_IconList->mcc_Class, NULL, TAG_DONE),
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_New		= MakeButton(MSG_BT_New),
						Child, tmp.BT_Remove = MakeButton(MSG_BT_Remove),
					End,
				End,
				Child, VGroup,
					MUIA_Group_Spacing, 1,
					Child, tmp.LV_ActiveIcons = ListviewObject,
						MUIA_CycleChain				, 1,
						MUIA_FrameTitle				, GetStr(MSG_LV_IconBarTitle),
						MUIA_Listview_MultiSelect	, MUIV_Listview_MultiSelect_Default,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_ActiveIcons = NewObject(CL_IconList->mcc_Class, NULL, TAG_DONE),
					End,
					Child, HGroup,
						Child, MakeKeyLabel2(MSG_LA_Rows, MSG_CC_Rows),
						Child, tmp.SL_Rows = MakeKeySlider(1, 10, Rows, MSG_CC_Rows),
					End,
				End,
			End,
			Child, HGroup,
				MUIA_HelpNode, "GR_IconBarPrefs_bottom",
				Child, tmp.GR_Button = VGroup,
					MUIA_HelpNode	, "GR_IconConrtol",
					MUIA_Frame		, MUIV_Frame_Group,
					MUIA_FrameTitle, GetStr(MSG_GR_ButtonConrtolTitle),
					Child, ColGroup(2),
						Child, MakeKeyLabel2(MSG_LA_Name, MSG_CC_Name),
						Child, tmp.STR_Name = MakeKeyString(MSG_STR_Empty, 80, MSG_CC_Name),
						Child, MakeKeyLabel2(MSG_LA_ProgramType, MSG_CC_ProgramType),
						Child, tmp.CY_Type = MakeKeyCycle(ARR_ProgramTypes, MSG_CC_ProgramType),
						Child, MakeKeyLabel2(MSG_LA_Program, MSG_CC_Program),
						Child, tmp.PA_Program = MakePopAsl(MakeKeyString(MSG_STR_Empty, MAXPATHLEN, MSG_CC_Program), MSG_LA_Program, FALSE),
						Child, MakeKeyLabel2(MSG_LA_Hotkey, MSG_CC_Hotkey),
						Child, tmp.STR_Hotkey = MakeKeyString(MSG_STR_Empty, 80, MSG_CC_Hotkey),
						Child, MakeKeyLabel2(MSG_LA_Image, MSG_CC_Image),
						Child, tmp.PA_Image = MakePopAsl(MakeKeyString(MSG_STR_Empty, MAXPATHLEN, MSG_CC_Image), MSG_LA_Image, FALSE),
						Child, MakeKeyLabel2(MSG_LA_Sound, MSG_CC_Sound),
						Child, HGroup,
							MUIA_Group_Spacing, 0,
							Child, tmp.PA_Sound = MakePopAsl(MakeKeyString(MSG_STR_Empty, MAXPATHLEN, MSG_CC_Sound), MSG_LA_Sound, FALSE),
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
						Child, tmp.SL_Volume = MakeKeySlider(0, 64, 64, MSG_CC_Volume),
					End,
				End,
				Child, BalanceObject, End,
				Child, tmp.GR_Script = VGroup,
					MUIA_HelpNode	, "GR_ScriptEdit",
					MUIA_Frame		, MUIV_Frame_Group,
					MUIA_FrameTitle, GetStr(MSG_GR_ScriptEditTitle),
					Child, tmp.GR_Editor = VGroup,
						MUIA_Group_Spacing, 0,
						Child, HGroup,
							MUIA_Group_Spacing, 0,
							Child, tmp.TF_Editor = BoopsiObject,
								InputListFrame,
								MUIA_CycleChain			, 1,
								MUIA_Boopsi_Class			, TextFieldClass,
								MUIA_Boopsi_Smart			, TRUE,
								MUIA_Boopsi_MinWidth		, 40, /* boopsi objects don't know */
								MUIA_Boopsi_MinHeight	, 40, /* their sizes, so we help   */
								ICA_TARGET					, ICTARGET_IDCMP, /* needed for notification */
								TEXTFIELD_Text				, (ULONG)"",
								TEXTFIELD_ClipStream		, clip_handle,
								TEXTFIELD_UndoStream		, undo_handle,
							End,
							Child, tmp.SB_Editor = ScrollbarObject, End,
						End,
						Child, HGroup,
							MUIA_Group_Spacing, 0,
							Child, tmp.BT_LoadScript = MakeButton(MSG_BT_LoadScript),
							Child, tmp.BT_SaveScript = MakeButton(MSG_BT_SaveScript),
							Child, tmp.BT_ClearScript = MakeButton(MSG_BT_ClearScript),
						End,
					End,
				End,
			End,
			Child, HGroup,
				MUIA_HelpNode			, "GR_IconBarPrefsControl",
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
		static const struct Hook AppMsgHook = { {NULL, NULL}, (VOID *)AppMsgFunc, NULL, NULL };
		static const struct Hook TF_AppMsgHook = { {NULL, NULL}, (VOID *)TF_AppMsgFunc, NULL, NULL };

		*data = tmp;

		set(tmp.BT_Remove, MUIA_Disabled, TRUE);
		set(tmp.GR_Script, MUIA_Disabled, TRUE);
		set(tmp.GR_Button, MUIA_Disabled, TRUE);

		set(tmp.LI_InactiveIcons, MUIA_UserData, tmp.LI_ActiveIcons);	/* show the lists from whom they have to accept drag requests */
		set(tmp.LI_ActiveIcons, MUIA_UserData, tmp.LI_InactiveIcons);

		set(tmp.LV_InactiveIcons, MUIA_ShortHelp, GetStr(MSG_HELP_InactiveIcons));
		set(tmp.LV_ActiveIcons	, MUIA_ShortHelp, GetStr(MSG_HELP_ActiveIcons));
		set(tmp.BT_New				, MUIA_ShortHelp, GetStr(MSG_HELP_New));
		set(tmp.BT_Remove			, MUIA_ShortHelp, GetStr(MSG_HELP_Remove));
		set(tmp.SL_Rows			, MUIA_ShortHelp, GetStr(MSG_HELP_Rows));
		set(tmp.STR_Name			, MUIA_ShortHelp, GetStr(MSG_HELP_Name));
		set(tmp.CY_Type			, MUIA_ShortHelp, GetStr(MSG_HELP_Type));
		set(tmp.PA_Program		, MUIA_ShortHelp, GetStr(MSG_HELP_Program));
		set(tmp.STR_Hotkey		, MUIA_ShortHelp, GetStr(MSG_HELP_Hotkey));
		set(tmp.PA_Image			, MUIA_ShortHelp, GetStr(MSG_HELP_Image));
		set(tmp.PA_Sound			, MUIA_ShortHelp, GetStr(MSG_HELP_Sound));
		set(tmp.BT_PlaySound		, MUIA_ShortHelp, GetStr(MSG_HELP_PlaySound));
		set(tmp.SL_Volume			, MUIA_ShortHelp, GetStr(MSG_HELP_Volume));
		set(tmp.TF_Editor			, MUIA_ShortHelp, GetStr(MSG_HELP_Editor));
		set(tmp.BT_LoadScript	, MUIA_ShortHelp, GetStr(MSG_HELP_LoadScript));
		set(tmp.BT_SaveScript	, MUIA_ShortHelp, GetStr(MSG_HELP_SaveScript));
		set(tmp.BT_ClearScript	, MUIA_ShortHelp, GetStr(MSG_HELP_ClearScript));
		set(tmp.BT_Save			, MUIA_ShortHelp, GetStr(MSG_HELP_Save));
		set(tmp.BT_Use				, MUIA_ShortHelp, GetStr(MSG_HELP_Use));
		set(tmp.BT_Cancel			, MUIA_ShortHelp, GetStr(MSG_HELP_Cancel));

		DoMethod(tmp.TF_Editor			, MUIM_Notify, TEXTFIELD_Lines	, MUIV_EveryTime		, tmp.SB_Editor, 3, MUIM_Set			, MUIA_Prop_Entries	, MUIV_TriggerValue);
		DoMethod(tmp.TF_Editor			, MUIM_Notify, TEXTFIELD_Visible	, MUIV_EveryTime		, tmp.SB_Editor, 3, MUIM_Set			, MUIA_Prop_Visible	, MUIV_TriggerValue);
		DoMethod(tmp.TF_Editor			, MUIM_Notify, TEXTFIELD_Top		, MUIV_EveryTime		, tmp.SB_Editor, 3, MUIM_NoNotifySet, MUIA_Prop_First		, MUIV_TriggerValue);
		DoMethod(tmp.SB_Editor			, MUIM_Notify, MUIA_Prop_First	, MUIV_EveryTime		, tmp.TF_Editor, 3, MUIM_NoNotifySet, TEXTFIELD_Top		, MUIV_TriggerValue);

		DoMethod(tmp.STR_Name			, MUIM_Notify, MUIA_AppMessage	, MUIV_EveryTime		, tmp.STR_Name		, 3, MUIM_CallHook	, &AppMsgHook			, MUIV_TriggerValue);
		DoMethod(tmp.PA_Program			, MUIM_Notify, MUIA_AppMessage	, MUIV_EveryTime		, tmp.PA_Program	, 3, MUIM_CallHook	, &AppMsgHook			, MUIV_TriggerValue);
		DoMethod(tmp.PA_Image			, MUIM_Notify, MUIA_AppMessage	, MUIV_EveryTime		, tmp.PA_Image		, 3, MUIM_CallHook	, &AppMsgHook			, MUIV_TriggerValue);
		DoMethod(tmp.PA_Sound			, MUIM_Notify, MUIA_AppMessage	, MUIV_EveryTime		, tmp.PA_Sound		, 3, MUIM_CallHook	, &AppMsgHook			, MUIV_TriggerValue);
		DoMethod(tmp.TF_Editor			, MUIM_Notify, MUIA_AppMessage	, MUIV_EveryTime		, tmp.TF_Editor	, 3, MUIM_CallHook	, &TF_AppMsgHook			, MUIV_TriggerValue);

		DoMethod(tmp.LV_ActiveIcons	, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_SetStates, 1);
		DoMethod(tmp.LV_InactiveIcons	, MUIM_Notify, MUIA_List_Active		, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_SetStates, 0);
		DoMethod(tmp.BT_New				, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 1, MUIM_IconBarPrefs_NewIcon);
		DoMethod(tmp.BT_Remove			, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Remove);
		DoMethod(tmp.SL_Rows				, MUIM_Notify, MUIA_Numeric_Value	, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_Rows);
		DoMethod(tmp.STR_Name			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Name);
		DoMethod(tmp.CY_Type				, MUIM_Notify, MUIA_Cycle_Active		, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Type);
		DoMethod(tmp.PA_Program			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Program);
		DoMethod(tmp.STR_Hotkey			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Hotkey);
		DoMethod(tmp.PA_Image			, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Image);
		DoMethod(tmp.PA_Sound			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Sound);
		DoMethod(tmp.BT_PlaySound		, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_PlaySound);
		DoMethod(tmp.SL_Volume			, MUIM_Notify, MUIA_Numeric_Value	, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Volume);
		DoMethod(tmp.BT_LoadScript		, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_LoadScript);
		DoMethod(tmp.BT_SaveScript		, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_SaveScript);
		DoMethod(tmp.BT_ClearScript	, MUIM_Notify, MUIA_Pressed			, FALSE				, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_ClearScript);

		DoMethod(obj						, MUIM_Notify, MUIA_Window_CloseRequest, TRUE			, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_IconBarPrefs_Finish, obj, 0);
		DoMethod(tmp.BT_Cancel			, MUIM_Notify, MUIA_Pressed			, FALSE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_IconBarPrefs_Finish, obj, 0);
		DoMethod(tmp.BT_Use				, MUIM_Notify, MUIA_Pressed			, FALSE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_IconBarPrefs_Finish, obj, 1);
		DoMethod(tmp.BT_Save				, MUIM_Notify, MUIA_Pressed			, FALSE				, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_IconBarPrefs_Finish, obj, 2);

		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_RESET)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 2, MUIM_IconBarPrefs_Reset, 0);
	}
	else
	{
		if(undo_handle)
			CloseClipboard(undo_handle);
		undo_handle = NULL;
		if(clip_handle)
			CloseClipboard(clip_handle);
		clip_handle = NULL;
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG IconBarPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW									: return(IconBarPrefs_New			(cl, obj, (APTR)msg));
		case OM_DISPOSE							: return(IconBarPrefs_Dispose		(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_LoadIcons		: return(IconBarPrefs_LoadIcons	(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_Reset			: return(IconBarPrefs_Reset		(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_NewIcon		: return(IconBarPrefs_NewIcon		(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_Rows			: return(IconBarPrefs_Rows			(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_ModifyIcon	: return(IconBarPrefs_ModifyIcon	(cl, obj, (APTR)msg));
		case MUIM_IconBarPrefs_SetStates		: return(IconBarPrefs_SetStates	(cl, obj, (APTR)msg));
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
	struct Button_Data tmp;
	struct Icon *icon;
	Object *bodychunk = NULL;

	if(icon = (struct Icon *)GetTagData(MUIA_NetConnect_Icon, (ULONG)"", msg->ops_AttrList))
	{
		if(load_icon(icon))
		{
			bodychunk = (Object *)DoSuperNew(cl, obj,
				ButtonFrame,
				MUIA_CycleChain				, 1,
				MUIA_Bitmap_SourceColors	, (ULONG *)icon->cols,
				MUIA_Bitmap_Width				, icon->bmhd->bmh_Width,
				MUIA_Bitmap_Height			, icon->bmhd->bmh_Height,
				MUIA_FixWidth					, icon->bmhd->bmh_Width,
				MUIA_FixHeight					, icon->bmhd->bmh_Height,
				MUIA_Background				, MUII_ButtonBack,
				MUIA_InputMode					, MUIV_InputMode_RelVerify,
				MUIA_Bodychunk_Depth			, icon->bmhd->bmh_Depth,
				MUIA_Bodychunk_Body			, icon->body,
				MUIA_Bodychunk_Compression	, icon->bmhd->bmh_Compression,
				MUIA_Bodychunk_Masking		, icon->bmhd->bmh_Masking,
				MUIA_Bitmap_Transparent		, 0,
				TAG_MORE, msg->ops_AttrList);
		}

		if(!bodychunk)
		{
			icon->body = NULL;		/* so it doesn't get free'ed */
			icon->bmhd = NULL;
			icon->cols = NULL;
			bodychunk = (Object *)DoSuperNew(cl, obj,
				ButtonFrame,
				MUIA_CycleChain				, 1,
				MUIA_Bitmap_SourceColors	, (ULONG *)default_icon_colors,
				MUIA_Bitmap_Width				, DEFAULT_ICON_WIDTH ,
				MUIA_Bitmap_Height			, DEFAULT_ICON_HEIGHT,
				MUIA_FixWidth					, DEFAULT_ICON_WIDTH ,
				MUIA_FixHeight					, DEFAULT_ICON_HEIGHT,
				MUIA_Background				, MUII_ButtonBack,
				MUIA_InputMode					, MUIV_InputMode_RelVerify,
				MUIA_Bodychunk_Depth			, DEFAULT_ICON_DEPTH ,
				MUIA_Bodychunk_Body			, (UBYTE *)default_icon_body,
				MUIA_Bodychunk_Compression	, DEFAULT_ICON_COMPRESSION,
				MUIA_Bodychunk_Masking		, DEFAULT_ICON_MASKING,
				MUIA_Bitmap_Transparent		, 0,
				TAG_MORE, msg->ops_AttrList);
		}

		if(bodychunk)
		{
			struct Button_Data *data = INST_DATA(cl, bodychunk);

			if(tmp.icon = AllocVec(sizeof(struct Icon), MEMF_ANY))
			{
				memcpy(tmp.icon, icon, sizeof(struct Icon));

				set(bodychunk, MUIA_ShortHelp, tmp.icon->Name);
				DoMethod(bodychunk, MUIM_Notify, MUIA_Pressed, FALSE, bodychunk, 1, MUIM_Button_Action);

				tmp.icon->cx_filter = NULL;
				if(*tmp.icon->Hotkey && CxBase && xget(app, MUIA_Application_Broker) && xget(app, MUIA_Application_BrokerPort))
				{
					if(tmp.icon->cx_filter = CxFilter(tmp.icon->Hotkey))
					{
						BOOL success = FALSE;
						CxObj *sender;
						CxObj *translator;

						AttachCxObj((CxObj *)xget(app, MUIA_Application_Broker), tmp.icon->cx_filter);
						if(sender = CxSender(xget(app, MUIA_Application_BrokerPort), bodychunk))
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

	return((ULONG)bodychunk);
}

SAVEDS ASM ULONG Button_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW									: return(Button_New				(cl, obj, (APTR)msg));
		case OM_DISPOSE							: return(Button_Dispose			(cl, obj, (APTR)msg));
		case MUIM_Button_Action					: return(Button_Action			(cl, obj, (APTR)msg));
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

	if(group = RowGroup(Rows),
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
									ReadChunkBytes(Handle, &Rows, MIN(sizeof(LONG), cn->cn_Size));
									set(group, MUIA_Group_Rows, Rows);
								}

								if(cn->cn_ID == ID_AICN)
								{
									if(ReadChunkBytes(Handle, icon, MIN(sizeof(struct Icon), cn->cn_Size)) == MIN(sizeof(struct Icon), cn->cn_Size))
									{
										icon->body		= NULL;
										icon->list		= NULL;
										icon->cols		= NULL;
										icon->bmhd		= NULL;
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
										if(button = NewObject(CL_Button->mcc_Class, NULL,
											MUIA_NetConnect_Icon, icon,
											TAG_DONE))
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
				/* there's nothing in both lists => create at least the start/stop icon */
				strcpy(icon->Name		, "Start / Stop");
				strcpy(icon->ImageFile, IMAGE_PATH"Start");
				strcpy(icon->Program	, "AmiTCP:bin/startnet");
				icon->Type	= 2;

				if(button = NewObject(CL_Button->mcc_Class, NULL,
					MUIA_NetConnect_Icon, icon,
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
		struct IconBarPrefs_Data *ibp_data = INST_DATA(CL_IconBarPrefs->mcc_Class, window);

		DoMethod(app, OM_ADDMEMBER, window);
		setslider(ibp_data->SL_Rows, Rows);

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
	LONG i;

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
						LONG pos = 0;

						if(!PushChunk(Handle, ID_NTCN, ID_ROWS, IFFSIZE_UNKNOWN))
							if(WriteChunkBytes(Handle, &Rows, sizeof(LONG)) == sizeof(LONG))
								PopChunk(Handle);

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
		DoMethod(win, MUIM_IconBar_LoadButtons);

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

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title		, GetStr(MSG_WI_IconBar),
		MUIA_Window_ID			, MAKE_ID('I','C','O','N'),
		MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, IconBarMenu,0),
		WindowContents			, VGroup,
			MUIA_Group_Spacing, 0,
			MUIA_InnerBottom	, 0,
			MUIA_InnerLeft		, 0,
			MUIA_InnerTop		, 0,
			MUIA_InnerRight	, 0,
			Child, tmp.GR_Buttons = VGroup,
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
	if(CL_IconList)				MUI_DeleteCustomClass(CL_IconList);
	if(CL_IconBarPrefs)			MUI_DeleteCustomClass(CL_IconBarPrefs);

	if(CL_About)					MUI_DeleteCustomClass(CL_About);
	if(CL_Button)					MUI_DeleteCustomClass(CL_Button);
	if(CL_IconBar)					MUI_DeleteCustomClass(CL_IconBar);

	CL_IconBar			= CL_Button				=
	CL_IconBarPrefs	= CL_IconList			= 
	CL_About				= NULL;
}


/*
 * initialize our custom classes
 */

BOOL init_classes(VOID)
{
	CL_IconBar				= MUI_CreateCustomClass(NULL, MUIC_Window		, NULL, sizeof(struct IconBar_Data)				, IconBar_Dispatcher);
	CL_Button				= MUI_CreateCustomClass(NULL, MUIC_Bodychunk	, NULL, sizeof(struct Button_Data)				, Button_Dispatcher);
	CL_About					= MUI_CreateCustomClass(NULL, MUIC_Window		, NULL, sizeof(struct About_Data)				, About_Dispatcher);

	CL_IconBarPrefs		= MUI_CreateCustomClass(NULL, MUIC_Window		, NULL, sizeof(struct IconBarPrefs_Data)		, IconBarPrefs_Dispatcher);
	CL_IconList				= MUI_CreateCustomClass(NULL, MUIC_List		, NULL, sizeof(struct IconList_Data)			, IconList_Dispatcher);

	if(CL_IconBar			&& CL_Button	&& CL_About	&&
		CL_IconBarPrefs	&& CL_IconList )
		return(TRUE);

	exit_classes();
	return(FALSE);
}
