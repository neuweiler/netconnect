#include "globals.c"
#include "globals2.c"
#include "protos.h"

/// MainWindow_About
ULONG MainWindow_About(struct IClass *cl, Object *obj, Msg msg)
{
	MUI_Request(app, win, NULL, NULL, "*_OK", "\033b\033c" VERS "\033n\033c\n\nMain programming by Michael Neuweiler\n\n\033bTHIS IS A DEMO VERSION !\033n\n\nAREXX port: '%ls'", xget(app, MUIA_Application_Base));
	return(NULL);
}

///
/// MainWindow_NextPage
ULONG MainWindow_NextPage(struct IClass *cl, Object *obj, Msg msg)
{
	struct MainWindow_Data *data = INST_DATA(cl, obj);

	switch(data->Page)
	{
		case 1:
			if(xget(data->RA_Interface, MUIA_Radio_Active))
				data->Page = 8;
			else
				data->Page++;
			break;
		case 2:
			if(open_serial(xgetstr(data->STR_SerialDevice), xget(data->SL_SerialUnit, MUIA_Numeric_Value)))
				data->Page++;
			else
				MUI_Request(app, win, NULL, NULL, "*_Cancel", "Can't open '%ls' unit %ld.\n\nMaybe the device is currently beeing\nused by another application.", xgetstr(data->STR_SerialDevice), xget(data->SL_SerialUnit, MUIA_Numeric_Value));
			break;
		case 3:
		{
			char buffer[81];

			set(app, MUIA_Application_Sleep, TRUE);
			if(info_win = NewObject(CL_InfoWindow->mcc_Class, NULL, TAG_DONE))
			{
				struct InfoWindow_Data *data = INST_DATA(CL_InfoWindow->mcc_Class, info_win);

				DoMethod(app, OM_ADDMEMBER, info_win);
				set(data->TX_Info, MUIA_Text_Contents, "checking modem...");
				set(info_win, MUIA_Window_Open, TRUE);
			}

			serial_buffer_old2[0] =
			serial_buffer_old1[0] =
			serial_buffer[0]      = NULL;
			ser_buf_pos = 0;
			checking_modem = 1;
//         StartTimer(1, 0);
			EscapeString(buffer, xgetstr(data->STR_ModemInit));
			send_serial(buffer);
			return(NULL);
		}
			break;
		case 8:
MUI_Request(app,win,0,0,"ok","would open sanaII device now and gather network info");
			data->Page--;
			break;
		default:
			data->Page++;
			break;
	}
	DoMethod(obj, MUIM_MainWindow_SetPage);
	return(NULL);
}

///
/// MainWindow_BackPage
ULONG MainWindow_BackPage(struct IClass *cl, Object *obj, Msg msg)
{
	struct MainWindow_Data *data = INST_DATA(cl, obj);

	switch(data->Page)
	{
		case 8:
			data->Page = 1;
			break;
		case 3:
//MUI_Request(app,win,0,0,"ok","would close serial device now");
			close_serial();
			data->Page--;
			break;
		case 7:
			if(xget(data->RA_Interface, MUIA_Radio_Active))
			{
//MUI_Request(app,win,0,0,"ok","would close sanaII device now");
				data->Page = 8;
			}
			else
				data->Page--;
			break;
		default:
			data->Page--;
			break;
	}
	DoMethod(obj, MUIM_MainWindow_SetPage);
	return(NULL);
}

///
/// MainWindow_SetPage
ULONG MainWindow_SetPage(struct IClass *cl, Object *obj, Msg msg)
{
	struct MainWindow_Data *data = INST_DATA(cl, obj);
	Object *new = NULL;

	if(data->Page < 0)
		data->Page = 0;
	if(data->Page > NUM_PAGES - 1)
		data->Page = NUM_PAGES - 1;

	set(data->BT_Back, MUIA_Disabled, (data->Page == 0));
	set(data->BT_Next, MUIA_Disabled, (data->Page == 7));

	new = data->ARR_Pages[data->Page];
	if(new && new != data->GR_Active)
	{
		DoMethod(group, MUIM_Group_InitChange);
		DoMethod(data->GR_Picture, MUIM_Group_InitChange);

		DoMethod(data->GR_Picture, OM_REMMEMBER, data->BC_Active);
		DoMethod(group, OM_ADDMEMBER, data->BC_Active);
		DoMethod(group, OM_REMMEMBER, data->ARR_Pictures[data->Page]);
		DoMethod(data->GR_Picture, OM_ADDMEMBER, data->ARR_Pictures[data->Page]);
		data->BC_Active = data->ARR_Pictures[data->Page];

		DoMethod(data->GR_Picture, MUIM_Group_ExitChange);
		set(data->FT_Info, MUIA_Floattext_Text, data->ARR_FT_Infos[data->Page]);
		DoMethod(data->GR_Pager, MUIM_Group_InitChange);

		DoMethod(data->GR_Pager, OM_REMMEMBER, data->GR_Active);
		DoMethod(group, OM_ADDMEMBER, data->GR_Active);
		DoMethod(group, OM_REMMEMBER, new);
		DoMethod(data->GR_Pager, OM_ADDMEMBER, new);
		data->GR_Active = new;

		DoMethod(data->GR_Pager, MUIM_Group_ExitChange);
		DoMethod(group, MUIM_Group_ExitChange);
	}

	return(NULL);
}

