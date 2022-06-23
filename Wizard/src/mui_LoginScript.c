/// includes
#include "/includes.h"

#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_LoginScript.h"
#include "mui_MainWindow.h"
#include "mui_Online.h"
#include "protos.h"

///
/// external variables
extern struct Library *MUIMasterBase;
extern Object *app, *win, *status_win;
extern struct Config Config;
extern struct MUI_CustomClass  *CL_Online;
extern int dialing_try, dial_number;
extern BOOL keyboard_input;
extern BOOL use_loginscript;
extern char serial_in[], serial_buffer[], serial_buffer_old1[], serial_buffer_old2[];
extern char keyboard_buffer[];
extern WORD ser_buf_pos, key_buf_pos;
extern struct   MsgPort        *SerReadPort;
extern struct   IOExtSer       *SerReadReq, *SerWriteReq;
extern struct ISP ISP;

///

/// add_sl
VOID add_sl(LONG command, STRPTR contents)
{
   struct ScriptLine *sl;

   if(sl = AllocVec(sizeof(struct ScriptLine), MEMF_ANY | MEMF_CLEAR))
   {
      sl->sl_command = command;
      if(contents)
         strncpy(sl->sl_contents, contents, sizeof(sl->sl_contents));
      AddTail((struct List *)&ISP.isp_loginscript, (struct Node *)sl);
   }
}

///
/// do_keyboard
VOID do_keyboard(VOID)
{
   if(keyboard_input)
   {
      add_sl(SL_Send, keyboard_buffer);
      keyboard_input = FALSE;
      keyboard_buffer[0] = NULL;
      key_buf_pos = 0;
   }
}

///
/// do_serial
VOID do_serial(VOID)
{
   add_sl(SL_WaitFor, (*serial_buffer ? serial_buffer : (*serial_buffer_old1 ? serial_buffer_old1 : serial_buffer_old2)));
   serial_buffer[0] = serial_buffer_old1[0] = serial_buffer_old2[0] = NULL;
   ser_buf_pos = 0;
}

///

