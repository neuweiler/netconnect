/// includes
#include "/includes.h"

#include "rev.h"
#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_MainWindow.h"
#include "mui_Finished.h"
#include "mui_Protocol.h"
#include "mui_LoginScript.h"
#include "mui_ModemStrings.h"
#include "mui_Online.h"
#include "mui_Sana2.h"
#include "mui_SanaConfig.h"
#include "mui_SerialModem.h"
#include "mui_SerialSana.h"
#include "mui_UserInfo.h"
#include "mui_Welcome.h"
#include "mui_Request.h"

#include "protos.h"
#include "images/setup_page0.h"

///
/// external variables
extern struct Library *MUIMasterBase;
extern Object *app, *win, *status_win;
extern struct Config Config;
extern struct IOExtSer  *SerReadReq;
extern struct MsgPort   *SerReadPort;
extern struct IOExtSer  *SerWriteReq;
extern struct MsgPort   *SerWritePort;
extern struct MUI_CustomClass *CL_Online, *CL_Finished, *CL_Protocol, *CL_LoginScript,
                              *CL_ModemStrings, *CL_Sana2, *CL_SerialModem, *CL_SerialSana,
                              *CL_UserInfo, *CL_Welcome, *CL_SanaConfig, *CL_Request,
                              *CL_About;
extern BOOL use_loginscript, use_modem, no_picture, keyboard_input;
extern int dialing_try;
extern struct ISP ISP;
extern struct Interface Iface;

extern int addr_assign, dst_assign, dns_assign, domainname_assign;
extern char serial_in[];
extern char sana2configtext[], configparams[];
extern struct NewMenu MainMenu[];

extern ULONG *colors[NUM_PAGES];
extern UBYTE *bodies[NUM_PAGES];

///

#define SIG_SER   (1L << SerReadPort->mp_SigBit)

/// MainWindow_About
ULONG MainWindow_About(struct IClass *cl, Object *obj, Msg msg)
{
   Object *req;

   set(app, MUIA_Application_Sleep, TRUE);
   if(req = (APTR)NewObject(CL_About->mcc_Class, NULL, TAG_DONE))
   {
      DoMethod(app, OM_ADDMEMBER, req);
      set(req, MUIA_Window_Open, TRUE);
   }
   else
      set(app, MUIA_Application_Sleep, FALSE);

   return(NULL);
}

