#include "globals.c"
#include "globals2.c"
#include "protos.h"

/// MainWindow_About
ULONG MainWindow_About(struct IClass *cl, Object *obj, Msg msg)
{
   MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), "\033b\033c" VERS "\033n\033c\n\n%ls\n\nAREXX port: '%ls'", GetStr(MSG_TX_About), xget(app, MUIA_Application_Base));
   return(NULL);
}

///
/// MainWindow_NextPage
ULONG MainWindow_NextPage(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);

   switch(data->Page)
   {
case 0:
      case 1:
         if(xget(data->RA_Interface, MUIA_Radio_Active))
            data->Page = 7;
         else
         {
            Object *window;

            strcpy(Config.cnf_serialdevice, "serial.device");
            Config.cnf_serialunit = 0;
            strcpy(Config.cnf_modemname, "Generic");
            strcpy(Config.cnf_initstring, "AT&F&D2");
            strcpy(Config.cnf_dialprefix, "ATDT");
            Config.cnf_dialsuffix[0] = NULL;
            Config.cnf_sana2device[0] = NULL;

            set(app, MUIA_Application_Sleep, TRUE);
            if(window = NewObject(CL_ModemDetect->mcc_Class, NULL, TAG_DONE))
            {
               DoMethod(app, OM_ADDMEMBER, window);
               set(window, MUIA_Window_Open, TRUE);
               DoMethod(window, MUIM_ModemDetect_FindModem);
            }
         }
         break;
      case 2:
         if(open_serial(Config.cnf_serialdevice, Config.cnf_serialunit))
            data->Page++;
         else
            MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_CouldNotOpenDevice_Unit), Config.cnf_serialdevice, Config.cnf_serialunit);
         break;
      case 6:
         if(xget(data->CH_Config, MUIA_Selected))
            if(!(save_config(xgetstr(data->STR_Config))))
               MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_ErrorSaveConfig), xgetstr(data->STR_Config));
         if(xget(data->CH_Info, MUIA_Selected))
         {
            BPTR fh;

            if(fh = Open(xgetstr(data->STR_Info), MODE_NEWFILE))
            {
               print_config(fh);
               Close(fh);
            }
            else
               MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_ErrorOpenX), xgetstr(data->STR_Info));
         }
         if(xget(data->CH_Printer, MUIA_Selected))
         {
            BPTR fh;

            if(fh = Open(xgetstr(data->STR_Printer), MODE_NEWFILE))
            {
               print_config(fh);
               Close(fh);
            }
            else
               MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_ErrorOpenX), xgetstr(data->STR_Printer));
         }
         DoMethod(app, MUIM_Application_PushMethod, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
         break;
      case 7:
/*         {
            Object *window;
            STRPTR ptr;

            Config.cnf_serialdevice[0] = NULL;
            strcpy(Config.cnf_sana2device, xgetstr(data->STR_SanaDevice));
            Config.cnf_sana2unit = xget(data->SL_SanaUnit, MUIA_Numeric_Value);
            strncpy(Config.cnf_ifname, FilePart(Config.cnf_sana2device), 20);
            if(ptr = strchr(Config.cnf_ifname, '.'))
               *ptr = NULL;
            sprintf(&Config.cnf_ifname[(strlen(Config.cnf_ifname) > 14 ? 14 : strlen(Config.cnf_ifname))], "%ld", Config.cnf_sana2unit);
            strlwr(Config.cnf_ifname);

            set(app, MUIA_Application_Sleep, TRUE);
            if(window = NewObject(CL_Online->mcc_Class, NULL, TAG_DONE))
            {
               DoMethod(app, OM_ADDMEMBER, window);
               set(window, MUIA_Window_Open, TRUE);
               DoMethod(window, MUIM_Online_GoOnline);
            }
         }
*/         data->Page--;
         break;
      default:
         data->Page++;
         break;
   }
   DoMethod(obj, MUIM_MainWindow_SetPage);
   return(NULL);
}

