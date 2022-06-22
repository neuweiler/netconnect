/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_Advanced.h"
#include "mui_MainWindow.h"
#include "protos.h"

#include "images/setup_page7.h"
///
/// external variables
extern Object *win;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct Config Config;
extern BOOL no_picture, easy_ppp;
extern struct ISP ISP;

extern ULONG setup_page7_colors[];
extern UBYTE setup_page7_body[];

///

/// Advanced_New
ULONG Advanced_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Advanced_Data tmp;
   static STRPTR ARR_RA_Advanced[3];

   ARR_RA_Advanced[0] = "Try to login now";
   ARR_RA_Advanced[1] = "Enter more info";
   ARR_RA_Advanced[2] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Group_Horiz, TRUE,
      Child, tmp.GR_Picture = VGroup,
         MUIA_ShowMe, !no_picture,
         Child, BodychunkObject,
            GroupFrame,
            InnerSpacing(0, 0),
            MUIA_FixWidth             , SETUP_PAGE7_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE7_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE7_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE7_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE7_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page7_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE7_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE7_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page7_colors,
         End,
         Child, HVSpace,
      End,
      Child, VGroup,
         GroupFrame,
         MUIA_Background, MUII_TextBack,
         Child, HVSpace,
         Child, HGroup,
            Child, HVSpace,
            Child, tmp.RA_Advanced = RadioObject,
               MUIA_CycleChain   , 1,
               MUIA_Radio_Entries, ARR_RA_Advanced,
               MUIA_Radio_Active , !easy_ppp,
            End,
            Child, HVSpace,
         End,
         Child, HVSpace,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Advanced_Data *data = INST_DATA(cl, obj);

      *data = tmp;
   }

   return((ULONG)obj);
}

///
/// Advanced_Dispatcher
SAVEDS ULONG Advanced_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                   : return(Advanced_New      (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

