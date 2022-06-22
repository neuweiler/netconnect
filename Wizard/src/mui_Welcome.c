/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui_Welcome.h"
#include "protos.h"

#include "images/logo.h"
///
/// external variables
extern ULONG logo_colors[];
extern UBYTE logo_body[];

///

/// Welcome_New
ULONG Welcome_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Welcome_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      Child, VGroup,
         GroupFrame,
         MUIA_Background, "2:9c9c9c9c,9c9c9c9c,9c9c9c9c",
         Child, CLabel(GetStr(MSG_TX_Welcome)),
         Child, HGroup,
            Child, HVSpace,
            Child, tmp.BC_Logo = BodychunkObject,
               MUIA_FixWidth             , LOGO_WIDTH,
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
      End,
      Child, ListviewObject,
         MUIA_Listview_Input  , FALSE,
         MUIA_Listview_List   , FloattextObject,
            MUIA_Frame, MUIV_Frame_ReadList,
            MUIA_Background         , MUII_ReadListBack,
            MUIA_Floattext_Text     , GetStr(MSG_TX_InfoWelcome),
            MUIA_Floattext_TabSize  , 4,
            MUIA_Floattext_Justify, TRUE,
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Welcome_Data *data = INST_DATA(cl, obj);

      *data = tmp;
   }

   return((ULONG)obj);
}

///
/// Welcome_Dispatcher
SAVEDS ULONG Welcome_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW    : return(Welcome_New         (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

