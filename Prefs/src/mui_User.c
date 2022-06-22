/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_User.h"
#include "protos.h"

///
/// external variables
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct MUI_CustomClass  *CL_Databases;
extern struct Hook objstrhook = { {NULL, NULL}, (VOID *)objstrfunc, NULL, NULL};
extern Object *win;

///


/// User_New
ULONG User_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct User_Data tmp;

   if(obj = tmp.GR_User = (Object *)DoSuperNew(cl, obj,
MUIA_InnerLeft, 0,
MUIA_InnerRight, 0,
MUIA_InnerBottom, 0,
MUIA_InnerTop, 0,
      Child, HVSpace,
      Child, MUI_MakeObject(MUIO_BarTitle, "User identification"),
      Child, ColGroup(2),
         Child, MakeKeyLabel2("  Login:", MSG_CC_LoginName),
         Child, tmp.STR_LoginName = MakeKeyString("", 80, MSG_CC_LoginName),
         Child, MakeKeyLabel2(MSG_LA_Password, MSG_CC_Password),
         Child, tmp.STR_Password = TextinputObject,
            MUIA_ControlChar     , *GetStr(MSG_CC_Password),
            MUIA_CycleChain      , 1,
            MUIA_Frame           , MUIV_Frame_String,
            MUIA_String_Secret   , TRUE,
            MUIA_String_MaxLen   , 80,
         End,
      End,
      Child, HVSpace,
      Child, MUI_MakeObject(MUIO_BarTitle, "Additional information"),
      Child, ColGroup(2),
         Child, MakeKeyLabel2(MSG_LA_RealName, MSG_CC_RealName),
         Child, tmp.STR_RealName = MakeKeyString("", 80, MSG_CC_RealName),
         Child, MakeKeyLabel2(MSG_LA_EMail, MSG_CC_EMail),
         Child, tmp.STR_EMail = MakeKeyString("", 80, MSG_CC_EMail),
         Child, MakeKeyLabel2(MSG_LA_Organisation, MSG_CC_Organisation),
         Child, tmp.STR_Organisation = MakeKeyString("Private User", 80, MSG_CC_Organisation),
      End,
      Child, HVSpace,
      TAG_MORE, msg->ops_AttrList))
   {
      struct User_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(data->STR_LoginName   , MUIA_ShortHelp, GetStr(MSG_Help_LoginName));
      set(data->STR_Password    , MUIA_ShortHelp, GetStr(MSG_Help_Password));
      set(data->STR_EMail       , MUIA_ShortHelp, GetStr(MSG_Help_EMail));
      set(data->STR_RealName    , MUIA_ShortHelp, GetStr(MSG_Help_RealName));
      set(data->STR_Organisation, MUIA_ShortHelp, GetStr(MSG_Help_Organisation));

      DoMethod(data->STR_LoginName, MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Password);
      DoMethod(data->STR_Password , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_EMail);
      DoMethod(data->STR_EMail    , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_RealName);
      DoMethod(data->STR_RealName , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Organisation);
   }
   return((ULONG)obj);
}

///
/// User_Dispatcher
SAVEDS ULONG User_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(User_New                      (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

