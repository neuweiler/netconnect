#include "globals.c"
#include "protos.h"

/// ModemProtocol_InitFromPC
ULONG ModemProtocol_InitFromPC(struct IClass *cl, Object *obj, struct MUIP_ModemProtocol_InitFromPC *msg)
{
	struct ModemProtocol_Data *data = INST_DATA(cl, obj);
	struct ModemProtocol *modem_protocol;

	if(modem_protocol = AllocVec(sizeof(struct ModemProtocol), MEMF_ANY))
	{
		do
		{
			if(!stricmp(msg->pc_data->Argument, "Modem"))
				break;

			if(!stricmp(msg->pc_data->Argument, "Protocol"))
				strncpy(modem_protocol->Name, msg->pc_data->Contents, 40);
			if(!stricmp(msg->pc_data->Argument, "InitString"))
			{
				strncpy(modem_protocol->InitString, msg->pc_data->Contents, 40);
				DoMethod(data->LV_Protocols, MUIM_List_InsertSingle, modem_protocol, MUIV_List_Insert_Bottom);
			}
		}  while(ParseNext(msg->pc_data));

		FreeVec(modem_protocol);
	}

	set(data->LV_Protocols, MUIA_List_Active, MUIV_List_Active_Top);

	return(NULL);
}

///
/// ModemProtocol_Use
ULONG ModemProtocol_Use(struct IClass *cl, Object *obj, Msg msg)
{
	struct ModemProtocol_Data *data = INST_DATA(cl, obj);
	struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
	struct ModemProtocol *modem_protocol;

	DoMethod(data->LV_Protocols, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem_protocol);
	if(modem_protocol)
	{
		strcpy(Config.cnf_initstring, modem_protocol->InitString);
		set(mw_data->TX_InitString, MUIA_Text_Contents, modem_protocol->InitString);
	}

	DoMethod(_app(obj), MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);

	return(NULL);
}

///
/// ModemProtocol_ConstructFunc
SAVEDS ASM struct ModemProtocol *ModemProtocol_ConstructFunc(REG(a2) APTR pool, REG(a1) struct ModemProtocol *src)
{
	struct ModemProtocol *new;

	if((new = (struct ModemProtocol *)AllocVec(sizeof(struct ModemProtocol), MEMF_ANY | MEMF_CLEAR)) && src)
		memcpy(new, src, sizeof(struct ModemProtocol));
	return(new);
}

///
/// ModemProtocol_New
ULONG ModemProtocol_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	static const struct Hook ModemProtocol_ConstructHook= { { 0,0 }, (VOID *)ModemProtocol_ConstructFunc, NULL, NULL };
	struct ModemProtocol_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title        , GetStr(MSG_LA_SelectProtocol),
		MUIA_Window_CloseGadget  , FALSE,
		MUIA_Window_RefWindow    , win,
		MUIA_Window_LeftEdge     , MUIV_Window_LeftEdge_Centered,
		MUIA_Window_TopEdge      , MUIV_Window_TopEdge_Centered,
		WindowContents          , VGroup,
			Child, MakeText(MSG_TX_ProtocolInfo),
			Child, tmp.LV_Protocols = ListviewObject,
				MUIA_Listview_List, ListObject,
					MUIA_Frame              , MUIV_Frame_InputList,
					MUIA_List_ConstructHook , &ModemProtocol_ConstructHook,
					MUIA_List_DestructHook  , &deshook,
				End,
			End,
			Child, tmp.BT_Use = MakeButton(MSG_BT_UseProtocol),
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct ModemProtocol_Data *data = INST_DATA(cl, obj);

		*data = tmp;

		set(obj, MUIA_Window_ActiveObject, data->LV_Protocols);

		DoMethod(data->BT_Use      , MUIM_Notify, MUIA_Pressed               , FALSE           , obj, 1, MUIM_ModemProtocol_Use);
		DoMethod(data->LV_Protocols, MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime  , obj, 1, MUIM_ModemProtocol_Use);
	}
	return((ULONG)obj);
}

///
/// ModemProtocol_Dispatcher
SAVEDS ASM ULONG ModemProtocol_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW                         : return(ModemProtocol_New         (cl, obj, (APTR)msg));
		case MUIM_ModemProtocol_Use         : return(ModemProtocol_Use         (cl, obj, (APTR)msg));
		case MUIM_ModemProtocol_InitFromPC  : return(ModemProtocol_InitFromPC  (cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}

///