///
/// MainWindow_GetPageData
ULONG MainWindow_GetPageData(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);

   switch((ULONG)data->Page)
   {
      case PAGE_SerialSana:
      {
         struct SerialSana_Data *ss_data = INST_DATA(CL_SerialSana->mcc_Class, data->GR_SerialSana);

         use_modem = !xget(ss_data->RA_Interface, MUIA_Radio_Active);
         break;
      }
      case PAGE_SerialModem:
      {
         struct SerialModem_Data *sm_data = INST_DATA(CL_SerialModem->mcc_Class, data->GR_SerialModem);

         strncpy(Config.cnf_serialdevice, (STRPTR)xget(sm_data->STR_SerialDevice, MUIA_String_Contents), 80);
         Config.cnf_serialunit = xget(sm_data->SL_SerialUnit, MUIA_Numeric_Value);
         strncpy(Config.cnf_modemname, (STRPTR)xget(sm_data->TX_ModemName, MUIA_Text_Contents), 80);
         break;
      }
      case PAGE_ModemStrings:
      {
         struct ModemStrings_Data *ms_data = INST_DATA(CL_ModemStrings->mcc_Class, data->GR_ModemStrings);

         strncpy(Config.cnf_initstring, (STRPTR)xget(ms_data->STR_InitString, MUIA_String_Contents), 80);
         strncpy(Config.cnf_dialprefix, (STRPTR)xget(ms_data->STR_DialPrefix, MUIA_String_Contents), 80);
         break;
      }
      case PAGE_UserInfo:
      {
         struct UserInfo_Data *ui_data = INST_DATA(CL_UserInfo->mcc_Class, data->GR_UserInfo);

         strncpy(ISP.isp_login       , (STRPTR)xget(ui_data->STR_LoginName, MUIA_String_Contents)   , sizeof(ISP.isp_login));
         strncpy(ISP.isp_password    , (STRPTR)xget(ui_data->STR_Password, MUIA_String_Contents)    , sizeof(ISP.isp_password));
         strncpy(ISP.isp_phonenumber , (STRPTR)xget(ui_data->STR_PhoneNumber, MUIA_String_Contents) , sizeof(ISP.isp_phonenumber));
         break;
      }
      case PAGE_Protocol:
      {
         struct Protocol_Data *pr_data = INST_DATA(CL_Protocol->mcc_Class, data->GR_Protocol);

         use_loginscript = xget(pr_data->CY_Script, MUIA_Cycle_Active);
         if(xget(pr_data->CY_Protocol, MUIA_Cycle_Active))
         {
            strcpy(Iface.if_sana2device, "DEVS:Networks/aslip.device");
            Iface.if_sana2unit = 0;
            strcpy(Iface.if_sana2config, "ENV:Sana2/aslip0.config");
            sprintf(sana2configtext, "%ls %ld %ld Shared %%a CD 7Wire MTU=%ld ", Config.cnf_serialdevice, Config.cnf_serialunit, Config.cnf_baudrate, Iface.if_MTU);
            Iface.if_sana2configtext = sana2configtext;
            strcpy(Iface.if_name, "slip");
            Iface.if_flags |= IFL_SLIP;
         }
         else
         {
            strcpy(Iface.if_sana2device, "DEVS:Networks/appp.device");
            Iface.if_sana2unit = 0;
            strcpy(Iface.if_sana2config, "ENV:Sana2/appp0.config");
            sprintf(sana2configtext, "sername %ls\nserunit %ld\nserbaud %ld\nlocalipaddress %%a\ncd yes\nuser %%u\nsecret %%p\ncd yes\nmppcomp no\nvjcomp no\nbsdcomp no\ndeflatecomp no\neof no\ndebug = 1\ndebug_window = 1\nlog_file = AmiTCP:log/appp.log\ncon_window = NIL:\n", Config.cnf_serialdevice, Config.cnf_serialunit, Config.cnf_baudrate);
            Iface.if_sana2configtext = sana2configtext;
            strcpy(Iface.if_name, "ppp");
            Iface.if_flags |= IFL_PPP;
         }
         break;
      }
      case PAGE_Sana2:
      {
         struct Sana2_Data *s2_data = INST_DATA(CL_Sana2->mcc_Class, data->GR_Sana2);
         struct SanaConfig_Data *sc_data = INST_DATA(CL_SanaConfig->mcc_Class, data->GR_SanaConfig);
         STRPTR ptr;

         Config.cnf_serialdevice[0] = NULL;
         strcpy(Iface.if_sana2device, (STRPTR)xget(s2_data->STR_SanaDevice, MUIA_String_Contents));
         Iface.if_sana2unit = xget(s2_data->SL_SanaUnit, MUIA_Numeric_Value);
         strncpy(Iface.if_name, FilePart(Iface.if_sana2device), 20);
         if(ptr = strchr(Iface.if_name, '.'))
            *ptr = NULL;
         sprintf(&Iface.if_name[(strlen(Iface.if_name) > 14 ? 14 : strlen(Iface.if_name))], "%ld", Iface.if_sana2unit);
         strlwr(Iface.if_name);
         if(!strlen((STRPTR)xget(sc_data->STR_Sana2ConfigFile, MUIA_String_Contents)))
         {
            sprintf(Iface.if_sana2config, "ENV:Sana2/%ls.config", Iface.if_name);
            setstring(sc_data->STR_Sana2ConfigFile, Iface.if_sana2config);
            DoMethod(data->GR_SanaConfig, MUIM_SanaConfig_LoadConfig);
         }

         break;
      }
      case PAGE_SanaConfig:
      {
         struct SanaConfig_Data *sc_data = INST_DATA(CL_SanaConfig->mcc_Class, data->GR_SanaConfig);

         strcpy(Iface.if_sana2config, (STRPTR)xget(sc_data->STR_Sana2ConfigFile, MUIA_String_Contents));
         strcpy(sana2configtext, (STRPTR)xget(sc_data->TI_Sana2ConfigText, MUIA_String_Contents));
         Iface.if_sana2configtext = sana2configtext;
         strcpy(configparams, (STRPTR)xget(sc_data->STR_ConfigParams, MUIA_String_Contents));
         Iface.if_configparams = configparams;
         Iface.if_flags = NULL;
      }
   }
   return(NULL);
}

