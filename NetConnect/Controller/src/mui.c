#include "globals.c"

extern STRPTR GetStr(STRPTR idstr);


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

Object *create_bodychunk(struct Icon *icon)
{
	Object *bodychunk = NULL;
	struct BitMapHeader *bmhd;
	struct IFFHandle *Handle;
	struct ContextNode *cn;
	struct StoredProperty *sp;
	int size, i;
	UBYTE *src;

	if(Handle=AllocIFF())
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
									bmhd = (struct BitMapHeader *)sp->sp_Data;
									size = CurrentChunk(Handle)->cn_Size;

									if(icon->body = AllocVec(size, MEMF_ANY))
									{
										if(ReadChunkBytes(Handle, icon->body, size) == size)
										{
											bodychunk = BodychunkObject,
												ButtonFrame,
												MUIA_Bitmap_SourceColors	, icon->cols,
												MUIA_Bitmap_Width				, bmhd->bmh_Width,
												MUIA_Bitmap_Height			, bmhd->bmh_Height,
												MUIA_FixWidth					, bmhd->bmh_Width,
												MUIA_FixHeight					, bmhd->bmh_Height,
												MUIA_Background				, MUII_ButtonBack,
												MUIA_InputMode					, MUIV_InputMode_RelVerify,
												MUIA_Bodychunk_Depth			, bmhd->bmh_Depth,
												MUIA_Bodychunk_Body			, icon->body,
												MUIA_Bodychunk_Compression	, bmhd->bmh_Compression,
												MUIA_Bodychunk_Masking		, bmhd->bmh_Masking,
												MUIA_Bitmap_Transparent		, 0,
												End;
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
	if(!bodychunk)
	{
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
		if(buf = AllocVec(size, MEMF_ANY))
		{
			if(fh = Open(file, MODE_OLDFILE))
			{
				if(Read(fh, buf, size) == size)
					success = TRUE;
				Close(fh);
			}
		}
	}

	if(!success)
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



/****************************************************************************/
/* About class                                                              */
/****************************************************************************/

ULONG About_New(struct IClass *cl, Object *obj, Msg msg)
{
	struct About_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title, "About",
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
				MUIA_Background, MUII_ReadListBack,
				MUIA_Scrollgroup_FreeHoriz, FALSE,
				MUIA_Scrollgroup_Contents, VirtgroupObject,
					ReadListFrame,
					Child, TextObject,
						MUIA_Text_Contents, "\33c\nNetConnect Control 1.00 (13.6.96)\n© 1996 Michael Neuweiler\n\nAuthors:\nMichael Neuweiler\n...\n\n\n\33bDEMO\33n\n",
					End,
					Child, MUI_MakeObject(MUIO_HBar, 2),
					Child, TextObject,
						MUIA_Text_Contents, "\33c\n\NetConnect uses MUI\nMUI is copyrighted by Stefan Stuntz",
					End,
				End,
			End,
			Child, HGroup,
				Child, HSpace(0),
				Child, tmp.BT_Button = SimpleButton("_Okay"),
				Child, HSpace(0),
			End,
		End))
	{
		struct About_Data *data = INST_DATA(cl,obj);

		*data = tmp;

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
		case OM_NEW : return(About_New		(cl,obj,(APTR)msg));
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
		return(DoSuperMethodA(cl, obj, msg));			/*  MUI shall handle the moving of objects within the list */
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

	if(new = (struct Icon *)AllocVec(sizeof(struct Icon), MEMF_ANY | MEMF_CLEAR))
	{
		if(src)
			memcpy(new, src, sizeof(struct Icon));
	}
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
		FreeVec(icon);
	}
}

/*
 * This function tells MUI how to display
 * the list's entries
 */

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

	obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Frame					, MUIV_Frame_InputList,
		MUIA_List_DisplayHook	, &IconList_DisplayHook,
		MUIA_List_ConstructHook	, &IconList_ConstructHook,
		MUIA_List_DestructHook	, &IconList_DestructHook,
		MUIA_List_Format			, ",",
		MUIA_List_MinLineHeight	, find_max() + 6,
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
	switch (msg->MethodID)
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

							/* is it an active or inactive icon ? */
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
			/* if there's nothing in both lists create at least the start/stop icon */
			strcpy(icon->Name		, "Start / Stop");
			strcpy(icon->ImageFile, "Images/Start");
			strcpy(icon->Script	, "AmiTCP:bin/startnet");
			icon->Advanced	= TRUE;

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
		strcpy(icon->Name, "WWW");
		strcpy(icon->ImageFile, "Images/WWW");
		strcpy(icon->Program, "IBrowse");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Telnet");
		strcpy(icon->ImageFile, "Images/Telnet");
		strcpy(icon->Program, "Telnet");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "FTP");
		strcpy(icon->ImageFile, "Images/FTP");
		strcpy(icon->Program, "mFTP");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Ping");
		strcpy(icon->ImageFile, "Images/Ping");
		strcpy(icon->Program, "Ping");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Traceroute");
		strcpy(icon->ImageFile, "Images/Traceroute");
		strcpy(icon->Program, "Traceroute");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Finger");
		strcpy(icon->ImageFile, "Images/Finger");
		strcpy(icon->Program, "Finger");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "IRC");
		strcpy(icon->ImageFile, "Images/IRC");
		strcpy(icon->Program, "Irc");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Mail");
		strcpy(icon->ImageFile, "Images/Mail");
		strcpy(icon->Program, "Mail");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "News");
		strcpy(icon->ImageFile, "Images/News");
		strcpy(icon->Program, "News");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Search");
		strcpy(icon->ImageFile, "Images/Search");
		strcpy(icon->Program, "Search");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Docs");
		strcpy(icon->ImageFile, "Images/Docs");
		strcpy(icon->Program, "Docs");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_ActiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Misc");
		strcpy(icon->ImageFile, "Images/Misc");
		strcpy(icon->Program, "Misc");
		init_icon(icon, data->LI_ActiveIcons);
		DoMethod(data->LI_InactiveIcons, MUIM_List_InsertSingle, icon, MUIV_List_Insert_Bottom);
		strcpy(icon->Name, "Start / Stop");
		strcpy(icon->ImageFile, "Images/Start");
		strcpy(icon->Script	, "AmiTCP:bin/startnet");
		*icon->Program	= NULL;
		icon->Advanced	= TRUE;
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
		strcpy(icon->Name, "New");
		strcpy(icon->Program, "NetConnect:Programs/");
		strcpy(icon->ImageFile, "NetConnect:Images/");
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

			case MUIV_IconBarPrefs_ModifyIcon_Program:
				strcpy(icon->Program, (STRPTR)xget(data->PA_Program, MUIA_String_Contents));
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

			case MUIV_IconBarPrefs_ModifyIcon_Volume:
				icon->Volume = xget(data->SL_Volume, MUIA_Numeric_Value);
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Advanced:
				icon->Advanced = xget(data->CH_Advanced, MUIA_Selected);
				set(data->GR_Script, MUIA_Disabled, (icon->Advanced ? FALSE : TRUE));
				set(data->PA_Program, MUIA_Disabled, (icon->Advanced ? TRUE : FALSE));
				break;

			case MUIV_IconBarPrefs_ModifyIcon_Script:
				strcpy(icon->Script, (STRPTR)xget(data->PA_SaveScript, MUIA_String_Contents));
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

	set(data->SL_Rows, MUIA_Slider_Max, (xget(data->LI_ActiveIcons, MUIA_List_Entries) ? xget(data->LI_ActiveIcons, MUIA_List_Entries) : 1));

	nnset((msg->level ? data->LI_InactiveIcons : data->LI_ActiveIcons), MUIA_List_Active, MUIV_List_Active_Off);
	DoMethod((msg->level ? data->LI_InactiveIcons : data->LI_ActiveIcons), MUIM_List_Select, MUIV_List_Select_All, MUIV_List_Select_Ask, &i);
	if(i)
		DoMethod((msg->level ? data->LI_InactiveIcons : data->LI_ActiveIcons), MUIM_List_Select, MUIV_List_Select_All, MUIV_List_Select_Off, NULL);

	DoMethod((msg->level ? data->LI_ActiveIcons : data->LI_InactiveIcons), MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &icon);
	if(icon)
	{
		set(data->BT_Remove, MUIA_Disabled, FALSE);
		set(data->GR_Button, MUIA_Disabled, FALSE);
		set(data->GR_Script, MUIA_Disabled, (icon->Advanced ? FALSE : TRUE));
		set(data->PA_Program, MUIA_Disabled, (icon->Advanced ? TRUE : FALSE));

		setstring(data->STR_Name		, icon->Name);
		setstring(data->PA_Program		, icon->Program);
		setstring(data->PA_Image		, icon->ImageFile);
		setstring(data->PA_Sound		, icon->Sound);
		setslider(data->SL_Volume		, icon->Volume);
		setcheckmark(data->CH_Advanced, icon->Advanced);
		setstring(data->PA_SaveScript	, icon->Script);

/*		if(icon->Advanced)
		{
			STRPTR buf;

			if(buf = LoadFile(icon->Script))
			{
			  DoMethod(data->TV_Editor, MUIM_TextView_InsertText, buf, TVFontF_Normal);
			  FreeVec(buf);
			}
		}
*/	}
	else
	{
		set(data->BT_Remove, MUIA_Disabled, TRUE);
		set(data->GR_Script, MUIA_Disabled, TRUE);
		set(data->GR_Button, MUIA_Disabled, TRUE);

		setstring(data->STR_Name	, "");
		setstring(data->PA_Program	, "");
		setstring(data->PA_Image	, "");
		setstring(data->PA_Sound	, "");
		setslider(data->SL_Volume	, 0);
		setcheckmark(data->CH_Advanced, FALSE);
		setstring(data->PA_SaveScript	, "");
	}
	return(NULL);
}

