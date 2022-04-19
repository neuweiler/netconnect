#include "globals.c"

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

	cat			= NULL;
	SoundObject	= NULL;
	IFFParseBase	= IntuitionBase	= UtilityBase		=
	MUIMasterBase	= LocaleBase		= DataTypesBase	=
	CxBase			= NULL;
}


/*
 * open all needed libraries
 */

BOOL init_libs(VOID)
{
	if(LocaleBase	= OpenLibrary("locale.library", 38))
		cat = OpenCatalog(NULL, "netconnect.catalog", OC_BuiltInLanguage, "english", TAG_DONE);

	MUIMasterBase	= OpenLibrary("muimaster.library"	, 11);
	UtilityBase		= OpenLibrary("utility.library"		, 36);
	IntuitionBase	= OpenLibrary("intuition.library"	, 36);
	IFFParseBase	= OpenLibrary("iffparse.library"		, 0);
	DataTypesBase	= OpenLibrary("datatypes.library"	, 39);
	CxBase			= OpenLibrary("commodities.library"	, 37);

	if(MUIMasterBase && UtilityBase && IntuitionBase && IFFParseBase)
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

LONG main(VOID)
{
	ULONG sigs = NULL;

	if(init_libs())
	{
		if(init_classes())
		{
			LocalizeNewMenu(IconBarMenu);
			LocalizeNewMenu(IconBarPrefsMenu);

			app = ApplicationObject,
				MUIA_Application_Author			, "Michael Neuweiler",
				MUIA_Application_Base			, "NetConnect",
				MUIA_Application_Title			, "NetConnect Controller",
				MUIA_Application_Version		, "$VER: NetConnect 1.0 (01.06.96)",
				MUIA_Application_Copyright		, GetStr(MSG_AppCopyright),
				MUIA_Application_Description	, GetStr(MSG_AppDescription),
				MUIA_Application_BrokerHook	, &BrokerHook,
				MUIA_Application_Window			, win = NewObject(CL_IconBar->mcc_Class, NULL, TAG_DONE),
				End;

			if(app)
			{
				DoMethod(win, MUIM_IconBar_LoadButtons);
				set(win, MUIA_Window_Open, TRUE);
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
				MUI_DisposeObject(app);
				app = NULL;
			}
			exit_classes();
		}
		exit_libs();
	}
	else
		return(RETURN_FAIL);

	return(RETURN_OK);
}
