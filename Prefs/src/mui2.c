#include "globals.c"
#include "protos.h"

/****************************************************************************/
/* About class                                                              */
/****************************************************************************/

ULONG About_New(struct IClass *cl, Object *obj, Msg msg)
{
	struct About_Data tmp;
#ifdef DEMO
	Object *BT_Update = NULL;
#endif

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
						MUIA_Text_PreParse, "\033b\033c\n",
						MUIA_Text_Contents, GetStr(MSG_TX_DemoWarning),
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
#ifdef DEMO
				Child, tmp.BT_Button = MakeButton("  Continue"),
				Child, HVSpace,
				Child, BT_Update = MakeButton("  \033bUPDATE"),
#else
				Child, HSpace(0),
				Child, tmp.BT_Button = MakeButton(MSG_BT_Okay),
				Child, HSpace(0),
#endif
			End,
		End))
	{
		struct About_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		set(tmp.BT_Button, MUIA_CycleChain, 1);
		set(obj, MUIA_Window_ActiveObject, tmp.BT_Button);

		DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE ,
			MUIV_Notify_Application, 6, MUIM_Application_PushMethod,
			win, 3, MUIM_AmiTCPPrefs_About_Finish, obj, 0);
		DoMethod(data->BT_Button, MUIM_Notify, MUIA_Pressed, FALSE ,
			MUIV_Notify_Application, 6, MUIM_Application_PushMethod,
			win, 3, MUIM_AmiTCPPrefs_About_Finish, obj, 0);
#ifdef DEMO
		DoMethod(BT_Update, MUIM_Notify, MUIA_Pressed, FALSE,
			MUIV_Notify_Application, 6, MUIM_Application_PushMethod,
			win, 3, MUIM_AmiTCPPrefs_About_Finish, obj, 1);
		set(obj, MUIA_Window_ActiveObject, BT_Update);
#endif
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

SAVEDS ASM LONG InfoList_DisplayFunc(REG(a2) char **array, REG(a1) struct InfoLine *info)
{
	if(info)
	{
		*array++	= info->Label;
		*array	= info->Contents;
	}
	return(NULL);
}

ULONG InfoWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	static const struct Hook InfoList_ConstructHook= { { 0,0 }, (VOID *)InfoList_ConstructFunc	, NULL, NULL };
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
					MUIA_List_DestructHook	, &des_hook,
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
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Provider class (GROUP)                                                   */
/****************************************************************************/