ULONG IconBarPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct IconBarPrefs_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title		, GetStr(MSG_WI_IconBarPrefs),
		MUIA_Window_ID			, MAKE_ID('I','P','R','F'),
		MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, IconBarPrefsMenu, 0),
		WindowContents		, VGroup,
			MUIA_HelpNode, "WI_IconBarPrefs",
			MUIA_Frame, MUIV_Frame_Group,
			Child, HGroup,
				MUIA_HelpNode, "GR_IconBarPrefs_top",
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_InactiveIcons = ListviewObject,
						MUIA_FrameTitle				, GetStr(MSG_LV_ButtonBankTitle),
						MUIA_Listview_MultiSelect	, MUIV_Listview_MultiSelect_Default,
//						MUIA_Listview_DoubleClick	, TRUE,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_InactiveIcons = NewObject(CL_IconList->mcc_Class, NULL, TAG_DONE),
					End,
					Child, HGroup,
						MUIA_Group_Spacing, 0,
						Child, tmp.BT_New		= SimpleButton(GetStr(MSG_BT_New)),
						Child, tmp.BT_Remove = SimpleButton(GetStr(MSG_BT_Remove)),
					End,
				End,
				Child, VGroup,
					Child, tmp.LV_ActiveIcons = ListviewObject,
						MUIA_FrameTitle				, GetStr(MSG_LV_IconBarTitle),
						MUIA_Listview_MultiSelect	, MUIV_Listview_MultiSelect_Default,