/// LoginScript_SerialInput
ULONG LoginScript_SerialInput(struct IClass *cl, Object *obj, Msg msg)
{
   struct LoginScript_Data *data = INST_DATA(cl, obj);
   BOOL used = FALSE;

   if(SerReadReq)
   {
      if(CheckIO((struct IORequest *)SerReadReq))
      {
         WaitIO((struct IORequest *)SerReadReq);

         DoMethod(data->TR_Terminal, TCM_WRITE, serial_in, 1);
         if(serial_in[0] == '\r' || serial_in[0] == '\n' || ser_buf_pos > 79)
         {
            if(serial_in[0] == '\r' && ser_buf_pos < 77)
               strcat(serial_buffer, "\\r");

            serial_startread(serial_in, 1);   // start read asap so we don't get hundreds of hits e.g. when doing MUI_Request below !!

            if(ser_buf_pos)
            {
               strcpy(serial_buffer_old2, serial_buffer_old1);
               strcpy(serial_buffer_old1, serial_buffer);

               if(strstr(serial_buffer, "CONNECT") == serial_buffer && !use_loginscript)
               {
                  serial_buffer[0] = NULL;   // to make sure it doesn't get called twice
                  DoMethod(obj, MUIM_LoginScript_GoOnline);
               }
               if(strstr(serial_buffer, "NO DIAL TONE") == serial_buffer || strstr(serial_buffer, "NO DIALTONE") == serial_buffer)
               {
                  serial_buffer[0] = NULL;   // otherwise the req is opened twice
                  MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_NoDialtone));
               }
               if((strstr(serial_buffer, "NO CARRIER") == serial_buffer || strstr(serial_buffer, "BUSY") == serial_buffer) && dialing_try)
               {
                  serial_buffer[0] = NULL;
                  set(app, MUIA_Application_Sleep, TRUE);
                  dial_number++;
                  Delay(80);
                  DoMethod(obj, MUIM_LoginScript_Dial, FALSE);
                  set(app, MUIA_Application_Sleep, FALSE);
               }
               serial_buffer[0] = NULL;
               ser_buf_pos = 0;
            }
         }
         else
         {
            switch((ULONG)serial_in[0])
            {
               case 0:  // ignore NULL !
                  serial_startread(serial_in, 1);
                  break;

               case 8:
                  if(ser_buf_pos > 0)
                     ser_buf_pos--;
                  serial_buffer[ser_buf_pos] = NULL;
                  serial_startread(serial_in, 1);
                  break;

               default:
                  serial_buffer[ser_buf_pos++]  = serial_in[0];
                  serial_buffer[ser_buf_pos]    = NULL;
                  serial_startread(serial_in, 1);

                  if(have_ppp_frame(serial_buffer, ser_buf_pos))
                  {
                     if(data->ihnode_added)
                     {
                        DoMethod(app, MUIM_Application_RemInputHandler, &data->ihnode);
                        data->ihnode_added = FALSE;
                     }
                     DoMethod(obj, MUIM_LoginScript_GoOnline);
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
/// LoginScript_KeyboardInput
ULONG LoginScript_KeyboardInput(struct IClass *cl, Object *obj, Msg msg)
{
   struct LoginScript_Data *data = INST_DATA(cl, obj);
   unsigned char *ptr;
   ULONG len;

   dialing_try = 0;

   get(data->TR_Terminal, TCA_OUTPTR, &ptr);
   get(data->TR_Terminal, TCA_OUTLEN, &len);
   if(len == 3)
   {
      if(ptr[0] == 155 && ptr[2] == '~')
      {
         switch((ULONG)ptr[1])
         {
            case '0':
               DoMethod(obj, MUIM_LoginScript_Dial, TRUE);
               break;
            case '1':
               DoMethod(obj, MUIM_LoginScript_GoOnline);
               break;
            case '2':
               DoMethod(obj, MUIM_LoginScript_HangUp);
               break;
            case '3':
               DoMethod(obj, MUIM_LoginScript_SendLogin);
               break;
            case '4':
               DoMethod(obj, MUIM_LoginScript_SendPassword);
               break;
            case '5':
               DoMethod(obj, MUIM_LoginScript_SendBreak);
               break;
         }
      }
   }
   else
   {
      switch((ULONG)*ptr)
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
            if(key_buf_pos > 0)
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

      serial_send(ptr, len);
   }

   DoMethod(data->TR_Terminal, TCM_OUTFLUSH);

   return(NULL);
}

///

/// LoginScript_Dial
ULONG LoginScript_Dial(struct IClass *cl, Object *obj, struct MUIP_LoginScript_Dial *msg)
{
   struct LoginScript_Data *data = INST_DATA(cl, obj);
   char buffer1[240], buffer2[81];
   STRPTR ptr;
   int i;

   if(msg->restart)
   {
      dialing_try = 1;
      dial_number = 0;
      do_keyboard();
      clear_list(&ISP.isp_loginscript);
      add_sl(SL_Dial, NULL);
   }

   if(!dialing_try)
      return(NULL);

   FOREVER
   {
      i = dial_number;
      ptr = ISP.isp_phonenumber;
      while(i-- > 0 && ptr)
      {
         ptr++;
         ptr = strchr(ptr, '|');
      }
      if(ptr)
      {
         while(*ptr == ' ' || *ptr == '|')
            ptr++;
         strncpy(buffer2, ptr, 80);
         if(ptr = strchr(buffer2, '|'))
            *ptr = NULL;

         break;
      }
      else
      {
         dial_number = 0;
         dialing_try++;
         if(dialing_try > 9)
         {
            MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Abort), GetStr(MSG_TX_CouldNotConnect));
            return(NULL);
         }
      }
   }

   DoMethod(data->TR_Terminal, TCM_INIT);
   Delay(10);
   sprintf(buffer1, GetStr(MSG_TX_Dialing), buffer2, dialing_try);
   DoMethod(data->TR_Terminal, TCM_WRITE, buffer1, strlen(buffer1));

   serial_send("AAT\r", -1);
   Delay(10);
   serial_send("ATZ\r", -1);
   Delay(20);

   EscapeString(buffer1, Config.cnf_initstring);
   strcat(buffer1, "\r");
   serial_send(buffer1, -1);
   Delay(10);

   EscapeString(buffer1, Config.cnf_dialprefix);
   strcat(buffer1, buffer2);
   strcat(buffer1, "\r");
   serial_send(buffer1, -1);

   return(NULL);
}

///
/// LoginScript_GoOnline
ULONG LoginScript_GoOnline(struct IClass *cl, Object *obj, Msg msg)
{
   struct LoginScript_Data *data = INST_DATA(cl, obj);

   if(!serial_carrier())
   {
      if(!(MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_ContinueAbort), GetStr(MSG_TX_WarningNoCarrierOnline))))
         return(NULL);
   }

   if(data->ihnode_added)
   {
      DoMethod(app, MUIM_Application_RemInputHandler, &data->ihnode);
      data->ihnode_added = FALSE;
   }
   serial_stopread();

   dialing_try = 0;
   do_keyboard();
   add_sl(SL_GoOnline, NULL);

   set(app, MUIA_Application_Sleep, TRUE);
   if(status_win = NewObject(CL_Online->mcc_Class, NULL, TAG_DONE))
   {
      DoMethod(app, OM_ADDMEMBER, status_win);
      set(status_win, MUIA_Window_Open, TRUE);
      DoMethod(status_win, MUIM_Online_GoOnline);
   }

   return(NULL);
}