///
/// MainWindow_BackPage
ULONG MainWindow_BackPage(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);

   switch(data->Page)
   {
      case 7:
         data->Page = 1;
         break;
case 2:
data->Page = 0;
break;
      case 3:
         close_serial();
         data->Page--;
         break;
      case 5:
         if(dialing_try)
            DoMethod(obj, MUIM_MainWindow_HangUp);
         data->Page--;
         break;
      case 6:
         if(xget(data->RA_Interface, MUIA_Radio_Active))
            data->Page = 7;
         else
         {
            if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_BackCancel), GetStr(MSG_TX_GoBack)))
               data->Page--;
         }
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
   Object *new = NULL;

   if(data->Page < 0)
      data->Page = 0;
   if(data->Page > NUM_PAGES - 1)
      data->Page = NUM_PAGES - 1;

   set(data->BT_Back, MUIA_Disabled, (data->Page == 0));
   set(data->BT_Next, MUIA_Disabled, (data->Page == 5));

   new = data->ARR_Pages[data->Page];
   if(new && new != data->GR_Active)
   {
      DoMethod(group, MUIM_Group_InitChange);
      DoMethod(data->GR_Picture, MUIM_Group_InitChange);

      DoMethod(data->GR_Picture, OM_REMMEMBER, data->BC_Active);
      DoMethod(group, OM_ADDMEMBER, data->BC_Active);
      DoMethod(group, OM_REMMEMBER, data->ARR_Pictures[data->Page]);
      DoMethod(data->GR_Picture, OM_ADDMEMBER, data->ARR_Pictures[data->Page]);
      data->BC_Active = data->ARR_Pictures[data->Page];

      DoMethod(data->GR_Picture, MUIM_Group_ExitChange);
      set(data->FT_Info, MUIA_Floattext_Text, data->ARR_FT_Infos[data->Page]);
      DoMethod(data->GR_Pager, MUIM_Group_InitChange);

      DoMethod(data->GR_Pager, OM_REMMEMBER, data->GR_Active);
      DoMethod(group, OM_ADDMEMBER, data->GR_Active);
      DoMethod(group, OM_REMMEMBER, new);
      DoMethod(data->GR_Pager, OM_ADDMEMBER, new);
      data->GR_Active = new;

      DoMethod(data->GR_Pager, MUIM_Group_ExitChange);
      DoMethod(group, MUIM_Group_ExitChange);
   }

   if(data->Page == 5)
   {
      DoMethod(data->BT_GoOnline, MUIM_MultiSet, MUIA_Disabled, xget(data->CY_Script, MUIA_Cycle_Active),
         data->BT_GoOnline, data->BT_SendLogin, data->BT_SendPassword, data->BT_SendBreak, NULL);

      set(obj, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_None);
      set(obj, MUIA_Window_DefaultObject, data->TR_Terminal);
      DoMethod(li_script, MUIM_List_Clear);
      keyboard_input = FALSE;

      data->ihnode.ihn_Object  = obj;
      data->ihnode.ihn_Signals = SIG_SER;
      data->ihnode.ihn_Flags   = 0;
      data->ihnode.ihn_Method  = MUIM_MainWindow_Trigger;
      DoMethod(_app(obj), MUIM_Application_AddInputHandler, &data->ihnode);
      data->ihnode_added = TRUE;
   }
   else
   {
      if(data->ihnode_added)
      {
         DoMethod(_app(obj), MUIM_Application_RemInputHandler, &data->ihnode);
         data->ihnode_added = FALSE;
      }

      switch(data->Page)
      {
         case 3:
            set(obj, MUIA_Window_ActiveObject, data->STR_FullName);
            break;
         case 4:
            set(obj, MUIA_Window_ActiveObject, data->STR_PhoneNumber);
            break;
         case 7:
            set(obj, MUIA_Window_ActiveObject, data->STR_SanaDevice);
            break;
         default:
            set(obj, MUIA_Window_ActiveObject, data->BT_Next);
      }
   }

   set(data->BT_Next, MUIA_Text_Contents, (data->Page == 6 ? GetStr(MSG_BT_Finish) : GetStr(MSG_BT_Next)));
   set(data->BT_Next, MUIA_ShortHelp, (data->Page == 6 ? GetStr(MSG_HELP_Finish) : GetStr(MSG_HELP_Next)));

   return(NULL);
}

///
/// MainWindow_Trigger
ULONG MainWindow_Trigger(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   BOOL used = FALSE;

   if(ReadSER)
   {
      if(CheckIO(ReadSER))
      {
         WaitIO(ReadSER);

         DoMethod(data->TR_Terminal, TCM_WRITE, serial_in, 1);

         if(serial_in[0] == '\r' || serial_in[0] == '\n' || ser_buf_pos > 79)
         {
            if(serial_in[0] == '\r' && ser_buf_pos < 77)
               strcat(serial_buffer, "\\r");

            StartSerialRead(serial_in, 1);   // start read asap so we don't get hundreds of hits e.g. when doing MUI_Request below !!

            if(ser_buf_pos)
            {
               if(strstr(serial_buffer, "CONNECT") == serial_buffer && xget(data->CY_Script, MUIA_Cycle_Active))
                  DoMethod(win, MUIM_MainWindow_GoOnline);
               if(strstr(serial_buffer, "NO DIAL TONE") == serial_buffer || strstr(serial_buffer, "NO DIALTONE") == serial_buffer)
                  DoMethod(app, MUIM_Application_ReturnID, ID_NODIALTONE_REQ);
               if((strstr(serial_buffer, "NO CARRIER") == serial_buffer || strstr(serial_buffer, "BUSY") == serial_buffer) && dialing_try)
               {
                  set(app, MUIA_Application_Sleep, TRUE);
                  dial_number++;
                  Delay(80);
                  DoMethod(win, MUIM_MainWindow_Dial, FALSE);
                  set(app, MUIA_Application_Sleep, FALSE);
               }

               strcpy(serial_buffer_old2, serial_buffer_old1);
               strcpy(serial_buffer_old1, serial_buffer);
               serial_buffer[0] = NULL;
               ser_buf_pos = 0;
            }
         }
         else
         {
            switch(serial_in[0])
            {
               case 8:
                  if(ser_buf_pos)
                     ser_buf_pos--;
                  serial_buffer[ser_buf_pos] = NULL;
                  StartSerialRead(serial_in, 1);
                  break;

               default:
               if(serial_in[0])
               {
                  serial_buffer[ser_buf_pos++]  = serial_in[0];
                  serial_buffer[ser_buf_pos]    = NULL;
                  StartSerialRead(serial_in, 1);

                  if(have_ppp_frame(serial_buffer, ser_buf_pos))
                  {
                     if(data->ihnode_added)
                     {
                        DoMethod(app, MUIM_Application_RemInputHandler, &data->ihnode);
                        data->ihnode_added = FALSE;
                     }
                     DoMethod(win, MUIM_MainWindow_GoOnline);
                  }
               }
               break;
            }
         }

         used = TRUE;
      }
   }

   return(used);
}

///
/// MainWindow_ChangeConfig
ULONG MainWindow_ChangeConfig(struct IClass *cl, Object *obj, Msg msg)
{
   Object *window;

   set(app, MUIA_Application_Sleep, TRUE);
   if(window = NewObject(CL_ModemWindow->mcc_Class, NULL, TAG_DONE))
   {
      DoMethod(app, OM_ADDMEMBER, window);
      DoMethod(window, MUIM_ModemWindow_Init);
      set(window, MUIA_Window_Open, TRUE);
   }

   return(NULL);
}

