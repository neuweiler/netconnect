#include "globals.c"
#include "protos.h"

/// About_New
ULONG About_New(struct IClass *cl, Object *obj, Msg msg)
{
   struct About_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title, VERS,
      MUIA_Window_Height, MUIV_Window_Height_MinMax(30),
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
            Child, HSpace(0),
            Child, tmp.BT_Button = MakeButton(MSG_BT_Okay),
            Child, HSpace(0),
         End,
      End))
   {
      struct About_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      set(tmp.BT_Button, MUIA_CycleChain, 1);
      set(obj, MUIA_Window_ActiveObject, tmp.BT_Button);

      DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE ,
         MUIV_Notify_Application, 6, MUIM_Application_PushMethod,
         win, 3, MUIM_MainWindow_AboutFinish, obj);
      DoMethod(data->BT_Button, MUIM_Notify, MUIA_Pressed, FALSE ,
         MUIV_Notify_Application, 6, MUIM_Application_PushMethod,
         win, 3, MUIM_MainWindow_AboutFinish, obj);
   }
   return((ULONG)obj);
}

///
/// About_Dispatcher
SAVEDS ASM ULONG About_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW : return(About_New      (cl,obj,(APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///

