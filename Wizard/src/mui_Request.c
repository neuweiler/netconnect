/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Request.h"
#include "mui_MainWindow.h"
#include "protos.h"

///
/// external variables
extern Object *win, *app;

///

/// Request_Finish
ULONG Request_Finish(struct IClass *cl, Object *obj, struct MUIP_Request_Finish *msg)
{
   struct Request_Data *data = INST_DATA(cl, obj);

   if(data->buffer)
   {
      if(msg->ok)
         strcpy(data->buffer, (STRPTR)xget(data->STR_Input, MUIA_String_Contents));
      else
         *data->buffer = NULL;
   }

   if(data->proc)
      Signal((struct Task *)data->proc, SIGBREAKF_CTRL_D);

   set(app, MUIA_Application_Sleep, FALSE);

   DoMethod(app, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, obj);

   return(NULL);
}

///

/// Request_New
ULONG Request_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Request_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , "Genesis Wizard",
      MUIA_Window_ID       , MAKE_ID('R','E','Q','U'),
      MUIA_Window_RefWindow, win,
      MUIA_Window_LeftEdge , MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge  , MUIV_Window_TopEdge_Centered,
      WindowContents       , VGroup,
         Child, VGroup,
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, MakeText((STRPTR)GetTagData(MUIA_Request_Text, 0, msg->ops_AttrList)),
         End,
         Child, tmp.STR_Input = MakeString(NULL, 80),
         Child, MUI_MakeObject(MUIO_HBar, 2),
         Child, HGroup,
            Child, tmp.BT_Okay   = MakeButton("  _Okay"),
            Child, tmp.BT_Cancel = MakeButton("  _Cancel"),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Request_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->proc    = (struct Process *)GetTagData(MUIA_Request_Process, NULL, msg->ops_AttrList);
      data->buffer  = (STRPTR)GetTagData(MUIA_Request_Buffer , NULL, msg->ops_AttrList);

      set(obj, MUIA_Window_ActiveObject, data->STR_Input);

      DoMethod(obj              , MUIM_Notify, MUIA_Window_CloseRequest, TRUE            , obj, 2, MUIM_Request_Finish, FALSE);
      DoMethod(data->BT_Cancel  , MUIM_Notify, MUIA_Pressed            , FALSE           , obj, 2, MUIM_Request_Finish, FALSE);
      DoMethod(data->BT_Okay    , MUIM_Notify, MUIA_Pressed            , FALSE           , obj, 2, MUIM_Request_Finish, TRUE);
      DoMethod(data->STR_Input  , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime  , obj, 2, MUIM_Request_Finish, TRUE);
   }
   return((ULONG)obj);
}

///
/// Request_Dispatcher
SAVEDS ULONG Request_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW              : return(Request_New           (cl, obj, (APTR)msg));
      case MUIM_Request_Finish : return(Request_Finish        (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

