/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_DataBase.h"
#include "mui_Dialer.h"
#include "mui_MainWindow.h"
#include "mui_Modem.h"
#include "mui_PasswdReq.h"
#include "mui_Provider.h"
#include "mui_User.h"
#include "protos.h"
#include "mui/Grouppager_mcc.h"
#include "images/information.h"
#include "images/databases.h"
#include "images/dialer.h"
#include "images/modem.h"
#include "images/provider.h"
#include "images/user.h"
#include "images/logo.h"

///
/// external variables
extern struct MUI_CustomClass  *CL_Provider;
extern struct MUI_CustomClass  *CL_User;
extern struct MUI_CustomClass  *CL_Dialer;
extern struct MUI_CustomClass  *CL_Modem;
extern struct MUI_CustomClass  *CL_Databases;
extern struct MUI_CustomClass  *CL_About;
extern char config_file[];
extern BOOL changed_passwd, changed_group, changed_hosts, changed_protocols,
            changed_services, changed_inetd, changed_networks, changed_rpc, changed_inetaccess;
extern Object *win;
extern Object *app;
extern struct NewMenu MainWindowMenu[];

extern struct BitMapHeader information_header, provider_header, user_header, modem_header, dialer_header, databases_header, logo_header;
extern ULONG information_colors[], provider_colors[], user_colors[], modem_colors[], dialer_colors[], databases_colors[], logo_colors[];
extern UBYTE information_body[], provider_body[], user_body[], modem_body[], dialer_body[], databases_body[], logo_body[];

///
            
/// create_bodychunk
Object *create_bodychunk(struct BitMapHeader *bmhd, ULONG *cols, UBYTE *body)
{
   return(BodychunkObject,
      MUIA_Background            , MUII_ButtonBack,
      MUIA_Bitmap_SourceColors   , cols,
      MUIA_Bitmap_Width          , bmhd->bmh_Width,
      MUIA_Bitmap_Height         , bmhd->bmh_Height,
      MUIA_FixWidth              , bmhd->bmh_Width ,
      MUIA_FixHeight             , bmhd->bmh_Height,
      MUIA_Bodychunk_Depth       , bmhd->bmh_Depth,
      MUIA_Bodychunk_Body        , body,
      MUIA_Bodychunk_Compression , bmhd->bmh_Compression,
      MUIA_Bodychunk_Masking     , bmhd->bmh_Masking,
      MUIA_Bitmap_Transparent    , 0,
   End);
}

///
/// win_stat
STRPTR win_stat(LONG active)
{
   if(active == 1)
      return("open");
   if(active == 2)
      return("close");
   return("ignore");
}

///

/// MainWindow_LoadConfig
ULONG MainWindow_LoadConfig(struct IClass *cl, Object *obj, struct MUIP_MainWindow_LoadConfig *msg)
{
   struct MainWindow_Data *data              = INST_DATA(cl                      , obj);
   struct Provider_Data    *provider_data    = INST_DATA(CL_Provider->mcc_Class  , data->GR_Provider);
   struct User_Data        *user_data        = INST_DATA(CL_User->mcc_Class      , data->GR_User);
   struct Modem_Data       *modem_data       = INST_DATA(CL_Modem->mcc_Class     , data->GR_Modem);
   struct Dialer_Data      *dialer_data      = INST_DATA(CL_Dialer->mcc_Class    , data->GR_Dialer);
   struct Databases_Data   *db_data          = INST_DATA(CL_Databases->mcc_Class , data->GR_Databases);
   struct pc_Data pc_data;
   char buffer[41];
   STRPTR ptr;
   int ns = 0;

   /**** load provider.conf ****/

   if(ParseConfig((msg->file ? msg->file : (STRPTR)config_file), &pc_data))
   {
      while(ParseNext(&pc_data))
      {
         if(!stricmp(pc_data.Argument, "LoginName"))
            setstring(user_data->STR_LoginName     , pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "Password"))
            setstring(user_data->STR_Password      , pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "RealName"))
            setstring(user_data->STR_RealName      , pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "EMail"))
            setstring(user_data->STR_EMail         , pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "Organisation"))
            setstring(user_data->STR_Organisation  , pc_data.Contents);

         else if(!stricmp(pc_data.Argument, "Phone"))
            setstring(dialer_data->STR_Phone, pc_data.Contents);

         else if(!stricmp(pc_data.Argument, "SerialDevice"))
            setstring(modem_data->PO_SerialDriver, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "SerialUnit"))
            set(modem_data->STR_SerialUnit, MUIA_String_Integer, atol(pc_data.Contents));
         else if(!stricmp(pc_data.Argument, "BaudRate"))
            set(modem_data->PO_BaudRate, MUIA_String_Integer, atol(pc_data.Contents));
         else if(!stricmp(pc_data.Argument, "CarrierDetect"))
            setcheckmark(modem_data->CH_Carrier, !stricmp(pc_data.Contents, "yes"));
         else if(!stricmp(pc_data.Argument, "7Wire"))
            setcheckmark(modem_data->CH_7Wire, !stricmp(pc_data.Contents, "yes"));
         else if(!stricmp(pc_data.Argument, "SerBufLen"))
            set(modem_data->STR_SerBufLen, MUIA_String_Integer, atol(pc_data.Contents));

         else if(!stricmp(pc_data.Argument, "Modem"))
            setstring(modem_data->STR_Modem, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "InitString"))
            setstring(modem_data->STR_ModemInit, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "DialPrefix"))
            setstring(modem_data->STR_DialPrefix, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "DialSuffix"))
            setstring(modem_data->STR_DialSuffix, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "RedialAttempts"))
            setslider(modem_data->SL_RedialAttempts, atol(pc_data.Contents));
         else if(!stricmp(pc_data.Argument, "RedialDelay"))
            setslider(modem_data->SL_RedialDelay, atol(pc_data.Contents));

         else if(!stricmp(pc_data.Argument, "Sana2Device"))
         {
            if(!stricmp(pc_data.Contents, "DEVS:Networks/appp.device"))
               setcycle(provider_data->CY_Sana2Device, 0);
            else if(!stricmp(pc_data.Contents, "DEVS:Networks/aslip.device"))
               setcycle(provider_data->CY_Sana2Device, 1);
            else
               setcycle(provider_data->CY_Sana2Device, 2);

            setstring(provider_data->STR_Sana2Device, pc_data.Contents);
         }
         else if(!stricmp(pc_data.Argument, "Sana2Unit"))
            set(provider_data->STR_Sana2Unit, MUIA_String_Integer, atol(pc_data.Contents));
         else if(!stricmp(pc_data.Argument, "Sana2Config"))
            setstring(provider_data->STR_Sana2ConfigFile, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "Sana2ConfigText"))
         {
            if(strstr(pc_data.Contents, "\\n"))
            {
               STRPTR ptr2;

               setstring(provider_data->TI_Sana2ConfigContents, "");
               ptr = pc_data.Contents;
               FOREVER
               {
                  if(ptr2 = strstr(ptr, "\\n"))
                     *ptr2 = NULL;
                  DoMethod(provider_data->TI_Sana2ConfigContents, MUIM_Textinput_AppendText, ptr, -1);
                  if(ptr2)
                     DoMethod(provider_data->TI_Sana2ConfigContents, MUIM_Textinput_AppendText, "\n", -1);
                  else
                     break;

                  ptr = ptr2 + 2;
               }
            }
            else
               setstring(provider_data->TI_Sana2ConfigContents, pc_data.Contents);
         }

         else if(!stricmp(pc_data.Argument, "Interface"))
            setstring(provider_data->STR_IfaceName, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "IfConfigParams"))
            setstring(provider_data->STR_IfaceConfigParams, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "MTU"))
            set(provider_data->STR_MTU, MUIA_String_Integer, atol(pc_data.Contents));
         else if(!stricmp(pc_data.Argument, "KeepAlive"))
            setcycle(provider_data->CY_Ping, atol(pc_data.Contents));
         else if(!stricmp(pc_data.Argument, "PingInterval"))
            set(provider_data->SL_Ping, MUIA_Numeric_Value, atol(pc_data.Contents));


         else if(!stricmp(pc_data.Argument, "IPAddr"))
         {
            setstring(provider_data->STR_IP_Address, pc_data.Contents);
            setcycle(provider_data->CY_Address, 1);
         }
         else if(!stricmp(pc_data.Argument, "HostName"))
         {
            setstring(provider_data->STR_HostName, pc_data.Contents);
            setcycle(provider_data->CY_Address, 1);
         }
