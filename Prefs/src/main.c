#include "globals.c"
#include "protos.h"

extern VOID exit_classes(VOID);
extern BOOL init_classes(VOID);


/*
 * close the libraries
 */

VOID exit_libs(VOID)
{
	if(cat)					CloseCatalog(cat);

	if(DataTypesBase)		CloseLibrary(DataTypesBase);
	if(IFFParseBase)		CloseLibrary(IFFParseBase);
	if(IntuitionBase)		CloseLibrary(IntuitionBase);
	if(UtilityBase)		CloseLibrary(UtilityBase);
	if(MUIMasterBase)		CloseLibrary(MUIMasterBase);
	if(LocaleBase)			CloseLibrary(LocaleBase);
	if(DOSBase)				CloseLibrary(DOSBase);

	cat			= NULL;
	IFFParseBase	= IntuitionBase	= UtilityBase		=
	MUIMasterBase	= LocaleBase		= DataTypesBase	=
	DOSBase			= NULL;
}


/*
 * open all needed libraries
 */

BOOL init_libs(VOID)
{
	DOSBase			= (struct DosLibrary *)OpenLibrary("dos.library", 0);
	IntuitionBase	= OpenLibrary("intuition.library"	, 0);

	if(LocaleBase	= OpenLibrary("locale.library", 38))
		cat = OpenCatalog(NULL, "AmiTCPPrefs.catalog", OC_BuiltInLanguage, "english", TAG_DONE);

	MUIMasterBase	= OpenLibrary("muimaster.library"	, 11);
	UtilityBase		= OpenLibrary("utility.library"		, 0);
	IFFParseBase	= OpenLibrary("iffparse.library"		, 0);
	DataTypesBase	= OpenLibrary("datatypes.library"	, 0);

	if(DOSBase && MUIMasterBase && UtilityBase && IntuitionBase && IFFParseBase && DataTypesBase)
		return(TRUE);
	/* the program will even work without locale.library */

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


#ifdef DEMO
#include <resources/battclock.h>
#include <clib/battclock_protos.h>
BOOL check_date(VOID)
{
	if(BattClockBase = OpenResource("battclock.resource"))
	{
		if(ReadBattClock() > 594345603)
			return(FALSE);
	}
	return(TRUE);
}
#endif

VOID __saveds Handler(VOID)
{
	ULONG sigs = NULL;

	app = ApplicationObject,
		MUIA_Application_Author			, "Michael Neuweiler",
		MUIA_Application_Base			, "AmiTCPPrefs",
		MUIA_Application_Title			, "AmiTCP Prefs",
		MUIA_Application_Version		, VERSTAG,
		MUIA_Application_Copyright		, GetStr(MSG_AppCopyright),
		MUIA_Application_Description	, GetStr(MSG_AppDescription),
		MUIA_Application_HelpFile		, "PROGDIR:Docs/AmiTCPPrefs.guide",
		MUIA_Application_Window			, WindowObject,
			WindowContents		, group = VGroup,
				Child, HVSpace,
			End,
		End,
	End;

	if(app)
	{
		if(win = NewObject(CL_AmiTCPPrefs->mcc_Class, NULL, TAG_DONE))
		{
			DoMethod(app, OM_ADDMEMBER, win);
			if(DoMethod(win, MUIM_AmiTCPPrefs_InitGroups, NULL))
			{
				set(win, MUIA_Window_Open, TRUE);
				DoMethod(win, MUIM_Provider_PopList_Update, "NetConnect:Data/Providers", MUIV_Provider_PopString_Country);
				DoMethod(win, MUIM_AmiTCPPrefs_LoadConfig, NULL);
				DoMethod(win, MUIM_InfoWindow_LoadFile);

#ifdef DEMO
				DoMethod(win, MUIM_AmiTCPPrefs_About);
				if(!check_date())
					MUI_Request(app, win, 0, 0, "*_Snif..", "Sorry, program has become invalid !");
				else
#endif
				while(DoMethod(app, MUIM_Application_NewInput, &sigs) != MUIV_Application_ReturnID_Quit)
				{
					if(sigs)
					{
						sigs = Wait(sigs | SIGBREAKF_CTRL_C);
						if(sigs & SIGBREAKF_CTRL_C)
							break;
					}
				}
				set(win, MUIA_Window_Open, FALSE);
			}
		}
		MUI_DisposeObject(app);
		app = NULL;
	}
}

/*
 * program entry point
 */

LONG main(VOID)
{
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
			WBenchLock = CurrentDir(WBenchMsg->sm_ArgList->wa_Lock);

		if(init_classes())
		{
			LocalizeNewMenu(AmiTCPPrefsMenu);

			if(StackSize(NULL) < 16384)
			{
				LONG success;
				StackCall(&success,16384,0,(LONG (* __stdargs)())Handler);
			}
			else
				Handler();

			exit_classes();
		}
		if(WBenchMsg)
			CurrentDir(WBenchLock);
		exit_libs();
	}
	if(WBenchMsg)
	{
		Forbid();
		ReplyMsg((struct Message *)WBenchMsg);
	}

	return(RETURN_OK);
}
