// do not call any bsdsocket.lib func unless SocketBase != 0 !!!

/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "/genesis.lib/proto/genesis.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_MainWindow.h"
#include "mui_Online.h"
#include "mui_IfaceReq.h"
#include "mui_Led.h"
#include "protos.h"
#include "sana.h"

///
/// external variables
extern int errno;
extern struct Config Config;
extern Object *app, *win, *status_win;
extern struct MUI_CustomClass  *CL_MainWindow, *CL_Online, *CL_IfaceReq, *CL_Led;
extern struct NewMenu MainMenu[];
extern struct Hook des_hook, txtobj_hook, objtxt_hook;
extern char connectspeed[];
extern char config_file[];
extern struct Library *SocketBase, *LockSocketBase, *GenesisBase;

///

/// is_true
BOOL is_true(struct ParseConfig_Data *pc_data)
{
   if(!*pc_data->pc_contents || !stricmp(pc_data->pc_contents, "yes") ||
      !stricmp(pc_data->pc_contents, "true") || !strcmp(pc_data->pc_contents, "1"))
      return(TRUE);
   return(FALSE);
}

///
/// is_false
BOOL is_false(struct ParseConfig_Data *pc_data)
{
   if(!stricmp(pc_data->pc_contents, "no") || !stricmp(pc_data->pc_contents, "false") ||
      !strcmp(pc_data->pc_contents, "0"))
      return(TRUE);
   return(FALSE);
}

///
/// in_cksum
unsigned short in_cksum( unsigned short *addr, int len )
{
   register int nleft = len;
   register unsigned short *w = addr;
   register int sum = 0;
   unsigned short answer = 0;

   /*
    *  Our algorithm is simple, using a 32 bit accumulator (sum),
    *  we add sequential 16 bit words to it, and at the end, fold
    *  back all the carry bits from the top 16 bits into the lower
    *  16 bits.
    */
   while( nleft > 1 )  {
      sum += *w++;
      nleft -= 2;
   }

   /* mop up an odd byte, if necessary */
   if( nleft == 1 ) {
      *(char *)(&answer) = *(char*)w ;
      sum += answer;
   }

   /*
    * add back carry outs from top 16 bits to low 16 bits
    */
   sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
   sum += (sum >> 16);        /* add carry */
   answer = ~sum;          /* truncate to 16 bits */
   return (answer);
}

///

enum { type_ISP=1, type_User, type_Iface, type_LoginScript };
struct Library *UserGroupBase;

/*
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>


/// MainWindow_SendPing
ULONG MainWindow_SendPing(struct IClass *cl, Object *obj, struct MUIP_MainWindow_SendPing *msg)
{
//   struct MainWindow_Data *data = INST_DATA(cl, obj);
   ULONG ip;
   int sock;
   unsigned short myid = ((ULONG)FindTask(NULL)) & 0xffff;
   static char outpack[256];
   struct icmp *icp;
   struct hostent *host;
   BOOL clear_socketbase = FALSE;

   if(!SocketBase)
   {
      clear_socketbase = TRUE;
      SocketBase = LockSocketBase;
   }

   if(!SocketBase)
      return(NULL);

   if(host = gethostbyname(msg->hostname))
   {
      memcpy(&ip, *host->h_addr_list, 4);
      if((sock = socket(AF_INET, SOCK_RAW, 1)) >= 0)
      {
         struct sockaddr_in sockadr = { 0 };

         icp = (struct icmp *)outpack;
         memset(outpack, 0, sizeof(outpack));
         icp->icmp_type  = ICMP_ECHO;
         icp->icmp_seq   = 0;
         icp->icmp_id    = myid;
         icp->icmp_cksum = in_cksum((USHORT *)icp, 16);

         memcpy(&sockadr.sin_addr, &ip, 4);
         sockadr.sin_family = AF_INET;
         sendto(sock, outpack, 16, 0, (struct sockaddr*)&sockadr, sizeof(sockadr));

         // restart timer


         CloseSocket(sock);
      }
   }
   if(clear_socketbase)
      SocketBase = NULL;
}

///
*/

