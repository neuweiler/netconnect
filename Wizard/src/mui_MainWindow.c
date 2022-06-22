/// includes
#include "/includes.h"
#pragma header

#include "rev.h"
#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_MainWindow.h"
#include "mui_Finished.h"
#include "mui_ISPInfo1.h"
#include "mui_ISPInfo2.h"
#include "mui_LoginScript.h"
#include "mui_ModemDetect.h"
#include "mui_ModemStrings.h"
#include "mui_Online.h"
//#include "mui_Sana2.h"
#include "mui_SerialModem.h"
//#include "mui_SerialSana.h"
#include "mui_Advanced.h"
#include "mui_UserInfo.h"
#include "mui_Welcome.h"

#include "protos.h"

///
/// external variables
extern Object *app, *win, *status_win;
extern struct Config Config;
extern struct IOExtSer  *SerReadReq;
extern struct MsgPort   *SerReadPort;
extern struct IOExtSer  *SerWriteReq;
extern struct MsgPort   *SerWritePort;
extern struct MUI_CustomClass  *CL_Online;
extern struct MUI_CustomClass  *CL_ModemDetect;
extern struct MUI_CustomClass  *CL_Advanced;
extern struct MUI_CustomClass  *CL_Finished;
extern struct MUI_CustomClass  *CL_ISPInfo1;
extern struct MUI_CustomClass  *CL_ISPInfo2;
extern struct MUI_CustomClass  *CL_LoginScript;
extern struct MUI_CustomClass  *CL_ModemStrings;
//extern struct MUI_CustomClass  *CL_Sana2;
extern struct MUI_CustomClass  *CL_SerialModem;
//extern struct MUI_CustomClass  *CL_SerialSana;
extern struct MUI_CustomClass  *CL_UserInfo;
extern struct MUI_CustomClass  *CL_Welcome;
extern BOOL use_loginscript, use_modem, no_picture, keyboard_input, easy_ppp;
extern int dialing_try;
extern struct ISP ISP;
extern struct Interface Iface;

extern int addr_assign, dst_assign, dns_assign, domainname_assign;
extern char serial_in[];
extern char sana2configtext[];
extern struct NewMenu MainMenu[];

///

#define SIG_SER   (1L << SerReadPort->mp_SigBit)

/* Pages :

0 = Welcome
1 = SerialSana (Serial or Sana2)
2 = serial device & modem
3 = Init string & dial prefix
4 = username, pw, phone
5 = advanced
6 = ip, ppp/slip, script
7 = dns, gateway
8 = loginscript
9 = finished
10= sana II
*/

/// MainWindow_About
ULONG MainWindow_About(struct IClass *cl, Object *obj, Msg msg)
{
   MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), "\033b\033cGenesis Wizard\033n\033c\n" VERTAG "\n\n%ls\n\nAREXX port: '%ls'", GetStr(MSG_TX_About), xget(app, MUIA_Application_Base));
   return(NULL);
}

