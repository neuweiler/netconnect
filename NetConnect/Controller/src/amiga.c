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
		icon->body = NULL;
		icon->bmhd = NULL;
		icon->cols = NULL;
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
	icon->body		= NULL;
	icon->list		= NULL;
	icon->cols		= NULL;
	icon->bmhd		= NULL;
	if(icon->bodychunk = create_bodychunk(icon, TRUE))
	{
		if(list)
			icon->list = (APTR)DoMethod(list, MUIM_List_CreateImage, icon->bodychunk, 0);
	}
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
	char buf[MAXPATHLEN];

	ap = amsg->am_ArgList;
	if(amsg->am_NumArgs)
	{
		NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
		AddPart(buf, ap->wa_Name, MAXPATHLEN);
		setstring(obj, buf);
	}

	return(NULL);
}

SAVEDS ASM LONG Editor_AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x)
{
	struct WBArg *ap;
	struct AppMessage *amsg = *x;
	char buf[MAXPATHLEN];

	ap = amsg->am_ArgList;
	if(amsg->am_NumArgs)
	{
		NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
		AddPart(buf, ap->wa_Name, MAXPATHLEN);
		DoMethod(obj, MUIM_List_InsertSingle, buf, MUIV_List_Insert_Active);
		set(obj, MUIA_UserData, 1);
	}

	return(NULL);
}

SAVEDS ASM LONG Button_AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x)
{
	struct Button_Data *data = INST_DATA(CL_Button->mcc_Class, obj);
	struct WBArg *ap;
	struct AppMessage *amsg = *x;
	char buf[MAXPATHLEN], program[MAXPATHLEN];
	STRPTR ptr;
	ap = amsg->am_ArgList;
	if(amsg->am_NumArgs)
	{
		NameFromLock(ap->wa_Lock, buf, MAXPATHLEN);
		AddPart(buf, ap->wa_Name, MAXPATHLEN);

		strncpy(program, data->icon->Program, MAXPATHLEN);
		if(ptr = strstr(program, "%f"))
		{
			*ptr = NULL;
			strncat(program, buf, MAXPATHLEN);
			if(ptr = strstr(data->icon->Program, "%f"))
			{
				ptr += 2;
				strncat(program, ptr, MAXPATHLEN);
			}
		}
		else
		{
			strncat(program, " ", MAXPATHLEN);
			strncat(program, buf, MAXPATHLEN);
		}
		strncpy(buf, data->icon->Program, MAXPATHLEN);
		strncpy(data->icon->Program, program, MAXPATHLEN);

		DoMethod(obj, MUIM_Button_Action);
		strncpy(data->icon->Program, buf, MAXPATHLEN);
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
	LONG i;
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