/// MainWindow_LoadConfig
ULONG MainWindow_LoadConfig(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct ParseConfig_Data pc_data;
   BOOL success = FALSE, first_provider = TRUE;
   LONG pos;
   char buffer[81];

   clear_config(&Config);
   DoMethod(data->LI_Providers, MUIM_List_Clear);
   DoMethod(data->LI_Users, MUIM_List_Clear);

   if(ParseConfig(config_file, &pc_data))
   {
      while(ParseNext(&pc_data))
      {
         if(!stricmp(pc_data.pc_argument, "SerialDevice"))
            strncpy(Config.cnf_serialdevice, pc_data.pc_contents, sizeof(Config.cnf_serialdevice));
         else if(!stricmp(pc_data.pc_argument, "SerialUnit"))
            Config.cnf_serialunit = atol(pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "BaudRate"))
            Config.cnf_baudrate = atol(pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "IgnoreDSR") && is_true(&pc_data))
            Config.cnf_flags |= CFL_IgnoreDSR;
         else if(!stricmp(pc_data.pc_argument, "7Wire") && is_false(&pc_data))
            Config.cnf_flags &= ~CFL_7Wire;
         else if(!stricmp(pc_data.pc_argument, "RadBoogie") && is_false(&pc_data))
            Config.cnf_flags &= ~CFL_RadBoogie;
         else if(!stricmp(pc_data.pc_argument, "XonXoff") && is_true(&pc_data))
            Config.cnf_flags |= CFL_IgnoreDSR;
         else if(!stricmp(pc_data.pc_argument, "OwnDevUnit") && is_true(&pc_data))
            Config.cnf_flags |= CFL_OwnDevUnit;
         else if(!stricmp(pc_data.pc_argument, "SerBufLen"))
            Config.cnf_serbuflen = atol(pc_data.pc_contents);

         else if(!stricmp(pc_data.pc_argument, "Modem"))
            strncpy(Config.cnf_modemname  , pc_data.pc_contents, sizeof(Config.cnf_modemname));
         else if(!stricmp(pc_data.pc_argument, "InitString"))
            strncpy(Config.cnf_initstring , pc_data.pc_contents, sizeof(Config.cnf_initstring));
         else if(!stricmp(pc_data.pc_argument, "DialPrefix"))
            strncpy(Config.cnf_dialprefix , pc_data.pc_contents, sizeof(Config.cnf_dialprefix));
         else if(!stricmp(pc_data.pc_argument, "DialSuffix"))
            strncpy(Config.cnf_dialsuffix , pc_data.pc_contents, sizeof(Config.cnf_dialsuffix));
         else if(!stricmp(pc_data.pc_argument, "RedialAttempts"))
            Config.cnf_redialattempts = atol(pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "RedialDelay"))
            Config.cnf_redialdelay = atol(pc_data.pc_contents);

         else if(!stricmp(pc_data.pc_argument, "QuickReconnect") && is_true(&pc_data))
            Config.cnf_flags |= CFL_QuickReconnect;
         else if(!stricmp(pc_data.pc_argument, "Debug") && is_true(&pc_data))
            Config.cnf_flags |= CFL_Debug;
         else if(!stricmp(pc_data.pc_argument, "ConfirmOffline") && is_true(&pc_data))
            Config.cnf_flags |= CFL_ConfirmOffline;
         else if(!stricmp(pc_data.pc_argument, "ShowLog") && is_false(&pc_data))
            Config.cnf_flags &= ~CFL_ShowLog;
         else if(!stricmp(pc_data.pc_argument, "ShowLamps") && is_false(&pc_data))
            Config.cnf_flags &= ~CFL_ShowLamps;
         else if(!stricmp(pc_data.pc_argument, "ShowConnect") && is_false(&pc_data))
            Config.cnf_flags &= ~CFL_ShowConnect;
         else if(!stricmp(pc_data.pc_argument, "ShowOnlineTime") && is_false(&pc_data))
            Config.cnf_flags &= ~CFL_ShowOnlineTime;
         else if(!stricmp(pc_data.pc_argument, "ShowButtons") && is_false(&pc_data))
            Config.cnf_flags &= ~CFL_ShowButtons;
         else if(!stricmp(pc_data.pc_argument, "ShowNetwork") && is_false(&pc_data))
            Config.cnf_flags &= ~CFL_ShowNetwork;
         else if(!stricmp(pc_data.pc_argument, "ShowUser") && is_false(&pc_data))
            Config.cnf_flags &= ~CFL_ShowUser;
         else if(!stricmp(pc_data.pc_argument, "ShowStatusWin") && is_false(&pc_data))
            Config.cnf_flags &= ~CFL_ShowStatusWin;
         else if(!stricmp(pc_data.pc_argument, "ShowSerialInput") && is_false(&pc_data))
            Config.cnf_flags &= ~CFL_ShowSerialInput;
         else if(!stricmp(pc_data.pc_argument, "StartupOpenWin") && is_false(&pc_data))
            Config.cnf_flags &= ~CFL_StartupOpenWin;
         else if(!stricmp(pc_data.pc_argument, "StartupIconify") && is_true(&pc_data))
            Config.cnf_flags |= CFL_StartupIconify;

         else if(!stricmp(pc_data.pc_argument, "Startup") && !Config.cnf_startup && (strlen(pc_data.pc_contents) > 2))
         {
            if(Config.cnf_startup = AllocVec(strlen(pc_data.pc_contents), MEMF_ANY))
            {
               strcpy(Config.cnf_startup, &pc_data.pc_contents[2]);
               Config.cnf_startuptype = *pc_data.pc_contents - 48;
            }
         }
         else if(!stricmp(pc_data.pc_argument, "Shutdown") && !Config.cnf_shutdown && (strlen(pc_data.pc_contents) > 2))
         {
            if(Config.cnf_shutdown = AllocVec(strlen(pc_data.pc_contents), MEMF_ANY))
            {
               strcpy(Config.cnf_shutdown, &pc_data.pc_contents[2]);
               Config.cnf_shutdowntype = *pc_data.pc_contents - 48;
            }
         }

         else if(!stricmp(pc_data.pc_argument, "Name"))
         {
            DoMethod(data->LI_Providers, MUIM_List_InsertSingle, pc_data.pc_contents, MUIV_List_Insert_Bottom);
            if(first_provider)
            {
               set(data->TX_Provider, MUIA_Text_Contents, pc_data.pc_contents);
               first_provider = FALSE;
            }
         }
      }
      ParseEnd(&pc_data);

      pos = 0;
      while(GetUserName(pos++, buffer, 80))
         DoMethod(data->LI_Users, MUIM_List_InsertSingle, buffer, MUIV_List_Insert_Bottom);

      if((Config.cnf_flags & CFL_ShowOnlineTime) || (Config.cnf_flags & CFL_ShowLamps))
      {
         set(data->GR_TimeLamps   , MUIA_ShowMe, TRUE);
         set(data->GR_Lamps       , MUIA_ShowMe, Config.cnf_flags & CFL_ShowLamps);
         set(data->GR_Online      , MUIA_ShowMe, Config.cnf_flags & CFL_ShowOnlineTime);
      }
      else
         set(data->GR_TimeLamps   , MUIA_ShowMe, FALSE);

      set(data->GR_Speed       , MUIA_ShowMe, Config.cnf_flags & CFL_ShowConnect);

      if((Config.cnf_flags & CFL_ShowNetwork) || (Config.cnf_flags & CFL_ShowUser))
      {
         set(data->GR_Config      , MUIA_ShowMe, TRUE);
         set(data->PO_Provider    , MUIA_ShowMe, Config.cnf_flags & CFL_ShowNetwork);
         set(data->PO_User        , MUIA_ShowMe, Config.cnf_flags & CFL_ShowUser);
         set(data->BO_ProviderUser, MUIA_ShowMe, ((Config.cnf_flags & CFL_ShowNetwork) && (Config.cnf_flags & CFL_ShowUser)));
      }
      else
         set(data->GR_Config      , MUIA_ShowMe, FALSE);

      set(data->GR_Log         , MUIA_ShowMe, Config.cnf_flags & CFL_ShowLog);
      set(data->GR_Buttons     , MUIA_ShowMe, Config.cnf_flags & CFL_ShowButtons);

      success = TRUE;
   }
   else
      MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_CouldNotOpenX), config_file);

   return((ULONG)success);
}
///
/// MainWindow_ChangeProvider
ULONG MainWindow_ChangeProvider(struct IClass *cl, Object *obj, struct MUIP_MainWindow_ChangeProvider *msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct ParseConfig_Data pc_data;
   struct Interface *iface = NULL;
   int current_type = 0, nr_ifaces;
   BOOL isp_ok = FALSE;
   BPTR fh;

   if(is_one_online(&data->isp.isp_ifaces))
   {
      set(win, MUIA_Window_Open, TRUE);
      if(!MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_StayOnline), GetStr(MSG_TX_ChangeProviderIfaceOnline)))
      {
         iterate_ifacelist(&data->isp.isp_ifaces, 0);
         DoMethod(win, MUIM_MainWindow_PutOffline);
      }
      else
      {
         nnset(data->TX_Provider, MUIA_Text_Contents, data->isp.isp_name);
         return(NULL);
      }
   }

   set(app, MUIA_Application_Sleep, TRUE);
   if(ParseConfig(config_file, &pc_data))
   {
      clear_isp(&data->isp);

      while(ParseNext(&pc_data))
      {
         if(!stricmp(pc_data.pc_argument, "ISP"))
         {
            if(isp_ok)
               break;
            current_type = type_ISP;
         }
         else if(!stricmp(pc_data.pc_argument, "LOGINSCRIPT") && isp_ok)
            current_type = type_LoginScript;
         else if(!stricmp(pc_data.pc_argument, "INTERFACE") && isp_ok)
         {
            if(iface = AllocVec(sizeof(struct Interface), MEMF_ANY | MEMF_CLEAR))
            {
               strcpy(iface->if_netmask, "255.255.255.0");
               iface->if_MTU             = 1500;
               NewList((struct List *)&iface->if_events);

               AddTail((struct List *)&data->isp.isp_ifaces, (struct Node *)iface);
               current_type = type_Iface;
            }
            else
               current_type = NULL;
         }

         if(!current_type)
            continue;

         switch(current_type)
         {
            case type_ISP:
               if(!stricmp(pc_data.pc_argument, "Name"))
               {
                  if(strcmp(pc_data.pc_contents, (STRPTR)xget(data->TX_Provider, MUIA_Text_Contents)))
                  {
                     current_type = NULL;
                     continue;
                  }
                  isp_ok = TRUE;
                  strncpy(data->isp.isp_name, pc_data.pc_contents, sizeof(data->isp.isp_name));
               }
               else if(!stricmp(pc_data.pc_argument, "Login"))
                  strncpy(data->isp.isp_login, pc_data.pc_contents, sizeof(data->isp.isp_login));
               else if(!stricmp(pc_data.pc_argument, "Password"))
                  decrypt(pc_data.pc_contents, data->isp.isp_password);
               if(!stricmp(pc_data.pc_argument, "Phone"))
                  strncpy(data->isp.isp_phonenumber, pc_data.pc_contents, sizeof(data->isp.isp_phonenumber));
               else if(!stricmp(pc_data.pc_argument, "NameServer"))
                  add_server(&data->isp.isp_nameservers, pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "DomainName"))
                  add_server(&data->isp.isp_domainnames, pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "BOOTPServer"))
                  strncpy(data->isp.isp_bootp, pc_data.pc_contents, sizeof(data->isp.isp_bootp));
               else if(!stricmp(pc_data.pc_argument, "UseBootP"))
                  data->isp.isp_flags |= ISF_UseBootp;
               else if(!stricmp(pc_data.pc_argument, "HostName"))
                  strncpy(data->isp.isp_hostname, pc_data.pc_contents, sizeof(data->isp.isp_hostname));
               else if(!stricmp(pc_data.pc_argument, "DontQueryHostname") && is_true(&pc_data))
                  data->isp.isp_flags |= ISF_DontQueryHostname;
               else if(!stricmp(pc_data.pc_argument, "GetTime") && is_true(&pc_data))
                  data->isp.isp_flags |= ISF_GetTime;
               else if(!stricmp(pc_data.pc_argument, "SaveTime") && is_true(&pc_data))
                  data->isp.isp_flags |= ISF_SaveTime;
               else if(!stricmp(pc_data.pc_argument, "TimeServer"))
                  strncpy(data->isp.isp_timename, pc_data.pc_contents, sizeof(data->isp.isp_timename));
               break;

            case type_Iface:
               if(iface)
               {
                  if(!stricmp(pc_data.pc_argument, "IfName"))
                     strncpy(iface->if_name, pc_data.pc_contents, sizeof(iface->if_name));
                  else if(!stricmp(pc_data.pc_argument, "Sana2Device"))
                     strncpy(iface->if_sana2device, pc_data.pc_contents, sizeof(iface->if_sana2device));
                  else if(!stricmp(pc_data.pc_argument, "Sana2Unit"))
                     iface->if_sana2unit = atol(pc_data.pc_contents);
                  else if(!stricmp(pc_data.pc_argument, "Sana2Config"))
                     strncpy(iface->if_sana2config, pc_data.pc_contents, sizeof(iface->if_sana2config));
                  else if(!stricmp(pc_data.pc_argument, "Sana2ConfigText") && !iface->if_sana2configtext)
                  {
                     if(iface->if_sana2configtext = AllocVec(strlen(pc_data.pc_contents) + 1, MEMF_ANY | MEMF_CLEAR))
                     {
                        if(strstr(pc_data.pc_contents, "\\n"))
                        {
                           STRPTR ptr, ptr2;

                           ptr = pc_data.pc_contents;
                           FOREVER
                           {
                              if(ptr2 = strstr(ptr, "\\n"))
                                 *ptr2 = NULL;
                              strcat(iface->if_sana2configtext, ptr);
                              if(ptr2)
                                 strcat(iface->if_sana2configtext, "\n");
                              else
                                 break;

                              ptr = ptr2 + 2;
                           }
                        }
                        else
                           strcpy(iface->if_sana2configtext, pc_data.pc_contents);
                     }
                  }
                  else if(!stricmp(pc_data.pc_argument, "IfConfigParams") && !iface->if_configparams)
                  {
                     if(iface->if_configparams = AllocVec(strlen(pc_data.pc_contents) + 1, MEMF_ANY))
                        strcpy(iface->if_configparams, pc_data.pc_contents);
                  }
                  else if(!stricmp(pc_data.pc_argument, "MTU"))
                     iface->if_MTU = atol(pc_data.pc_contents);
                  else if(!stricmp(pc_data.pc_argument, "IPAddr") && strcmp(pc_data.pc_contents, "0.0.0.0"))
                     strncpy(iface->if_addr, pc_data.pc_contents, sizeof(iface->if_addr));
                  else if(!stricmp(pc_data.pc_argument, "DestIP") && strcmp(pc_data.pc_contents, "0.0.0.0"))
                     strncpy(iface->if_dst, pc_data.pc_contents, sizeof(iface->if_dst));
                  else if(!stricmp(pc_data.pc_argument, "Gateway") && strcmp(pc_data.pc_contents, "0.0.0.0"))
                     strncpy(iface->if_gateway, pc_data.pc_contents, sizeof(iface->if_gateway));
                  else if(!stricmp(pc_data.pc_argument, "Netmask"))
                     strncpy(iface->if_netmask, pc_data.pc_contents, sizeof(iface->if_netmask));
                  else if(!stricmp(pc_data.pc_argument, "KeepAlive"))
                     iface->if_keepalive = atol(pc_data.pc_contents);
                  else if(!stricmp(pc_data.pc_argument, "AlwaysOnline") && is_true(&pc_data))
                     iface->if_flags |= IFL_AlwaysOnline;
                  else
                  {
                     struct ScriptLine *script_line;
                     int command;

                     if(!stricmp(pc_data.pc_argument, event_commands[IFE_Online]))
                        command = IFE_Online;
                     else if(!stricmp(pc_data.pc_argument, event_commands[IFE_OnlineFail]))
                        command = IFE_OnlineFail;
                     else if(!stricmp(pc_data.pc_argument, event_commands[IFE_OfflineActive]))
                        command = IFE_OfflineActive;
                     else if(!stricmp(pc_data.pc_argument, event_commands[IFE_OfflinePassive]))
                        command = IFE_OfflinePassive;
                     else
                        command = -1;

                     if(command != -1)
                     {
                        if(script_line = AllocVec(sizeof(struct ScriptLine), MEMF_ANY))
                        {
                           script_line->sl_command = command;
                           strncpy(script_line->sl_contents, &pc_data.pc_contents[2], script_line->sl_command);
                           script_line->sl_userdata = *pc_data.pc_contents - 48;

                           AddTail((struct List *)&iface->if_events, (struct Node *)script_line);
                        }
                     }
                  }
               }
               break;

            case type_LoginScript:
            {
               struct ScriptLine *script_line;
               int command;

               if(!stricmp(pc_data.pc_argument, script_commands[SL_Send]))
                  command = SL_Send;
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_WaitFor]))
                  command = SL_WaitFor;
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_Dial]))
                  command = SL_Dial;
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_GoOnline]))
                  command = SL_GoOnline;
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_SendLogin]))
                  command = SL_SendLogin;
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_SendPassword]))
                  command = SL_SendPassword;
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_SendBreak]))
                  command = SL_SendBreak;
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_Exec]))
                  command = SL_Exec;
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_Pause]))
                  command = SL_Pause;
               else
                  command = -1;

               if(command != -1)
               {
                  if(script_line = AllocVec(sizeof(struct ScriptLine), MEMF_ANY))
                  {
                     script_line->sl_command = command;
                     strncpy(script_line->sl_contents, pc_data.pc_contents, script_line->sl_command);

                     AddTail((struct List *)&data->isp.isp_loginscript, (struct Node *)script_line);
                  }
               }
            }
            break;
         }
      }
      ParseEnd(&pc_data);
   }

   // prepare some files

   if(fh = CreateDir("ENV:NetConfig"))
      UnLock(fh);

   nr_ifaces = 0;
   if(fh = Open("ENV:NetConfig/AutoInterfaces", MODE_NEWFILE))
   {
      if(data->isp.isp_ifaces.mlh_TailPred != (struct MinNode *)&data->isp.isp_ifaces)
      {
         iface = (struct Interface *)data->isp.isp_ifaces.mlh_Head;
         while(iface->if_node.mln_Succ)
         {
            iface->if_userdata = (APTR)nr_ifaces++;   // so they know which led they belong to
            FPrintf(fh, "%ls DEV=%ls UNIT=%ld", iface->if_name, iface->if_sana2device, iface->if_sana2unit);
            if(iface->if_configparams)
               FPrintf(fh, " %ls", iface->if_configparams);
            FPrintf(fh, "\n");

            iface = (struct Interface *)iface->if_node.mln_Succ;
         }
      }
      Close(fh);
   }
   if(fh = Open("AmiTCP:db/resolv.conf", MODE_NEWFILE))
   {
      FPrintf(fh, "; This file is built dynamically - do not edit\n");
      Close(fh);
   }

   amirexx_do_command("RESET");

   DoMethod(obj, MUIM_MainWindow_SetStates);
   set(app, MUIA_Application_Sleep, FALSE);

   if(data->nr_leds != nr_ifaces)
   {
      DoMethod(data->GR_Status, MUIM_Group_InitChange);
      while((data->nr_leds > nr_ifaces) && (data->nr_leds > 1))
      {
         data->nr_leds--;
         DoMethod(data->GR_Status, OM_REMMEMBER, data->GR_Led[data->nr_leds]);
         MUI_DisposeObject(data->GR_Led[data->nr_leds]);
         data->GR_Led[data->nr_leds] = NULL;
      }

      while((data->nr_leds < nr_ifaces) && (data->nr_leds < 32))
      {
         data->GR_Led[data->nr_leds] = NewObject(CL_Led->mcc_Class, NULL, TAG_DONE),
         DoMethod(data->GR_Status, OM_ADDMEMBER, data->GR_Led[data->nr_leds]);
         data->nr_leds++;
         data->GR_Led[data->nr_leds] = NULL;
      }

      DoMethod(data->GR_Status, MUIM_Group_ExitChange);
   }
   current_type = data->nr_leds;
   while(current_type--)
      set(data->GR_Led[current_type], MUIA_Group_ActivePage, MUIV_Led_Black);

   // only go online when amitcp's already running
   if(LockSocketBase)
   {
      iterate_ifacelist(&data->isp.isp_ifaces, 1);
      DoMethod(win, MUIM_MainWindow_PutOnline);
   }

   return(NULL);
}

