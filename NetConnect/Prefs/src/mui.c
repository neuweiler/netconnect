#include "globals.c"

extern STRPTR GetStr(STRPTR idstr);


ULONG __stdargs DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...)
{
	return(DoSuperMethod(cl, obj, OM_NEW, &tag1, NULL));
}

LONG xget(Object *obj, ULONG attribute)
{
	LONG x;
	get(obj, attribute, &x);
	return(x);
}

SAVEDS ASM LONG sortfunc(REG(a1) STRPTR str1, REG(a2) STRPTR str2)
{
	return(stricmp(str1, str2));
}
static struct Hook sorthook = { {NULL, NULL}, (VOID *)sortfunc, NULL, NULL};


SAVEDS ASM LONG strobjfunc(REG(a2) Object *list, REG(a1) Object *str)
{
	char *x, *s;
	int i;

	get(str, MUIA_Text_Contents, &s);

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
static struct Hook strobjhook = { {NULL, NULL}, (VOID *)strobjfunc, NULL, NULL};


LONG get_file_size(STRPTR file)
{
	struct FileInfoBlock *fib;
	BPTR lock;
	LONG size = 0;

	if(lock = Lock(file, ACCESS_READ))
	{
		if(fib = AllocDosObject(DOS_FIB, NULL))
		{
			if(Examine(lock, fib))
				size = (fib->fib_DirEntryType > 0 ? -1 : fib->fib_Size);

			FreeDosObject(DOS_FIB, fib);
		}
		UnLock(lock);
	}
	return(size);
}


BOOL ParseConfig(STRPTR file, struct pc_Data *pc_data)
{
	LONG size;
	STRPTR buf = NULL;
	BPTR fh;
	BOOL success = FALSE;

	if((size = get_file_size(file)) > 0)
	{
		if(buf = AllocVec(size, MEMF_ANY))
		{
			if(fh = Open(file, MODE_OLDFILE))
			{
				if(Read(fh, buf, size) == size)
				{
					success = TRUE;

					pc_data->Buffer	= buf;
					pc_data->Size		= size;
					pc_data->Current	= buf;

					pc_data->Argument	= NULL;
					pc_data->Contents	= NULL;
				}

				Close(fh);
			}
		}
	}

	return(success);
}

BOOL ParseNext(struct pc_Data *pc_data)
{
	BOOL success = FALSE;
	STRPTR ptr_eol, ptr_tmp;

	if(pc_data->Current && pc_data->Current < pc_data->Buffer + pc_data->Size)
	{
		if(ptr_eol = strchr(pc_data->Current, '\n'))
		{
			*ptr_eol = NULL;

			pc_data->Contents	= strchr(pc_data->Current, ' ');							/* a space	*/
			ptr_tmp				= strchr(pc_data->Current, 9);							/* or a TAB	*/

			if((ptr_tmp < pc_data->Contents && ptr_tmp) || !pc_data->Contents)	/* which one comes first ? */
				pc_data->Contents = ptr_tmp;
			if(pc_data->Contents)
			{
				*pc_data->Contents++ = NULL;
				while((*pc_data->Contents == ' ') || (*pc_data->Contents == 9))
					pc_data->Contents++;

				ptr_tmp = NULL;
				if(*pc_data->Contents == 34)							/* we take everything that's between two '"' */
				{
					pc_data->Contents++;
					if(ptr_tmp = strchr(pc_data->Contents, 34))	/* find the ending '"' */
						*ptr_tmp = NULL;
				}
				if(!ptr_tmp)												/* there was no '"' */
				{
					if(ptr_tmp = strchr(pc_data->Contents, ';'))	/* cut out the comment */
						*ptr_tmp = NULL;
					if(ptr_tmp = strchr(pc_data->Contents, ' '))	/* is there a space left ? */
						*ptr_tmp = NULL;
					if(ptr_tmp = strchr(pc_data->Contents, 9))	/* or a TAB ? */
						*ptr_tmp = NULL;
				}
			}
			else
				pc_data->Contents = "";

			pc_data->Argument = pc_data->Current;
			pc_data->Current	= ptr_eol + 1;
			success = TRUE;
		}
		else
			pc_data->Current = NULL;
	}
	return(success);
}

VOID ParseEnd(struct pc_Data *pc_data)
{
	if(pc_data->Buffer)
		FreeVec(pc_data->Buffer);

	pc_data->Buffer	= NULL;
	pc_data->Size		= NULL;
	pc_data->Current	= NULL;

	pc_data->Argument	= NULL;
	pc_data->Contents	= NULL;
}



/****************************************************************************/
/* Server Prefs class (GROUP)                                               */
/****************************************************************************/

ULONG ServerPrefs_PopList_Update(struct IClass *cl, Object *obj, struct MUIP_ServerPrefs_PopList_Update *msg)
{
	struct ServerPrefs_Data *data = INST_DATA(cl, obj);
	Object *list;
	BPTR lock;
	struct FileInfoBlock *fib;

	switch(msg->flags)
	{
		case MUIV_ServerPrefs_PopString_Country:
			list = data->LV_Country;
			break;
		case MUIV_ServerPrefs_PopString_Provider:
			list = data->LV_Provider;
			break;
		case MUIV_ServerPrefs_PopString_PoP:
			list = data->LV_PoP;
			break;
	}

	DoMethod(list, MUIM_List_Clear);
	if(lock = Lock(msg->path, ACCESS_READ))
	{
		if(fib = AllocDosObject(DOS_FIB, NULL))
		{
			if(Examine(lock, fib))
			{
				while(ExNext(lock, fib))
				{
					if((msg->flags != MUIV_ServerPrefs_PopString_PoP && fib->fib_DirEntryType > 0) || (msg->flags == MUIV_ServerPrefs_PopString_PoP && fib->fib_DirEntryType < 0))
						DoMethod(list, MUIM_List_InsertSingle, fib->fib_FileName, MUIV_List_Insert_Sorted);
				}
			}
			FreeDosObject(DOS_FIB, fib);
		}
		UnLock(lock);
	}
	return(NULL);
}

ULONG ServerPrefs_PopString_Close(struct IClass *cl, Object *obj, struct MUIP_ServerPrefs_PopString_Close *msg)
{
	struct ServerPrefs_Data *data = INST_DATA(cl, obj);
	Object *list_view, *string;
	STRPTR x;
	char path[MAXPATHLEN];

	switch(msg->flags)
	{
		case MUIV_ServerPrefs_PopString_Country:
			list_view	= data->LV_Country;
			string		= data->PO_Country;
			break;
		case MUIV_ServerPrefs_PopString_Provider:
			list_view	= data->LV_Provider;
			string		= data->PO_Provider;
			break;
		case MUIV_ServerPrefs_PopString_PoP:
			list_view	= data->LV_PoP;
			string		= data->PO_PoP;
			break;
	}

	DoMethod(list_view, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &x);
	if(x)
		set(string, MUIA_Text_Contents, x);
	DoMethod(string, MUIM_Popstring_Close, TRUE);

	switch(msg->flags)
	{
		case MUIV_ServerPrefs_PopString_Country:
			strcpy(path, "AmiTCP:Providers");
			AddPart(path, (STRPTR)xget(data->PO_Country, MUIA_Text_Contents), MAXPATHLEN);
			DoMethod(obj, MUIM_ServerPrefs_PopList_Update, path, MUIV_ServerPrefs_PopString_Provider);
			DoMethod(obj, MUIM_ServerPrefs_PopList_Update, path, MUIV_ServerPrefs_PopString_PoP);
			set(data->PO_Provider, MUIA_Text_Contents, "");
			set(data->PO_PoP, MUIA_Text_Contents, "");
			break;

		case MUIV_ServerPrefs_PopString_Provider:
			strcpy(path, "AmiTCP:Providers");
			AddPart(path, (STRPTR)xget(data->PO_Country	, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(path, (STRPTR)xget(data->PO_Provider	, MUIA_Text_Contents), MAXPATHLEN);
			DoMethod(obj, MUIM_ServerPrefs_PopList_Update, path, MUIV_ServerPrefs_PopString_PoP);
			set(data->PO_PoP, MUIA_Text_Contents, "");
			break;

		case MUIV_ServerPrefs_PopString_PoP:
			strcpy(path, "AmiTCP:Providers");
			AddPart(path, (STRPTR)xget(data->PO_Country	, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(path, (STRPTR)xget(data->PO_Provider	, MUIA_Text_Contents), MAXPATHLEN);
			AddPart(path, (STRPTR)xget(data->PO_PoP		, MUIA_Text_Contents), MAXPATHLEN);
			DoMethod(win, MUIM_AmiTCPPrefs_LoadConfig, path);
			break;
	}

	return(NULL);
}

ULONG ServerPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct ServerPrefs_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_Server",
		Child, ColGroup(2),
			MUIA_HelpNode	, "GR_Provider",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_ProviderTitle),
			Child, Label(GetStr(MSG_LA_Country)),
			Child, tmp.PO_Country = PopobjectObject,
				MUIA_Popstring_String		, TextObject, TextFrame, End,
				MUIA_Popstring_Button		, PopButton(MUII_PopUp),
			   MUIA_Popobject_StrObjHook	, &strobjhook,
				MUIA_Popobject_Object		, tmp.LV_Country = ListviewObject,
					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_List			, ListObject,
						MUIA_Frame					, MUIV_Frame_InputList,
						MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
						MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
						MUIA_List_CompareHook	, &sorthook,
					End,
				End,
			End,
			Child, Label(GetStr(MSG_LA_Provider)),
			Child, tmp.PO_Provider = PopobjectObject,
				MUIA_Popstring_String		, TextObject,
					TextFrame,
					MUIA_Text_Contents		, "user defined",
				End,
				MUIA_Popstring_Button		, PopButton(MUII_PopUp),
			   MUIA_Popobject_StrObjHook	, &strobjhook,
				MUIA_Popobject_Object		, tmp.LV_Provider = ListviewObject,
					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_List			, ListObject,
						MUIA_Frame					, MUIV_Frame_InputList,
						MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
						MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
						MUIA_List_CompareHook	, &sorthook,
					End,
				End,
			End,
			Child, Label(GetStr(MSG_LA_PoP)),
			Child, tmp.PO_PoP = PopobjectObject,
				MUIA_Popstring_String		, TextObject, TextFrame, End,
				MUIA_Popstring_Button		, PopButton(MUII_PopUp),
			   MUIA_Popobject_StrObjHook	, &strobjhook,
				MUIA_Popobject_Object		, tmp.LV_PoP = ListviewObject,
					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_List			, ListObject,
						MUIA_Frame			, MUIV_Frame_InputList,
						MUIA_List_ConstructHook	, MUIV_List_ConstructHook_String,
						MUIA_List_DestructHook	, MUIV_List_DestructHook_String,
						MUIA_List_CompareHook	, &sorthook,
					End,
				End,
			End,
			Child, Label(GetStr(MSG_LA_Phone)),
			Child, tmp.STR_Phone = String("", 80),
		End,
		Child, ColGroup(4),
			MUIA_HelpNode	, "GR_Details",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_DetailsTitle),
			Child, Label(GetStr(MSG_LA_NameServer1)),
			Child, tmp.STR_NameServer1 = StringObject,
				MUIA_Frame				, MUIV_Frame_String,
				MUIA_String_Contents	, "",
				MUIA_String_Accept	, "0123456789.",
				MUIA_String_MaxLen	, 18,
			End,
			Child, Label(GetStr(MSG_LA_MailServer)),
			Child, tmp.STR_MailServer = String("mail", 80),
			Child, Label(GetStr(MSG_LA_NameServer2)),
			Child, tmp.STR_NameServer2 = StringObject,
				MUIA_Frame				, MUIV_Frame_String,
				MUIA_String_Accept	, "0123456789.",
				MUIA_String_MaxLen	, 18,
			End,
			Child, Label(GetStr(MSG_LA_NewsServer)),
			Child, tmp.STR_NewsServer = String("news", 80),
			Child, Label(GetStr(MSG_LA_Netmask)),
			Child, tmp.STR_Netmask = StringObject,
				MUIA_Frame				, MUIV_Frame_String,
				MUIA_String_Contents	, "255.255.255.0",
				MUIA_String_Accept	, "0123456789.",
				MUIA_String_MaxLen	, 18,
			End,
			Child, Label(GetStr(MSG_LA_IRCServer)),
			Child, tmp.STR_IRCServer = String("irc.funet.fi", 80),
			Child, Label(GetStr(MSG_LA_Gateway)),
			Child, tmp.STR_Gateway = StringObject,
				MUIA_Frame				, MUIV_Frame_String,
				MUIA_String_Contents	, "",
				MUIA_String_Accept	, "0123456789.",
				MUIA_String_MaxLen	, 18,
			End,
			Child, Label(GetStr(MSG_LA_DomainName)),
			Child, tmp.STR_DomainName = String("", 80),
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct ServerPrefs_Data *data = INST_DATA(cl,obj);

		*data = tmp;

		DoMethod(tmp.LV_Country	, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime	, obj, 2, MUIM_ServerPrefs_PopString_Close, MUIV_ServerPrefs_PopString_Country);
		DoMethod(tmp.LV_Provider, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime	, obj, 2, MUIM_ServerPrefs_PopString_Close, MUIV_ServerPrefs_PopString_Provider);
		DoMethod(tmp.LV_PoP		, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime	, obj, 2, MUIM_ServerPrefs_PopString_Close, MUIV_ServerPrefs_PopString_PoP);
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG ServerPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(ServerPrefs_New					(cl, obj, (APTR)msg));
		case MUIM_ServerPrefs_PopString_Close	: return(ServerPrefs_PopString_Close	(cl, obj, (APTR)msg));
		case MUIM_ServerPrefs_PopList_Update	: return(ServerPrefs_PopList_Update		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* User Prefs class (GROUP)                                                 */
/****************************************************************************/

ULONG UserPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct UserPrefs_Data tmp;

	STR_RA_Connection[0] = GetStr(MSG_RA_Connection0);
	STR_RA_Connection[1] = GetStr(MSG_RA_Connection1);
	STR_RA_Connection[2] = NULL;

	STR_RA_Interface[0] = "PPP";
	STR_RA_Interface[1] = "SLIP";
	STR_RA_Interface[2] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_User",
		Child, HGroup,
			MUIA_HelpNode	, "GR_Connection",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_ConnectionTitle),
			Child, HVSpace,
			Child, tmp.RA_Connection = RadioObject,
				MUIA_HelpNode		, "RA_Connection",
				MUIA_Radio_Entries, STR_RA_Connection,
			End,
			Child, HVSpace,
			Child, tmp.RA_Interface = RadioObject,
				MUIA_HelpNode		, "RA_Interface",
				MUIA_Radio_Entries, STR_RA_Interface,
			End,
			Child, HVSpace,
			Child, tmp.CH_BOOTP = CheckMark(FALSE),
			Child, LLabel("BOOTP"),
			Child, HVSpace,
		End,
		Child, ColGroup(4),
			MUIA_HelpNode	, "GR_UserDetails",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_UserDetailsTitle),
			Child, Label(GetStr(MSG_LA_UserName)),
			Child, tmp.STR_UserName = String("", 80),
			Child, Label(GetStr(MSG_LA_Organisation)),
			Child, tmp.STR_Organisation = String("Private User", 80),
			Child, Label(GetStr(MSG_LA_RealName)),
			Child, tmp.STR_RealName = String("", 80),
			Child, Label(GetStr(MSG_LA_NodeName)),
			Child, tmp.STR_NodeName = String("", 80),
			Child, Label(GetStr(MSG_LA_Password)),
			Child, tmp.STR_Password = StringObject,
				MUIA_Frame				, MUIV_Frame_String,
				MUIA_HelpNode			, "STR_Password",
				MUIA_String_Contents	, "",
				MUIA_String_Secret	, TRUE,
			End,
			Child, Label(GetStr(MSG_LA_Address)),
			Child, tmp.STR_IP_Address = StringObject,
				MUIA_Frame				, MUIV_Frame_String,
				MUIA_HelpNode			, "STR_IP_Address",
				MUIA_String_Contents	, "0.0.0.0",
				MUIA_String_Accept	, "0123456789.",
				MUIA_String_MaxLen	, 18,
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct UserPrefs_Data *data = INST_DATA(cl,obj);

		*data = tmp;
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG UserPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(UserPrefs_New		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Misc Prefs class (GROUP)                                                 */
/****************************************************************************/

ULONG MiscPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct MiscPrefs_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_Misc",
		Child, ColGroup(2),
			MUIA_HelpNode	, "GR_Drawers",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_DrawersTitle),
			Child, Label(GetStr(MSG_LA_MailIn)),
			Child, tmp.PA_MailIn = PopaslObject,
				MUIA_Popasl_Type		, 0,
				MUIA_Popstring_String, String("NetConnect:Data/MailIn", MAXPATHLEN),
				MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
			End,
			Child, Label(GetStr(MSG_LA_MailOut)),
			Child, tmp.PA_MailOut = PopaslObject,
				MUIA_Popasl_Type		, 0,
				MUIA_Popstring_String, String("NetConnect:Data/MailOut", MAXPATHLEN),
				MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
			End,
			Child, Label(GetStr(MSG_LA_FilesIn)),
			Child, tmp.PA_FileIn = PopaslObject,
				MUIA_Popasl_Type		, 0,
				MUIA_Popstring_String, String("NetConnect:Data/Download", MAXPATHLEN),
				MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
			End,
			Child, Label(GetStr(MSG_LA_FilesOut)),
			Child, tmp.PA_FileOut = PopaslObject,
				MUIA_Popasl_Type		, 0,
				MUIA_Popstring_String, String("NetConnect:Data/Upload", MAXPATHLEN),
				MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
			End,
		End,
		Child, ColGroup(4),
			MUIA_HelpNode	, "GR_Modem",
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, GetStr(MSG_GR_ModemTitle),
			Child, Label(GetStr(MSG_LA_SerialDriver)),
			Child, tmp.PA_SerialDriver = PopaslObject,
				MUIA_Popasl_Type		, 0,
				MUIA_Popstring_String, String("serial.device", MAXPATHLEN),
				MUIA_Popstring_Button, PopButton(MUII_PopFile),
			End,
			Child, Label(GetStr(MSG_LA_ModemInit)),
			Child, tmp.STR_ModemInit = String("ATZ", 80),
			Child, Label(GetStr(MSG_LA_SerialUnit)),
			Child, tmp.STR_SerialUnit = StringObject,
				StringFrame,
				MUIA_String_MaxLen	, 5,
				MUIA_String_Integer	, 0,
				MUIA_String_Accept	, "1234567890",
			End,
			Child, Label(GetStr(MSG_LA_DialPrefix)),
			Child, tmp.PO_DialPrefix = PopobjectObject,
				MUIA_Popstring_String		, String("ATDT", 80),
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
			Child, Label(GetStr(MSG_LA_DialSuffix)),
			Child, tmp.STR_DialSuffix = String("\\r", 80),
			Child, Label(GetStr(MSG_LA_BaudRate)),
			Child, tmp.PO_BaudRate = PopobjectObject,
				MUIA_Popstring_String, StringObject,
					StringFrame,
					MUIA_String_MaxLen	, 5,
					MUIA_String_Integer	, 56700,
					MUIA_String_Accept	, "1234567890",
				End,
				MUIA_Popstring_Button, PopButton(MUII_PopUp),
				MUIA_Popobject_Object, tmp.LV_BaudRate = ListviewObject,
					MUIA_Listview_DoubleClick	, TRUE,
					MUIA_Listview_List			, ListObject,
						MUIA_Frame				, MUIV_Frame_InputList,
						MUIA_List_Active		, MUIV_List_Active_Top,
						MUIA_List_SourceArray, ARR_BaudRates,
					End,
				End,
			End,
			Child, HVSpace,
			Child, HVSpace,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct MiscPrefs_Data *data = INST_DATA(cl,obj);

		*data = tmp;
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG MiscPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(MiscPrefs_New		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Interface Prefs class (GROUP)                                            */
/****************************************************************************/

ULONG InterfacePrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct InterfacePrefs_Data tmp;

	STR_CY_Compression[0] = "Auto";
	STR_CY_Compression[1] = "On";
	STR_CY_Compression[2] = "Off";
	STR_CY_Compression[3] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_Interface",
		Child, VGroup,
			MUIA_Frame		, MUIV_Frame_Group,
			MUIA_FrameTitle, "Interface",
			Child, HGroup,
				Child, ColGroup(2),
					Child, tmp.CH_Carrier = CheckMark(FALSE),
					Child, LLabel("Carrier Detect"),
					Child, tmp.CH_7Wire = CheckMark(TRUE),
					Child, LLabel("Hardware handshake (RTS/CTS)"),
					Child, tmp.CH_OwnDevUnit = CheckMark(TRUE),
					Child, LLabel("Use OwnDevUnit"),
					Child, tmp.CH_Shared = CheckMark(TRUE),
					Child, LLabel("Shared mode"),
					Child, HVSpace,
					Child, HVSpace,
				End,
				Child, ColGroup(2),
					Child, Label("MTU: "),
					Child, tmp.SL_MTU = Slider(72, 1500, 1500),
					Child, Label("IP header compression: "),
					Child, tmp.CY_Compression = Cycle(STR_CY_Compression),
				End,
			End,
			Child, ColGroup(2),
				Child, Label("Additional options: "),
				Child, tmp.STR_Options = String("", 160),
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct InterfacePrefs_Data *data = INST_DATA(cl,obj);

		*data = tmp;
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG InterfacePrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(InterfacePrefs_New		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/****************************************************************************/
/* Dialscript Prefs class (GROUP)                                           */
/****************************************************************************/

ULONG DialscriptPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct DialscriptPrefs_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_Dialscript",
		Child, tmp.GR_ScriptEditor = ScrollgroupObject,
			MUIA_Scrollgroup_Contents, HVSpace,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct DialscriptPrefs_Data *data = INST_DATA(cl,obj);

		*data = tmp;
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG DialscriptPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(DialscriptPrefs_New		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}




/****************************************************************************/
/* UserStartnet Prefs class (GROUP)                                          */
/****************************************************************************/

ULONG UserStartnetPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct UserStartnetPrefs_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_HelpNode, "GR_UserStartnet",
		Child, tmp.GR_Editor = ScrollgroupObject,
			MUIA_Scrollgroup_Contents, HVSpace,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct UserStartnetPrefs_Data *data = INST_DATA(cl,obj);

		*data = tmp;
	}
	return((ULONG)obj);
}

SAVEDS ASM ULONG UserStartnetPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW										: return(UserStartnetPrefs_New		(cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}




/****************************************************************************/
/* AmiTCP Prefs class                                                       */
/****************************************************************************/

ULONG AmiTCPPrefs_LoadConfig(struct IClass *cl, Object *obj, Msg msg)
{
	struct AmiTCPPrefs_Data			*data						= INST_DATA(cl											, obj);
	struct ServerPrefs_Data			*server_data			= INST_DATA(CL_ServerPrefs->mcc_Class			, data->GR_Server);
	struct UserPrefs_Data			*user_data				= INST_DATA(CL_UserPrefs->mcc_Class				, data->GR_User);
	struct InterfacePrefs_Data		*interface_data		= INST_DATA(CL_InterfacePrefs->mcc_Class		, data->GR_Interface);
	struct MiscPrefs_Data			*misc_data				= INST_DATA(CL_MiscPrefs->mcc_Class				, data->GR_Misc);
//	struct UserStartnetPrefs_Data	*user_startnet_data	= INST_DATA(CL_UserStartnetPrefs->mcc_Class	, data->GR_UserStartnet);
	struct pc_Data pc_data;

	if(ParseConfig((get_file_size("ENV:AmiTCP/Provider.conf") ? "ENV:AmiTCP/Provider.conf" : "AmiTCP:db/Provider.conf"), &pc_data))
	{
		while(ParseNext(&pc_data))
		{
//			if(!stricmp(pc_data.Argument, "Name"))
//			if(!stricmp(pc_data.Argument, "DialUp"))

			if(!stricmp(pc_data.Argument, "Interface"))
					setmutex(user_data->RA_Interface, (stricmp(pc_data.Contents, "ppp") ? 1 : 0));

			if(!stricmp(pc_data.Argument, "InterfaceConfig"))
				setstring(interface_data->STR_Options, pc_data.Contents);

//			if(!stricmp(pc_data.Argument, "NeedSerial"))

			if(!stricmp(pc_data.Argument, "IPDynamic"))
				setmutex(user_data->RA_Connection, (atol(pc_data.Contents) ? 0 : 1));

			if(!stricmp(pc_data.Argument, "IPAddr"))
				setstring(user_data->STR_IP_Address, pc_data.Contents);

//			if(!stricmp(pc_data.Argument, "DestIP"))

			if(!stricmp(pc_data.Argument, "Gateway"))
				setstring(server_data->STR_Gateway, pc_data.Contents);

			if(!stricmp(pc_data.Argument, "Netmask"))
				setstring(server_data->STR_Netmask, pc_data.Contents);

//			if(!stricmp(pc_data.Argument, "NSDynamic"))

			if(!stricmp(pc_data.Argument, "UseBootP"))
				setcheckmark(user_data->CH_BOOTP, atol(pc_data.Contents));

			if(!stricmp(pc_data.Argument, "MTU"))
				setslider(interface_data->SL_MTU, atol(pc_data.Contents));

			if(!stricmp(pc_data.Argument, "Phone"))
				setstring(server_data->STR_Phone, pc_data.Contents);

//			if(!stricmp(pc_data.Argument, "HdrCompress"))


//			if(!stricmp(pc_data.Argument, "MailServer"))
//			if(!stricmp(pc_data.Argument, "NewsServer"))
//			if(!stricmp(pc_data.Argument, "IRCServer"))
//			if(!stricmp(pc_data.Argument, "DomainName"))  only for netconnect provider file (normally stored in resolv.conf)
//			if(!stricmp(pc_data.Argument, "NameServer1"))  only for netconnect provider file (normally stored in resolv.conf)
//			if(!stricmp(pc_data.Argument, "NameServer2"))  only for netconnect provider file (normally stored in resolv.conf)

		}
		ParseEnd(&pc_data);
	}

	if(ParseConfig((get_file_size("ENV:AmiTCP/resolv.conf") ? "ENV:AmiTCP/resolv.conf" : "AmiTCP:db/resolv.conf"), &pc_data))
	{
		int second = FALSE;

		while(ParseNext(&pc_data))
		{
			if(!stricmp(pc_data.Argument, "NAMESERVER"))
				setstring((second++ ? server_data->STR_NameServer1 : server_data->STR_NameServer2), pc_data.Contents);

			if(!stricmp(pc_data.Argument, "DOMAIN"))
				setstring(server_data->STR_DomainName, pc_data.Contents);
		}
		ParseEnd(&pc_data);
	}

	if(ParseConfig((get_file_size("ENV:AmiTCP/User.conf") ? "ENV:AmiTCP/User.conf" : "AmiTCP:db/User.conf"), &pc_data))
	{
		while(ParseNext(&pc_data))
		{
			if(!stricmp(pc_data.Argument, "UserName"))						// is in ENV:LOGNAME
				setstring(user_data->STR_UserName, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "RealName"))
				setstring(user_data->STR_RealName, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "Organisation"))
				setstring(user_data->STR_Organisation, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "Password"))
				setstring(user_data->STR_Password, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "NodeName"))
				setstring(user_data->STR_NodeName, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "MailIn"))
				setstring(misc_data->PA_MailIn, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "MailOut"))
				setstring(misc_data->PA_MailOut, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "FileIn"))
				setstring(misc_data->PA_FileIn, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "FileOut"))
				setstring(misc_data->PA_FileOut, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "ModemInit"))						// is in ENV:ModemInitString
				setstring(misc_data->STR_ModemInit, pc_data.Contents);
			if(!stricmp(pc_data.Argument, "DialPrefix"))						// is in ENV:ModemDialPrefix
				setstring(misc_data->PO_DialPrefix, pc_data.Contents);
		}
		ParseEnd(&pc_data);
	}
	return(NULL);
}

