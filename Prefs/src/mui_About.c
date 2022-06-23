/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/pragmas/nc_lib.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_About.h"
#include "mui_MainWindow.h"
#include "protos.h"
#include "images/logo.h"

///
/// external variables
extern struct Library *NetConnectBase;
extern ULONG logo_colors[];
extern UBYTE logo_body[];
extern Object *win, *app;

///

/// About_New
ULONG About_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct About_Data tmp;
   STRPTR info1, info2;

   if(!(info1 = AllocVec(512, MEMF_ANY)))
      return(NULL);
   if(!(info2 = AllocVec(512, MEMF_ANY)))
   {
      FreeVec(info1);
      return(NULL);
   }

   strcpy(info1, GetStr(MSG_TX_About1a));
   strncat(info1, " " VERTAG, 512);
#ifdef NETCONNECT
   strncat(info1, " - NetConnect", 512);
#endif
   strncat(info1, "\n\n\033n\033cCopyright © 1997-99 by\n", 512);
   strncat(info1, "\0338Michael Neuweiler & Active Technologies\0332\033n\033c\n", 512);
   strncat(info1, GetStr(MSG_TX_About1b), 512);

   // don't replace with sprintf, will overfill info2 and cause enf. hit !!
   strcpy(info2, GetStr(MSG_TX_About2));
   strncat(info2, NCL_GetOwner(), 512);
   strncat(info2, "\n(", 512);
   strncat(info2, NCL_GetSerial(), 512);
   strncat(info2, ")\n\n\033n\033cARexx port:\n\033i'", 512);
   strncat(info2, (STRPTR)xget(app, MUIA_Application_Base), 512);
   strncat(info2, "'\n", 512);

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title       , "GENESiSPrefs · Copyright Information",
      MUIA_Window_ID          , MAKE_ID('A','B','O','U'),
      MUIA_Window_RefWindow   , win,
      MUIA_Window_LeftEdge    , MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge     , MUIV_Window_TopEdge_Centered,
      MUIA_Window_Height      , MUIV_Window_Height_MinMax(50),
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
                  MUIA_Text_Contents, info1,
               End,
#ifdef DEMO
               Child, MUI_MakeObject(MUIO_HBar, 2),
               Child, TextObject,
#ifdef BETA
                  MUIA_Text_Contents, "\n\033b\033cBETA VERSION\n",
#else
                  MUIA_Text_Contents, "\n\033b\033cDEMO VERSION\n",
#endif
                  MUIA_Font         , MUIV_Font_Big,
               End,
#endif
               Child, MUI_MakeObject(MUIO_HBar, 2),
               Child, TextObject,
                  MUIA_Text_Contents, info2,
               End,
               Child, MUI_MakeObject(MUIO_HBar, 2),
               Child, TextObject,
                  MUIA_Text_Contents, GetStr(MSG_TX_About3),
               End,
               Child, MUI_MakeObject(MUIO_HBar, 2),
               Child, TextObject,
                  MUIA_Text_Contents, GetStr(MSG_TX_About4),
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
      struct About_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(tmp.BT_Button, MUIA_CycleChain, 1);
      set(obj, MUIA_Window_ActiveObject, tmp.BT_Button);

      DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE ,
         MUIV_Notify_Application, 6, MUIM_Application_PushMethod,
         win, 3, MUIM_MainWindow_AboutFinish, obj);
      DoMethod(tmp.BT_Button, MUIM_Notify, MUIA_Pressed, FALSE ,
         MUIV_Notify_Application, 6, MUIM_Application_PushMethod,
         win, 3, MUIM_MainWindow_AboutFinish, obj);
   }

   FreeVec(info1);
   FreeVec(info2);

   return((ULONG)obj);
}

///
/// About_Dispatcher
SAVEDS ASM ULONG About_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(About_New(cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