///
/// MainWindow_ChangeUser
ULONG MainWindow_ChangeUser(struct IClass *cl, Object *obj, struct MUIP_MainWindow_ChangeUser *msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct User *user;
   STRPTR ptr;

   DoMethod(data->PO_User, MUIM_Popstring_Close, TRUE);
   DoMethod(data->LI_Users, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
   if(user = GetUser(ptr, NULL, NULL))
   {
      SetGlobalUser(user);
      set(data->TX_User, MUIA_Text_Contents, user->us_name);
      FreeUser(user);
   }

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
/// MainWindow_About
ULONG MainWindow_About(struct IClass *cl, Object *obj, Msg msg)
{
#ifdef DEMO
   MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), "\033b\033cGenesis\033n\033c\n"VERTAG"\n\n%ls\033bTHIS IS A DEMO VERSION !\033n\n\nAREXX port: '%ls'", GetStr(MSG_TX_About), xget(app, MUIA_Application_Base));
#else
   MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), "\033b\033cGenesis\033n\033c\n"VERTAG"\n\n%ls\n\nAREXX port: '%ls'", GetStr(MSG_TX_About), xget(app, MUIA_Application_Base));
#endif
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
/// MainWindow_OnOffline
ULONG MainWindow_OnOffline(struct IClass *cl, Object *obj, struct MUIP_MainWindow_OnOffline *msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   Object *window;

   if(data->isp.isp_ifaces.mlh_TailPred != (struct MinNode *)&data->isp.isp_ifaces)
   {
      struct Interface *iface;

      iface = (struct Interface *)data->isp.isp_ifaces.mlh_Head;
      if(iface->if_node.mln_Succ)   // is there at least one iface ?
      {
         iface = (struct Interface *)iface->if_node.mln_Succ;
         if(iface->if_node.mln_Succ)   // is there a second iface ?
         {
            set(app, MUIA_Application_Sleep, TRUE);
            if(window = NewObject(CL_IfaceReq->mcc_Class, NULL, TAG_DONE))
            {
               DoMethod(app, OM_ADDMEMBER, window);
               if(DoMethod(window, MUIM_IfaceReq_BuildList, &data->isp.isp_ifaces, msg->online))
                  set(window, MUIA_Window_Open, TRUE);
               else
                  DoMethod(window, MUIM_IfaceReq_Finished, FALSE);
            }
            else
               set(app, MUIA_Application_Sleep, FALSE);
         }
         else
         {
            iface = (struct Interface *)data->isp.isp_ifaces.mlh_Head;
            iface->if_flags |= (msg->online ? IFL_PutOnline : IFL_PutOffline);   // in case there's only one iface
            DoMethod(obj, (msg->online ? MUIM_MainWindow_PutOnline : MUIM_MainWindow_PutOffline));
         }
      }
   }

   return(NULL);
}