///
/// LoginScript_HangUp
ULONG LoginScript_HangUp(struct IClass *cl, Object *obj, Msg msg)
{
//   struct LoginScript_Data *data = INST_DATA(cl, obj);

   set(obj, MUIA_Window_Sleep, TRUE);

   dialing_try = 0;
   do_keyboard();

   serial_hangup();

   set(obj, MUIA_Window_Sleep, FALSE);

   return(NULL);
}

///
/// LoginScript_SendLogin
ULONG LoginScript_SendLogin(struct IClass *cl, Object *obj, Msg msg)
{
//   struct LoginScript_Data *data = INST_DATA(cl, obj);
   char buffer[101];

   dialing_try = 0;
   do_keyboard();
   do_serial();
   add_sl(SL_SendLogin, NULL);

   sprintf(buffer, "%ls\r", ISP.isp_login);
   serial_send(buffer, -1);

   return(NULL);
}

///
/// LoginScript_SendPassword
ULONG LoginScript_SendPassword(struct IClass *cl, Object *obj, Msg msg)
{
//   struct LoginScript_Data *data = INST_DATA(cl, obj);
   char buffer[101];

   dialing_try = 0;
   do_keyboard();
   do_serial();
   add_sl(SL_SendPassword, NULL);

   sprintf(buffer, "%ls\r", ISP.isp_password);
   serial_send(buffer, -1);

   return(NULL);
}

///
/// LoginScript_SendBreak
ULONG LoginScript_SendBreak(struct IClass *cl, Object *obj, Msg msg)
{
   dialing_try = 0;
   do_keyboard();
   do_serial();
   add_sl(SL_SendBreak, NULL);
   SerWriteReq->IOSer.io_Command = SDCMD_BREAK;
   DoIO((struct IORequest *)SerWriteReq);

   return(NULL);
}

///

/// LoginScript_Dispose
ULONG LoginScript_Dispose(struct IClass *cl, Object *obj, Msg msg)
{
   struct LoginScript_Data *data = INST_DATA(cl, obj);

   if(data->ihnode_added)
   {
      data->ihnode_added = FALSE;
      DoMethod(app, MUIM_Application_RemInputHandler, &data->ihnode);
   }
   serial_stopread();

   return(DoSuperMethodA(cl, obj, msg));
}