// DestIP
// Gateway
// Netmask
         else if(!stricmp(pc_data.Argument, "DomainName"))
         {
            setstring(provider_data->STR_DomainName, pc_data.Contents);
            setcycle(provider_data->CY_Resolv, 1);
         }
         else if(!stricmp(pc_data.Argument, "NameServer"))
         {
            setstring((ns++ ? provider_data->STR_NameServer2 : provider_data->STR_NameServer1), pc_data.Contents);
            setcycle(provider_data->CY_Resolv, 1);
         }
// BootPServer
         else if(!stricmp(pc_data.Argument, "UseBootP"))
            setcheckmark(provider_data->CH_BOOTP, !stricmp(pc_data.Contents, "yes"));

         else if(!stricmp(pc_data.Argument, "MailServer"))
            setstring(provider_data->STR_MailServer, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "POPServer"))
            setstring(provider_data->STR_POPServer, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "NewsServer"))
            setstring(provider_data->STR_NewsServer, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "IRCServer"))
            setstring(provider_data->STR_IRCServer, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "IRCPort"))
            set(provider_data->STR_IRCPort, MUIA_String_Integer, atol(pc_data.Contents));
         else if(!stricmp(pc_data.Argument, "TimeServer"))
            setstring(provider_data->STR_TimeServer, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "ProxyServer"))
            setstring(provider_data->STR_ProxyServer, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "ProxyPort"))
            set(provider_data->STR_ProxyPort, MUIA_String_Integer, atol(pc_data.Contents));

         else if(!stricmp(pc_data.Argument, "AutoLogin"))
            setcycle(dialer_data->CY_AutoLogin, stricmp(pc_data.Contents, "yes"));
         else if(!stricmp(pc_data.Argument, "OnlineOnStartup"))
            setcheckmark(dialer_data->CH_GoOnline, !stricmp(pc_data.Contents, "yes"));
         else if(!stricmp(pc_data.Argument, "QuickReconnect"))
            setcheckmark(dialer_data->CH_QuickReconnect, !stricmp(pc_data.Contents, "yes"));
         else if(!stricmp(pc_data.Argument, "SynClock"))
            setcheckmark(dialer_data->CH_SynClock, !stricmp(pc_data.Contents, "yes"));
         else if(!stricmp(pc_data.Argument, "ShowStatus"))
            setcheckmark(dialer_data->CH_ShowStatus, !stricmp(pc_data.Contents, "yes"));
         else if(!stricmp(pc_data.Argument, "ShowSpeed"))
            setcheckmark(dialer_data->CH_ShowSpeed, !stricmp(pc_data.Contents, "yes"));
         else if(!stricmp(pc_data.Argument, "ShowOnline"))
            setcheckmark(dialer_data->CH_ShowOnline, !stricmp(pc_data.Contents, "yes"));
         else if(!stricmp(pc_data.Argument, "ShowButtons"))
            setcheckmark(dialer_data->CH_ShowButtons, !stricmp(pc_data.Contents, "yes"));

         else if(!stricmp(pc_data.Argument, "Online"))
            setstring(dialer_data->STR_Online, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "OnlineFail"))
            setstring(dialer_data->STR_OnlineFailed, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "OfflineActive"))
            setstring(dialer_data->STR_OfflineActive, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "OfflinePassive"))
            setstring(dialer_data->STR_OfflinePassive, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "Startup"))
            setstring(dialer_data->STR_Startup, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "Shutdown"))
            setstring(dialer_data->STR_Shutdown, pc_data.Contents);

         else if(!stricmp(pc_data.Argument, "WinOnline"))
            setcycle(dialer_data->CY_WinOnline, (!stricmp(pc_data.Contents, "open") ? 1 : (!stricmp(pc_data.Contents, "close") ? 2 : 0)));
         else if(!stricmp(pc_data.Argument, "WinOnlineFail"))
            setcycle(dialer_data->CY_WinOnlineFailed, (!stricmp(pc_data.Contents, "open") ? 1 : (!stricmp(pc_data.Contents, "close") ? 2 : 0)));
         else if(!stricmp(pc_data.Argument, "WinOfflineActive"))
            setcycle(dialer_data->CY_WinOfflineActive, (!stricmp(pc_data.Contents, "open") ? 1 : (!stricmp(pc_data.Contents, "close") ? 2 : 0)));
         else if(!stricmp(pc_data.Argument, "WinOfflinePassive"))
            setcycle(dialer_data->CY_WinOfflinePassive, (!stricmp(pc_data.Contents, "open") ? 1 : (!stricmp(pc_data.Contents, "close") ? 2 : 0)));
         else if(!stricmp(pc_data.Argument, "WinStartup"))
            setcycle(dialer_data->CY_WinStartup, (!stricmp(pc_data.Contents, "open") ? 1 : (!stricmp(pc_data.Contents, "close") ? 2 : 0)));

         else if(!stricmp(pc_data.Argument, "LoginScript"))
         {
            struct ScriptLine *script_line;

            DoMethod(dialer_data->LI_Script, MUIM_List_Clear);
            if(script_line = AllocVec(sizeof(struct ScriptLine), MEMF_ANY))
            {
               while(ParseNext(&pc_data))
               {
                  if(!stricmp(pc_data.Argument, "EOS"))
                     break;

                  if(!stricmp(pc_data.Argument, script_commands[SL_Send]))
                     script_line->sl_command = SL_Send;
                  else if(!stricmp(pc_data.Argument, script_commands[SL_WaitFor]))
                     script_line->sl_command = SL_WaitFor;
                  else if(!stricmp(pc_data.Argument, script_commands[SL_Dial]))
                     script_line->sl_command = SL_Dial;
                  else if(!stricmp(pc_data.Argument, script_commands[SL_GoOnline]))
                     script_line->sl_command = SL_GoOnline;
                  else if(!stricmp(pc_data.Argument, script_commands[SL_SendLogin]))
                     script_line->sl_command = SL_SendLogin;
                  else if(!stricmp(pc_data.Argument, script_commands[SL_SendPassword]))
                     script_line->sl_command = SL_SendPassword;
                  else if(!stricmp(pc_data.Argument, script_commands[SL_SendBreak]))
                     script_line->sl_command = SL_SendBreak;
                  else if(!stricmp(pc_data.Argument, script_commands[SL_Exec]))
                     script_line->sl_command = SL_Exec;
                  else if(!stricmp(pc_data.Argument, script_commands[SL_Pause]))
                     script_line->sl_command = SL_Pause;
                  else
                     script_line->sl_command = 0;

                  strncpy(script_line->sl_contents, pc_data.Contents, 250);

                  DoMethod(dialer_data->LI_Script, MUIM_NList_InsertSingle, script_line, MUIV_NList_Insert_Bottom);
               }
               FreeVec(script_line);
            }
         }
      }
      ParseEnd(&pc_data);
   }

   /**** load the ModemSettings into the List ****/
   if(ParseConfig("AmiTCP:db/modems", &pc_data))
   {
      struct Modem *modem;
      int protocol_nr = 0;

      if(modem = (struct Modem *)AllocVec(sizeof(struct Modem), MEMF_ANY))
      {
         set(modem_data->LV_Modems, MUIA_List_Quiet, TRUE);
         while(ParseNext(&pc_data))
         {
            if(!stricmp(pc_data.Argument, "Modem"))
            {
               if(*modem->Name)
               {
                  modem->ProtocolName[protocol_nr][0] = NULL;
                  DoMethod(modem_data->LV_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Sorted);
                  bzero(modem, sizeof(struct Modem));
               }
               strncpy(modem->Name, pc_data.Contents, 80);
               protocol_nr = 0;
            }
            if(!stricmp(pc_data.Argument, "Protocol"))
               strncpy(modem->ProtocolName[protocol_nr], pc_data.Contents, 20);

            if(!stricmp(pc_data.Argument, "InitString"))
            {
               strncpy(modem->InitString[protocol_nr], pc_data.Contents, 20);
               if(!*modem->ProtocolName[protocol_nr])
                  strcpy(modem->ProtocolName[protocol_nr], "Default");
               if(protocol_nr < 9)
                  protocol_nr++;
            }
         }

         if(*modem->Name)
         {
            modem->ProtocolName[protocol_nr][0] = NULL;
            DoMethod(modem_data->LV_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Sorted);
         }
         strcpy(modem->Name, "Generic");
         strcpy(modem->ProtocolName[0], "Default");
         strcpy(modem->InitString[0], "AT&F&D2");
         modem->ProtocolName[1][0] = NULL;
         DoMethod(modem_data->LV_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Top);

         FreeVec(modem);
         set(modem_data->LV_Modems, MUIA_List_Quiet, FALSE);
      }
      ParseEnd(&pc_data);
   }
   DoMethod(data->GR_Modem, MUIM_Modem_UpdateProtocolList);


   /** put devices into device list of modem page **/

   set(modem_data->LV_Devices, MUIA_List_Quiet, TRUE);
   {
      BPTR lock;
      struct FileInfoBlock *fib;

      if(lock = Lock("DEVS:", ACCESS_READ))
      {
         if(fib = AllocDosObject(DOS_FIB, NULL))
         {
            if(Examine(lock, fib))
            {
               while(ExNext(lock, fib))
               {
                  if((fib->fib_DirEntryType < 0) && strstr(fib->fib_FileName, ".device"))
                  {
                     if(stricmp(fib->fib_FileName, "printer.device") &&
                        stricmp(fib->fib_FileName, "mfm.device") &&
                        stricmp(fib->fib_FileName, "clipboard.device") &&
                        stricmp(fib->fib_FileName, "parallel.device"))
                     DoMethod(modem_data->LV_Devices, MUIM_List_InsertSingle, fib->fib_FileName, MUIV_List_Insert_Sorted);
                  }
               }
            }
            FreeDosObject(DOS_FIB, fib);
         }
         UnLock(lock);
      }
   }
   set(modem_data->LV_Devices, MUIA_List_Quiet, FALSE);


   /**** parse the passwd file ****/

   set(db_data->LV_Users, MUIA_List_Quiet, TRUE);
   if(ParseConfig("AmiTCP:db/passwd", &pc_data))
   {
      struct User *user;

      if(user = AllocVec(sizeof(struct User), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.Contents, '|'))
            {
               *ptr = NULL;
               strncpy(user->Login, pc_data.Contents, 40);
               pc_data.Contents = ptr + 1;
               if(ptr = strchr(pc_data.Contents, '|'))
               {
                  *ptr = NULL;
                  strncpy(user->Password, pc_data.Contents, 20);
                  pc_data.Contents = ptr + 1;
                  if(ptr = strchr(pc_data.Contents, '|'))
                  {
                     *ptr = NULL;
                     user->UserID = atol(pc_data.Contents);
                     pc_data.Contents = ptr + 1;
                     if(ptr = strchr(pc_data.Contents, '|'))
                     {
                        *ptr = NULL;
                        user->GroupID = atol(pc_data.Contents);
                        pc_data.Contents = ptr + 1;
                        if(ptr = strchr(pc_data.Contents, '|'))
                        {
                           *ptr = NULL;
                           strncpy(user->Name, pc_data.Contents, 80);
                           pc_data.Contents = ptr + 1;
                           if(ptr = strchr(pc_data.Contents, '|'))
                           {
                              *ptr = NULL;
                              strncpy(user->HomeDir, pc_data.Contents, MAXPATHLEN - 1);
                              strncpy(user->Shell, ptr + 1, 80);
                              if(!strcmp(user->Password, "*"))
                              {
                                 user->Disabled = TRUE;
                                 *user->Password = NULL;
                              }
                              else
                                 user->Disabled = FALSE;
                              DoMethod(db_data->LV_Users, MUIM_List_InsertSingle, user, MUIV_List_Insert_Bottom);
                           }
                        }
                     }
                  }
               }
            }
         }
         FreeVec(user);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LV_Users, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Users);

   /**** parse the group file ****/

   set(db_data->LV_Groups, MUIA_List_Quiet, TRUE);
   if(ParseConfig("AmiTCP:db/group", &pc_data))
   {
      struct Group *group;

      if(group = AllocVec(sizeof(struct Group), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.Contents, '|'))
            {
               *ptr = NULL;
               strncpy(group->Name, pc_data.Contents, 40);
               pc_data.Contents = ptr + 1;
               if(ptr = strchr(pc_data.Contents, '|'))
               {
                  *ptr = NULL;
                  strncpy(group->Password, pc_data.Contents, 20);
                  pc_data.Contents = ptr + 1;
                  if(ptr = strchr(pc_data.Contents, '|'))
                  {
                     *ptr = NULL;
                     group->ID = atol(pc_data.Contents);
                     strncpy(group->Members, ptr + 1, 400);
                     DoMethod(db_data->LV_Groups, MUIM_List_InsertSingle, group, MUIV_List_Insert_Bottom);
                  }
               }
            }
         }
         FreeVec(group);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LV_Groups, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Groups);

   /**** parse the protocols file ****/

   set(db_data->LV_Protocols, MUIA_List_Quiet, TRUE);
   if(ParseConfig("AmiTCP:db/protocols", &pc_data))
   {
      struct Protocol *protocol;

      if(protocol = AllocVec(sizeof(struct Protocol), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.Contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.Contents, ';'))
               *ptr = NULL;

            if(pc_data.Contents = extract_arg(pc_data.Contents, protocol->Name, 40, NULL))
            {
               pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, NULL);
               protocol->ID = atol(buffer);

               if(pc_data.Contents)
                  strncpy(protocol->Aliases, pc_data.Contents, 80);
               else
                  *protocol->Aliases = NULL;

               DoMethod(db_data->LV_Protocols, MUIM_List_InsertSingle, protocol, MUIV_List_Insert_Bottom);
            }
         }
         FreeVec(protocol);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LV_Protocols, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Protocols);


   /**** parse the services file ****/

   set(db_data->LV_Services, MUIA_List_Quiet, TRUE);
   if(ParseConfig("AmiTCP:db/services", &pc_data))
   {
      struct Service *service;

      if(service = AllocVec(sizeof(struct Service), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.Contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.Contents, ';'))
               *ptr = NULL;

            if(pc_data.Contents = extract_arg(pc_data.Contents, service->Name, 40, NULL))
            {
               if(pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, '/'))
               {
                  service->Port = atol(buffer);

                  if(pc_data.Contents = extract_arg(pc_data.Contents, service->Protocol, 40, NULL))
                     strncpy(service->Aliases, pc_data.Contents, 80);
                  else
                     *service->Aliases = NULL;

                  DoMethod(db_data->LV_Services, MUIM_List_InsertSingle, service, MUIV_List_Insert_Bottom);
               }
            }
         }
         FreeVec(service);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LV_Services, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Services);


   /**** parse the inet.access file ****/

   set(db_data->LV_InetAccess, MUIA_List_Quiet, TRUE);
   if(ParseConfig("AmiTCP:db/inet.access", &pc_data))
   {
      struct InetAccess *inet_access;

      if(inet_access = AllocVec(sizeof(struct InetAccess), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.Contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.Contents, ';'))
               *ptr = NULL;

            if(pc_data.Contents = extract_arg(pc_data.Contents, inet_access->Service, 40, NULL))
            {
               if(pc_data.Contents = extract_arg(pc_data.Contents, inet_access->Host, 80, NULL))
               {
                  if(pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, NULL))
                  {
                     inet_access->Access = (stricmp(buffer, "allow") ? 0 : 1);
                     inet_access->Log = (stricmp(pc_data.Contents, "LOG") ? 0 : 1);
                  }
                  else
                  {
                     inet_access->Access = (stricmp(pc_data.Contents, "allow") ? 0 : 1);
                     inet_access->Log = FALSE;
                  }
                  DoMethod(db_data->LV_InetAccess, MUIM_List_InsertSingle, inet_access, MUIV_List_Insert_Bottom);
               }
            }
         }
         FreeVec(inet_access);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LV_InetAccess, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_InetAccess);


   /**** parse the inetd.conf file ****/

   set(db_data->LV_Inetd, MUIA_List_Quiet, TRUE);
   if(ParseConfig("AmiTCP:db/inetd.conf", &pc_data))
   {
      struct Inetd *inetd;

      if(inetd = AllocVec(sizeof(struct Inetd), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(*pc_data.Contents == '#')
            {
               inetd->Active = FALSE;
               pc_data.Contents++;
               if(*pc_data.Contents == ' ' || *pc_data.Contents == 9)
                  continue;
            }
            else
               inetd->Active = TRUE;

            if(ptr = strchr(pc_data.Contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.Contents, ';'))
               *ptr = NULL;

            if(pc_data.Contents = extract_arg(pc_data.Contents, inetd->Service, 40, NULL))
            {
               if(pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, NULL))
               {
                  inetd->Socket = !stricmp(buffer, "dgram");
                  if(pc_data.Contents = extract_arg(pc_data.Contents, inetd->Protocol, 40, NULL))
                  {
                     if(pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, NULL))
                     {
                        inetd->Wait = (stricmp(buffer, "wait") ? (stricmp(buffer, "dos") ? 0 : 2) : 1);
                        if(pc_data.Contents = extract_arg(pc_data.Contents, inetd->User, 40, NULL))
                        {
                           if(pc_data.Contents = extract_arg(pc_data.Contents, inetd->Server, 80, NULL))
                              strncpy(inetd->Args, pc_data.Contents, 80);
                           else
                              *inetd->Args = NULL;

                           DoMethod(db_data->LV_Inetd, MUIM_List_InsertSingle, inetd, MUIV_List_Insert_Bottom);
                        }
                     }
                  }
               }
            }
         }
         FreeVec(inetd);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LV_Inetd, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Inetd);


   /**** parse the hosts file ****/

   set(db_data->LV_Hosts, MUIA_List_Quiet, TRUE);
   if(ParseConfig("AmiTCP:db/Hosts", &pc_data))
   {
      struct Host *host;

      if(host = AllocVec(sizeof(struct Host), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.Contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.Contents, ';'))
               *ptr = NULL;

            if(pc_data.Contents = extract_arg(pc_data.Contents, host->Addr, 20, NULL))
            {
               if(pc_data.Contents = extract_arg(pc_data.Contents, host->Name, 40, NULL))
                  strncpy(host->Aliases, pc_data.Contents, 80);
               else
                  *host->Aliases = NULL;

               DoMethod(db_data->LV_Hosts, MUIM_List_InsertSingle, host, MUIV_List_Insert_Bottom);
            }
         }
         FreeVec(host);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LV_Hosts, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Hosts);


   /**** parse the networks file ****/

   set(db_data->LV_Networks, MUIA_List_Quiet, TRUE);
   if(ParseConfig("AmiTCP:db/Networks", &pc_data))
   {
      struct Network *network;

      if(network = AllocVec(sizeof(struct Network), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.Contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.Contents, ';'))
               *ptr = NULL;

            if(pc_data.Contents = extract_arg(pc_data.Contents, network->Name, 40, NULL))
            {
               pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, NULL);
               network->Number = atol(buffer);

               if(pc_data.Contents)
                  strncpy(network->Aliases, pc_data.Contents, 80);
               else
                  *network->Aliases = NULL;

               DoMethod(db_data->LV_Networks, MUIM_List_InsertSingle, network, MUIV_List_Insert_Bottom);
            }
         }
         FreeVec(network);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LV_Networks, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Networks);


   /**** parse the rpc file ****/

   set(db_data->LV_Rpcs, MUIA_List_Quiet, TRUE);
   if(ParseConfig("AmiTCP:db/rpc", &pc_data))
   {
      struct Rpc *rpc;

      if(rpc = AllocVec(sizeof(struct Rpc), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.Contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.Contents, ';'))
               *ptr = NULL;

            if(pc_data.Contents = extract_arg(pc_data.Contents, rpc->Name, 40, NULL))
            {
               pc_data.Contents = extract_arg(pc_data.Contents, buffer, 40, NULL);
               rpc->Number = atol(buffer);

               if(pc_data.Contents)
                  strncpy(rpc->Aliases, pc_data.Contents, 80);
               else
                  *rpc->Aliases = NULL;

               DoMethod(db_data->LV_Rpcs, MUIM_List_InsertSingle, rpc, MUIV_List_Insert_Bottom);
            }
         }
         FreeVec(rpc);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LV_Rpcs, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Rpcs);

   return(NULL);
}

