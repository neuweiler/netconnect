/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_PasswdReq.h"
#include "protos.h"

///

/// PasswdReq_New
ULONG PasswdReq_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct PasswdReq_Data tmp;
   Object *originator;

   originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , "Change password",
      MUIA_Window_ID       , MAKE_ID('P','A','S','S'),
      WindowContents       , VGroup,
         Child, ColGroup(2),
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, MakeKeyLabel2("  New Password:", "  p"),
            Child, tmp.STR_pw1 = TextinputObject,
               MUIA_ControlChar        , *GetStr("  p"),
               MUIA_CycleChain         , 1,
               MUIA_Frame              , MUIV_Frame_String,
               MUIA_Textinput_Secret   , TRUE,
               MUIA_Textinput_Multiline, FALSE,
               MUIA_Textinput_MaxLen   , 40,
            End,
            Child, MakeKeyLabel2("  Verify Password:", "  v"),
            Child, tmp.STR_pw2 = TextinputObject,
               MUIA_ControlChar        , *GetStr("  v"),
               MUIA_CycleChain         , 1,
               MUIA_Frame              , MUIV_Frame_String,
               MUIA_Textinput_Secret   , TRUE,
               MUIA_Textinput_Multiline, FALSE,
               MUIA_Textinput_MaxLen   , 40,
            End,
         End,
         Child, HGroup,
            Child, tmp.BT_Okay   = MakeButton("  _Okay"),
            Child, tmp.BT_Cancel = MakeButton("  _Cancel"),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct PasswdReq_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(obj, MUIA_Window_ActiveObject, data->STR_pw1);

      DoMethod(obj              , MUIM_Notify, MUIA_Window_CloseRequest, TRUE            ,
         MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator   , 3, MUIM_Genesis_Finish, obj, 0);
      DoMethod(data->BT_Cancel  , MUIM_Notify, MUIA_Pressed            , FALSE           ,
         MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator   , 3, MUIM_Genesis_Finish, obj, 0);
      DoMethod(data->BT_Okay    , MUIM_Notify, MUIA_Pressed            , FALSE           ,
         MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator   , 3, MUIM_Genesis_Finish, obj, 1);
      DoMethod(data->STR_pw1    , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime  , obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_pw2);
      DoMethod(data->STR_pw2    , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime  , obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->BT_Okay);
   }
   return((ULONG)obj);
}

///
/// PasswdReq_Dispatcher
SAVEDS ULONG PasswdReq_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(PasswdReq_New           (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