///
/// MainWindow_NextPage
ULONG MainWindow_NextPage(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   LONG old_page = data->Page;

   DoMethod(obj, MUIM_MainWindow_GetPageData);
   switch((ULONG)data->Page)
   {
      case PAGE_SerialSana:
      {
         if(use_modem)
            data->Page = PAGE_SerialModem;
         else
            data->Page = PAGE_Sana2;
         break;
      }
      case PAGE_SerialModem:
      {
         struct SerialModem_Data *sm_data = INST_DATA(CL_SerialModem->mcc_Class, data->GR_SerialModem);

         Config.cnf_flags &= ~CFL_IgnoreDSR;
         if(serial_create(Config.cnf_serialdevice, Config.cnf_serialunit))
         {
            if(!serial_dsr())
            {
               if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_IgnoreCancel), GetStr(MSG_TX_NoDSR)))
               {
                  Config.cnf_flags |= CFL_IgnoreDSR;
                  data->Page = PAGE_ModemStrings;
               }
               else
                  serial_delete();
            }
            else
               data->Page = PAGE_ModemStrings;
         }
         else
         {
            MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_CouldNotOpenDevice_Unit), Config.cnf_serialdevice, Config.cnf_serialunit);
            set(win, MUIA_Window_ActiveObject, sm_data->STR_SerialDevice);
         }
         break;
      }
      case PAGE_ModemStrings:
      {
         struct ModemStrings_Data *ms_data = INST_DATA(CL_ModemStrings->mcc_Class, data->GR_ModemStrings);

         if(*Config.cnf_dialprefix)
         {
            set(app, MUIA_Application_Sleep, TRUE);

            serial_send("\r", -1);
            Delay(10);
            serial_send("AAT\r", -1);
            if(serial_waitfor("OK", NULL, NULL, 2))
            {
               char buffer[MAXPATHLEN];

               Delay(10);
               EscapeString(buffer, Config.cnf_initstring);
               strcat(buffer, "\r");
               serial_send(buffer, -1);
               if(serial_waitfor("OK", NULL, NULL, 2))
                  data->Page = PAGE_UserInfo;
               else
               {
                  if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_IgnoreCancel), GetStr(MSG_TX_NoOKOnInit)))
                     data->Page = PAGE_UserInfo;
               }
            }
            else
            {
               if(MUI_Request(app, win, NULL,  NULL, GetStr(MSG_ReqBT_IgnoreCancel), GetStr(MSG_TX_NoOKOnAT)))
                  data->Page = PAGE_UserInfo;
            }
            set(app, MUIA_Application_Sleep, FALSE);
         }
         else
         {
            MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_SpecifyDialPrefix));
            set(win, MUIA_Window_ActiveObject, ms_data->STR_DialPrefix);
         }
         break;
      }
      case PAGE_UserInfo:
      {
         struct UserInfo_Data *ui_data = INST_DATA(CL_UserInfo->mcc_Class, data->GR_UserInfo);

         if(*ISP.isp_login)
         {
            if(*ISP.isp_phonenumber)
            {
               data->Page = PAGE_Protocol;
            }
            else
            {
               MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_SpecifyPhoneNumber));
               set(win, MUIA_Window_ActiveObject, ui_data->STR_PhoneNumber);
            }
         }
         else
         {
            MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_SpecifyLoginName));
            set(win, MUIA_Window_ActiveObject, ui_data->STR_LoginName);
         }
         break;
      }

      case PAGE_Finished:
      {
         struct Finished_Data *fi_data = INST_DATA(CL_Finished->mcc_Class, data->GR_Finished);

         if(xget(fi_data->CH_Config, MUIA_Selected))
            if(!(save_config((STRPTR)xget(fi_data->STR_Config, MUIA_String_Contents), &ISP, &Iface, &Config)))
               MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_ErrorSaveConfig), xget(fi_data->STR_Config, MUIA_String_Contents));
         if(xget(fi_data->CH_Info, MUIA_Selected))
         {
            BPTR fh;

            if(fh = Open((STRPTR)xget(fi_data->STR_Info, MUIA_String_Contents), MODE_NEWFILE))
            {
               print_config(fh, &ISP, &Iface, &Config);
               Close(fh);
            }
            else
               MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_ErrorOpenX), xget(fi_data->STR_Info, MUIA_String_Contents));
         }
         if(xget(fi_data->CH_Printer, MUIA_Selected))
         {
            BPTR fh;

            if(fh = Open((STRPTR)xget(fi_data->STR_Printer, MUIA_String_Contents), MODE_NEWFILE))
            {
               print_config(fh, &ISP, &Iface, &Config);
               Close(fh);
            }
            else
               MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_ErrorOpenX), xget(fi_data->STR_Printer, MUIA_String_Contents));
         }
         DoMethod(app, MUIM_Application_PushMethod, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
         break;
      }
      case PAGE_SanaConfig:
      {
//         struct SanaConfig_Data *sc_data = INST_DATA(CL_SanaConfig->mcc_Class, data->GR_SanaConfig);

         set(app, MUIA_Application_Sleep, TRUE);
         if(status_win = NewObject(CL_Online->mcc_Class, NULL, TAG_DONE))
         {
            DoMethod(app, OM_ADDMEMBER, status_win);
            set(status_win, MUIA_Window_Open, TRUE);
            DoMethod(status_win, MUIM_Online_GoOnline);
         }
         break;
      }
      default:
         data->Page++;
         break;
   }

   if(old_page != data->Page)
      DoMethod(obj, MUIM_MainWindow_SetPage);
   return(NULL);
}