///
/// MainWindow_SaveConfig
ULONG MainWindow_SaveConfig(struct IClass *cl, Object *obj, struct MUIP_MainWindow_SaveConfig *msg)
{
   struct MainWindow_Data *data              = INST_DATA(cl                      , obj);
   struct Provider_Data    *provider_data    = INST_DATA(CL_Provider->mcc_Class  , data->GR_Provider);
   struct User_Data        *user_data        = INST_DATA(CL_User->mcc_Class      , data->GR_User);
   struct Modem_Data       *modem_data       = INST_DATA(CL_Modem->mcc_Class     , data->GR_Modem);
   struct Dialer_Data      *dialer_data      = INST_DATA(CL_Dialer->mcc_Class    , data->GR_Dialer);
   struct Databases_Data   *db_data          = INST_DATA(CL_Databases->mcc_Class , data->GR_Databases);
   struct ScriptLine *script_line;
   BPTR fh;
   STRPTR ptr;
   LONG pos;

   if(fh = CreateDir("AmiTCP:config"))
      UnLock(fh);

   if(fh = Open((msg->file ? msg->file : (STRPTR)config_file), MODE_NEWFILE))
   {
         FPrintf(fh, "LoginName       %ls\n", xget(user_data->STR_LoginName    , MUIA_String_Contents));
         FPrintf(fh, "Password        %ls\n", xget(user_data->STR_Password     , MUIA_String_Contents));
         FPrintf(fh, "RealName        %ls\n", xget(user_data->STR_RealName     , MUIA_String_Contents));
         FPrintf(fh, "EMail           %ls\n", xget(user_data->STR_EMail        , MUIA_String_Contents));
         FPrintf(fh, "Organisation    %ls\n\n", xget(user_data->STR_Organisation , MUIA_String_Contents));

         FPrintf(fh, "Phone           %ls\n\n", xget(dialer_data->STR_Phone      , MUIA_String_Contents));

         FPrintf(fh, "SerialDevice    %ls\n", xget(modem_data->STR_SerialDriver, MUIA_String_Contents));
         FPrintf(fh, "SerialUnit      %ls\n", xget(modem_data->STR_SerialUnit  , MUIA_String_Contents));
         FPrintf(fh, "BaudRate        %ls\n", xget(modem_data->STR_BaudRate    , MUIA_String_Contents));
         FPrintf(fh, "CarrierDetect   %ls\n", (xget(modem_data->CH_Carrier     , MUIA_Selected) ? "yes" : "no"));
         FPrintf(fh, "7Wire           %ls\n", (xget(modem_data->CH_7Wire       , MUIA_Selected) ? "yes" : "no"));
         FPrintf(fh, "SerBufLen       %ls\n\n", xget(modem_data->STR_SerBufLen   , MUIA_String_Contents));

         FPrintf(fh, "Modem           %ls\n", xget(modem_data->STR_Modem       , MUIA_String_Contents));
         FPrintf(fh, "InitString      %ls\n", xget(modem_data->STR_ModemInit   , MUIA_String_Contents));
         FPrintf(fh, "DialPrefix      %ls\n", xget(modem_data->STR_DialPrefix  , MUIA_String_Contents));
         FPrintf(fh, "DialSuffix      %ls\n", xget(modem_data->STR_DialSuffix  , MUIA_String_Contents));
         FPrintf(fh, "RedialAttempts  %ld\n", xget(modem_data->SL_RedialAttempts, MUIA_Numeric_Value));
         FPrintf(fh, "RedialDelay     %ld\n\n", xget(modem_data->SL_RedialDelay  , MUIA_Numeric_Value));

         ptr = NULL;
         switch(xget(provider_data->CY_Sana2Device, MUIA_Cycle_Active))
         {
            case 0:
               FPrintf(fh, "Sana2Device     DEVS:Networks/appp.device\nSana2Unit       0\n");
               FPrintf(fh, "Sana2Config     ENV:Sana2/appp0.config\n");
               ptr = get_configcontents(TRUE);
               break;
            case 1:
               FPrintf(fh, "Sana2Device     DEVS:Networks/aslip.device\nSana2Unit       0\n");
               FPrintf(fh, "Sana2Config     ENV:Sana2/aslip0.config\n");
               ptr = get_configcontents(FALSE);
               break;
            default:
               FPrintf(fh, "Sana2Device     %ls\n", xget(provider_data->STR_Sana2Device        , MUIA_String_Contents));
               FPrintf(fh, "Sana2Unit       %ls\n", xget(provider_data->STR_Sana2Unit          , MUIA_String_Contents));
               FPrintf(fh, "Sana2Config     %ls\n", xget(provider_data->STR_Sana2ConfigFile    , MUIA_String_Contents));
               ptr = (STRPTR)xget(provider_data->TI_Sana2ConfigContents , MUIA_Textinput_Contents);
               break;
         }
         if(ptr)
         {
            if(strchr(ptr, '\n'))
            {
               STRPTR buf, ptr2;

               FPrintf(fh, "Sana2ConfigText ");
               if(buf = AllocVec(strlen(ptr) + 1, MEMF_ANY))
               {
                  memcpy(buf, ptr, strlen(ptr) + 1);

                  ptr = buf;
                  FOREVER
                  {
                     if(ptr2 = strchr(ptr, '\n'))
                        *ptr2 = NULL;
                     FPrintf(fh, "%ls", ptr);
                     if(ptr2)
                        FPrintf(fh, "\\n");
                     else
                        break;
                     ptr = ptr2 + 1;
                  }
                  FreeVec(buf);
               }
               FPrintf(fh, "\n");
            }
            else
               FPrintf(fh, "Sana2ConfigText %ls\n", ptr);
         }
         FPrintf(fh, "Interface       %ls\n", xget(provider_data->STR_IfaceName          , MUIA_String_Contents));
         if(strlen((STRPTR)xget(provider_data->STR_IfaceConfigParams  , MUIA_String_Contents)))
            FPrintf(fh, "IfConfigParams  %ls\n", xget(provider_data->STR_IfaceConfigParams  , MUIA_String_Contents));
         FPrintf(fh, "MTU             %ls\n", xget(provider_data->STR_MTU      , MUIA_String_Contents));
         if(xget(provider_data->CY_Ping, MUIA_Cycle_Active))
         {
            FPrintf(fh, "KeepAlive       %ld\n", xget(provider_data->CY_Ping      , MUIA_Cycle_Active));
            FPrintf(fh, "PingInterval    %ld\n", xget(provider_data->SL_Ping      , MUIA_Numeric_Value));
         }
         FPrintf(fh, "\n");

         if(xget(provider_data->CY_Address, MUIA_Cycle_Active))
         {
            FPrintf(fh, "IPAddr          %ls\n", xget(provider_data->STR_IP_Address , MUIA_String_Contents));
            FPrintf(fh, "HostName        %ls\n", xget(provider_data->STR_HostName   , MUIA_String_Contents));
         }
// DestIP
// Gateway
// Netmask
         if(xget(provider_data->CY_Resolv, MUIA_Cycle_Active))
         {
            FPrintf(fh, "DomainName      %ls\n", xget(provider_data->STR_DomainName , MUIA_String_Contents));
            FPrintf(fh, "NameServer      %ls\n", xget(provider_data->STR_NameServer1, MUIA_String_Contents));
            FPrintf(fh, "NameServer      %ls\n", xget(provider_data->STR_NameServer2, MUIA_String_Contents));
         }
// BootPServer
         FPrintf(fh, "UseBootP        %ls\n\n", (xget(provider_data->CH_BOOTP      , MUIA_Selected) ? "yes" : "no"));

         FPrintf(fh, "MailServer      %ls\n", xget(provider_data->STR_MailServer , MUIA_String_Contents));
         FPrintf(fh, "POPServer       %ls\n", xget(provider_data->STR_POPServer  , MUIA_String_Contents));
         FPrintf(fh, "NewsServer      %ls\n", xget(provider_data->STR_NewsServer , MUIA_String_Contents));
         FPrintf(fh, "IRCServer       %ls\n", xget(provider_data->STR_IRCServer  , MUIA_String_Contents));
         FPrintf(fh, "IRCPort         %ls\n", xget(provider_data->STR_IRCPort    , MUIA_String_Contents));
         FPrintf(fh, "TimeServer      %ls\n", xget(provider_data->STR_TimeServer , MUIA_String_Contents));
         FPrintf(fh, "ProxyServer     %ls\n", xget(provider_data->STR_ProxyServer, MUIA_String_Contents));
         FPrintf(fh, "ProxyPort       %ls\n\n", xget(provider_data->STR_ProxyPort  , MUIA_String_Contents));

         FPrintf(fh, "AutoLogin       %ls\n", (xget(dialer_data->CY_AutoLogin     , MUIA_Cycle_Active) ? "no" : "yes"));
         FPrintf(fh, "OnlineOnStartup %ls\n", (xget(dialer_data->CH_GoOnline      , MUIA_Selected) ? "yes" : "no"));
         FPrintf(fh, "QuickReconnect  %ls\n", (xget(dialer_data->CH_QuickReconnect, MUIA_Selected) ? "yes" : "no"));
         FPrintf(fh, "SynClock        %ls\n", (xget(dialer_data->CH_SynClock      , MUIA_Selected) ? "yes" : "no"));
         FPrintf(fh, "ShowStatus      %ls\n", (xget(dialer_data->CH_ShowStatus    , MUIA_Selected) ? "yes" : "no"));
         FPrintf(fh, "ShowSpeed       %ls\n", (xget(dialer_data->CH_ShowSpeed     , MUIA_Selected) ? "yes" : "no"));
         FPrintf(fh, "ShowOnline      %ls\n", (xget(dialer_data->CH_ShowOnline    , MUIA_Selected) ? "yes" : "no"));
         FPrintf(fh, "ShowButtons     %ls\n\n", (xget(dialer_data->CH_ShowButtons   , MUIA_Selected) ? "yes" : "no"));

         FPrintf(fh, "Online          %ls\n", xget(dialer_data->STR_Online        , MUIA_String_Contents));
         FPrintf(fh, "OnlineFail      %ls\n", xget(dialer_data->STR_OnlineFailed  , MUIA_String_Contents));
         FPrintf(fh, "OfflineActive   %ls\n", xget(dialer_data->STR_OfflineActive , MUIA_String_Contents));
         FPrintf(fh, "OfflinePassive  %ls\n", xget(dialer_data->STR_OfflinePassive, MUIA_String_Contents));
         FPrintf(fh, "Startup         %ls\n", xget(dialer_data->STR_Startup       , MUIA_String_Contents));
         FPrintf(fh, "Shutdown        %ls\n\n", xget(dialer_data->STR_Shutdown      , MUIA_String_Contents));

         FPrintf(fh, "WinOnline         %ls\n", win_stat(xget(dialer_data->CY_WinOnline        , MUIA_Cycle_Active)));
         FPrintf(fh, "WinOnlineFail     %ls\n", win_stat(xget(dialer_data->CY_WinOnlineFailed  , MUIA_Cycle_Active)));
         FPrintf(fh, "WinOfflineActive  %ls\n", win_stat(xget(dialer_data->CY_WinOfflineActive , MUIA_Cycle_Active)));
         FPrintf(fh, "WinOfflinePassive %ls\n", win_stat(xget(dialer_data->CY_WinOfflinePassive, MUIA_Cycle_Active)));
         FPrintf(fh, "WinStartup        %ls\n\n", win_stat(xget(dialer_data->CY_WinStartup       , MUIA_Cycle_Active)));

         FPrintf(fh, "LoginScript\n");
         pos = 0;
         FOREVER
         {
            DoMethod(dialer_data->LV_Script, MUIM_List_GetEntry, pos++, &script_line);
            if(!script_line)
               break;
            FPrintf(fh, "%ls %ls\n", script_commands[script_line->sl_command], script_line->sl_contents);
         }
         FPrintf(fh, "EOS\n\n");

      Close(fh);
   }

   if(changed_passwd)
   {
      if(fh = Open("AmiTCP:db/passwd", MODE_NEWFILE))
      {
         struct User *user;

         pos = 0;
         FOREVER
         {
            DoMethod(db_data->LI_Users, MUIM_List_GetEntry, pos++, &user);
            if(!user)
               break;
            FPrintf(fh, "%ls|%ls|%ld|%ld|%ls|%ls|%ls\n", user->Login, (user->Disabled ? "*" : user->Password), user->UserID, user->GroupID, user->Name, user->HomeDir, user->Shell);
         }
         Close(fh);
      }
   }

   if(changed_group)
   {
      if(fh = Open("AmiTCP:db/group", MODE_NEWFILE))
      {
         struct Group *group;

         pos = 0;
         FOREVER
         {
            DoMethod(db_data->LI_Groups, MUIM_List_GetEntry, pos++, &group);
            if(!group)
               break;
            FPrintf(fh, "%ls|%ls|%ld|%ls\n", group->Name, /*group->Password*/ "*", group->ID, group->Members);
         }
         Close(fh);
      }
   }

   if(changed_hosts)
   {
      if(fh = Open("AmiTCP:db/hosts", MODE_NEWFILE))
      {
         struct Host *host;

         FPrintf(fh, "## This file was generated by Genesis Prefs\n##\n");
         pos = 0;
         FOREVER
         {
            DoMethod(db_data->LI_Hosts, MUIM_List_GetEntry, pos++, &host);
            if(!host)
               break;
            FPrintf(fh, "%-18ls %ls", host->Addr, host->Name);
            if(*host->Aliases)
               FPrintf(fh, "  %ls", host->Aliases);
            FPrintf(fh, "\n");
         }
         Close(fh);
      }
   }

   if(changed_protocols)
   {
      if(fh = Open("AmiTCP:db/protocols", MODE_NEWFILE))
      {
         struct Protocol *protocol;

         FPrintf(fh, "## This file was generated by Genesis Prefs\n##\n");
         pos = 0;
         FOREVER
         {
            DoMethod(db_data->LI_Protocols, MUIM_List_GetEntry, pos++, &protocol);
            if(!protocol)
               break;
            FPrintf(fh, "%-10ls %ld", protocol->Name, protocol->ID);
            if(*protocol->Aliases)
               FPrintf(fh, "  %ls", protocol->Aliases);
            FPrintf(fh, "\n");
         }
         Close(fh);
      }
   }

   if(changed_services)
   {
      if(fh = Open("AmiTCP:db/services", MODE_NEWFILE))
      {
         struct Service *service;

         FPrintf(fh, "## This file was generated by Genesis Prefs\n##\n");
         pos = 0;
         FOREVER
         {
            DoMethod(db_data->LI_Services, MUIM_List_GetEntry, pos++, &service);
            if(!service)
               break;
            FPrintf(fh, "%-15ls %4ld/%-5ls", service->Name, service->Port, service->Protocol);
            if(*service->Aliases)
               FPrintf(fh, "  %ls", service->Aliases);
            FPrintf(fh, "\n");
         }
         Close(fh);
      }
   }

   if(changed_inetaccess)
   {
      if(fh = Open("AmiTCP:db/inet.access", MODE_NEWFILE))
      {
         struct InetAccess *inet_access;

         FPrintf(fh, "## This file was generated by Genesis Prefs\n##\n");
         pos = 0;
         FOREVER
         {
            DoMethod(db_data->LI_InetAccess, MUIM_List_GetEntry, pos++, &inet_access);
            if(!inet_access)
               break;
            FPrintf(fh, "%-15ls %-20ls %-5ls", inet_access->Service, inet_access->Host, (inet_access->Access ? "allow" : "deny"));
            if(inet_access->Log)
               FPrintf(fh, " LOG");
            FPrintf(fh, "\n");
         }
         Close(fh);
      }
   }

   if(changed_inetd)
   {
      if(fh = Open("AmiTCP:db/inetd.conf", MODE_NEWFILE))
      {
         struct Inetd *inetd;

         FPrintf(fh, "## This file was generated by Genesis Prefs\n##\n");
         pos = 0;
         FOREVER
         {
            DoMethod(db_data->LI_Inetd, MUIM_List_GetEntry, pos++, &inetd);
            if(!inetd)
               break;
            if(!inetd->Active)
               FPrintf(fh, "#");
            FPrintf(fh, "%-15ls %-6ls %-5ls %-6ls %-8ls %ls", inetd->Service, (inetd->Socket ? "dgram" : "stream"), inetd->Protocol, (inetd->Wait ? (inetd->Wait == 2 ? "dos" : "wait") : "nowait"), inetd->User, inetd->Server);
            if(*inetd->Args)
               FPrintf(fh, "  %ls", inetd->Args);
            FPrintf(fh, "\n");
         }
         Close(fh);
      }
   }

   if(changed_networks)
   {
      if(fh = Open("AmiTCP:db/networks", MODE_NEWFILE))
      {
         struct Network *network;

         FPrintf(fh, "## This file was generated by Genesis Prefs\n##\n");
         pos = 0;
         FOREVER
         {
            DoMethod(db_data->LI_Networks, MUIM_List_GetEntry, pos++, &network);
            if(!network)
               break;
            FPrintf(fh, "%ls     %ld", network->Name, network->Number);
            if(*network->Aliases)
               FPrintf(fh, "  %ls", network->Aliases);
            FPrintf(fh, "\n");
         }
         Close(fh);
      }
   }

   if(changed_rpc)
   {
      if(fh = Open("AmiTCP:db/rpc", MODE_NEWFILE))
      {
         struct Rpc *rpc;

         FPrintf(fh, "## This file was generated by Genesis Prefs\n##\n");
         pos = 0;
         FOREVER
         {
            DoMethod(db_data->LI_Rpcs, MUIM_List_GetEntry, pos++, &rpc);
            if(!rpc)
               break;
            FPrintf(fh, "%-15ls %10ld", rpc->Name, rpc->Number);
            if(*rpc->Aliases)
               FPrintf(fh, "  %ls", rpc->Aliases);
            FPrintf(fh, "\n");
         }
         Close(fh);
      }
   }

   return(NULL);
}

