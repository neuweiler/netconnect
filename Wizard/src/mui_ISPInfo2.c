/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui_ISPInfo2.h"
#include "mui_MainWindow.h"
#include "protos.h"

#include "images/setup_page4.h"
///
/// external variables
extern Object *win;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct Config Config;
extern BOOL use_loginscript, no_picture;
extern int addr_assign;
extern struct Interface Iface;
extern struct ISP ISP;

extern ULONG setup_page4_colors[];
extern struct BitMapHeader setup_page4_header[];
extern UBYTE setup_page4_body[];

///

/// ISPInfo2_New
ULONG ISPInfo2_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct ISPInfo2_Data tmp;
   static STRPTR ARR_CY_Protocol[3], ARR_CY_IPAddress[3], ARR_CY_Script[3];

   ARR_CY_Protocol[0] = "ppp";
   ARR_CY_Protocol[1] = "slip";
   ARR_CY_Protocol[2] = NULL;

   ARR_CY_IPAddress[0] = GetStr(MSG_TX_Dynamic);
   ARR_CY_IPAddress[1] = GetStr(MSG_TX_Static);
   ARR_CY_IPAddress[2] = NULL;

   ARR_CY_Script[0] = GetStr(MSG_CY_Script_Record);
   ARR_CY_Script[1] = GetStr(MSG_CY_Script_DontRecord);
   ARR_CY_Script[2] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Group_Horiz, TRUE,
      Child, tmp.GR_Picture = VGroup,
         MUIA_ShowMe, !no_picture,
         Child, BodychunkObject,
            GroupFrame,
            InnerSpacing(0, 0),
            MUIA_FixWidth             , SETUP_PAGE4_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE4_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE4_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE4_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE4_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page4_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE4_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE4_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page4_colors,
         End,
         Child, HVSpace,
      End,
      Child, VGroup,   // enter/verify basic isp information
         GroupFrame,
         MUIA_Background, MUII_TextBack,
         Child, HVSpace,
         Child, MakeText(GetStr("  DNS1 Address")),
         Child, tmp.STR_DNS1 = StringObject,
            MUIA_CycleChain      , 1,
            StringFrame,
            MUIA_String_MaxLen   , 20,
//            MUIA_String_Contents , ISP.isp_dns1,
            MUIA_String_Accept, "1234567890.",
         End,
         Child, tmp.STR_DNS2 = StringObject,
            MUIA_CycleChain      , 1,
            StringFrame,
            MUIA_String_MaxLen   , 20,
//            MUIA_String_Contents , ISP.isp_dns2,
            MUIA_String_Accept, "1234567890.",
         End,
         Child, MakeText(GetStr("  Gateway Address")),
         Child, HGroup,
            Child, tmp.STR_Gateway = StringObject,
               MUIA_CycleChain      , 1,
               StringFrame,
               MUIA_String_MaxLen   , 20,
               MUIA_String_Contents , Iface.if_gateway,
               MUIA_String_Accept, "1234567890.",
            End,
         End,
         Child, HVSpace,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct ISPInfo2_Data *data = INST_DATA(cl, obj);

      *data = tmp;
   }

   return((ULONG)obj);
}

///
/// ISPInfo2_Dispatcher
SAVEDS ULONG ISPInfo2_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(ISPInfo2_New         (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