///
/// MainWindow_ShowConfig
ULONG MainWindow_ShowConfig(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   Object *window, *script;
   char ip_info[21], dest_info[21], dns1_info[21], dns2_info[21], device_info[81];

   if(addr_assign == CNF_Assign_Static)
      sprintf(ip_info, "%ls (%ls)", ip, GetStr(MSG_TX_Static));
   else
   {
      if(addr_assign == CNF_Assign_BootP)
         sprintf(ip_info, "%ls (BOOTP)", GetStr(MSG_TX_Dynamic));
      else
         sprintf(ip_info, "%ls (ICPC)", GetStr(MSG_TX_Dynamic));
   }

   if(*dest)
   {
      if(dst_assign == CNF_Assign_BootP)
         sprintf(dest_info, "%ls (BOOTP)", dest);
      else
         sprintf(dest_info, "%ls (ICPC)", dest);
   }
   else
      strcpy(dest_info, GetStr(MSG_TX_Undefined));

   if(*dns1)
   {
      if(dns_assign == CNF_Assign_BootP)
         sprintf(dns1_info, "%ls (BOOTP)", dns1);
      else if(dns_assign == CNF_Assign_Root)
         sprintf(dns1_info, "%ls (ROOT)", dns1);
      else
         sprintf(dns1_info, "%ls (MSDNS)", dns1);
   }
   else
      strcpy(dns1_info, GetStr(MSG_TX_Undefined));

   if(*dns2)
   {
      if(dns_assign == CNF_Assign_BootP)
         sprintf(dns2_info, "%ls (BOOTP)", dns2);
      else if(dns_assign == CNF_Assign_Root)
         sprintf(dns2_info, "%ls (ROOT)", dns2);
      else
         sprintf(dns2_info, "%ls (MSDNS)", dns2);
   }
   else
      strcpy(dns2_info, GetStr(MSG_TX_Undefined));

   if(xget(data->RA_Interface, MUIA_Radio_Active))
      sprintf(device_info, "%ls, unit %ld", FilePart(Config.cnf_sana2device), Config.cnf_sana2unit);
   else
      sprintf(device_info, "%ls, unit %ld", Config.cnf_serialdevice, Config.cnf_serialunit);

   if(window = WindowObject,
      MUIA_Window_Title , GetStr(MSG_TX_ShowConfigWindowTitle),
      MUIA_Window_ID    , MAKE_ID('N','C','O','F'),
      WindowContents    , VGroup,
         Child, CLabel(GetStr(MSG_LA_ShowConfigTitle)),
         Child, ColGroup(2),
            Child, ColGroup(2),
               GroupFrame,
               MUIA_Background, MUII_GroupBack,
               Child, Label(GetStr(MSG_BLA_Device)),
               Child, MakeText(device_info),
               Child, Label(GetStr(MSG_BLA_Modem)),
               Child, MakeText(Config.cnf_modemname),
               Child, Label(GetStr(MSG_BLA_Phone)),
               Child, MakeText(xgetstr(data->STR_PhoneNumber)),
               Child, Label(GetStr(MSG_BLA_UserName)),
               Child, MakeText(xgetstr(data->STR_UserName)),
            End,
            Child, ColGroup(2),
               GroupFrame,
               MUIA_Background, MUII_GroupBack,
               Child, Label(GetStr(MSG_BLA_Speed)),
               Child, MakeText("38'400 baud"),
               Child, Label(GetStr(MSG_BLA_InitString)),
               Child, MakeText(Config.cnf_initstring),
               Child, Label(GetStr(MSG_BLA_DialPrefix)),
               Child, MakeText(Config.cnf_dialprefix),
               Child, Label(GetStr(MSG_BLA_DialSuffix)),
               Child, MakeText(Config.cnf_dialsuffix),
            End,
            Child, ColGroup(2),
               GroupFrame,
               MUIA_Background, MUII_GroupBack,
               Child, Label(GetStr(MSG_BLA_LocalIPAddr)),
               Child, MakeText(ip_info),
               Child, Label(GetStr(MSG_BLA_RemoteIPAddr)),
               Child, MakeText(dest_info),
               Child, Label(GetStr(MSG_BLA_Netmask)),
               Child, MakeText((*mask ? (STRPTR)mask : GetStr(MSG_TX_Undefined))),
            End,
            Child, ColGroup(2),
               GroupFrame,
               MUIA_Background, MUII_GroupBack,
               Child, Label(GetStr(MSG_BLA_DNS1IPAddr)),
               Child, MakeText(dns1_info),
               Child, Label(GetStr(MSG_BLA_DNS2IPAddr)),
               Child, MakeText(dns2_info),
               Child, Label(GetStr(MSG_BLA_DomainName)),
               Child, MakeText(Config.cnf_domainname),
            End,
         End,
         Child, script = ListviewObject,
            MUIA_ShowMe    , !xget(data->RA_Interface, MUIA_Radio_Active),
            MUIA_FrameTitle, GetStr(MSG_LA_LoginScript_List),
            MUIA_Listview_Input, FALSE,
            MUIA_Listview_List, ListObject,
               ReadListFrame,
               MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
               MUIA_List_DestructHook, MUIV_List_DestructHook_String,
            End,
         End,
      End,
   End)
   {
      STRPTR ptr;
      LONG pos;

      DoMethod(window, MUIM_Notify, MUIA_Window_CloseRequest, TRUE ,
      MUIV_Notify_Application, 5, MUIM_Application_PushMethod,
      win, 2, MUIM_MainWindow_DisposeWindow, window);

      set(app, MUIA_Application_Sleep, TRUE);
      DoMethod(app, OM_ADDMEMBER, window);

      pos = 0;
      FOREVER
      {
         DoMethod(li_script, MUIM_List_GetEntry, pos++, &ptr);
         if(!ptr)
            break;
         DoMethod(script, MUIM_List_InsertSingle, ptr, MUIV_List_Insert_Bottom);
      }

      set(window, MUIA_Window_Open, TRUE);
   }
   return(NULL);
}

///
/// MainWindow_Abort
ULONG MainWindow_Abort(struct IClass *cl, Object *obj, Msg msg)
{
   if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_QuitCancel), GetStr(MSG_TX_ReallyQuit)))
      DoMethod(app, MUIM_Application_PushMethod, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

   return(NULL);
}