///
/// MainWindow_Finish
ULONG MainWindow_Finish(struct IClass *cl, Object *obj, struct MUIP_MainWindow_Finish *msg)
{
//   struct MainWindow_Data *data = INST_DATA(cl, obj);

   if(msg->level)
      DoMethod(obj, MUIM_MainWindow_SaveConfig, config_file);

   DoMethod((Object *)xget(obj, MUIA_ApplicationObject), MUIM_Application_PushMethod, (Object *)xget(obj, MUIA_ApplicationObject), 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

   return(NULL);
}

///
/// MainWindow_About
ULONG MainWindow_About(struct IClass *cl, Object *obj, Msg msg)
{
   Object *req;

   set(win, MUIA_Window_Sleep, TRUE);
   if(req = (APTR)NewObject(CL_About->mcc_Class, NULL, TAG_DONE))
   {
      DoMethod(app, OM_ADDMEMBER, req);
      set(req, MUIA_Window_Open, TRUE);
   }
   else
      set(win, MUIA_Window_Sleep, FALSE);

   return(NULL);
}

///
/// MainWindow_AboutFinish
ULONG MainWindow_AboutFinish(struct IClass *cl, Object *obj, struct MUIP_MainWindow_AboutFinish *msg)
{
   Object *window = msg->window;

   set(window, MUIA_Window_Open, FALSE);
   set(win, MUIA_Window_Sleep, FALSE);
   DoMethod(app, OM_REMMEMBER, window);
   MUI_DisposeObject(window);

   return(NULL);
}

///

#ifdef DO_LISTTREE
/// MainWindow_InitGroups (listtree)
ULONG MainWindow_InitGroups(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   BOOL success = FALSE;

   data->GR_Provider       = data->GR_User      =
   data->GR_Dialer         = data->GR_Modem     =
   data->GR_Databases      = NULL;

   if(data->GR_Info)
   {
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Info"), data->GR_Info, NULL, NULL);
   }
   if(data->GR_User        = NewObject(CL_User->mcc_Class      , NULL, TAG_DONE))
   {
      struct User_Data *u_data = INST_DATA(CL_User->mcc_Class, data->GR_User);

      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr(MSG_Pages3), data->GR_User, NULL, NULL);
   }
   if(data->GR_Provider    = NewObject(CL_Provider->mcc_Class  , NULL, TAG_DONE))
   {
      struct Provider_Data *p_data = INST_DATA(CL_Provider->mcc_Class, data->GR_Provider);

      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  TCP/IP"), data->GR_Provider, NULL, GPE_LIST | GPE_OPEN);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Sana II"), p_data->GR_Sana2          , data->GR_Provider, NULL);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr(MSG_ProviderRegister3), p_data->GR_Interface      , data->GR_Provider, NULL);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr(MSG_ProviderRegister4), p_data->GR_Server         , data->GR_Provider, NULL);
   }
   if(data->GR_Modem       = NewObject(CL_Modem->mcc_Class     , NULL, TAG_DONE))
   {
      struct Modem_Data *m_data = INST_DATA(CL_Modem->mcc_Class, data->GR_Modem);

      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Modem / TA"), m_data->GR_Modem , NULL, GPE_LIST);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr(MSG_ModemRegister2), m_data->GR_Serial , data->GR_Modem, NULL);
   }
   if(data->GR_Dialer       = NewObject(CL_Dialer->mcc_Class     , NULL, TAG_DONE))
   {
      struct Dialer_Data *d_data = INST_DATA(CL_Dialer->mcc_Class, data->GR_Dialer);

      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Dialer"), data->GR_Dialer, NULL, GPE_LIST);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Events"), d_data->GR_Events , data->GR_Dialer, NULL);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Misc")  , d_data->GR_Misc , data->GR_Dialer, NULL);
   }
   if(data->GR_Databases   = NewObject(CL_Databases->mcc_Class , NULL, TAG_DONE))
   {
      struct Databases_Data *d_data = INST_DATA(CL_Databases->mcc_Class, data->GR_Databases);

      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Passwd"), data->GR_Databases, NULL, GPE_LIST);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Group"), d_data->GR_Groups, data->GR_Databases, NULL);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr(MSG_DatabasesRegister1), d_data->GR_Hosts      , data->GR_Databases, NULL);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr(MSG_DatabasesRegister2), d_data->GR_Protocols  , data->GR_Databases, NULL);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr(MSG_DatabasesRegister3), d_data->GR_Services   , data->GR_Databases, NULL);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Access"), d_data->GR_InetAccess      , data->GR_Databases, NULL);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr(MSG_DatabasesRegister4), d_data->GR_Inetd      , data->GR_Databases, NULL);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr(MSG_DatabasesRegister5), d_data->GR_Networks   , data->GR_Databases, NULL);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr(MSG_DatabasesRegister6), d_data->GR_Rpc        , data->GR_Databases, NULL);
   }

   if(data->GR_Provider && data->GR_User  && data->GR_Dialer &&
      data->GR_Databases && data->GR_Modem)
   {
//    struct Provider_Data    *provider_data    = INST_DATA(CL_Provider->mcc_Class  , data->GR_Provider);
//    struct User_Data        *user_data        = INST_DATA(CL_User->mcc_Class      , data->GR_User);
//    struct Databases_Data   *datab_data       = INST_DATA(CL_Databases->mcc_Class , data->GR_Databases);
//    struct Events_Data      *events_data      = INST_DATA(CL_Events->mcc_Class    , data->GR_Events);

      success = TRUE;
   }

   return(success);
}

