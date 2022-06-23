/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_About.h"
#include "mui_DataBase.h"
#include "mui_Interfaces.h"
#include "mui_MainWindow.h"
#include "mui_Modems.h"
#include "mui_Options.h"
#include "mui_PasswdReq.h"
#include "mui_User.h"
#include "protos.h"
#include "mui/grouppager_mcc.h"
#include "images/information.h"
#include "images/database.h"
#include "images/options.h"
#include "images/modems.h"
#include "images/interfaces.h"
#include "images/logo.h"
#include "mcc.h"

///
/// external variables
extern struct GenesisBase *GenesisBase;
extern struct MUI_CustomClass *CL_About, *CL_Database, *CL_Interfaces, *CL_MainWindow,
                              *CL_Modems, *CL_Options, *CL_PasswdReq, *CL_User, *CL_UserWindow;
extern char config_file[];
extern BOOL changed_passwd, changed_group, changed_hosts, changed_protocols,
            changed_services, changed_inetd, changed_networks, changed_rpc, changed_inetaccess;
extern Object *win, *app;
extern struct NewMenu MainWindowMenu[];
extern struct MinList McpList;
extern struct User *current_user;

extern struct BitMapHeader information_header, interfaces_header, user_header, modems_header, options_header, database_header, logo_header, default_header;
extern ULONG information_colors[], interfaces_colors[], user_colors[], modems_colors[], options_colors[], database_colors[], logo_colors[], default_colors[];
extern UBYTE information_body[], interfaces_body[], user_body[], modems_body[], options_body[], database_body[], logo_body[], default_body[];

///
/// protos
ULONG SAVEDS ASM MCC_Query (REG(d0) LONG which);

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
/// yes_no
const STRPTR yes_no(BOOL value)
{
   return((value ? "yes" : "no"));
}

///

/// MainWindow_ClearConfig
ULONG MainWindow_ClearConfig(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data           = INST_DATA(cl                         , obj);
   struct Interfaces_Data  *ifaces_data   = INST_DATA(CL_Interfaces->mcc_Class   , data->GR_Interfaces);
   struct Modems_Data      *modems_data   = INST_DATA(CL_Modems->mcc_Class       , data->GR_Modems);
   struct Options_Data     *options_data  = INST_DATA(CL_Options->mcc_Class      , data->GR_Options);
#ifdef INTERNAL_USER
   struct User_Data        *user_data     = INST_DATA(CL_User->mcc_Class         , data->GR_User);
#endif

   DoMethod(ifaces_data->LI_Interfaces, MUIM_List_Clear);

   setcheckmark(options_data->CH_ShowOnlineTime , TRUE);
   setcheckmark(options_data->CH_ShowLamps      , TRUE);
   setcheckmark(options_data->CH_ShowConnect    , TRUE);
   setcheckmark(options_data->CH_ShowLog        , TRUE);
   setcheckmark(options_data->CH_ShowIface      , TRUE);
   setcheckmark(options_data->CH_ShowUser       , TRUE);
   setcheckmark(options_data->CH_ShowButtons    , TRUE);
   setcycle(options_data->CY_MainWindow         , 0);
   setcheckmark(options_data->CH_ShowStatusWin  , TRUE);
   setcheckmark(options_data->CH_ShowSerialInput, TRUE);
   setcheckmark(options_data->CH_ConfirmOffline , FALSE);
   setcheckmark(options_data->CH_Debug          , FALSE);
   setcheckmark(options_data->CH_FlushUserOnExit, FALSE);
   setcheckmark(options_data->CH_NoAutoTraffic  , FALSE);
   setcheckmark(options_data->CH_StartupInetd   , TRUE);
   setcheckmark(options_data->CH_StartupLoopback, TRUE);
   setcheckmark(options_data->CH_StartupTCP     , TRUE);
   setslider(options_data->SL_KernelPriority    , 5);
   setstring(options_data->STR_LogFile          , "AmiTCP:log/syslog");
   setslider(options_data->SL_LogLevel          , 0);
   setslider(options_data->SL_LogFileLevel      , 0);
   setstring(options_data->STR_Startup          , NULL);
   setstring(options_data->STR_Shutdown         , NULL);
   setinteger(options_data->STR_MBufInitial     , 2);
   setinteger(options_data->STR_MBufChunk       , 64);
   setinteger(options_data->STR_MBufClChunk     , 4);
   setinteger(options_data->STR_MBufMaxMem      , 256);
   setinteger(options_data->STR_MBufClusterSize , 2048);
   setcheckmark(options_data->CH_DebugSana      , FALSE);
   setcheckmark(options_data->CH_DebugICMP      , FALSE);
   setcheckmark(options_data->CH_DebugIP        , FALSE);
   setcheckmark(options_data->CH_BeGateway      , FALSE);
   setcheckmark(options_data->CH_IPSendRedirects, FALSE);
   setinteger(options_data->STR_TCPSendSpace    , 8192);
   setinteger(options_data->STR_TCPRecvSpace    , 8192);

   DoMethod(modems_data->LI_Modems, MUIM_List_Clear);

#ifdef INTERNAL_USER
   DoMethod(user_data->LI_User, MUIM_List_Clear);
#endif

   return(NULL);
}

///
/// MainWindow_LoadDatabase
ULONG MainWindow_LoadDatabase(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data  *data             = INST_DATA(cl                      , obj);
   struct Database_Data    *db_data          = INST_DATA(CL_Database->mcc_Class , data->GR_Database);
#ifdef INTERNAL_USER
   struct User_Data        *user_data        = INST_DATA(CL_User->mcc_Class      , data->GR_User);
#endif
   struct ParseConfig_Data pc_data;
   char buffer[41];
   STRPTR ptr;

#ifdef INTERNAL_USER
   /**** parse the passwd file ****/

   set(user_data->LI_User, MUIA_List_Quiet, TRUE);
   DoMethod(user_data->LI_User, MUIM_List_Clear);
   if(ParseConfig("AmiTCP:db/passwd", &pc_data))
   {
      struct Prefs_User *p_user;

      if(p_user = AllocVec(sizeof(struct Prefs_User), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.pc_contents, '|'))
            {
               *ptr = NULL;
               strncpy(p_user->pu_login, pc_data.pc_contents, sizeof(p_user->pu_login));
               pc_data.pc_contents = ptr + 1;
               if(ptr = strchr(pc_data.pc_contents, '|'))
               {
                  *ptr = NULL;
                  strncpy(p_user->pu_password, pc_data.pc_contents, sizeof(p_user->pu_password));
                  pc_data.pc_contents = ptr + 1;
                  if(ptr = strchr(pc_data.pc_contents, '|'))
                  {
                     *ptr = NULL;
                     p_user->pu_uid = atol(pc_data.pc_contents);
                     pc_data.pc_contents = ptr + 1;
                     if(ptr = strchr(pc_data.pc_contents, '|'))
                     {
                        *ptr = NULL;
                        p_user->pu_gid = atol(pc_data.pc_contents);
                        pc_data.pc_contents = ptr + 1;
                        if(ptr = strchr(pc_data.pc_contents, '|'))
                        {
                           *ptr = NULL;
                           strncpy(p_user->pu_realname, pc_data.pc_contents, sizeof(p_user->pu_realname));
                           pc_data.pc_contents = ptr + 1;
                           if(ptr = strchr(pc_data.pc_contents, '|'))
                           {
                              *ptr = NULL;
                              strncpy(p_user->pu_homedir, pc_data.pc_contents, sizeof(p_user->pu_homedir));
                              strncpy(p_user->pu_shell, ptr + 1, sizeof(p_user->pu_shell));
                              DoMethod(user_data->LI_User, MUIM_List_InsertSingle, p_user, MUIV_List_Insert_Bottom);
                           }
                        }
                     }
                  }
               }
            }
         }
         FreeVec(p_user);
      }
      ParseEnd(&pc_data);
   }
   set(user_data->LI_User, MUIA_List_Quiet, FALSE);