///
/// do_keyboard
VOID do_keyboard(VOID)
{
   if(keyboard_input)
   {
      char buffer[100];

      sprintf(buffer, "Send \"%ls\"", keyboard_buffer);
      DoMethod(li_script, MUIM_List_InsertSingle, buffer, MUIV_List_Insert_Bottom);
      keyboard_input = FALSE;
      keyboard_buffer[0] = NULL;
      key_buf_pos = 0;
   }
}

///
/// do_serial
VOID do_serial(VOID)
{
   char buffer[101];

   sprintf(buffer, "WaitFor \"%ls\"", (*serial_buffer ? serial_buffer : (*serial_buffer_old1 ? serial_buffer_old1 : serial_buffer_old2)));
   DoMethod(li_script, MUIM_List_InsertSingle, buffer, MUIV_List_Insert_Bottom);
   serial_buffer[0] = serial_buffer_old1[0] = serial_buffer_old2[0] = NULL;
   ser_buf_pos = 0;
}

///
/// MainWindow_Dial
ULONG MainWindow_Dial(struct IClass *cl, Object *obj, struct MUIP_MainWindow_Dial *msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   char buffer1[240], buffer2[81];
   STRPTR ptr;
   int i;

   if(msg->restart)
   {
      dialing_try = 1;
      dial_number = 0;
      do_keyboard();
      DoMethod(li_script, MUIM_List_Clear);
      DoMethod(li_script, MUIM_List_InsertSingle, "Dial", MUIV_List_Insert_Bottom);
   }

   if(!dialing_try)
      return(NULL);

   do
   {
      i = dial_number + 1;
      ptr = xgetstr(data->STR_PhoneNumber);
      while(i-- && ptr)
         ptr = extract_arg(ptr, buffer2, 80, '|');

      if(i > -1)
      {
         dial_number = 0;
         dialing_try++;
         if(dialing_try > 9)
         {
            MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_CouldNotConnect));
            return(NULL);
         }
      }
   } while(i > -1);

   DoMethod(data->TR_Terminal, TCM_INIT);
   Delay(10);
   sprintf(buffer1, GetStr(MSG_TX_Dialing), buffer2, dialing_try);
   DoMethod(data->TR_Terminal, TCM_WRITE, buffer1, strlen(buffer1));

   EscapeString(buffer1, Config.cnf_initstring);
   strcat(buffer1, "\r");
   send_serial(buffer1, -1);
   Delay(50);

   EscapeString(buffer1, Config.cnf_dialprefix);
   strcat(buffer1, buffer2);
   EscapeString(buffer2, Config.cnf_dialsuffix);
   strcat(buffer1, buffer2);
   strcat(buffer1, "\r");
   send_serial(buffer1, -1);

   return(NULL);
}

///
/// MainWindow_GoOnline
ULONG MainWindow_GoOnline(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   Object *window;

   if(data->ihnode_added)
   {
      DoMethod(app, MUIM_Application_RemInputHandler, &data->ihnode);
      data->ihnode_added = FALSE;
   }

   strcpy(Config.cnf_ifname, (xget(data->CY_Protocol, MUIA_Cycle_Active) ? "slip" : "ppp"));
   dialing_try = 0;
   do_keyboard();
   DoMethod(li_script, MUIM_List_InsertSingle, "GoOnline", MUIV_List_Insert_Bottom);

   set(app, MUIA_Application_Sleep, TRUE);
   if(window = NewObject(CL_Online->mcc_Class, NULL, TAG_DONE))
   {
      DoMethod(app, OM_ADDMEMBER, window);
      set(window, MUIA_Window_Open, TRUE);
      DoMethod(window, MUIM_Online_GoOnline);
   }

   return(NULL);
}

///
/// MainWindow_HangUp
ULONG MainWindow_HangUp(struct IClass *cl, Object *obj, Msg msg)
{
//   struct MainWindow_Data *data = INST_DATA(cl, obj);

   set(obj, MUIA_Window_Sleep, TRUE);

   dialing_try = 0;
   do_keyboard();

   close_serial();
   Delay(70);
   open_serial(Config.cnf_serialdevice, Config.cnf_serialunit);

   if(serial_carrier())
   {
      send_serial("+", 1);
      Delay(20);
      send_serial("+", 1);
      Delay(20);
      send_serial("+", 1);
      Delay(20);
      send_serial("ATH0\r", -1);
   }
   else
      send_serial("\r", 1);

   set(obj, MUIA_Window_Sleep, FALSE);

   return(NULL);
}

///
/// MainWindow_SendLogin
ULONG MainWindow_SendLogin(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   char buffer[101];

   dialing_try = 0;
   do_keyboard();
   do_serial();
   DoMethod(li_script, MUIM_List_InsertSingle, "SendLogin", MUIV_List_Insert_Bottom);

   EscapeString(buffer, xgetstr(data->STR_UserName));
   strcat(buffer, "\r");
   send_serial(buffer, -1);

   return(NULL);
}

///
/// MainWindow_SendPassword
ULONG MainWindow_SendPassword(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   char buffer[101];

   dialing_try = 0;
   do_keyboard();
   do_serial();
   DoMethod(li_script, MUIM_List_InsertSingle, "SendPassword", MUIV_List_Insert_Bottom);

   EscapeString(buffer, xgetstr(data->STR_Password));
   strcat(buffer, "\r");
   send_serial(buffer, -1);

   return(NULL);
}

///
/// MainWindow_SendBreak
ULONG MainWindow_SendBreak(struct IClass *cl, Object *obj, Msg msg)
{
   dialing_try = 0;
   do_keyboard();
   do_serial();
   DoMethod(li_script, MUIM_List_InsertSingle, "SendBreak", MUIV_List_Insert_Bottom);
   WriteSER->IOSer.io_Command = SDCMD_BREAK;
   DoIO(WriteSER);

   return(NULL);
}

///
/// MainWindow_DisposeWindow
ULONG MainWindow_DisposeWindow(struct IClass *cl, Object *obj, struct MUIP_MainWindow_DisposeWindow *msg)
{
   if(msg->window)
   {
      set(msg->window, MUIA_Window_Open, FALSE);
Delay(10);
      DoMethod(_app(msg->window), OM_REMMEMBER, msg->window);
      MUI_DisposeObject(msg->window);
   }

   set(app, MUIA_Application_Sleep, FALSE);

   return(NULL);
}