//						MUIA_Listview_DoubleClick	, TRUE,
						MUIA_Listview_DragType		, 1,
						MUIA_Listview_List			, tmp.LI_ActiveIcons = NewObject(CL_IconList->mcc_Class, NULL, TAG_DONE),
					End,
					Child, HGroup,
						Child, Label("Rows :"),
						Child, tmp.SL_Rows = Slider(1, 10, Rows),
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
						Child, Label(GetStr(MSG_LA_Name)),
						Child, tmp.STR_Name = String("", 80),
						Child, Label(GetStr(MSG_LA_Program)),
						Child, tmp.PA_Program = PopaslObject,
							MUIA_Popasl_Type		, 0,
							MUIA_Popstring_String, String("", MAXPATHLEN),
							MUIA_Popstring_Button, PopButton(MUII_PopFile),
						End,
						Child, Label(GetStr(MSG_LA_Image)),
						Child, tmp.PA_Image = PopaslObject,
							MUIA_Popasl_Type		, 0,
							MUIA_Popstring_String, String("", MAXPATHLEN),
							MUIA_Popstring_Button, PopButton(MUII_PopFile),
						End,
						Child, Label(GetStr(MSG_LA_Sound)),
						Child, tmp.PA_Sound = PopaslObject,
							MUIA_Popasl_Type		, 0,
							MUIA_Popstring_String, String("", MAXPATHLEN),
							MUIA_Popstring_Button, PopButton(MUII_PopFile),
						End,
					End,
					Child, HGroup,
						Child, ColGroup(2),
							Child, HVSpace,
							Child, HVSpace,
							Child, tmp.CH_Advanced = CheckMark(FALSE),
							Child, LLabel1(GetStr(MSG_LA_Advanced)),
							Child, HVSpace,
							Child, HVSpace,
						End,
						Child, HVSpace,
						Child, VGroup,
							Child, tmp.SL_Volume = KnobObject,
								MUIA_Numeric_Min		, 0,
								MUIA_Numeric_Max		, 64,
								MUIA_Numeric_Value	, 64,
								MUIA_Numeric_Format	, "%3ld",
							End,
							Child, TextObject,
								MUIA_Font, MUIV_Font_Tiny,
								MUIA_Text_Contents, GetStr(MSG_LA_Volume),
								MUIA_Text_PreParse, "\33c",
							End,
						End,
					End,
				End,
				Child, tmp.GR_Script = VGroup,
					MUIA_HelpNode	, "GR_ScriptEdit",
					MUIA_Frame		, MUIV_Frame_Group,
					MUIA_FrameTitle, GetStr(MSG_GR_ScriptEditTitle),
					Child, tmp.GR_Editor = ScrollgroupObject,
						MUIA_Scrollgroup_Contents, tmp.TV_Editor =