#endif

   /**** parse the group file ****/

   set(db_data->LI_Groups, MUIA_List_Quiet, TRUE);
   DoMethod(db_data->LI_Groups, MUIM_List_Clear);
   if(ParseConfig("AmiTCP:db/group", &pc_data))
   {
      struct Group *group;

      if(group = AllocVec(sizeof(struct Group), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.pc_contents, '|'))
            {
               *ptr = NULL;
               strncpy(group->Name, pc_data.pc_contents, sizeof(group->Name));
               pc_data.pc_contents = ptr + 1;
               if(ptr = strchr(pc_data.pc_contents, '|'))
               {
                  *ptr = NULL;
                  strncpy(group->Password, pc_data.pc_contents, sizeof(group->Password));
                  pc_data.pc_contents = ptr + 1;
                  if(ptr = strchr(pc_data.pc_contents, '|'))
                  {
                     *ptr = NULL;
                     group->ID = atol(pc_data.pc_contents);
                     strncpy(group->Members, ptr + 1, sizeof(group->Members));
                     DoMethod(db_data->LI_Groups, MUIM_List_InsertSingle, group, MUIV_List_Insert_Bottom);
                  }
               }
            }
         }
         FreeVec(group);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_Groups, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Database, MUIM_Database_SetStates, MUIV_Database_SetStates_Groups);

   /**** parse the protocols file ****/

   set(db_data->LI_Protocols, MUIA_List_Quiet, TRUE);
   DoMethod(db_data->LI_Protocols, MUIM_List_Clear);
   if(ParseConfig("AmiTCP:db/protocols", &pc_data))
   {
      struct Protocol *protocol;

      if(protocol = AllocVec(sizeof(struct Protocol), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.pc_contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.pc_contents, ';'))
               *ptr = NULL;

            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, protocol->Name, sizeof(protocol->Name), NULL))
            {
               pc_data.pc_contents = extract_arg(pc_data.pc_contents, buffer, sizeof(buffer), NULL);
               protocol->ID = atol(buffer);

               if(pc_data.pc_contents)
                  strncpy(protocol->Aliases, pc_data.pc_contents, sizeof(protocol->Aliases));
               else
                  *protocol->Aliases = NULL;

               DoMethod(db_data->LI_Protocols, MUIM_List_InsertSingle, protocol, MUIV_List_Insert_Bottom);
            }
         }
         FreeVec(protocol);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_Protocols, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Database, MUIM_Database_SetStates, MUIV_Database_SetStates_Protocols);


   /**** parse the services file ****/

   set(db_data->LI_Services, MUIA_List_Quiet, TRUE);
   DoMethod(db_data->LI_Services, MUIM_List_Clear);
   if(ParseConfig("AmiTCP:db/services", &pc_data))
   {
      struct Service *service;

      if(service = AllocVec(sizeof(struct Service), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.pc_contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.pc_contents, ';'))
               *ptr = NULL;

            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, service->Name, sizeof(service->Name), NULL))
            {
               if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buffer, sizeof(buffer), '/'))
               {
                  service->Port = atol(buffer);

                  if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, service->Protocol, sizeof(service->Protocol), NULL))
                     strncpy(service->Aliases, pc_data.pc_contents, sizeof(service->Aliases));
                  else
                     *service->Aliases = NULL;

                  DoMethod(db_data->LI_Services, MUIM_List_InsertSingle, service, MUIV_List_Insert_Bottom);
               }
            }
         }
         FreeVec(service);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_Services, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Database, MUIM_Database_SetStates, MUIV_Database_SetStates_Services);


   /**** parse the inet.access file ****/

   set(db_data->LI_InetAccess, MUIA_List_Quiet, TRUE);
   DoMethod(db_data->LI_InetAccess, MUIM_List_Clear);
   if(ParseConfig("AmiTCP:db/inet.access", &pc_data))
   {
      struct InetAccess *inet_access;

      if(inet_access = AllocVec(sizeof(struct InetAccess), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.pc_contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.pc_contents, ';'))
               *ptr = NULL;

            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, inet_access->Service, sizeof(inet_access->Service), NULL))
            {
               if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, inet_access->Host, sizeof(inet_access->Host), NULL))
               {
                  if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buffer, sizeof(buffer), NULL))
                  {
                     inet_access->Access = (stricmp(buffer, "allow") ? 0 : 1);
                     inet_access->Log = (stricmp(pc_data.pc_contents, "LOG") ? 0 : 1);
                  }
                  else
                  {
                     inet_access->Access = (stricmp(buffer, "allow") ? 0 : 1);
                     inet_access->Log = FALSE;
                  }
                  DoMethod(db_data->LI_InetAccess, MUIM_List_InsertSingle, inet_access, MUIV_List_Insert_Bottom);
               }
            }
         }
         FreeVec(inet_access);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_InetAccess, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Database, MUIM_Database_SetStates, MUIV_Database_SetStates_InetAccess);


   /**** parse the inetd.conf file ****/

   set(db_data->LI_Inetd, MUIA_List_Quiet, TRUE);
   DoMethod(db_data->LI_Inetd, MUIM_List_Clear);
   if(ParseConfig("AmiTCP:db/inetd.conf", &pc_data))
   {
      struct Inetd *inetd;

      if(inetd = AllocVec(sizeof(struct Inetd), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(*pc_data.pc_contents == '#')
            {
               inetd->Active = FALSE;
               pc_data.pc_contents++;
               if(*pc_data.pc_contents == ' ' || *pc_data.pc_contents == 9)
                  continue;
            }
            else
               inetd->Active = TRUE;

            if(ptr = strchr(pc_data.pc_contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.pc_contents, ';'))
               *ptr = NULL;

            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, inetd->Service, sizeof(inetd->Service), NULL))
            {
               if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buffer, sizeof(buffer), NULL))
               {
                  inetd->Socket = !stricmp(buffer, "dgram");
                  if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, inetd->Protocol, sizeof(inetd->Protocol), NULL))
                  {
                     if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buffer, sizeof(buffer), NULL))
                     {
                        inetd->Wait = (stricmp(buffer, "wait") ? (stricmp(buffer, "dos") ? 0 : 2) : 1);
                        if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, inetd->User, sizeof(inetd->User), NULL))
                        {
                           *inetd->Args = NULL;
                           *inetd->CliName = NULL;
                           if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, inetd->Server, sizeof(inetd->Server), NULL))
                           {
                              if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, inetd->CliName, sizeof(inetd->CliName), NULL))
                                 strncpy(inetd->Args, pc_data.pc_contents, sizeof(inetd->Args));
                           }

                           DoMethod(db_data->LI_Inetd, MUIM_List_InsertSingle, inetd, MUIV_List_Insert_Bottom);
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
   set(db_data->LI_Inetd, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Database, MUIM_Database_SetStates, MUIV_Database_SetStates_Inetd);


   /**** parse the hosts file ****/

   set(db_data->LI_Hosts, MUIA_List_Quiet, TRUE);
   DoMethod(db_data->LI_Hosts, MUIM_List_Clear);
   if(ParseConfig("AmiTCP:db/Hosts", &pc_data))
   {
      struct Host *host;

      if(host = AllocVec(sizeof(struct Host), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.pc_contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.pc_contents, ';'))
               *ptr = NULL;

            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, host->Addr, sizeof(host->Addr), NULL))
            {
               if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, host->Name, sizeof(host->Name), NULL))
                  strncpy(host->Aliases, pc_data.pc_contents, sizeof(host->Aliases));
               else
                  *host->Aliases = NULL;

               DoMethod(db_data->LI_Hosts, MUIM_List_InsertSingle, host, MUIV_List_Insert_Bottom);
            }
         }
         FreeVec(host);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_Hosts, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Database, MUIM_Database_SetStates, MUIV_Database_SetStates_Hosts);


   /**** parse the networks file ****/

   set(db_data->LI_Networks, MUIA_List_Quiet, TRUE);
   DoMethod(db_data->LI_Networks, MUIM_List_Clear);
   if(ParseConfig("AmiTCP:db/Networks", &pc_data))
   {
      struct DB_Network *db_network;

      if(db_network = AllocVec(sizeof(struct DB_Network), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.pc_contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.pc_contents, ';'))
               *ptr = NULL;

            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, db_network->Name, sizeof(db_network->Name), NULL))
            {
               pc_data.pc_contents = extract_arg(pc_data.pc_contents, buffer, sizeof(buffer), NULL);
               db_network->Number = atol(buffer);

               if(pc_data.pc_contents)
                  strncpy(db_network->Aliases, pc_data.pc_contents, sizeof(db_network->Aliases));
               else
                  *db_network->Aliases = NULL;

               DoMethod(db_data->LI_Networks, MUIM_List_InsertSingle, db_network, MUIV_List_Insert_Bottom);
            }
         }
         FreeVec(db_network);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_Networks, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Database, MUIM_Database_SetStates, MUIV_Database_SetStates_Networks);


   /**** parse the rpc file ****/

   set(db_data->LI_Rpcs, MUIA_List_Quiet, TRUE);
   DoMethod(db_data->LI_Rpcs, MUIM_List_Clear);
   if(ParseConfig("AmiTCP:db/rpc", &pc_data))
   {
      struct Rpc *rpc;

      if(rpc = AllocVec(sizeof(struct Rpc), MEMF_ANY))
      {
         while(ParseNextLine(&pc_data))
         {
            if(ptr = strchr(pc_data.pc_contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.pc_contents, ';'))
               *ptr = NULL;

            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, rpc->Name, sizeof(rpc->Name), NULL))
            {
               pc_data.pc_contents = extract_arg(pc_data.pc_contents, buffer, sizeof(rpc->Name), NULL);
               rpc->Number = atol(buffer);

               if(pc_data.pc_contents)
                  strncpy(rpc->Aliases, pc_data.pc_contents, sizeof(rpc->Aliases));
               else
                  *rpc->Aliases = NULL;

               DoMethod(db_data->LI_Rpcs, MUIM_List_InsertSingle, rpc, MUIV_List_Insert_Bottom);
            }
         }
         FreeVec(rpc);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_Rpcs, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Database, MUIM_Database_SetStates, MUIV_Database_SetStates_Rpcs);

   return(NULL);
}

