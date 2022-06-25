#include "mui_About.h"

extern ULONG logo_colors[];
extern UBYTE logo_body[];
extern Object *win;

///
ULONG About_New(struct IClass *cl, Object *obj, Msg msg)
{
   struct About_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title, GetStr(MSG_LA_About),
      MUIA_Window_ID   , MAKE_ID('A','B','O','U'),
      MUIA_Window_Height, MUIV_Window_Height_MinMax(100),
      WindowContents, VGroup,
         MUIA_Background, MUII_RequesterBack,
         Child, HGroup,
            TextFrame,
            MUIA_Background, "2:ffffffff,ffffffff,ffffffff",
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
//               MUIA_Bitmap_Transparent   , 0,
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
                  MUIA_Font, MUIV_Font_Big,
                  MUIA_Text_Contents, "\n\033c\033bNetConnectPrefs "VERTAG"\n",
               End,
               Child, MUI_MakeObject(MUIO_HBar, 2),
#ifdef DEMO
               Child, TextObject,
                  MUIA_Text_PreParse, "\033b\033c",
                  MUIA_Text_Contents, "DEMO VERSION !\n\nAdding of new icons or docks has been disabled.\n",
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
            Child, tmp.BT_Button = MakeButton(MSG_BT_Okay),
            Child, HSpace(0),
         End,
      End))
   {
      struct About_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      set(obj, MUIA_Window_ActiveObject, tmp.BT_Button);

      DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_About_Finish, obj, 0);
      DoMethod(data->BT_Button, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_About_Finish, obj, 0);
   }
   return((ULONG)obj);
}

#ifdef __SASC
SAVEDS ASM ULONG About_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg) {
#else /* gcc */
ULONG About_Dispatcher()
{
   register struct IClass *cl __asm("a0");
   register Object *obj __asm("a2");
   register Msg msg __asm("a1");
#endif

   switch (msg->MethodID)
   {
      case OM_NEW : return(About_New(cl, obj, (APTR)msg));
   }
   return(DoSuperMethodA(cl, obj, msg));
}