HVSpace,
//TextViewObject,
//							VirtualFrame,
//							MUIA_Background, MUII_TextBack,
//							MUIA_TextView_Editable, TRUE,
//						End,
					End,
					Child, ColGroup(2),
						Child, Label(GetStr(MSG_LA_FindProgram)),
						Child, HGroup,
							MUIA_Group_HorizSpacing, 0,
							Child, tmp.PA_FindProgram = PopaslObject,
								MUIA_Popasl_Type		, 0,
								MUIA_Popstring_String, String("", MAXPATHLEN),
								MUIA_Popstring_Button, PopButton(MUII_PopFile),
							End,
							Child, tmp.BT_Add = SimpleButton(GetStr(MSG_BT_Add)),
						End,
						Child, Label(GetStr(MSG_LA_SaveScript)),
						Child, tmp.PA_SaveScript = PopaslObject,
							MUIA_Popasl_Type		, 0,
							MUIA_Popstring_String, String("", MAXPATHLEN),
							MUIA_Popstring_Button, PopButton(MUII_PopFile),
						End,
					End,
				End,
			End,
			Child, HGroup,
				MUIA_HelpNode			, "GR_IconBarPrefsControl",
				MUIA_Group_SameSize	, TRUE,
				Child, tmp.BT_Save	= SimpleButton(GetStr(MSG_BT_Save)),
				Child, HSpace(0),
				Child, tmp.BT_Use		= SimpleButton(GetStr(MSG_BT_Use)),
				Child, HSpace(0),
				Child, tmp.BT_Cancel	= SimpleButton(GetStr(MSG_BT_Cancel)),
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct IconBarPrefs_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		set(tmp.BT_Add, MUIA_Weight, 1);

		set(tmp.BT_Remove, MUIA_Disabled, TRUE);
		set(tmp.GR_Script, MUIA_Disabled, TRUE);
		set(tmp.GR_Button, MUIA_Disabled, TRUE);

		set(tmp.LI_InactiveIcons, MUIA_UserData, tmp.LI_ActiveIcons);
		set(tmp.LI_ActiveIcons, MUIA_UserData, tmp.LI_InactiveIcons);

		DoMethod(tmp.LV_ActiveIcons	, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime			, obj, 2, MUIM_IconBarPrefs_SetStates, 1);
		DoMethod(tmp.LV_InactiveIcons	, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime			, obj, 2, MUIM_IconBarPrefs_SetStates, 0);
		DoMethod(tmp.BT_New				, MUIM_Notify, MUIA_Pressed				, FALSE			, obj, 1, MUIM_IconBarPrefs_NewIcon);
		DoMethod(tmp.BT_Remove			, MUIM_Notify, MUIA_Pressed				, FALSE			, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Remove);
		DoMethod(tmp.SL_Rows				, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime		, obj, 2, MUIM_IconBarPrefs_Rows);
		DoMethod(tmp.STR_Name			, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Name);
		DoMethod(tmp.PA_Program			, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Program);
		DoMethod(tmp.PA_Image			, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Image);
		DoMethod(tmp.PA_Sound			, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Sound);
		DoMethod(tmp.SL_Volume			, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime		, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Volume);
		DoMethod(tmp.CH_Advanced		, MUIM_Notify, MUIA_Selected, MUIV_EveryTime				, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Advanced);
		DoMethod(tmp.PA_SaveScript		, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime	, obj, 2, MUIM_IconBarPrefs_ModifyIcon, MUIV_IconBarPrefs_ModifyIcon_Script);

		DoMethod(obj						, MUIM_Notify, MUIA_Window_CloseRequest, TRUE	, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_IconBarPrefs_Finish, obj, 0);
		DoMethod(tmp.BT_Cancel			, MUIM_Notify, MUIA_Pressed				, FALSE	, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_IconBarPrefs_Finish, obj, 0);
		DoMethod(tmp.BT_Use				, MUIM_Notify, MUIA_Pressed				, FALSE	, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_IconBarPrefs_Finish, obj, 1);
		DoMethod(tmp.BT_Save				, MUIM_Notify, MUIA_Pressed				, FALSE	, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_IconBar_IconBarPrefs_Finish, obj, 2);

		DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_RESET)		, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
			obj, 2, MUIM_IconBarPrefs_Reset, 0);
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG IconBarPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW									: return(IconBarPrefs_New			(cl, obj, (APTR)msg));
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

	if(*data->icon->Sound && DataTypesBase)
	{
		if(SoundObject)
			DisposeDTObject(SoundObject);

		if(SoundObject = NewDTObject(data->icon->Sound,
			DTA_SourceType	, DTST_FILE,
			DTA_GroupID		, GID_SOUND,
			SDTA_Volume		, data->icon->Volume,
			SDTA_Cycles		, 1,
		TAG_DONE))
		{
			DoMethod(SoundObject, DTM_TRIGGER, NULL, STM_PLAY, NULL);
		}
	}

	if(data->icon->Advanced)
		sprintf(command, "Execute %ls", data->icon->Script);
	else
		strcpy(command, data->icon->Program);

	SystemTags(command,
		SYS_Input		, NULL,
		SYS_Output		, NULL,
		SYS_Asynch		, TRUE,
		NP_CloseInput	, FALSE,
		NP_CloseOutput	, FALSE,
		TAG_DONE);

	if(data->icon->Advanced && (!stricmp(data->icon->Script, "AmiTCP:bin/startnet") || !stricmp(data->icon->Script, "AmiTCP:bin/stopnet"))) 
	{
		set(win, MUIA_Window_Sleep, TRUE);
		Delay(150);
		set(win, MUIA_Window_Sleep, FALSE);
		DoMethod(app, MUIM_Application_PushMethod, win, 1, MUIM_IconBar_LoadButtons);
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

	if(data->icon)
		FreeVec(data->icon);

	return(DoSuperMethodA(cl, obj, msg));
}