///
/// MainWindow_LoadConfig
ULONG MainWindow_LoadConfig(struct IClass *cl, Object *obj, struct MUIP_MainWindow_LoadConfig *msg)
{
   struct MainWindow_Data *data              = INST_DATA(cl                      , obj);
   struct Interfaces_Data  *ifaces_data      = INST_DATA(CL_Interfaces->mcc_Class   , data->GR_Interfaces);
   struct Modems_Data      *modems_data      = INST_DATA(CL_Modems->mcc_Class       , data->GR_Modems);
   struct Options_Data     *options_data     = INST_DATA(CL_Options->mcc_Class      , data->GR_Options);
   struct ParseConfig_Data pc_data;
   STRPTR ptr;
   struct Interface *iface = NULL;
   struct PrefsPPPIface *ppp_if = NULL;
   struct Modem *modem = NULL;
   struct ServerEntry *server;

   if(!(server = AllocVec(sizeof(struct ServerEntry), MEMF_ANY)))
      return(NULL);

   if(ParseConfig((msg->file ? msg->file : (STRPTR)config_file), &pc_data))
   {
      if(ParseNext(&pc_data))
      {
         if(!strcmp(pc_data.pc_argument, "##") && !strcmp(pc_data.pc_contents, "This file was generated by GENESiS Prefs"))
         {
            ParseEnd(&pc_data);
            SystemTags("AmiTCP:genesis_converter", TAG_DONE);
            if(!ParseConfig((msg->file ? msg->file : (STRPTR)config_file), &pc_data))
               return(NULL);
         }
      }

      while(ParseNext(&pc_data))
      {
         if(!stricmp(pc_data.pc_argument, "MODEM"))
         {
            DoMethod(modems_data->LI_Modems, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
            DoMethod(modems_data->LI_Modems, MUIM_List_GetEntry, xget(modems_data->LI_Modems, MUIA_List_Entries) - 1 , &modem);
            if(modem)
            {
               modem->mo_flags |= MFL_7Wire | MFL_RadBoogie | MFL_DropDTR;
               modem->mo_id = 1;
               uniquify_modem_id(modems_data->LI_Modems, modem);
            }
         }
         else if(!stricmp(pc_data.pc_argument, "INTERFACE"))
         {
            DoMethod(ifaces_data->LI_Interfaces, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
            DoMethod(ifaces_data->LI_Interfaces, MUIM_List_GetEntry, xget(ifaces_data->LI_Interfaces, MUIA_List_Entries) - 1 , &iface);
            if(iface)
            {
               NewList((struct List *)&iface->if_events);
               NewList((struct List *)&iface->if_loginscript);
               NewList((struct List *)&iface->if_nameservers);
               NewList((struct List *)&iface->if_domainnames);
               ppp_if = iface->if_userdata = AllocVec(sizeof(struct PrefsPPPIface), MEMF_ANY | MEMF_CLEAR);
               iface->if_flags = NULL;
            }
         }
         else if(!stricmp(pc_data.pc_argument, "ShowOnlineTime") && is_false(&pc_data))
            setcheckmark(options_data->CH_ShowOnlineTime, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowLamps") && is_false(&pc_data))
            setcheckmark(options_data->CH_ShowLamps, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowConnect") && is_false(&pc_data))
            setcheckmark(options_data->CH_ShowConnect, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowLog") && is_false(&pc_data))
            setcheckmark(options_data->CH_ShowLog, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowInterface") && is_false(&pc_data))
            setcheckmark(options_data->CH_ShowIface, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowUser") && is_false(&pc_data))
            setcheckmark(options_data->CH_ShowUser, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowButtons") && is_false(&pc_data))
            setcheckmark(options_data->CH_ShowButtons, FALSE);
         else if(!stricmp(pc_data.pc_argument, "StartupOpenWin") && is_false(&pc_data))
            setcycle(options_data->CY_MainWindow, 2);
         else if(!stricmp(pc_data.pc_argument, "StartupIconify") && is_true(&pc_data))
            setcycle(options_data->CY_MainWindow, 1);
         else if(!stricmp(pc_data.pc_argument, "ShowStatusWin") && is_false(&pc_data))
            setcheckmark(options_data->CH_ShowStatusWin, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowSerialInput") && is_false(&pc_data))
            setcheckmark(options_data->CH_ShowSerialInput, FALSE);

         else if(!stricmp(pc_data.pc_argument, "ConfirmOffline") && is_true(&pc_data))
            setcheckmark(options_data->CH_ConfirmOffline, TRUE);
         else if(!stricmp(pc_data.pc_argument, "Debug") && is_true(&pc_data))
            setcheckmark(options_data->CH_Debug, TRUE);
         else if(!stricmp(pc_data.pc_argument, "FlushUserOnExit") && is_true(&pc_data))
            setcheckmark(options_data->CH_FlushUserOnExit, TRUE);
         else if(!stricmp(pc_data.pc_argument, "NoAutoTraffic") && is_true(&pc_data))
            setcheckmark(options_data->CH_NoAutoTraffic, TRUE);
         else if(!stricmp(pc_data.pc_argument, "StartupInetd") && is_false(&pc_data))
            setcheckmark(options_data->CH_StartupInetd, FALSE);
         else if(!stricmp(pc_data.pc_argument, "StartupLoopback") && is_false(&pc_data))
            setcheckmark(options_data->CH_StartupLoopback, FALSE);
         else if(!stricmp(pc_data.pc_argument, "StartupTCP") && is_false(&pc_data))
            setcheckmark(options_data->CH_StartupTCP, FALSE);

         else if(!stricmp(pc_data.pc_argument, "Startup") && strlen(pc_data.pc_contents) > 2)
         {
            setstring(options_data->STR_Startup, &pc_data.pc_contents[2]);
            setcycle(options_data->CY_Startup, *pc_data.pc_contents - 48);
         }
         else if(!stricmp(pc_data.pc_argument, "Shutdown") && strlen(pc_data.pc_contents) > 2)
         {
            setstring(options_data->STR_Shutdown, &pc_data.pc_contents[2]);
            setcycle(options_data->CY_Shutdown, *pc_data.pc_contents - 48);
         }
         else
         {
            if(iface && iface->if_userdata)
            {
               if(!stricmp(pc_data.pc_argument, "IfName"))
               {
                  strncpy(iface->if_name, pc_data.pc_contents, sizeof(iface->if_name));
                  uniquify_iface_name(ifaces_data->LI_Interfaces, iface);
               }
               if(!stricmp(pc_data.pc_argument, "IfComment"))
                  strncpy(iface->if_comment, pc_data.pc_contents, sizeof(iface->if_name));
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
                        STRPTR ptr2;

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
                  ReallocCopy((STRPTR *)&iface->if_configparams, pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "IPAddr"))
                  strncpy(iface->if_addr, pc_data.pc_contents, sizeof(iface->if_addr));
               else if(!stricmp(pc_data.pc_argument, "DestIP"))
                  strncpy(iface->if_dst, pc_data.pc_contents, sizeof(iface->if_dst));
               else if(!stricmp(pc_data.pc_argument, "Gateway"))
                  strncpy(iface->if_gateway, pc_data.pc_contents, sizeof(iface->if_gateway));
               else if(!stricmp(pc_data.pc_argument, "Netmask"))
                  strncpy(iface->if_netmask, pc_data.pc_contents, sizeof(iface->if_netmask));
               else if(!stricmp(pc_data.pc_argument, "MTU"))
                  iface->if_MTU = atol(pc_data.pc_contents);

               else if(!stricmp(pc_data.pc_argument, "AutoOnline") && is_true(&pc_data))
                  iface->if_flags |= IFL_AutoOnline;
               else if(!stricmp(pc_data.pc_argument, "DefaultPPP") && is_true(&pc_data))
                  iface->if_flags |= IFL_PPP;
               else if(!stricmp(pc_data.pc_argument, "DefaultSLIP") && is_true(&pc_data))
                  iface->if_flags |= IFL_SLIP;
               else if(!stricmp(pc_data.pc_argument, "UseBOOTP") && is_true(&pc_data))
                  iface->if_flags |= IFL_BOOTP;
               else if(!stricmp(pc_data.pc_argument, "GetTime") && is_true(&pc_data))
                  iface->if_flags |= IFL_GetTime;
               else if(!stricmp(pc_data.pc_argument, "SaveTime") && is_true(&pc_data))
                  iface->if_flags |= IFL_SaveTime;
               else if(!stricmp(pc_data.pc_argument, "UseDomainName") && is_true(&pc_data))
                  iface->if_flags |= IFL_UseDomainName;
               else if(!stricmp(pc_data.pc_argument, "UseHostName") && is_true(&pc_data))
                  iface->if_flags |= IFL_UseHostName;
               else if(!stricmp(pc_data.pc_argument, "UseNameServer") && is_true(&pc_data))
                  iface->if_flags |= IFL_UseNameServer;
               else if(!stricmp(pc_data.pc_argument, "DialIn") && is_true(&pc_data))
                  iface->if_flags |= IFL_DialIn;
               else if(!stricmp(pc_data.pc_argument, "KeepAlive"))
                  iface->if_keepalive = atol(pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "TimeServer"))
                  strncpy(iface->if_timename, pc_data.pc_contents, sizeof(iface->if_timename));

               else if(!stricmp(pc_data.pc_argument, "CarrierDetect") && is_true(&pc_data) && ppp_if)
                  ppp_if->ppp_carrierdetect = TRUE;
               else if(!stricmp(pc_data.pc_argument, "ConnectTimeout") && ppp_if)
                  ppp_if->ppp_connecttimeout = atol(pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "Callback") && ppp_if)
                  strncpy(ppp_if->ppp_callback, pc_data.pc_contents, sizeof(ppp_if->ppp_callback));
               else if(!stricmp(pc_data.pc_argument, "MPPCompression") && is_true(&pc_data) && ppp_if)
                  ppp_if->ppp_mppcomp = TRUE;
               else if(!stricmp(pc_data.pc_argument, "VJCompression") && is_true(&pc_data) && ppp_if)
                  ppp_if->ppp_vjcomp = TRUE;
               else if(!stricmp(pc_data.pc_argument, "BSDCompression") && is_true(&pc_data) && ppp_if)
                  ppp_if->ppp_bsdcomp = TRUE;
               else if(!stricmp(pc_data.pc_argument, "DeflateCompression") && is_true(&pc_data) && ppp_if)
                  ppp_if->ppp_deflatecomp = TRUE;
               else if(!stricmp(pc_data.pc_argument, "EOF") && is_true(&pc_data) && ppp_if)
                  ppp_if->ppp_eof = TRUE;

               else if(!stricmp(pc_data.pc_argument, "IfModemID"))
                  iface->if_modemid = atol(pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "Login"))
                  strncpy(iface->if_login, pc_data.pc_contents, sizeof(iface->if_login));
               else if(!stricmp(pc_data.pc_argument, "Password"))
                  decrypt(pc_data.pc_contents, iface->if_password);
               else if(!stricmp(pc_data.pc_argument, "Phone"))
                  strncpy(iface->if_phonenumber, pc_data.pc_contents, sizeof(iface->if_phonenumber));
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_Send]))
                  add_script_line(&iface->if_loginscript, SL_Send, pc_data.pc_contents, NULL);
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_WaitFor]))
                  add_script_line(&iface->if_loginscript, SL_WaitFor, pc_data.pc_contents, NULL);
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_Dial]))
                  add_script_line(&iface->if_loginscript, SL_Dial, NULL, NULL);
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_GoOnline]))
                  add_script_line(&iface->if_loginscript, SL_GoOnline, NULL, NULL);
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_SendLogin]))
                  add_script_line(&iface->if_loginscript, SL_SendLogin, NULL, NULL);
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_SendPassword]))
                  add_script_line(&iface->if_loginscript, SL_SendPassword, NULL, NULL);
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_SendBreak]))
                  add_script_line(&iface->if_loginscript, SL_SendBreak, NULL, NULL);
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_Exec]))
                  add_script_line(&iface->if_loginscript, SL_Exec, NULL, NULL);
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_Pause]))
                  add_script_line(&iface->if_loginscript, SL_Pause, NULL, NULL);
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_ParseIP]))
                  add_script_line(&iface->if_loginscript, SL_ParseIP, NULL, NULL);

               else if(!stricmp(pc_data.pc_argument, event_commands[IFE_Online]) && (strlen(pc_data.pc_contents) > 2))
                  add_script_line(&iface->if_events, IFE_Online, &pc_data.pc_contents[2], *pc_data.pc_contents - 48);
               else if(!stricmp(pc_data.pc_argument, event_commands[IFE_OnlineFail]) && (strlen(pc_data.pc_contents) > 2))
                  add_script_line(&iface->if_events, IFE_OnlineFail, &pc_data.pc_contents[2], *pc_data.pc_contents - 48);
               else if(!stricmp(pc_data.pc_argument, event_commands[IFE_OfflineActive]) && (strlen(pc_data.pc_contents) > 2))
                  add_script_line(&iface->if_events, IFE_OfflineActive, &pc_data.pc_contents[2], *pc_data.pc_contents - 48);
               else if(!stricmp(pc_data.pc_argument, event_commands[IFE_OfflinePassive]) && (strlen(pc_data.pc_contents) > 2))
                  add_script_line(&iface->if_events, IFE_OfflinePassive, &pc_data.pc_contents[2], *pc_data.pc_contents - 48);

               else if(!stricmp(pc_data.pc_argument, "NameServer"))
                  add_server(&iface->if_nameservers, pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "DomainName"))
                  add_server(&iface->if_domainnames, pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "HostName"))
                  strncpy(iface->if_hostname, pc_data.pc_contents, sizeof(iface->if_hostname));
            }
            if(modem)  // no else ! oterwise it wouldn't check if iface != 0 !!
            {
               if(!stricmp(pc_data.pc_argument, "ModemID"))
               {
                  modem->mo_id = atol(pc_data.pc_contents);
                  if(modem->mo_id < 1)
                     modem->mo_id = 1;
                  uniquify_modem_id(modems_data->LI_Modems, modem);
               }
               else if(!stricmp(pc_data.pc_argument, "ModemName"))
                  strncpy(modem->mo_name, pc_data.pc_contents, sizeof(modem->mo_name));
               else if(!stricmp(pc_data.pc_argument, "ModemComment"))
                  strncpy(modem->mo_comment, pc_data.pc_contents, sizeof(modem->mo_comment));

               if(!stricmp(pc_data.pc_argument, "SerialDevice"))
                  strncpy(modem->mo_device, pc_data.pc_contents, sizeof(modem->mo_device));
               else if(!stricmp(pc_data.pc_argument, "SerialUnit"))
                  modem->mo_unit = atol(pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "BaudRate"))
                  modem->mo_baudrate = atol(pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "SerBufLen"))
                  modem->mo_serbuflen = atol(pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "IgnoreDSR") && is_true(&pc_data))
                  modem->mo_flags |= MFL_IgnoreDSR;
               else if(!stricmp(pc_data.pc_argument, "7Wire") && is_false(&pc_data))
                  modem->mo_flags &= ~MFL_7Wire;
               else if(!stricmp(pc_data.pc_argument, "RadBoogie") && is_false(&pc_data))
                  modem->mo_flags &= ~MFL_RadBoogie;
               else if(!stricmp(pc_data.pc_argument, "XonXoff") && is_true(&pc_data))
                  modem->mo_flags |= MFL_XonXoff;
               else if(!stricmp(pc_data.pc_argument, "OwnDevUnit") && is_true(&pc_data))
                  modem->mo_flags |= MFL_OwnDevUnit;
               else if(!stricmp(pc_data.pc_argument, "DropDTR") && is_false(&pc_data))
                  modem->mo_flags &= ~MFL_DropDTR;

               else if(!stricmp(pc_data.pc_argument, "InitString"))
                  strncpy(modem->mo_init, pc_data.pc_contents, sizeof(modem->mo_init));
               else if(!stricmp(pc_data.pc_argument, "DialPrefix"))
                  strncpy(modem->mo_dialprefix, pc_data.pc_contents, sizeof(modem->mo_dialprefix));
               else if(!stricmp(pc_data.pc_argument, "DialSuffix"))
                  strncpy(modem->mo_dialsuffix, pc_data.pc_contents, sizeof(modem->mo_dialsuffix));
               else if(!stricmp(pc_data.pc_argument, "Answer"))
                  strncpy(modem->mo_answer, pc_data.pc_contents, sizeof(modem->mo_answer));
               else if(!stricmp(pc_data.pc_argument, "Hangup"))
                  strncpy(modem->mo_hangup, pc_data.pc_contents, sizeof(modem->mo_hangup));

               else if(!stricmp(pc_data.pc_argument, "Ring"))
                  strncpy(modem->mo_ring, pc_data.pc_contents, sizeof(modem->mo_ring));
               else if(!stricmp(pc_data.pc_argument, "Connect"))
                  strncpy(modem->mo_connect, pc_data.pc_contents, sizeof(modem->mo_connect));
               else if(!stricmp(pc_data.pc_argument, "NoCarrier"))
                  strncpy(modem->mo_nocarrier, pc_data.pc_contents, sizeof(modem->mo_nocarrier));
               else if(!stricmp(pc_data.pc_argument, "NoDialtone"))
                  strncpy(modem->mo_nodialtone, pc_data.pc_contents, sizeof(modem->mo_nodialtone));
               else if(!stricmp(pc_data.pc_argument, "Busy"))
                  strncpy(modem->mo_busy, pc_data.pc_contents, sizeof(modem->mo_busy));
               else if(!stricmp(pc_data.pc_argument, "Ok"))
                  strncpy(modem->mo_ok, pc_data.pc_contents, sizeof(modem->mo_ok));
               else if(!stricmp(pc_data.pc_argument, "Error"))
                  strncpy(modem->mo_error, pc_data.pc_contents, sizeof(modem->mo_error));

               else if(!stricmp(pc_data.pc_argument, "RedialAttempts"))
                  modem->mo_redialattempts = atol(pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "RedialDelay"))
                  modem->mo_redialdelay = atol(pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "CommandDelay"))
                  modem->mo_commanddelay = atol(pc_data.pc_contents);
            }
         }
      }
      ParseEnd(&pc_data);
   }

   if(ParseConfig("AmiTCP:db/AmiTCP.config", &pc_data))
   {
      char buf[81];

      while(ParseNext(&pc_data))
      {
         if(!stricmp(pc_data.pc_argument, "PRIORITY"))
         {
            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buf, sizeof(buf), '='))
               setslider(options_data->SL_KernelPriority, atol(pc_data.pc_contents));
         }
         else if(!stricmp(pc_data.pc_argument, "MBUF_CONF"))
         {
            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buf, sizeof(buf), '='))
            {
               if(ptr = strchr(buf, ' '))
                  *ptr = NULL;
               if(!stricmp(buf, "Initial"))
                  setstring(options_data->STR_MBufInitial, pc_data.pc_contents);
               else if(!stricmp(buf, "Chunk"))
                  setstring(options_data->STR_MBufChunk, pc_data.pc_contents);
               else if(!stricmp(buf, "CLChunk"))
                  setstring(options_data->STR_MBufClChunk, pc_data.pc_contents);
               else if(!stricmp(buf, "MaxMem"))
                  setstring(options_data->STR_MBufMaxMem, pc_data.pc_contents);
               else if(!stricmp(buf, "ClusterSize"))
                  setstring(options_data->STR_MBufClusterSize, pc_data.pc_contents);
            }
         }
         else if(!stricmp(pc_data.pc_argument, "LOG"))
         {
            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buf, sizeof(buf), '='))
            {
               if(ptr = strchr(buf, ' '))
                  *ptr = NULL;
               if(!stricmp(buf, "FilterCons"))
                  setslider(options_data->SL_LogFileLevel, 7 - (atol(pc_data.pc_contents) * 10));
               else if(!stricmp(buf, "FilterFile"))
                  setslider(options_data->SL_LogLevel, 7 - (atol(pc_data.pc_contents) * 10));
            }
         }
         else if(!stricmp(pc_data.pc_argument, "DebugSana"))
         {
            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buf, sizeof(buf), '='))
               setcheckmark(options_data->CH_DebugSana, !stricmp(pc_data.pc_contents, "YES"));
         }
         else if(!stricmp(pc_data.pc_argument, "DebugICMP"))
         {
            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buf, sizeof(buf), '='))
               setcheckmark(options_data->CH_DebugICMP, !stricmp(pc_data.pc_contents, "YES"));
         }
         else if(!stricmp(pc_data.pc_argument, "DebugIP"))
         {
            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buf, sizeof(buf), '='))
               setcheckmark(options_data->CH_DebugIP, !stricmp(pc_data.pc_contents, "YES"));
         }
         else if(!stricmp(pc_data.pc_argument, "Gateway"))
         {
            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buf, sizeof(buf), '='))
               setcheckmark(options_data->CH_BeGateway, !stricmp(pc_data.pc_contents, "YES"));
         }
         else if(!stricmp(pc_data.pc_argument, "IPSendRedirects"))
         {
            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buf, sizeof(buf), '='))
               setcheckmark(options_data->CH_IPSendRedirects, !stricmp(pc_data.pc_contents, "YES"));
         }
         else if(!stricmp(pc_data.pc_argument, "TCP_SendSpace"))
         {
            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buf, sizeof(buf), '='))
               setstring(options_data->STR_TCPSendSpace, pc_data.pc_contents);
         }
         else if(!stricmp(pc_data.pc_argument, "TCP_RecvSpace"))
         {
            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, buf, sizeof(buf), '='))
               setstring(options_data->STR_TCPRecvSpace, pc_data.pc_contents);
         }
         else if(!stricmp(pc_data.pc_argument, "ConsoleName =")) // logfile
            setstring(options_data->STR_LogFile, pc_data.pc_contents);
      }
      ParseEnd(&pc_data);
   }


   FreeVec(server);

   return(NULL);
}