///
/// MainWindow_BackPage
ULONG MainWindow_BackPage(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);

   DoMethod(obj, MUIM_MainWindow_GetPageData);

   switch((ULONG)data->Page)
   {
      case PAGE_Sana2:
         data->Page = PAGE_SerialSana;
         break;
      case PAGE_LoginScript:
      {
         struct LoginScript_Data *ls_data = INST_DATA(CL_LoginScript->mcc_Class, data->GR_LoginScript);

         if(dialing_try)
            DoMethod(data->GR_LoginScript, MUIM_LoginScript_HangUp);
         Delay(50);

         if(ls_data->ihnode_added)
         {
            ls_data->ihnode_added = FALSE;
            DoMethod(app, MUIM_Application_RemInputHandler, &ls_data->ihnode);
         }
         serial_stopread();
         data->Page = PAGE_Protocol;
         break;
      }
      case PAGE_Finished:
         if(use_modem)
         {
            if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_BackCancel), GetStr(MSG_TX_GoBack)))
               data->Page = PAGE_LoginScript;
         }
         else
            data->Page = PAGE_SanaConfig;
         break;
      default:
         data->Page--;
         break;
   }
   DoMethod(obj, MUIM_MainWindow_SetPage);
   return(NULL);
}

///
/// MainWindow_SetPage
ULONG MainWindow_SetPage(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   Object *new_pic;

   if(data->Page < 0)
      data->Page = 0;

   if(data->Page < PAGE_ModemStrings)
      serial_delete();

   set(data->BT_Back, MUIA_Disabled, (data->Page == PAGE_Welcome));
   set(data->BT_Next, MUIA_Disabled, (data->Page == PAGE_LoginScript));
   set(data->GR_Pager, MUIA_Group_ActivePage, data->Page);

   DoMethod(data->GR_Picture, MUIM_Group_InitChange);
   if(new_pic = BodychunkObject,
         GroupFrame,
         InnerSpacing(0, 0),
         MUIA_FixWidth             , SETUP_PAGE0_WIDTH,
         MUIA_FixHeight            , SETUP_PAGE0_HEIGHT,
         MUIA_Bitmap_Width         , SETUP_PAGE0_WIDTH ,
         MUIA_Bitmap_Height        , SETUP_PAGE0_HEIGHT,
         MUIA_Bodychunk_Depth      , SETUP_PAGE0_DEPTH ,
         MUIA_Bodychunk_Body       , (UBYTE *)bodies[data->Page],
         MUIA_Bodychunk_Compression, SETUP_PAGE0_COMPRESSION,
         MUIA_Bodychunk_Masking    , SETUP_PAGE0_MASKING,
         MUIA_Bitmap_SourceColors  , (ULONG *)colors[data->Page],
      End)
   {
      DoMethod(data->GR_Picture, OM_ADDMEMBER, new_pic);
   }
   if(data->BC_Picture)
   {
      DoMethod(data->GR_Picture, OM_REMMEMBER, data->BC_Picture);
      MUI_DisposeObject(data->BC_Picture);
   }
   data->BC_Picture = new_pic;
   DoMethod(data->GR_Picture, MUIM_Group_ExitChange);

   switch((ULONG)data->Page)
   {
      case PAGE_UserInfo:
      {
         struct UserInfo_Data *ui_data = INST_DATA(CL_UserInfo->mcc_Class, data->GR_UserInfo);

         set(win, MUIA_Window_ActiveObject, ui_data->STR_LoginName);
         break;
      }
      case PAGE_LoginScript:
      {
         struct LoginScript_Data *ls_data = INST_DATA(CL_LoginScript->mcc_Class, data->GR_LoginScript);

         DoMethod(ls_data->BT_GoOnline, MUIM_MultiSet, MUIA_Disabled, !use_loginscript,
            ls_data->BT_GoOnline, ls_data->BT_SendLogin, ls_data->BT_SendPassword, ls_data->BT_SendBreak, NULL);

         clear_list(&ISP.isp_loginscript);
         keyboard_input = FALSE;

         serial_startread(serial_in, 1);

         if(!ls_data->ihnode_added && SerReadPort)
         {
            ls_data->ihnode.ihn_Object  = data->GR_LoginScript;
            ls_data->ihnode.ihn_Signals = SIG_SER;
            ls_data->ihnode.ihn_Flags   = 0;
            ls_data->ihnode.ihn_Method  = MUIM_LoginScript_SerialInput;
            DoMethod(app, MUIM_Application_AddInputHandler, &ls_data->ihnode);
            ls_data->ihnode_added = TRUE;
         }
         set(win, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_None);
         set(win, MUIA_Window_DefaultObject, ls_data->TR_Terminal);
         break;
      }
      case PAGE_Sana2:
      {
         struct Sana2_Data *s2_data = INST_DATA(CL_Sana2->mcc_Class, data->GR_Sana2);

         set(win, MUIA_Window_ActiveObject, s2_data->STR_SanaDevice);
         break;
      }
      default:
         set(win, MUIA_Window_ActiveObject, data->BT_Next);
         break;
   }

   set(data->BT_Next, MUIA_Text_Contents, (data->Page == PAGE_Finished ? GetStr(MSG_BT_Finish) : GetStr(MSG_BT_Next)));
   set(data->BT_Next, MUIA_ShortHelp, (data->Page == PAGE_Finished ? GetStr(MSG_HELP_Finish) : GetStr(MSG_HELP_Next)));

   return(NULL);
}