ULONG AmiTCPPrefs_Finish(struct IClass *cl, Object *obj, struct MUIP_AmiTCPPrefs_Finish *msg)
{
//	struct AmiTCPPrefs_Data *data = INST_DATA(CL_AmiTCPPrefs->mcc_Class, window);

	if(msg->level)
	{
// save to env:
	}
	if(msg->level == 2)
	{
// copy amitcp:db/provider.conf to appropriate file in amitcp:providers/...
	}

	DoMethod((Object *)xget(obj, MUIA_ApplicationObject), MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

	return(NULL);
}

ULONG AmiTCPPrefs_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct AmiTCPPrefs_Data tmp;

	STR_GR_Register[0] = GetStr(MSG_GR_Register0);
	STR_GR_Register[1] = GetStr(MSG_GR_Register1);
	STR_GR_Register[2] = GetStr(MSG_GR_Register2);
	STR_GR_Register[3] = GetStr(MSG_GR_Register3);
	STR_GR_Register[4] = GetStr(MSG_GR_Register4);
	STR_GR_Register[5] = GetStr(MSG_GR_Register5);
	STR_GR_Register[6] = NULL;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title	, GetStr(MSG_WI_AmiTCPPrefs),
		MUIA_Window_ID		, MAKE_ID('A','R','E','F'),
		WindowContents		, VGroup,
			Child, tmp.GR_Register = RegisterObject,
				MUIA_Register_Titles, STR_GR_Register,
				MUIA_HelpNode, "GR_Register",
				Child, tmp.GR_Server			= NewObject(CL_ServerPrefs->mcc_Class			, NULL, TAG_DONE),
				Child, tmp.GR_User			= NewObject(CL_UserPrefs->mcc_Class				, NULL, TAG_DONE),
				Child, tmp.GR_Misc			= NewObject(CL_MiscPrefs->mcc_Class				, NULL, TAG_DONE),
				Child, tmp.GR_Interface		= NewObject(CL_InterfacePrefs->mcc_Class		, NULL, TAG_DONE),
				Child, tmp.GR_DialScript	= NewObject(CL_DialscriptPrefs->mcc_Class		, NULL, TAG_DONE),
				Child, tmp.GR_UserStartnet	= NewObject(CL_UserStartnetPrefs->mcc_Class	, NULL, TAG_DONE),
			End,
			Child, HGroup,
				MUIA_HelpNode			, "GR_PrefsControl",
				MUIA_Group_SameSize	, TRUE,
				Child, tmp.BT_Save	= SimpleButton(GetStr(MSG_BT_Save)),
				Child, HSpace(0),
				Child, tmp.BT_Use		= SimpleButton(GetStr(MSG_BT_Use)),
				Child, HSpace(0),
				Child, tmp.BT_Cancel	= SimpleButton(GetStr(MSG_BT_Cancel)),
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct AmiTCPPrefs_Data *data = INST_DATA(cl, obj);

		*data = tmp;

		DoMethod(obj				, MUIM_Notify, MUIA_Window_CloseRequest, TRUE	, obj, 2, MUIM_AmiTCPPrefs_Finish, 0);
		DoMethod(tmp.BT_Cancel	, MUIM_Notify, MUIA_Pressed				, FALSE	, obj, 2, MUIM_AmiTCPPrefs_Finish, 0);
		DoMethod(tmp.BT_Use		, MUIM_Notify, MUIA_Pressed				, FALSE	, obj, 2, MUIM_AmiTCPPrefs_Finish, 1);
		DoMethod(tmp.BT_Save		, MUIM_Notify, MUIA_Pressed				, FALSE	, obj, 2, MUIM_AmiTCPPrefs_Finish, 2);
	}

	return((ULONG)obj);
}

