/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui_Sana2.h"
#include "mui_MainWindow.h"
#include "protos.h"

#include "images/setup_page7.h"
///
/// external variables
extern Object *win;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct Config Config;
extern BOOL no_picture;

extern ULONG setup_page7_colors[];
extern UBYTE setup_page7_body[];

///

/// Sana2_New
ULONG Sana2_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Sana2_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Group_Horiz, TRUE,
/*      Child, tmp.GR_Picture = VGroup,
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
*/      Child, VGroup,   // Choose sanaII device (try to find out configuration)
         Child, HVSpace,
         Child, ColGroup(2),
            Child, Label(GetStr(MSG_LA_SanaDeviceDriver)),
            Child, MakePopAsl(tmp.STR_SanaDevice = MakeString(Config.cnf_sana2device, MAXPATHLEN), MSG_TX_SanaDeviceDriver, FALSE),
            Child, Label(GetStr(MSG_LA_Unit)),
            Child, HGroup,
               Child, tmp.SL_SanaUnit = NumericbuttonObject,
                  MUIA_CycleChain      , 1,
                  MUIA_ControlChar     , *GetStr(MSG_CC_Unit),
                  MUIA_Numeric_Min     , 0,
                  MUIA_Numeric_Max     , 20,
                  MUIA_Numeric_Value   , Config.cnf_sana2unit,
               End,
               Child, HVSpace,
            End,
         End,
         Child, HVSpace,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Sana2_Data *data = INST_DATA(cl, obj);
      struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);

      *data = tmp;

      DoMethod(data->STR_SanaDevice      , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, mw_data->BT_Next);
   }

   return((ULONG)obj);
}

///
/// Sana2_Dispatcher
SAVEDS ULONG Sana2_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(Sana2_New         (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///