///
#else
/// MainWindow_InitGroups
ULONG MainWindow_InitGroups(struct IClass *cl, Object *obj, Msg msg)
{
   static STRPTR provider_titles[] = { "TCP/IP", "Sana2", "Interface", "Services", NULL };
   static STRPTR user_titles[] = { "Settings", NULL };
   static STRPTR modem_titles[] = { "Modem / TA", "Device", NULL };
   static STRPTR dialer_titles[] = { "Script", "Events", "Misc", NULL };
   static STRPTR databases_titles[] = { "Passwd", "Group", "Hosts", "Protocols", "Services", "Access", "Inetd", "Networks", "Rpc", NULL };

   struct MainWindow_Data *data = INST_DATA(cl, obj);
   Object *tmp_group, *bodychunk;
   BOOL success = FALSE;

   data->GR_Provider       = data->GR_User       =
   data->GR_Dialer         = data->GR_Modem      =
   data->GR_Databases      = NULL;

   if(data->GR_Info)
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&information_header, (ULONG *)information_colors , (UBYTE *)information_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr("  Info"), data->GR_Info, NULL, NULL, bodychunk);
   }
   if(data->GR_User        = NewObject(CL_User->mcc_Class      , NULL, TAG_DONE))
   {
      struct User_Data *u_data = INST_DATA(CL_User->mcc_Class, data->GR_User);

      bodychunk = create_bodychunk((struct BitMapHeader *)&user_header, (ULONG *)user_colors , (UBYTE *)user_body);
      if(tmp_group = RegisterGroup(user_titles),
         MUIA_CycleChain      , 1,
         Child, u_data->GR_User,
      End)
         DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_Pages3), tmp_group, NULL, NULL, bodychunk);
   }
   if(data->GR_Provider = NewObject(CL_Provider->mcc_Class  , NULL, TAG_DONE))
   {
      struct Provider_Data *p_data = INST_DATA(CL_Provider->mcc_Class, data->GR_Provider);

      bodychunk = create_bodychunk((struct BitMapHeader *)&provider_header, (ULONG *)provider_colors , (UBYTE *)provider_body);
      if(tmp_group = RegisterGroup(provider_titles),
         MUIA_CycleChain      , 1,
         Child, p_data->GR_TCPIP,
         Child, p_data->GR_Sana2,
         Child, p_data->GR_Interface,
         Child, p_data->GR_Server,
      End)
         DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr("  Provider"), tmp_group, NULL, NULL, bodychunk);
   }
   if(data->GR_Modem      = NewObject(CL_Modem->mcc_Class    , NULL, TAG_DONE))
   {
      struct Modem_Data *m_data = INST_DATA(CL_Modem->mcc_Class, data->GR_Modem);

      bodychunk = create_bodychunk((struct BitMapHeader *)&modem_header, (ULONG *)modem_colors , (UBYTE *)modem_body);
      if(tmp_group = RegisterGroup(modem_titles),
         MUIA_CycleChain      , 1,
         Child, m_data->GR_Modem,
         Child, m_data->GR_Serial,
      End)
         DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr("  Modem"), tmp_group, NULL, NULL, bodychunk);
   }
   if(data->GR_Dialer       = NewObject(CL_Dialer->mcc_Class     , NULL, TAG_DONE))
   {
      struct Dialer_Data *d_data = INST_DATA(CL_Dialer->mcc_Class, data->GR_Dialer);

      bodychunk = create_bodychunk((struct BitMapHeader *)&dialer_header, (ULONG *)dialer_colors , (UBYTE *)dialer_body);
      if(tmp_group = RegisterGroup(dialer_titles),
         MUIA_CycleChain      , 1,
         Child, d_data->GR_Script,
         Child, d_data->GR_Events,
         Child, d_data->GR_Misc,
      End)
         DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr("  Dialer"), tmp_group, NULL, NULL, bodychunk);
   }
   if(data->GR_Databases   = NewObject(CL_Databases->mcc_Class , NULL, TAG_DONE))
   {
      struct Databases_Data *d_data = INST_DATA(CL_Databases->mcc_Class, data->GR_Databases);

      bodychunk = create_bodychunk((struct BitMapHeader *)&databases_header, (ULONG *)databases_colors , (UBYTE *)databases_body);
      if(tmp_group = RegisterGroup(databases_titles),
         MUIA_CycleChain      , 1,
         Child, d_data->GR_Passwd,
         Child, d_data->GR_Groups,
         Child, d_data->GR_Hosts,
         Child, d_data->GR_Protocols,
         Child, d_data->GR_Services,
         Child, d_data->GR_InetAccess,
         Child, d_data->GR_Inetd,
         Child, d_data->GR_Networks,
         Child, d_data->GR_Rpc,
      End)
         DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr("  DataBase"), tmp_group, NULL, NULL, bodychunk);
   }

   if(data->GR_Provider && data->GR_User  && data->GR_Dialer &&
      data->GR_Databases && data->GR_Modem)
   {
//    struct Provider_Data    *provider_data    = INST_DATA(CL_Provider->mcc_Class  , data->GR_Provider);
//    struct User_Data        *user_data        = INST_DATA(CL_User->mcc_Class      , data->GR_User);
//    struct Databases_Data   *datab_data       = INST_DATA(CL_Databases->mcc_Class , data->GR_Databases);
//    struct Events_Data      *events_data      = INST_DATA(CL_Events->mcc_Class    , data->GR_Events);

      success = TRUE;
   }

   return(success);
}