///
/// MainWindow_PopString
ULONG MainWindow_PopString(struct IClass *cl, Object *obj, struct MUIP_MainWindow_PopString *msg)
{
	struct MainWindow_Data *data = INST_DATA(cl, obj);
	Object *list_view = NULL, *string = NULL;
	STRPTR x;

	switch(msg->flags)
	{
		case MUIV_MainWindow_PopString_SerialDevice:
			list_view   = data->LV_SerialDevices;
			string      = data->PO_SerialDevice;
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

///
/// MainWindow_ModemList_ConstructFunc
SAVEDS ASM struct Modem *MainWindow_ModemList_ConstructFunc(REG(a2) APTR pool, REG(a1) struct Modem *src)
{
	struct Modem *new;

	if((new = (struct Modem *)AllocVec(sizeof(struct Modem), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct Modem));
	return(new);
}

///
/// MainWindow_ModemActive
ULONG MainWindow_ModemActive(struct IClass *cl, Object *obj, Msg msg)
{
	struct MainWindow_Data *data = INST_DATA(cl, obj);
	struct Modem *modem;

	DoMethod(data->LV_Modems, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem);
	if(modem)
	{
		setstring(data->STR_ModemName, modem->Name);
		setstring(data->STR_ModemInit, modem->InitString);
	}

	return(NULL);
}
///
/// create_providerlist
VOID create_providerlist(Object *obj, STRPTR path, struct MUIS_Listtree_TreeNode *list)
{
	BPTR lock;
	struct FileInfoBlock *fib;
	struct MUIS_Listtree_TreeNode *tn;
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
							DoMethod(obj, MUIM_Listtree_Insert, fib->fib_FileName, new_path, list, MUIV_Listtree_Insert_PrevNode_Sorted, TNF_LIST);
						else
						{
							if(tn = (struct MUIS_Listtree_TreeNode *)DoMethod(obj, MUIM_Listtree_Insert, fib->fib_FileName, NULL, list, MUIV_Listtree_Insert_PrevNode_Sorted, TNF_LIST))
								create_providerlist(obj, new_path, tn);
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
/// MainWindow_CreateProviderList
ULONG MainWindow_CreateProviderList(struct IClass *cl, Object *obj, struct MUIP_MainWindow_CreateProviderList *msg)
{
	struct MainWindow_Data *data = INST_DATA(cl, obj);

	set(data->LT_Providers, MUIA_Listtree_Quiet, TRUE);
	DoMethod(data->LT_Providers, MUIM_Listtree_Remove, MUIV_Listtree_Remove_ListNode_Root, MUIV_Listtree_Remove_TreeNode_All, NULL);
	create_providerlist(data->LT_Providers, msg->path, MUIV_Listtree_Insert_ListNode_Root);
	set(data->LT_Providers, MUIA_Listtree_Quiet, FALSE);

	return(NULL);
}

///
/// MainWindow_ProviderList_OpenFunc
SAVEDS ASM LONG MainWindow_ProviderList_OpenFunc(REG(a1) struct MUIS_Listtree_TreeNode *tn)
{
	struct pc_Data pc_data;

	if(tn && tn->tn_User)
	{
		if(!DoMethod(LT_Providers, MUIM_Listtree_GetEntry, tn, 0, MUIV_Listtree_GetEntry_Flags_SameLevel))
		{
			if(ParseConfig(tn->tn_User, &pc_data))
			{
				set(LT_Providers, MUIA_Listtree_Quiet, TRUE);
				while(ParseNext(&pc_data))
				{
					if(*pc_data.Argument == '#' && !stricmp(pc_data.Contents, "POPList"))
					{
						while(ParseNext(&pc_data))
						{
							if(*pc_data.Argument == '#')
								break;

							if(*pc_data.Argument)
								DoMethod(LT_Providers, MUIM_Listtree_Insert, pc_data.Argument, pc_data.Contents, tn, MUIV_Listtree_Insert_PrevNode_Sorted, 0);
						}
					}
				}
				ParseEnd(&pc_data);
				set(LT_Providers, MUIA_Listtree_Quiet, FALSE);
			}
		}
	}
	return(NULL);
}

///
/// MainWindow_ProviderList_DisplayFunc
SAVEDS ASM LONG MainWindow_ProviderList_DisplayFunc(REG(a2) char **array, REG(a1) struct MUIS_Listtree_TreeNode *node)
{
	if(node)
	{
		*array++ = node->tn_Name;
		*array = ((node->tn_User && !(node->tn_Flags & TNF_LIST)) ? (STRPTR)node->tn_User : (STRPTR)"");
	}
	else
	{
		static char buf1[50], buf2[30];

		strcpy(buf1, "\33b");
		strcpy(buf2, "\33b\33l");
		strcat(buf1, "Country / Provider / PoP");
		strcat(buf2, "Phone Number");
		*array++ = buf1;
		*array   = buf2;
	}

	return(NULL);
}

///
/// MainWindow_ProviderSelect
ULONG MainWindow_ProviderSelect(struct IClass *cl, Object *obj, Msg msg)
{
	struct MainWindow_Data *data = INST_DATA(cl, obj);
	struct MUIS_Listtree_TreeNode *tn;

	if(tn = (struct MUIS_Listtree_TreeNode *)DoMethod(data->LT_Providers, MUIM_Listtree_GetEntry, NULL, MUIV_Listtree_GetEntry_Position_Active, 0))
	{
		if(tn->tn_Flags & TNF_LIST)
			DoMethod(data->LT_Providers, MUIM_Listtree_Open, NULL, tn, NULL);
		else
		{
			char buffer[MAXPATHLEN];
			struct MUIS_Listtree_TreeNode *parent_tn1, *parent_tn2;

			buffer[0] = NULL;
			if(parent_tn1 = (struct MUIS_Listtree_TreeNode *)DoMethod(data->LT_Providers, MUIM_Listtree_GetEntry, tn, MUIV_Listtree_GetEntry_Position_Parent, 0))
			{
				if(parent_tn2 = (struct MUIS_Listtree_TreeNode *)DoMethod(data->LT_Providers, MUIM_Listtree_GetEntry, parent_tn1, MUIV_Listtree_GetEntry_Position_Parent, 0))
					sprintf(buffer, "%ls/%ls/", parent_tn2->tn_Name, parent_tn1->tn_Name);
				else
					sprintf(buffer, "%ls/", parent_tn1->tn_Name);
			}
			strncat(buffer, tn->tn_Name, MAXPATHLEN);
			set(data->TX_PoP, MUIA_Text_Contents, buffer);
			set(data->TX_Phone, MUIA_Text_Contents, tn->tn_User);

		}
	}
	return(NULL);
}

///
/// MainWindow_Dial
ULONG MainWindow_Dial(struct IClass *cl, Object *obj, Msg msg)
{
	struct MainWindow_Data *data = INST_DATA(cl, obj);
	char buffer1[240], buffer2[81];

	EscapeString(buffer1, xgetstr(data->STR_DialPrefix));
	strcat(buffer1, xgetstr(data->STR_PhoneNumber));
	EscapeString(buffer2, xgetstr(data->STR_DialSuffix));
	strcat(buffer1, buffer2);
	send_serial(buffer1);

	return(NULL);
}

///
/// MainWindow_GoOnline
ULONG MainWindow_GoOnline(struct IClass *cl, Object *obj, Msg msg)
{
MUI_Request(app, win, 0,0,"eh, sorry ! :)", "kiddin ?!?");
	return(NULL);
}

///
/// MainWindow_HangUp
ULONG MainWindow_HangUp(struct IClass *cl, Object *obj, Msg msg)
{
	struct MainWindow_Data *data = INST_DATA(cl, obj);

	close_serial();
	Delay(70);
	open_serial(xgetstr(data->STR_SerialDevice), xget(data->SL_SerialUnit, MUIA_Numeric_Value));

	if(serial_carrier())
	{
		send_serial("+");
		Delay(20);
		send_serial("+");
		Delay(20);
		send_serial("+");
		Delay(20);
		send_serial("ATH0\r");
	}
	else
		send_serial("\r");

	return(NULL);
}

///
/// MainWindow_SendLogin
ULONG MainWindow_SendLogin(struct IClass *cl, Object *obj, Msg msg)
{
	struct MainWindow_Data *data = INST_DATA(cl, obj);
	char buffer[82];

	EscapeString(buffer, xgetstr(data->STR_UserName));
	strcat(buffer, "\r");
	send_serial(buffer);

	return(NULL);
}

///
/// MainWindow_SendPassword
ULONG MainWindow_SendPassword(struct IClass *cl, Object *obj, Msg msg)
{
	struct MainWindow_Data *data = INST_DATA(cl, obj);
	char buffer[82];

	EscapeString(buffer, xgetstr(data->STR_Password));
	strcat(buffer, "\r");
	send_serial(buffer);

	return(NULL);
}

///
/// MainWindow_SendBreak
ULONG MainWindow_SendBreak(struct IClass *cl, Object *obj, Msg msg)
{
	WriteSER->IOSer.io_Command = SDCMD_BREAK;
	DoIO(WriteSER);

	return(NULL);
}

///
/// MainWindow_CloseInfoWindow
ULONG MainWindow_CloseInfoWindow(struct IClass *cl, Object *obj, Msg msg)
{
	if(info_win)
	{
		set(info_win, MUIA_Window_Open, FALSE);
		DoMethod(app, OM_REMMEMBER, info_win);
		MUI_DisposeObject(info_win);
		info_win = NULL;
	}

	set(app, MUIA_Application_Sleep, FALSE);

	return(NULL);
}

///
/// MainWindow_Input
ULONG MainWindow_Input(struct IClass *cl, Object *obj, struct MUIP_MainWindow_Input *msg)
{
//   struct MainWindow_Data *data = INST_DATA(cl, obj);

Printf("input event: class=%ld code=%ld qualifier=%ld\n", msg->ie->ie_Class, msg->ie->ie_Code, msg->ie->ie_Qualifier);

	return(NULL);
}

///
/// MainWindow_New
ULONG MainWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct MainWindow_Data tmp;
	Object *GR_Pages;
	static STRPTR ARR_RA_Interface[3], ARR_RA_Provider[3], ARR_CY_Protocol[3], ARR_CY_IPAddress[3];
	static const struct Hook MainWindow_ModemList_ConstructHook= { { 0,0 }, (VOID *)MainWindow_ModemList_ConstructFunc , NULL, NULL };
	static const struct Hook MainWindow_ProviderList_OpenHook= { { 0,0 }, (VOID *)MainWindow_ProviderList_OpenFunc , NULL, NULL };
	static const struct Hook MainWindow_ProviderList_DisplayHook= { { 0,0 }, (VOID *)MainWindow_ProviderList_DisplayFunc , NULL, NULL };

	ARR_RA_Interface[0] = "\nModem over serial line (ppp or slip)\n";
	ARR_RA_Interface[1] = "\nNetwork card (Ethernet, Arcanet, etc.)\nSana-II driver required !";
	ARR_RA_Interface[2] = NULL;

	ARR_RA_Provider[0] = "Use selected provider";
	ARR_RA_Provider[1] = "Provider not found, enter manually";
	ARR_RA_Provider[2] = NULL;

	ARR_CY_Protocol[0] = "ppp";
	ARR_CY_Protocol[1] = "slip";
	ARR_CY_Protocol[2] = NULL;

	ARR_CY_IPAddress[0] = "dynamic";
	ARR_CY_IPAddress[1] = "static";
	ARR_CY_IPAddress[2] = NULL;

	tmp.ARR_FT_Infos[0] = "This wizard helps you to configure AmiTCP very easily.\n\nIt will guide you through several pages where you can enter the information which is needed to get connected to the network. After having entered the required data, please use \"Next\" or \"Back\" to get to the next or last page.\n\nAt the end AmiTCP will try to connect to your provider and gather some network configuration details automatically.\n\nPlease click on \"Next\" now to get to the next page.";
	tmp.ARR_FT_Infos[1] = "Please choose the type of interface you are using.";
	tmp.ARR_FT_Infos[2] = "Please choose the device for your serial interface";
	tmp.ARR_FT_Infos[3] = "choose which modem you are using or enter the required command strings by hand.";
	tmp.ARR_FT_Infos[4] = "If your provider is listed you can choose it here in the listtree and a predefined configuration will be used. Otherwise you'll have to enter some settings yourself";
	tmp.ARR_FT_Infos[5] = "Please enter or verify the required ISP configuration";
	tmp.ARR_FT_Infos[6] = "Dialing...";
	tmp.ARR_FT_Infos[7] = "AmiTCP has now gathered all the necessary information to establish a network connection. Please choose whether you want to use this information to configure AmiTCP now or not.";
	tmp.ARR_FT_Infos[8] = "Please choose the device for your network interface";

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title    , "Wizard for setup of AmiTCP © 1997 by Michael Neuweiler, Active Software",
		MUIA_Window_ID       , MAKE_ID('M','A','I','N'),
		MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, MainMenu, NULL),
		WindowContents       , VGroup,
			Child, HGroup,
				Child, VGroup,
					GroupSpacing(0),
					Child, tmp.GR_Picture = VGroup,
						GroupSpacing(0),
						Child, tmp.ARR_Pictures[0] = tmp.BC_Active = BodychunkObject,
							ReadListFrame,
							MUIA_FixWidth             , SETUP_PAGE0_WIDTH,
							MUIA_FixHeight            , SETUP_PAGE0_HEIGHT,
							MUIA_Bitmap_Width         , SETUP_PAGE0_WIDTH ,
							MUIA_Bitmap_Height        , SETUP_PAGE0_HEIGHT,
							MUIA_Bodychunk_Depth      , SETUP_PAGE0_DEPTH ,
							MUIA_Bodychunk_Body       , (UBYTE *)setup_page0_body,
							MUIA_Bodychunk_Compression, SETUP_PAGE0_COMPRESSION,
							MUIA_Bodychunk_Masking    , SETUP_PAGE0_MASKING,
							MUIA_Bitmap_SourceColors  , (ULONG *)setup_page0_colors,
							MUIA_Bitmap_Transparent   , 0,
						End,
					End,
					Child, HVSpace,
				End,
				Child, VGroup,
					Child, ListviewObject,
						MUIA_Background, MUII_TextBack,
						MUIA_Listview_Input  , FALSE,
						MUIA_Listview_List   , tmp.FT_Info = FloattextObject,
							ReadListFrame,
							MUIA_Floattext_Text, tmp.ARR_FT_Infos[0],
						End,
					End,
					Child, BalanceObject, End,
					Child, tmp.GR_Pager = VGroup,
						GroupFrame,
						MUIA_Background, MUII_GroupBack,
						Child, tmp.ARR_Pages[0] = tmp.GR_Active = VGroup,
							Child, HVSpace,
							Child, CLabel("                              \033bWelcome to AmiTCP !\033n                              "),
							Child, HVSpace,
						End,
					End,
				End,
			End,
			Child, MUI_MakeObject(MUIO_HBar, 2),
			Child, HGroup,
				MUIA_Group_SameSize  , TRUE,
				GroupSpacing(1),
				Child, HVSpace,
				Child, HVSpace,
				Child, tmp.BT_Back   = MakeButton("   < _Back"),
				Child, tmp.BT_Next   = MakeButton("   _Next >"),
				Child, HSpace(0),
				Child, tmp.BT_Abort   = MakeButton("  _Abort"),
			End,
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct MainWindow_Data *data = INST_DATA(cl, obj);

		if(GR_Pages = VGroup,
			Child, tmp.ARR_Pages[1] = VGroup,   // Serial connection with Modem (PPP/Slip) or direct connection (Ethernet/Arcnet etc.) with Network card
				Child, HVSpace,
				Child, HGroup,
					Child, HVSpace,
					Child, tmp.RA_Interface = RadioObject,
						MUIA_Radio_Entries, ARR_RA_Interface,
					End,
					Child, HVSpace,
				End,
				Child, HVSpace,
			End,
			Child, tmp.ARR_Pages[2] = VGroup,   // Choose serial device (open device afterwards)
				Child, HVSpace,
				Child, ColGroup(2),
					Child, KeyLabel2("Serial Device:", 's'),
					Child, tmp.PO_SerialDevice = PopobjectObject,
						MUIA_Popstring_String      , tmp.STR_SerialDevice = MakeKeyString("serial.device", MAXPATHLEN, 'd'),
						MUIA_Popstring_Button      , PopButton(MUII_PopUp),
						MUIA_Popobject_StrObjHook  , &strobjhook,
						MUIA_Popobject_Object      , tmp.LV_SerialDevices = ListviewObject,
							MUIA_Listview_DoubleClick  , TRUE,
							MUIA_Listview_List         , ListObject,
								MUIA_Frame              , MUIV_Frame_InputList,
								MUIA_List_ConstructHook , MUIV_List_ConstructHook_String,
								MUIA_List_DestructHook  , MUIV_List_DestructHook_String,
								MUIA_List_CompareHook   , &sorthook,
								MUIA_List_AutoVisible   , TRUE,
							End,
						End,
					End,
					Child, KeyLabel("Unit:", 'u'),
					Child, HGroup,
						Child, tmp.SL_SerialUnit = NumericbuttonObject,
							MUIA_CycleChain      , 1,
							MUIA_ControlChar     , 'u',
							MUIA_Numeric_Min     , 0,
							MUIA_Numeric_Max     , 20,
							MUIA_Numeric_Value   , 0,
						End,
						Child, HVSpace,
					End,
				End,
				Child, HVSpace,
			End,
			Child, tmp.ARR_Pages[3] = HGroup,   // evtl. choose modem type/strings (send init string to modem => response: OK resp. 0, ERROR resp. 4, or nothing)
				Child, tmp.LV_Modems = ListviewObject,
					MUIA_Listview_Input           , TRUE,
					MUIA_Listview_List, ListObject,
						InputListFrame,
						MUIA_List_ConstructHook , &MainWindow_ModemList_ConstructHook,
						MUIA_List_DestructHook  , &deshook,
						MUIA_List_CompareHook   , &sorthook,
						MUIA_List_AutoVisible   , TRUE,
					End,
				End,
				Child, VGroup,
					Child, ColGroup(2),
						Child, Label2("Modem Name:"),
						Child, tmp.STR_ModemName = MakeKeyString("Generic", 80, 'o'),
						Child, Label2("Modem Init:"),
						Child, tmp.STR_ModemInit = MakeKeyString("AT&F&D0\\r", 80, 'i'),
						Child, Label2("Dial Prefix:"),
						Child, tmp.STR_DialPrefix = MakeKeyString("ATDT", 80, 'p'),
						Child, Label2("Dial Suffix:"),
						Child, tmp.STR_DialSuffix = MakeKeyString("\\r", 10, 's'),
					End,
					Child, HVSpace,
				End,
			End,
			Child, tmp.ARR_Pages[4] = VGroup,   // Choose provider from list if available
				Child, ListviewObject,
					MUIA_Listview_Input  , TRUE,
					MUIA_Listview_List   , tmp.LT_Providers = LT_Providers = ListtreeObject,   // need global var for OpenFunc
						InputListFrame,
						MUIA_Listtree_ConstructHook, MUIV_Listtree_ConstructHook_String,
						MUIA_Listtree_DestructHook , MUIV_Listtree_DestructHook_String,
						MUIA_Listtree_SortHook     , &sorthook,
						MUIA_Listtree_OpenHook     , &MainWindow_ProviderList_OpenHook,
						MUIA_Listtree_DisplayHook  , &MainWindow_ProviderList_DisplayHook,
						MUIA_Listtree_Format       , "BAR,",
						MUIA_Listtree_Title        , TRUE,
					End,
				End,
				Child, ColGroup(2),
					Child, Label("PoP:"),
					Child, tmp.TX_PoP = TextObject, TextFrame, MUIA_Background, MUII_TextBack, End,
					Child, Label("Phone:"),
					Child, HGroup,
						Child, tmp.TX_Phone = TextObject, TextFrame, MUIA_Background, MUII_TextBack, End,
						Child, tmp.BT_ClearProvider = MakeButton("  _Clear"),
					End,
				End,
			End,
			Child, tmp.ARR_Pages[5] = VGroup,   // enter/verify basic isp information
				Child, HVSpace,
				Child, ColGroup(2),
					Child, Label("User name:"),
					Child, tmp.STR_UserName = MakeKeyString("username", 80, 'u'),
					Child, Label("Password:"),
					Child, tmp.STR_Password = StringObject,
						MUIA_ControlChar     , 'p',
						MUIA_CycleChain      , 1,
						MUIA_Frame           , MUIV_Frame_String,
						MUIA_String_Secret   , TRUE,
						MUIA_String_MaxLen   , 80,
					End,
					Child, Label("Phone number:"),
					Child, tmp.STR_PhoneNumber = MakeKeyString("", 80, 'p'),
					Child, Label("IP address:"),
					Child, HGroup,
						Child, tmp.STR_IPAddress = StringObject,
							MUIA_ControlChar     , 'i',
							MUIA_CycleChain      , 1,
							StringFrame,
							MUIA_String_MaxLen   , 20,
							MUIA_String_Contents , "0.0.0.0",
							MUIA_String_Accept   , "1234567890.",
						End,
						Child, tmp.CY_IPAddress = Cycle(ARR_CY_IPAddress),
					End,
					Child, Label("Protocol:"),
					Child, tmp.CY_Protocol = MakeKeyCycle(ARR_CY_Protocol, 'p'),
				End,
				Child, HVSpace,
			End,
			Child, tmp.ARR_Pages[6] = VGroup,   // login script recorder
				GroupSpacing(0),
				Child, tmp.LV_Terminal = ListviewObject,
					MUIA_Listview_List, tmp.LI_Terminal = ListObject,
						InputListFrame,
						MUIA_Font, MUIV_Font_Fixed,
						MUIA_List_DestructHook  , &deshook, // no cons_hook since we're going to alloc the mem ourselfes - but don't free it
					End,
				End,
				Child, ColGroup(3),
					GroupSpacing(0),
					Child, tmp.BT_Dial = MakeButton("  _dial"),
					Child, tmp.BT_GoOnline = MakeButton("  go _online"),
					Child, tmp.BT_HangUp = MakeButton("  _hang up"),
					Child, tmp.BT_SendLogin = MakeButton("  send _user name"),
					Child, tmp.BT_SendPassword = MakeButton("  send _password"),
					Child, tmp.BT_SendBreak = MakeButton("  send _break"),
				End,
			End,
			Child, tmp.ARR_Pages[7] = VGroup,   // finished (option to save config and save/print info sheet)
				Child, HVSpace,
				Child, ColGroup(3),
					Child, Label("Save configuration:"),
					Child, CheckMark(TRUE),
					Child, MakePopAsl(tmp.STR_Config = MakeKeyString("PROGDIR:Setup.config", MAXPATHLEN, 's'), "Save configuration", FALSE),
					Child, Label("Save config information:"),
					Child, CheckMark(TRUE),
					Child, MakePopAsl(tmp.STR_Info = MakeKeyString("PROGDIR:Setup.txt", MAXPATHLEN, 'c'), "Save config information", FALSE),
					Child, Label("Print config information:"),
					Child, CheckMark(FALSE),
					Child, tmp.STR_Printer = MakeKeyString("PRT:", 80, 'p'),
				End,
				Child, HVSpace,
			End,
			Child, tmp.ARR_Pages[8] = VGroup,   // Choose sanaII device (try to find out configuration)
				Child, HVSpace,
				Child, ColGroup(2),
					Child, Label2("Device driver:"),
					Child, MakePopAsl(tmp.STR_SanaDevice = MakeKeyString("DEVS:Networks/", MAXPATHLEN, 's'), "Choose sanaII device", FALSE),
					Child, KeyLabel2("Unit:", 'u'),
					Child, HGroup,
						Child, tmp.SL_SanaUnit = NumericbuttonObject,
							MUIA_CycleChain      , 1,
							MUIA_ControlChar     , 'u',
							MUIA_Numeric_Min     , 0,
							MUIA_Numeric_Max     , 20,
							MUIA_Numeric_Value   , 0,
						End,
						Child, HVSpace,
					End,
				End,
				Child, HVSpace,
			End,

			Child, tmp.ARR_Pictures[1] = BodychunkObject,
				ReadListFrame,
				MUIA_FixWidth             , SETUP_PAGE1_WIDTH,
				MUIA_FixHeight            , SETUP_PAGE1_HEIGHT,
				MUIA_Bitmap_Width         , SETUP_PAGE1_WIDTH ,
				MUIA_Bitmap_Height        , SETUP_PAGE1_HEIGHT,
				MUIA_Bodychunk_Depth      , SETUP_PAGE1_DEPTH ,
				MUIA_Bodychunk_Body       , (UBYTE *)setup_page1_body,
				MUIA_Bodychunk_Compression, SETUP_PAGE1_COMPRESSION,
				MUIA_Bodychunk_Masking    , SETUP_PAGE1_MASKING,
				MUIA_Bitmap_SourceColors  , (ULONG *)setup_page1_colors,
				MUIA_Bitmap_Transparent   , 0,
			End,
			Child, tmp.ARR_Pictures[2] = BodychunkObject,
				ReadListFrame,
				MUIA_FixWidth             , SETUP_PAGE2_WIDTH,
				MUIA_FixHeight            , SETUP_PAGE2_HEIGHT,
				MUIA_Bitmap_Width         , SETUP_PAGE2_WIDTH ,
				MUIA_Bitmap_Height        , SETUP_PAGE2_HEIGHT,
				MUIA_Bodychunk_Depth      , SETUP_PAGE2_DEPTH ,
				MUIA_Bodychunk_Body       , (UBYTE *)setup_page2_body,
				MUIA_Bodychunk_Compression, SETUP_PAGE2_COMPRESSION,
				MUIA_Bodychunk_Masking    , SETUP_PAGE2_MASKING,
				MUIA_Bitmap_SourceColors  , (ULONG *)setup_page2_colors,
				MUIA_Bitmap_Transparent   , 0,
			End,
			Child, tmp.ARR_Pictures[3] = BodychunkObject,
				ReadListFrame,
				MUIA_FixWidth             , SETUP_PAGE3_WIDTH,
				MUIA_FixHeight            , SETUP_PAGE3_HEIGHT,
				MUIA_Bitmap_Width         , SETUP_PAGE3_WIDTH ,
				MUIA_Bitmap_Height        , SETUP_PAGE3_HEIGHT,
				MUIA_Bodychunk_Depth      , SETUP_PAGE3_DEPTH ,
				MUIA_Bodychunk_Body       , (UBYTE *)setup_page3_body,
				MUIA_Bodychunk_Compression, SETUP_PAGE3_COMPRESSION,
				MUIA_Bodychunk_Masking    , SETUP_PAGE3_MASKING,
				MUIA_Bitmap_SourceColors  , (ULONG *)setup_page3_colors,
				MUIA_Bitmap_Transparent   , 0,
			End,
			Child, tmp.ARR_Pictures[4] = BodychunkObject,
				ReadListFrame,
				MUIA_FixWidth             , SETUP_PAGE4_WIDTH,
				MUIA_FixHeight            , SETUP_PAGE4_HEIGHT,
				MUIA_Bitmap_Width         , SETUP_PAGE4_WIDTH ,
				MUIA_Bitmap_Height        , SETUP_PAGE4_HEIGHT,
				MUIA_Bodychunk_Depth      , SETUP_PAGE4_DEPTH ,
				MUIA_Bodychunk_Body       , (UBYTE *)setup_page4_body,
				MUIA_Bodychunk_Compression, SETUP_PAGE4_COMPRESSION,
				MUIA_Bodychunk_Masking    , SETUP_PAGE4_MASKING,
				MUIA_Bitmap_SourceColors  , (ULONG *)setup_page4_colors,
				MUIA_Bitmap_Transparent   , 0,
			End,
			Child, tmp.ARR_Pictures[5] = BodychunkObject,
				ReadListFrame,
				MUIA_FixWidth             , SETUP_PAGE5_WIDTH,
				MUIA_FixHeight            , SETUP_PAGE5_HEIGHT,
				MUIA_Bitmap_Width         , SETUP_PAGE5_WIDTH ,
				MUIA_Bitmap_Height        , SETUP_PAGE5_HEIGHT,
				MUIA_Bodychunk_Depth      , SETUP_PAGE5_DEPTH ,
				MUIA_Bodychunk_Body       , (UBYTE *)setup_page5_body,
				MUIA_Bodychunk_Compression, SETUP_PAGE5_COMPRESSION,
				MUIA_Bodychunk_Masking    , SETUP_PAGE5_MASKING,
				MUIA_Bitmap_SourceColors  , (ULONG *)setup_page5_colors,
				MUIA_Bitmap_Transparent   , 0,
			End,
			Child, tmp.ARR_Pictures[6] = BodychunkObject,
				ReadListFrame,
				MUIA_FixWidth             , SETUP_PAGE6_WIDTH,
				MUIA_FixHeight            , SETUP_PAGE6_HEIGHT,
				MUIA_Bitmap_Width         , SETUP_PAGE6_WIDTH ,
				MUIA_Bitmap_Height        , SETUP_PAGE6_HEIGHT,
				MUIA_Bodychunk_Depth      , SETUP_PAGE6_DEPTH ,
				MUIA_Bodychunk_Body       , (UBYTE *)setup_page6_body,
				MUIA_Bodychunk_Compression, SETUP_PAGE6_COMPRESSION,
				MUIA_Bodychunk_Masking    , SETUP_PAGE6_MASKING,
				MUIA_Bitmap_SourceColors  , (ULONG *)setup_page6_colors,
				MUIA_Bitmap_Transparent   , 0,
			End,
			Child, tmp.ARR_Pictures[7] = BodychunkObject,
				ReadListFrame,
				MUIA_FixWidth             , SETUP_PAGE7_WIDTH,
				MUIA_FixHeight            , SETUP_PAGE7_HEIGHT,
				MUIA_Bitmap_Width         , SETUP_PAGE7_WIDTH ,
				MUIA_Bitmap_Height        , SETUP_PAGE7_HEIGHT,
				MUIA_Bodychunk_Depth      , SETUP_PAGE7_DEPTH ,
				MUIA_Bodychunk_Body       , (UBYTE *)setup_page7_body,
				MUIA_Bodychunk_Compression, SETUP_PAGE7_COMPRESSION,
				MUIA_Bodychunk_Masking    , SETUP_PAGE7_MASKING,
				MUIA_Bitmap_SourceColors  , (ULONG *)setup_page7_colors,
				MUIA_Bitmap_Transparent   , 0,
			End,
			Child, tmp.ARR_Pictures[8] = BodychunkObject,
				ReadListFrame,
				MUIA_FixWidth             , SETUP_PAGE8_WIDTH,
				MUIA_FixHeight            , SETUP_PAGE8_HEIGHT,
				MUIA_Bitmap_Width         , SETUP_PAGE8_WIDTH ,
				MUIA_Bitmap_Height        , SETUP_PAGE8_HEIGHT,
				MUIA_Bodychunk_Depth      , SETUP_PAGE8_DEPTH ,
				MUIA_Bodychunk_Body       , (UBYTE *)setup_page8_body,
				MUIA_Bodychunk_Compression, SETUP_PAGE8_COMPRESSION,
				MUIA_Bodychunk_Masking    , SETUP_PAGE8_MASKING,
				MUIA_Bitmap_SourceColors  , (ULONG *)setup_page8_colors,
				MUIA_Bitmap_Transparent   , 0,
			End,
		End)
		{
			DoMethod(group, OM_ADDMEMBER, GR_Pages);
			group = GR_Pages;    // the old group isn't the parent of the pages => replace with correct group-pointer
			tmp.Page = 0;

			*data = tmp;

//      set(tmp.BT_Cancel , MUIA_ShortHelp, GetStr(MSG_Help_Cancel));
			set(obj, MUIA_Window_ActiveObject, tmp.BT_Next);
			set(tmp.BT_Back, MUIA_Disabled, TRUE);
			set(tmp.STR_IPAddress, MUIA_Disabled, TRUE);
			set(tmp.BT_ClearProvider, MUIA_Weight, 0);
			set(tmp.CY_IPAddress, MUIA_Weight, 10);

			DoMethod(obj            , MUIM_Notify, MUIA_Window_CloseRequest, TRUE, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
			DoMethod(obj            , MUIM_Notify, MUIA_Window_InputEvent, "-repeat f1", obj, 2, MUIM_MainWindow_Input, MUIV_TriggerValue);
			DoMethod(tmp.BT_Next    , MUIM_Notify, MUIA_Pressed   , FALSE  , obj, 1, MUIM_MainWindow_NextPage);
			DoMethod(tmp.BT_Back    , MUIM_Notify, MUIA_Pressed   , FALSE  , obj, 1, MUIM_MainWindow_BackPage);
			DoMethod(tmp.BT_Abort   , MUIM_Notify, MUIA_Pressed   , FALSE  , app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

			DoMethod(tmp.LV_SerialDevices    , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime  , obj, 2, MUIM_MainWindow_PopString, MUIV_MainWindow_PopString_SerialDevice);
			DoMethod(tmp.LV_Modems           , MUIM_Notify, MUIA_List_Active           , MUIV_EveryTime  , obj, 1, MUIM_MainWindow_ModemActive);
			DoMethod(tmp.LT_Providers        , MUIM_Notify, MUIA_Listtree_DoubleClick  , MUIV_EveryTime  , obj, 1, MUIM_MainWindow_ProviderSelect);

			DoMethod(tmp.BT_ClearProvider    , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 6, MUIM_MultiSet, MUIA_Text_Contents, "", tmp.TX_PoP, tmp.TX_Phone, NULL);

			DoMethod(tmp.CY_IPAddress        , MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, tmp.STR_IPAddress, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

			DoMethod(tmp.BT_Dial             , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_Dial);
			DoMethod(tmp.BT_GoOnline         , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_GoOnline);
			DoMethod(tmp.BT_HangUp           , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_HangUp);
			DoMethod(tmp.BT_SendLogin        , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_SendLogin);
			DoMethod(tmp.BT_SendPassword     , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_SendPassword);
			DoMethod(tmp.BT_SendBreak        , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_SendBreak);

			DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_ABOUT)    , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, obj, 1, MUIM_MainWindow_About);
			DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_ABOUT_MUI), MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_AboutMUI, win);
			DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_QUIT)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
			DoMethod((Object *)DoMethod(tmp.MN_Strip, MUIM_FindUData, MEN_MUI)      , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
		}
		else
		{
			MUI_DisposeObject(obj);
			obj = NULL;
		}
	}
	return((ULONG)obj);
}

///
/// MainWindow_Dispatcher
SAVEDS ASM ULONG MainWindow_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW                            : return(MainWindow_New                (cl, obj, (APTR)msg));
		case MUIM_MainWindow_SetPage           : return(MainWindow_SetPage            (cl, obj, (APTR)msg));
		case MUIM_MainWindow_NextPage          : return(MainWindow_NextPage           (cl, obj, (APTR)msg));
		case MUIM_MainWindow_BackPage          : return(MainWindow_BackPage           (cl, obj, (APTR)msg));
		case MUIM_MainWindow_About             : return(MainWindow_About              (cl, obj, (APTR)msg));
		case MUIM_MainWindow_PopString         : return(MainWindow_PopString          (cl, obj, (APTR)msg));
		case MUIM_MainWindow_ModemActive       : return(MainWindow_ModemActive        (cl, obj, (APTR)msg));
		case MUIM_MainWindow_CreateProviderList: return(MainWindow_CreateProviderList (cl, obj, (APTR)msg));
		case MUIM_MainWindow_ProviderSelect    : return(MainWindow_ProviderSelect     (cl, obj, (APTR)msg));
		case MUIM_MainWindow_Dial              : return(MainWindow_Dial               (cl, obj, (APTR)msg));
		case MUIM_MainWindow_GoOnline          : return(MainWindow_GoOnline           (cl, obj, (APTR)msg));
		case MUIM_MainWindow_HangUp            : return(MainWindow_HangUp             (cl, obj, (APTR)msg));
		case MUIM_MainWindow_SendLogin         : return(MainWindow_SendLogin          (cl, obj, (APTR)msg));
		case MUIM_MainWindow_SendPassword      : return(MainWindow_SendPassword       (cl, obj, (APTR)msg));
		case MUIM_MainWindow_SendBreak         : return(MainWindow_SendBreak          (cl, obj, (APTR)msg));
		case MUIM_MainWindow_CloseInfoWindow   : return(MainWindow_CloseInfoWindow    (cl, obj, (APTR)msg));
		case MUIM_MainWindow_Input             : return(MainWindow_Input              (cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}

///