///
/// MainWindow_PutOnline
ULONG MainWindow_PutOnline(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct Interface *iface;
   BOOL put_one_online = FALSE;

   if(data->isp.isp_ifaces.mlh_TailPred != (struct MinNode *)&data->isp.isp_ifaces)
   {
      iface = (struct Interface *)data->isp.isp_ifaces.mlh_Head;
      while(iface->if_node.mln_Succ)
      {
         if(iface->if_flags & IFL_PutOnline)
            put_one_online = TRUE;
         iface = (struct Interface *)iface->if_node.mln_Succ;
      }
   }
   if(put_one_online)
   {
      if(Config.cnf_flags & CFL_ShowStatusWin)
         set(app, MUIA_Application_Sleep, TRUE);
      if(status_win = NewObject(CL_Online->mcc_Class, NULL, TAG_DONE))
      {
         DoMethod(app, OM_ADDMEMBER, status_win);
         if(Config.cnf_flags & CFL_ShowStatusWin)
            set(status_win, MUIA_Window_Open, TRUE);
         DoMethod(status_win, MUIM_Online_GoOnline);
      }
   }
   return(NULL);
}

///
/// MainWindow_PutOffline
ULONG MainWindow_PutOffline(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);

   set(app, MUIA_Application_Sleep, TRUE);

   if(data->isp.isp_ifaces.mlh_TailPred != (struct MinNode *)&data->isp.isp_ifaces)
   {
      struct sana2 *s2;
      struct Interface *iface;

      iface = (struct Interface *)data->isp.isp_ifaces.mlh_Head;
      while(iface->if_node.mln_Succ)
      {
         if(iface->if_flags & IFL_PutOffline)
         {
            iface->if_flags &= ~IFL_PutOffline;

            if(s2 = sana2_create(iface->if_sana2device, iface->if_sana2unit))
            {
               if(sana2_offline(s2))
                  exec_event(&iface->if_events, IFE_OfflineActive);
               else
               {
                  SocketBase = LockSocketBase;
                  syslog_AmiTCP(LOG_CRIT, "offline: could not put %ls unit %ld offline.", iface->if_sana2device, iface->if_sana2unit);
                  SocketBase = NULL;
               }

               sana2_delete(s2);
            }
            else
            {
               SocketBase = LockSocketBase;
               syslog_AmiTCP(LOG_CRIT, "offline: could not open %ls unit %ld.", iface->if_sana2device, iface->if_sana2unit);
               SocketBase = NULL;
            }
            iface->if_flags &= ~IFL_IsOnline;
            set(data->GR_Led[(ULONG)iface->if_userdata], MUIA_Group_ActivePage, MUIV_Led_Black);
         }
         iface = (struct Interface *)iface->if_node.mln_Succ;
      }
   }

   DoMethod(obj, MUIM_MainWindow_SetStates);
   set(app, MUIA_Application_Sleep, FALSE);
   return(NULL);
}

