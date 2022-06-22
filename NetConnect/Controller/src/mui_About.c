/// includes
#include "/includes.h"

#include "/NetConnect.h"
#include "/locale/Strings.h"
#include "mui.h"
#include "mui_About.h"
#include "protos.h"
#include "rev.h"

///

/// About_New
ULONG About_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct About_Data tmp;
   Object *originator;
#ifdef DEMO
   char demo[256];
   ULONG days_running;
#endif

#ifdef DEMO
   if((days_running = check_date()) > WARN_DAYS)
      sprintf(demo, "\n\033b\033cDEMO VERSION\nWARNING: Timeout limit exceeded !\nThis demo will become inoperative in %ld days.\n", MAX_DAYS - days_running);
   else
      sprintf(demo, "\n\033b\033cDEMO VERSION\033n\nThis version will timeout in %ld days.\n", WARN_DAYS - days_running);
#endif

   originator = (Object *)GetTagData(MUIA_NetConnect_Originator, 0, msg->ops_AttrList);

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title, GetStr(MSG_LA_About),
      MUIA_Window_ID   , MAKE_ID('A','B','O','U'),
      MUIA_Window_Height, MUIV_Window_Height_MinMax(100),
      WindowContents, VGroup,
         MUIA_Background, MUII_RequesterBack,
         Child, ScrollgroupObject,
            MUIA_CycleChain, 1,
            MUIA_Background, MUII_ReadListBack,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_Contents, VirtgroupObject,
               ReadListFrame,
               Child, TextObject,
                  MUIA_Font, MUIV_Font_Big,
                  MUIA_Text_Contents, "\n\033c\033bNetConnect "VERTAG"\n",
               End,
               Child, MUI_MakeObject(MUIO_HBar, 2),
#ifdef DEMO
               Child, TextObject,
                  MUIA_Text_Contents, demo,
                  MUIA_Font         , MUIV_Font_Big,
               End,
#endif
               Child, TextObject,
                  MUIA_Text_PreParse, "\033c",
                  MUIA_Text_Contents, GetStr(MSG_TX_About1),
               End,
               Child, MUI_MakeObject(MUIO_HBar, 2),
               Child, TextObject,
                  MUIA_Text_PreParse, "\033c",
                  MUIA_Text_Contents, GetStr(MSG_TX_About2),
               End,
            End,
         End,
         Child, HGroup,
            Child, HSpace(0),
            Child, tmp.BT_Button = SimpleButton(GetStr(MSG_BT_Okay)),
            Child, HSpace(0),
         End,
      End))
   {
      struct About_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      set(tmp.BT_Button, MUIA_CycleChain, 1);
      set(obj, MUIA_Window_ActiveObject, tmp.BT_Button);

      DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE , MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_ABOUTFINISH);
      DoMethod(data->BT_Button, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_ABOUTFINISH);
   }
   return((ULONG)obj);
}

///
/// About_Dispatcher
SAVEDS ASM ULONG About_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW : return(About_New(cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}
///