ULONG Button_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct Button_Data tmp;
	struct Icon *icon;
	struct BitMapHeader *bmhd;
	struct IFFHandle *Handle;
	struct ContextNode *cn;
	struct StoredProperty *sp;
	int size, i;
	UBYTE *src;
	BOOL success = FALSE;
	Object *bodychunk = NULL;

	if(icon = (struct Icon *)GetTagData(MUIA_NetConnect_Icon, (ULONG)"", msg->ops_AttrList))
	{
		if(Handle=AllocIFF())
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
										bmhd = (struct BitMapHeader *)sp->sp_Data;
										size = CurrentChunk(Handle)->cn_Size;

										if(icon->body = AllocVec(size, MEMF_ANY))
										{
											if(ReadChunkBytes(Handle, icon->body, size) == size)
											{
												if(bodychunk = (Object *)DoSuperNew(cl, obj,
													ButtonFrame,
													MUIA_Bitmap_SourceColors	, (ULONG *)icon->cols,
													MUIA_Bitmap_Width				, bmhd->bmh_Width,
													MUIA_Bitmap_Height			, bmhd->bmh_Height,
													MUIA_FixWidth					, bmhd->bmh_Width,
													MUIA_FixHeight					, bmhd->bmh_Height,
													MUIA_Background				, MUII_ButtonBack,
													MUIA_InputMode					, MUIV_InputMode_RelVerify,
													MUIA_Bodychunk_Depth			, bmhd->bmh_Depth,
													MUIA_Bodychunk_Body			, icon->body,
													MUIA_Bodychunk_Compression	, bmhd->bmh_Compression,
													MUIA_Bodychunk_Masking		, bmhd->bmh_Masking,
													MUIA_Bitmap_Transparent		, 0,
													TAG_MORE, msg->ops_AttrList))
												{
													success = TRUE;
												}
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
			icon->body = NULL;		/* so it doesn't get freed */
			if(bodychunk = (Object *)DoSuperNew(cl, obj,
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
				TAG_MORE, msg->ops_AttrList))
			{
				success = TRUE;
			}
		}
		if(success && bodychunk)
		{
			struct Button_Data *data = INST_DATA(cl, bodychunk);

			if(tmp.icon = AllocVec(sizeof(struct Icon), MEMF_ANY))
			{
				memcpy(tmp.icon, icon, sizeof(struct Icon));

				DoMethod(bodychunk, MUIM_Notify, MUIA_Pressed, FALSE	, bodychunk, 1, MUIM_Button_Action);
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

								/* is it an active icon ? */
								if(cn->cn_ID == ID_AICN)
								{
									/* load the data into the buffer */
									if(ReadChunkBytes(Handle, icon, MIN(sizeof(struct Icon), cn->cn_Size)) == MIN(sizeof(struct Icon), cn->cn_Size))
									{
										if(icon->Advanced && (!stricmp(icon->Script, "AmiTCP:bin/startnet") || !stricmp(icon->Script, "AmiTCP:bin/stopnet"))) 
										{
											STRPTR ptr;
											struct Library *lib;

											if(lib = OpenLibrary("bsdsocket.library", NULL))
											{
												strcpy(icon->Script, "AmiTCP:bin/stopnet");
												if(ptr = FilePart(icon->ImageFile))
												{
													*ptr = NULL;
													AddPart(icon->ImageFile, "Stop", MAXPATHLEN);
												}
												CloseLibrary(lib);
											}
											else
											{
												strcpy(icon->Script, "AmiTCP:bin/startnet");
												if(ptr = FilePart(icon->ImageFile))
												{
													*ptr = NULL;
													AddPart(icon->ImageFile, "Start", MAXPATHLEN);
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
				strcpy(icon->ImageFile, "Images/Start");
				strcpy(icon->Script	, "AmiTCP:bin/startnet");
				icon->Advanced	= TRUE;

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
	}
	set(app, MUIA_Application_Sleep, FALSE);
	set(win, MUIA_Window_Sleep, TRUE);

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
	SystemTags("Config",
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

	set(win, MUIA_Window_Sleep, TRUE);
	if(req = (APTR)NewObject(CL_About->mcc_Class, NULL, TAG_DONE))
	{
		DoMethod(app, OM_ADDMEMBER, req);
		set(req, MUIA_Window_Open, TRUE);
	}
	else
		set(win, MUIA_Window_Sleep, FALSE);

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

//	TextView_ExitClasses();

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
//&& TextView_InitClasses())
		return(TRUE);

	exit_classes();
	return(FALSE);
}