///
/// MainWindow_Quit
ULONG MainWindow_Quit(struct IClass *cl, Object *obj, Msg msg)
{
   if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_QuitCancel), GetStr(MSG_TX_ReallyQuit)))
      DoMethod(app, MUIM_Application_PushMethod, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

   return(NULL);
}

///
/// MainWindow_MUIRequest
ULONG MainWindow_MUIRequest(struct IClass *cl, Object *obj, struct MUIP_MainWindow_MUIRequest *msg)
{
   MUI_Request(app, win, NULL, NULL, msg->buttons, msg->message);
   return(NULL);
}

///
/// MainWindow_Request
ULONG MainWindow_Request(struct IClass *cl, Object *obj, struct MUIP_MainWindow_Request *msg)
{
   Object *window;

   set(app, MUIA_Application_Sleep, TRUE);

   if(window = NewObject(CL_Request->mcc_Class, NULL,
      MUIA_Request_Buffer  , msg->buffer,
      MUIA_Request_Process , msg->proc,
      MUIA_Request_Text    , msg->text,

      TAG_DONE))
   {
      DoMethod(app, OM_ADDMEMBER, window);
      set(window, MUIA_Window_Open, TRUE);
   }

   return(NULL);
}

///

/// MainWindow_DisposeWindow
ULONG MainWindow_DisposeWindow(struct IClass *cl, Object *obj, struct MUIP_MainWindow_DisposeWindow *msg)
{
   if(msg->window)
   {
      set(msg->window, MUIA_Window_Open, FALSE);
      Delay(10);  // allow the calling method to terminate
      DoMethod(app, OM_REMMEMBER, msg->window);
      MUI_DisposeObject(msg->window);
   }

   set(app, MUIA_Application_Sleep, FALSE);

   if(status_win == msg->window)
      status_win = NULL;

   return(NULL);
}

