/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui_SerialSana.h"
#include "protos.h"

#include "images/setup_page1.h"
///

/// external variables
extern BOOL no_picture;

extern ULONG setup_page1_colors[];
extern UBYTE setup_page1_body[];

///

/// SerialSana_New
ULONG SerialSana_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct SerialSana_Data tmp;
   static STRPTR ARR_RA_Interface[3];

   ARR_RA_Interface[0] = GetStr(MSG_RA_Interface_Serial);
   ARR_RA_Interface[1] = GetStr(MSG_RA_Interface_Sana);
   ARR_RA_Interface[2] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Group_Horiz, TRUE,
/*      Child, tmp.GR_Picture = VGroup,
         MUIA_ShowMe, !no_picture,
         Child, BodychunkObject,
            GroupFrame,
            InnerSpacing(0, 0),
            MUIA_FixWidth             , SETUP_PAGE1_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE1_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE1_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE1_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE1_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page1_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE1_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE1_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page1_colors,
         End,
         Child, HVSpace,
      End,
*/      Child, VGroup,   // Serial connection with Modem (PPP/Slip) or direct connection (Ethernet/Arcnet etc.) with Network card
         GroupFrame,
         Child, HVSpace,
         Child, HGroup,
            Child, HVSpace,
            Child, tmp.RA_Interface = RadioObject,
               MUIA_CycleChain   , 1,
               MUIA_Radio_Entries, ARR_RA_Interface,
            End,
            Child, HVSpace,
         End,
         Child, HVSpace,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct SerialSana_Data *data = INST_DATA(cl, obj);

      *data = tmp;
   }

   return((ULONG)obj);
}

///
/// SerialSana_Dispatcher
SAVEDS ULONG SerialSana_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(SerialSana_New         (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

