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
		MUIA_Window_Title, "About",
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
				MUIA_Background, MUII_ReadListBack,
				MUIA_Scrollgroup_FreeHoriz, FALSE,
				MUIA_Scrollgroup_Contents, VirtgroupObject,
					ReadListFrame,
					Child, TextObject,
						MUIA_Text_Contents, "\33c\nAmiTCP Prefs 0.54 (03.08.96)\n© 1996 Michael Neuweiler\n\nAuthors:\nMichael Neuweiler\n...\n\n\n\33bDEMO\33n\n",
					End,
					Child, MUI_MakeObject(MUIO_HBar, 2),
					Child, TextObject,
						MUIA_Text_Contents, "\33c\n\AmiTCP Prefs uses MUI\nMUI is copyrighted by Stefan Stuntz",
					End,
				End,
			End,
			Child, HGroup,
				Child, HSpace(0),
				Child, tmp.BT_Button = MakeButton("  _Okay"),
				Child, HSpace(0),
			End,
		End))
	{
		struct About_Data *data = INST_DATA(cl,obj);

		*data = tmp;

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

ULONG InfoWindow_LoadFile(struct IClass *cl, Object *obj, struct MUIP_InfoWindow_LoadFile *msg)
{
	struct InfoWindow_Data *data = INST_DATA(cl, obj);
	struct InfoLine *info;
	struct pc_Data *pc_data;
	BOOL success = FALSE;

	set(data->LV_Info, MUIA_List_Quiet, TRUE);
	DoMethod(data->LV_Info, MUIM_List_Clear);
	if(msg->file)
	{
		if(info = (struct InfoLine *)AllocVec(sizeof(struct InfoLine), MEMF_ANY))
		{
			if(pc_data = AllocVec(sizeof(struct pc_Data), MEMF_ANY | MEMF_CLEAR))
			{
				if(ParseConfig(msg->file, pc_data))
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
	}
	set(data->LV_Info, MUIA_List_Quiet, FALSE);

	return((ULONG)success);
}

ULONG InfoWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	static const struct Hook InfoList_ConstructHook= { { 0,0 }, (VOID *)InfoList_ConstructFunc	, NULL, NULL };
	static const struct Hook InfoList_DestructHook	= { { 0,0 }, (VOID *)InfoList_DestructFunc	, NULL, NULL };
	static const struct Hook InfoList_DisplayHook	= { { 0,0 }, (VOID *)InfoList_DisplayFunc		, NULL, NULL };
	struct InfoWindow_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title, "Provider Information",
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
			break;
		case MUIV_Provider_PopString_Provider:
			list = data->LV_Provider;
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
			strcpy(path, "AmiTCP:Providers");
			AddPart(path, (STRPTR)xget(data->PO_Country, MUIA_Text_Contents), MAXPATHLEN);
			DoMethod(obj, MUIM_Provider_PopList_Update, path, MUIV_Provider_PopString_Provider);
			DoMethod(data->LV_PoP, MUIM_List_Clear);
			set(data->PO_Provider, MUIA_Text_Contents, "");
			set(data->PO_PoP, MUIA_Text_Contents, "");
			break;

		case MUIV_Provider_PopString_Provider:
			strcpy(path, "AmiTCP:Providers");
			AddPart(path, (STRPTR)xget(data->PO_Country	, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(path, (STRPTR)xget(data->PO_Provider	, MUIA_Text_Contents), MAXPATHLEN);
			DoMethod(obj, MUIM_Provider_PopList_Update, path, MUIV_Provider_PopString_PoP);
			set(data->PO_PoP, MUIA_Text_Contents, "");
			break;

		case MUIV_Provider_PopString_PoP:
			strcpy(path, "AmiTCP:Providers");
			AddPart(path, (STRPTR)xget(data->PO_Country	, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(path, (STRPTR)xget(data->PO_Provider	, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(path, (STRPTR)xget(data->PO_PoP		, MUIA_Text_Contents), MAXPATHLEN);
			if(get_file_size(path) == -2)
			{
				// the pop has got its own directory
Printf("pop uses its own config files in directory '%ls'.\n", path);
				DoMethod(win, MUIM_AmiTCPPrefs_LoadConfig, path);
			}
			else
			{
				struct PoP *pop;

				// the pop uses the provider's standard files
				strcpy(path, "AmiTCP:Providers");
				AddPart(path, (STRPTR)xget(data->PO_Country	, MUIA_Text_Contents), MAXPATHLEN);
				AddPart(path, (STRPTR)xget(data->PO_Provider	, MUIA_Text_Contents), MAXPATHLEN);
Printf("pop uses the provider's default config files in directory '%ls'.\n", path);
				DoMethod(win, MUIM_AmiTCPPrefs_LoadConfig, path);
				pop = (struct PoP *)x;
				setstring(data->STR_Phone, pop->Phone);
			}
			break;
	}

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
	STRPTR buf, ptr1, ptr2;

	switch(xget(data->CY_Authentication, MUIA_Cycle_Active))
	{
		case 0:
			set(data->STR_HostID, MUIA_Disabled		, TRUE);
			set(data->STR_YourID, MUIA_Disabled		, TRUE);
			set(data->STR_Password, MUIA_Disabled	, TRUE);
			break;
		case 1:
			set(data->STR_HostID, MUIA_Disabled		, FALSE);
			set(data->STR_YourID, MUIA_Disabled		, FALSE);
			set(data->STR_Password, MUIA_Disabled	, FALSE);
			if(buf = LoadFile("ENV:NetConfig/CHAP"))
			{
				if(ptr1 = strchr(buf, ' '))
				{
					*ptr1 = NULL;
					setstring(data->STR_HostID, buf);
					ptr2 = ptr1 + 1;
					if(ptr1 = strchr(ptr2, ' '))
					{
						*ptr1 = NULL;
						setstring(data->STR_Password, ptr2);
						ptr2 = ptr1 + 1;
						if(ptr1 = strchr(ptr2, '\n'))
						{
							*ptr1 = NULL;
							setstring(data->STR_YourID, ptr2);
						}
					}
				}
				FreeVec(buf);
			}
			else
			{
				struct AmiTCPPrefs_Data *xdata = INST_DATA(CL_AmiTCPPrefs->mcc_Class, win);
				struct User_Data *user_data = INST_DATA(CL_User->mcc_Class, xdata->GR_User);

				setstring(data->STR_Password, xget(user_data->STR_Password, MUIA_String_Contents));
				setstring(data->STR_YourID, xget(user_data->STR_Password, MUIA_String_Contents));
			}
			break;
		case 2:
			set(data->STR_HostID, MUIA_Disabled, TRUE);
			set(data->STR_YourID, MUIA_Disabled, FALSE);
			set(data->STR_Password, MUIA_Disabled, FALSE);
			if(buf = LoadFile("ENV:NetConfig/PAP"))
			{
				if(ptr1 = strchr(buf, ' '))
				{
					*ptr1 = NULL;
					setstring(data->STR_YourID, buf);
					ptr2 = ptr1 + 1;
					if(ptr1 = strchr(ptr2, '\n'))
					{
						*ptr1 = NULL;
						setstring(data->STR_Password, ptr2);
					}
				}
				FreeVec(buf);
			}
			else
			{
				struct AmiTCPPrefs_Data *xdata = INST_DATA(CL_AmiTCPPrefs->mcc_Class, win);
				struct User_Data *user_data = INST_DATA(CL_User->mcc_Class, xdata->GR_User);
				setstring(data->STR_Password, xget(user_data->STR_Password, MUIA_String_Contents));
				setstring(data->STR_YourID, xget(user_data->STR_Password, MUIA_String_Contents));
			}
			break;
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
	struct Provider_Data tmp;

	STR_GR_ProviderRegister[0] = "Provider";
	STR_GR_ProviderRegister[1] = "Protocol";
	STR_GR_ProviderRegister[2] = "Services";
	STR_GR_ProviderRegister[3] = "Dialscript";
	STR_GR_ProviderRegister[4] = NULL;

	STR_RA_Connection[0] = GetStr(MSG_RA_Connection0);
	STR_RA_Connection[1] = GetStr(MSG_RA_Connection1);
	STR_RA_Connection[2] = NULL;

	STR_RA_Interface[0] = "PPP";
	STR_RA_Interface[1] = "SLIP";
	STR_RA_Interface[2] = NULL;

	STR_CY_Authentication[0] = "None - Use PPP normally";
	STR_CY_Authentication[1] = "CHAP - Use PPP with CHAP";
	STR_CY_Authentication[2] = "PAP - Use PPP with PAP";
	STR_CY_Authentication[3] = NULL;

	STR_CY_Header[0] = "AUTO Header Compression";
	STR_CY_Header[1] = "Header Compression ON";
	STR_CY_Header[2] = "Header Compression OFF";
	STR_CY_Header[3] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_Provider",
		Child, tmp.GR_Register = RegisterObject,
			MUIA_Background	, MUII_RegisterBack,
			MUIA_Register_Titles	, STR_GR_ProviderRegister,
			MUIA_CycleChain		, 1,
			Child, VGroup,
				MUIA_HelpNode	, "GR_Provider",
				Child, HVSpace,
				Child, ColGroup(2),
					GroupFrame,
					Child, Label2(GetStr(MSG_LA_Country)),
					Child, tmp.PO_Country = PopobjectObject,
						MUIA_Popstring_String		, tmp.TX_Country = TextObject, TextFrame, End,
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
					Child, MakeKeyLabel2(MSG_LA_Phone, "  p"),
					Child, tmp.STR_Phone = MakeKeyString("", 80, "  p"),
					Child, MakeKeyLabel2("  Show Provider Info :", "  i"),
					Child, HGroup,
						Child, tmp.CH_ProviderInfo = KeyCheckMark(FALSE, 'i'),
						Child, HVSpace,
					End,
				End,
				Child, HVSpace,
			End,
			Child, VGroup,
				Child, HVSpace,
				Child, HGroup,
					MUIA_HelpNode	, "GR_Connection",
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
						Child, tmp.CH_BOOTP = KeyCheckMark(FALSE, 'b'),
						Child, MakeKeyLabel1("  \33lBOOTP", "  b"),
						Child, tmp.SL_MTU = NumericbuttonObject,
							MUIA_CycleChain		, 1,
							MUIA_ControlChar		, 't',
							MUIA_Numeric_Min		, 72,
							MUIA_Numeric_Max		, 1524,
							MUIA_Numeric_Value	, 1500,
						End,
						Child, MakeKeyLabel1("  \33lMTU", "  t"),
					End,
					Child, HVSpace,
				End,
				Child, HVSpace,
				Child, ColGroup(2),
					MUIA_HelpNode	, "GR_PPPOptions",
					GroupFrameT("Interface Options"),
					Child, MakeKeyLabel2("  Compression :", "  o"),
					Child, tmp.CY_Header = MakeKeyCycle(STR_CY_Header, "  o"),
					Child, MakeKeyLabel2("  Authentication :", "  a"),
					Child, tmp.CY_Authentication = MakeKeyCycle(STR_CY_Authentication, "  a"),
					Child, MakeKeyLabel2("  PoP's Host-ID :", "  h"),
					Child, tmp.STR_HostID = MakeKeyString("", 80, "  h"),
					Child, MakeKeyLabel2("  Your Host-ID :", "  y"),
					Child, tmp.STR_YourID = MakeKeyString("", 80, "  y"),
					Child, MakeKeyLabel2("  Password :", "  p"),
					Child, tmp.STR_Password = StringObject,
						MUIA_CycleChain		, 1,
						MUIA_ControlChar		, 'p',
						MUIA_Frame				, MUIV_Frame_String,
						MUIA_String_Secret	, TRUE,
					End,
				End,
				Child, HVSpace,
			End,
			Child, VGroup,
				Child, HVSpace,
				MUIA_HelpNode	, "GR_Services",
				Child, ColGroup(2),
					GroupFrame,
					Child, MakeKeyLabel2(MSG_LA_DomainName, "  d"),
					Child, tmp.STR_DomainName = MakeKeyString("", 80, "  d"),
					Child, MakeKeyLabel2(MSG_LA_NameServer1, "  1"),
					Child, tmp.STR_NameServer1 = StringObject,
						MUIA_ControlChar		, '1',
						MUIA_CycleChain		, 1,
						MUIA_Frame				, MUIV_Frame_String,
						MUIA_String_Accept	, "0123456789.",
						MUIA_String_MaxLen	, 18,
					End,
					Child, MakeKeyLabel2(MSG_LA_NameServer2, "  2"),
					Child, tmp.STR_NameServer2 = StringObject,
						MUIA_ControlChar		, '2',
						MUIA_CycleChain		, 1,
						MUIA_Frame				, MUIV_Frame_String,
						MUIA_String_Accept	, "0123456789.",
						MUIA_String_MaxLen	, 18,
					End,
				End,
				Child, HVSpace,
				Child, ColGroup(2),
					GroupFrame,
					Child, MakeKeyLabel2(MSG_LA_MailServer, "  m"),
					Child, tmp.STR_MailServer = MakeKeyString("", 80, "  m"),
					Child, MakeKeyLabel2("  POP3 Server :", "  p"),
					Child, tmp.STR_POPServer = MakeKeyString("", 80, "  p"),
					Child, MakeKeyLabel2(MSG_LA_NewsServer, "  n"),
					Child, tmp.STR_NewsServer = MakeKeyString("", 80, "  n"),
					Child, MakeKeyLabel2("  WWW Server :", "  w"),
					Child, tmp.STR_WWWServer = MakeKeyString("", 80, "  w"),
					Child, MakeKeyLabel2("  FTP Server :", "  f"),
					Child, tmp.STR_FTPServer = MakeKeyString("", 80, "  f"),
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
					Child, tmp.BT_New		= MakeButton("  _New"),
					Child, tmp.BT_Delete	= MakeButton("  _Delete"),
					Child, tmp.BT_Clear	= MakeButton("  C_lear"),
				End,
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct Provider_Data *data = INST_DATA(cl,obj);

		*data = tmp;

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

		set(tmp.TX_Country		, MUIA_ShortHelp, "Please select the country you live in.");
		set(tmp.TX_Provider		, MUIA_ShortHelp, "Select the provider you want to use");
		set(tmp.TX_PoP				, MUIA_ShortHelp, "Choose the Point of Presence of your provider");
		set(tmp.STR_Phone			, MUIA_ShortHelp, "Enter the phone number(s) of your provider.\nUse a space to separate phone numbers. i.e. \"2574011 2574012\"");
		set(tmp.CH_ProviderInfo	, MUIA_ShortHelp, "Opens a window which shows information about the Provider/PoP you have chosen");

		set(tmp.RA_Connection	, MUIA_ShortHelp, "Is your IP Adress static or dynamic ?\nIf it is static you have to set the IP Address in the \"User\" Page.");
		set(tmp.RA_Interface		, MUIA_ShortHelp, "Do you use ppp or slip as interface ?");
		set(tmp.CH_BOOTP			, MUIA_ShortHelp, "Does your provider offer BOOTP ?");
		set(tmp.SL_MTU				, MUIA_ShortHelp, "Set the Max Transfer Unit");
		set(tmp.CY_Header			, MUIA_ShortHelp, "Does your provider support Header Compression ?\nIf you don't know you could simply choose \"AUTO\"");
		set(tmp.CY_Authentication, MUIA_ShortHelp, "Does your provider want PAP or CHAP authentication ?");
		set(tmp.STR_HostID		, MUIA_ShortHelp, "Enter the ID of the providers host");
		set(tmp.STR_YourID		, MUIA_ShortHelp, "Enter your ID");
		set(tmp.STR_Password		, MUIA_ShortHelp, "Enter the password required for PAP/CHAP authentication.");

		set(tmp.STR_DomainName	, MUIA_ShortHelp, "The name of your domain.");
		set(tmp.STR_NameServer1	, MUIA_ShortHelp, "The IP Adress of your primary NameServer");
		set(tmp.STR_NameServer2	, MUIA_ShortHelp, "The IP Adress of your secondary NameServer");
		set(tmp.STR_MailServer	, MUIA_ShortHelp, "Enter the name of your providers Mail-Server");
		set(tmp.STR_POPServer	, MUIA_ShortHelp, "Enter the name of your providers POP3-Server");
		set(tmp.STR_NewsServer	, MUIA_ShortHelp, "Enter the name of your providers News-Server");
		set(tmp.STR_WWWServer	, MUIA_ShortHelp, "Enter the name of your providers WWW-Server");
		set(tmp.STR_FTPServer	, MUIA_ShortHelp, "Enter the name of your providers FTP-Server");

		set(tmp.LV_DialScript	, MUIA_ShortHelp, "This is the AREXX script that will be used to login at your provider.\nYou can use drag & drop to move lines in this list\n\nRemember: ARexx-Scripts DO have to\nstart with a comment in the first line !!");
		set(tmp.STR_Line			, MUIA_ShortHelp, "Edit the contents of the currently selected line here.\nYou can insert some predefined strings if you click on the button right to this field");
		set(tmp.BT_New				, MUIA_ShortHelp, "Add a new line at the end of the script.\nYou can use drag & drop to move the lines in the list");
		set(tmp.BT_Delete			, MUIA_ShortHelp, "Delete the active line.");
		set(tmp.BT_Clear			, MUIA_ShortHelp, "Clear the entire script.");


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
Printf("replacing %ls with %ls in the dialscript.\n", ptr, string);
			DoMethod(provider_data->LV_DialScript, MUIM_List_InsertSingle, string, i + 1);
			DoMethod(provider_data->LV_DialScript, MUIM_List_Remove, i);
		}
		if((STRPTR)strstr(ptr, "YourPassword=") == ptr)
		{
			sprintf(string, "YourPassword=\"%ls\"", xget(user_data->STR_Password, MUIA_String_Contents));
Printf("replacing %ls with %ls in the dialscript.\n", ptr, string);
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

	STR_GR_UserRegister[0] = "User Information";
	STR_GR_UserRegister[1] = "User StartNet";
	STR_GR_UserRegister[2] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_User",
		Child, tmp.GR_Register = RegisterObject,
			MUIA_Background		, MUII_RegisterBack,
			MUIA_Register_Titles	, STR_GR_UserRegister,
			MUIA_CycleChain		, 1,
			Child, ColGroup(2),
				GroupFrame,
				MUIA_HelpNode	, "GR_UserInformation",
				Child, HVSpace,
				Child, HVSpace,
				Child, MakeKeyLabel2(MSG_LA_LoginName, "  n"),
				Child, tmp.STR_LoginName = MakeKeyString("", 80, "  n"),
				Child, MakeKeyLabel2(MSG_LA_Password, "  p"),
				Child, tmp.STR_Password = StringObject,
					MUIA_ControlChar		, 'p',
					MUIA_CycleChain		, 1,
					MUIA_Frame				, MUIV_Frame_String,
					MUIA_String_Secret	, TRUE,
					MUIA_String_MaxLen	, 80,
				End,
				Child, HVSpace,
				Child, HVSpace,
				Child, MakeKeyLabel2("  EMail address :", "  e"),
				Child, tmp.STR_EMail = MakeKeyString("", 80, "  e"),
				Child, MakeKeyLabel2(MSG_LA_RealName, "  r"),
				Child, tmp.STR_RealName = MakeKeyString("", 80, "  r"),
				Child, MakeKeyLabel2(MSG_LA_Organisation, "  o"),
				Child, tmp.STR_Organisation = MakeKeyString("Private User", 80, "  o"),
				Child, MakeKeyLabel2(MSG_LA_HostName, "  h"),
				Child, tmp.STR_HostName = MakeKeyString("", 80, "  h"),
				Child, MakeKeyLabel2(MSG_LA_Address, "  a"),
				Child, tmp.STR_IP_Address = StringObject,
					MUIA_ControlChar		, 'a',
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
					Child, tmp.BT_New		= MakeButton("  _New"),
					Child, tmp.BT_Delete	= MakeButton("  _Delete"),
					Child, tmp.BT_Clear	= MakeButton("  C_lear"),
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


		set(tmp.STR_LoginName		, MUIA_ShortHelp, "Enter your login name here.\nIt will be used during logging in at your provider.");
		set(tmp.STR_Password		, MUIA_ShortHelp, "This is the password that will be used during the login procedure at your provider.");
		set(tmp.STR_EMail			, MUIA_ShortHelp, "Your EMail address.");
		set(tmp.STR_RealName		, MUIA_ShortHelp, "Your full name");
		set(tmp.STR_Organisation, MUIA_ShortHelp, "The name of your organisation or something like \"Private User\"");
		set(tmp.STR_HostName		, MUIA_ShortHelp, "The name of your computer.\nIf you use dynamic IP Adresses your provider probably won't give you a host name.");
		set(tmp.STR_IP_Address	, MUIA_ShortHelp, "If you use a static IP Address you have to enter it here.");
		set(tmp.LV_UserStartnet	, MUIA_ShortHelp, "This is the script that will be executed after the connection to your provider has been established. It can be used to start a httpd server for example.\nYou can use drag & drop to move lines in this list");
		set(tmp.STR_Line			, MUIA_ShortHelp, "Edit the contents of the currently selected line here.");
		set(tmp.BT_New				, MUIA_ShortHelp, "Add a new line at the end of the script.\nYou can use drag & drop to move the lines in the list");
		set(tmp.BT_Delete			, MUIA_ShortHelp, "Delete the active line.");
		set(tmp.BT_Clear			, MUIA_ShortHelp, "Clear the entire script.");

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

ULONG Modem_ModemList_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct Modem_Data *data = INST_DATA(cl, obj);
	struct Modem *modem = NULL;

	DoMethod(data->LV_Modem, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem);
	if(modem)
	{
		set(data->TX_Modem, MUIA_Text_Contents, modem->Name);
		setstring(data->STR_ModemInit, modem->InitString);
	}

	return(NULL);
}

ULONG Modem_PopString_Close(struct IClass *cl, Object *obj, struct MUIP_Modem_PopString_Close *msg)
{
	struct Modem_Data *data = INST_DATA(cl, obj);
	Object *list_view, *string;
	STRPTR x;

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
	}

	if(list_view && string)
	{
		DoMethod(list_view, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &x);
		if(x)
			setstring(string, x);
		DoMethod(string, MUIM_Popstring_Close, TRUE);
	}

	return(NULL);
}

ULONG Modem_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	static const struct Hook ModemList_ConstructHook= { { 0,0 }, (VOID *)ModemList_ConstructFunc	, NULL, NULL };
	static const struct Hook ModemList_DestructHook	= { { 0,0 }, (VOID *)ModemList_DestructFunc	, NULL, NULL };
	struct Modem_Data tmp;

	STR_GR_ModemRegister[0] = "Modem Settings";
	STR_GR_ModemRegister[1] = "Serial Settings";
	STR_GR_ModemRegister[2] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_Modem",
		Child, tmp.GR_Register = RegisterObject,
			MUIA_Background		, MUII_RegisterBack,
			MUIA_Register_Titles	, STR_GR_ModemRegister,
			MUIA_CycleChain		, 1,
			Child, VGroup,
				Child, VGroup,
					GroupFrameT("Modem Type"),
					MUIA_Group_Spacing, 0,
					Child, tmp.LV_Modem = ListviewObject,
						MUIA_CycleChain		, 1,
						MUIA_Listview_List			, tmp.LI_Modem = ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_ConstructHook	, &ModemList_ConstructHook,
							MUIA_List_DestructHook	, &ModemList_DestructHook,
							MUIA_List_CompareHook	, &sorthook,
							MUIA_List_AutoVisible	, TRUE,
						End,
					End,
					Child, tmp.TX_Modem = TextObject,
						TextFrame,
					End,
				End,
				Child, ColGroup(2),
					GroupFrameT("Modem Strings"),
					Child, MakeKeyLabel2(MSG_LA_ModemInit, "  i"),
					Child, tmp.STR_ModemInit = MakeKeyString("ATZ\\r", 80, "  i"),
					Child, MakeKeyLabel2(MSG_LA_DialPrefix, "  p"),
					Child, tmp.PO_DialPrefix = PopobjectObject,
						MUIA_Popstring_String		, tmp.STR_DialPrefix = MakeKeyString("ATDT", 80, "  p"),
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
					Child, MakeKeyLabel2(MSG_LA_DialSuffix, "  d"),
					Child, tmp.STR_DialSuffix = MakeKeyString("\\r", 80, "  d"),
				End,
			End,
			Child, VGroup,
				Child, HVSpace,
				Child, ColGroup(2),
					GroupFrame,
					Child, MakeKeyLabel2("  Device :", "  d"),
					Child, tmp.PA_SerialDriver = MakePopAsl(tmp.STR_SerialDriver = MakeKeyString("serial.device", MAXPATHLEN, "  d"), "  Device :", FALSE),
					Child, MakeKeyLabel2("  Unit :", "  n"),
					Child, tmp.STR_SerialUnit = StringObject,
						MUIA_ControlChar		, 'n',
						MUIA_CycleChain		, 1,
						StringFrame,
						MUIA_String_MaxLen	, 5,
						MUIA_String_Integer	, 0,
						MUIA_String_Accept	, "1234567890",
					End,
					Child, MakeKeyLabel2(MSG_LA_BaudRate, "  b"),
					Child, tmp.PO_BaudRate = PopobjectObject,
						MUIA_Popstring_String, tmp.STR_BaudRate = StringObject,
							MUIA_ControlChar		, 'b',
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
					Child, MakeKeyLabel2("  Redial Attempts :", "  r"),
					Child, tmp.SL_RedialAttempts = MakeKeySlider(0, 15, 15, "  r"),
				End,
				Child, HVSpace,
				Child, ColGroup(4),
					GroupFrame,
					Child, HVSpace,
					Child, MakeKeyLabel1("  Carrier Detect", "  a"),
					Child, tmp.CH_Carrier = KeyCheckMark(TRUE, 'a'),
					Child, HVSpace,
					Child, HVSpace,
					Child, MakeKeyLabel1("  Hardware handshake (RTS/CTS)", "  h"),
					Child, tmp.CH_7Wire = KeyCheckMark(TRUE, 'h'),
					Child, HVSpace,
					Child, HVSpace,
					Child, MakeKeyLabel1("  Use OwnDevUnit", "  o"),
					Child, tmp.CH_OwnDevUnit = KeyCheckMark(FALSE, 'o'),
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

		set(tmp.LV_Modem			, MUIA_ShortHelp, "Select your modem");
		set(tmp.STR_ModemInit	, MUIA_ShortHelp, "Enter the initialisation string of your modem");
		set(tmp.STR_DialPrefix	, MUIA_ShortHelp, "The dial prefix for your modem");
		set(tmp.STR_DialSuffix	, MUIA_ShortHelp, "The dial suffix for your modem");
		set(tmp.STR_SerialDriver, MUIA_ShortHelp, "Enter the name of your serial driver. (case sensitive !)");
		set(tmp.STR_SerialUnit	, MUIA_ShortHelp, "Enter the unit number you want to use for your serial driver.");
		set(tmp.STR_BaudRate		, MUIA_ShortHelp, "Enter the transfer speed between your Amiga and your Modem. (should be higher than the modem's max. connection speed)");
		set(tmp.SL_RedialAttempts, MUIA_ShortHelp, "Howmany times should the dialer try to establish a connection ?");
		set(tmp.CH_Carrier		, MUIA_ShortHelp, "Shall the network be stopped if the modem drops the carrier ?");
		set(tmp.CH_7Wire			, MUIA_ShortHelp, "If you use baud rates higher than 9600 you should set this.");
		set(tmp.CH_OwnDevUnit	, MUIA_ShortHelp, "Use OwnDevUnit to access the serial device.");

		DoMethod(tmp.LV_Modem		, MUIM_Notify, MUIA_List_Active				, MUIV_EveryTime , obj, 1, MUIM_Modem_ModemList_Active);
		DoMethod(tmp.LV_BaudRate	, MUIM_Notify, MUIA_Listview_DoubleClick	, MUIV_EveryTime , obj, 2, MUIM_Modem_PopString_Close, MUIV_Modem_PopString_BaudRate);
		DoMethod(tmp.LV_DialPrefix	, MUIM_Notify, MUIA_Listview_DoubleClick	, MUIV_EveryTime , obj, 2, MUIM_Modem_PopString_Close, MUIV_Modem_PopString_DialPrefix);
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG Modem_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(Modem_New					(cl, obj, (APTR)msg));
		case MUIM_Modem_ModemList_Active	: return(Modem_ModemList_Active	(cl, obj, (APTR)msg));
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
		MUIA_HelpNode, "GR_Paths",
		Child, ColGroup(2),
			GroupFrame,
			MUIA_Background, MUII_RegisterBack,
			Child, HVSpace,
			Child, HVSpace,
			Child, MakeKeyLabel2(MSG_LA_MailIn, "  m"),
			Child, tmp.PA_MailIn = MakePopAsl(tmp.STR_MailIn = MakeKeyString("NetConnect:Data/MailIn", MAXPATHLEN, "  m"), MSG_LA_MailIn, TRUE),
			Child, MakeKeyLabel2(MSG_LA_MailOut, "  a"),
			Child, tmp.PA_MailOut = MakePopAsl(tmp.STR_MailOut = MakeKeyString("NetConnect:Data/MailOut", MAXPATHLEN, "  a"), MSG_LA_MailOut, TRUE),
			Child, HVSpace,
			Child, HVSpace,
			Child, MakeKeyLabel2(MSG_LA_NewsIn, "  n"),
			Child, tmp.PA_NewsIn = MakePopAsl(tmp.STR_NewsIn = MakeKeyString("NetConnect:Data/NewsIn", MAXPATHLEN, "  n"), MSG_LA_NewsIn, TRUE),
			Child, MakeKeyLabel2(MSG_LA_NewsOut, "  e"),
			Child, tmp.PA_NewsOut = MakePopAsl(tmp.STR_NewsOut = MakeKeyString("NetConnect:Data/NewsOut", MAXPATHLEN, "  e"), MSG_LA_NewsOut, TRUE),
			Child, HVSpace,
			Child, HVSpace,
			Child, MakeKeyLabel2(MSG_LA_FilesIn, "  f"),
			Child, tmp.PA_FileIn = MakePopAsl(tmp.STR_FileIn = MakeKeyString("NetConnect:Data/Download", MAXPATHLEN, "  f"), MSG_LA_FilesIn, TRUE),
			Child, MakeKeyLabel2(MSG_LA_FilesOut, "  i"),
			Child, tmp.PA_FileOut = MakePopAsl(tmp.STR_FileOut = MakeKeyString("NetConnect:Data/Upload", MAXPATHLEN, "  i"), MSG_LA_FilesOut, TRUE),
			Child, HVSpace,
			Child, HVSpace,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct Paths_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		set(tmp.STR_MailIn	, MUIA_ShortHelp, "Where does incoming mail go to.");
		set(tmp.STR_MailOut	, MUIA_ShortHelp, "Here we store outgoing mails.");
		set(tmp.STR_NewsIn	, MUIA_ShortHelp, "Where the news go in");
		set(tmp.STR_NewsOut	, MUIA_ShortHelp, "Drawer for outgoing news");
		set(tmp.STR_FileIn	, MUIA_ShortHelp, "Drawer to download files to.");
		set(tmp.STR_FileOut	, MUIA_ShortHelp, "Drawer to upload files from.");
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
