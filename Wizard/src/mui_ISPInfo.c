/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui_ISPInfo.h"
#include "mui_MainWindow.h"
#include "protos.h"

#include "images/setup_page4.h"
///

/// external variables
extern Object *win;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct config Config;
extern char ip[];
extern BOOL use_loginscript, no_picture;
extern int addr_assign;

extern ULONG setup_page4_colors[];
extern struct BitMapHeader setup_page4_header[];
extern UBYTE setup_page4_body[];

///

/// ISPInfo_New
ULONG ISPInfo_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct ISPInfo_Data tmp;
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
         Child, MakeText(GetStr(MSG_TX_InfoIPAddress)),
         Child, HGroup,
            Child, tmp.CY_IPAddress = Cycle(ARR_CY_IPAddress),
            Child, tmp.STR_IPAddress = StringObject,
               MUIA_CycleChain      , 1,
               StringFrame,
               MUIA_String_MaxLen   , 20,
               MUIA_String_Contents , ip,
               MUIA_String_Accept, "1234567890.",
//               MUIA_Textinput_Multiline, FALSE,
            End,
         End,
         Child, HVSpace,
         Child, MakeText(GetStr(MSG_TX_InfoProtocol)),
         Child, tmp.CY_Protocol = Cycle(ARR_CY_Protocol),
         Child, HVSpace,
         Child, MakeText(GetStr(MSG_TX_InfoUseLoginScript)),
         Child, tmp.CY_Script = Cycle(ARR_CY_Script),
         Child, HVSpace,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct ISPInfo_Data *data = INST_DATA(cl, obj);
      struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);

      *data = tmp;

      set(data->CY_IPAddress, MUIA_Weight, 10);
      set(data->CY_IPAddress, MUIA_CycleChain, 1);
      set(data->CY_Protocol , MUIA_CycleChain, 1);
      set(data->CY_Script   , MUIA_CycleChain, 1);
      set(data->CY_IPAddress, MUIA_Cycle_Active, ((strcmp(ip, "0.0.0.0") && *ip && addr_assign == CNF_Assign_Static) ? 1 : 0));
      set(data->CY_Protocol , MUIA_Cycle_Active, (strcmp(Config.cnf_ifname, "ppp") ? 1 : 0));
      set(data->CY_Script   , MUIA_Cycle_Active, (use_loginscript ? 0 : 1));
      set(data->STR_IPAddress, MUIA_Disabled, !xget(data->CY_IPAddress, MUIA_Cycle_Active));

      set(data->STR_IPAddress    , MUIA_ShortHelp, GetStr(MSG_HELP_IPAddress));
      set(data->CY_IPAddress     , MUIA_ShortHelp, GetStr(MSG_HELP_DynamicStatic));
      set(data->CY_Protocol      , MUIA_ShortHelp, GetStr(MSG_HELP_Protocol));
      set(data->CY_Script        , MUIA_ShortHelp, GetStr(MSG_HELP_RecordScript));

      DoMethod(data->CY_IPAddress        , MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, data->STR_IPAddress, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
   }

   return((ULONG)obj);
}

///
/// ISPInfo_Dispatcher
SAVEDS ULONG ISPInfo_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(ISPInfo_New         (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