///
/// MainWindow_GetPageData
ULONG MainWindow_GetPageData(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);

   switch((ULONG)data->Page)
   {
/*      case 1:
      {
         struct SerialSana_Data *ss_data = INST_DATA(CL_SerialSana->mcc_Class, data->GR_Active);

         use_modem = !xget(ss_data->RA_Interface, MUIA_Radio_Active);
         break;
      }
*/      case 2:
      {
         struct SerialModem_Data *sm_data = INST_DATA(CL_SerialModem->mcc_Class, data->GR_Active);

         strncpy(Config.cnf_serialdevice, (STRPTR)xget(sm_data->STR_SerialDevice, MUIA_String_Contents), 80);
         Config.cnf_serialunit = xget(sm_data->SL_SerialUnit, MUIA_Numeric_Value);
         strncpy(Config.cnf_modemname, (STRPTR)xget(sm_data->TX_ModemName, MUIA_Text_Contents), 80);
         break;
      }
      case 3:
      {
         struct ModemStrings_Data *ms_data = INST_DATA(CL_ModemStrings->mcc_Class, data->GR_Active);

         strncpy(Config.cnf_initstring, (STRPTR)xget(ms_data->STR_InitString, MUIA_String_Contents), 80);
         strncpy(Config.cnf_dialprefix, (STRPTR)xget(ms_data->STR_DialPrefix, MUIA_String_Contents), 80);
         break;
      }
      case 4:
      {
         struct UserInfo_Data *ui_data = INST_DATA(CL_UserInfo->mcc_Class, data->GR_Active);

         strncpy(ISP.isp_login       , (STRPTR)xget(ui_data->STR_LoginName, MUIA_String_Contents)   , sizeof(ISP.isp_login));
         strncpy(ISP.isp_password    , (STRPTR)xget(ui_data->STR_Password, MUIA_String_Contents)    , sizeof(ISP.isp_password));
         strncpy(ISP.isp_phonenumber , (STRPTR)xget(ui_data->STR_PhoneNumber, MUIA_String_Contents) , sizeof(ISP.isp_phonenumber));
         break;
      }
      case 5:
      {
         struct Advanced_Data *av_data = INST_DATA(CL_Advanced->mcc_Class, data->GR_Active);

         easy_ppp = !xget(av_data->RA_Advanced, MUIA_Radio_Active);

         break;
      }
      case 6:
      {
         struct ISPInfo1_Data *ii_data = INST_DATA(CL_ISPInfo1->mcc_Class, data->GR_Active);

         addr_assign = (xget(ii_data->CY_IPAddress, MUIA_Cycle_Active) ? CNF_Assign_Static : NULL);
         strcpy(Iface.if_addr, (addr_assign ? (STRPTR)xget(ii_data->STR_IPAddress, MUIA_String_Contents) : ""));
         use_loginscript = !xget(ii_data->CY_Script, MUIA_Cycle_Active);
         if(xget(ii_data->CY_Protocol, MUIA_Cycle_Active))
         {
            strcpy(Iface.if_sana2device, "DEVS:Networks/aslip.device");
            Iface.if_sana2unit = 0;
            strcpy(Iface.if_sana2config, "ENV:Sana2/aslip0.config");
            sprintf(sana2configtext, "%ls %ld %ld Shared %ls CD 7Wire MTU=%ld", Config.cnf_serialdevice, Config.cnf_serialunit, Config.cnf_baudrate, (*Iface.if_addr ? Iface.if_addr : "0.0.0.0"), Iface.if_MTU);
            Iface.if_sana2configtext = sana2configtext;
            strcpy(Iface.if_name, "slip");
         }
         else
         {
            strcpy(Iface.if_sana2device, "DEVS:Networks/appp.device");
            Iface.if_sana2unit = 0;
            strcpy(Iface.if_sana2config, "ENV:Sana2/appp0.config");
            sprintf(sana2configtext, "sername %ls\nserunit %ld\nserbaud %ld\nlocalipaddress %ls\ncd yes\nuser %ls\nsecret %ls\n", Config.cnf_serialdevice, Config.cnf_serialunit, Config.cnf_baudrate, (*Iface.if_addr ? Iface.if_addr : "0.0.0.0"), ISP.isp_login, ISP.isp_password);
            Iface.if_sana2configtext = sana2configtext;
            strcpy(Iface.if_name, "ppp");
         }
         break;
      }
      case 7:
      {
         struct ISPInfo2_Data *ii_data = INST_DATA(CL_ISPInfo2->mcc_Class, data->GR_Active);

//         strcpy(ISP.isp_dns1, (STRPTR)xget(ii_data->STR_DNS1, MUIA_String_Contents));
//         strcpy(ISP.isp_dns2, (STRPTR)xget(ii_data->STR_DNS2, MUIA_String_Contents));
         strcpy(Iface.if_gateway, (STRPTR)xget(ii_data->STR_Gateway, MUIA_String_Contents));
         break;
      }
/*      case 10:
      {
         struct Sana2_Data *s2_data = INST_DATA(CL_Sana2->mcc_Class, data->GR_Active);
         STRPTR ptr;

         Config.cnf_serialdevice[0] = NULL;
         strcpy(Config.cnf_sana2device, xget(s2_data->STR_SanaDevice, MUIA_String_Contents));
         Config.cnf_sana2unit = xget(s2_data->SL_SanaUnit, MUIA_Numeric_Value);
         strncpy(Config.cnf_ifname, FilePart(Config.cnf_sana2device), 20);
         if(ptr = strchr(Config.cnf_ifname, '.'))
            *ptr = NULL;
         sprintf(&Config.cnf_ifname[(strlen(Config.cnf_ifname) > 14 ? 14 : strlen(Config.cnf_ifname))], "%ld", Config.cnf_sana2unit);
         strlwr(Config.cnf_ifname);

         break;
      }
*/
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
      case 0:
      case 1:
      {
//         struct SerialSana_Data *ss_data = INST_DATA(CL_SerialSana->mcc_Class, data->GR_Active);

use_modem = TRUE;
         if(use_modem)
         {
            set(app, MUIA_Application_Sleep, TRUE);
            if(status_win = NewObject(CL_ModemDetect->mcc_Class, NULL, TAG_DONE))
            {
               DoMethod(app, OM_ADDMEMBER, status_win);
               set(status_win, MUIA_Window_Open, TRUE);
               DoMethod(status_win, MUIM_ModemDetect_FindModem);
            }
         }
         else
            data->Page = 8;
         break;
      }
      case 2:
      {
         struct SerialModem_Data *sm_data = INST_DATA(CL_SerialModem->mcc_Class, data->GR_Active);

         if(serial_create(Config.cnf_serialdevice, Config.cnf_serialunit))
         {
            if(!serial_dsr())
            {
               Config.cnf_flags |= CFL_IgnoreDSR;
               MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), "Could not detect the DSR (data set ready)\nsignal from your modem. Will ignore it now.\nIf your modem wasn't switched on, please go\nback again to macke sure the DSR is properly\ndetected.");
            }
            serial_delete();
            data->Page++;
         }
         else
         {
            MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_CouldNotOpenDevice_Unit), Config.cnf_serialdevice, Config.cnf_serialunit);
            set(win, MUIA_Window_ActiveObject, sm_data->STR_SerialDevice);
         }
         break;
      }
      case 3:
      {
         struct ModemStrings_Data *ms_data = INST_DATA(CL_ModemStrings->mcc_Class, data->GR_Active);

         if(*Config.cnf_dialprefix)
         {
            if(*Config.cnf_initstring)
               data->Page++;
            else
            {
               MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_SpecifyInitString));
               set(win, MUIA_Window_ActiveObject, ms_data->STR_InitString);
            }
         }
         else
         {
            MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_SpecifyDialPrefix));
            set(win, MUIA_Window_ActiveObject, ms_data->STR_DialPrefix);
         }
         break;
      }
      case 4:
      {
         struct UserInfo_Data *ui_data = INST_DATA(CL_UserInfo->mcc_Class, data->GR_Active);

         if(*ISP.isp_login)
         {
            if(*ISP.isp_phonenumber)
            {
               data->Page++;
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
      case 5:
         if(easy_ppp)
         {
            *Iface.if_addr = NULL;
            use_loginscript = FALSE;
            strcpy(Iface.if_sana2device, "DEVS:Networks/appp.device");
            Iface.if_sana2unit = 0;
            strcpy(Iface.if_sana2config, "ENV:Sana2/appp0.config");
            sprintf(sana2configtext, "sername %ls\nserunit %ld\nserbaud %ld\nlocalipaddress 0.0.0.0\ncd yes\nuser %ls\nsecret %ls\n", Config.cnf_serialdevice, Config.cnf_serialunit, Config.cnf_baudrate, ISP.isp_login, ISP.isp_password);
            Iface.if_sana2configtext = sana2configtext;
            strcpy(Iface.if_name, "ppp");

            set(app, MUIA_Application_Sleep, TRUE);
            if(status_win = NewObject(CL_Online->mcc_Class, NULL, TAG_DONE))
            {
               DoMethod(app, OM_ADDMEMBER, status_win);
               set(status_win, MUIA_Window_Open, TRUE);
               DoMethod(status_win, MUIM_Online_GoOnline);
            }
         }
         else
            data->Page++;
         break;
      case 6:
      {
         struct ISPInfo1_Data *ii_data = INST_DATA(CL_ISPInfo1->mcc_Class, data->GR_Active);

         if(xget(ii_data->CY_IPAddress, MUIA_Cycle_Active))
         {
            if(strlen((STRPTR)xget(ii_data->STR_IPAddress, MUIA_String_Contents)))
               data->Page++;
            else
            {
               MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_SpecifyIPAddress));
               set(win, MUIA_Window_ActiveObject, ii_data->STR_IPAddress);
            }
         }
         else
            data->Page++;

         break;
      }
      case 7:
      {
         if(serial_create(Config.cnf_serialdevice, Config.cnf_serialunit))
            data->Page++;
         else
            MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_CouldNotOpenDevice_Unit), Config.cnf_serialdevice, Config.cnf_serialunit);

         break;
      }
      case 9:
      {
         struct Finished_Data *fi_data = INST_DATA(CL_Finished->mcc_Class, data->GR_Active);

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
/*      case 10:
      {
         struct Sana2_Data *s2_data = INST_DATA(CL_Sana2->mcc_Class, data->GR_Active);
         Object *window;

         set(app, MUIA_Application_Sleep, TRUE);
         if(window = NewObject(CL_Online->mcc_Class, NULL, TAG_DONE))
         {
            DoMethod(app, OM_ADDMEMBER, window);
            set(window, MUIA_Window_Open, TRUE);
            DoMethod(window, MUIM_Online_GoOnline);
         }
         data->Page--;
         break;
      }
*/      default:
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
      case 10:
         data->Page = 1;
         break;
case 2:
data->Page = 0;
break;
      case 8:
         if(dialing_try)
            DoMethod(data->GR_Active, MUIM_LoginScript_HangUp);
         data->Page--;
         break;
      case 9:
         if(use_modem)
         {
            if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_BackCancel), GetStr(MSG_TX_GoBack)))
            {
               if(easy_ppp)
                  data->Page = 5;
               else
               {
                  if(serial_create(Config.cnf_serialdevice, Config.cnf_serialunit))
                     data->Page--;
                  else
                     MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_CouldNotOpenDevice_Unit), Config.cnf_serialdevice, Config.cnf_serialunit);
               }
            }
         }
         else
            data->Page = 10;
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
   APTR new_class = NULL;
   Object *new;

   if(data->Page < 0)
      data->Page = 0;

   set(data->BT_Back, MUIA_Disabled, (data->Page == 0));
   set(data->BT_Next, MUIA_Disabled, (data->Page == 8));

   switch((ULONG)data->Page)
   {
//      case 1:
//         new_class = CL_SerialSana->mcc_Class;
//         break;
      case 2:
         new_class = CL_SerialModem->mcc_Class;
         break;
      case 3:
         new_class = CL_ModemStrings->mcc_Class;
         break;
      case 4:
         new_class = CL_UserInfo->mcc_Class;
         break;
      case 5:
         new_class = CL_Advanced->mcc_Class;
         break;
      case 6:
         new_class = CL_ISPInfo1->mcc_Class;
         break;
      case 7:
         new_class = CL_ISPInfo2->mcc_Class;
         break;
      case 8:
         new_class = CL_LoginScript->mcc_Class;
         break;
      case 9:
         new_class = CL_Finished->mcc_Class;
         break;
//      case 10:
//         new_class = CL_Sana2->mcc_Class;
//         break;
      default:
         new_class = CL_Welcome->mcc_Class;
         break;
   }
   if(new_class)
   {
      /** dispose old page and insert new one **/

      set(app, MUIA_Application_Sleep, TRUE);
      if(new = NewObject(new_class, NULL, TAG_DONE))
      {
         DoMethod(data->GR_Pager, MUIM_Group_InitChange);
         DoMethod(data->GR_Pager, OM_ADDMEMBER, new);
         DoMethod(data->GR_Pager, OM_REMMEMBER, data->GR_Active);
         DoMethod(data->GR_Pager, MUIM_Group_ExitChange);
         MUI_DisposeObject(data->GR_Active);
         data->GR_Active = new;

         /** set page specific defaults **/

         switch((ULONG)data->Page)
         {
/*            case 1:
            {
               struct SerialSana_Data *ss_data = INST_DATA(CL_SerialSana->mcc_Class, new);

               set(win, MUIA_Window_ActiveObject, data->BT_Next);
               break;
            }
*/            case 2:
            {
               struct SerialModem_Data *sm_data = INST_DATA(CL_SerialModem->mcc_Class, new);

               DoMethod(app, MUIM_Application_PushMethod, new, 1, MUIM_SerialModem_LoadData);
               break;
            }
            case 3:
            {
               struct ModemStrings_Data *ms_data = INST_DATA(CL_ModemStrings->mcc_Class, new);

               DoMethod(app, MUIM_Application_PushMethod, new, 1, MUIM_ModemStrings_LoadData);
               set(win, MUIA_Window_ActiveObject, data->BT_Next);
               break;
            }
            case 4:
            {
               struct UserInfo_Data *ui_data = INST_DATA(CL_UserInfo->mcc_Class, new);

               // direct set() doesn't always work.. who knows why...
               DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_Set, MUIA_Window_ActiveObject, ui_data->STR_LoginName);
               break;
            }
            case 5:
            {
               struct Advanced_Data *av_data = INST_DATA(CL_Advanced->mcc_Class, new);

               set(win, MUIA_Window_ActiveObject, data->BT_Next);
               break;
            }
            case 6:
            {
               struct ISPInfo1_Data *ii_data = INST_DATA(CL_ISPInfo1->mcc_Class, new);

               // direct set() doesn't always work.. who knows why...
               set(win, MUIA_Window_ActiveObject, data->BT_Next);
               break;
            }
            case 7:
            {
               serial_delete();
               break;
            }
            case 8:
            {
               struct LoginScript_Data *ls_data = INST_DATA(CL_LoginScript->mcc_Class, new);

               DoMethod(ls_data->BT_GoOnline, MUIM_MultiSet, MUIA_Disabled, !use_loginscript,
                  ls_data->BT_GoOnline, ls_data->BT_SendLogin, ls_data->BT_SendPassword, ls_data->BT_SendBreak, NULL);

               clear_list(&ISP.isp_loginscript);
               keyboard_input = FALSE;

               serial_send("\r", -1);
               Delay(25);
               serial_clear();
               serial_startread(serial_in, 1);

               if(!ls_data->ihnode_added && SerReadPort)
               {
                  ls_data->ihnode.ihn_Object  = new;
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
            case 9:
            {
               struct Finished_Data *fi_data = INST_DATA(CL_Finished->mcc_Class, new);

               serial_delete();
               set(win, MUIA_Window_ActiveObject, data->BT_Next);
               break;
            }
/*            case 10:
            {
               struct Sana2_Data *s2_data = INST_DATA(CL_Sana2->mcc_Class, new);

               DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_Set, MUIA_Window_ActiveObject, s2_data->STR_SanaDevice);
               break;
            }
*/            default:
            {
               struct Welcome_Data *wc_data = INST_DATA(CL_Welcome->mcc_Class, new);

               set(win, MUIA_Window_ActiveObject, data->BT_Next);
               break;
            }
         }
      }
      else
         MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_ErrorCreatePage));

      set(app, MUIA_Application_Sleep, FALSE);
   }
   else
      MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_ErrorNoPage));

   set(data->BT_Next, MUIA_Text_Contents, (data->Page == 9 ? GetStr(MSG_BT_Finish) : GetStr(MSG_BT_Next)));
   set(data->BT_Next, MUIA_ShortHelp, (data->Page == 9 ? GetStr(MSG_HELP_Finish) : GetStr(MSG_HELP_Next)));

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
/// MainWindow_Help
ULONG MainWindow_Help(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   char buf[21];

   sprintf(buf, "Page%ld", data->Page + 1);
   DoMethod(app, MUIM_Application_ShowHelp, win, "GenesisWizard.guide", buf, 0);

   return(NULL);
}

