#include "globals.c"

/// xget
LONG xget(Object *obj,ULONG attribute)
{
	LONG x;
	get(obj,attribute,&x);
	return(x);
}

///
/// xgetstr
char *xgetstr(Object *obj)
{
	return((char *)xget(obj,MUIA_String_Contents));
}

///
/// xgetbool
BOOL xgetbool(Object *obj)
{
	return((BOOL)xget(obj,MUIA_Selected));
}

///
/// DoSuperNew
ULONG __stdargs DoSuperNew(struct IClass *cl,Object *obj,ULONG tag1,...)
{
	return(DoSuperMethod(cl,obj,OM_NEW,&tag1,NULL));
}

///
/// MakeButton
Object *MakeButton(STRPTR string)
{
	Object *obj = MUI_MakeObject(MUIO_Button, GetStr(string));
	if(obj)
		set(obj,MUIA_CycleChain,1);
	return(obj);
}

///
/// MakeText
Object *MakeText(STRPTR string)
{
	Object *obj = TextObject,
		MUIA_Text_Contents,  (string ? string : (STRPTR)""),
		MUIA_FixWidthTxt, "00000000000000000000000000000000000",
	End;

	return(obj);
}

///
/// MakeKeyString
Object *MakeKeyString(STRPTR string, LONG len, char c)
{
	Object *obj = KeyString(string, len, c);

	if(obj)
		set(obj,MUIA_CycleChain,1);
	return(obj);
}

///
/// MakeString
Object *MakeString(STRPTR string, LONG len)
{
	Object *obj = String(string, len);

	if(obj)
		set(obj,MUIA_CycleChain,1);
	return(obj);
}

///
/// MakeKeyCycle
Object *MakeKeyCycle(STRPTR *array, char control_char)
{
	Object *obj = KeyCycle(array, control_char);

	if(obj)
		set(obj,MUIA_CycleChain,1);
	return(obj);
}

///
/// MakeKeyCheckMark
Object *MakeKeyCheckMark(BOOL selected, char control_char)
{
	Object *obj = KeyCheckMark(selected, control_char);

	if(obj)
		set(obj,MUIA_CycleChain,1);
	return(obj);
}

///
/// MakePopAsl
Object *MakePopAsl(Object *string, STRPTR title, BOOL drawers_only)
{
	Object *obj = PopaslObject,
		MUIA_Popstring_String, string,
		MUIA_Popstring_Button, PopButton((drawers_only ? MUII_PopDrawer : MUII_PopFile)),
		MUIA_Popasl_Type     , ASL_FileRequest,
		ASLFR_TitleText      , title,
		ASLFR_DrawersOnly    , drawers_only,
	End;

	if(obj)
		set(obj,MUIA_CycleChain,1);
	return(obj);
}

///
/// desfunc
SAVEDS ASM VOID desfunc(REG(a2) APTR pool, REG(a1) APTR *entry)
{
	if(entry)
		FreeVec(entry);
}

///
/// sortfunc
SAVEDS ASM LONG sortfunc(REG(a1) STRPTR str1, REG(a2) STRPTR str2)
{
	return(stricmp(str1, str2));
}

///
/// strobjfunc
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

///
/// GetStr
STRPTR GetStr(STRPTR idstr)
{
	STRPTR local;

	local = idstr + 2;

	if(LocaleBase)
		return((STRPTR)GetCatalogStr(cat, *(UWORD *)idstr, local));

	return(local);
}

///
/// SetEnvDOS
BOOL SetEnvDOS(STRPTR name, STRPTR string, LONG len, BOOL save)
{
	char  file[MAXPATHLEN];
	BPTR  fh;
	BOOL  success = FALSE;

	strcpy(file, (save ? "EnvArc:": "Env:"));
	AddPart(file, name, MAXPATHLEN);
	if(fh = Open(file, MODE_NEWFILE))
	{
		if(len == -1)
			len = strlen(string);
		if(Write(fh, string, len) == len)
			success = TRUE;
		Close(fh);
	}
	return(success);
}

///
/// GetEnvDOS
LONG GetEnvDOS(STRPTR name, STRPTR buffer, LONG max_len)
{
	char  file[MAXPATHLEN];
	LONG  size;
	BPTR  fh;

	*buffer = NULL;
	strcpy(file, "Env:");
	AddPart(file, name, MAXPATHLEN);
	if(fh = Open(file, MODE_OLDFILE))
	{
		size = Read(fh, buffer, max_len);
		Close(fh);
		if(size >= 0)
			buffer[size] = NULL;
	}
	return((LONG)strlen(buffer));
}

