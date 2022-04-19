#include "globals.c"
#include "protos.h"


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

SAVEDS ASM LONG sortfunc(REG(a1) STRPTR str1, REG(a2) STRPTR str2)
{
	return(stricmp(str1, str2));
}


SAVEDS ASM LONG strobjfunc(REG(a2) Object *list, REG(a1) Object *str)
{
	char *x, *s;
	int i;

	get(str, MUIA_String_Contents, &s);

	i = 0;
	FOREVER
	{
		DoMethod(list, MUIM_List_GetEntry, i, &x);
		if(!x)
		{
			nnset(list, MUIA_List_Active, MUIV_List_Active_Off);
			break;
		}
		else
		{
			if(!stricmp(x, s))
			{
				nnset(list, MUIA_List_Active, i);
				break;
			}
		}

		i++;
	}
	return(TRUE);
}

SAVEDS ASM LONG txtobjfunc(REG(a2) Object *list, REG(a1) Object *str)
{
	char *x, *s;
	int i;

	get(str, MUIA_Text_Contents, &s);

	i = 0;
	FOREVER
	{
		DoMethod(list, MUIM_List_GetEntry, i, &x);
		if(!x)
		{
			nnset(list, MUIA_List_Active, MUIV_List_Active_Off);
			break;
		}
		else
		{
			if(!stricmp(x, s))
			{
				nnset(list, MUIA_List_Active, i);
				break;
			}
		}

		i++;
	}
	return(TRUE);
}


Object *MakeKeyLabel1(STRPTR label, STRPTR control_char)
{
	return(KeyLabel1(GetStr(label), *GetStr(control_char)));
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
		}
	}
	return(success);
}

BOOL ParseConfig(STRPTR file, struct pc_Data *pc_data)
{
	LONG size;
	STRPTR buf = NULL;
	BPTR fh;
	BOOL success = FALSE;

	if((size = get_file_size(file)) > -1)
	{
		if(buf = AllocVec(size, MEMF_ANY))
		{
			if(fh = Open(file, MODE_OLDFILE))
			{
				if(Read(fh, buf, size) == size)
				{
					success = TRUE;

					pc_data->Buffer	= buf;
					pc_data->Size		= size;
					pc_data->Current	= buf;

					pc_data->Argument	= NULL;
					pc_data->Contents	= NULL;
				}

				Close(fh);
			}
		}
	}

	return(success);
}

BOOL ParseNext(struct pc_Data *pc_data)
{
	BOOL success = FALSE;
	STRPTR ptr_eol, ptr_tmp;

	if(pc_data->Current && pc_data->Current < pc_data->Buffer + pc_data->Size)
	{
		if(ptr_eol = strchr(pc_data->Current, '\n'))
		{
			*ptr_eol = NULL;

			if(pc_data->Contents = strchr(pc_data->Current, 34))					/* is the content between ""'s ? */
			{
				pc_data->Contents++;
				if(ptr_tmp = strchr(pc_data->Contents, 34))	/* find the ending '"' */
					*ptr_tmp = NULL;

				ptr_tmp = pc_data->Contents - 2;
				while(((*ptr_tmp == ' ') || (*ptr_tmp == 9)) && ptr_tmp >= pc_data->Current)
					ptr_tmp--;

				ptr_tmp++;
				*ptr_tmp = NULL;
			}
			else
			{
				pc_data->Contents	= strchr(pc_data->Current, ' ');							/* a space	*/
				ptr_tmp				= strchr(pc_data->Current, 9);							/* or a TAB	*/

				if((ptr_tmp < pc_data->Contents && ptr_tmp) || !pc_data->Contents)	/* which one comes first ? */
					pc_data->Contents = ptr_tmp;
				if(pc_data->Contents)
				{
					*pc_data->Contents++ = NULL;
					while((*pc_data->Contents == ' ') || (*pc_data->Contents == 9))
						pc_data->Contents++;

					ptr_tmp = NULL;
					if(!ptr_tmp)
					{
						if(ptr_tmp = strchr(pc_data->Contents, ';'))	/* cut out the comment */
							*ptr_tmp = NULL;
						if(ptr_tmp = strchr(pc_data->Contents, ' '))	/* is there a space left ? */
							*ptr_tmp = NULL;
						if(ptr_tmp = strchr(pc_data->Contents, 9))	/* or a TAB ? */
							*ptr_tmp = NULL;
					}
				}
				else
					pc_data->Contents = "";
			}

			pc_data->Argument = pc_data->Current;
			pc_data->Current	= ptr_eol + 1;
			success = TRUE;
		}
		else
			pc_data->Current = NULL;
	}

	return(success);
}

BOOL ParseNextLine(struct pc_Data *pc_data)
{
	BOOL success = FALSE;
	STRPTR ptr_eol;

	if(pc_data->Current && pc_data->Current < pc_data->Buffer + pc_data->Size)
	{
		if(ptr_eol = strchr(pc_data->Current, '\n'))
		{
			*ptr_eol = NULL;

			pc_data->Argument = "";
			pc_data->Contents = pc_data->Current;
			pc_data->Current	= ptr_eol + 1;
			success = TRUE;
		}
		else
			pc_data->Current = NULL;
	}

	return(success);
}

VOID ParseEnd(struct pc_Data *pc_data)
{
	if(pc_data->Buffer)
		FreeVec(pc_data->Buffer);

	pc_data->Buffer	= NULL;
	pc_data->Size		= NULL;
	pc_data->Current	= NULL;

	pc_data->Argument	= NULL;
	pc_data->Contents	= NULL;
}

BOOL CopyFile(STRPTR infile, STRPTR outfile)
{
	BPTR in, out;
	char buf[100];
	LONG i;
	BOOL success = FALSE;

	if(in = Open(infile, MODE_OLDFILE))
	{
		if(out = Open(outfile, MODE_NEWFILE))
		{
			success = TRUE;
			while(i = Read(in, buf, 100))
			{
				if(Write(out, buf, i) != i)
				{
					success = FALSE;
					break;
				}
			}
			Close(out);
		}
		Close(in);
	}
	return(success);
}