///
#endif

/// MainWindow_New
ULONG MainWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct MainWindow_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , VERS "  © 1997 by Michael Neuweiler, Active Software",
      MUIA_Window_ID       , MAKE_ID('A','R','E','F'),
      MUIA_Window_AppWindow, TRUE,
      MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, MainWindowMenu,0),
      WindowContents       , VGroup,
         Child, tmp.GR_Pager = GrouppagerObject,
            MUIA_Grouppager_ListHorizWeight  , 40,
            MUIA_Grouppager_ListMinLineHeight, MAX(PROVIDER_HEIGHT, MAX(USER_HEIGHT, MAX(MODEM_HEIGHT, MAX(DIALER_HEIGHT, MAX(INFORMATION_HEIGHT, DATABASES_HEIGHT))))),
            MUIA_Grouppager_ListAdjustWidth  , TRUE,
            MUIA_Grouppager_DefaultGroup, VGroup,
               GroupFrame,
               MUIA_Background, MUII_GroupBack,
               Child, HVSpace,
               Child, HVSpace,
               Child, CLabel("                    loading...                    "),
               Child, HVSpace,
               Child, CLabel("please wait"),
               Child, HVSpace,
               Child, HVSpace,
            End,
//            MUIA_Grouppager_BalanceObject    , TRUE,
#ifdef DO_LISTTREE
            MUIA_Grouppager_Type, MUIV_Grouppager_Type_Listtree,