///
/// extract_arg
STRPTR extract_arg(STRPTR string, STRPTR buffer, LONG len, char sep)
{
	STRPTR ptr1, ptr2;

	strncpy(buffer, string, len);

	ptr1 = strchr(buffer, (sep ? sep : ' '));
	ptr2 = strchr(buffer, 9);

	if(ptr2 && ((ptr2 < ptr1) || !ptr1))
		ptr1 = ptr2;
	if(ptr1)
		*ptr1 = NULL;

	string += strlen(buffer);

	while(*string == ' ' || *string == 9 || (sep ? *string == sep : NULL))
		string++;

	return((*string ? string : NULL));
}
///
/// get_file_size
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

///
/// ParseConfig
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

					pc_data->Buffer   = buf;
					pc_data->Size     = size;
					pc_data->Current  = buf;

					pc_data->Argument = NULL;
					pc_data->Contents = NULL;
				}

				Close(fh);
			}
		}
	}

	return(success);
}

///
/// ParseNext
BOOL ParseNext(struct pc_Data *pc_data)
{
	BOOL success = FALSE;
	STRPTR ptr_eol, ptr_tmp;

	if(pc_data->Current && pc_data->Current < pc_data->Buffer + pc_data->Size)
	{
		if(ptr_eol = strchr(pc_data->Current, '\n'))
		{
			*ptr_eol = NULL;

			if(pc_data->Contents = strchr(pc_data->Current, 34))              /* is the content between ""'s ? */
			{
				pc_data->Contents++;
				if(ptr_tmp = strchr(pc_data->Contents, 34))  /* find the ending '"' */
					*ptr_tmp = NULL;

				ptr_tmp = pc_data->Contents - 2;
				while(((*ptr_tmp == ' ') || (*ptr_tmp == 9)) && ptr_tmp >= pc_data->Current)
					ptr_tmp--;

				ptr_tmp++;
				*ptr_tmp = NULL;
			}
			else
			{
				pc_data->Contents = strchr(pc_data->Current, ' ');                   /* a space  */
				ptr_tmp           = strchr(pc_data->Current, 9);                     /* or a TAB */

				if((ptr_tmp < pc_data->Contents && ptr_tmp) || !pc_data->Contents)   /* which one comes first ? */
					pc_data->Contents = ptr_tmp;
				if(pc_data->Contents)
				{
					*pc_data->Contents++ = NULL;
					while((*pc_data->Contents == ' ') || (*pc_data->Contents == 9))
						pc_data->Contents++;

					ptr_tmp = NULL;
					if(!ptr_tmp)
					{
						if(ptr_tmp = strchr(pc_data->Contents, ';')) /* cut out the comment */
							*ptr_tmp = NULL;
						if(ptr_tmp = strchr(pc_data->Contents, ' ')) /* is there a space left ? */
							*ptr_tmp = NULL;
						if(ptr_tmp = strchr(pc_data->Contents, 9))   /* or a TAB ? */
							*ptr_tmp = NULL;
					}
				}
				else
					pc_data->Contents = "";
			}

			pc_data->Argument = pc_data->Current;
			pc_data->Current  = ptr_eol + 1;
			success = TRUE;
		}
		else
			pc_data->Current = NULL;
	}
	return(success);
}

///
/// ParseNextLine
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
			pc_data->Current  = ptr_eol + 1;
			success = TRUE;
		}
		else
			pc_data->Current = NULL;
	}

	return(success);
}

///
/// ParseEnd
VOID ParseEnd(struct pc_Data *pc_data)
{
	if(pc_data->Buffer)
		FreeVec(pc_data->Buffer);

	pc_data->Buffer   = NULL;
	pc_data->Size     = NULL;
	pc_data->Current  = NULL;

	pc_data->Argument = NULL;
	pc_data->Contents = NULL;
}

///
/// EscapeString
VOID EscapeString(STRPTR buffer, STRPTR str)
{
/* with backlash '\' :
	B   - backspace
	T   - horizontal tab
	N   - newline
	V   - vertical tab
	F   - formfeed
	R   - return
	E   - escape (ASCII 27 decimal)
	Xnn - character represented by hex value nn.
*/
	ULONG len;
	UWORD offs;
	STRPTR help;

	offs  = 0;
	len   = 80;
	help  = str;

	while(*help && len)
	{
		if(*help != '\\')
		{
			buffer[offs++] = *help++;
		}
		else
		{
			help++;     /* auf cmd-char stellen.. */
			switch(*help++)
			{
				case 'b':
				case 'B':
					buffer[offs++] = '\b';
					break;
				case 't':
				case 'T':
					buffer[offs++] = '\t';
					break;
				case 'n':
				case 'N':
					buffer[offs++] = '\n';
					break;
				case 'v':
				case 'V':
					buffer[offs++] = '\v';
					break;
				case 'f':
				case 'F':
					buffer[offs++] = '\f';
					break;
				case 'r':
				case 'R':
					buffer[offs++] = '\r';
					break;
				case 'e':
				case 'E':
					buffer[offs++] = 27;
					break;
				case 'x':
				case 'X':
				{
					UWORD i;
					UBYTE cnt = 0;
					UBYTE c;

					for(i=0; i<2; i++)
					{
						c = *help;
						cnt <<= 4;     /* mit 16 multiplizieren */

						if(c >= '0' && c <= '9')
							cnt += c - '0';
						else
						{
							if(c >= 'A' && c <= 'F')
								cnt += 9 + (c - ('A'-1));
							else
							{
								if(c >= 'a' && c <= 'f')
									cnt += 9 + (c - ('a'-1));
							}
						}
						help++;
					}
					buffer[offs++] = cnt;
				}
					break;
				case '\\':
					buffer[offs++] = '\\';
					break;
			}
		}
		len--;
	}
	buffer[offs] = '\0';
}

