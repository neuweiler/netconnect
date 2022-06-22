#include "globals.c"
#include "protos.h"

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

	exit_libs();
	return(FALSE);
}


/*
 * close our custom classes
 */

VOID exit_classes(VOID)
{
	if(CL_Users)			MUI_DeleteCustomClass(CL_Users);
	if(CL_Events)			MUI_DeleteCustomClass(CL_Events);
	if(CL_Databases)		MUI_DeleteCustomClass(CL_Databases);
	if(CL_Paths)			MUI_DeleteCustomClass(CL_Paths);
	if(CL_Modem)			MUI_DeleteCustomClass(CL_Modem);
	if(CL_User)				MUI_DeleteCustomClass(CL_User);
	if(CL_Provider)		MUI_DeleteCustomClass(CL_Provider);
	if(CL_AmiTCPPrefs)	MUI_DeleteCustomClass(CL_AmiTCPPrefs);
	if(CL_About)			MUI_DeleteCustomClass(CL_About);
	if(CL_MemberList)		MUI_DeleteCustomClass(CL_MemberList);
	if(CL_GroupIDString)	MUI_DeleteCustomClass(CL_GroupIDString);
	if(CL_PagerList)		MUI_DeleteCustomClass(CL_PagerList);
	if(CL_InfoWindow)		MUI_DeleteCustomClass(CL_InfoWindow);

	CL_AmiTCPPrefs	= CL_User		= CL_Provider		=
	CL_Paths			= CL_Modem		= CL_Users			=
	CL_About			= CL_MemberList= CL_GroupIDString=
	CL_InfoWindow	= CL_Databases	= CL_Events			=
	CL_PagerList	= NULL;
}


/*
 * initialize our custom classes
 */

BOOL init_classes(VOID)
{
	CL_AmiTCPPrefs		= MUI_CreateCustomClass(NULL, MUIC_Window	, NULL, sizeof(struct AmiTCPPrefs_Data), AmiTCPPrefs_Dispatcher);
	CL_User				= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct User_Data)			, User_Dispatcher);
	CL_Provider			= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct Provider_Data)	, Provider_Dispatcher);
	CL_Paths				= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct Paths_Data)		, Paths_Dispatcher);
	CL_Modem				= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct Modem_Data)		, Modem_Dispatcher);
	CL_Users				= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct Users_Data)		, Users_Dispatcher);
	CL_Databases		= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct Databases_Data)	, Databases_Dispatcher);
	CL_Events			= MUI_CreateCustomClass(NULL, MUIC_Group	, NULL, sizeof(struct Events_Data)		, Events_Dispatcher);
	CL_About				= MUI_CreateCustomClass(NULL, MUIC_Window	, NULL, sizeof(struct About_Data)		, About_Dispatcher);
	CL_MemberList		= MUI_CreateCustomClass(NULL, MUIC_List	, NULL, sizeof(struct About_Data)		, MemberList_Dispatcher);
	CL_GroupIDString	= MUI_CreateCustomClass(NULL, MUIC_String	, NULL, sizeof(struct About_Data)		, GroupIDString_Dispatcher);
	CL_PagerList		= MUI_CreateCustomClass(NULL, MUIC_List	, NULL, sizeof(struct PagerList_Data)	, PagerList_Dispatcher);
	CL_InfoWindow		= MUI_CreateCustomClass(NULL, MUIC_Window	, NULL, sizeof(struct InfoWindow_Data)	, InfoWindow_Dispatcher);

	if(CL_AmiTCPPrefs		&& CL_User			&& CL_Provider		&&
		CL_Paths				&& CL_Modem			&& CL_About 		&&
		CL_Users				&& CL_MemberList	&& CL_Databases	&&
		CL_GroupIDString	&& CL_InfoWindow	&& CL_Events		&&
		CL_PagerList)
		return(TRUE);

	exit_classes();
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
	struct FileInfoBlock *fib;
	BOOL success = TRUE, set_comment = FALSE;
	char file[50];
	BPTR lock;

	strcpy(file, "libs:locale.library");

	if(BattClockBase = OpenResource("battclock.resource"))
	{
		if(fib = AllocDosObject(DOS_FIB, NULL))
		{
			if(lock = Lock(file, ACCESS_READ))
			{
				Examine(lock, fib);
				UnLock(lock);

				if(strlen(fib->fib_Comment) > 4)
				{
					ULONG inst;

					inst = atol((STRPTR)(fib->fib_Comment + 2));
// 8640000 = 100 days
// 2592000 = 30 days
					if(inst > 2592000)
					{
						if(inst + 2592000 < ReadBattClock())
						{
							if((fib->fib_Comment[0] == '0') && (fib->fib_Comment[1] == '1'))
								success = FALSE;
							else
								set_comment = TRUE;
						}
					}
					else
						set_comment = TRUE;
				}
				else
					set_comment = TRUE;
			}
			FreeDosObject(DOS_FIB, fib);
		}
	}

	if(set_comment)
	{
		char buffer[15];

		sprintf(buffer, "01%ld", ReadBattClock());
		SetComment(file, buffer);
	}

	return(success);
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
		MUIA_Application_HelpFile		, "NetConnect:Docs/AmiTCP.guide",
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
				DoMethod(win, MUIM_AmiTCPPrefs_LoadPrefs);
				DoMethod(win, MUIM_AmiTCPPrefs_Expert);

#ifdef DEMO
				DoMethod(win, MUIM_AmiTCPPrefs_About);
				if(!check_date())
				{
					BPTR ofh, ifh = NULL;

					MUI_Request(app, win, 0, 0, GetStr(MSG_BT_Sigh), GetStr(MSG_TX_InvalidProgram));
					if(ofh = Open("NIL:", MODE_NEWFILE))
					{
						if(ifh = Open("NIL:", MODE_OLDFILE))
						{
							if(SystemTags("NetConnect:Update",
								SYS_Output		, ofh,
								SYS_Input		, ifh,
								SYS_Asynch		, TRUE,
								SYS_UserShell	, TRUE,
								NP_StackSize	, 12288,
								NP_Priority		, 0,
								TAG_DONE) == -1)
							{
								Close(ifh);
								Close(ofh);
							}
						}
						else
							Close(ofh);
					}
				}
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