ULONG Provider_PopList_Update(struct IClass *cl, Object *obj, struct MUIP_Provider_PopList_Update *msg)
{
	struct Provider_Data *data = INST_DATA(cl, obj);
	struct AmiTCPPrefs_Data *atcp_data = INST_DATA(CL_AmiTCPPrefs->mcc_Class, win);
	struct InfoWindow_Data	*info_data	= INST_DATA(CL_InfoWindow->mcc_Class, atcp_data->GR_InfoWindow);
	Object *list;

	switch(msg->flags)
	{
		case MUIV_Provider_PopString_Country:
			list = data->LV_Country;
			DoMethod(data->LV_Provider	, MUIM_List_Clear);
			DoMethod(data->LV_PoP		, MUIM_List_Clear);
			set(data->PO_Provider		, MUIA_Text_Contents, "");
			set(data->PO_PoP				, MUIA_Text_Contents, "");
			set(data->CH_ProviderInfo, MUIA_Selected, FALSE);
			set(data->CH_ProviderInfo, MUIA_Disabled, TRUE);
			DoMethod(info_data->LV_Info, MUIM_List_Clear);
			break;
		case MUIV_Provider_PopString_Provider:
			list = data->LV_Provider;
			DoMethod(data->LV_PoP		, MUIM_List_Clear);
			set(data->PO_PoP				, MUIA_Text_Contents, "");
			set(data->CH_ProviderInfo, MUIA_Selected, FALSE);
			set(data->CH_ProviderInfo, MUIA_Disabled, TRUE);
			DoMethod(info_data->LV_Info, MUIM_List_Clear);
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
		struct InfoLine *info;

		if(ParseConfig(msg->path, &pc_data))
		{
			if(pop = (struct PoP *)AllocVec(sizeof(struct PoP), MEMF_ANY))
			{
				if(info = (struct InfoLine *)AllocVec(sizeof(struct InfoLine), MEMF_ANY))
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
								{
									strncpy(pop->Name, pc_data.Argument, 80);
									strncpy(pop->Phone, pc_data.Contents, 80);
									DoMethod(list, MUIM_List_InsertSingle, pop, MUIV_List_Insert_Sorted);
								}
							}
						}
						if(*pc_data.Argument == '#' && !stricmp(pc_data.Contents, "Provider.txt"))
						{
							while(ParseNext(&pc_data))
							{
								if(*pc_data.Argument == '#')
									break;

								strncpy(info->Label, pc_data.Argument, 40);
								strncpy(info->Contents, pc_data.Contents, 80);
								DoMethod(info_data->LV_Info, MUIM_List_InsertSingle, info, MUIV_List_Insert_Bottom);
							}
							set(data->CH_ProviderInfo, MUIA_Disabled, FALSE);
						}
					}
					FreeVec(info);
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
						if((fib->fib_DirEntryType > 0 && msg->flags == MUIV_Provider_PopString_Country) || (fib->fib_DirEntryType < 0 && msg->flags == MUIV_Provider_PopString_Provider))
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
			nnset(data->PO_Provider, MUIA_Text_Contents, GetStr(MSG_TX_SelectProvider));
			break;

		case MUIV_Provider_PopString_Provider:
			strcpy(path, "NetConnect:Data/Providers");
			AddPart(path, (STRPTR)xget(data->PO_Country	, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(path, (STRPTR)xget(data->PO_Provider	, MUIA_Text_Contents), MAXPATHLEN);
			DoMethod(obj, MUIM_Provider_PopList_Update, path, MUIV_Provider_PopString_PoP);
			nnset(data->PO_PoP, MUIA_Text_Contents, GetStr(MSG_TX_SelectPoP));
			break;

		case MUIV_Provider_PopString_PoP:
			strcpy(path, "NetConnect:Data/Providers");
			AddPart(path, (STRPTR)xget(data->PO_Country	, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(path, (STRPTR)xget(data->PO_Provider	, MUIA_Text_Contents), MAXPATHLEN);
			DoMethod(win, MUIM_AmiTCPPrefs_LoadProvider, path, xget(data->PO_PoP, MUIA_Text_Contents));
			break;
	}

	return(NULL);
}

ULONG Provider_Authentication_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct Provider_Data *data = INST_DATA(cl, obj);

	if(xget(data->CY_Authentication, MUIA_Cycle_Active))
		set(data->GR_LoginScript, MUIA_Disabled, TRUE);
	else
		set(data->GR_LoginScript, MUIA_Disabled, FALSE);

	return(NULL);
}

ULONG Provider_Interface_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct Provider_Data *data = INST_DATA(cl, obj);

	switch(xget(data->CY_Interface, MUIA_Cycle_Active))
	{
		case 0:
			set(data->CY_Authentication, MUIA_Disabled, FALSE);
			DoMethod(obj, MUIM_Provider_Authentication_Active);
			break;
		case 1:
			set(data->CY_Authentication, MUIA_Cycle_Active, 0);
			set(data->CY_Authentication, MUIA_Disabled, TRUE);
			set(data->GR_LoginScript, MUIA_Disabled, FALSE);
			break;
	}

	return(NULL);
}

ULONG Provider_ChangeAction(struct IClass *cl, Object *obj, struct MUIP_Provider_ChangeAction *msg)
{
	struct Provider_Data *data = INST_DATA(cl, obj);

	switch(xget(data->CY_Action[msg->which], MUIA_Cycle_Active))
	{
		case 0:
			set(data->STR_Line[msg->which], MUIA_Disabled, FALSE);
			set(data->CH_CR[msg->which], MUIA_Selected, FALSE);
			break;
		case 1:
			set(data->STR_Line[msg->which], MUIA_Disabled, FALSE);
			set(data->CH_CR[msg->which], MUIA_Selected, TRUE);
			break;
		case 2:
			set(data->STR_Line[msg->which], MUIA_Disabled, TRUE);
			set(data->CH_CR[msg->which], MUIA_Selected, TRUE);
			break;
		case 3:
			set(data->STR_Line[msg->which], MUIA_Disabled, TRUE);
			set(data->CH_CR[msg->which], MUIA_Selected, TRUE);
			break;
	}
	return(NULL);
}


#ifdef NETCOM
ULONG Provider_Reset(struct IClass *cl, Object *obj, Msg msg)
{
	struct Provider_Data *data = INST_DATA(cl, obj);

	set(data->STR_Phone			, MUIA_String_Contents, "08450798888");
	set(data->STR_DomainName	, MUIA_String_Contents, "netcomuk.co.uk");
	set(data->STR_NameServer1	, MUIA_String_Contents, "194.42.224.130");
	set(data->STR_NameServer2	, MUIA_String_Contents, "194.42.224.131");
	set(data->CY_Authentication, MUIA_Cycle_Active, 2);
	set(data->STR_MailServer	, MUIA_String_Contents, "smtp.netcomuk.co.uk");
	set(data->STR_POPServer		, MUIA_String_Contents, "popd.netcomuk.co.uk");
	set(data->STR_NewsServer	, MUIA_String_Contents, "nntp.netcomuk.co.uk");
	set(data->STR_WWWServer		, MUIA_String_Contents, "www.netcom.net.uk");
	set(data->STR_FTPServer		, MUIA_String_Contents, "ftp.netcom.net.uk");
	set(data->STR_IRCServer		, MUIA_String_Contents, "irc.netcomuk.co.uk");
	set(data->STR_ProxyServer	, MUIA_String_Contents, "www-cache.netcomuk.co.uk");
	set(data->STR_ProxyPort		, MUIA_String_Integer, 8080);
	set(data->STR_TimeServer	, MUIA_String_Contents, "ntp.netcomuk.co.uk");

	return(NULL);
}
#endif

SAVEDS ASM struct PoP *PoP_ConstructFunc(REG(a2) APTR pool, REG(a1) struct PoP *src)
{
	struct PoP *new;

	if((new = (struct PoP *)AllocVec(sizeof(struct PoP), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct PoP));
	return(new);
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
	static const struct Hook PoP_DisplayHook		= { { 0,0 }, (VOID *)PoP_DisplayFunc	, NULL, NULL };
	static STRPTR STR_CY_Action[] = { "WaitFor", "Send", "Send Username", "Send Password", NULL };
	static STRPTR STR_GR_ProviderRegister[6];
	static STRPTR STR_CY_Address[3];
	static STRPTR STR_CY_Interface[3];
	static STRPTR STR_CY_Authentication[4];
	static STRPTR STR_CY_Header[4];
	struct Provider_Data tmp;

	STR_GR_ProviderRegister[0] = GetStr(MSG_ProviderRegister1);
	STR_GR_ProviderRegister[1] = GetStr(MSG_ProviderRegister2);
	STR_GR_ProviderRegister[2] = GetStr(MSG_ProviderRegister3);
	STR_GR_ProviderRegister[3] = GetStr(MSG_ProviderRegister4);
	STR_GR_ProviderRegister[4] = GetStr(MSG_ProviderRegister5);
	STR_GR_ProviderRegister[5] = NULL;

	STR_CY_Address[0] = GetStr(MSG_CY_Address0);
	STR_CY_Address[1] = GetStr(MSG_CY_Address1);
	STR_CY_Address[2] = NULL;

	STR_CY_Interface[0] = "ppp";
	STR_CY_Interface[1] = "slip";
	STR_CY_Interface[2] = NULL;

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
			Child, ColGroup(2),
				Child, VVSpace,
				Child, HVSpace,
				Child, VVSpace,
				Child, HVSpace,
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
							MUIA_List_DestructHook	, &des_hook,
							MUIA_List_DisplayHook	, &PoP_DisplayHook,
							MUIA_List_CompareHook	, &sorthook,
							MUIA_List_Format			, "BAR,",
							MUIA_List_AutoVisible	, TRUE,
						End,
					End,
				End,
				Child, VVSpace,
				Child, HVSpace,
				Child, MakeKeyLabel2(MSG_LA_Phone, MSG_CC_Phone),
				Child, tmp.STR_Phone = MakeKeyString("", 80, MSG_CC_Phone),
				Child, MakeKeyLabel2(MSG_LA_ShowProviderInfo, MSG_CC_ShowProviderInfo),
				Child, HGroup,
					Child, tmp.CH_ProviderInfo = KeyCheckMark(FALSE, *GetStr(MSG_CC_ShowProviderInfo)),
					Child, HVSpace,
				End,
				Child, VVSpace,
				Child, HVSpace,
				Child, VVSpace,
				Child, HVSpace,
			End,
			Child, ColGroup(2),
				Child, VVSpace,
				Child, HVSpace,
				Child, VVSpace,
				Child, HVSpace,
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
				Child, MakeKeyLabel2(MSG_LA_ProxyServer, MSG_CC_ProxyServer),
				Child, HGroup,
					Child, tmp.STR_ProxyServer = MakeKeyString("", 80, MSG_CC_ProxyServer),
					Child, MakeKeyLabel2(MSG_LA_ProxyPort, MSG_CC_ProxyPort),
					Child, tmp.STR_ProxyPort = StringObject,
						MUIA_Weight				, 50,
						MUIA_ControlChar		, *GetStr(MSG_CC_ProxyPort),
						MUIA_CycleChain		, 1,
						MUIA_Frame				, MUIV_Frame_String,
						MUIA_String_Accept	, "0123456789",
						MUIA_String_MaxLen	, 10,
					End,
				End,
				Child, VVSpace,
				Child, HVSpace,
				Child, MakeKeyLabel2(MSG_LA_HostName, MSG_CC_HostName),
				Child, tmp.STR_HostName = MakeKeyString("", 80, MSG_CC_HostName),
				Child, MakeKeyLabel2(MSG_LA_Address, MSG_CC_Address),
				Child, HGroup,
					Child, tmp.STR_IP_Address = StringObject,
						MUIA_ControlChar		, *GetStr(MSG_CC_Address),
						MUIA_CycleChain		, 1,
						MUIA_Frame				, MUIV_Frame_String,
						MUIA_String_Contents	, "0.0.0.0",
						MUIA_String_Accept	, "0123456789.",
						MUIA_String_MaxLen	, 18,
					End,
					Child, tmp.CY_Address = Cycle(STR_CY_Address),
				End,
				Child, VVSpace,
				Child, HVSpace,
				Child, VVSpace,
				Child, HVSpace,
			End,
			Child, ColGroup(2),
				Child, MakeKeyLabel2(MSG_LA_Interface, MSG_CC_Interface),
				Child, tmp.CY_Interface = MakeKeyCycle(STR_CY_Interface, MSG_CC_Interface),
				Child, MakeKeyLabel2(MSG_LA_Compression, MSG_CC_Compression),
				Child, tmp.CY_Header = MakeKeyCycle(STR_CY_Header, MSG_CC_Compression),
				Child, MakeKeyLabel2(MSG_LA_Authentication, MSG_CC_Authentication),
				Child, tmp.CY_Authentication = MakeKeyCycle(STR_CY_Authentication, MSG_CC_Authentication),
				Child, MakeKeyLabel2(MSG_LA_MTU, MSG_CC_MTU),
				Child, HGroup,
					Child, tmp.SL_MTU = NumericbuttonObject,
						MUIA_CycleChain		, 1,
						MUIA_ControlChar		, *GetStr(MSG_CC_MTU),
						MUIA_Numeric_Min		, 72,
						MUIA_Numeric_Max		, 1524,
						MUIA_Numeric_Value	, 1500,
					End,
					Child, HVSpace,
				End,
				Child, MakeKeyLabel2(MSG_LA_BOOTP, MSG_CC_BOOTP),
				Child, HGroup,
					Child, tmp.CH_BOOTP = KeyCheckMark(FALSE, *GetStr(MSG_CC_BOOTP)),
					Child, HVSpace,
				End,
			End,
			Child, VGroup,
				Child, HVSpace,
				Child, ColGroup(2),
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
					Child, MakeKeyLabel2(MSG_LA_TimeServer, MSG_CC_TimeServer),
					Child, tmp.STR_TimeServer = MakeKeyString("", 80, MSG_CC_TimeServer),
					Child, MakeKeyLabel2(MSG_LA_IRCServer, MSG_CC_IRCServer),
					Child, HGroup,
						Child, tmp.STR_IRCServer = MakeKeyString("", 80, MSG_CC_IRCServer),
						Child, MakeKeyLabel2(MSG_LA_IRCPort, MSG_CC_IRCPort),
						Child, tmp.STR_IRCPort = StringObject,
							MUIA_Weight				, 50,
							MUIA_ControlChar		, *GetStr(MSG_CC_IRCPort),
							MUIA_CycleChain		, 1,
							MUIA_Frame				, MUIV_Frame_String,
							MUIA_String_Accept	, "0123456789",
							MUIA_String_MaxLen	, 10,
						End,
					End,
				End,
				Child, HVSpace,
			End,
			Child, tmp.GR_LoginScript = ColGroup(3),
				Child, CLabel(GetStr(MSG_LA_Action)),
				Child, CLabel(GetStr(MSG_LA_String)),
				Child, CLabel(GetStr(MSG_LA_CR)),
				Child, tmp.CY_Action[0]	= Cycle(STR_CY_Action),
				Child, tmp.STR_Line[0]	= String("", 80),
				Child, tmp.CH_CR[0]		= CheckMark(FALSE),
				Child, tmp.CY_Action[1]	= Cycle(STR_CY_Action),
				Child, tmp.STR_Line[1]	= String("", 80),
				Child, tmp.CH_CR[1]		= CheckMark(FALSE),
				Child, tmp.CY_Action[2]	= Cycle(STR_CY_Action),
				Child, tmp.STR_Line[2]	= String("", 80),
				Child, tmp.CH_CR[2]		= CheckMark(FALSE),
				Child, tmp.CY_Action[3]	= Cycle(STR_CY_Action),
				Child, tmp.STR_Line[3]	= String("", 80),
				Child, tmp.CH_CR[3]		= CheckMark(FALSE),
				Child, tmp.CY_Action[4]	= Cycle(STR_CY_Action),
				Child, tmp.STR_Line[4]	= String("", 80),
				Child, tmp.CH_CR[4]		= CheckMark(FALSE),
				Child, tmp.CY_Action[5]	= Cycle(STR_CY_Action),
				Child, tmp.STR_Line[5]	= String("", 80),
				Child, tmp.CH_CR[5]		= CheckMark(FALSE),
				Child, tmp.CY_Action[6]	= Cycle(STR_CY_Action),
				Child, tmp.STR_Line[6]	= String("", 80),
				Child, tmp.CH_CR[6]		= CheckMark(FALSE),
				Child, tmp.CY_Action[7]	= Cycle(STR_CY_Action),
				Child, tmp.STR_Line[7]	= String("", 80),
				Child, tmp.CH_CR[7]		= CheckMark(FALSE),
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct Provider_Data *data = INST_DATA(cl,obj);
		int i;

		*data = tmp;

		set(tmp.CH_ProviderInfo	, MUIA_Disabled, TRUE);
		set(tmp.STR_IP_Address	, MUIA_Disabled, TRUE);
		set(tmp.CY_Address		, MUIA_Weight, 5);

		set(tmp.CH_ProviderInfo	, MUIA_CycleChain, 1);
		set(tmp.CH_BOOTP			, MUIA_CycleChain, 1);
		set(tmp.CY_Address		, MUIA_CycleChain, 1);

		set(tmp.TX_Country		, MUIA_ShortHelp, GetStr(MSG_Help_Country));
		set(tmp.TX_Provider		, MUIA_ShortHelp, GetStr(MSG_Help_Provider));
		set(tmp.TX_PoP				, MUIA_ShortHelp, GetStr(MSG_Help_PoP));
		set(tmp.STR_Phone			, MUIA_ShortHelp, GetStr(MSG_Help_Phone));
		set(tmp.CH_ProviderInfo	, MUIA_ShortHelp, GetStr(MSG_Help_ProviderInfo));

		set(tmp.STR_DomainName	, MUIA_ShortHelp, GetStr(MSG_Help_DomainName));
		set(tmp.STR_NameServer1	, MUIA_ShortHelp, GetStr(MSG_Help_NameServer1));
		set(tmp.STR_NameServer2	, MUIA_ShortHelp, GetStr(MSG_Help_NameServer2));
		set(tmp.STR_ProxyServer	, MUIA_ShortHelp, GetStr(MSG_Help_ProxyServer));
		set(tmp.STR_ProxyPort	, MUIA_ShortHelp, GetStr(MSG_Help_ProxyPort));
		set(tmp.STR_HostName		, MUIA_ShortHelp, GetStr(MSG_Help_HostName));
		set(tmp.STR_IP_Address	, MUIA_ShortHelp, GetStr(MSG_Help_IP_Address));
		set(tmp.CY_Address		, MUIA_ShortHelp, GetStr(MSG_Help_Address));
		set(tmp.CH_BOOTP			, MUIA_ShortHelp, GetStr(MSG_Help_BOOTP));

		set(tmp.CY_Interface		, MUIA_ShortHelp, GetStr(MSG_Help_Interface));
		set(tmp.CY_Header			, MUIA_ShortHelp, GetStr(MSG_Help_Header));
		set(tmp.CY_Authentication, MUIA_ShortHelp,GetStr(MSG_Help_Authentication));
		set(tmp.SL_MTU				, MUIA_ShortHelp, GetStr(MSG_Help_MTU));

		set(tmp.STR_MailServer	, MUIA_ShortHelp, GetStr(MSG_Help_MailServer));
		set(tmp.STR_POPServer	, MUIA_ShortHelp, GetStr(MSG_Help_POPServer));
		set(tmp.STR_NewsServer	, MUIA_ShortHelp, GetStr(MSG_Help_NewsServer));
		set(tmp.STR_WWWServer	, MUIA_ShortHelp, GetStr(MSG_Help_WWWServer));
		set(tmp.STR_FTPServer	, MUIA_ShortHelp, GetStr(MSG_Help_FTPServer));
		set(tmp.STR_IRCServer	, MUIA_ShortHelp, GetStr(MSG_Help_IRCServer));
		set(tmp.STR_IRCPort		, MUIA_ShortHelp, GetStr(MSG_Help_IRCPort));
		set(tmp.STR_TimeServer	, MUIA_ShortHelp, GetStr(MSG_Help_TimeServer));


		DoMethod(tmp.LV_Country	, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime	, obj, 2, MUIM_Provider_PopString_Close, MUIV_Provider_PopString_Country);
		DoMethod(tmp.LV_Provider, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime	, obj, 2, MUIM_Provider_PopString_Close, MUIV_Provider_PopString_Provider);
		DoMethod(tmp.LV_PoP		, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime	, obj, 2, MUIM_Provider_PopString_Close, MUIV_Provider_PopString_PoP);

		DoMethod(tmp.CY_Interface, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 1, MUIM_Provider_Interface_Active);
		DoMethod(tmp.CY_Authentication, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 1, MUIM_Provider_Authentication_Active);
		DoMethod(tmp.CY_Address	, MUIM_Notify, MUIA_Cycle_Active			, MUIV_EveryTime	, tmp.STR_IP_Address		, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

		for(i = 0; i < 8; i++)
		{
			set(tmp.CY_Action[i]	, MUIA_Weight, 10);

			set(tmp.CY_Action[i]	, MUIA_CycleChain, 1);
			set(tmp.STR_Line[i]	, MUIA_CycleChain, 1);
			set(tmp.CH_CR[i]		, MUIA_CycleChain, 1);

			set(tmp.CY_Action[i]	, MUIA_ShortHelp, GetStr(MSG_Help_Action));
			set(tmp.STR_Line[i]	, MUIA_ShortHelp, GetStr(MSG_Help_String));
			set(tmp.CH_CR[i]		, MUIA_ShortHelp, GetStr(MSG_Help_CR));

			DoMethod(tmp.CY_Action[i], MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 2, MUIM_Provider_ChangeAction, i);
		}

#ifdef NETCOM
		DoMethod(tmp.STR_Phone			, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.STR_DomainName	, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.STR_NameServer1	, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.STR_NameServer2	, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.CY_Authentication, MUIM_Notify, MUIA_Cycle_Active		, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.STR_MailServer	, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.STR_POPServer		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.STR_NewsServer	, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.STR_WWWServer		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.STR_FTPServer		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.STR_IRCServer		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.STR_ProxyServer	, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.STR_ProxyPort		, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
		DoMethod(tmp.STR_TimeServer	, MUIM_Notify, MUIA_String_Contents	, MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
#endif

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
		case MUIM_Provider_ChangeAction					: return(Provider_ChangeAction					(cl, obj, (APTR)msg));
		case MUIM_Provider_Interface_Active				: return(Provider_Interface_Active				(cl, obj, (APTR)msg));
		case MUIM_Provider_Authentication_Active		: return(Provider_Authentication_Active		(cl, obj, (APTR)msg));
#ifdef NETCOM
		case MUIM_Provider_Reset							: return(Provider_Reset								(cl, obj, (APTR)msg));
#endif
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
		set(data->BT_Remove, MUIA_Disabled, FALSE);
		setstring(data->STR_Line, ptr);
	}
	else
	{
		set(data->STR_Line, MUIA_Disabled, TRUE);
		set(data->BT_Remove, MUIA_Disabled, TRUE);
		setstring(data->STR_Line, "");
	}

	return(NULL);
}

ULONG User_UserStopnetList_Active(struct IClass *cl, Object *obj, Msg msg)
{
	struct User_Data *data = INST_DATA(cl, obj);
	STRPTR ptr;

	DoMethod(data->LV_UserStopnet, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
	if(ptr)
	{
		set(data->STR_StopnetLine, MUIA_Disabled, FALSE);
		set(data->BT_StopnetRemove, MUIA_Disabled, FALSE);
		setstring(data->STR_StopnetLine, ptr);
	}
	else
	{
		set(data->STR_StopnetLine, MUIA_Disabled, TRUE);
		set(data->BT_StopnetRemove, MUIA_Disabled, TRUE);
		setstring(data->STR_StopnetLine, "");
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

ULONG User_StopnetChangeLine(struct IClass *cl, Object *obj, Msg msg)
{
	struct User_Data *data = INST_DATA(cl, obj);
	LONG i;

	i = xget(data->LV_UserStopnet, MUIA_List_Active);
	if(i != MUIV_List_Active_Off)
	{
		set(data->LV_UserStopnet, MUIA_List_Quiet, TRUE);
		DoMethod(data->LV_UserStopnet, MUIM_List_InsertSingle, xget(data->STR_StopnetLine, MUIA_String_Contents), i + 1);
		DoMethod(data->LV_UserStopnet, MUIM_List_Remove, i);
		set(data->LV_UserStopnet, MUIA_List_Quiet, FALSE);
		set(win, MUIA_Window_ActiveObject, data->STR_StopnetLine);
	}

	return(NULL);
}

ULONG User_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct User_Data tmp;
	static STRPTR STR_GR_UserRegister[4];

	STR_GR_UserRegister[0] = GetStr(MSG_UserRegister1);
	STR_GR_UserRegister[1] = GetStr(MSG_UserRegister2);
	STR_GR_UserRegister[2] = GetStr(MSG_UserRegister3);
	STR_GR_UserRegister[3] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		Child, tmp.GR_Register = RegisterObject,
			MUIA_Background		, MUII_RegisterBack,
			MUIA_Register_Titles	, STR_GR_UserRegister,
			MUIA_CycleChain		, 1,
			Child, ColGroup(2),
				Child, VVSpace,
				Child, HVSpace,
				Child, VVSpace,
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
				Child, VVSpace,
				Child, HVSpace,
				Child, MakeKeyLabel2(MSG_LA_EMail, MSG_CC_EMail),
				Child, tmp.STR_EMail = MakeKeyString("", 80, MSG_CC_EMail),
				Child, MakeKeyLabel2(MSG_LA_RealName, MSG_CC_RealName),
				Child, tmp.STR_RealName = MakeKeyString("", 80, MSG_CC_RealName),
				Child, MakeKeyLabel2(MSG_LA_Organisation, MSG_CC_Organisation),
				Child, tmp.STR_Organisation = MakeKeyString("Private User", 80, MSG_CC_Organisation),
				Child, VVSpace,
				Child, HVSpace,
				Child, VVSpace,
				Child, HVSpace,
			End,
			Child, tmp.GR_UserStartnet = VGroup,
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
					Child, tmp.BT_Remove	= MakeButton(MSG_BT_Remove),
				End,
			End,
			Child, tmp.GR_UserStopnet = VGroup,
				MUIA_Group_Spacing, 0,
				Child, tmp.LV_UserStopnet = ListviewObject,
					MUIA_CycleChain		, 1,
					MUIA_Listview_DragType		, 1,
					MUIA_Listview_Input			, TRUE,
					MUIA_Listview_List			, tmp.LI_UserStopnet = ListObject,
						MUIA_Frame					, MUIV_Frame_InputList,
						MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
						MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
						MUIA_List_DragSortable	, TRUE,
						MUIA_List_AutoVisible	, TRUE,
					End,
				End,
				Child, tmp.STR_StopnetLine = MakeKeyString("", MAXPATHLEN, "   "),
				Child, HGroup,
					MUIA_Group_Spacing, 0,
					Child, tmp.BT_StopnetNew		= MakeButton(MSG_BT_New),
					Child, tmp.BT_StopnetRemove	= MakeButton(MSG_BT_Remove),
				End,
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct User_Data *data = INST_DATA(cl, obj);

		*data = tmp;

		set(tmp.STR_Line			, MUIA_String_AttachedList, tmp.LV_UserStartnet);
		set(tmp.STR_StopnetLine	, MUIA_String_AttachedList, tmp.LV_UserStopnet);

		set(tmp.STR_LoginName	, MUIA_ShortHelp, GetStr(MSG_Help_LoginName));
		set(tmp.STR_Password		, MUIA_ShortHelp, GetStr(MSG_Help_Password));
		set(tmp.STR_EMail			, MUIA_ShortHelp, GetStr(MSG_Help_EMail));
		set(tmp.STR_RealName		, MUIA_ShortHelp, GetStr(MSG_Help_RealName));
		set(tmp.STR_Organisation, MUIA_ShortHelp, GetStr(MSG_Help_Organisation));

		set(tmp.LV_UserStartnet	, MUIA_ShortHelp, GetStr(MSG_Help_UserStartnet));
		set(tmp.STR_Line			, MUIA_ShortHelp, GetStr(MSG_Help_Line));
		set(tmp.BT_New				, MUIA_ShortHelp, GetStr(MSG_Help_New));
		set(tmp.BT_Remove			, MUIA_ShortHelp, GetStr(MSG_Help_Remove));

		set(tmp.LV_UserStopnet	, MUIA_ShortHelp, GetStr(MSG_Help_UserStopnet));
		set(tmp.STR_StopnetLine	, MUIA_ShortHelp, GetStr(MSG_Help_Line));
		set(tmp.BT_StopnetNew	, MUIA_ShortHelp, GetStr(MSG_Help_New));
		set(tmp.BT_StopnetRemove, MUIA_ShortHelp, GetStr(MSG_Help_Remove));

		DoMethod(tmp.LV_UserStartnet, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, obj, 1, MUIM_User_UserStartnetList_Active);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_UserStartnet, 3, MUIM_List_InsertSingle, "", MUIV_List_Insert_Bottom);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_UserStartnet, 3, MUIM_Set, MUIA_List_Active, MUIV_List_Active_Bottom);
		DoMethod(tmp.BT_New, MUIM_Notify, MUIA_Pressed, FALSE, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Line);
		DoMethod(tmp.BT_Remove, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_UserStartnet, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.STR_Line, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 1, MUIM_User_ChangeLine);
		DoMethod(tmp.LV_UserStopnet, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, obj, 1, MUIM_User_UserStopnetList_Active);
		DoMethod(tmp.BT_StopnetNew, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_UserStopnet, 3, MUIM_List_InsertSingle, "", MUIV_List_Insert_Bottom);
		DoMethod(tmp.BT_StopnetNew, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_UserStopnet, 3, MUIM_Set, MUIA_List_Active, MUIV_List_Active_Bottom);
		DoMethod(tmp.BT_StopnetNew, MUIM_Notify, MUIA_Pressed, FALSE, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_StopnetLine);
		DoMethod(tmp.BT_StopnetRemove, MUIM_Notify, MUIA_Pressed, FALSE, tmp.LV_UserStopnet, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
		DoMethod(tmp.STR_StopnetLine, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 1, MUIM_User_StopnetChangeLine);
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
		case MUIM_User_UserStopnetList_Active	: return(User_UserStopnetList_Active	(cl, obj, (APTR)msg));
		case MUIM_User_StopnetChangeLine			: return(User_StopnetChangeLine			(cl, obj, (APTR)msg));
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
				Child, VVSpace,
				Child, HVSpace,
				Child, VVSpace,
				Child, HVSpace,
				Child, Label2(GetStr(MSG_LA_ModemType)),
				Child, tmp.PO_Modem = PopobjectObject,
					MUIA_Popstring_String		, tmp.TX_Modem = TextObject, TextFrame, End,
					MUIA_Popstring_Button		, PopButton(MUII_PopUp),
				   MUIA_Popobject_StrObjHook	, &txtobjhook,
					MUIA_Popobject_Object		, tmp.LV_Modem = ListviewObject,
						MUIA_CycleChain		, 1,
						MUIA_Listview_List			, ListObject,
							MUIA_Frame					, MUIV_Frame_InputList,
							MUIA_List_ConstructHook	, &ModemList_ConstructHook,
							MUIA_List_DestructHook	, &des_hook,
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
				Child, VVSpace,
				Child, HVSpace,
				Child, MakeKeyLabel2(MSG_LA_RedialAttempts, MSG_CC_RedialAttempts),
				Child, tmp.SL_RedialAttempts = MakeKeySlider(0, 99, 15, MSG_CC_RedialAttempts),
				Child, MakeKeyLabel2(MSG_LA_RedialDelay, MSG_CC_RedialDelay),
				Child, tmp.SL_RedialDelay = MakeKeySlider(0, 120, 5, MSG_CC_RedialDelay),
				Child, VVSpace,
				Child, HVSpace,
				Child, VVSpace,
				Child, HVSpace,
			End,
			Child, VGroup,
				Child, HVSpace,
				Child, HVSpace,
				Child, ColGroup(2),
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
					Child, HVSpace,
					Child, tmp.CH_Carrier = KeyCheckMark(TRUE, *GetStr(MSG_CC_CarrierDetect)),
					Child, KeyLLabel1(GetStr(MSG_LA_CarrierDetect), *GetStr(MSG_CC_CarrierDetect)),
					Child, HVSpace,
					Child, HVSpace,
					Child, tmp.CH_7Wire = KeyCheckMark(TRUE, *GetStr(MSG_CC_HardwareHandshake)),
					Child, KeyLLabel1(GetStr(MSG_LA_HardwareHandshake), *GetStr(MSG_CC_HardwareHandshake)),
					Child, HVSpace,
					Child, HVSpace,
					Child, tmp.CH_OwnDevUnit = KeyCheckMark(FALSE, *GetStr(MSG_CC_OwnDevUnit)),
					Child, KeyLLabel1(GetStr(MSG_LA_OwnDevUnit), *GetStr(MSG_CC_OwnDevUnit)),
					Child, HVSpace,
				End,
				Child, HVSpace,
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
		set(tmp.STR_DialPrefix	, MUIA_ShortHelp, GetStr(MSG_Help_DialPrefix));
		set(tmp.STR_ModemInit	, MUIA_ShortHelp, GetStr(MSG_Help_ModemInit));
		set(tmp.SL_RedialAttempts, MUIA_ShortHelp, GetStr(MSG_Help_RedialAttempts));
		set(tmp.SL_RedialDelay	, MUIA_ShortHelp, GetStr(MSG_Help_RedialDelay));

		set(tmp.STR_SerialDriver, MUIA_ShortHelp, GetStr(MSG_Help_SerialDriver));
		set(tmp.STR_SerialUnit	, MUIA_ShortHelp, GetStr(MSG_Help_SerialUnit));
		set(tmp.STR_BaudRate		, MUIA_ShortHelp, GetStr(MSG_Help_BaudRate));
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
		MUIA_Background, MUII_RegisterBack,
		MUIA_Group_Columns, 2,
		GroupFrame,
		Child, VVSpace,
		Child, HVSpace,
		Child, VVSpace,
		Child, HVSpace,
		Child, MakeKeyLabel2(MSG_LA_MailIn, MSG_CC_MailIn),
		Child, tmp.PA_MailIn = MakePopAsl(tmp.STR_MailIn = MakeKeyString("NetConnect:Data/MailIn", MAXPATHLEN, MSG_CC_MailIn), MSG_LA_MailIn, TRUE),
		Child, MakeKeyLabel2(MSG_LA_MailOut, MSG_CC_MailOut),
		Child, tmp.PA_MailOut = MakePopAsl(tmp.STR_MailOut = MakeKeyString("NetConnect:Data/MailOut", MAXPATHLEN, MSG_CC_MailOut), MSG_LA_MailOut, TRUE),
		Child, VVSpace,
		Child, HVSpace,
		Child, MakeKeyLabel2(MSG_LA_NewsIn, MSG_CC_NewsIn),
		Child, tmp.PA_NewsIn = MakePopAsl(tmp.STR_NewsIn = MakeKeyString("NetConnect:Data/NewsIn", MAXPATHLEN, MSG_CC_NewsIn), MSG_LA_NewsIn, TRUE),
		Child, MakeKeyLabel2(MSG_LA_NewsOut, MSG_CC_NewsOut),
		Child, tmp.PA_NewsOut = MakePopAsl(tmp.STR_NewsOut = MakeKeyString("NetConnect:Data/NewsOut", MAXPATHLEN, MSG_CC_NewsOut), MSG_LA_NewsOut, TRUE),
		Child, VVSpace,
		Child, HVSpace,
		Child, MakeKeyLabel2(MSG_LA_FilesIn, MSG_CC_FilesIn),
		Child, tmp.PA_FileIn = MakePopAsl(tmp.STR_FileIn = MakeKeyString("NetConnect:Data/Download", MAXPATHLEN, MSG_CC_FilesIn), MSG_LA_FilesIn, TRUE),
		Child, MakeKeyLabel2(MSG_LA_FilesOut, MSG_CC_FilesOut),
		Child, tmp.PA_FileOut = MakePopAsl(tmp.STR_FileOut = MakeKeyString("NetConnect:Data/Upload", MAXPATHLEN, MSG_CC_FilesOut), MSG_LA_FilesOut, TRUE),
		Child, VVSpace,
		Child, HVSpace,
		Child, VVSpace,
		Child, HVSpace,
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


/****************************************************************************/
/* Pager (LIST)                                                             */
/****************************************************************************/

SAVEDS ASM LONG PagerList_DisplayFunc(REG(a0) struct Hook *hook, REG(a2) char **array, REG(a1) STRPTR string)
{
	struct PagerList_Data *data = (APTR)hook->h_Data;

	if(string)
	{
		static char buf1[50];

		if(!strcmp(string, data->provider))
			sprintf(buf1,"\33c\33O[%08lx]", data->i_provider);
		else if(!strcmp(string, data->user))
			sprintf(buf1,"\33c\33O[%08lx]", data->i_user);
		else if(!strcmp(string, data->modem))
			sprintf(buf1,"\33c\33O[%08lx]", data->i_modem);
		else if(!strcmp(string, data->groups))
			sprintf(buf1,"\33c\33O[%08lx]", data->i_groups);
		else if(!strcmp(string, data->databases))
			sprintf(buf1,"\33c\33O[%08lx]", data->i_databases);
		else if(!strcmp(string, data->paths))
			sprintf(buf1,"\33c\33O[%08lx]", data->i_paths);
		else
			sprintf(buf1,"\33c\33O[%08lx]", data->i_information);

		*array++ = buf1;
		*array = string;
	}
	return(NULL);
}

Object *create_image(struct BitMapHeader *bmhd, ULONG *cols, UBYTE *body)
{
	return(BodychunkObject,
		MUIA_Background				, MUII_ButtonBack,
		MUIA_Bitmap_SourceColors	, cols,
		MUIA_Bitmap_Width				, bmhd->bmh_Width,
		MUIA_Bitmap_Height			, bmhd->bmh_Height,
		MUIA_FixWidth					, bmhd->bmh_Width ,
		MUIA_FixHeight					, bmhd->bmh_Height,
		MUIA_Bodychunk_Depth			, bmhd->bmh_Depth,
		MUIA_Bodychunk_Body			, body,
		MUIA_Bodychunk_Compression	, bmhd->bmh_Compression,
		MUIA_Bodychunk_Masking		, bmhd->bmh_Masking,
		MUIA_Bitmap_Transparent		, 0,
	End);
}

ULONG PagerList_Setup(struct IClass *cl, Object *obj, Msg msg)
{
	struct PagerList_Data *data = INST_DATA(cl, obj);

	if(!DoSuperMethodA(cl, obj, msg))
		return(FALSE);

	data->o_information	= create_image((struct BitMapHeader *)&information_header, (ULONG *)information_colors	, (UBYTE *)information_body);
	data->o_provider		= create_image((struct BitMapHeader *)&provider_header	, (ULONG *)provider_colors		, (UBYTE *)provider_body);
	data->o_user			= create_image((struct BitMapHeader *)&user_header			, (ULONG *)user_colors			, (UBYTE *)user_body);
	data->o_modem			= create_image((struct BitMapHeader *)&modem_header		, (ULONG *)modem_colors			, (UBYTE *)modem_body);
	data->o_groups			= create_image((struct BitMapHeader *)&groups_header		, (ULONG *)groups_colors		, (UBYTE *)groups_body);
	data->o_databases		= create_image((struct BitMapHeader *)&databases_header	, (ULONG *)databases_colors	, (UBYTE *)databases_body);
	data->o_paths			= create_image((struct BitMapHeader *)&paths_header		, (ULONG *)paths_colors			, (UBYTE *)paths_body);

	data->i_information	= (APTR)DoMethod(obj, MUIM_List_CreateImage, data->o_information	, 0);
	data->i_provider		= (APTR)DoMethod(obj, MUIM_List_CreateImage, data->o_provider		, 0);
	data->i_user			= (APTR)DoMethod(obj, MUIM_List_CreateImage, data->o_user			, 0);
	data->i_modem			= (APTR)DoMethod(obj, MUIM_List_CreateImage, data->o_modem			, 0);
	data->i_groups			= (APTR)DoMethod(obj, MUIM_List_CreateImage, data->o_groups			, 0);
	data->i_databases		= (APTR)DoMethod(obj, MUIM_List_CreateImage, data->o_databases		, 0);
	data->i_paths			= (APTR)DoMethod(obj, MUIM_List_CreateImage, data->o_paths			, 0);

//	MUI_RequestIDCMP(obj,IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);

	return(TRUE);
}


ULONG PagerList_Cleanup(struct IClass *cl,Object *obj,Msg msg)
{
	struct PagerList_Data *data = INST_DATA(cl,obj);

	DoMethod(obj, MUIM_List_DeleteImage, data->i_information);
	DoMethod(obj, MUIM_List_DeleteImage, data->i_provider);
	DoMethod(obj, MUIM_List_DeleteImage, data->i_user);
	DoMethod(obj, MUIM_List_DeleteImage, data->i_modem);
	DoMethod(obj, MUIM_List_DeleteImage, data->i_groups);
	DoMethod(obj, MUIM_List_DeleteImage, data->i_databases);
	DoMethod(obj, MUIM_List_DeleteImage, data->i_paths);

	if(data->o_information)
		MUI_DisposeObject(data->o_information);
	if(data->o_provider)
		MUI_DisposeObject(data->o_provider);
	if(data->o_user)
		MUI_DisposeObject(data->o_user);
	if(data->o_modem)
		MUI_DisposeObject(data->o_modem);
	if(data->o_groups)
		MUI_DisposeObject(data->o_groups);
	if(data->o_databases)
		MUI_DisposeObject(data->o_databases);
	if(data->o_paths)
		MUI_DisposeObject(data->o_paths);

//	MUI_RejectIDCMP(obj,IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);

	return(DoSuperMethodA(cl, obj ,msg));
}

ULONG PagerList_New(struct IClass *cl,Object *obj,struct opSet *msg)
{
	obj = (Object *)DoSuperNew(cl, obj,
		InputListFrame,
		MUIA_List_Format			, ",",
		MUIA_List_MinLineHeight	, MAX(INFORMATION_HEIGHT, MAX(PROVIDER_HEIGHT, MAX(USER_HEIGHT, MAX(MODEM_HEIGHT, MAX(GROUPS_HEIGHT, MAX(DATABASES_HEIGHT, PATHS_HEIGHT)))))),
		TAG_MORE, msg->ops_AttrList);

	if(obj)
	{
		struct PagerList_Data *data = INST_DATA(cl, obj);

		data->information	= GetStr(MSG_Pages1);
		data->provider		= GetStr(MSG_Pages2);
		data->user			= GetStr(MSG_Pages3);
		data->modem			= GetStr(MSG_Pages4);
		data->groups		= GetStr(MSG_Pages5);
		data->databases	= GetStr(MSG_Pages6);
		data->paths			= GetStr(MSG_Pages7);

		data->DisplayHook.h_Entry = (VOID *)PagerList_DisplayFunc;
		data->DisplayHook.h_Data  = (APTR)data;
		set(obj, MUIA_List_DisplayHook, &data->DisplayHook);
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG PagerList_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg)
{
	switch(msg->MethodID)
	{
		case OM_NEW			: return(PagerList_New		(cl, obj, (APTR)msg));
		case MUIM_Setup	: return(PagerList_Setup	(cl, obj, (APTR)msg));
		case MUIM_Cleanup	: return(PagerList_Cleanup	(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}