///
/// MainWindow_TimeTrigger
ULONG MainWindow_TimeTrigger(struct IClass *cl, Object *obj)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   ULONG h, m, s;

   GetSysTime(&data->time);
   s = data->time.tv_secs - data->online;
   m = s / 60;
   h = m / 60;
   s = s % 60;
   m = m % 60;
   sprintf(data->time_str, "%ls %02ld:%02ld:%02ld", GetStr(MSG_TX_TimeOnline), h, m, s);
   set(data->TX_Online, MUIA_Text_Contents, data->time_str);

   return(TRUE);
}
///
/// MainWindow_SetStates
ULONG MainWindow_SetStates(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct Interface *iface;
   BOOL one_online = FALSE, one_offline = FALSE;

   if(data->isp.isp_ifaces.mlh_TailPred != (struct MinNode *)&data->isp.isp_ifaces)
   {
      iface = (struct Interface *)data->isp.isp_ifaces.mlh_Head;
      while(iface->if_node.mln_Succ)
      {
         if((iface->if_flags & IFL_IsOnline) && !(iface->if_flags & IFL_AlwaysOnline))
            one_online = TRUE;
         if(!(iface->if_flags & IFL_IsOnline))
            one_offline = TRUE;
         iface = (struct Interface *)iface->if_node.mln_Succ;
      }
   }
   set(data->BT_Online, MUIA_Disabled, !one_offline);
   set(data->BT_Offline, MUIA_Disabled, !one_online);
   set(data->BT_Provider, MUIA_Disabled, one_online);

   if(one_online)
   {
      set(data->TX_Speed, MUIA_Text_Contents, connectspeed);

      if(!data->online)
      {
         GetSysTime(&data->time);
         data->online = data->time.tv_secs;

         data->online_ihn.ihn_Object = obj;
         data->online_ihn.ihn_Method = MUIM_MainWindow_TimeTrigger;
         data->online_ihn.ihn_Flags  = MUIIHNF_TIMER;
         data->online_ihn.ihn_Millis = 1000;
         DoMethod(_app(obj), MUIM_Application_AddInputHandler, &data->online_ihn);
      }
   }
   else
   {
      if(data->online)
      {
         DoMethod(_app(obj), MUIM_Application_RemInputHandler, &data->online_ihn);
         data->online = 0;
      }
      set(data->TX_Speed   , MUIA_Text_Contents, "-");
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
      Delay(10);
      DoMethod(_app(msg->window), OM_REMMEMBER, msg->window);
      MUI_DisposeObject(msg->window);
   }

   set(app, MUIA_Application_Sleep, FALSE);

   if(status_win == msg->window)
      status_win = NULL;

   return(NULL);
}