///
/// MainWindow_SaveConfig
ULONG MainWindow_SaveConfig(struct IClass *cl, Object *obj, struct MUIP_MainWindow_SaveConfig *msg)
{
   struct MainWindow_Data  *data          = INST_DATA(cl                      , obj);
   struct Interfaces_Data  *ifaces_data   = INST_DATA(CL_Interfaces->mcc_Class, data->GR_Interfaces);
   struct Modems_Data      *modems_data   = INST_DATA(CL_Modems->mcc_Class    , data->GR_Modems);
   struct Options_Data     *options_data  = INST_DATA(CL_Options->mcc_Class   , data->GR_Options);
   struct Database_Data    *db_data       = INST_DATA(CL_Database->mcc_Class  , data->GR_Database);
#ifdef INTERNAL_USER
   struct User_Data        *user_data     = INST_DATA(CL_User->mcc_Class      , data->GR_User);
#endif
   struct Interface *iface = NULL;
   struct ServerEntry *server;
   struct ScriptLine *script_line;
   struct PrefsPPPIface *ppp_if;
   struct Modem *modem;
   BPTR fh;
   char buff[110];
   STRPTR ptr;
   LONG pos;

   // save order is: services, passwd, group, genesis.conf (which initiates a RESET), inetd.conf

   if(fh = Open("AmiTCP:db/AmiTCP.config", MODE_NEWFILE))
   {
      FPrintf(fh, "## This file was generated by GENESiSPrefs " VERSIONSTRING "\n##\n\n");

      FPrintf(fh, "PRIORITY = %ld\n\n", xget(options_data->SL_KernelPriority, MUIA_Numeric_Value));
      FPrintf(fh, "MBUF_CONF Initial =     %4ls\n", xget(options_data->STR_MBufInitial, MUIA_String_Contents));
      FPrintf(fh, "MBUF_CONF Chunk =       %4ls\n", xget(options_data->STR_MBufChunk, MUIA_String_Contents));
      FPrintf(fh, "MBUF_CONF ClChunk =     %4ls\n", xget(options_data->STR_MBufClChunk, MUIA_String_Contents));
      FPrintf(fh, "MBUF_CONF MaxMem =      %4ls\n", xget(options_data->STR_MBufMaxMem, MUIA_String_Contents));
      FPrintf(fh, "MBUF_CONF ClusterSize = %4ls\n\n", xget(options_data->STR_MBufClusterSize, MUIA_String_Contents));
      FPrintf(fh, ";LOG Count =     4\n");
      FPrintf(fh, ";LOG Len =     128\n");
      FPrintf(fh, ";LOG Filter =    7\n");
      FPrintf(fh, "LOG FilterCons = %ld\n", 7 - (xget(options_data->SL_LogFileLevel, MUIA_Numeric_Value) / 10));
      FPrintf(fh, "LOG FilterFile = %ld\n\n", 7 - (xget(options_data->SL_LogLevel, MUIA_Numeric_Value) / 10));
      FPrintf(fh, "DebugSana = %ls\n", (xget(options_data->CH_DebugSana , MUIA_Selected) ? "YES" : "NO"));
      FPrintf(fh, "DebugICMP = %ls\n", (xget(options_data->CH_DebugICMP , MUIA_Selected) ? "YES" : "NO"));
      FPrintf(fh, "DebugIP =   %ls\n", (xget(options_data->CH_DebugIP   , MUIA_Selected) ? "YES" : "NO"));
      FPrintf(fh, "Gateway =   %ls\n", (xget(options_data->CH_BeGateway , MUIA_Selected) ? "YES" : "NO"));
      FPrintf(fh, "IPSendRedirects = %ls\n", (xget(options_data->CH_IPSendRedirects, MUIA_Selected) ? "YES" : "NO"));
      FPrintf(fh, ";UseNameServer = SECOND\n");
      FPrintf(fh, ";UseLoopBack = YES\n\n");
      FPrintf(fh, "TCP_SendSpace = %ls\n", xget(options_data->STR_TCPSendSpace, MUIA_String_Contents));
      FPrintf(fh, "TCP_RecvSpace = %ls\n\n", xget(options_data->STR_TCPRecvSpace, MUIA_String_Contents));
      FPrintf(fh, "ConsoleName = \"%ls\"\n", xget(options_data->STR_LogFile, MUIA_String_Contents));
      FPrintf(fh, "LogFileName = \"t:AmiTCP.log\"\n");
      Close(fh);
   }



   if(changed_services)
   {
      if(fh = Open("AmiTCP:db/services", MODE_NEWFILE))
      {
         struct Service *service;

         FPrintf(fh, "## This file was generated by GENESiSPrefs " VERSIONSTRING "\n##\n");
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
      changed_services = FALSE;
   }

   // tell external classes to save their config

   if(McpList.mlh_TailPred != (struct MinNode *)&McpList)
   {
      struct McpNode *node;

      node = (struct McpNode *)McpList.mlh_Head;
      while(node->mcp_node.mln_Succ)
      {
         DoMethod(node->mcp_object, MUIM_Settingsgroup_GadgetsToConfig, current_user);
         node = (struct McpNode *)node->mcp_node.mln_Succ;
      }
   }

#ifdef INTERNAL_USER
   if(changed_passwd)
   {
      if(fh = Open("AmiTCP:db/passwd", MODE_NEWFILE))
      {
         struct Prefs_User *p_user;

         pos = 0;
         FOREVER
         {
            DoMethod(user_data->LI_User, MUIM_List_GetEntry, pos++, &p_user);
            if(!p_user)
               break;
            FPrintf(fh, "%ls|%ls|%ld|%ld|%ls|%ls|%ls\n", p_user->pu_login, p_user->pu_password, p_user->pu_uid, p_user->pu_gid, p_user->pu_realname, p_user->pu_homedir, p_user->pu_shell);
         }
         Close(fh);
      }
      changed_passwd = FALSE;
   }
#endif

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
            FPrintf(fh, "%ls|%ls|%ld|%ls\n", group->Name, "*", group->ID, group->Members);
         }
         Close(fh);
      }
      changed_group = FALSE;
   }

   if(fh = Open((msg->file ? msg->file : (STRPTR)config_file), MODE_NEWFILE))
   {
      FPrintf(fh, "## This file was generated by GENESiSPrefs " VERSIONSTRING "\n##\n\n");

      FPrintf(fh, "ShowOnlineTime      %ls\n", yes_no(xget(options_data->CH_ShowOnlineTime , MUIA_Selected)));
      FPrintf(fh, "ShowLamps           %ls\n", yes_no(xget(options_data->CH_ShowLamps      , MUIA_Selected)));
      FPrintf(fh, "ShowConnect         %ls\n", yes_no(xget(options_data->CH_ShowConnect    , MUIA_Selected)));
      FPrintf(fh, "ShowLog             %ls\n", yes_no(xget(options_data->CH_ShowLog        , MUIA_Selected)));
      FPrintf(fh, "ShowInterface       %ls\n", yes_no(xget(options_data->CH_ShowIface      , MUIA_Selected)));
      FPrintf(fh, "ShowUser            %ls\n", yes_no(xget(options_data->CH_ShowUser       , MUIA_Selected)));
      FPrintf(fh, "ShowButtons         %ls\n", yes_no(xget(options_data->CH_ShowButtons    , MUIA_Selected)));
      switch(xget(options_data->CY_MainWindow, MUIA_Cycle_Active))
      {
         case 1:
            FPrintf(fh, "StartupIconify      yes\n");
            break;
         case 2:
            FPrintf(fh, "StartupOpenWin      no\n");
            break;
      }
      FPrintf(fh, "ShowStatusWin       %ls\n", yes_no(xget(options_data->CH_ShowStatusWin  , MUIA_Selected)));
      FPrintf(fh, "ShowSerialInput     %ls\n", yes_no(xget(options_data->CH_ShowSerialInput, MUIA_Selected)));
      FPrintf(fh, "ConfirmOffline      %ls\n", yes_no(xget(options_data->CH_ConfirmOffline , MUIA_Selected)));
      FPrintf(fh, "Debug               %ls\n", yes_no(xget(options_data->CH_Debug          , MUIA_Selected)));
      FPrintf(fh, "FlushUserOnExit     %ls\n", yes_no(xget(options_data->CH_FlushUserOnExit, MUIA_Selected)));
      FPrintf(fh, "NoAutoTraffic       %ls\n", yes_no(xget(options_data->CH_NoAutoTraffic  , MUIA_Selected)));
      FPrintf(fh, "StartupInetd        %ls\n", yes_no(xget(options_data->CH_StartupInetd  , MUIA_Selected)));
      FPrintf(fh, "StartupLoopback     %ls\n", yes_no(xget(options_data->CH_StartupLoopback, MUIA_Selected)));
      FPrintf(fh, "StartupTCP          %ls\n\n", yes_no(xget(options_data->CH_StartupTCP, MUIA_Selected)));
      if(strlen((STRPTR)xget(options_data->STR_Startup   , MUIA_String_Contents)))
         FPrintf(fh, "Startup             %ld %ls\n", xget(options_data->CY_Startup , MUIA_Cycle_Active), xget(options_data->STR_Startup , MUIA_String_Contents));
      if(strlen((STRPTR)xget(options_data->STR_Shutdown  , MUIA_String_Contents)))
         FPrintf(fh, "Shutdown            %ld %ls\n", xget(options_data->CY_Shutdown, MUIA_Cycle_Active), xget(options_data->STR_Shutdown, MUIA_String_Contents));

      pos = 0;
      FOREVER
      {
         DoMethod(ifaces_data->LI_Interfaces, MUIM_List_GetEntry, pos++, &iface);
         if(!iface)
            break;

         ppp_if = iface->if_userdata;
         uniquify_iface_name(ifaces_data->LI_Interfaces, iface);
         modem = get_modem_by_id(modems_data->LI_Modems, iface->if_modemid);

         FPrintf(fh, "\nINTERFACE\n");
         FPrintf(fh, "IfName              %ls\n", (*iface->if_name ? iface->if_name : "xxx"));
         if(*iface->if_comment)
            FPrintf(fh, "IfComment           \"%ls\"\n", iface->if_comment);
         FPrintf(fh, "Sana2Device         %ls\n", iface->if_sana2device);
         FPrintf(fh, "Sana2Unit           %ld\n", iface->if_sana2unit);
         if(*iface->if_sana2config)
            FPrintf(fh, "Sana2Config         %ls\n", iface->if_sana2config);
         if(iface->if_flags & IFL_PPP)
         {
            FPrintf(fh, "Sana2ConfigText     \"");
            if(modem)
            {
               FPrintf(fh, "sername %ls\\nserunit %ld\\nserbaud %ld\\nlocalipaddress %%a\\n",
               modem->mo_device, modem->mo_unit, modem->mo_baudrate);
               FPrintf(fh, "sevenwire      %ld\\n", (modem->mo_flags & MFL_7Wire ? 1 : 0));
               FPrintf(fh, "xonxoff        %ld\\n", (modem->mo_flags & MFL_XonXoff ? 1 : 0));
            }
            if(*iface->if_login)
               FPrintf(fh, "user \"%%u\"\\n");
            if(*iface->if_password)
               FPrintf(fh, "secret_secret \"%%p\"\\n");
            if(xget(options_data->CH_Debug, MUIA_Selected))
               FPrintf(fh, "debug = 1\\ndebug_window = 1\\nerror_requesters = 0\\nlog_file = t:appp.log\\n");
            if(ppp_if)
            {
               if(ppp_if->ppp_connecttimeout)
                  FPrintf(fh, "connecttimeout %ld\\n", ppp_if->ppp_connecttimeout);
               if(*ppp_if->ppp_callback)
                  FPrintf(fh, "callback       \"%ls\"\\n", ppp_if->ppp_callback);
               FPrintf(fh, "cd             %ls\\n", yes_no(ppp_if->ppp_carrierdetect));
               FPrintf(fh, "mppcomp        %ls\\n", yes_no(ppp_if->ppp_mppcomp));
               FPrintf(fh, "vjcomp         %ls\\n", yes_no(ppp_if->ppp_vjcomp));
               FPrintf(fh, "bsdcomp        %ls\\n", yes_no(ppp_if->ppp_bsdcomp));
               FPrintf(fh, "deflatecomp    %ls\\n", yes_no(ppp_if->ppp_deflatecomp));
               FPrintf(fh, "eof            %ls\\n", yes_no(ppp_if->ppp_eof));
            }
            FPrintf(fh, "\"\n");
         }
         else if(iface->if_flags & IFL_SLIP)
         {
            FPrintf(fh, "Sana2ConfigText     \"");
            if(modem)
            {
               FPrintf(fh, "%ls %ld %ld Shared %%a MTU=%ld",
                  modem->mo_device, modem->mo_unit, modem->mo_baudrate, iface->if_MTU);
               if(modem->mo_flags & MFL_7Wire)
                  FPrintf(fh, " 7Wire");
            }
            if(ppp_if)
            {
               if(ppp_if->ppp_carrierdetect)
                  FPrintf(fh, " CD");
            }
            FPrintf(fh, "\"\n");
         }
         else if(iface->if_sana2configtext && *iface->if_sana2configtext)
         {
            FPrintf(fh, "Sana2ConfigText     \"");

            ptr = iface->if_sana2configtext;
            while(*ptr)
            {
               if(*ptr == '\n')
                  FPrintf(fh, "\\n");
               else
                  FPrintf(fh, "%lc", *ptr);
               ptr++;
            }
            FPrintf(fh, "\"\n");
         }
         if(iface->if_configparams && *iface->if_configparams)
            FPrintf(fh, "IfConfigParams      \"%ls\"\n", iface->if_configparams);
         if(*iface->if_addr)
            FPrintf(fh, "IPAddr              %ls\n", iface->if_addr);
         if(*iface->if_dst)
            FPrintf(fh, "DestIP              %ls\n", iface->if_dst);
         if(*iface->if_gateway)
            FPrintf(fh, "Gateway             %ls\n", iface->if_gateway);
         if(*iface->if_netmask)
            FPrintf(fh, "Netmask             %ls\n", iface->if_netmask);
         FPrintf(fh, "MTU                 %ld\n", iface->if_MTU);

         if(iface->if_flags & IFL_AutoOnline)
            FPrintf(fh, "AutoOnline\n");
         if(iface->if_flags & IFL_PPP)
            FPrintf(fh, "DefaultPPP\n");
         if(iface->if_flags & IFL_SLIP)
            FPrintf(fh, "DefaultSLIP\n");
         if(iface->if_flags & IFL_BOOTP)
            FPrintf(fh, "UseBOOTP\n");
         if(iface->if_flags & IFL_GetTime)
            FPrintf(fh, "GetTime\n");
         if(iface->if_flags & IFL_SaveTime)
            FPrintf(fh, "SaveTime\n");
         if(*iface->if_timename)
            FPrintf(fh, "TimeServer          %ls\n", iface->if_timename);
         if(iface->if_flags & IFL_UseDomainName)
            FPrintf(fh, "UseDomainName\n");
         if(iface->if_flags & IFL_UseHostName)
            FPrintf(fh, "UseHostName\n");
         if(iface->if_flags & IFL_UseNameServer)
            FPrintf(fh, "UseNameServer\n");
         if(iface->if_flags & IFL_DialIn)
            FPrintf(fh, "DialIn\n");
         if(iface->if_keepalive)
            FPrintf(fh, "KeepAlive           %ld\n", iface->if_keepalive);

         if(ppp_if && ((iface->if_flags & IFL_PPP) || (iface->if_flags & IFL_SLIP)))
         {
            FPrintf(fh, "CarrierDetect       %ls\n", yes_no(ppp_if->ppp_carrierdetect));
            if(iface->if_flags & IFL_PPP)
            {
               if(ppp_if->ppp_connecttimeout)
                  FPrintf(fh, "ConnectTimeout   %ld\n", ppp_if->ppp_connecttimeout);
               if(*ppp_if->ppp_callback)
                  FPrintf(fh, "Callback         \"%ls\"\n", ppp_if->ppp_callback);
               FPrintf(fh, "MPPCompression      %ls\n", yes_no(ppp_if->ppp_mppcomp));
               FPrintf(fh, "VJCompression       %ls\n", yes_no(ppp_if->ppp_vjcomp));
               FPrintf(fh, "BSDCompression      %ls\n", yes_no(ppp_if->ppp_bsdcomp));
               FPrintf(fh, "DeflateCompression  %ls\n", yes_no(ppp_if->ppp_deflatecomp));
               FPrintf(fh, "EOF                 %ls\n", yes_no(ppp_if->ppp_eof));
            }
         }

         if(*iface->if_hostname)
            FPrintf(fh, "HostName            %ls\n", iface->if_hostname);
         if(iface->if_nameservers.mlh_TailPred != (struct MinNode *)&iface->if_nameservers)
         {
            server = (struct ServerEntry *)iface->if_nameservers.mlh_Head;
            while(server->se_node.mln_Succ)
            {
               FPrintf(fh, "NameServer          %ls\n", server->se_name);
               server = (struct ServerEntry *)server->se_node.mln_Succ;
            }
         }
         if(iface->if_domainnames.mlh_TailPred != (struct MinNode *)&iface->if_domainnames)
         {
            server = (struct ServerEntry *)iface->if_domainnames.mlh_Head;
            while(server->se_node.mln_Succ)
            {
               FPrintf(fh, "DomainName          %ls\n", server->se_name);
               server = (struct ServerEntry *)server->se_node.mln_Succ;
            }
         }

         if(iface->if_modemid > 0)   // operates over serial line
         {
            FPrintf(fh, "IfModemID           %ld\n", iface->if_modemid);
            if(*iface->if_login)
               FPrintf(fh, "Login               \"%ls\"\n", iface->if_login);
            if(*iface->if_password)
            {
               encrypt(iface->if_password, buff);
               FPrintf(fh, "Password            \"%ls\"\n", buff);
            }
            if(*iface->if_phonenumber)
               FPrintf(fh, "Phone               \"%ls\"\n", iface->if_phonenumber);

            if(iface->if_loginscript.mlh_TailPred != (struct MinNode *)&iface->if_loginscript)
            {
               FPrintf(fh, "\nLOGINSCRIPT\n");
               script_line = (struct ScriptLine *)iface->if_loginscript.mlh_Head;
               while(script_line->sl_node.mln_Succ)
               {
                  if(*script_line->sl_contents)
                     FPrintf(fh, "%-19ls \"%ls\"\n", script_commands[script_line->sl_command], script_line->sl_contents);
                  else
                     FPrintf(fh, "%ls\n", script_commands[script_line->sl_command]);

                  script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
               }
            }
         }

         if(iface->if_events.mlh_TailPred != (struct MinNode *)&iface->if_events)
         {
            FPrintf(fh, "\nEVENTS\n");
            script_line = (struct ScriptLine *)iface->if_events.mlh_Head;
            while(script_line->sl_node.mln_Succ)
            {
               FPrintf(fh, "%-19ls \"%ld %ls\"\n", event_commands[script_line->sl_command], script_line->sl_userdata, script_line->sl_contents);
               script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
            }
         }
      }

      pos = 0;
      FOREVER
      {
         DoMethod(modems_data->LI_Modems, MUIM_List_GetEntry, pos++, &modem);
         if(!modem)
            break;

         FPrintf(fh, "\nMODEM\n");
         FPrintf(fh, "ModemID             %ld\n", modem->mo_id);
         if(*modem->mo_name)
            FPrintf(fh, "ModemName           \"%ls\"\n", modem->mo_name);
         if(*modem->mo_comment)
            FPrintf(fh, "ModemComment        \"%ls\"\n", modem->mo_comment);

         FPrintf(fh, "SerialDevice        %ls\n", modem->mo_device);
         FPrintf(fh, "SerialUnit          %ld\n", modem->mo_unit);
         FPrintf(fh, "BaudRate            %ld\n", modem->mo_baudrate);
         FPrintf(fh, "SerBufLen           %ld\n", modem->mo_serbuflen);
         if(modem->mo_flags & MFL_IgnoreDSR)
            FPrintf(fh, "IgnoreDSR\n");
         if(!(modem->mo_flags & MFL_7Wire))
               FPrintf(fh, "7Wire               no\n");
         if(!(modem->mo_flags & MFL_RadBoogie))
               FPrintf(fh, "RadBoogie           no\n");
         if(!(modem->mo_flags & MFL_DropDTR))
               FPrintf(fh, "DropDTR             no\n");
         if(modem->mo_flags & MFL_XonXoff)
               FPrintf(fh, "XonXoff\n");
         if(modem->mo_flags & MFL_OwnDevUnit)
               FPrintf(fh, "OwnDevUnit\n");

         if(*modem->mo_init)
            FPrintf(fh, "InitString          \"%ls\"\n", modem->mo_init);
         if(*modem->mo_dialprefix)
            FPrintf(fh, "DialPrefix          \"%ls\"\n", modem->mo_dialprefix);
         if(*modem->mo_dialsuffix)
            FPrintf(fh, "DialSuffix          \"%ls\"\n", modem->mo_dialsuffix);
         if(*modem->mo_answer)
            FPrintf(fh, "Answer              \"%ls\"\n", modem->mo_answer);
         if(*modem->mo_hangup)
            FPrintf(fh, "Hangup              \"%ls\"\n", modem->mo_hangup);
         FPrintf(fh, "RedialAttempts      %ld\n", modem->mo_redialattempts);
         FPrintf(fh, "RedialDelay         %ld\n", modem->mo_redialdelay);
         FPrintf(fh, "CommandDelay        %ld\n", modem->mo_commanddelay);

         if(*modem->mo_ring)
            FPrintf(fh, "Ring                \"%ls\"\n", modem->mo_ring);
         if(*modem->mo_connect)
            FPrintf(fh, "Connect             \"%ls\"\n", modem->mo_connect);
         if(*modem->mo_nocarrier)
            FPrintf(fh, "NoCarrier           \"%ls\"\n", modem->mo_nocarrier);
         if(*modem->mo_nodialtone)
            FPrintf(fh, "NoDialtone          \"%ls\"\n", modem->mo_nodialtone);
         if(*modem->mo_busy)
            FPrintf(fh, "Busy                \"%ls\"\n", modem->mo_busy);
         if(*modem->mo_ok)
            FPrintf(fh, "Ok                  \"%ls\"\n", modem->mo_ok);
         if(*modem->mo_error)
            FPrintf(fh, "Error               \"%ls\"\n", modem->mo_error);
      }
      Close(fh);
   }

   if(changed_hosts)
   {
      if(fh = Open("AmiTCP:db/hosts", MODE_NEWFILE))
      {
         struct Host *host;

         FPrintf(fh, "## This file was generated by GENESiSPrefs " VERSIONSTRING "\n##\n");
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
      changed_hosts = FALSE;
   }

   if(changed_protocols)
   {
      if(fh = Open("AmiTCP:db/protocols", MODE_NEWFILE))
      {
         struct Protocol *protocol;

         FPrintf(fh, "## This file was generated by GENESiSPrefs " VERSIONSTRING "\n##\n");
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
      changed_protocols = FALSE;
   }

   if(changed_inetaccess)
   {
      if(fh = Open("AmiTCP:db/inet.access", MODE_NEWFILE))
      {
         struct InetAccess *inet_access;

         FPrintf(fh, "## This file was generated by GENESiSPrefs " VERSIONSTRING "\n##\n");
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
      changed_inetaccess = FALSE;
   }

   if(changed_inetd)
   {
      if(fh = Open("AmiTCP:db/inetd.conf", MODE_NEWFILE))
      {
         struct Inetd *inetd;

         FPrintf(fh, "## This file was generated by GENESiSPrefs " VERSIONSTRING "\n##\n");
         pos = 0;
         FOREVER
         {
            DoMethod(db_data->LI_Inetd, MUIM_List_GetEntry, pos++, &inetd);
            if(!inetd)
               break;
            if(!inetd->Active)
               FPrintf(fh, "#");
            FPrintf(fh, "%-15ls %-6ls %-5ls %-6ls %-8ls %ls", inetd->Service, (inetd->Socket ? "dgram" : "stream"), inetd->Protocol, (inetd->Wait ? (inetd->Wait == 2 ? "dos" : "wait") : "nowait"), inetd->User, inetd->Server);
            if(*inetd->CliName)
               FPrintf(fh, "  %-8ls", inetd->CliName);
            if(*inetd->Args)
               FPrintf(fh, "  %ls", inetd->Args);
            FPrintf(fh, "\n");
         }
         Close(fh);
      }
      changed_inetd = FALSE;
   }

   if(changed_networks)
   {
      if(fh = Open("AmiTCP:db/networks", MODE_NEWFILE))
      {
         struct DB_Network *db_network;

         FPrintf(fh, "## This file was generated by GENESiSPrefs " VERSIONSTRING "\n##\n");
         pos = 0;
         FOREVER
         {
            DoMethod(db_data->LI_Networks, MUIM_List_GetEntry, pos++, &db_network);
            if(!db_network)
               break;
            FPrintf(fh, "%ls     %ld", db_network->Name, db_network->Number);
            if(*db_network->Aliases)
               FPrintf(fh, "  %ls", db_network->Aliases);
            FPrintf(fh, "\n");
         }
         Close(fh);
      }
      changed_networks = FALSE;
   }

   if(changed_rpc)
   {
      if(fh = Open("AmiTCP:db/rpc", MODE_NEWFILE))
      {
         struct Rpc *rpc;

         FPrintf(fh, "## This file was generated by GENESiSPrefs " VERSIONSTRING "\n##\n");
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
      changed_rpc = FALSE;
   }

   ReloadUserList(); // tell the library to reload user database

   return(NULL);
}

///
/// MainWindow_Finish
ULONG MainWindow_Finish(struct IClass *cl, Object *obj, struct MUIP_MainWindow_Finish *msg)
{
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
/// MainWindow_AboutFinish
ULONG MainWindow_AboutFinish(struct IClass *cl, Object *obj, struct MUIP_MainWindow_AboutFinish *msg)
{
   Object *window = msg->window;

   set(window, MUIA_Window_Open, FALSE);
   DoMethod(app, OM_REMMEMBER, window);
   MUI_DisposeObject(window);
   set(app, MUIA_Application_Sleep, FALSE);

   return(NULL);
}

///
/// MainWindow_MenuLoad
ULONG MainWindow_MenuLoad(struct IClass *cl, Object *obj, Msg msg)
{
//   struct MainWindow_Data *data = INST_DATA(cl, obj);
   STRPTR file;

   if(file = getfilename(obj, GetStr(MSG_TX_ChooseConfigFileLoad), config_file, FALSE))
   {
      strncpy(config_file, file, MAXPATHLEN);
      DoMethod(obj, MUIM_MainWindow_ClearConfig);
      DoMethod(obj, MUIM_MainWindow_LoadConfig, config_file);
   }

   return(NULL);
}

///
/// MainWindow_MenuImport
ULONG MainWindow_MenuImport(struct IClass *cl, Object *obj, Msg msg)
{
//   struct MainWindow_Data *data = INST_DATA(cl, obj);

   STRPTR file;

   if(file = getfilename(obj, GetStr(MSG_TX_ChooseConfigFileImport), config_file, FALSE))
      DoMethod(obj, MUIM_MainWindow_LoadConfig, file);

   return(NULL);
}

///
/// MainWindow_MenuSaveAs
ULONG MainWindow_MenuSaveAs(struct IClass *cl, Object *obj, Msg msg)
{
//   struct MainWindow_Data *data = INST_DATA(cl, obj);
   STRPTR file;

   if(file = getfilename(obj, GetStr(MSG_TX_ChooseConfigFileSave), config_file, TRUE))
   {
      strncpy(config_file, file, MAXPATHLEN);
      DoMethod(obj, MUIM_MainWindow_SaveConfig, config_file);
   }

   return(NULL);
}

///

/// MainWindow_InitGroups
ULONG MainWindow_InitGroups(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   Object *bodychunk;
   struct FileInfoBlock *fib;
   BPTR lock;
   BOOL success = FALSE;

   data->GR_Interfaces  = data->GR_Options   = data->GR_Modems =
   data->GR_Database    = NULL;
#ifdef INTERNAL_USER
   data->GR_User           = NULL;
#endif

   if(data->GR_Info)
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&information_header, (ULONG *)information_colors , (UBYTE *)information_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_TX_GroupNames1), data->GR_Info, NULL, NULL, bodychunk);
   }
   if(data->GR_Interfaces     = NewObject(CL_Interfaces->mcc_Class, NULL, TAG_DONE))
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&interfaces_header, (ULONG *)interfaces_colors , (UBYTE *)interfaces_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_TX_GroupNames2), data->GR_Interfaces, NULL, NULL, bodychunk);
   }
   if(data->GR_Options        = NewObject(CL_Options->mcc_Class   , NULL, TAG_DONE))
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&options_header, (ULONG *)options_colors , (UBYTE *)options_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_TX_GroupNames4), data->GR_Options, NULL, NULL, bodychunk);
   }
   if(data->GR_Modems         = NewObject(CL_Modems->mcc_Class    , NULL, TAG_DONE))
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&modems_header, (ULONG *)modems_colors , (UBYTE *)modems_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_TX_GroupNames5), data->GR_Modems, NULL, NULL, bodychunk);
   }
   if(data->GR_Database       = NewObject(CL_Database->mcc_Class  , NULL, TAG_DONE))
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&database_header, (ULONG *)database_colors , (UBYTE *)database_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_TX_GroupNames6), data->GR_Database, NULL, NULL, bodychunk);
   }
