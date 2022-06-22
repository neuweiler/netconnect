#include "globals.c"

/// exit_libs
VOID exit_libs(VOID)
{
	if(cat)              CloseCatalog(cat);

	if(UtilityBase)      CloseLibrary(UtilityBase);
	if(MUIMasterBase)    CloseLibrary(MUIMasterBase);
	if(LocaleBase)       CloseLibrary(LocaleBase);
	if(IntuitionBase)    CloseLibrary(IntuitionBase);
	if(DOSBase)          CloseLibrary(DOSBase);

	cat            = NULL;
	IntuitionBase  = UtilityBase =
	MUIMasterBase  = LocaleBase  =
	DOSBase        = NULL;
}

///
/// init_libs
BOOL init_libs(VOID)
{
	DOSBase        = (struct DosLibrary *)OpenLibrary("dos.library", 0);
	IntuitionBase  = OpenLibrary("intuition.library"   , 0);

	if(LocaleBase  = OpenLibrary("locale.library", 38))
		cat = OpenCatalog(NULL, "SetupAmiTCP.catalog", OC_BuiltInLanguage, "english", TAG_DONE);

	MUIMasterBase  = OpenLibrary("muimaster.library"   , 11);
	UtilityBase    = OpenLibrary("utility.library"     , 36);

	if(DOSBase && IntuitionBase && MUIMasterBase && UtilityBase)
		return(TRUE);

	exit_libs();
	return(FALSE);
}

///
/// exit_classes
VOID exit_classes(VOID)
{
	if(CL_MainWindow)          MUI_DeleteCustomClass(CL_MainWindow);
	if(CL_InfoWindow)          MUI_DeleteCustomClass(CL_InfoWindow);
	if(CL_InfoText)            MUI_DeleteCustomClass(CL_InfoText);

	CL_MainWindow = CL_InfoWindow = CL_InfoText = NULL;
}

///
/// init_classes
BOOL init_classes(VOID)
{
	CL_MainWindow     = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct MainWindow_Data)    , MainWindow_Dispatcher);
	CL_InfoWindow     = MUI_CreateCustomClass(NULL, MUIC_Window , NULL, sizeof(struct InfoWindow_Data)    , InfoWindow_Dispatcher);
	CL_InfoText       = MUI_CreateCustomClass(NULL, MUIC_Text   , NULL, sizeof(struct InfoText_Data)      , InfoText_Dispatcher);

	if(CL_MainWindow && CL_InfoWindow && CL_InfoText)
		return(TRUE);

	exit_classes();
	return(FALSE);
}

///
/// check_date
#ifdef DEMO
#include <resources/battclock.h>
#include <clib/battclock_protos.h>
BOOL check_date(VOID)
{
	if(BattClockBase = OpenResource("battclock.resource"))
	{
		if(ReadBattClock() < 613044696)
			return(TRUE);
	}
	return(FALSE);
}
#endif

///
/// LocalizeNewMenu
VOID LocalizeNewMenu(struct NewMenu *nm)
{
	for(; nm && nm->nm_Type!=NM_END; nm++)
	{
		if(nm->nm_Label != NM_BARLABEL)
			nm->nm_Label = GetStr(nm->nm_Label);
		if(nm->nm_CommKey)
			nm->nm_CommKey = GetStr(nm->nm_CommKey);
	}
}

