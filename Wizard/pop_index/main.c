#define MAXPATHLEN   256


struct DOSBase *DOSBase;

struct pc_Data
{
	STRPTR Buffer;    /* buffer holding the file (internal use only) */
	LONG Size;        /* variable holding the size of the buffer (internal use only) */
	STRPTR Current;   /* pointer to the current position (internal use only) */

	STRPTR Argument;  /* pointer to the argument */
	STRPTR Contents;  /* pointer to its contents */
};

/// exit_libs
VOID exit_libs(VOID)
{
//   if(UtilityBase)      CloseLibrary(UtilityBase);
	if(DOSBase)          CloseLibrary(DOSBase);

//   UtilityBase = DOSBase = NULL;
}

///
/// init_libs
BOOL init_libs(VOID)
{
	DOSBase        = (struct DosLibrary *)OpenLibrary("dos.library", 0);
//   UtilityBase    = OpenLibrary("utility.library"     , 36);

	if(DOSBase)
		return(TRUE);

	exit_libs();
	return(FALSE);
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
/// create_providerlist
VOID create_providerlist(BPTR fh, STRPTR path, int level)
{
	BPTR lock;
	struct FileInfoBlock *fib;
	STRPTR new_path;

	if(new_path = AllocVec(MAXPATHLEN, MEMF_ANY))
	{
		if(lock = Lock(path, ACCESS_READ))
		{
			if(fib = AllocDosObject(DOS_FIB, NULL))
			{
				if(Examine(lock, fib))
				{
					while(ExNext(lock, fib))
					{
						strcpy(new_path, path);
						AddPart(new_path, fib->fib_FileName, MAXPATHLEN);

						if(fib->fib_DirEntryType < 0)
						{
							struct pc_Data pc_data;

							FPrintf(fh, "*%ls\n", fib->fib_FileName);
							if(ParseConfig(new_path, &pc_data))
							{
								while(ParseNext(&pc_data))
								{
									if(*pc_data.Argument == '#' && !stricmp(pc_data.Contents, "POPList"))
									{
										while(ParseNext(&pc_data))
										{
											if(*pc_data.Argument == '#')
												break;
											if(*pc_data.Argument)
												FPrintf(fh, "  %ls \"%ls\"\n", pc_data.Argument, pc_data.Contents);
										}
									}
								}
								ParseEnd(&pc_data);
							}
							FPrintf(fh, "\n");
						}
						else
						{
							FPrintf(fh, "#%ls\n", fib->fib_FileName);
							create_providerlist(fh, new_path, level + 1);
						}
					}
				}
				FreeDosObject(DOS_FIB, fib);
			}
			UnLock(lock);
		}
		FreeVec(new_path);
	}
}

///
/// main
LONG main(VOID)
{
	BPTR fh;

	if(init_libs())
	{
		Printf("Creating output file: NetConnect:Data/Providers.idx\n");
		if(fh = Open("NetConnect:Data/Providers.idx", MODE_NEWFILE))
		{
			FPrintf(fh, "####\n# Provider index file, do not edit manually !\n###\n\n");
			Printf("Scanning provider directory for entries.\n");
			create_providerlist(fh, "NetConnect:Data/Providers", 0);
			Printf("\nfinished.\n\n");
			Close(fh);
		}
		exit_libs();
	}

	return(RETURN_OK);
}

///