#ifdef INTERNAL_USER
   if(data->GR_User            = NewObject(CL_User->mcc_Class      , NULL, TAG_DONE))
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&user_header, (ULONG *)user_colors , (UBYTE *)user_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_TX_GroupNames7), data->GR_User, NULL, NULL, bodychunk);
   }
   if(data->GR_Interfaces && data->GR_Options && data->GR_Database && data->GR_Modems && data->GR_User)
      success = TRUE;
#else
   if(data->GR_Interfaces && data->GR_Options && data->GR_Database && data->GR_Modems)
      success = TRUE;
#endif

   // load external prefs groups
   if(lock = Lock("AmiTCP:MUI", ACCESS_READ))
   {
      if(fib = AllocDosObject(DOS_FIB, NULL))
      {
         if(Examine(lock, fib))
         {
            Object *ext_class;
            struct Library *MCCBase;
            char buf[MAXPATHLEN];
            struct McpNode *node;

            while(ExNext(lock, fib))
            {
               if(strstr(fib->fib_FileName, ".mcp") == (fib->fib_FileName + strlen(fib->fib_FileName) - 4))
               {
                  sprintf(buf, "AmiTCP:MUI/%ls", fib->fib_FileName);
                  if(ext_class = MUI_NewObject(buf, TAG_DONE))
                  {
                     bodychunk  = NULL;
                     if(MCCBase = OpenLibrary(buf, 0))
                     {
                        bodychunk = (Object *)MCC_Query(2);
                        CloseLibrary(MCCBase);
                     }

                     sprintf(buf, "\0333%ls", fib->fib_FileName);
                     buf[strlen(buf) - 4] = NULL;

                     if(!bodychunk)
                        bodychunk = create_bodychunk((struct BitMapHeader *)&default_header, (ULONG *)default_colors , (UBYTE *)default_body);
                     if(bodychunk)
                        DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, buf, ext_class, NULL, NULL, bodychunk);
                     else
                        DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, buf, ext_class, NULL, NULL);

                     DoMethod(ext_class, MUIM_Settingsgroup_ConfigToGadgets, current_user);
                     if(node = (struct McpNode *)AllocVec(sizeof(struct McpNode), MEMF_ANY | MEMF_CLEAR))
                     {
                        node->mcp_object = ext_class;
                        AddTail((struct List *)&McpList, (struct Node *)node);
                     }
                  }
               }
            }
         }
         FreeDosObject(DOS_FIB, fib);
      }
      UnLock(lock);
   }

   return(success);
}