///
/// MainWindow_Input
ULONG MainWindow_Input(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   unsigned char *ptr;
   ULONG len;

   dialing_try = 0;

   get(data->TR_Terminal, TCA_OUTPTR, &ptr);
   get(data->TR_Terminal, TCA_OUTLEN, &len);
   if(len == 3)
   {
      if(ptr[0] == 155 && ptr[2] == '~')
      {
         switch(ptr[1])
         {
            case '0':
               DoMethod(obj, MUIM_MainWindow_Dial, TRUE);
               break;
            case '1':
               DoMethod(obj, MUIM_MainWindow_GoOnline);
               break;
            case '2':
               DoMethod(obj, MUIM_MainWindow_HangUp);
               break;
            case '3':
               DoMethod(obj, MUIM_MainWindow_SendLogin);
               break;
            case '4':
               DoMethod(obj, MUIM_MainWindow_SendPassword);
               break;
            case '5':
               DoMethod(obj, MUIM_MainWindow_SendBreak);
               break;
         }
      }
   }
   else
   {
      switch(*ptr)
      {
         case '\r':
            keyboard_buffer[key_buf_pos++] = '\\';
            keyboard_buffer[key_buf_pos++] = 'r';
            break;
         case '\n':
            keyboard_buffer[key_buf_pos++] = '\\';
            keyboard_buffer[key_buf_pos++] = 'n';
            break;
         case 8:
            if(key_buf_pos)
               key_buf_pos--;
            keyboard_buffer[key_buf_pos] = NULL;
            break;
         default:
            keyboard_buffer[key_buf_pos++] = *ptr;
            break;
      }
      keyboard_buffer[key_buf_pos]  = NULL;
      if(!keyboard_input)
      {
         do_serial();
         keyboard_input = TRUE;
      }

      if(*ptr == '\r' || key_buf_pos > 79)
         do_keyboard();

      send_serial(ptr, len);
   }

   DoMethod(data->TR_Terminal, TCM_OUTFLUSH);

   return(NULL);
}