///
/// Handler
VOID __saveds Handler(VOID)
{
	if(init_classes())
	{
		LocalizeNewMenu(MainMenu);
		if(app = ApplicationObject,
			MUIA_Application_Author       , "Michael Neuweiler",
			MUIA_Application_Base         , "SetupAmiTCP",
			MUIA_Application_Title        , "Setup AmiTCP",
			MUIA_Application_Version      , VERSTAG,
			MUIA_Application_Copyright    , "Michael Neuweiler 1997",
			MUIA_Application_Description  , "Tool to setup AmiTCP for the first time",
			MUIA_Application_HelpFile     , "HELP:SetupAmiTCP.guide",
			MUIA_Application_Window       , WindowObject,
				WindowContents, group = VGroup,
					Child, HVSpace,
				End,
			End,
		End)
		{
			if(win = NewObject(CL_MainWindow->mcc_Class, NULL, TAG_DONE))
			{
				struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);
				struct pc_Data pc_data;
				STRPTR term_line;

				DoMethod(app, OM_ADDMEMBER, win);
				set(win, MUIA_Window_Open, TRUE);
				if(!xget(win, MUIA_Window_Open))    // check if there was enogh space to open window
				{
					set(data->GR_Picture, MUIA_ShowMe, FALSE);
					set(win, MUIA_Window_Open, TRUE);
				}

				set(app, MUIA_Application_Sleep, TRUE);

				/** load devices into serial.dev list **/

				set(data->LV_SerialDevices, MUIA_List_Quiet, TRUE);
				{
					BPTR lock;
					struct FileInfoBlock *fib;

					if(lock = Lock("DEVS:", ACCESS_READ))
					{
						if(fib = AllocDosObject(DOS_FIB, NULL))
						{
							if(Examine(lock, fib))
							{
								while(ExNext(lock, fib))
								{
									if((fib->fib_DirEntryType < 0) && strstr(fib->fib_FileName, ".device"))
										DoMethod(data->LV_SerialDevices, MUIM_List_InsertSingle, fib->fib_FileName, MUIV_List_Insert_Sorted);
								}
							}
							FreeDosObject(DOS_FIB, fib);
						}
						UnLock(lock);
					}
				}
				set(data->LV_SerialDevices, MUIA_List_Quiet, FALSE);

				/**** load the ModemSettings into the List ****/

				if(ParseConfig("NetConnect:Data/Misc/ModemSettings", &pc_data))
				{
					struct Modem *modem;

					if(modem = (struct Modem *)AllocVec(sizeof(struct Modem), MEMF_ANY))
					{
						set(data->LV_Modems, MUIA_List_Quiet, TRUE);
						while(ParseNext(&pc_data))
						{
							strncpy(modem->Name, pc_data.Argument, 80);
							strncpy(modem->InitString, pc_data.Contents, 80);
							DoMethod(data->LV_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Sorted);
						}
						FreeVec(modem);
						set(data->LV_Modems, MUIA_List_Quiet, FALSE);
					}
					ParseEnd(&pc_data);
				}

				DoMethod(win, MUIM_MainWindow_CreateProviderList, "NetConnect:Data/Providers");

				set(app, MUIA_Application_Sleep, FALSE);

				DoMethod(win, MUIM_MainWindow_About);

				ser_buf_pos = 0;
				last_input = 1;
				serial_buffer_old1[0] = NULL;
				serial_buffer_old2[0] = NULL;
				checking_modem = 0;
#ifdef DEMO
				if(!check_date())
					MUI_Request(app, 0, 0, 0, "*_Sigh..", "Sorry, program has become invalid !");
				else
#endif
				if(term_line = AllocVec(81, MEMF_ANY))
				{
					DoMethod(data->LI_Terminal, MUIM_List_InsertSingle, term_line, MUIV_List_Insert_Bottom);
					FOREVER
					{
						while(DoMethod(app, MUIM_Application_NewInput, &sigs) != MUIV_Application_ReturnID_Quit)
						{
							if(sigs)
							{
								if(ReadSER)
									sigs = Wait(sigs | SIGBREAKF_CTRL_C | SIG_SER);
								else
									sigs = Wait(sigs | SIGBREAKF_CTRL_C);

								if(sigs & SIGBREAKF_CTRL_C)
									break;

								if(ReadSER)
								{
									if(sigs & SIG_SER)
									{
										if(CheckIO(ReadSER))
										{
											WaitIO(ReadSER);

											if(last_input == 0)
											{
												last_input = 1;
											}

											if(serial_in[0] == '\r' || serial_in[0] == '\n' || ser_buf_pos > 79 || serial_in[0] == 0)
											{
												if(serial_in[0] == '\n')
												{
													STRPTR ptr;

													if(ptr = AllocVec(81, MEMF_ANY))
													{
														term_line = ptr;
														term_line[0] = NULL;
														DoMethod(data->LI_Terminal, MUIM_List_InsertSingle, term_line, MUIV_List_Insert_Bottom);
														DoMethod(data->LI_Terminal, MUIM_List_Jump, MUIV_List_Jump_Bottom);
													}
												}
												if(serial_in[0] == '\r' && ser_buf_pos < 77)
													strcat(serial_buffer, "\\r");
												if(ser_buf_pos)
												{
//Printf("serial_buffer: '%ls'  (pos:%ld, len:%ld)   old1: '%ls'   old2: '%ls'\n", serial_buffer, ser_buf_pos, strlen(serial_buffer), serial_buffer_old1, serial_buffer_old2);
													strcpy(serial_buffer_old2, serial_buffer_old1);
													strcpy(serial_buffer_old1, serial_buffer);
													serial_buffer[0] = NULL;
													ser_buf_pos = 0;
												}
											}
											else
											{
												term_line[ser_buf_pos] = serial_buffer[ser_buf_pos] = serial_in[0];
												ser_buf_pos++;
												term_line[ser_buf_pos] = serial_buffer[ser_buf_pos] = NULL;
												DoMethod(data->LI_Terminal, MUIM_List_Redraw, xget(data->LI_Terminal, MUIA_List_Entries) - 1);
											}
											StartSerialRead(serial_in, 1);
										}
									}
								}
//                        if(sigs & SIG_CON)
//                        {
//                           last_input = 0;
//                        }
							}
						}
						if(xget(app, MUIA_Application_ForceQuit) || sigs & SIGBREAKF_CTRL_C)
							break;
						if(MUI_Request(app, win, NULL, NULL, "_Quit|*_Cancel", "Do you really want to quit ?"))
							break;
					}
				}
				set(win, MUIA_Window_Open, FALSE);
			}
			MUI_DisposeObject(app);
			app = NULL;

			close_serial();
		}
		exit_classes();
	}
}

///
/// main
LONG main(VOID)
{
	process = (struct Process *)FindTask(NULL);

	if(!process->pr_CLI)
	{
		WaitPort(&process->pr_MsgPort);

		WBenchMsg = (struct WBStartup *)GetMsg(&process->pr_MsgPort);
	}
	else
		WBenchMsg = NULL;

	if(init_libs())
	{
		if(!process->pr_CLI)
		{
			if(LocalCLI = CloneCLI(&WBenchMsg->sm_Message))
			{
				OldCLI = process->pr_CLI;
				process->pr_CLI = MKBADDR(LocalCLI);
			}
			WBenchLock = CurrentDir(WBenchMsg->sm_ArgList->wa_Lock);
		}

		if(StackSize(NULL) < 16384)
		{
			LONG success;
			StackCall(&success, 16384, 0, (LONG (* __stdargs)())Handler);
		}
		else
			Handler();

		if(WBenchMsg)
			CurrentDir(WBenchLock);
		if(LocalCLI)
		{
			process->pr_CLI = OldCLI;
			DeleteCLI(LocalCLI);
		}
		exit_libs();
	}

	if(WBenchMsg)
	{
		Forbid();
		ReplyMsg((struct Message *)WBenchMsg);
	}

	return(RETURN_OK);
}

///