///

/// MainWindow_New
ULONG MainWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct MainWindow_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , "GENESiSPrefs © 1997-99 by Michael Neuweiler & Active Technologies",
      MUIA_Window_ID       , MAKE_ID('G','P','R','F'),
      MUIA_Window_Height   , MUIV_Window_Height_MinMax(0),
      MUIA_Window_Width    , MUIV_Window_Width_MinMax(0),
      MUIA_Window_AppWindow, TRUE,
      MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, MainWindowMenu, 0),
      WindowContents       , VGroup,
         Child, tmp.GR_Pager = GrouppagerObject,
            MUIA_Grouppager_ListHorizWeight  , 40,
            MUIA_Grouppager_ListMinLineHeight, MAX(INTERFACES_HEIGHT, MAX(MODEMS_HEIGHT, MAX(OPTIONS_HEIGHT, MAX(INFORMATION_HEIGHT, DATABASE_HEIGHT)))),
            MUIA_Grouppager_ListAdjustWidth  , TRUE,
            MUIA_Grouppager_DefaultGroup, VGroup,
               GroupFrame,
               MUIA_Background, MUII_GroupBack,
               Child, HVSpace,
            End,
//            MUIA_Grouppager_BalanceObject    , TRUE,
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
            End,
            Child, HVSpace,
         End,
         Child, CLabel("Preferences "VERSIONSTRING),
         Child, HVSpace,