SAVEDS ASM ULONG AmiTCPPrefs_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW								: return(AmiTCPPrefs_New			(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_Finish		: return(AmiTCPPrefs_Finish		(cl, obj, (APTR)msg));
		case MUIM_AmiTCPPrefs_LoadConfig	: return(AmiTCPPrefs_LoadConfig	(cl, obj, (APTR)msg));

		case MUIM_ServerPrefs_PopList_Update	:
		{
			struct AmiTCPPrefs_Data *data = INST_DATA(cl, obj);
		 	return(DoMethodA(data->GR_Server, msg));
		}
	}
	return(DoSuperMethodA(cl, obj, msg));
}



/*
 * close our custom classes
 */

VOID exit_classes(VOID)
{
	if(CL_UserStartnetPrefs)	MUI_DeleteCustomClass(CL_UserStartnetPrefs);
	if(CL_DialscriptPrefs)		MUI_DeleteCustomClass(CL_DialscriptPrefs);
	if(CL_MiscPrefs)				MUI_DeleteCustomClass(CL_MiscPrefs);
	if(CL_InterfacePrefs)		MUI_DeleteCustomClass(CL_InterfacePrefs);
	if(CL_ServerPrefs)			MUI_DeleteCustomClass(CL_ServerPrefs);
	if(CL_UserPrefs)				MUI_DeleteCustomClass(CL_UserPrefs);
	if(CL_AmiTCPPrefs)			MUI_DeleteCustomClass(CL_AmiTCPPrefs);

	CL_AmiTCPPrefs		= CL_UserPrefs			=
	CL_ServerPrefs		= CL_InterfacePrefs	=
	CL_MiscPrefs		= CL_DialscriptPrefs	= NULL;
}