///
/// LoginScript_New
ULONG LoginScript_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct LoginScript_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      Child, ListviewObject,
         MUIA_Weight, 70,
         MUIA_Listview_Input  , FALSE,
         MUIA_Listview_List   , FloattextObject,
            MUIA_Frame, MUIV_Frame_ReadList,
            MUIA_Background         , MUII_ReadListBack,
            MUIA_Floattext_Text     , GetStr(MSG_TX_InfoLoginScript),
            MUIA_Floattext_TabSize  , 4,
            MUIA_Floattext_Justify, TRUE,
         End,
      End,
      Child, BalanceObject, End,
      Child, VGroup,
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
      TAG_MORE, msg->ops_AttrList))
   {
      struct LoginScript_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->ihnode_added = FALSE;

      set(data->BT_Dial          , MUIA_ShortHelp, GetStr(MSG_HELP_Dial));
      set(data->BT_GoOnline      , MUIA_ShortHelp, GetStr(MSG_HELP_GoOnline));
      set(data->BT_HangUp        , MUIA_ShortHelp, GetStr(MSG_HELP_HangUp));
      set(data->BT_SendLogin     , MUIA_ShortHelp, GetStr(MSG_HELP_SendLogin));
      set(data->BT_SendPassword  , MUIA_ShortHelp, GetStr(MSG_HELP_SendPassword));
      set(data->BT_SendBreak     , MUIA_ShortHelp, GetStr(MSG_HELP_SendBreak));

      set(data->TR_Terminal, TCA_SCROLLER, data->SB_Terminal);

      DoMethod(data->SB_Terminal    , MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime, data->TR_Terminal, 1, TCM_SCROLLER);
      DoMethod(data->TR_Terminal    , MUIM_Notify, TCA_OUTLEN, MUIV_EveryTime, obj, 1, MUIM_LoginScript_KeyboardInput);
      DoMethod(data->BT_Dial        , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 2, MUIM_LoginScript_Dial, TRUE);
      DoMethod(data->BT_GoOnline    , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_LoginScript_GoOnline);
      DoMethod(data->BT_HangUp      , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_LoginScript_HangUp);
      DoMethod(data->BT_SendLogin   , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_LoginScript_SendLogin);
      DoMethod(data->BT_SendPassword, MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_LoginScript_SendPassword);
      DoMethod(data->BT_SendBreak   , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_LoginScript_SendBreak);
   }

   return((ULONG)obj);
}

///
/// LoginScript_Dispatcher
SAVEDS ASM ULONG LoginScript_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                         : return(LoginScript_New            (cl, obj, (APTR)msg));
      case OM_DISPOSE                     : return(LoginScript_Dispose        (cl, obj, (APTR)msg));
      case MUIM_LoginScript_SerialInput   : return(LoginScript_SerialInput    (cl, obj, (APTR)msg));
      case MUIM_LoginScript_KeyboardInput : return(LoginScript_KeyboardInput  (cl, obj, (APTR)msg));
      case MUIM_LoginScript_Dial          : return(LoginScript_Dial           (cl, obj, (APTR)msg));
      case MUIM_LoginScript_GoOnline      : return(LoginScript_GoOnline       (cl, obj, (APTR)msg));
      case MUIM_LoginScript_HangUp        : return(LoginScript_HangUp         (cl, obj, (APTR)msg));
      case MUIM_LoginScript_SendLogin     : return(LoginScript_SendLogin      (cl, obj, (APTR)msg));
      case MUIM_LoginScript_SendPassword  : return(LoginScript_SendPassword   (cl, obj, (APTR)msg));
      case MUIM_LoginScript_SendBreak     : return(LoginScript_SendBreak      (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