///
/// StopSerialRead
VOID StopSerialRead(VOID)
{
	if(ReadSER && ReadQueued)
	{
		if(!(CheckIO(ReadSER)))
			AbortIO(ReadSER);

		WaitIO(ReadSER);
		ReadQueued = FALSE;
	}
}

///
/// StartSerialRead
VOID __regargs StartSerialRead(register APTR Data, register ULONG Length)
{
	if(ReadSER)
	{
		if(ReadQueued)
			StopSerialRead();

		ReadSER->IOSer.io_Command  = CMD_READ;
		ReadSER->IOSer.io_Length   = Length;
		ReadSER->IOSer.io_Data     = Data;

		SetSignal(0, 1L << ReadPortSER->mp_SigBit);

		SendIO(ReadSER);

		ReadQueued = TRUE;
	}
}

///
/// FlushSerialRead
VOID FlushSerialRead(VOID)
{
	if(ReadSER)
	{
		BOOL WasRunning;

		if(WasRunning = ReadQueued)
			StopSerialRead();

		ReadSER->IOSer.io_Command = CMD_CLEAR;
		DoIO(ReadSER);

		if(WasRunning)
			StartSerialRead(serial_in, 1);
	}
}

///
/// close_serial
VOID close_serial(VOID)
{
	if(ReadSER && ReadSER->IOSer.io_Device)
	{
		if(!(CheckIO(ReadSER)))
		{
			AbortIO(ReadSER);
			WaitIO(ReadSER);
		}
		CloseDevice(ReadSER);
		ReadSER->IOSer.io_Device = NULL;
	}
	if(WriteSER)         DeleteIORequest(WriteSER);
	if(ReadSER)          DeleteIORequest(ReadSER);
	if(WritePortSER)     DeleteMsgPort(WritePortSER);
	if(ReadPortSER)      DeleteMsgPort(ReadPortSER);

	WriteSER = ReadSER  = NULL;
	WritePortSER = ReadPortSER = NULL;
	ReadQueued = FALSE;
}

///
/// open_serial
BOOL open_serial(STRPTR device_name, LONG unit)
{
	WritePortSER   = CreateMsgPort();
	ReadPortSER    = CreateMsgPort();

	if(WritePortSER && ReadPortSER)
	{
		WriteSER = CreateIORequest(WritePortSER,  sizeof(struct IOExtSer));
		ReadSER  = CreateIORequest(ReadPortSER,   sizeof(struct IOExtSer));

		if(WriteSER && ReadSER)
		{
			ReadSER->io_SerFlags = SERF_7WIRE | SERF_RAD_BOOGIE | SERF_XDISABLED;
			if(!(OpenDevice(device_name, unit, ReadSER, NULL)))
			{
				ReadSER->io_SerFlags = SERF_7WIRE | SERF_RAD_BOOGIE | SERF_XDISABLED;
				ReadSER->io_Baud     = 19200;
				ReadSER->io_RBufLen  = 1024;
				ReadSER->io_WriteLen = 8;
				ReadSER->io_ReadLen  = 8;
				ReadSER->io_ExtFlags = NULL;
				ReadSER->IOSer.io_Command  = SDCMD_SETPARAMS;
				DoIO(ReadSER);

				memcpy(WriteSER, ReadSER, sizeof(struct IOExtSer));
				WriteSER->IOSer.io_Message.mn_ReplyPort = WritePortSER;
				StartSerialRead(serial_in, 1);
				return(TRUE);
			}
		}
	}
	close_serial();
	return(FALSE);
}

///
/// send_serial
VOID send_serial(STRPTR cmd)
{
	WriteSER->IOSer.io_Length  = -1;
	WriteSER->IOSer.io_Command = CMD_WRITE;
	WriteSER->IOSer.io_Data    = cmd;
	DoIO(WriteSER);
}

///
/// serial_carrier
BOOL serial_carrier(VOID)
{
	ULONG CD = 1<<5;

	if(!WriteSER)
		return(FALSE);
	WriteSER->IOSer.io_Command = SDCMD_QUERY;
	DoIO(WriteSER);
	return((BOOL)(CD & WriteSER->io_Status ? FALSE : TRUE));
}

///

