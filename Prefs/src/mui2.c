#include "globals.c"
#include "protos.h"

static struct Hook sorthook = { {NULL, NULL}, (VOID *)sortfunc, NULL, NULL};
static struct Hook strobjhook = { {NULL, NULL}, (VOID *)strobjfunc, NULL, NULL};
static struct Hook txtobjhook = { {NULL, NULL}, (VOID *)txtobjfunc, NULL, NULL};

/****************************************************************************/
/* About class                                                              */
/****************************************************************************/

ULONG About_New(struct IClass *cl, Object *obj, Msg msg)
{
	struct About_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title, VERS,
		MUIA_Window_ID   , MAKE_ID('A','B','O','U'),
		WindowContents, VGroup,
			MUIA_Background, MUII_RequesterBack,
			Child, HGroup,
				TextFrame,
				MUIA_Background, MUII_GroupBack,
				Child, HVSpace,
				Child, BodychunkObject,
					MUIA_FixWidth             , LOGO_WIDTH ,
					MUIA_FixHeight            , LOGO_HEIGHT,
					MUIA_Bitmap_Width         , LOGO_WIDTH ,
					MUIA_Bitmap_Height        , LOGO_HEIGHT,
					MUIA_Bodychunk_Depth      , LOGO_DEPTH ,
					MUIA_Bodychunk_Body       , (UBYTE *)logo_body,
					MUIA_Bodychunk_Compression, LOGO_COMPRESSION,
					MUIA_Bodychunk_Masking    , LOGO_MASKING,
					MUIA_Bitmap_SourceColors  , (ULONG *)logo_colors,
					MUIA_Bitmap_Transparent   , 0,
				End,
				Child, HVSpace,
			End,
			Child, ScrollgroupObject,
				MUIA_CycleChain, 1,
				MUIA_Background, MUII_ReadListBack,
				MUIA_Scrollgroup_FreeHoriz, FALSE,
				MUIA_Scrollgroup_Contents, VirtgroupObject,
					ReadListFrame,
					Child, TextObject,
						MUIA_Text_Contents, "\33c\33b\n"VERS"\n",
					End,
					Child, MUI_MakeObject(MUIO_HBar, 2),
#ifdef DEMO
					Child, TextObject,
						MUIA_Font, MUIV_Font_Big,
						MUIA_Text_Contents, "\33b\33c\nDEMO VERSION !\n\nThis program will become invalid on\n1st of November 1996. After that you\nwill have to buy the full version\nif you want to continue using it.",
					End,
#endif
					Child, TextObject,
						MUIA_Text_Contents, GetStr(MSG_TX_About1),
					End,
					Child, MUI_MakeObject(MUIO_HBar, 2),
					Child, TextObject,
						MUIA_Text_Contents, GetStr(MSG_TX_About2),
					End,
				End,
			End,
			Child, HGroup,
				Child, HSpace(0),
				Child, tmp.BT_Button = MakeButton(MSG_BT_Okay),
				Child, HSpace(0),
			End,
		End))
	{
		struct About_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		set(tmp.BT_Button, MUIA_CycleChain, 1);
		set(obj, MUIA_Window_ActiveObject, tmp.BT_Button);

		DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE ,
			MUIV_Notify_Application, 5, MUIM_Application_PushMethod,
			win, 2, MUIM_AmiTCPPrefs_About_Finish, obj);
		DoMethod(data->BT_Button, MUIM_Notify, MUIA_Pressed, FALSE ,
			MUIV_Notify_Application, 5, MUIM_Application_PushMethod,
			win, 2, MUIM_AmiTCPPrefs_About_Finish, obj);
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG About_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW : return(About_New		(cl,obj,(APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Info Window class                                                        */
/****************************************************************************/

SAVEDS ASM struct InfoLine *InfoList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct InfoLine *src)
{
	struct InfoLine *new;

	if((new = (struct InfoLine *)AllocVec(sizeof(struct InfoLine), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct InfoLine));
	return(new);
}

SAVEDS ASM VOID InfoList_DestructFunc(REG(a2) APTR pool, REG(a1) struct InfoLine *info)
{
	if(info)
		FreeVec(info);
}

SAVEDS ASM LONG InfoList_DisplayFunc(REG(a2) char **array, REG(a1) struct InfoLine *info)
{
	if(info)
	{
		*array++	= info->Label;
		*array	= info->Contents;
	}
	return(NULL);
}

ULONG InfoWindow_LoadFile(struct IClass *cl, Object *obj, Msg msg)
{
	struct AmiTCPPrefs_Data	*mw_data				= INST_DATA(CL_AmiTCPPrefs->mcc_Class	, win);
	struct Provider_Data		*provider_data		= INST_DATA(CL_Provider->mcc_Class		, mw_data->GR_Provider);
	struct InfoWindow_Data *data = INST_DATA(cl, obj);
	struct InfoLine *info;
	struct pc_Data *pc_data;
	char file[MAXPATHLEN];
	BOOL success = FALSE;

	strcpy(file, "NetConnect:Data/Providers");
	AddPart(file, (STRPTR)xget(provider_data->PO_Country	, MUIA_Text_Contents), MAXPATHLEN);
	AddPart(file, (STRPTR)xget(provider_data->PO_Provider	, MUIA_Text_Contents), MAXPATHLEN);
	AddPart(file, (STRPTR)xget(provider_data->PO_PoP		, MUIA_Text_Contents), MAXPATHLEN);
	if(get_file_size(file) != -2)
	{
		strcpy(file, "NetConnect:Data/Providers");
		AddPart(file, (STRPTR)xget(provider_data->PO_Country	, MUIA_Text_Contents), MAXPATHLEN);
		AddPart(file, (STRPTR)xget(provider_data->PO_Provider	, MUIA_Text_Contents), MAXPATHLEN);
	}
	AddPart(file, "provider.txt", MAXPATHLEN);

	set(data->LV_Info, MUIA_List_Quiet, TRUE);
	DoMethod(data->LV_Info, MUIM_List_Clear);
	if(info = (struct InfoLine *)AllocVec(sizeof(struct InfoLine), MEMF_ANY))
	{
		if(pc_data = AllocVec(sizeof(struct pc_Data), MEMF_ANY | MEMF_CLEAR))
		{
			if(ParseConfig(file, pc_data))
			{
				success = TRUE;
				while(ParseNext(pc_data))
				{
					strncpy(info->Label, pc_data->Argument, 40);
					strncpy(info->Contents, pc_data->Contents, 80);
					DoMethod(data->LV_Info, MUIM_List_InsertSingle, info, MUIV_List_Insert_Bottom);
				}
				ParseEnd(pc_data);
			}
			FreeVec(pc_data);
		}
		FreeVec(info);
	}
	set(data->LV_Info, MUIA_List_Quiet, FALSE);
	set(provider_data->CH_ProviderInfo, MUIA_Disabled, !success);
	if(!success && xget(provider_data->CH_ProviderInfo, MUIA_Selected))
		set(provider_data->CH_ProviderInfo, MUIA_Selected, FALSE);

	return((ULONG)success);
}

ULONG InfoWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	static const struct Hook InfoList_ConstructHook= { { 0,0 }, (VOID *)InfoList_ConstructFunc	, NULL, NULL };
	static const struct Hook InfoList_DestructHook	= { { 0,0 }, (VOID *)InfoList_DestructFunc	, NULL, NULL };
	static const struct Hook InfoList_DisplayHook	= { { 0,0 }, (VOID *)InfoList_DisplayFunc		, NULL, NULL };
	struct InfoWindow_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title, GetStr(MSG_WI_ProviderInfo),
		MUIA_Window_ID   , MAKE_ID('I','N','F','O'),
		MUIA_Window_Activate, FALSE,
		WindowContents, VGroup,
			Child, tmp.LV_Info = ListviewObject,
				MUIA_Weight, 10,
				MUIA_ShowMe, FALSE,
				MUIA_Listview_Input			, FALSE,
				MUIA_Listview_List			, tmp.LI_Info = ListObject,
					MUIA_Frame					, MUIV_Frame_InputList,
					MUIA_List_ConstructHook	, &InfoList_ConstructHook,
					MUIA_List_DestructHook	, &InfoList_DestructHook,
					MUIA_List_DisplayHook	, &InfoList_DisplayHook,
					MUIA_List_Format			, "BAR,",
					MUIA_List_AdjustWidth	, TRUE,
				End,
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct InfoWindow_Data *data = INST_DATA(cl,obj);

		*data = tmp;
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG InfoWindow_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW							: return(InfoWindow_New			(cl, obj, (APTR)msg));
		case MUIM_InfoWindow_LoadFile	: return(InfoWindow_LoadFile	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Provider class (GROUP)                                                   */
/****************************************************************************/

ULONG Provider_PopList_Update(struct IClass *cl, Object *obj, struct MUIP_Provider_PopList_Update *msg)
{
	struct Provider_Data *data = INST_DATA(cl, obj);
	Object *list;

	switch(msg->flags)
	{
		case MUIV_Provider_PopString_Country:
			list = data->LV_Country;
			DoMethod(data->LV_Provider	, MUIM_List_Clear);
			DoMethod(data->LV_PoP		, MUIM_List_Clear);
			set(data->PO_Provider		, MUIA_Text_Contents, "");
			set(data->PO_PoP				, MUIA_Text_Contents, "");
			break;
		case MUIV_Provider_PopString_Provider:
			list = data->LV_Provider;
			DoMethod(data->LV_PoP		, MUIM_List_Clear);
			set(data->PO_PoP				, MUIA_Text_Contents, "");
			break;
		case MUIV_Provider_PopString_PoP:
			list = data->LV_PoP;
			break;
	}

	DoMethod(list, MUIM_List_Clear);
	if(msg->flags == MUIV_Provider_PopString_PoP)
	{
		struct pc_Data pc_data;
		struct PoP *pop;
		char file[MAXPATHLEN];

		strcpy(file, msg->path);
		AddPart(file, "PoPList", MAXPATHLEN);
		if(ParseConfig(file, &pc_data))
		{
			if(pop = (struct PoP *)AllocVec(sizeof(struct PoP), MEMF_ANY))
			{
				while(ParseNext(&pc_data))
				{
					strncpy(pop->Name, pc_data.Argument, 80);
					strncpy(pop->Phone, pc_data.Contents, 80);
					DoMethod(list, MUIM_List_InsertSingle, pop, MUIV_List_Insert_Sorted);
				}
				FreeVec(pop);
			}
			ParseEnd(&pc_data);
		}
	}
	else
	{
		BPTR lock;
		struct FileInfoBlock *fib;

		if(lock = Lock(msg->path, ACCESS_READ))
		{
			if(fib = AllocDosObject(DOS_FIB, NULL))
			{
				if(Examine(lock, fib))
				{
					while(ExNext(lock, fib))
					{
						if(fib->fib_DirEntryType > 0)
							DoMethod(list, MUIM_List_InsertSingle, fib->fib_FileName, MUIV_List_Insert_Sorted);
					}
				}
				FreeDosObject(DOS_FIB, fib);
			}
			UnLock(lock);
		}
	}
	return(NULL);
}

ULONG Provider_PopString_Close(struct IClass *cl, Object *obj, struct MUIP_Provider_PopString_Close *msg)
{
	struct Provider_Data *data = INST_DATA(cl, obj);
	Object *list_view, *string;
	STRPTR x;
	char path[MAXPATHLEN];

	switch(msg->flags)
	{
		case MUIV_Provider_PopString_Country:
			list_view	= data->LV_Country;
			string		= data->PO_Country;
			break;
		case MUIV_Provider_PopString_Provider:
			list_view	= data->LV_Provider;
			string		= data->PO_Provider;
			break;
		case MUIV_Provider_PopString_PoP:
			list_view	= data->LV_PoP;
			string		= data->PO_PoP;
			break;
	}

	if(list_view && string)
	{
		DoMethod(list_view, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &x);
		if(x)
			set(string, MUIA_Text_Contents, x);
		DoMethod(string, MUIM_Popstring_Close, TRUE);
	}

	switch(msg->flags)
	{
		case MUIV_Provider_PopString_Country:
			strcpy(path, "NetConnect:Data/Providers");
			AddPart(path, (STRPTR)xget(data->PO_Country, MUIA_Text_Contents), MAXPATHLEN);
			DoMethod(obj, MUIM_Provider_PopList_Update, path, MUIV_Provider_PopString_Provider);
			set(data->PO_Provider, MUIA_Text_Contents, GetStr(MSG_TX_SelectProvider));
			break;

		case MUIV_Provider_PopString_Provider:
			strcpy(path, "NetConnect:Data/Providers");
			AddPart(path, (STRPTR)xget(data->PO_Country	, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(path, (STRPTR)xget(data->PO_Provider	, MUIA_Text_Contents), MAXPATHLEN);
			DoMethod(obj, MUIM_Provider_PopList_Update, path, MUIV_Provider_PopString_PoP);
			set(data->PO_PoP, MUIA_Text_Contents, GetStr(MSG_TX_SelectPoP));
			break;

		case MUIV_Provider_PopString_PoP:
			strcpy(path, "NetConnect:Data/Providers");
			AddPart(path, (STRPTR)xget(data->PO_Country	, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(path, (STRPTR)xget(data->PO_Provider	, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(path, (STRPTR)xget(data->PO_PoP		, MUIA_Text_Contents), MAXPATHLEN);
			if(get_file_size(path) == -2)
			{
				// the pop has got its own directory
				DoMethod(win, MUIM_AmiTCPPrefs_LoadConfig, path);
			}
			else
			{
				struct PoP *pop;

				// the pop uses the provider's standard files
				strcpy(path, "NetConnect:Data/Providers");
				AddPart(path, (STRPTR)xget(data->PO_Country	, MUIA_Text_Contents), MAXPATHLEN);
				AddPart(path, (STRPTR)xget(data->PO_Provider	, MUIA_Text_Contents), MAXPATHLEN);
				DoMethod(win, MUIM_AmiTCPPrefs_LoadConfig, path);
				pop = (struct PoP *)x;
				setstring(data->STR_Phone, pop->Phone);
			}
			break;
	}
	DoMethod(win, MUIM_InfoWindow_LoadFile);

	return(NULL);
}

ULONG Provider_Interface_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct Provider_Data *data = INST_DATA(cl, obj);

	switch(xget(data->RA_Interface, MUIA_Radio_Active))
	{
		case 0:
			set(data->CY_Authentication, MUIA_Disabled, FALSE);
			DoMethod(obj, MUIM_Provider_Authentication_Active);
			break;
		case 1:
			set(data->CY_Authentication, MUIA_Disabled, TRUE);
			set(data->STR_HostID, MUIA_Disabled, TRUE);
			set(data->STR_YourID, MUIA_Disabled, TRUE);
			set(data->STR_Password, MUIA_Disabled, TRUE);
			break;
	}

	return(NULL);
}


ULONG Provider_Authentication_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct Provider_Data *data = INST_DATA(cl, obj);
	int i;

	i = xget(data->CY_Authentication, MUIA_Cycle_Active);
	set(data->STR_HostID, MUIA_Disabled		, (i != 1));
	set(data->STR_YourID, MUIA_Disabled		, !i);
	set(data->STR_Password, MUIA_Disabled	, !i);
	if(i)
	{
		struct AmiTCPPrefs_Data *xdata = INST_DATA(CL_AmiTCPPrefs->mcc_Class, win);
		struct User_Data *user_data = INST_DATA(CL_User->mcc_Class, xdata->GR_User);

		if(!strlen((STRPTR)xget(data->STR_Password, MUIA_String_Contents)))
			setstring(data->STR_Password, xget(user_data->STR_Password, MUIA_String_Contents));
		if(!strlen((STRPTR)xget(data->STR_YourID, MUIA_String_Contents)))
			setstring(data->STR_YourID, xget(user_data->STR_LoginName, MUIA_String_Contents));
	}

	return(NULL);
}

ULONG Provider_DialScriptList_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct Provider_Data *data = INST_DATA(cl, obj);
	STRPTR ptr;

	DoMethod(data->LV_DialScript, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
	if(ptr)
	{
		set(data->STR_Line, MUIA_Disabled, FALSE);
		set(data->BT_Delete, MUIA_Disabled, FALSE);
		setstring(data->STR_Line, ptr);
	}
	else
	{
		set(data->STR_Line, MUIA_Disabled, TRUE);
		set(data->BT_Delete, MUIA_Disabled, TRUE);
		setstring(data->STR_Line, "");
	}

	return(NULL);
}

ULONG Provider_DialScriptPopString_Close(struct IClass *cl, Object *obj, Msg msg)
{
	struct Provider_Data *data = INST_DATA(cl, obj);
	STRPTR x;

	DoMethod(data->LV_Line, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &x);
	if(x)
		DoMethod(data->LV_DialScript, MUIM_List_InsertSingle, x, (xget(data->LV_Line, MUIA_List_Active) ? MUIV_List_Insert_Active : MUIV_List_Insert_Top));

	DoMethod(data->PO_Line, MUIM_Popstring_Close, TRUE);

	return(NULL);
}

ULONG Provider_ChangeLine(struct IClass *cl, Object *obj, Msg msg)
{
	struct Provider_Data *data = INST_DATA(cl, obj);
	LONG i;

	i = xget(data->LV_DialScript, MUIA_List_Active);
	if(i != MUIV_List_Active_Off)
	{
		set(data->LV_DialScript, MUIA_List_Quiet, TRUE);
		DoMethod(data->LV_DialScript, MUIM_List_InsertSingle, xget(data->STR_Line, MUIA_String_Contents), i + 1);
		DoMethod(data->LV_DialScript, MUIM_List_Remove, i);
		set(data->LV_DialScript, MUIA_List_Quiet, FALSE);
		set(win, MUIA_Window_ActiveObject, data->STR_Line);
	}

	return(NULL);
}

SAVEDS ASM struct PoP *PoP_ConstructFunc(REG(a2) APTR pool, REG(a1) struct PoP *src)
{
	struct PoP *new;

	if((new = (struct PoP *)AllocVec(sizeof(struct PoP), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct PoP));
	return(new);
}

SAVEDS ASM VOID PoP_DestructFunc(REG(a2) APTR pool, REG(a1) struct PoP *pop)
{
	if(pop)
		FreeVec(pop);
}

SAVEDS ASM LONG PoP_DisplayFunc(REG(a2) char **array, REG(a1) struct PoP *pop)
{
	if(pop)
	{
		*array++	= pop->Name;
		*array	= pop->Phone;
	}
	return(NULL);
}

ULONG Provider_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	static const struct Hook PoP_ConstructHook	= { { 0,0 }, (VOID *)PoP_ConstructFunc	, NULL, NULL };
	static const struct Hook PoP_DestructHook		= { { 0,0 }, (VOID *)PoP_DestructFunc	, NULL, NULL };
	static const struct Hook PoP_DisplayHook		= { { 0,0 }, (VOID *)PoP_DisplayFunc	, NULL, NULL };
	static STRPTR ARR_DialScript_AddLine[] = { "/* Login Script */", "YourLogin=\"\"", "YourPassword=\"\"", "ShowConsole", "call CommandState", "call Dial", "WaitFor", "WaitFor \"sername:\"", "WaitFor \"assword:\"", "SendLn", "SendLn YourLogin", "SendLn YourPassword", "SendLn \"slip\"", "SendLn \"ppp\"", NULL };
	static STRPTR STR_GR_ProviderRegister[5];
	static STRPTR STR_RA_Connection[3];
	static STRPTR STR_RA_Interface[3];
	static STRPTR STR_CY_Authentication[4];
	static STRPTR STR_CY_Header[4];
	struct Provider_Data tmp;

	STR_GR_ProviderRegister[0] = GetStr(MSG_ProviderRegister1);
	STR_GR_ProviderRegister[1] = GetStr(MSG_ProviderRegister2);
	STR_GR_ProviderRegister[2] = GetStr(MSG_ProviderRegister3);
	STR_GR_ProviderRegister[3] = GetStr(MSG_ProviderRegister4);
	STR_GR_ProviderRegister[4] = NULL;

	STR_RA_Connection[0] = GetStr(MSG_RA_Connection0);
	STR_RA_Connection[1] = GetStr(MSG_RA_Connection1);
	STR_RA_Connection[2] = NULL;

	STR_RA_Interface[0] = "PPP";
	STR_RA_Interface[1] = "SLIP";
	STR_RA_Interface[2] = NULL;

	STR_CY_Authentication[0] = GetStr(MSG_CY_Authentication1);
	STR_CY_Authentication[1] = GetStr(MSG_CY_Authentication2);
	STR_CY_Authentication[2] = GetStr(MSG_CY_Authentication3);
	STR_CY_Authentication[3] = NULL;

	STR_CY_Header[0] = GetStr(MSG_CY_Header1);
	STR_CY_Header[1] = GetStr(MSG_CY_Header2);
	STR_CY_Header[2] = GetStr(MSG_CY_Header3);
	STR_CY_Header[3] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		Child, tmp.GR_Register = RegisterObject,
			MUIA_Background	, MUII_RegisterBack,
			MUIA_Register_Titles	, STR_GR_ProviderRegister,
			MUIA_CycleChain		, 1,
			Child, VGroup,
				Child, HVSpace,
				Child, ColGroup(2),
					GroupFrame,
					Child, Label2(GetStr(MSG_LA_Country)),
					Child, tmp.PO_Country = PopobjectObject,
						MUIA_Popstring_String		, tmp.TX_Country = TextObject,
							TextFrame,
							MUIA_Text_Contents, GetStr(MSG_TX_SelectCountry),
						End,
						MUIA_Popstring_Button		, PopButton(MUII_PopUp),
					   MUIA_Popobject_StrObjHook	, &txtobjhook,
						MUIA_Popobject_Object		, tmp.LV_Country = ListviewObject,
							MUIA_Listview_DoubleClick	, TRUE,
							MUIA_Listview_List			, ListObject,
								MUIA_Frame					, MUIV_Frame_InputList,
								MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
								MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
								MUIA_List_CompareHook	, &sorthook,
								MUIA_List_AutoVisible	, TRUE,
							End,
						End,
					End,
					Child, Label2(GetStr(MSG_LA_Provider)),
					Child, tmp.PO_Provider = PopobjectObject,
						MUIA_Popstring_String		, tmp.TX_Provider = TextObject, TextFrame, End,
						MUIA_Popstring_Button		, PopButton(MUII_PopUp),
					   MUIA_Popobject_StrObjHook	, &txtobjhook,
						MUIA_Popobject_Object		, tmp.LV_Provider = ListviewObject,
							MUIA_Listview_DoubleClick	, TRUE,
							MUIA_Listview_List			, ListObject,
								MUIA_Frame					, MUIV_Frame_InputList,
								MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
								MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
								MUIA_List_CompareHook	, &sorthook,
								MUIA_List_AutoVisible	, TRUE,
							End,
						End,
					End,
					Child, Label2(GetStr(MSG_LA_PoP)),
					Child, tmp.PO_PoP = PopobjectObject,
						MUIA_Popstring_String		, tmp.TX_PoP = TextObject, TextFrame, End,
						MUIA_Popstring_Button		, PopButton(MUII_PopUp),
					   MUIA_Popobject_StrObjHook	, &txtobjhook,
						MUIA_Popobject_Object		, tmp.LV_PoP = ListviewObject,
							MUIA_Listview_DoubleClick	, TRUE,
							MUIA_Listview_List			, ListObject,
								MUIA_Frame			, MUIV_Frame_InputList,
								MUIA_List_ConstructHook	, &PoP_ConstructHook,
								MUIA_List_DestructHook	, &PoP_DestructHook,
								MUIA_List_DisplayHook	, &PoP_DisplayHook,
								MUIA_List_CompareHook	, &sorthook,
								MUIA_List_Format			, "BAR,",
								MUIA_List_AutoVisible	, TRUE,
							End,
						End,
					End,
				End,
				Child, HVSpace,
				Child, ColGroup(2),
					GroupFrame,
					Child, MakeKeyLabel2(MSG_LA_Phone, MSG_CC_Phone),
					Child, tmp.STR_Phone = MakeKeyString("", 80, MSG_CC_Phone),
					Child, MakeKeyLabel2(MSG_LA_ShowProviderInfo, MSG_CC_ShowProviderInfo),
					Child, HGroup,
						Child, tmp.CH_ProviderInfo = KeyCheckMark(FALSE, *GetStr(MSG_CC_ShowProviderInfo)),
						Child, HVSpace,
					End,
				End,
				Child, HVSpace,
			End,
			Child, VGroup,
				Child, HVSpace,
				Child, HGroup,
					GroupFrameT(GetStr(MSG_GR_ConnectionTitle)),
					Child, HVSpace,
					Child, tmp.RA_Connection = RadioObject,
						MUIA_Radio_Entries, STR_RA_Connection,
					End,
					Child, HVSpace,
					Child, tmp.RA_Interface = RadioObject,
						MUIA_Radio_Entries, STR_RA_Interface,
					End,
					Child, HVSpace,
					Child, ColGroup(2),
						Child, tmp.CH_BOOTP = KeyCheckMark(FALSE, *GetStr(MSG_CC_BOOTP)),
						Child, MakeKeyLabel1(MSG_LA_BOOTP, MSG_CC_BOOTP),
						Child, tmp.SL_MTU = NumericbuttonObject,
							MUIA_CycleChain		, 1,
							MUIA_ControlChar		, 't',
							MUIA_Numeric_Min		, 72,
							MUIA_Numeric_Max		, 1524,
							MUIA_Numeric_Value	, 1500,
						End,
						Child, MakeKeyLabel1(MSG_LA_MTU, MSG_CC_MTU),
					End,
					Child, HVSpace,
				End,
				Child, HVSpace,
				Child, ColGroup(2),
					GroupFrameT(GetStr(MSG_TX_InterfaceOptions)),
					Child, MakeKeyLabel2(MSG_LA_Compression, MSG_CC_Compression),
					Child, tmp.CY_Header = MakeKeyCycle(STR_CY_Header, MSG_CC_Compression),
					Child, MakeKeyLabel2(MSG_LA_Authentication, MSG_CC_Authentication),
					Child, tmp.CY_Authentication = MakeKeyCycle(STR_CY_Authentication, MSG_CC_Authentication),
					Child, MakeKeyLabel2(MSG_LA_ISPHostID, MSG_CC_ISPHostID),
					Child, tmp.STR_HostID = MakeKeyString("", 80, MSG_CC_ISPHostID),
					Child, MakeKeyLabel2(MSG_LA_YourHostID, MSG_CC_YourHostID),
					Child, tmp.STR_YourID = MakeKeyString("", 80, MSG_CC_YourHostID),
					Child, MakeKeyLabel2(MSG_LA_Password, MSG_CC_Password),
					Child, tmp.STR_Password = StringObject,
						MUIA_CycleChain		, 1,
						MUIA_ControlChar		, *GetStr(MSG_CC_Password),
						MUIA_Frame				, MUIV_Frame_String,
						MUIA_String_Secret	, TRUE,
					End,
				End,
				Child, HVSpace,
			End,
			Child, VGroup,
				Child, HVSpace,
				Child, ColGroup(2),
					GroupFrame,
					Child, MakeKeyLabel2(MSG_LA_DomainName, MSG_CC_DomainName),
					Child, tmp.STR_DomainName = MakeKeyString("", 80, MSG_CC_DomainName),
					Child, MakeKeyLabel2(MSG_LA_NameServer1, MSG_CC_NameServer1),
					Child, tmp.STR_NameServer1 = StringObject,
						MUIA_ControlChar		, *GetStr(MSG_CC_NameServer1),
						MUIA_CycleChain		, 1,
						MUIA_Frame				, MUIV_Frame_String,
						MUIA_String_Accept	, "0123456789.",
						MUIA_String_MaxLen	, 18,
					End,
					Child, MakeKeyLabel2(MSG_LA_NameServer2, MSG_CC_NameServer2),
					Child, tmp.STR_NameServer2 = StringObject,
						MUIA_ControlChar		, *GetStr(MSG_CC_NameServer2),
						MUIA_CycleChain		, 1,
						MUIA_Frame				, MUIV_Frame_String,
						MUIA_String_Accept	, "0123456789.",
						MUIA_String_MaxLen	, 18,
					End,
				End,
				Child, HVSpace,
				Child, ColGroup(2),
					GroupFrame,
					Child, MakeKeyLabel2(MSG_LA_MailServer, MSG_CC_MailServer),
					Child, tmp.STR_MailServer = MakeKeyString("", 80, MSG_CC_MailServer),
					Child, MakeKeyLabel2(MSG_LA_POP3Server, MSG_CC_POP3Server),
					Child, tmp.STR_POPServer = MakeKeyString("", 80, MSG_CC_POP3Server),
					Child, MakeKeyLabel2(MSG_LA_NewsServer, MSG_CC_NewsServer),
					Child, tmp.STR_NewsServer = MakeKeyString("", 80, MSG_CC_NewsServer),
					Child, MakeKeyLabel2(MSG_LA_WWWServer, MSG_CC_WWWServer),
					Child, tmp.STR_WWWServer = MakeKeyString("", 80, MSG_CC_WWWServer),
					Child, MakeKeyLabel2(MSG_LA_FTPServer, MSG_CC_FTPServer),
					Child, tmp.STR_FTPServer = MakeKeyString("", 80, MSG_CC_FTPServer),
				End,
				Child, HVSpace,
			End,
			Child, VGroup,
				MUIA_Group_Spacing, 0,
				Child, tmp.LV_DialScript = ListviewObject,
					MUIA_CycleChain		, 1,
					MUIA_Listview_DragType		, 1,
					MUIA_Listview_List			, tmp.LI_DialScript = ListObject,
						MUIA_Frame					, MUIV_Frame_InputList,
						MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
						MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
						MUIA_List_DragSortable	, TRUE,
						MUIA_List_AutoVisible	, TRUE,
					End,
				End,
				Child, tmp.PO_Line = PopobjectObject,
					MUIA_Popstring_String		, tmp.STR_Line = MakeKeyString("", MAXPATHLEN, "   "),
					MUIA_Popstring_Button		, PopButton(MUII_PopUp),
					MUIA_Popobject_Object		, tmp.LV_Line = ListviewObject,
						MUIA_Listview_DoubleClick	, TRUE,
						MUIA_Listview_List			, ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
							MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
							MUIA_List_SourceArray	, ARR_DialScript_AddLine,
						End,
					End,
				End,
				Child, HGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.BT_New		= MakeButton(MSG_BT_New),
					Child, tmp.BT_Delete	= MakeButton(MSG_BT_Delete),
					Child, tmp.BT_Clear	= MakeButton(MSG_BT_Clear),
				End,
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct Provider_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		set(tmp.CH_ProviderInfo, MUIA_Disabled, TRUE);
		set(tmp.STR_HostID, MUIA_Disabled, TRUE);
		set(tmp.STR_YourID, MUIA_Disabled, TRUE);
		set(tmp.STR_Password, MUIA_Disabled, TRUE);
		set(tmp.STR_Line, MUIA_Disabled, TRUE);
		set(tmp.BT_Delete, MUIA_Disabled, TRUE);

		set(tmp.STR_Line, MUIA_String_AttachedList, tmp.LV_DialScript);
		set(tmp.CH_ProviderInfo, MUIA_CycleChain, 1);
		set(tmp.RA_Connection, MUIA_CycleChain, 1);
		set(tmp.RA_Interface, MUIA_CycleChain, 1);
		set(tmp.CH_BOOTP, MUIA_CycleChain, 1);

		set(tmp.TX_Country		, MUIA_ShortHelp, GetStr(MSG_Help_Country));
		set(tmp.TX_Provider		, MUIA_ShortHelp, GetStr(MSG_Help_Provider));
		set(tmp.TX_PoP				, MUIA_ShortHelp, GetStr(MSG_Help_PoP));
		set(tmp.STR_Phone			, MUIA_ShortHelp, GetStr(MSG_Help_Phone));
		set(tmp.CH_ProviderInfo	, MUIA_ShortHelp, GetStr(MSG_Help_ProviderInfo));

		set(tmp.RA_Connection	, MUIA_ShortHelp, GetStr(MSG_Help_Connection));
		set(tmp.RA_Interface		, MUIA_ShortHelp, GetStr(MSG_Help_Interface));
		set(tmp.CH_BOOTP			, MUIA_ShortHelp, GetStr(MSG_Help_BOOTP));
		set(tmp.SL_MTU				, MUIA_ShortHelp, GetStr(MSG_Help_MTU));
		set(tmp.CY_Header			, MUIA_ShortHelp, GetStr(MSG_Help_Header));
		set(tmp.CY_Authentication, MUIA_ShortHelp,GetStr(MSG_Help_Authentication));
		set(tmp.STR_HostID		, MUIA_ShortHelp, GetStr(MSG_Help_HostID));
		set(tmp.STR_YourID		, MUIA_ShortHelp, GetStr(MSG_Help_YourID));
		set(tmp.STR_Password		, MUIA_ShortHelp, GetStr(MSG_Help_AuthPassword));

		set(tmp.STR_DomainName	, MUIA_ShortHelp, GetStr(MSG_Help_DomainName));
		set(tmp.STR_NameServer1	, MUIA_ShortHelp, GetStr(MSG_Help_NameServer1));
		set(tmp.STR_NameServer2	, MUIA_ShortHelp, GetStr(MSG_Help_NameServer2));
		set(tmp.STR_MailServer	, MUIA_ShortHelp, GetStr(MSG_Help_MailServer));
		set(tmp.STR_POPServer	, MUIA_ShortHelp, GetStr(MSG_Help_POPServer));
		set(tmp.STR_NewsServer	, MUIA_ShortHelp, GetStr(MSG_Help_NewsServer));
		set(tmp.STR_WWWServer	, MUIA_ShortHelp, GetStr(MSG_Help_WWWServer));
		set(tmp.STR_FTPServer	, MUIA_ShortHelp, GetStr(MSG_Help_FTPServer));

		set(tmp.LV_DialScript	, MUIA_ShortHelp, GetStr(MSG_Help_DialScript));
		set(tmp.STR_Line			, MUIA_ShortHelp, GetStr(MSG_Help_Line));
		set(tmp.BT_New				, MUIA_ShortHelp, GetStr(MSG_Help_New));
		set(tmp.BT_Delete			, MUIA_ShortHelp, GetStr(MSG_Help_Delete));
		set(tmp.BT_Clear			, MUIA_ShortHelp, GetStr(MSG_Help_Clear));


		DoMethod(tmp.LV_Country	, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime	, obj, 2, MUIM_Provider_PopString_Close, MUIV_Provider_PopString_Country);
		DoMethod(tmp.LV_Provider, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime	, obj, 2, MUIM_Provider_PopString_Close, MUIV_Provider_PopString_Provider);
		DoMethod(tmp.LV_PoP		, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime	, obj, 2, MUIM_Provider_PopString_Close, MUIV_Provider_PopString_PoP);

		DoMethod(tmp.RA_Interface, MUIM_Notify, MUIA_Radio_Active, MUIV_EveryTime, obj, 1, MUIM_Provider_Interface_Active);
		DoMethod(tmp.CY_Authentication, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 1, MUIM_Provider_Authentication_Active);

		DoMethod(tmp.LV_DialScript, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, obj, 1, MUIM_Provider_DialScriptList_Active);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_DialScript, 3, MUIM_List_InsertSingle, "", MUIV_List_Insert_Bottom);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_DialScript, 3, MUIM_Set, MUIA_List_Active, MUIV_List_Active_Bottom);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Line);
		DoMethod(tmp.BT_Delete, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_DialScript, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.BT_Clear, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_DialScript, 1, MUIM_List_Clear);
		DoMethod(tmp.STR_Line, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 1, MUIM_Provider_ChangeLine);
		DoMethod(tmp.LV_Line		, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime	, obj, 1, MUIM_Provider_DialScriptPopString_Close);
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG Provider_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW												: return(Provider_New								(cl, obj, (APTR)msg));
		case MUIM_Provider_PopString_Close				: return(Provider_PopString_Close				(cl, obj, (APTR)msg));
		case MUIM_Provider_PopList_Update				: return(Provider_PopList_Update					(cl, obj, (APTR)msg));
		case MUIM_Provider_DialScriptList_Active		: return(Provider_DialScriptList_Active		(cl, obj, (APTR)msg));
		case MUIM_Provider_ChangeLine						: return(Provider_ChangeLine						(cl, obj, (APTR)msg));
		case MUIM_Provider_DialScriptPopString_Close	: return(Provider_DialScriptPopString_Close	(cl, obj, (APTR)msg));
		case MUIM_Provider_Interface_Active				: return(Provider_Interface_Active				(cl, obj, (APTR)msg));
		case MUIM_Provider_Authentication_Active		: return(Provider_Authentication_Active		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* User class (GROUP)                                                       */
/****************************************************************************/

ULONG User_UserStartnetList_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct User_Data *data = INST_DATA(cl, obj);
	STRPTR ptr;

	DoMethod(data->LV_UserStartnet, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
	if(ptr)
	{
		set(data->STR_Line, MUIA_Disabled, FALSE);
		set(data->BT_Delete, MUIA_Disabled, FALSE);
		setstring(data->STR_Line, ptr);
	}
	else
	{
		set(data->STR_Line, MUIA_Disabled, TRUE);
		set(data->BT_Delete, MUIA_Disabled, TRUE);
		setstring(data->STR_Line, "");
	}

	return(NULL);
}

ULONG User_ChangeLine(struct IClass *cl, Object *obj, Msg msg)
{
	struct User_Data *data = INST_DATA(cl, obj);
	LONG i;

	i = xget(data->LV_UserStartnet, MUIA_List_Active);
	if(i != MUIV_List_Active_Off)
	{
		set(data->LV_UserStartnet, MUIA_List_Quiet, TRUE);
		DoMethod(data->LV_UserStartnet, MUIM_List_InsertSingle, xget(data->STR_Line, MUIA_String_Contents), i + 1);
		DoMethod(data->LV_UserStartnet, MUIM_List_Remove, i);
		set(data->LV_UserStartnet, MUIA_List_Quiet, FALSE);
		set(win, MUIA_Window_ActiveObject, data->STR_Line);
	}

	return(NULL);
}

ULONG User_ChangeDialScript(struct IClass *cl, Object *obj, Msg msg)
{
	struct AmiTCPPrefs_Data	*data					= INST_DATA(CL_AmiTCPPrefs->mcc_Class	, win);
	struct Provider_Data		*provider_data		= INST_DATA(CL_Provider->mcc_Class		, data->GR_Provider);
	struct User_Data			*user_data			= INST_DATA(cl									, obj);
	int i;
	STRPTR ptr;
	char string[101];

	i = 0;
	FOREVER
	{
		DoMethod(provider_data->LV_DialScript, MUIM_List_GetEntry, i, &ptr);
		if(!ptr)
			break;
		if((STRPTR)strstr(ptr, "YourLogin=") == ptr)
		{
			sprintf(string, "YourLogin=\"%ls\"", xget(user_data->STR_LoginName, MUIA_String_Contents));
			DoMethod(provider_data->LV_DialScript, MUIM_List_InsertSingle, string, i + 1);
			DoMethod(provider_data->LV_DialScript, MUIM_List_Remove, i);
		}
		if((STRPTR)strstr(ptr, "YourPassword=") == ptr)
		{
			sprintf(string, "YourPassword=\"%ls\"", xget(user_data->STR_Password, MUIA_String_Contents));
			DoMethod(provider_data->LV_DialScript, MUIM_List_InsertSingle, string, i + 1);
			DoMethod(provider_data->LV_DialScript, MUIM_List_Remove, i);
		}
		i++;
	}
	return(NULL);
}

ULONG User_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct User_Data tmp;
	static STRPTR STR_GR_UserRegister[3];

	STR_GR_UserRegister[0] = GetStr(MSG_UserRegister1);
	STR_GR_UserRegister[1] = GetStr(MSG_UserRegister2);
	STR_GR_UserRegister[2] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		Child, tmp.GR_Register = RegisterObject,
			MUIA_Background		, MUII_RegisterBack,
			MUIA_Register_Titles	, STR_GR_UserRegister,
			MUIA_CycleChain		, 1,
			Child, ColGroup(2),
				GroupFrame,
				Child, HVSpace,
				Child, HVSpace,
				Child, MakeKeyLabel2(MSG_LA_LoginName, MSG_CC_LoginName),
				Child, tmp.STR_LoginName = MakeKeyString("", 80, MSG_CC_LoginName),
				Child, MakeKeyLabel2(MSG_LA_Password, MSG_CC_Password),
				Child, tmp.STR_Password = StringObject,
					MUIA_ControlChar		, *GetStr(MSG_CC_Password),
					MUIA_CycleChain		, 1,
					MUIA_Frame				, MUIV_Frame_String,
					MUIA_String_Secret	, TRUE,
					MUIA_String_MaxLen	, 80,
				End,
				Child, HVSpace,
				Child, HVSpace,
				Child, MakeKeyLabel2(MSG_LA_EMail, MSG_CC_EMail),
				Child, tmp.STR_EMail = MakeKeyString("", 80, MSG_CC_EMail),
				Child, MakeKeyLabel2(MSG_LA_RealName, MSG_CC_RealName),
				Child, tmp.STR_RealName = MakeKeyString("", 80, MSG_CC_RealName),
				Child, MakeKeyLabel2(MSG_LA_Organisation, MSG_CC_Organisation),
				Child, tmp.STR_Organisation = MakeKeyString("Private User", 80, MSG_CC_Organisation),
				Child, MakeKeyLabel2(MSG_LA_HostName, MSG_CC_HostName),
				Child, tmp.STR_HostName = MakeKeyString("", 80, MSG_CC_HostName),
				Child, MakeKeyLabel2(MSG_LA_Address, MSG_CC_Address),
				Child, tmp.STR_IP_Address = StringObject,
					MUIA_ControlChar		, *GetStr(MSG_CC_Address),
					MUIA_CycleChain		, 1,
					MUIA_Frame				, MUIV_Frame_String,
					MUIA_HelpNode			, "STR_IP_Address",
					MUIA_String_Contents	, "0.0.0.0",
					MUIA_String_Accept	, "0123456789.",
					MUIA_String_MaxLen	, 18,
				End,
				Child, HVSpace,
				Child, HVSpace,
			End,
			Child, VGroup,
				MUIA_Group_Spacing, 0,
				Child, tmp.LV_UserStartnet = ListviewObject,
					MUIA_CycleChain		, 1,
					MUIA_Listview_DragType		, 1,
					MUIA_Listview_Input			, TRUE,
					MUIA_Listview_List			, tmp.LI_UserStartnet = ListObject,
						MUIA_Frame					, MUIV_Frame_InputList,
						MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
						MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
						MUIA_List_DragSortable	, TRUE,
						MUIA_List_AutoVisible	, TRUE,
					End,
				End,
				Child, tmp.STR_Line = MakeKeyString("", MAXPATHLEN, "   "),
				Child, HGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.BT_New		= MakeButton(MSG_BT_New),
					Child, tmp.BT_Delete	= MakeButton(MSG_BT_Delete),
					Child, tmp.BT_Clear	= MakeButton(MSG_BT_Clear),
				End,
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct User_Data *data = INST_DATA(cl, obj);

		*data = tmp;

		set(tmp.STR_IP_Address, MUIA_Disabled, TRUE);
		set(tmp.STR_Line, MUIA_Disabled, TRUE);
		set(tmp.BT_Delete, MUIA_Disabled, TRUE);
		set(tmp.STR_Line, MUIA_String_AttachedList, tmp.LV_UserStartnet);


		set(tmp.STR_LoginName	, MUIA_ShortHelp, GetStr(MSG_Help_LoginName));
		set(tmp.STR_Password		, MUIA_ShortHelp, GetStr(MSG_Help_Password));
		set(tmp.STR_EMail			, MUIA_ShortHelp, GetStr(MSG_Help_EMail));
		set(tmp.STR_RealName		, MUIA_ShortHelp, GetStr(MSG_Help_RealName));
		set(tmp.STR_Organisation, MUIA_ShortHelp, GetStr(MSG_Help_Organisation));
		set(tmp.STR_HostName		, MUIA_ShortHelp, GetStr(MSG_Help_HostName));
		set(tmp.STR_IP_Address	, MUIA_ShortHelp, GetStr(MSG_Help_IP_Address));
		set(tmp.LV_UserStartnet	, MUIA_ShortHelp, GetStr(MSG_Help_UserStartnet));
		set(tmp.STR_Line			, MUIA_ShortHelp, GetStr(MSG_Help_Line));
		set(tmp.BT_New				, MUIA_ShortHelp, GetStr(MSG_Help_New));
		set(tmp.BT_Delete			, MUIA_ShortHelp, GetStr(MSG_Help_Delete));
		set(tmp.BT_Clear			, MUIA_ShortHelp, GetStr(MSG_Help_Clear));

		DoMethod(tmp.LV_UserStartnet, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, obj, 1, MUIM_User_UserStartnetList_Active);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_UserStartnet, 3, MUIM_List_InsertSingle, "", MUIV_List_Insert_Bottom);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_UserStartnet, 3, MUIM_Set, MUIA_List_Active, MUIV_List_Active_Bottom);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Line);
		DoMethod(tmp.BT_Delete, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_UserStartnet, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.BT_Clear, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_UserStartnet, 1, MUIM_List_Clear);
		DoMethod(tmp.STR_Line, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 1, MUIM_User_ChangeLine);
		DoMethod(tmp.STR_LoginName, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 1, MUIM_User_ChangeDialScript);
		DoMethod(tmp.STR_Password, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 1, MUIM_User_ChangeDialScript);
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG User_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(User_New								(cl, obj, (APTR)msg));
		case MUIM_User_UserStartnetList_Active	: return(User_UserStartnetList_Active	(cl, obj, (APTR)msg));
		case MUIM_User_ChangeLine					: return(User_ChangeLine					(cl, obj, (APTR)msg));
		case MUIM_User_ChangeDialScript			: return(User_ChangeDialScript			(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Modem class (GROUP)                                                      */
/****************************************************************************/

SAVEDS ASM struct Modem *ModemList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Modem *src)
{
	struct Modem *new;

	if((new = (struct Modem *)AllocVec(sizeof(struct Modem), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct Modem));
	return(new);
}

SAVEDS ASM VOID ModemList_DestructFunc(REG(a2) APTR pool, REG(a1) struct Modem *modem)
{
	if(modem)
		FreeVec(modem);
}

ULONG Modem_PopString_Close(struct IClass *cl, Object *obj, struct MUIP_Modem_PopString_Close *msg)
{
	struct Modem_Data *data = INST_DATA(cl, obj);
	Object *list_view, *string;
	STRPTR x;

	if(msg->flags == MUIV_Modem_PopString_Modem)
	{
		struct Modem *modem;

		DoMethod(data->LV_Modem, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem);
		if(modem)
		{
			set(data->TX_Modem, MUIA_Text_Contents, modem->Name);
			setstring(data->STR_ModemInit, modem->InitString);
		}
		DoMethod(data->PO_Modem, MUIM_Popstring_Close, TRUE);
	}
	else
	{
		switch(msg->flags)
		{
			case MUIV_Modem_PopString_BaudRate:
				list_view	= data->LV_BaudRate;
				string		= data->PO_BaudRate;
				break;
			case MUIV_Modem_PopString_DialPrefix:
				list_view	= data->LV_DialPrefix;
				string		= data->PO_DialPrefix;
				break;
			case MUIV_Modem_PopString_Device:
				list_view	= data->LV_Devices;
				string		= data->PO_SerialDriver;
				break;
		}
		if(list_view && string)
		{
			DoMethod(list_view, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &x);
			if(x)
				setstring(string, x);
			DoMethod(string, MUIM_Popstring_Close, TRUE);
		}
	}

	return(NULL);
}

ULONG Modem_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	static const struct Hook ModemList_ConstructHook= { { 0,0 }, (VOID *)ModemList_ConstructFunc	, NULL, NULL };
	static const struct Hook ModemList_DestructHook	= { { 0,0 }, (VOID *)ModemList_DestructFunc	, NULL, NULL };
	static STRPTR STR_GR_ModemRegister[3];
	static STRPTR ARR_BaudRates[] = { "9600", "14400", "19200", "38400", "57600", "76800", "115200", "230400", "345600", "460800" , NULL };
	static STRPTR ARR_DialPrefix[] = { "ATDT", "ATDP", "ATD0w", "ATD0,", NULL };
	struct Modem_Data tmp;

	STR_GR_ModemRegister[0] = GetStr(MSG_ModemRegister1);
	STR_GR_ModemRegister[1] = GetStr(MSG_ModemRegister2);
	STR_GR_ModemRegister[2] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		Child, tmp.GR_Register = RegisterObject,
			MUIA_Background		, MUII_RegisterBack,
			MUIA_Register_Titles	, STR_GR_ModemRegister,
			MUIA_CycleChain		, 1,
			Child, ColGroup(2),
				GroupFrameT(NULL),
				Child, MakeKeyLabel2(MSG_LA_ModemType, "   "),
				Child, tmp.PO_Modem = PopobjectObject,
					MUIA_Popstring_String		, tmp.TX_Modem = TextObject, TextFrame, End,
					MUIA_Popstring_Button		, PopButton(MUII_PopUp),
				   MUIA_Popobject_StrObjHook	, &txtobjhook,
					MUIA_Popobject_Object		, tmp.LV_Modem = ListviewObject,
						MUIA_CycleChain		, 1,
						MUIA_Listview_List			, ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_ConstructHook	, &ModemList_ConstructHook,
							MUIA_List_DestructHook	, &ModemList_DestructHook,
							MUIA_List_CompareHook	, &sorthook,
							MUIA_List_AutoVisible	, TRUE,
						End,
					End,
				End,
				Child, MakeKeyLabel2(MSG_LA_DialPrefix, MSG_CC_DialPrefix),
				Child, tmp.PO_DialPrefix = PopobjectObject,
					MUIA_Popstring_String		, tmp.STR_DialPrefix = MakeKeyString("ATDT", 80, MSG_CC_DialPrefix),
					MUIA_Popstring_Button		, PopButton(MUII_PopUp),
				   MUIA_Popobject_StrObjHook	, &strobjhook,
					MUIA_Popobject_Object		, tmp.LV_DialPrefix = ListviewObject,
						MUIA_Listview_DoubleClick	, TRUE,
						MUIA_Listview_List			, ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
							MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
							MUIA_List_SourceArray	, ARR_DialPrefix,
						End,
					End,
				End,
				Child, MakeKeyLabel2(MSG_LA_ModemInit, MSG_CC_ModemInit),
				Child, tmp.STR_ModemInit = MakeKeyString("ATZ\\r", 80, MSG_CC_ModemInit),
				Child, MakeKeyLabel2(MSG_LA_RedialAttempts, MSG_CC_RedialAttempts),
				Child, tmp.SL_RedialAttempts = MakeKeySlider(0, 99, 15, MSG_CC_RedialAttempts),
				Child, MakeKeyLabel2(MSG_LA_RedialDelay, MSG_CC_RedialDelay),
				Child, tmp.SL_RedialDelay = MakeKeySlider(0, 120, 5, MSG_CC_RedialDelay),
			End,
			Child, VGroup,
				Child, HVSpace,
				Child, ColGroup(2),
					GroupFrame,
					Child, MakeKeyLabel2(MSG_LA_Device, MSG_CC_Device),
					Child, tmp.PO_SerialDriver = PopobjectObject,
						MUIA_Popstring_String, tmp.STR_SerialDriver = MakeKeyString("serial.device", MAXPATHLEN, MSG_CC_Device),
						MUIA_Popstring_Button		, PopButton(MUII_PopUp),
					   MUIA_Popobject_StrObjHook	, &strobjhook,
						MUIA_Popobject_Object		, tmp.LV_Devices = ListviewObject,
							MUIA_Listview_DoubleClick	, TRUE,
							MUIA_Listview_List			, ListObject,
								MUIA_Frame					, MUIV_Frame_InputList,
								MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
								MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
								MUIA_List_CompareHook	, &sorthook,
								MUIA_List_AutoVisible	, TRUE,
							End,
						End,
					End,
					Child, MakeKeyLabel2(MSG_LA_Unit, MSG_CC_Unit),
					Child, tmp.STR_SerialUnit = StringObject,
						MUIA_ControlChar		, *GetStr(MSG_CC_Unit),
						MUIA_CycleChain		, 1,
						StringFrame,
						MUIA_String_MaxLen	, 5,
						MUIA_String_Integer	, 0,
						MUIA_String_Accept	, "1234567890",
					End,
					Child, MakeKeyLabel2(MSG_LA_BaudRate, MSG_CC_BaudRate),
					Child, tmp.PO_BaudRate = PopobjectObject,
						MUIA_Popstring_String, tmp.STR_BaudRate = StringObject,
							MUIA_ControlChar		, *GetStr(MSG_CC_BaudRate),
							MUIA_CycleChain		, 1,
							StringFrame,
							MUIA_String_MaxLen	, 8,
							MUIA_String_Integer	, 57600,
							MUIA_String_Accept	, "1234567890",
						End,
						MUIA_Popstring_Button		, PopButton(MUII_PopUp),
					   MUIA_Popobject_StrObjHook	, &strobjhook,
						MUIA_Popobject_Object		, tmp.LV_BaudRate = ListviewObject,
							MUIA_Listview_DoubleClick	, TRUE,
							MUIA_Listview_List			, ListObject,
								MUIA_Frame				, MUIV_Frame_InputList,
								MUIA_List_SourceArray, ARR_BaudRates,
								MUIA_List_AutoVisible	, TRUE,
							End,
						End,
					End,
				End,
				Child, HVSpace,
				Child, ColGroup(4),
					GroupFrame,
					Child, HVSpace,
					Child, MakeKeyLabel1(MSG_LA_CarrierDetect, MSG_CC_CarrierDetect),
					Child, tmp.CH_Carrier = KeyCheckMark(TRUE, *GetStr(MSG_CC_CarrierDetect)),
					Child, HVSpace,
					Child, HVSpace,
					Child, MakeKeyLabel1(MSG_LA_HardwareHandshake, MSG_CC_HardwareHandshake),
					Child, tmp.CH_7Wire = KeyCheckMark(TRUE, *GetStr(MSG_CC_HardwareHandshake)),
					Child, HVSpace,
					Child, HVSpace,
					Child, MakeKeyLabel1(MSG_LA_OwnDevUnit, MSG_CC_OwnDevUnit),
					Child, tmp.CH_OwnDevUnit = KeyCheckMark(FALSE, *GetStr(MSG_CC_OwnDevUnit)),
					Child, HVSpace,
				End,
				Child, HVSpace,
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct Modem_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		set(tmp.CH_Carrier, MUIA_CycleChain, 1);
		set(tmp.CH_7Wire, MUIA_CycleChain, 1);
		set(tmp.CH_OwnDevUnit, MUIA_CycleChain, 1);

		set(tmp.TX_Modem			, MUIA_ShortHelp, GetStr(MSG_Help_Modem));
		set(tmp.STR_ModemInit	, MUIA_ShortHelp, GetStr(MSG_Help_ModemInit));
		set(tmp.STR_DialPrefix	, MUIA_ShortHelp, GetStr(MSG_Help_DialPrefix));
		set(tmp.STR_SerialDriver, MUIA_ShortHelp, GetStr(MSG_Help_SerialDriver));
		set(tmp.STR_SerialUnit	, MUIA_ShortHelp, GetStr(MSG_Help_SerialUnit));
		set(tmp.STR_BaudRate		, MUIA_ShortHelp, GetStr(MSG_Help_BaudRate));
		set(tmp.SL_RedialAttempts, MUIA_ShortHelp, GetStr(MSG_Help_RedialAttempts));
		set(tmp.SL_RedialDelay	, MUIA_ShortHelp, GetStr(MSG_Help_RedialDelay));
		set(tmp.CH_Carrier		, MUIA_ShortHelp, GetStr(MSG_Help_Carrier));
		set(tmp.CH_7Wire			, MUIA_ShortHelp, GetStr(MSG_Help_7Wire));
		set(tmp.CH_OwnDevUnit	, MUIA_ShortHelp, GetStr(MSG_Help_OwnDevUnit));

		DoMethod(tmp.LV_Modem		, MUIM_Notify, MUIA_Listview_DoubleClick	, MUIV_EveryTime , obj, 2, MUIM_Modem_PopString_Close, MUIV_Modem_PopString_Modem);
		DoMethod(tmp.LV_BaudRate	, MUIM_Notify, MUIA_Listview_DoubleClick	, MUIV_EveryTime , obj, 2, MUIM_Modem_PopString_Close, MUIV_Modem_PopString_BaudRate);
		DoMethod(tmp.LV_DialPrefix	, MUIM_Notify, MUIA_Listview_DoubleClick	, MUIV_EveryTime , obj, 2, MUIM_Modem_PopString_Close, MUIV_Modem_PopString_DialPrefix);
		DoMethod(tmp.LV_Devices		, MUIM_Notify, MUIA_Listview_DoubleClick	, MUIV_EveryTime , obj, 2, MUIM_Modem_PopString_Close, MUIV_Modem_PopString_Device);
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG Modem_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(Modem_New					(cl, obj, (APTR)msg));
		case MUIM_Modem_PopString_Close		: return(Modem_PopString_Close	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Paths class (GROUP)                                                      */
/****************************************************************************/

ULONG Paths_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct Paths_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		Child, ColGroup(2),
			GroupFrame,
			MUIA_Background, MUII_RegisterBack,
			Child, HVSpace,
			Child, HVSpace,
			Child, MakeKeyLabel2(MSG_LA_MailIn, MSG_CC_MailIn),
			Child, tmp.PA_MailIn = MakePopAsl(tmp.STR_MailIn = MakeKeyString("NetConnect:Data/MailIn", MAXPATHLEN, MSG_CC_MailIn), MSG_LA_MailIn, TRUE),
			Child, MakeKeyLabel2(MSG_LA_MailOut, MSG_CC_MailOut),
			Child, tmp.PA_MailOut = MakePopAsl(tmp.STR_MailOut = MakeKeyString("NetConnect:Data/MailOut", MAXPATHLEN, MSG_CC_MailOut), MSG_LA_MailOut, TRUE),
			Child, HVSpace,
			Child, HVSpace,
			Child, MakeKeyLabel2(MSG_LA_NewsIn, MSG_CC_NewsIn),
			Child, tmp.PA_NewsIn = MakePopAsl(tmp.STR_NewsIn = MakeKeyString("NetConnect:Data/NewsIn", MAXPATHLEN, MSG_CC_NewsIn), MSG_LA_NewsIn, TRUE),
			Child, MakeKeyLabel2(MSG_LA_NewsOut, MSG_CC_NewsOut),
			Child, tmp.PA_NewsOut = MakePopAsl(tmp.STR_NewsOut = MakeKeyString("NetConnect:Data/NewsOut", MAXPATHLEN, MSG_CC_NewsOut), MSG_LA_NewsOut, TRUE),
			Child, HVSpace,
			Child, HVSpace,
			Child, MakeKeyLabel2(MSG_LA_FilesIn, MSG_CC_FilesIn),
			Child, tmp.PA_FileIn = MakePopAsl(tmp.STR_FileIn = MakeKeyString("NetConnect:Data/Download", MAXPATHLEN, MSG_CC_FilesIn), MSG_LA_FilesIn, TRUE),
			Child, MakeKeyLabel2(MSG_LA_FilesOut, MSG_CC_FilesOut),
			Child, tmp.PA_FileOut = MakePopAsl(tmp.STR_FileOut = MakeKeyString("NetConnect:Data/Upload", MAXPATHLEN, MSG_CC_FilesOut), MSG_LA_FilesOut, TRUE),
			Child, HVSpace,
			Child, HVSpace,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct Paths_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		set(tmp.STR_MailIn	, MUIA_ShortHelp, GetStr(MSG_Help_MailIn));
		set(tmp.STR_MailOut	, MUIA_ShortHelp, GetStr(MSG_Help_MailOut));
		set(tmp.STR_NewsIn	, MUIA_ShortHelp, GetStr(MSG_Help_NewsIn));
		set(tmp.STR_NewsOut	, MUIA_ShortHelp, GetStr(MSG_Help_NewsOut));
		set(tmp.STR_FileIn	, MUIA_ShortHelp, GetStr(MSG_Help_FileIn));
		set(tmp.STR_FileOut	, MUIA_ShortHelp, GetStr(MSG_Help_FileOut));
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG Paths_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(Paths_New		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}