#ifdef DEMO
         Child, TextObject,
#ifdef BETA
            MUIA_Text_Contents, "\033b\033cTHIS IS A BETA VERSION",
#else
            MUIA_Text_Contents, "\033b\033cTHIS IS A DEMO VERSION",
#endif
            MUIA_Font         , MUIV_Font_Big,
         End,
         Child, HVSpace,
#endif
      End))
         return(NULL);

      set(data->BT_Save   , MUIA_ShortHelp, GetStr(MSG_Help_Save));
      set(data->BT_Cancel , MUIA_ShortHelp, GetStr(MSG_Help_Cancel));

      DoMethod(obj            , MUIM_Notify, MUIA_Window_CloseRequest, TRUE   , obj, 2, MUIM_MainWindow_Finish, 0);
      DoMethod(data->BT_Cancel, MUIM_Notify, MUIA_Pressed            , FALSE  , obj, 2, MUIM_MainWindow_Finish, 0);
      DoMethod(data->BT_Save  , MUIM_Notify, MUIA_Pressed            , FALSE  , obj, 2, MUIM_MainWindow_Finish, 1);

      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT)       , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 1, MUIM_MainWindow_About);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ABOUT_MUI)   , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         MUIV_Notify_Application, 2, MUIM_Application_AboutMUI, obj);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_ICONIFY)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         MUIV_Notify_Application, 3, MUIM_Set, MUIA_Application_Iconified, TRUE);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_QUIT)        , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         MUIV_Notify_Application, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_LOAD)      , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 1, MUIM_MainWindow_MenuLoad);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_IMPORT)    , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 1, MUIM_MainWindow_MenuImport);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_SAVE)      , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 2, MUIM_MainWindow_SaveConfig, config_file);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_SAVEAS)    , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 1, MUIM_MainWindow_MenuSaveAs);

      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_MUI)       , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         MUIV_Notify_Application, 2, MUIM_Application_OpenConfigWindow, 0);
   }
   return((ULONG)obj);
}