///
/// MainWindow_MUIRequest
ULONG MainWindow_MUIRequest(struct IClass *cl, Object *obj, struct MUIP_MainWindow_MUIRequest *msg)
{
   char buf[MAXPATHLEN + 20];

   vsprintf(buf, msg->message, (va_list)(&msg->message + (va_list)1));
   MUI_Request(app, win, NULL, NULL, msg->buttons, buf);
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
      MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, MainMenu, NULL),
      WindowContents       , VGroup,
         Child, tmp.GR_Pager = VGroup,
            Child, tmp.GR_Active = NewObject(CL_Welcome->mcc_Class, NULL, TAG_DONE),
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
            Child, tmp.BT_Help    = MakeButton("  _Help"),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct MainWindow_Data *data = INST_DATA(cl, obj);

      *data = tmp;

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
      DoMethod(data->BT_Help    , MUIM_Notify, MUIA_Pressed   , FALSE  , obj, 1, MUIM_MainWindow_Help);

      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT)    , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, obj, 1, MUIM_MainWindow_About);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT_MUI), MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_AboutMUI, win);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_QUIT)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, obj, 1, MUIM_MainWindow_Quit);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_MUI)      , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
   }
   return((ULONG)obj);
}

///
/// MainWindow_Dispatcher
SAVEDS ULONG MainWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                         : return(MainWindow_New             (cl, obj, (APTR)msg));
      case MUIM_MainWindow_GetPageData    : return(MainWindow_GetPageData     (cl, obj, (APTR)msg));
      case MUIM_MainWindow_SetPage        : return(MainWindow_SetPage         (cl, obj, (APTR)msg));
      case MUIM_MainWindow_NextPage       : return(MainWindow_NextPage        (cl, obj, (APTR)msg));
      case MUIM_MainWindow_BackPage       : return(MainWindow_BackPage        (cl, obj, (APTR)msg));
      case MUIM_MainWindow_About          : return(MainWindow_About           (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Help           : return(MainWindow_Help            (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Quit           : return(MainWindow_Quit            (cl, obj, (APTR)msg));
      case MUIM_MainWindow_DisposeWindow  : return(MainWindow_DisposeWindow   (cl, obj, (APTR)msg));
      case MUIM_MainWindow_MUIRequest     : return(MainWindow_MUIRequest      (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

