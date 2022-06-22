/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_UserInfo.h"
#include "mui_MainWindow.h"
#include "protos.h"

#include "images/setup_page3.h"
///
/// external variables
extern Object *win;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct Config Config;
extern BOOL no_picture;
extern struct ISP ISP;

extern ULONG setup_page3_colors[];
extern UBYTE setup_page3_body[];

///

/// UserInfo_AddPhone
ULONG UserInfo_AddPhone(struct IClass *cl, Object *obj, struct MUIP_UserInfo_AddPhone *msg)
{
   struct UserInfo_Data *data = INST_DATA(cl, obj);

   if(msg->doit)
   {
      STRPTR ptr;

      if(ptr = (STRPTR)xget(data->STR_AddPhone, MUIA_String_Contents))
      {
         if(*ptr)
         {
            strcpy(ISP.isp_phonenumber, (STRPTR)xget(data->STR_PhoneNumber, MUIA_String_Contents));
            if(*ISP.isp_phonenumber)
               strncat(ISP.isp_phonenumber, " | ", sizeof(ISP.isp_phonenumber));
            strncat(ISP.isp_phonenumber, ptr, sizeof(ISP.isp_phonenumber));
            set(data->STR_PhoneNumber, MUIA_String_Contents, ISP.isp_phonenumber);
         }
      }
   }
   DoMethod(data->PO_PhoneNumber, MUIM_Popstring_Close, TRUE);
   set(data->STR_AddPhone, MUIA_String_Contents, "");

   return(NULL);
}


///
/// UserInfo_New
ULONG UserInfo_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct UserInfo_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Group_Horiz, TRUE,
      Child, tmp.GR_Picture = VGroup,
         MUIA_ShowMe, !no_picture,
         Child, BodychunkObject,
            GroupFrame,
            InnerSpacing(0, 0),
            MUIA_FixWidth             , SETUP_PAGE3_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE3_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE3_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE3_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE3_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page3_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE3_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE3_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page3_colors,
         End,
         Child, HVSpace,
      End,
      Child, VGroup,   // enter user info
         GroupFrame,
         MUIA_Background, MUII_TextBack,
         Child, HVSpace,
         Child, MakeText(GetStr(MSG_TX_InfoLoginName)),
         Child, tmp.STR_LoginName = MakeString(ISP.isp_login, 80),
         Child, HVSpace,
         Child, MakeText(GetStr(MSG_TX_InfoPassword)),
         Child, tmp.STR_Password = StringObject,
            MUIA_CycleChain      , 1,
            MUIA_Frame           , MUIV_Frame_String,
            MUIA_String_Secret   , TRUE,
            MUIA_String_MaxLen   , 80,
            MUIA_String_Contents , ISP.isp_password,
         End,
         Child, HVSpace,
         Child, MakeText(GetStr(MSG_TX_InfoPhoneNumber)),
         Child, tmp.PO_PhoneNumber = PopobjectObject,
            MUIA_Popstring_String      , tmp.STR_PhoneNumber = StringObject,
               MUIA_CycleChain      , 1,
               StringFrame,
               MUIA_String_MaxLen   , 80,
               MUIA_String_Contents , ISP.isp_phonenumber,
               MUIA_String_Accept, "1234567890 |",
            End,
            MUIA_Popstring_Button      , PopButton(MUII_PopUp),
            MUIA_Popobject_Object      , VGroup,
               MUIA_Frame, MUIV_Frame_Group,
               Child, LLabel(GetStr(MSG_LA_AddNumber)),
               Child, tmp.STR_AddPhone = MakeString(NULL, 30),
               Child, HGroup,
                  Child, tmp.BT_AddPhone = MakeButton(MSG_BT_Add),
                  Child, tmp.BT_CancelPhone = MakeButton(MSG_BT_Cancel),
               End,
            End,
         End,
         Child, HVSpace,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct UserInfo_Data *data = INST_DATA(cl, obj);
      struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);

      *data = tmp;

      set(data->STR_LoginName    , MUIA_ShortHelp, GetStr(MSG_HELP_LoginName));
      set(data->STR_Password     , MUIA_ShortHelp, GetStr(MSG_HELP_Password));
      set(data->STR_PhoneNumber  , MUIA_ShortHelp, GetStr(MSG_HELP_PhoneNumber));

      DoMethod(data->STR_LoginName  , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Password);
      DoMethod(data->STR_Password   , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_PhoneNumber);
      DoMethod(data->STR_PhoneNumber, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, mw_data->BT_Next);
      DoMethod(data->BT_AddPhone    , MUIM_Notify, MUIA_Pressed           , FALSE         , obj, 2, MUIM_UserInfo_AddPhone, TRUE);
      DoMethod(data->BT_CancelPhone , MUIM_Notify, MUIA_Pressed           , FALSE         , obj, 2, MUIM_UserInfo_AddPhone, FALSE);
   }

   return((ULONG)obj);
}

///
/// UserInfo_Dispatcher
SAVEDS ULONG UserInfo_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                   : return(UserInfo_New      (cl, obj, (APTR)msg));
      case MUIM_UserInfo_AddPhone   : return(UserInfo_AddPhone (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

