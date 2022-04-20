#include "globals.c"
#include "protos.h"

extern BOOL init_classes(VOID);
extern VOID exit_classes(VOID);

/*
 * close the libraries
 */

VOID exit_libs(VOID)
{
	if(cat)					CloseCatalog(cat);
	if(SoundObject)		DisposeDTObject(SoundObject);

	if(DataTypesBase)		CloseLibrary(DataTypesBase);
	if(IFFParseBase)		CloseLibrary(IFFParseBase);
	if(IntuitionBase)		CloseLibrary(IntuitionBase);
	if(UtilityBase)		CloseLibrary(UtilityBase);
	if(MUIMasterBase)		CloseLibrary(MUIMasterBase);
	if(LocaleBase)			CloseLibrary(LocaleBase);
	if(CxBase)				CloseLibrary(CxBase);
	if(WorkbenchBase)		CloseLibrary(WorkbenchBase);
	if(IconBase)			CloseLibrary(IconBase);
	if(IntuitionBase)		CloseLibrary(IntuitionBase);
	if(DOSBase)				CloseLibrary(DOSBase);

	cat			= NULL;
	SoundObject	= NULL;
	IFFParseBase	= IntuitionBase	= UtilityBase		=
	MUIMasterBase	= LocaleBase		= DataTypesBase	=
	CxBase			= WorkbenchBase	= IconBase			=
	DOSBase			= NULL;
}


/*
 * open all needed libraries
 */

BOOL init_libs(VOID)
{
	DOSBase			= (struct DosLibrary *)OpenLibrary("dos.library"			, 0);
	IntuitionBase	= OpenLibrary("intuition.library"	, 0);

	if(LocaleBase	= OpenLibrary("locale.library", 38))
		cat = OpenCatalog(NULL, "netconnect.catalog", OC_BuiltInLanguage, "english", TAG_DONE);

	MUIMasterBase	= OpenLibrary("muimaster.library"	, 11);
	UtilityBase		= OpenLibrary("utility.library"		, 0);
	IFFParseBase	= OpenLibrary("iffparse.library"		, 0);
	DataTypesBase	= OpenLibrary("datatypes.library"	, 0);
	CxBase			= OpenLibrary("commodities.library"	, 0);
	WorkbenchBase	= OpenLibrary(WORKBENCH_NAME			, 0);
	IconBase			= OpenLibrary("icon.library"			, 0);

	if(MUIMasterBase && UtilityBase && IntuitionBase && IFFParseBase && DOSBase && WorkbenchBase && DataTypesBase && IconBase)
		return(TRUE);
	/* the program will still work without locale.library or commodity.library*/

	exit_libs();
	return(FALSE);
}


/*
 * extract the correct string from a locale made by "cat2h"
 */

STRPTR GetStr(STRPTR idstr)
{
	STRPTR local;

	local = idstr + 2;

	if(LocaleBase)
		return((STRPTR)GetCatalogStr(cat, *(UWORD *)idstr, local));

	return(local);
}

VOID LocalizeNewMenu(struct NewMenu *nm)
{
	for (;nm->nm_Type!=NM_END;nm++)
		if (nm->nm_Label != NM_BARLABEL)
			nm->nm_Label = GetStr(nm->nm_Label);
}

/*
 * program entry point
 */

BOOL BuildApp(VOID)
{
	BOOL success = FALSE;

	if(app = ApplicationObject,
		MUIA_Application_Author			, "Michael Neuweiler",
		MUIA_Application_Base			, "NetConnect",
		MUIA_Application_Title			, "NetConnect Controller",
		MUIA_Application_Version		, "$VER: NetConnect 0.9 (01.08.96)",
		MUIA_Application_Copyright		, GetStr(MSG_AppCopyright),
		MUIA_Application_Description	, GetStr(MSG_AppDescription),
		MUIA_Application_BrokerHook	, &BrokerHook,
		MUIA_Application_Window			, win = NewObject(CL_IconBar->mcc_Class, NULL, TAG_DONE),
		End)
	{
		DoMethod(win, MUIM_IconBar_LoadButtons);
		set(win, MUIA_Window_Open, TRUE);

		if(menu_win = (Object *)NewObject(CL_MenuPrefs->mcc_Class, NULL, TAG_DONE))
		{
			DoMethod(app, OM_ADDMEMBER, menu_win);

			ihnode.ihn_Object		= menu_win;
			ihnode.ihn_Signals	= (LONG)(1L << appmenu_port->mp_SigBit);
			ihnode.ihn_Method		= MUIM_MenuPrefs_TriggerMenu;
			ihnode.ihn_Flags		= 0;
			DoMethod(app, MUIM_Application_AddInputHandler, &ihnode);

			DoMethod(menu_win, MUIM_MenuPrefs_LoadMenus);

			success = TRUE;
		}
	}
	return(success);
}

VOID rem_inputhandler(VOID)
{
	struct Message *message;

	DoMethod(app, MUIM_Application_RemInputHandler, &ihnode);
	while(message = GetMsg(appmenu_port))
		ReplyMsg(message);
}

int main(int argc, char *argv[])
{
	ULONG sigs = NULL;
	BOOL running;

	ThisProcess = (struct Process *)FindTask(NULL);

	if(!ThisProcess->pr_CLI)
	{
		WaitPort(&ThisProcess->pr_MsgPort);

		WBenchMsg = (struct WBStartup *)GetMsg(&ThisProcess->pr_MsgPort);
	}
	else
		WBenchMsg = NULL;


	if(init_libs())
	{
		if(!ThisProcess -> pr_CLI)
		{
			if(LocalCLI = CloneCLI(&WBenchMsg -> sm_Message))
			{
				OldCLI = ThisProcess -> pr_CLI;
				ThisProcess -> pr_CLI = MKBADDR(LocalCLI);
			}
			WBenchLock = CurrentDir(WBenchMsg->sm_ArgList->wa_Lock);
		}

		if(init_classes())
		{
			if(appmenu_port = CreateMsgPort())
			{
				LocalizeNewMenu(IconBarMenu);
				LocalizeNewMenu(IconBarPrefsMenu);

				if(BuildApp())
				{
if(MUI_Request(app, win, 0, 0, "_Yes, I am !|*\33bGosh _no, I'm not !", "\33cThis version of NetConnect Controller is a\n trial version and might only be used\nby NSDI, Active Software and those who\ngot direct permission from either NSDI\nor Active Software !\n\nCopyright © 1996 by Michael Neuweiler\n\n\33bAre you allowed to use this program ?"))
{
					running = TRUE;
					while(running)
					{
						switch(DoMethod(app, MUIM_Application_NewInput, &sigs))
						{
							case MUIV_Application_ReturnID_Quit:
								running = FALSE;
								break;
							case ID_REBUILD:
								set(win, MUIA_Window_Open, FALSE);
								rem_inputhandler();
								MUI_DisposeObject(app);
								if(!BuildApp())
									running = FALSE;
								break;
						}
						if(sigs)
						{
							sigs = Wait(sigs | SIGBREAKF_CTRL_C);
							if(sigs & SIGBREAKF_CTRL_C)
								break;
						}
					}
}
					set(win, MUIA_Window_Open, FALSE);
					rem_inputhandler();
					MUI_DisposeObject(app);
					app = NULL;
				}
				DeleteMsgPort(appmenu_port);
			}
			exit_classes();
		}
		if(WBenchMsg)
			CurrentDir(WBenchLock);
		if(LocalCLI)
		{
			ThisProcess->pr_CLI = OldCLI;
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