///
/// MainWindow_New
ULONG MainWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct MainWindow_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , GetStr(MSG_TX_MainWindowTitle),
      MUIA_Window_ID       , MAKE_ID('M','A','I','N'),
      MUIA_Window_Width    , MUIV_Window_Width_MinMax(0),
      MUIA_Window_Height   , MUIV_Window_Height_MinMax(0),
      MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, MainMenu, NULL),
      WindowContents       , VGroup,
         Child, HGroup,
            Child, VGroup, // needed to keep HVSpace at bottom
               Child, tmp.GR_Picture = VGroup,
                  Child, tmp.BC_Picture = BodychunkObject,
                     GroupFrame,
                     InnerSpacing(0, 0),
                     MUIA_FixWidth             , SETUP_PAGE0_WIDTH,
                     MUIA_FixHeight            , SETUP_PAGE0_HEIGHT,
                     MUIA_Bitmap_Width         , SETUP_PAGE0_WIDTH ,
                     MUIA_Bitmap_Height        , SETUP_PAGE0_HEIGHT,
                     MUIA_Bodychunk_Depth      , SETUP_PAGE0_DEPTH ,
                     MUIA_Bodychunk_Body       , bodies[PAGE_Welcome],
                     MUIA_Bodychunk_Compression, SETUP_PAGE0_COMPRESSION,
                     MUIA_Bodychunk_Masking    , SETUP_PAGE0_MASKING,
                     MUIA_Bitmap_SourceColors  , colors[PAGE_Welcome],
                  End,
               End,
               Child, HVSpace,
            End,
            Child, tmp.GR_Pager = VGroup,
               MUIA_Group_PageMode, TRUE,
               // children will be added below
            End,
         End,
         Child, MUI_MakeObject(MUIO_HBar, 2),
         Child, HGroup,
            MUIA_Group_SameSize  , TRUE,
            GroupSpacing(1),
            Child, HVSpace,
            Child, HVSpace,
            Child, tmp.BT_Back   = MakeButton(MSG_BT_Back),
            Child, tmp.BT_Next   = TextObject,
               ButtonFrame,
               MUIA_CycleChain   , 1,
               MUIA_Background   , MUII_ButtonBack,
               MUIA_Font         , MUIV_Font_Button,
               MUIA_Text_Contents, GetStr(MSG_BT_Next),
               MUIA_Text_PreParse, "\033c",
               MUIA_InputMode    , MUIV_InputMode_RelVerify,
            End,
            Child, HSpace(0),
            Child, tmp.BT_Abort   = MakeButton(MSG_BT_Abort),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct MainWindow_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->GR_Welcome      = NewObject(CL_Welcome->mcc_Class      , NULL, MUIA_Genesis_Originator, obj, TAG_DONE);
      data->GR_SerialSana   = NewObject(CL_SerialSana->mcc_Class   , NULL, MUIA_Genesis_Originator, obj, TAG_DONE);
      data->GR_SerialModem  = NewObject(CL_SerialModem->mcc_Class  , NULL, MUIA_Genesis_Originator, obj, TAG_DONE);
      data->GR_ModemStrings = NewObject(CL_ModemStrings->mcc_Class , NULL, MUIA_Genesis_Originator, obj, TAG_DONE);
      data->GR_UserInfo     = NewObject(CL_UserInfo->mcc_Class     , NULL, MUIA_Genesis_Originator, obj, TAG_DONE);
      data->GR_Protocol     = NewObject(CL_Protocol->mcc_Class     , NULL, MUIA_Genesis_Originator, obj, TAG_DONE);
      data->GR_LoginScript  = NewObject(CL_LoginScript->mcc_Class  , NULL, MUIA_Genesis_Originator, obj, TAG_DONE);
      data->GR_Finished     = NewObject(CL_Finished->mcc_Class     , NULL, MUIA_Genesis_Originator, obj, TAG_DONE);
      data->GR_Sana2        = NewObject(CL_Sana2->mcc_Class        , NULL, MUIA_Genesis_Originator, obj, TAG_DONE);
      data->GR_SanaConfig   = NewObject(CL_SanaConfig->mcc_Class   , NULL, MUIA_Genesis_Originator, obj, TAG_DONE);
      if(data->GR_Welcome && data->GR_SerialSana && data->GR_SerialModem &&
         data->GR_ModemStrings && data->GR_UserInfo && data->GR_Protocol &&
         data->GR_LoginScript && data->GR_Finished && data->GR_Sana2 &&
         data->GR_SanaConfig)
      {
         DoMethod(data->GR_Pager, OM_ADDMEMBER, data->GR_Welcome);
         DoMethod(data->GR_Pager, OM_ADDMEMBER, data->GR_SerialSana);
         DoMethod(data->GR_Pager, OM_ADDMEMBER, data->GR_SerialModem);
         DoMethod(data->GR_Pager, OM_ADDMEMBER, data->GR_ModemStrings);
         DoMethod(data->GR_Pager, OM_ADDMEMBER, data->GR_UserInfo);
         DoMethod(data->GR_Pager, OM_ADDMEMBER, data->GR_Protocol);
         DoMethod(data->GR_Pager, OM_ADDMEMBER, data->GR_LoginScript);
         DoMethod(data->GR_Pager, OM_ADDMEMBER, data->GR_Finished);
         DoMethod(data->GR_Pager, OM_ADDMEMBER, data->GR_Sana2);
         DoMethod(data->GR_Pager, OM_ADDMEMBER, data->GR_SanaConfig);

         data->Page = 0;

         set(obj, MUIA_Window_ActiveObject, data->BT_Next);
         set(data->BT_Back, MUIA_Disabled, TRUE);

         set(data->BT_Back    , MUIA_ShortHelp, GetStr(MSG_HELP_Back));
         set(data->BT_Next    , MUIA_ShortHelp, GetStr(MSG_HELP_Next));
         set(data->BT_Abort   , MUIA_ShortHelp, GetStr(MSG_HELP_Abort));

         DoMethod(obj              , MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 1, MUIM_MainWindow_Quit);
         DoMethod(data->BT_Next    , MUIM_Notify, MUIA_Pressed   , FALSE  , obj, 1, MUIM_MainWindow_NextPage);
         DoMethod(data->BT_Back    , MUIM_Notify, MUIA_Pressed   , FALSE  , obj, 1, MUIM_MainWindow_BackPage);
         DoMethod(data->BT_Abort   , MUIM_Notify, MUIA_Pressed   , FALSE  , obj, 1, MUIM_MainWindow_Quit);

         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT)    , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, obj, 1, MUIM_MainWindow_About);
         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT_MUI), MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_AboutMUI, win);
         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_QUIT)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, obj, 1, MUIM_MainWindow_Quit);
         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_MUI)      , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
      }
      else
      {
         MUI_DisposeObject(obj);
         obj = NULL;
      }
   }
   return((ULONG)obj);
}

///
/// MainWindow_Dispatcher
SAVEDS ASM ULONG MainWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                         : return(MainWindow_New             (cl, obj, (APTR)msg));
      case MUIM_MainWindow_GetPageData    : return(MainWindow_GetPageData     (cl, obj, (APTR)msg));
      case MUIM_MainWindow_SetPage        : return(MainWindow_SetPage         (cl, obj, (APTR)msg));
      case MUIM_MainWindow_NextPage       : return(MainWindow_NextPage        (cl, obj, (APTR)msg));
      case MUIM_MainWindow_BackPage       : return(MainWindow_BackPage        (cl, obj, (APTR)msg));
      case MUIM_MainWindow_About          : return(MainWindow_About           (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Quit           : return(MainWindow_Quit            (cl, obj, (APTR)msg));
      case MUIM_MainWindow_DisposeWindow  : return(MainWindow_DisposeWindow   (cl, obj, (APTR)msg));
      case MUIM_MainWindow_MUIRequest     : return(MainWindow_MUIRequest      (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Request        : return(MainWindow_Request         (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