///
/// MainWindow_UpdateLog
ULONG MainWindow_UpdateLog(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   struct LogEntry *log_entry;
   char buf[MAXPATHLEN];
   STRPTR ptr1, ptr2, ptr3;
   BPTR fh;

   if(log_entry = AllocVec(sizeof(struct LogEntry), MEMF_ANY))
   {
      if(fh = Open("T:AmiTCP.log", MODE_OLDFILE))
      {
         Seek(fh, data->log_pos, OFFSET_BEGINNING);
         DoMethod(data->LI_Log, MUIA_NList_Quiet, TRUE);

         while(FGets(fh, buf, MAXPATHLEN))
         {
            bzero(log_entry, sizeof(struct LogEntry));

            if(ptr1 = strchr(buf, '['))
            {
               ptr1--;
               *ptr1 = NULL;

               strncpy(log_entry->time, buf, 30);
               ptr1 += 2;

               if(ptr3 = ptr2 = strchr(ptr1, ']'))
               {
                  ptr2--;
                  while(*ptr2 == ' ')
                     ptr2--;
                  ptr2++;
                  *ptr2 = NULL;

                  strncpy(log_entry->type, ptr1, 20);

                  ptr3 += 2;
                  strncpy(log_entry->info, ptr3, 80);

                  DoMethod(data->LI_Log, MUIM_NList_InsertSingle, log_entry, MUIV_NList_Insert_Bottom);


                  if(ptr2 = strstr(ptr3, "has been put offline"))
                  {
                     struct Interface *iface;

                     ptr2--;
                     *ptr2 = NULL;

                     if(iface = find_by_name(&data->isp.isp_ifaces, ptr3))
                     {
                        set(data->GR_Led[(ULONG)iface->if_userdata], MUIA_Group_ActivePage, MUIV_Led_Red);
                        exec_event(&iface->if_events, IFE_OfflinePassive);
                     }

                     DoMethod(win, MUIM_MainWindow_SetStates);
                  }
               }
            }
         }

         data->log_pos = Seek(fh, 0, OFFSET_CURRENT);

         while(xget(data->LI_Log, MUIA_NList_Entries) > 256)
            DoMethod(data->LI_Log, MUIM_NList_Remove, MUIV_NList_Remove_First);
         DoMethod(data->LI_Log, MUIA_NList_Quiet, FALSE);

         Close(fh);
      }
      FreeVec(log_entry);
   }

   DoMethod(data->LI_Log, MUIM_NList_Jump, xget(data->LI_Log, MUIA_NList_Entries)-1);

   return(NULL);
}
///
/// MainWindow_GenesisPrefs
ULONG MainWindow_GenesisPrefs(struct IClass *cl, Object *obj, Msg msg)
{
   STRPTR try[] = { "AMITCP:GenesisPrefs", "PROGDIR:GenesisPrefs", "GenesisPrefs", NULL };
   STRPTR *ptr;

   ptr = &try[0];
   while(*ptr)
   {
      if(GetFileSize(*ptr) >= 0) // the file might be a link => size might be zero
      {
         run_async(*ptr);
         break;
      }
      ptr++;
   }
   return(NULL);
}
///