///
/// MainWindow_New
ULONG MainWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct MainWindow_Data tmp;
   Object *GR_Pages;
   static STRPTR ARR_RA_Interface[3], ARR_CY_Protocol[3], ARR_CY_IPAddress[3], ARR_CY_Script[3];

   ARR_RA_Interface[0] = GetStr(MSG_RA_Interface_Serial);
   ARR_RA_Interface[1] = GetStr(MSG_RA_Interface_Sana);
   ARR_RA_Interface[2] = NULL;

   ARR_CY_Protocol[0] = "ppp";
   ARR_CY_Protocol[1] = "slip";
   ARR_CY_Protocol[2] = NULL;

   ARR_CY_IPAddress[0] = GetStr(MSG_TX_Dynamic);
   ARR_CY_IPAddress[1] = GetStr(MSG_TX_Static);
   ARR_CY_IPAddress[2] = NULL;

   ARR_CY_Script[0] = GetStr(MSG_CY_Script_Record);
   ARR_CY_Script[1] = GetStr(MSG_CY_Script_DontRecord);
   ARR_CY_Script[2] = NULL;

   tmp.ARR_FT_Infos[0] = GetStr(MSG_FT_Infos1);
   tmp.ARR_FT_Infos[1] = GetStr(MSG_FT_Infos2);
   tmp.ARR_FT_Infos[2] = GetStr(MSG_FT_Infos3);
   tmp.ARR_FT_Infos[3] = GetStr(MSG_FT_Infos4);
   tmp.ARR_FT_Infos[4] = GetStr(MSG_FT_Infos5);
   tmp.ARR_FT_Infos[5] = GetStr(MSG_FT_Infos6);
   tmp.ARR_FT_Infos[6] = GetStr(MSG_FT_Infos7);
   tmp.ARR_FT_Infos[7] = GetStr(MSG_FT_Infos8);

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , GetStr(MSG_TX_MainWindowTitle),
      MUIA_Window_ID       , MAKE_ID('M','A','I','N'),
      MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, MainMenu, NULL),
      WindowContents       , VGroup,
         Child, HGroup,
            Child, VGroup,
               GroupSpacing(0),
               Child, tmp.GR_Picture = VGroup,
                  GroupSpacing(0),
                  Child, tmp.ARR_Pictures[0] = tmp.BC_Active = BodychunkObject,
                     ReadListFrame,
                     MUIA_FixWidth             , SETUP_PAGE0_WIDTH,
                     MUIA_FixHeight            , SETUP_PAGE0_HEIGHT,
                     MUIA_Bitmap_Width         , SETUP_PAGE0_WIDTH ,
                     MUIA_Bitmap_Height        , SETUP_PAGE0_HEIGHT,
                     MUIA_Bodychunk_Depth      , SETUP_PAGE0_DEPTH ,
                     MUIA_Bodychunk_Body       , (UBYTE *)setup_page0_body,
                     MUIA_Bodychunk_Compression, SETUP_PAGE0_COMPRESSION,
                     MUIA_Bodychunk_Masking    , SETUP_PAGE0_MASKING,
                     MUIA_Bitmap_SourceColors  , (ULONG *)setup_page0_colors,
                     MUIA_Bitmap_Transparent   , 0,
                  End,
               End,
               Child, HVSpace,
            End,
            Child, VGroup,
               Child, ListviewObject,
                  MUIA_Background, MUII_TextBack,
                  MUIA_Listview_Input  , FALSE,
                  MUIA_Listview_List   , tmp.FT_Info = FloattextObject,
                     ReadListFrame,
                     MUIA_Floattext_Text, tmp.ARR_FT_Infos[0],
                  End,
               End,
               Child, BalanceObject, End,
               Child, tmp.GR_Pager = VGroup,
                  GroupFrame,
                  MUIA_Background, MUII_GroupBack,
                  Child, tmp.ARR_Pages[0] = tmp.GR_Active = VGroup,
                     Child, HVSpace,
                     Child, CLabel(GetStr(MSG_TX_Welcome)),
                     Child, HVSpace,
                  End,
               End,
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

      if(GR_Pages = VGroup,
         Child, tmp.ARR_Pages[1] = VGroup,   // Serial connection with Modem (PPP/Slip) or direct connection (Ethernet/Arcnet etc.) with Network card
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
         Child, tmp.ARR_Pages[2] = VGroup,   // Verify serial options
            Child, HVSpace,
            Child, ColGroup(2),
               Child, Label(GetStr(MSG_BLA_Device)),
               Child, tmp.TX_Device = MakeText(NULL),
               Child, HVSpace,
               Child, HVSpace,
               Child, Label(GetStr(MSG_BLA_Modem)),
               Child, tmp.TX_Modem = MakeText(NULL),
               Child, HVSpace,
               Child, HVSpace,
               Child, Label(GetStr(MSG_BLA_InitString)),
               Child, tmp.TX_InitString = MakeText(NULL),
               Child, Label(GetStr(MSG_BLA_DialPrefix)),
               Child, tmp.TX_DialPrefix = MakeText(NULL),
            End,
            Child, HVSpace,
            Child, tmp.BT_ChangeConfig = MakeButton(MSG_BT_ModifyModemSettings),
         End,
         Child, tmp.ARR_Pages[3] = VGroup,   // enter user info
            Child, HVSpace,
            Child, ColGroup(2),
               Child, MakeKeyLabel(MSG_LA_FullName, MSG_CC_FullName),
               Child, tmp.STR_FullName = MakeKeyString(NULL, 80, MSG_CC_FullName),
               Child, MakeKeyLabel(MSG_LA_UserName, MSG_CC_UserName),
               Child, tmp.STR_UserName = MakeKeyString(NULL, 80, MSG_CC_UserName),
               Child, MakeKeyLabel(MSG_LA_Password, MSG_CC_Password),
               Child, tmp.STR_Password = TextinputObject,
                  MUIA_ControlChar        , *GetStr(MSG_CC_Password),
                  MUIA_CycleChain         , 1,
                  MUIA_Frame              , MUIV_Frame_String,
                  MUIA_Textinput_Secret   , TRUE,
                  MUIA_Textinput_Multiline, FALSE,
                  MUIA_Textinput_MaxLen   , 80,
               End,
            End,
            Child, HVSpace,
         End,
         Child, tmp.ARR_Pages[4] = VGroup,   // enter/verify basic isp information
            Child, HVSpace,
            Child, ColGroup(2),
               Child, MakeKeyLabel(MSG_LA_Phone, MSG_CC_Phone),
               Child, tmp.STR_PhoneNumber = MakeKeyString("", 80, MSG_CC_Phone),
               Child, MakeKeyLabel(MSG_LA_IPAddress, MSG_CC_IPAddress),
               Child, HGroup,
                  Child, tmp.CY_IPAddress = Cycle(ARR_CY_IPAddress),
                  Child, tmp.STR_IPAddress = TextinputObject,
                     MUIA_ControlChar     , *GetStr(MSG_CC_IPAddress),
                     MUIA_CycleChain      , 1,
                     StringFrame,
                     MUIA_Textinput_MaxLen   , 20,
                     MUIA_Textinput_Contents , "0.0.0.0",
                     MUIA_Textinput_AcceptChars, "1234567890.",
                     MUIA_Textinput_Multiline, FALSE,
                  End,
               End,
               Child, MakeKeyLabel(MSG_LA_Protocol, MSG_CC_Protocol),
               Child, tmp.CY_Protocol = MakeKeyCycle(ARR_CY_Protocol, MSG_CC_Protocol),
               Child, MakeKeyLabel(MSG_LA_LoginScript, MSG_CC_LoginScript),
               Child, tmp.CY_Script = MakeKeyCycle(ARR_CY_Script, MSG_CC_LoginScript),
            End,
            Child, HVSpace,
         End,
         Child, tmp.ARR_Pages[5] = VGroup,   // login script recorder
            GroupSpacing(0),
            Child, HGroup,
               GroupSpacing(0),
               InnerSpacing(0, 0),
               Child, tmp.TR_Terminal = TermObject,
                  MUIA_CycleChain, 1,
                  InputListFrame,
                  TCA_EMULATION  , TCV_EMULATION_VT100,
                  TCA_LFASCRLF   , TRUE,
                  TCA_DESTRBS    , TRUE,
                  TCA_ECHO       , FALSE,
                  TCA_DELASBS    , TRUE,
                  TCA_CURSORSTYLE, TCV_CURSORSTYLE_UNDERLINED,
                  TCA_SELECT     , TRUE,
                  TCA_8BIT       , TRUE,
                  TCA_WRAP       , FALSE,
               End,
               Child, tmp.SB_Terminal = ScrollbarObject,
                  MUIA_Weight, 0,
               End,
            End,
            Child, ColGroup(3),
               GroupSpacing(0),
               Child, tmp.BT_Dial         = MakeButton(MSG_BT_Dial),
               Child, tmp.BT_GoOnline     = MakeButton(MSG_BT_GoOnline),
               Child, tmp.BT_HangUp       = MakeButton(MSG_BT_HangUp),
               Child, tmp.BT_SendLogin    = MakeButton(MSG_BT_SendLogin),
               Child, tmp.BT_SendPassword = MakeButton(MSG_BT_SendPassword),
               Child, tmp.BT_SendBreak    = MakeButton(MSG_BT_SendBreak),
            End,
         End,
         Child, tmp.ARR_Pages[6] = VGroup,   // finished (option to save config and save/print info sheet)
            Child, HVSpace,
            Child, ColGroup(3),
               Child, MakeKeyLabel(MSG_LA_SaveConfiguration, MSG_CC_SaveConfiguration),
               Child, tmp.CH_Config = MakeCheckMark(TRUE),
               Child, MakePopAsl(tmp.STR_Config = MakeKeyString("AmiTCP:db/SetupAmiTCP.config", MAXPATHLEN, MSG_CC_SaveConfiguration), MSG_TX_SaveConfiguration, FALSE),
               Child, MakeKeyLabel(MSG_LA_SaveInformation, MSG_CC_SaveInformation),
               Child, tmp.CH_Info = MakeCheckMark(TRUE),
               Child, MakePopAsl(tmp.STR_Info = MakeKeyString("PROGDIR:SetupAmiTCP.log", MAXPATHLEN, MSG_CC_SaveInformation), MSG_TX_SaveInformation, FALSE),
               Child, MakeKeyLabel(MSG_LA_PrintConfig, MSG_CC_PrintConfig),
               Child, tmp.CH_Printer = MakeCheckMark(FALSE),
               Child, tmp.STR_Printer = MakeKeyString("PRT:", 80, MSG_CC_PrintConfig),
            End,
            Child, HVSpace,
            Child, HGroup,
               Child, HVSpace,
               Child, tmp.BT_ShowConfig = MakeButton(MSG_BT_ShowConfigDetails),
               Child, HVSpace,
            End,
            Child, HVSpace,
         End,
         Child, tmp.ARR_Pages[7] = VGroup,   // Choose sanaII device (try to find out configuration)
            Child, HVSpace,
            Child, ColGroup(2),
               Child, MakeKeyLabel(MSG_LA_SanaDeviceDriver, MSG_CC_SanaDeviceDriver),
               Child, MakePopAsl(tmp.STR_SanaDevice = MakeKeyString("DEVS:Networks/", MAXPATHLEN, MSG_CC_SanaDeviceDriver), MSG_TX_SanaDeviceDriver, FALSE),
               Child, MakeKeyLabel(MSG_LA_Unit, MSG_CC_Unit),
               Child, HGroup,
                  Child, tmp.SL_SanaUnit = NumericbuttonObject,
                     MUIA_CycleChain      , 1,
                     MUIA_ControlChar     , *GetStr(MSG_CC_Unit),
                     MUIA_Numeric_Min     , 0,
                     MUIA_Numeric_Max     , 20,
                     MUIA_Numeric_Value   , 0,
                  End,
                  Child, HVSpace,
               End,
            End,
            Child, HVSpace,
         End,

/*         Child, tmp.ARR_Pictures[1] = BodychunkObject,
            ReadListFrame,
            MUIA_FixWidth             , SETUP_PAGE1_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE1_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE1_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE1_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE1_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page1_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE1_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE1_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page1_colors,
            MUIA_Bitmap_Transparent   , 0,
         End,
*/         Child, tmp.ARR_Pictures[2] = BodychunkObject,
            ReadListFrame,
            MUIA_FixWidth             , SETUP_PAGE2_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE2_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE2_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE2_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE2_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page2_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE2_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE2_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page2_colors,
            MUIA_Bitmap_Transparent   , 0,
         End,
         Child, tmp.ARR_Pictures[3] = BodychunkObject,
            ReadListFrame,
            MUIA_FixWidth             , SETUP_PAGE3_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE3_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE3_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE3_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE3_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page3_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE3_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE3_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page3_colors,
            MUIA_Bitmap_Transparent   , 0,
         End,
         Child, tmp.ARR_Pictures[4] = BodychunkObject,
            ReadListFrame,
            MUIA_FixWidth             , SETUP_PAGE4_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE4_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE4_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE4_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE4_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page4_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE4_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE4_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page4_colors,
            MUIA_Bitmap_Transparent   , 0,
         End,
         Child, tmp.ARR_Pictures[5] = BodychunkObject,
            ReadListFrame,
            MUIA_FixWidth             , SETUP_PAGE5_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE5_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE5_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE5_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE5_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page5_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE5_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE5_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page5_colors,
            MUIA_Bitmap_Transparent   , 0,
         End,
         Child, tmp.ARR_Pictures[6] = BodychunkObject,
            ReadListFrame,
            MUIA_FixWidth             , SETUP_PAGE6_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE6_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE6_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE6_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE6_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page6_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE6_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE6_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page6_colors,
            MUIA_Bitmap_Transparent   , 0,
         End,
/*         Child, tmp.ARR_Pictures[7] = BodychunkObject,
            ReadListFrame,
            MUIA_FixWidth             , SETUP_PAGE7_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE7_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE7_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE7_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE7_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page7_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE7_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE7_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page7_colors,
            MUIA_Bitmap_Transparent   , 0,
         End,
*/      End)
      {
         DoMethod(group, OM_ADDMEMBER, GR_Pages);
         group = GR_Pages;    // the old group isn't the parent of the pages => replace with correct group-pointer
         tmp.Page = 0;

         *data = tmp;

         data->ihnode_added = FALSE;

         set(obj, MUIA_Window_ActiveObject, data->BT_Next);
         set(data->BT_Back, MUIA_Disabled, TRUE);
         set(data->STR_IPAddress, MUIA_Disabled, TRUE);
         set(data->CY_IPAddress, MUIA_Weight, 10);
         set(data->BT_ShowConfig, MUIA_Weight, 300);
         set(data->CY_IPAddress, MUIA_CycleChain, 1);

         set(data->BT_Back          , MUIA_ShortHelp, GetStr(MSG_HELP_Back));
         set(data->BT_Next          , MUIA_ShortHelp, GetStr(MSG_HELP_Next));
         set(data->BT_Abort         , MUIA_ShortHelp, GetStr(MSG_HELP_Abort));
         set(data->BT_ChangeConfig  , MUIA_ShortHelp, GetStr(MSG_HELP_ChangeModem));
         set(data->STR_FullName     , MUIA_ShortHelp, GetStr(MSG_HELP_FullName));
         set(data->STR_UserName     , MUIA_ShortHelp, GetStr(MSG_HELP_UserName));
         set(data->STR_Password     , MUIA_ShortHelp, GetStr(MSG_HELP_Password));
         set(data->STR_PhoneNumber  , MUIA_ShortHelp, GetStr(MSG_HELP_PhoneNumber));
         set(data->STR_IPAddress    , MUIA_ShortHelp, GetStr(MSG_HELP_IPAddress));
         set(data->CY_IPAddress     , MUIA_ShortHelp, GetStr(MSG_HELP_DynamicStatic));
         set(data->CY_Protocol      , MUIA_ShortHelp, GetStr(MSG_HELP_Protocol));
         set(data->CY_Script        , MUIA_ShortHelp, GetStr(MSG_HELP_RecordScript));
         set(data->BT_Dial          , MUIA_ShortHelp, GetStr(MSG_HELP_Dial));
         set(data->BT_GoOnline      , MUIA_ShortHelp, GetStr(MSG_HELP_GoOnline));
         set(data->BT_HangUp        , MUIA_ShortHelp, GetStr(MSG_HELP_HangUp));
         set(data->BT_SendLogin     , MUIA_ShortHelp, GetStr(MSG_HELP_SendLogin));
         set(data->BT_SendPassword  , MUIA_ShortHelp, GetStr(MSG_HELP_SendPassword));
         set(data->BT_SendBreak     , MUIA_ShortHelp, GetStr(MSG_HELP_SendBreak));
         set(data->CH_Config        , MUIA_ShortHelp, GetStr(MSG_HELP_DoSaveConfig));
         set(data->CH_Info          , MUIA_ShortHelp, GetStr(MSG_HELP_DoSaveInfo));
         set(data->CH_Printer       , MUIA_ShortHelp, GetStr(MSG_HELP_DoPrint));
         set(data->STR_Config       , MUIA_ShortHelp, GetStr(MSG_HELP_ConfigFilename));
         set(data->STR_Info         , MUIA_ShortHelp, GetStr(MSG_HELP_InfoFilename));
         set(data->STR_Printer      , MUIA_ShortHelp, GetStr(MSG_HELP_PrinterDevice));
         set(data->BT_ShowConfig    , MUIA_ShortHelp, GetStr(MSG_HELP_ShowConfig));

         set(data->TR_Terminal, TCA_SCROLLER, data->SB_Terminal);

         DoMethod(obj            , MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 1, MUIM_MainWindow_Abort);
         DoMethod(data->BT_Next    , MUIM_Notify, MUIA_Pressed   , FALSE  , obj, 1, MUIM_MainWindow_NextPage);
         DoMethod(data->BT_Back    , MUIM_Notify, MUIA_Pressed   , FALSE  , obj, 1, MUIM_MainWindow_BackPage);
         DoMethod(data->BT_Abort   , MUIM_Notify, MUIA_Pressed   , FALSE  , obj, 1, MUIM_MainWindow_Abort);

         DoMethod(data->BT_ChangeConfig     , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_ChangeConfig);

         DoMethod(data->STR_FullName        , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_UserName);
         DoMethod(data->STR_UserName        , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Password);
         DoMethod(data->STR_Password        , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->BT_Next);

         DoMethod(data->STR_PhoneNumber     , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->BT_Next);
         DoMethod(data->CY_IPAddress        , MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, data->STR_IPAddress, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

         DoMethod(data->SB_Terminal, MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime, data->TR_Terminal, 1, TCM_SCROLLER);
         DoMethod(data->TR_Terminal, MUIM_Notify, TCA_OUTLEN, MUIV_EveryTime, obj, 1, MUIM_MainWindow_Input);
         DoMethod(data->BT_Dial             , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 2, MUIM_MainWindow_Dial, TRUE);
         DoMethod(data->BT_GoOnline         , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_GoOnline);
         DoMethod(data->BT_HangUp           , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_HangUp);
         DoMethod(data->BT_SendLogin        , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_SendLogin);
         DoMethod(data->BT_SendPassword     , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_SendPassword);
         DoMethod(data->BT_SendBreak        , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_SendBreak);

         DoMethod(data->BT_ShowConfig       , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_MainWindow_ShowConfig);

         DoMethod(data->STR_SanaDevice      , MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->BT_Next);

         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT)    , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, obj, 1, MUIM_MainWindow_About);
         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT_MUI), MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_AboutMUI, win);
         DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_QUIT)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, obj, 1, MUIM_MainWindow_Abort);
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
SAVEDS ASM ULONG MainWindow_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW                            : return(MainWindow_New                (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Input             : return(MainWindow_Input              (cl, obj, (APTR)msg));
      case MUIM_MainWindow_SetPage           : return(MainWindow_SetPage            (cl, obj, (APTR)msg));
      case MUIM_MainWindow_NextPage          : return(MainWindow_NextPage           (cl, obj, (APTR)msg));
      case MUIM_MainWindow_BackPage          : return(MainWindow_BackPage           (cl, obj, (APTR)msg));
      case MUIM_MainWindow_About             : return(MainWindow_About              (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Abort             : return(MainWindow_Abort              (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Dial              : return(MainWindow_Dial               (cl, obj, (APTR)msg));
      case MUIM_MainWindow_GoOnline          : return(MainWindow_GoOnline           (cl, obj, (APTR)msg));
      case MUIM_MainWindow_HangUp            : return(MainWindow_HangUp             (cl, obj, (APTR)msg));
      case MUIM_MainWindow_SendLogin         : return(MainWindow_SendLogin          (cl, obj, (APTR)msg));
      case MUIM_MainWindow_SendPassword      : return(MainWindow_SendPassword       (cl, obj, (APTR)msg));
      case MUIM_MainWindow_SendBreak         : return(MainWindow_SendBreak          (cl, obj, (APTR)msg));
      case MUIM_MainWindow_DisposeWindow     : return(MainWindow_DisposeWindow      (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Trigger           : return(MainWindow_Trigger            (cl, obj, (APTR)msg));
      case MUIM_MainWindow_ChangeConfig      : return(MainWindow_ChangeConfig       (cl, obj, (APTR)msg));
      case MUIM_MainWindow_ShowConfig        : return(MainWindow_ShowConfig         (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