///
/// MainWindow_Dispatcher
SAVEDS ASM ULONG MainWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                       : return(MainWindow_New           (cl, obj, (APTR)msg));
      case MUIM_MainWindow_InitGroups   : return(MainWindow_InitGroups    (cl, obj, (APTR)msg));
      case MUIM_MainWindow_Finish       : return(MainWindow_Finish        (cl, obj, (APTR)msg));
      case MUIM_MainWindow_ClearConfig  : return(MainWindow_ClearConfig   (cl, obj, (APTR)msg));
      case MUIM_MainWindow_LoadConfig   : return(MainWindow_LoadConfig    (cl, obj, (APTR)msg));
      case MUIM_MainWindow_LoadDatabase : return(MainWindow_LoadDatabase  (cl, obj, (APTR)msg));
      case MUIM_MainWindow_SaveConfig   : return(MainWindow_SaveConfig    (cl, obj, (APTR)msg));
      case MUIM_MainWindow_About        : return(MainWindow_About         (cl, obj, (APTR)msg));
      case MUIM_MainWindow_AboutFinish  : return(MainWindow_AboutFinish   (cl, obj, (APTR)msg));
      case MUIM_MainWindow_MenuLoad     : return(MainWindow_MenuLoad      (cl, obj, (APTR)msg));
      case MUIM_MainWindow_MenuImport   : return(MainWindow_MenuImport    (cl, obj, (APTR)msg));
      case MUIM_MainWindow_MenuSaveAs   : return(MainWindow_MenuSaveAs    (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