/// Log_ConstructFunc
SAVEDS ASM struct LogEntry *Log_ConstructFunc(register __a2 APTR pool, register __a1 struct LogEntry *src)
{
   struct LogEntry *new;

   if((new = (struct LogEntry *)AllocVec(sizeof(struct LogEntry), MEMF_ANY | MEMF_CLEAR)) && src && src != (APTR)-1)
      memcpy(new, src, sizeof(struct LogEntry));
   return(new);
}
const struct Hook Log_ConstructHook= { { 0,0 }, (VOID *)Log_ConstructFunc , NULL, NULL };

///
/// Log_DisplayFunc
SAVEDS ASM LONG Log_DisplayFunc(register __a2 char **array, register __a1 struct LogEntry *log_entry)
{
   if(log_entry)
   {
      *array++ = log_entry->type;
      *array++ = log_entry->info;
      *array   = log_entry->time;
   }
   else
   {
      *array++ = GetStr(MSG_TX_LogType);
      *array++ = GetStr(MSG_TX_LogInformation);
      *array   = GetStr(MSG_TX_LogTime);
   }
   return(NULL);
}
const struct Hook Log_DisplayHook  = { { 0,0 }, (VOID *)Log_DisplayFunc   , NULL, NULL };

///

/// MainWindow_Dispose
ULONG MainWindow_Dispose(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);

   clear_isp(&data->isp);

   return(DoSuperMethodA(cl, obj, msg));
}