/*
 * initialize our custom classes
 */

BOOL init_classes(VOID)
{
	CL_AmiTCPPrefs			= MUI_CreateCustomClass(NULL, MUIC_Window		, NULL, sizeof(struct AmiTCPPrefs_Data)		, AmiTCPPrefs_Dispatcher);
	CL_UserPrefs			= MUI_CreateCustomClass(NULL, MUIC_Group		, NULL, sizeof(struct UserPrefs_Data)			, UserPrefs_Dispatcher);
	CL_ServerPrefs			= MUI_CreateCustomClass(NULL, MUIC_Group		, NULL, sizeof(struct ServerPrefs_Data)		, ServerPrefs_Dispatcher);
	CL_InterfacePrefs		= MUI_CreateCustomClass(NULL, MUIC_Group		, NULL, sizeof(struct InterfacePrefs_Data)	, InterfacePrefs_Dispatcher);
	CL_MiscPrefs			= MUI_CreateCustomClass(NULL, MUIC_Group		, NULL, sizeof(struct MiscPrefs_Data)			, MiscPrefs_Dispatcher);
	CL_DialscriptPrefs	= MUI_CreateCustomClass(NULL, MUIC_Group		, NULL, sizeof(struct DialscriptPrefs_Data)	, DialscriptPrefs_Dispatcher);
	CL_UserStartnetPrefs	= MUI_CreateCustomClass(NULL, MUIC_Group		, NULL, sizeof(struct UserStartnetPrefs_Data), UserStartnetPrefs_Dispatcher);

	if(CL_AmiTCPPrefs		&& CL_UserPrefs			&& CL_ServerPrefs		&& CL_InterfacePrefs	&&
		CL_MiscPrefs		&& CL_DialscriptPrefs	&& CL_UserStartnetPrefs)
		return(TRUE);

	exit_classes();
	return(FALSE);
}
