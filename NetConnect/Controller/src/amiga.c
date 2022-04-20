#include "globals.c"
#include "protos.h"


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
	return(KeyLabel2(GetStr(label), *GetStr(control_char)));
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
	Object *obj = KeyString(string, len, *(GetStr(control_char)));
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

/*
 * loads bitmap image of an icon.
 * icon->cols, icon->bmhd and icon->body
 * are beeing allocated
 */

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

/*
 * calls load_icon() and generates
 * a BodychunkObject.
 * nothing is changed in icon by
 * this routine (but load_icon does !)
 */

Object *create_bodychunk(struct Icon *icon, BOOL frame)
{
	Object *bodychunk = NULL;

	if(load_icon(icon))
	{
		bodychunk = BodychunkObject,
			MUIA_Background				, MUII_ButtonBack,
			MUIA_Frame						, (frame ? MUIV_Frame_Button : MUIV_Frame_None),
			MUIA_Bitmap_SourceColors	, icon->cols,
			MUIA_Bitmap_Width				, icon->bmhd->bmh_Width,
			MUIA_Bitmap_Height			, icon->bmhd->bmh_Height,
			MUIA_FixWidth					, icon->bmhd->bmh_Width,
			MUIA_FixHeight					, icon->bmhd->bmh_Height,
			MUIA_Bodychunk_Depth			, icon->bmhd->bmh_Depth,
			MUIA_Bodychunk_Body			, icon->body,
			MUIA_Bodychunk_Compression	, icon->bmhd->bmh_Compression,
			MUIA_Bodychunk_Masking		, icon->bmhd->bmh_Masking,
			MUIA_Bitmap_Transparent		, 0,
			End;
	}
	if(!bodychunk)
	{
		if(!stricmp(&icon->ImageFile[strlen(icon->ImageFile) - 5], ".info"))
			icon->ImageFile[strlen(icon->ImageFile) - 5] = NULL;
		if(icon->disk_object = GetDiskObjectNew(icon->ImageFile))
		{
			if(!(bodychunk = ImageObject,
				MUIA_Background		, MUII_ButtonBack,
				MUIA_Frame				, (frame ? MUIV_Frame_Button : MUIV_Frame_None),
				MUIA_Image_OldImage	, icon->disk_object->do_Gadget.GadgetRender,
			End))
			{
				FreeDiskObject(icon->disk_object);
				icon->disk_object = NULL;
			}
		}
	}
	if(!bodychunk)
	{
		icon->body = NULL;
		icon->bmhd = NULL;
		icon->cols = NULL;
		icon->disk_object = NULL;
		bodychunk = BodychunkObject,
			MUIA_Background				, MUII_ButtonBack,
			MUIA_Frame						, (frame ? MUIV_Frame_Button : MUIV_Frame_None),
			MUIA_Bitmap_SourceColors	, (ULONG *)default_icon_colors,
			MUIA_Bitmap_Width				, DEFAULT_ICON_WIDTH ,
			MUIA_Bitmap_Height			, DEFAULT_ICON_HEIGHT,
			MUIA_FixWidth					, DEFAULT_ICON_WIDTH ,
			MUIA_FixHeight					, DEFAULT_ICON_HEIGHT,
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
	icon->body			= NULL;
	icon->list			= NULL;
	icon->cols			= NULL;
	icon->bmhd			= NULL;
	icon->cx_filter	= NULL;
	icon->edit_window	= NULL;
	icon->disk_object	= NULL;

	if((icon->bodychunk = create_bodychunk(icon, TRUE)) && list && !icon->disk_object)
		icon->list = (APTR)DoMethod(list, MUIM_List_CreateImage, icon->bodychunk, 0);
}

LONG get_file_size(STRPTR file)
{
	struct FileInfoBlock *fib;
	BPTR lock;
	LONG size = -1;

	if(lock = Lock(file, ACCESS_READ))
	{
		if(fib = AllocDosObject(DOS_FIB, NULL))
		{
			if(Examine(lock, fib))
				size = (fib->fib_DirEntryType > 0 ? -2 : fib->fib_Size);

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

	if((size = get_file_size(file)) > -1)
	{
		if(buf = AllocVec(size + 1, MEMF_ANY))
		{
			if(fh = Open(file, MODE_OLDFILE))
			{
				if(Read(fh, buf, size) == size)
					success = TRUE;
				Close(fh);
			}
			buf[size] = NULL;		// We need buffers which are terminated by a zero
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
	STRPTR buf;

	ap = amsg->am_ArgList;
	if(amsg->am_NumArgs)
	{
		if(buf = AllocVec(MAXPATHLEN + 1, MEMF_ANY))
		{
			NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
			AddPart(buf, ap->wa_Name, MAXPATHLEN);
			setstring(obj, buf);
			FreeVec(buf);
		}
	}
	return(NULL);
}

SAVEDS ASM LONG Editor_AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x)
{
	struct WBArg *ap;
	struct AppMessage *amsg = *x;
	STRPTR buf;
	int i;

	if(amsg->am_NumArgs)
	{
		if(buf = AllocVec(MAXPATHLEN + 1, MEMF_ANY))
		{
			for(ap = amsg->am_ArgList, i = 0; i < amsg->am_NumArgs; i++, ap++)
			{
				NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
				AddPart(buf, ap->wa_Name, MAXPATHLEN);
				DoMethod(obj, MUIM_List_InsertSingle, buf, MUIV_List_Insert_Active);
			}
			set(obj, MUIA_UserData, 1);
			FreeVec(buf);
		}
	}
	return(NULL);
}

BOOL editor_load(STRPTR file, Object *editor)
{
	STRPTR buf, ptr1, ptr2;
	LONG size;
	BOOL success = FALSE;

	if(*file)
	{
		set(editor, MUIA_List_Quiet, TRUE);
		DoMethod(editor, MUIM_List_Clear);

		size = get_file_size(file);
		if(buf = LoadFile(file))
		{
			ptr1 = buf;
			while(ptr1 && ptr1 < buf + size)
			{
				if(ptr2 = strchr(ptr1, '\n'))
				{
					*ptr2 = NULL;
					DoMethod(editor, MUIM_List_InsertSingle, ptr1, MUIV_List_Insert_Bottom);
					ptr1 = ptr2 + 1;
				}
				else
					ptr1 = NULL;
			}
			FreeVec(buf);
			success = TRUE;
		}
		set(editor, MUIA_List_Quiet, FALSE);
		set(editor, MUIA_List_Active, MUIV_List_Active_Top);
	}
	return(success);
}

BOOL editor_save(STRPTR file, Object *editor)
{
	BPTR fh;
	STRPTR ptr;
	int i;
	BOOL success = FALSE;

	if(*file)
	{
		if(fh = Open(file, MODE_NEWFILE))
		{
			i = 0;
			FOREVER
			{
				DoMethod(editor, MUIM_List_GetEntry, i++, &ptr);
				if(!ptr)
					break;
				FPrintf(fh, "%ls\n", ptr);
			}

			Close(fh);
			success = TRUE;
			set(editor, MUIA_UserData, NULL);
		}
	}
	return(success);
}

BOOL editor_checksave(STRPTR file, Object *editor)
{
	BOOL success = FALSE;

	if(xget(editor, MUIA_UserData))
	{
		if(*file)
		{
			DoMethod((Object *)xget(editor, MUIA_WindowObject), MUIM_Window_ToFront);
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

ULONG BuildCommandLine(char *buf, struct Program *program, BPTR curdir, struct AppMessage *msg)
{
	ULONG cmdlen;      /* Command line length */
	STRPTR lp;          /* Pointer to current cmdline pos. */
	STRPTR com = program->File;

	*buf = NULL;
	if(program->Type == TYPE_SCRIPT)
		strcpy(buf, "Execute ");
	if(program->Type == TYPE_AREXX)
		strcpy(buf, "SYS:Rexxc/rx ");

	strcat(buf, com);

	if(lp = strchr(buf, '['))
		*lp = NULL;

	cmdlen = strlen(buf);
	lp = buf + cmdlen;

	if(msg)
	{
		STRPTR dir; /* Buffer for file names */

		if(dir = AllocVec(MAXPATHLEN, MEMF_ANY))
		{
			struct WBArg *wa = msg->am_ArgList;		/* Pointer to WBArgs */
			int i;											/* Counter for WBArgs */

			for(i = msg->am_NumArgs; i; i--, wa++)
			{
				char *name, *space;
				ULONG namelen;

				if(!wa->wa_Lock)
					continue;

				if(cmdlen > CMDLINELEN - 2)
					break;
				*lp++=' ';
				cmdlen++;

				/* Build parameter from Lock & name */
				if(*(wa->wa_Name))
				{
					if(SameLock(curdir, wa->wa_Lock) == LOCK_SAME)
						name=wa->wa_Name;
					else
					{
						if(!NameFromLock(wa->wa_Lock, dir, MAXPATHLEN))
							continue;
						if(!AddPart(dir, wa->wa_Name, MAXPATHLEN))
							continue;
						name = dir;
					}
				}
				else		// no filename => drawer
				{
					if(!NameFromLock(wa->wa_Lock, dir, MAXPATHLEN))
						continue;
					name = dir;
				}
				namelen = strlen(name);

				if(space = strchr(name,' '))
					namelen += 2;

				if(cmdlen + namelen > CMDLINELEN - 2)
					break;

				if(space)
					*lp++ = '"';
				strcpy(lp, name);
				lp += namelen;
				if(space)
				{
					lp--;
					*(lp-1) = '"';
				}
				cmdlen += namelen;
			}
			FreeVec(dir);
		}
	}

	if((com = strchr(com, ']')) && (cmdlen + strlen(++com) < CMDLINELEN - 1))
	{
		strcpy(lp, com);
		lp = lp + strlen(lp);
	}
	else
		*lp = NULL;

	return((ULONG)(lp - buf));
}

BOOL StartCLIProgram(struct Program *program, struct AppMessage *msg)
{
	char *cmd;     /* Buffer for command line */
	BOOL success = FALSE;

	if(cmd = AllocVec(CMDLINELEN, MEMF_ANY))
	{
		BPTR newcd = NULL;

		if(*program->CurrentDir)
		{
			newcd = Lock(program->CurrentDir, SHARED_LOCK);
		}
		else
		{
			STRPTR buf, ptr;

			if(buf = AllocVec(MAXPATHLEN + 1, MEMF_ANY))
			{
				strncpy(buf, program->File, MAXPATHLEN);
				if(ptr = FilePart(buf))
					*ptr = NULL;
				newcd = Lock(buf, SHARED_LOCK);

				FreeVec(buf);
			}
		}
		if(!newcd)
			newcd = Lock("SYS:", SHARED_LOCK);
		if(newcd)
		{
			BPTR ifh, ofh;
			BPTR oldcd = CurrentDir(newcd);

			BuildCommandLine(cmd, program, newcd, msg);

			if(ofh = Open((*program->OutputFile ? program->OutputFile : "NIL:"), MODE_NEWFILE))
			{
				if(ifh = Open("NIL:", MODE_OLDFILE))
				{
					if(SystemTags(cmd,
						SYS_Output		, ofh,
						SYS_Input		, ifh,
						SYS_Asynch		, program->Asynch,
						SYS_UserShell	, TRUE,
						NP_StackSize	, program->Stack,
						NP_Priority		, program->Priority,
						TAG_DONE) != -1)
							success = TRUE;

					if(!success || !program->Asynch)
						Close(ifh);
				}
				if(!success || !program->Asynch)
					Close(ofh);
			}
			CurrentDir(oldcd);
			UnLock(newcd);
		}
		FreeVec(cmd);
	}
	return(success);
}

BOOL StartWBProgram(struct Program *program, struct AppMessage *msg)
{
	struct MsgPort *hp, *mp;
	struct WBStartMsg wbsm;
	BOOL success = FALSE;

	if(mp = CreateMsgPort())
	{
		if(*program->CurrentDir)
		{
			wbsm.wbsm_DirLock = Lock(program->CurrentDir, SHARED_LOCK);
		}
		else
		{
			STRPTR buf, ptr;

			wbsm.wbsm_DirLock = NULL;
			if(buf = AllocVec(MAXPATHLEN + 1, MEMF_ANY))
			{
				strncpy(buf, program->File, MAXPATHLEN);
				if(ptr = FilePart(buf))
					*ptr = NULL;
				wbsm.wbsm_DirLock = Lock(buf, SHARED_LOCK);

				FreeVec(buf);
			}
		}

		wbsm.wbsm_Msg.mn_Node.ln_Pri	= 0;
		wbsm.wbsm_Msg.mn_ReplyPort		= mp;
		wbsm.wbsm_Name						= program->File;
		wbsm.wbsm_Stack					= program->Stack;
		wbsm.wbsm_Prio						= program->Priority;
		wbsm.wbsm_NumArgs					= msg ? msg->am_NumArgs : NULL;
		wbsm.wbsm_ArgList					= msg ? msg->am_ArgList : NULL;

		Forbid();
		if(hp = FindPort(WBS_PORTNAME))
			PutMsg(hp, (struct Message *)&wbsm);
		Permit();

		/* No WBStart-Handler, try to start it! */
		if(!hp)
		{
			BPTR ifh = Open("NIL:", MODE_NEWFILE);
			BPTR ofh = Open("NIL:", MODE_OLDFILE);

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

				for(i = 0; i < 10; i++)
				{
					Forbid();
					if(hp = FindPort(WBS_PORTNAME))
						PutMsg(hp, (struct Message *)&wbsm);
					Permit();
					if(hp)
						break;
					Delay(25);
				}
			}
			else
			{
				Close(ofh);
				Close(ifh);
			}
		}

		if(hp)
		{
			WaitPort(mp);
			GetMsg(mp);
			success = wbsm.wbsm_Stack;		// Has tool been started?
		}

		if(wbsm.wbsm_DirLock)
			UnLock(wbsm.wbsm_DirLock);
		DeleteMsgPort(mp);
	}

	return(success);
}


VOID StartProgram(struct Program *program, struct AppMessage *msg)
{
	struct AppMessage *args = (program->Flags & PRG_Arguments) ? msg : NULL;
	BOOL success = FALSE;

	set(win, MUIA_Window_Sleep, TRUE);
	switch(program->Type)
	{
		case TYPE_CLI:
		case TYPE_SCRIPT:
		case TYPE_AREXX:
			success = StartCLIProgram(program, args);
			break;
		case TYPE_WORKBENCH:
			success = StartWBProgram(program, args);
			break;
	}

	if(!success)
		DisplayBeep(NULL);
	set(win, MUIA_Window_Sleep, FALSE);
}