///
/// MainWindow_New
ULONG MainWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct MainWindow_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , GetStr(MSG_TX_MainWindowTitle),
      MUIA_Window_ID       , MAKE_ID('M','A','I','N'),
      MUIA_Window_Height   , MUIV_Window_Height_MinMax(2),
      MUIA_Window_Width    , MUIV_Window_Width_MinMax(5),
      MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, MainMenu, NULL),
      WindowContents       , VGroup,
         Child, tmp.GR_TimeLamps = HGroup,
            MUIA_Weight, 1,
            MUIA_Group_SameHeight, TRUE,
            Child, tmp.GR_Online = HGroup,
               Child, tmp.TX_Online = MakeText(GetStr(MSG_TX_TimeOnline), FALSE),
            End,
            Child, tmp.GR_Lamps = VGroup,
               MUIA_Weight, 1,
               TextFrame,
               GroupSpacing(0),
               MUIA_InnerTop     , 0,
               MUIA_InnerBottom  , 0,
               Child, HVSpace,
               Child, HGroup,
                  InnerSpacing(0, 0),
                  GroupSpacing(0),
                  Child, HVSpace,
                  Child, tmp.GR_Status = HGroup,
                     InnerSpacing(0, 0),
                     GroupSpacing(2),
                     Child, tmp.GR_Led[0] = NewObject(CL_Led->mcc_Class, NULL, TAG_DONE),
                  End,
                  Child, HVSpace,
               End,
               Child, HVSpace,
            End,
         End,
         Child, tmp.GR_Speed = HGroup,
            Child, tmp.TX_Speed = MakeText(NULL, FALSE),
         End,
         Child, tmp.GR_Config = HGroup,
            Child, tmp.PO_Provider = PopobjectObject,
               MUIA_Popstring_String      , tmp.TX_Provider = MakeText(NULL, FALSE),
               MUIA_Popstring_Button      , tmp.BT_Provider = PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &txtobj_hook,
               MUIA_Popobject_ObjStrHook  , &objtxt_hook,
               MUIA_Popobject_Object      , tmp.LV_Providers = ListviewObject,
                  MUIA_Listview_List, tmp.LI_Providers = ListObject,
                     MUIA_Frame             , MUIV_Frame_InputList,
                     MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
                     MUIA_List_DestructHook , MUIV_List_DestructHook_String,
                  End,
               End,
            End,
            Child, tmp.BO_ProviderUser = BalanceObject, End,
            Child, tmp.PO_User = PopobjectObject,
               MUIA_Weight, 70,
               MUIA_Popstring_String      , tmp.TX_User = MakeText(NULL, FALSE),
               MUIA_Popstring_Button      , tmp.BT_User = PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &txtobj_hook,
               MUIA_Popobject_Object      , tmp.LV_Users = ListviewObject,
                  MUIA_Listview_List, tmp.LI_Users = ListObject,
                     MUIA_Frame             , MUIV_Frame_InputList,
                     MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
                     MUIA_List_DestructHook , MUIV_List_DestructHook_String,
                  End,
               End,
            End,
         End,
         Child, tmp.GR_Log = VGroup,
            Child, tmp.LV_Log = NListviewObject,
               MUIA_CycleChain            , 1,
               MUIA_NListview_NList        , tmp.LI_Log = NListObject,
                  MUIA_Font                , MUIV_Font_Tiny,
                  MUIA_Frame               , MUIV_Frame_InputList,
                  MUIA_NList_DisplayHook   , &Log_DisplayHook,
                  MUIA_NList_ConstructHook , &Log_ConstructHook,
                  MUIA_NList_DestructHook  , &des_hook,
                  MUIA_NList_Format        , "BAR,BAR,",
                  MUIA_NList_Title         , TRUE,
                  MUIA_NList_Input         , FALSE,
               End,
            End,
         End,
         Child, tmp.GR_Buttons = VGroup,
            Child, MUI_MakeObject(MUIO_HBar, 2),
            Child, HGroup,
               Child, tmp.BT_Online = MakeButton(MSG_BT_Connect),
               Child, tmp.BT_Offline = MakeButton(MSG_BT_Disconnect),
            End,
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct MainWindow_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->online = 0;
      data->nr_leds = 1;

      bzero(&data->isp, sizeof(struct ISP));
      NewList((struct List *)&data->isp.isp_ifaces);
      NewList((struct List *)&data->isp.isp_loginscript);

      set(data->BT_Offline, MUIA_Disabled, TRUE);

      set(data->BT_Online  , MUIA_ShortHelp, GetStr(MSG_HELP_Connect));
      set(data->BT_Offline , MUIA_ShortHelp, GetStr(MSG_HELP_Disconnect));
      set(data->TX_Online  , MUIA_ShortHelp, GetStr(MSG_HELP_TimeOnline));
      set(data->TX_Speed   , MUIA_ShortHelp, GetStr(MSG_HELP_ConnectSpeed));
      set(data->TX_Provider, MUIA_ShortHelp, GetStr(MSG_HELP_Provider));
      set(data->TX_User    , MUIA_ShortHelp, GetStr(MSG_HELP_User));
      set(data->LV_Log     , MUIA_ShortHelp, GetStr(MSG_HELP_Log));

      DoMethod(obj               , MUIM_Notify, MUIA_Window_CloseRequest , TRUE , obj, 1, MUIM_MainWindow_Quit);
      DoMethod(data->LV_Providers, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime , data->PO_Provider, 2, MUIM_Popstring_Close, TRUE);
      DoMethod(data->LV_Users    , MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime , obj, 1, MUIM_MainWindow_ChangeUser);
      DoMethod(data->BT_Online   , MUIM_Notify, MUIA_Pressed             , FALSE, obj, 2, MUIM_MainWindow_OnOffline, TRUE);
      DoMethod(data->BT_Offline  , MUIM_Notify, MUIA_Pressed             , FALSE, obj, 2, MUIM_MainWindow_OnOffline, FALSE);

      // these have to be push methods so the popup can get closed first
      DoMethod(data->TX_Provider , MUIM_Notify, MUIA_Text_Contents       , MUIV_EveryTime , MUIV_Notify_Application, 4, MUIM_Application_PushMethod, obj, 1, MUIM_MainWindow_ChangeProvider);

      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT)    , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, obj, 1, MUIM_MainWindow_About);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT_MUI), MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_AboutMUI, win);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_QUIT)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, obj, 1, MUIM_MainWindow_Quit);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_GENESIS)  , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, obj, 1, MUIM_MainWindow_GenesisPrefs);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_MUI)      , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
   }
   return((ULONG)obj);
}

///
/// MainWindow_Dispatcher
SAVEDS ASM ULONG MainWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                         : return(MainWindow_New                (cl, obj, (APTR)msg));
      case OM_DISPOSE                     : return(MainWindow_Dispose            (cl, obj, (APTR)msg));
      case MUIM_MainWindow_LoadConfig     : return(MainWindow_LoadConfig         (cl, obj, (APTR)msg));
      case MUIM_MainWindow_MUIRequest     : return(MainWindow_MUIRequest         (cl, obj, (APTR)msg));
      case MUIM_MainWindow_About          : return(MainWindow_About              (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Quit           : return(MainWindow_Quit               (cl, obj, (APTR)msg));
      case MUIM_MainWindow_OnOffline      : return(MainWindow_OnOffline          (cl, obj, (APTR)msg));
      case MUIM_MainWindow_PutOnline      : return(MainWindow_PutOnline          (cl, obj, (APTR)msg));
      case MUIM_MainWindow_PutOffline     : return(MainWindow_PutOffline         (cl, obj, (APTR)msg));
      case MUIM_MainWindow_SetStates      : return(MainWindow_SetStates          (cl, obj, (APTR)msg));
      case MUIM_MainWindow_DisposeWindow  : return(MainWindow_DisposeWindow      (cl, obj, (APTR)msg));
      case MUIM_MainWindow_TimeTrigger    : return(MainWindow_TimeTrigger        (cl, obj));
      case MUIM_MainWindow_UpdateLog      : return(MainWindow_UpdateLog          (cl, obj, (APTR)msg));
      case MUIM_MainWindow_ChangeProvider : return(MainWindow_ChangeProvider     (cl, obj, (APTR)msg));
      case MUIM_MainWindow_ChangeUser     : return(MainWindow_ChangeUser         (cl, obj, (APTR)msg));
      case MUIM_MainWindow_GenesisPrefs   : return(MainWindow_GenesisPrefs       (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///
