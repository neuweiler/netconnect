#include "globals.c"
#include "protos.h"

/// InfoText_TimeTrigger
ULONG InfoText_TimeTrigger(struct IClass *cl, Object *obj, Msg msg)
{
	struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
	int ret = 0;

	if(strstr(serial_buffer, "OK") || strstr(serial_buffer_old1, "OK") || strstr(serial_buffer_old2, "OK") ||
		!strcmp(serial_buffer, "0\\r") || !strcmp(serial_buffer_old1, "0\\r") || !strcmp(serial_buffer_old2, "0\\r"))
	{
		mw_data->Page++;
		DoMethod(win, MUIM_MainWindow_SetPage);
		checking_modem = 0;
	}
	else
	{
		if(strstr(serial_buffer, "ERROR") || strstr(serial_buffer_old1, "ERROR") || strstr(serial_buffer_old2, "ERROR") ||
			!strcmp(serial_buffer, "4\\r") || !strcmp(serial_buffer_old1, "4\\r") || !strcmp(serial_buffer_old2, "4\\r"))
		{
			ret = 1;
			checking_modem = 0;
		}
		else
		{
			if(checking_modem > 10)
			{
				ret = 2;
				checking_modem = 0;
			}
			else
			{
				char buffer[81];

				EscapeString(buffer, xgetstr(mw_data->STR_ModemInit));
				send_serial(buffer);
				checking_modem++;
			}
		}
	}
	if(checking_modem == 0)  // we're finished, close info_win
	{
		DoMethod(app, MUIM_Application_PushMethod, win, 1, MUIM_MainWindow_CloseInfoWindow);

		if(ret == 1)
			MUI_Request(app, win, NULL, NULL, "*_Cancel", "The modem returned an ERROR !");
		if(ret == 2)
			MUI_Request(app, win, NULL, NULL, "*_Cancel", "The modem is not responding !");
	}

	return(FALSE);
}

///
/// InfoText_New
ULONG InfoText_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	if(obj = (Object *)DoSuperNew(cl, obj,
		TAG_MORE, msg->ops_AttrList))
	{
		struct InfoText_Data *data = INST_DATA(cl, obj);

		data->ihnode.ihn_Object = obj;
		data->ihnode.ihn_Millis = 1000;
		data->ihnode.ihn_Method = MUIM_InfoText_TimeTrigger;
		data->ihnode.ihn_Flags  = MUIIHNF_TIMER;
	}
	return((ULONG)obj);
}

///
/// InfoText_Setup
ULONG InfoText_Setup(struct IClass *cl, Object *obj, Msg msg)
{
	struct InfoText_Data *data = INST_DATA(cl, obj);

	if(!DoSuperMethodA(cl, obj, msg))
		return(FALSE);

	DoMethod(_app(obj), MUIM_Application_AddInputHandler, &data->ihnode);

	return(TRUE);
}

///
/// InfoText_Cleanup
ULONG InfoText_Cleanup(struct IClass *cl, Object *obj, Msg msg)
{
	struct InfoText_Data *data = INST_DATA(cl, obj);

	DoMethod(_app(obj), MUIM_Application_RemInputHandler, &data->ihnode);

	return(DoSuperMethodA(cl, obj, msg));
}

///
/// InfoText_Dispatcher
SAVEDS ASM ULONG InfoText_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW                   : return(InfoText_New          (cl, obj, (APTR)msg));
		case MUIM_Setup               : return(InfoText_Setup        (cl, obj, (APTR)msg));
		case MUIM_Cleanup             : return(InfoText_Cleanup      (cl, obj, (APTR)msg));
		case MUIM_InfoText_TimeTrigger: return(InfoText_TimeTrigger  (cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}

///

/// InfoWindow_Abort
ULONG InfoWindow_Abort(struct IClass *cl, Object *obj, Msg msg)
{
	checking_modem = 0;
	DoMethod(app, MUIM_Application_PushMethod, win, 1, MUIM_MainWindow_CloseInfoWindow);

	return(NULL);
}

///
/// InfoWindow_New
ULONG InfoWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
	struct InfoWindow_Data tmp;

	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Window_Title       , "Please wait...",
		MUIA_Window_CloseGadget , FALSE,
		MUIA_Window_RefWindow    , win,
		MUIA_Window_LeftEdge     , MUIV_Window_LeftEdge_Centered,
		MUIA_Window_TopEdge      , MUIV_Window_TopEdge_Centered,
		WindowContents          , VGroup,
			Child, tmp.TX_Info = NewObject(CL_InfoText->mcc_Class, NULL, TAG_DONE),
			Child, tmp.BU_Busy = BusyObject,
			End,
			Child, tmp.BT_Abort = MakeButton("  _Abort"),
		End,
		TAG_MORE, msg->ops_AttrList))
	{
		struct InfoWindow_Data *data = INST_DATA(cl, obj);

		*data = tmp;

		DoMethod(tmp.BT_Abort, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_InfoWindow_Abort);
	}
	return((ULONG)obj);
}

///
/// InfoWindow_Dispatcher
SAVEDS ASM ULONG InfoWindow_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW                      : return(InfoWindow_New          (cl, obj, (APTR)msg));
		case MUIM_InfoWindow_Abort       : return(InfoWindow_Abort        (cl, obj, (APTR)msg));
	}
	return(DoSuperMethodA(cl, obj, msg));
}

///