#endif
         End,
         Child, MUI_MakeObject(MUIO_HBar, 2),
         Child, HGroup,
            MUIA_Group_SameSize  , TRUE,
            Child, tmp.BT_Save   = MakeButton(MSG_BT_Save),
            Child, HSpace(0),
            Child, tmp.BT_Cancel = MakeButton(MSG_BT_Cancel),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct MainWindow_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      if(!(data->GR_Info = VGroup,
         GroupFrame,
         MUIA_Background, "2:9c9c9c9c,9c9c9c9c,9c9c9c9c",
         Child, HVSpace,
         Child, HGroup,
            Child, HVSpace,
            Child, BodychunkObject,
               MUIA_FixWidth             , LOGO_WIDTH ,
               MUIA_FixHeight            , LOGO_HEIGHT,
               MUIA_Bitmap_Width         , LOGO_WIDTH ,
               MUIA_Bitmap_Height        , LOGO_HEIGHT,
               MUIA_Bodychunk_Depth      , LOGO_DEPTH ,
               MUIA_Bodychunk_Body       , (UBYTE *)logo_body,
               MUIA_Bodychunk_Compression, LOGO_COMPRESSION,
               MUIA_Bodychunk_Masking    , LOGO_MASKING,
               MUIA_Bitmap_SourceColors  , (ULONG *)logo_colors,
//               MUIA_Bitmap_Transparent   , 0,
            End,
            Child, HVSpace,
         End,
         Child, CLabel("Preferences "VSTRING" ("__DATE__")"),
         Child, HVSpace,
#ifdef DEMO
         Child, CLabel(GetStr(MSG_TX_DemoVersion)),
         Child, HVSpace,
#endif
      End))
         return(NULL);

      set(data->BT_Save   , MUIA_ShortHelp, GetStr(MSG_Help_Save));
      set(data->BT_Cancel , MUIA_ShortHelp, GetStr(MSG_Help_Cancel));

      DoMethod(obj            , MUIM_Notify, MUIA_Window_CloseRequest, TRUE            , obj, 2, MUIM_MainWindow_Finish, 0);
      DoMethod(data->BT_Cancel  , MUIM_Notify, MUIA_Pressed            , FALSE           , obj, 2, MUIM_MainWindow_Finish, 0);
      DoMethod(data->BT_Save    , MUIM_Notify, MUIA_Pressed            , FALSE           , obj, 2, MUIM_MainWindow_Finish, 1);

      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ICONIFY)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         MUIV_Notify_Application, 3, MUIM_Set, MUIA_Application_Iconified, TRUE);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT)       , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 1, MUIM_MainWindow_About);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_QUIT)        , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         MUIV_Notify_Application, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_MUI)         , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
   }
   return((ULONG)obj);
}

///
/// MainWindow_Dispatcher
SAVEDS ASM ULONG MainWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(MainWindow_New           (cl, obj, (APTR)msg));
   if(msg->MethodID == MUIM_MainWindow_InitGroups)
      return(MainWindow_InitGroups    (cl, obj, (APTR)msg));
   if(msg->MethodID == MUIM_MainWindow_Finish)
      return(MainWindow_Finish        (cl, obj, (APTR)msg));
   if(msg->MethodID == MUIM_MainWindow_LoadConfig)
      return(MainWindow_LoadConfig    (cl, obj, (APTR)msg));
   if(msg->MethodID == MUIM_MainWindow_SaveConfig)
      return(MainWindow_SaveConfig    (cl, obj, (APTR)msg));
   if(msg->MethodID == MUIM_MainWindow_About)
      return(MainWindow_About         (cl, obj, (APTR)msg));
   if(msg->MethodID == MUIM_MainWindow_AboutFinish)
      return(MainWindow_AboutFinish   (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

