/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/genesis_lib.h"
#include "rev.h"
#include "Strings.h"
#include "mui.h"
#include "mui_DataBase.h"
#include "mui_Dialer.h"
#include "mui_MainWindow.h"
#include "mui_Modem.h"
#include "mui_PasswdReq.h"
#include "mui_User.h"
#include "mui_Provider.h"
#include "mui_ProviderWindow.h"
#include "protos.h"
#include "mui/Grouppager_mcc.h"
#include "images/information.h"
#include "images/databases.h"
#include "images/dialer.h"
#include "images/modem.h"
#include "images/provider.h"
#include "images/logo.h"

///
/// external variables
extern struct MUI_CustomClass  *CL_User, *CL_Provider, *CL_ProviderWindow, *CL_Dialer,
                               *CL_Modem, *CL_Databases, *CL_About;
extern char config_file[];
extern BOOL changed_passwd, changed_group, changed_hosts, changed_protocols,
            changed_services, changed_inetd, changed_networks, changed_rpc, changed_inetaccess;
extern Object *win, *app;
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
/// is_true
BOOL is_true(struct pc_Data *pc_data)
{
   if(!*pc_data->Contents || !stricmp(pc_data->Contents, "yes") ||
      !stricmp(pc_data->Contents, "true") || !strcmp(pc_data->Contents, "1"))
      return(TRUE);
   return(FALSE);
}

///
/// is_false
BOOL is_false(struct pc_Data *pc_data)
{
   if(!stricmp(pc_data->Contents, "no") || !stricmp(pc_data->Contents, "false") ||
      !strcmp(pc_data->Contents, "0"))
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

enum { type_ISP=1, type_User, type_Iface, type_LoginScript };

/// MainWindow_ClearConfig
ULONG MainWindow_ClearConfig(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data              = INST_DATA(cl                      , obj);
   struct Provider_Data    *provider_data    = INST_DATA(CL_Provider->mcc_Class  , data->GR_Provider);
   struct User_Data        *user_data        = INST_DATA(CL_User->mcc_Class      , data->GR_User);
   struct Modem_Data       *modem_data       = INST_DATA(CL_Modem->mcc_Class     , data->GR_Modem);
   struct Dialer_Data      *dialer_data      = INST_DATA(CL_Dialer->mcc_Class    , data->GR_Dialer);

   DoMethod(provider_data->LI_ISP, MUIM_List_Clear);
   DoMethod(user_data->LI_User, MUIM_List_Clear);

   setstring(modem_data->STR_Modem, "Generic");
   setstring(modem_data->STR_InitString, "AT&F&D2");
   setstring(modem_data->STR_DialPrefix, "ATDT");
   setstring(modem_data->STR_DialSuffix, NULL);
   setslider(modem_data->SL_RedialAttempts, 15);
   setslider(modem_data->SL_RedialDelay, 5);

   setstring(modem_data->STR_SerialDevice, "serial.device");
   set(modem_data->STR_SerialUnit, MUIA_String_Integer, 0);
   set(modem_data->STR_BaudRate, MUIA_String_Integer, 38400);
   set(modem_data->STR_SerBufLen, MUIA_String_Integer, 16384);
   setcheckmark(modem_data->CH_IgnoreDSR, FALSE);
   setcheckmark(modem_data->CH_7Wire, FALSE);

   setcheckmark(dialer_data->CH_ConfirmOffline, FALSE);
   setcheckmark(dialer_data->CH_QuickReconnect, FALSE);
   setcheckmark(dialer_data->CH_Debug, FALSE);
   setcheckmark(dialer_data->CH_ShowLog, FALSE);
   setcheckmark(dialer_data->CH_ShowConnect, FALSE);
   setcheckmark(dialer_data->CH_ShowOnlineTime, FALSE);
   setcheckmark(dialer_data->CH_ShowButtons, FALSE);
   setcheckmark(dialer_data->CH_ShowNetwork, FALSE);
   setcheckmark(dialer_data->CH_ShowUser, FALSE);

   setstring(dialer_data->STR_Startup, NULL);
   setstring(dialer_data->STR_Shutdown, NULL);

   return(NULL);
}

///
/// MainWindow_LoadDatabases
ULONG MainWindow_LoadDatabases(struct IClass *cl, Object *obj, Msg msg)
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
   struct User *user = NULL;
   struct ISP *isp = NULL;
   struct Interface *iface = NULL;
   int current_type = NULL;

   /**** load the ModemDatabase into the List ****/
   if(ParseConfig("AmiTCP:db/modems", &pc_data))
   {
      struct Modem *modem;
      int protocol_nr = 0;

      if(modem = (struct Modem *)AllocVec(sizeof(struct Modem), MEMF_ANY | MEMF_CLEAR))
      {
         set(modem_data->LI_Modems, MUIA_List_Quiet, TRUE);
         DoMethod(modem_data->LI_Modems, MUIM_List_Clear);
         while(ParseNext(&pc_data))
         {
            if(!stricmp(pc_data.Argument, "Modem"))
            {
               if(*modem->Name)
               {
                  modem->ProtocolName[protocol_nr][0] = NULL;
                  DoMethod(modem_data->LI_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Sorted);
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
            DoMethod(modem_data->LI_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Sorted);
         }
         strcpy(modem->Name, "Generic");
         strcpy(modem->ProtocolName[0], "Default");
         strcpy(modem->InitString[0], "AT&F&D2");
         modem->ProtocolName[1][0] = NULL;
         DoMethod(modem_data->LI_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Top);

         FreeVec(modem);
         set(modem_data->LI_Modems, MUIA_List_Quiet, FALSE);
      }
      ParseEnd(&pc_data);
   }
   DoMethod(data->GR_Modem, MUIM_Modem_UpdateProtocolList);


   /** put devices into device list of modem page **/

   set(modem_data->LI_Devices, MUIA_List_Quiet, TRUE);
   DoMethod(modem_data->LI_Devices, MUIM_List_Clear);
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
                     DoMethod(modem_data->LI_Devices, MUIM_List_InsertSingle, fib->fib_FileName, MUIV_List_Insert_Sorted);
                  }
               }
            }
            FreeDosObject(DOS_FIB, fib);
         }
         UnLock(lock);
      }
   }
   set(modem_data->LI_Devices, MUIA_List_Quiet, FALSE);


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
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Groups);

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

               DoMethod(db_data->LI_Protocols, MUIM_List_InsertSingle, protocol, MUIV_List_Insert_Bottom);
            }
         }
         FreeVec(protocol);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_Protocols, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Protocols);


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

                  DoMethod(db_data->LI_Services, MUIM_List_InsertSingle, service, MUIV_List_Insert_Bottom);
               }
            }
         }
         FreeVec(service);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_Services, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Services);


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
                  DoMethod(db_data->LI_InetAccess, MUIM_List_InsertSingle, inet_access, MUIV_List_Insert_Bottom);
               }
            }
         }
         FreeVec(inet_access);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_InetAccess, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_InetAccess);


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
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Inetd);


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

               DoMethod(db_data->LI_Hosts, MUIM_List_InsertSingle, host, MUIV_List_Insert_Bottom);
            }
         }
         FreeVec(host);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_Hosts, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Hosts);


   /**** parse the networks file ****/

   set(db_data->LI_Networks, MUIA_List_Quiet, TRUE);
   DoMethod(db_data->LI_Networks, MUIM_List_Clear);
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

               DoMethod(db_data->LI_Networks, MUIM_List_InsertSingle, network, MUIV_List_Insert_Bottom);
            }
         }
         FreeVec(network);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_Networks, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Networks);


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

               DoMethod(db_data->LI_Rpcs, MUIM_List_InsertSingle, rpc, MUIV_List_Insert_Bottom);
            }
         }
         FreeVec(rpc);
      }
      ParseEnd(&pc_data);
   }
   set(db_data->LI_Rpcs, MUIA_List_Quiet, FALSE);
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Rpcs);

   return(NULL);
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
   STRPTR ptr;
   int ns = 0;
   struct User *user = NULL;
   struct ISP *isp = NULL;
   struct Interface *iface = NULL;
   struct PrefsPPPIface *ppp_if = NULL;
   int current_type = NULL;

   /**** load providers ****/
   if(ParseConfig((msg->file ? msg->file : (STRPTR)config_file), &pc_data))
   {
      while(ParseNext(&pc_data))
      {
         if(!stricmp(pc_data.Argument, "ISP"))
         {
            current_type = type_ISP;
            DoMethod(provider_data->LI_ISP, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
            DoMethod(provider_data->LI_ISP, MUIM_List_GetEntry, xget(provider_data->LI_ISP, MUIA_List_Entries) - 1 , &isp);
            continue;
         }
         else if(!stricmp(pc_data.Argument, "USER"))
         {
            current_type = type_User;
            DoMethod(user_data->LI_User, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
            DoMethod(user_data->LI_User, MUIM_List_GetEntry, xget(user_data->LI_User, MUIA_List_Entries) - 1, &user);
            continue;
         }
         else if(!stricmp(pc_data.Argument, "INTERFACE"))
         {
            if(isp)
            {
               if(iface = AllocVec(sizeof(struct Interface), MEMF_ANY | MEMF_CLEAR))
               {
                  NewList((struct List *)&iface->if_events);
                  ppp_if = iface->if_userdata = AllocVec(sizeof(struct PrefsPPPIface), MEMF_ANY | MEMF_CLEAR);
                  AddTail((struct List *)&isp->isp_ifaces, (struct Node *)iface);
                  current_type = type_Iface;
               }
               else
                  current_type = NULL;
            }
            continue;
         }
         else if(!stricmp(pc_data.Argument, "LOGINSCRIPT"))
         {
            current_type = type_LoginScript;
            continue;
         }

         else if(!stricmp(pc_data.Argument, "SerialDevice"))
            setstring(modem_data->STR_SerialDevice, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "SerialUnit"))
            set(modem_data->STR_SerialUnit, MUIA_String_Integer, atol(pc_data.Contents));
         else if(!stricmp(pc_data.Argument, "BaudRate"))
            set(modem_data->STR_BaudRate, MUIA_String_Integer, atol(pc_data.Contents));
         else if(!stricmp(pc_data.Argument, "SerBufLen"))
            set(modem_data->STR_SerBufLen, MUIA_String_Integer, atol(pc_data.Contents));

         else if(!stricmp(pc_data.Argument, "Modem"))
            setstring(modem_data->STR_Modem, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "InitString"))
            setstring(modem_data->STR_InitString, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "DialPrefix"))
            setstring(modem_data->STR_DialPrefix, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "DialSuffix"))
            setstring(modem_data->STR_DialSuffix, pc_data.Contents);
         else if(!stricmp(pc_data.Argument, "RedialAttempts"))
            setslider(modem_data->SL_RedialAttempts, atol(pc_data.Contents));
         else if(!stricmp(pc_data.Argument, "RedialDelay"))
            setslider(modem_data->SL_RedialDelay, atol(pc_data.Contents));

         else if(!stricmp(pc_data.Argument, "IgnoreDSR") && is_true(&pc_data))
            setcheckmark(modem_data->CH_IgnoreDSR, TRUE);
         else if(!stricmp(pc_data.Argument, "7Wire") && is_false(&pc_data))
            setcheckmark(modem_data->CH_7Wire, FALSE);
         else if(!stricmp(pc_data.Argument, "RadBoogie") && is_false(&pc_data))
            setcheckmark(modem_data->CH_RadBoogie, FALSE);
         else if(!stricmp(pc_data.Argument, "XonXoff") && is_true(&pc_data))
            setcheckmark(modem_data->CH_XonXoff, TRUE);
         else if(!stricmp(pc_data.Argument, "OwnDevUnit") && is_true(&pc_data))
            setcheckmark(modem_data->CH_OwnDevUnit, TRUE);

         else if(!stricmp(pc_data.Argument, "QuickReconnect") && is_true(&pc_data))
            setcheckmark(dialer_data->CH_QuickReconnect, TRUE);
         else if(!stricmp(pc_data.Argument, "Debug") && is_true(&pc_data))
            setcheckmark(dialer_data->CH_Debug, TRUE);
         else if(!stricmp(pc_data.Argument, "ConfirmOffline") && is_true(&pc_data))
            setcheckmark(dialer_data->CH_ConfirmOffline, TRUE);
         else if(!stricmp(pc_data.Argument, "ShowLog") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowLog, FALSE);
         else if(!stricmp(pc_data.Argument, "ShowLamps") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowLamps, FALSE);
         else if(!stricmp(pc_data.Argument, "ShowConnect") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowConnect, FALSE);
         else if(!stricmp(pc_data.Argument, "ShowOnlineTime") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowOnlineTime, FALSE);
         else if(!stricmp(pc_data.Argument, "ShowButtons") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowButtons, FALSE);
         else if(!stricmp(pc_data.Argument, "ShowNetwork") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowNetwork, FALSE);
         else if(!stricmp(pc_data.Argument, "ShowUser") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowUser, FALSE);
         else if(!stricmp(pc_data.Argument, "ShowStatusWin") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowStatusWin, FALSE);
         else if(!stricmp(pc_data.Argument, "ShowSerialInput") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowSerialInput, FALSE);
         else if(!stricmp(pc_data.Argument, "StartupOpenWin") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_StartupOpenWin, FALSE);
         else if(!stricmp(pc_data.Argument, "StartupIconify") && is_true(&pc_data))
            setcheckmark(dialer_data->CH_StartupIconify, TRUE);

         else if(!stricmp(pc_data.Argument, "Startup") && strlen(pc_data.Contents) > 2)
            setstring(dialer_data->STR_Startup, &pc_data.Contents[2]);
         else if(!stricmp(pc_data.Argument, "Shutdown") && strlen(pc_data.Contents) > 2)
            setstring(dialer_data->STR_Shutdown, &pc_data.Contents[2]);

         else switch(current_type)
         {
            case type_ISP:
               if(isp)
               {
                  if(!stricmp(pc_data.Argument, "Name"))
                     strcpy(isp->isp_name, (*pc_data.Contents ? pc_data.Contents : "noname ISP"));
                  else if(!stricmp(pc_data.Argument, "Comment"))
                     strcpy(isp->isp_comment, pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "Login"))
                     strcpy(isp->isp_login, pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "Password"))
                     decrypt(pc_data.Contents, isp->isp_password);

                  else if(!stricmp(pc_data.Argument, "Organisation"))
                     strcpy(isp->isp_organisation  , pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "Phone"))
                     strcpy(isp->isp_phonenumber, pc_data.Contents);

                  else if(!stricmp(pc_data.Argument, "BOOTPServer"))
                     strcpy(isp->isp_bootp, pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "HostName"))
                     strcpy(isp->isp_hostname, pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "TimeServer"))
                     strcpy(isp->isp_timename, pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "DontQueryHostname") && is_true(&pc_data))
                     isp->isp_flags |= ISF_DontQueryHostname;
                  else if(!stricmp(pc_data.Argument, "UseBootP") && is_true(&pc_data))
                     isp->isp_flags |= ISF_UseBootp;
                  else if(!stricmp(pc_data.Argument, "GetTime") && is_true(&pc_data))
                     isp->isp_flags |= ISF_GetTime;
                  else if(!stricmp(pc_data.Argument, "SaveTime") && is_true(&pc_data))
                     isp->isp_flags |= ISF_SaveTime;
                  else if(!stricmp(pc_data.Argument, "NameServer"))
                  {
                     struct ServerEntry *server;

                     if(server = AllocVec(sizeof(struct ServerEntry), MEMF_ANY))
                     {
                        strncpy(server->se_name, pc_data.Contents, 40);
                        AddTail((struct List *)&isp->isp_nameservers, (struct Node *)server);
                     }
                  }
                  else if(!stricmp(pc_data.Argument, "DomainName"))
                  {
                     struct ServerEntry *server;

                     if(server = AllocVec(sizeof(struct ServerEntry), MEMF_ANY))
                     {
                        strncpy(server->se_name, pc_data.Contents, 40);
                        AddTail((struct List *)&isp->isp_domainnames, (struct Node *)server);
                     }
                  }
               }
               break;
            case type_User:
               if(user)
               {
                  if(!stricmp(pc_data.Argument, "Name"))
                     strncpy(user->us_login, pc_data.Contents, sizeof(user->us_login));
                  else if(!stricmp(pc_data.Argument, "RealName"))
                     strncpy(user->us_realname, pc_data.Contents, sizeof(user->us_realname));
                  else if(!stricmp(pc_data.Argument, "Password"))
                  {
                     strncpy(user->us_password, pc_data.Contents, sizeof(user->us_password));
                     if(!strcmp(user->us_password, "*"))
                        user->us_flags |= UFL_Disabled;
                  }
                  else if(!stricmp(pc_data.Argument, "UID"))
                     user->us_uid = atol(pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "GID"))
                     user->us_gid = atol(pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "HomeDir"))
                     strncpy(user->us_homedir, pc_data.Contents, sizeof(user->us_homedir));
                  else if(!stricmp(pc_data.Argument, "Shell"))
                     strncpy(user->us_shell, pc_data.Contents, sizeof(user->us_shell));

//                  else if(!stricmp(pc_data.Argument, "MailLogin"))
else if(!stricmp(pc_data.Argument, "MailLogin") || !stricmp(pc_data.Argument, "POPLogin"))
                     strncpy(user->us_maillogin, pc_data.Contents, sizeof(user->us_maillogin));
//                  else if(!stricmp(pc_data.Argument, "MailPassword"))
else if(!stricmp(pc_data.Argument, "MailPassword") || !stricmp(pc_data.Argument, "POPPassword"))
                     decrypt(pc_data.Contents, user->us_mailpassword);
                  else if(!stricmp(pc_data.Argument, "EMail"))
                     strncpy(user->us_email, pc_data.Contents, sizeof(user->us_email));
//                  else if(!stricmp(pc_data.Argument, "MailServer"))
else if(!stricmp(pc_data.Argument, "MailServer") || !stricmp(pc_data.Argument, "POPServer"))
                     strncpy(user->us_mailserver, pc_data.Contents, sizeof(user->us_mailserver));
               }
               break;

            case type_Iface:
               if(iface)
               {
                  if(!stricmp(pc_data.Argument, "IfName"))
                     strcpy(iface->if_name, pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "Sana2Device"))
                     strcpy(iface->if_sana2device, pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "Sana2Unit"))
                     iface->if_sana2unit = atol(pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "Sana2Config"))
                     strcpy(iface->if_sana2config, pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "Sana2ConfigText") && !iface->if_sana2configtext)
                  {
                     if(iface->if_sana2configtext = AllocVec(strlen(pc_data.Contents) + 1, MEMF_ANY | MEMF_CLEAR))
                     {
                        if(strstr(pc_data.Contents, "\\n"))
                        {
                           STRPTR ptr2;

                           ptr = pc_data.Contents;
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
                           strcpy(iface->if_sana2configtext, pc_data.Contents);
                     }
                  }
                  else if(!stricmp(pc_data.Argument, "IfConfigParams") && !iface->if_configparams)
                     realloc_copy((STRPTR *)&iface->if_configparams, pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "MTU"))
                     iface->if_MTU = atol(pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "IPAddr"))
                     strcpy(iface->if_addr, pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "DestIP"))
                     strcpy(iface->if_dst, pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "Gateway"))
                     strcpy(iface->if_gateway, pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "Netmask"))
                     strcpy(iface->if_netmask, pc_data.Contents);

                  else if(!stricmp(pc_data.Argument, "AlwaysOnline") && is_true(&pc_data))
                     iface->if_flags |= IFL_AlwaysOnline;
                  else if(!stricmp(pc_data.Argument, "DefaultPPP") && is_true(&pc_data))
                     iface->if_flags |= IFL_PPP;
                  else if(!stricmp(pc_data.Argument, "DefaultSLIP") && is_true(&pc_data))
                     iface->if_flags |= IFL_SLIP;
                  else if(!stricmp(pc_data.Argument, "KeepAlive"))
                     iface->if_keepalive = atol(pc_data.Contents);

                  else if(!stricmp(pc_data.Argument, "CarrierDetect") && is_true(&pc_data) && ppp_if)
                     ppp_if->ppp_carrierdetect = TRUE;
                  else if(!stricmp(pc_data.Argument, "ConnectTimeout") && ppp_if)
                     ppp_if->ppp_connecttimeout = atol(pc_data.Contents);
                  else if(!stricmp(pc_data.Argument, "Callback") && ppp_if)
                     strncpy(ppp_if->ppp_callback, pc_data.Contents, sizeof(ppp_if->ppp_callback));
                  else if(!stricmp(pc_data.Argument, "MPPCompression") && is_true(&pc_data) && ppp_if)
                     ppp_if->ppp_mppcomp = TRUE;
                  else if(!stricmp(pc_data.Argument, "VJCompression") && is_true(&pc_data) && ppp_if)
                     ppp_if->ppp_vjcomp = TRUE;
                  else if(!stricmp(pc_data.Argument, "BSDCompression") && is_true(&pc_data) && ppp_if)
                     ppp_if->ppp_bsdcomp = TRUE;
                  else if(!stricmp(pc_data.Argument, "DeflateCompression") && is_true(&pc_data) && ppp_if)
                     ppp_if->ppp_deflatecomp = TRUE;
                  else if(!stricmp(pc_data.Argument, "EOF") && is_true(&pc_data) && ppp_if)
                     ppp_if->ppp_eof = TRUE;

                  else
                  {
                     struct ScriptLine *script_line;
                     int command;

                     if(!stricmp(pc_data.Argument, event_commands[IFE_Online]))
                        command = IFE_Online;
                     else if(!stricmp(pc_data.Argument, event_commands[IFE_OnlineFail]))
                        command = IFE_OnlineFail;
                     else if(!stricmp(pc_data.Argument, event_commands[IFE_OfflineActive]))
                        command = IFE_OfflineActive;
                     else if(!stricmp(pc_data.Argument, event_commands[IFE_OfflinePassive]))
                        command = IFE_OfflinePassive;
                     else
                        command = -1;

                     if(command != -1)
                     {
                        if(script_line = AllocVec(sizeof(struct ScriptLine), MEMF_ANY))
                        {
                           script_line->sl_command = command;
                           strncpy(script_line->sl_contents, &pc_data.Contents[2], script_line->sl_command);
                           script_line->sl_userdata = *pc_data.Contents - 48;

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

               if(!stricmp(pc_data.Argument, script_commands[SL_Send]))
                  command = SL_Send;
               else if(!stricmp(pc_data.Argument, script_commands[SL_WaitFor]))
                  command = SL_WaitFor;
               else if(!stricmp(pc_data.Argument, script_commands[SL_Dial]))
                  command = SL_Dial;
               else if(!stricmp(pc_data.Argument, script_commands[SL_GoOnline]))
                  command = SL_GoOnline;
               else if(!stricmp(pc_data.Argument, script_commands[SL_SendLogin]))
                  command = SL_SendLogin;
               else if(!stricmp(pc_data.Argument, script_commands[SL_SendPassword]))
                  command = SL_SendPassword;
               else if(!stricmp(pc_data.Argument, script_commands[SL_SendBreak]))
                  command = SL_SendBreak;
               else if(!stricmp(pc_data.Argument, script_commands[SL_Exec]))
                  command = SL_Exec;
               else if(!stricmp(pc_data.Argument, script_commands[SL_Pause]))
                  command = SL_Pause;
               else if(!stricmp(pc_data.Argument, script_commands[SL_ParseIP]))
                  command = SL_ParseIP;
               else if(!stricmp(pc_data.Argument, script_commands[SL_ParsePasswd]))
                  command = SL_ParsePasswd;
               else
                  command = -1;

               if(command != -1 && isp)
               {
                  if(script_line = AllocVec(sizeof(struct ScriptLine), MEMF_ANY))
                  {
                     script_line->sl_command = command;
                     strncpy(script_line->sl_contents, pc_data.Contents, 250);

                     AddTail((struct List *)&isp->isp_loginscript, (struct Node *)script_line);
                  }
               }
            }
            break;
         }
      }
      ParseEnd(&pc_data);
   }
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
   struct User *user = NULL;
   struct ISP *isp = NULL;
   BPTR fh;
   char buff[110];
   STRPTR ptr;
   LONG pos;
   BOOL got_root = FALSE;

   if(fh = Open((msg->file ? msg->file : (STRPTR)config_file), MODE_NEWFILE))
   {
      FPrintf(fh, "## This file was generated by Genesis Prefs\n##\n\n");

      FPrintf(fh, "SerialDevice    %ls\n", xget(modem_data->STR_SerialDevice  , MUIA_String_Contents));
      FPrintf(fh, "SerialUnit      %ls\n", xget(modem_data->STR_SerialUnit    , MUIA_String_Contents));
      FPrintf(fh, "BaudRate        %ls\n", xget(modem_data->STR_BaudRate      , MUIA_String_Contents));
      FPrintf(fh, "SerBufLen       %ls\n\n", xget(modem_data->STR_SerBufLen   , MUIA_String_Contents));

      FPrintf(fh, "Modem           %ls\n", xget(modem_data->STR_Modem         , MUIA_String_Contents));
      FPrintf(fh, "InitString      %ls\n", xget(modem_data->STR_InitString    , MUIA_String_Contents));
      FPrintf(fh, "DialPrefix      %ls\n", xget(modem_data->STR_DialPrefix    , MUIA_String_Contents));
      FPrintf(fh, "DialSuffix      %ls\n", xget(modem_data->STR_DialSuffix    , MUIA_String_Contents));
      FPrintf(fh, "RedialAttempts  %ld\n", xget(modem_data->SL_RedialAttempts , MUIA_Numeric_Value));
      FPrintf(fh, "RedialDelay     %ld\n", xget(modem_data->SL_RedialDelay    , MUIA_Numeric_Value));
      FPrintf(fh, "IgnoreDSR       %ls\n", yes_no(xget(modem_data->CH_IgnoreDSR       , MUIA_Selected)));
      FPrintf(fh, "7Wire           %ls\n", yes_no(xget(modem_data->CH_7Wire           , MUIA_Selected)));
      FPrintf(fh, "RadBoogie       %ls\n", yes_no(xget(modem_data->CH_RadBoogie       , MUIA_Selected)));
      FPrintf(fh, "XonXoff         %ls\n", yes_no(xget(modem_data->CH_XonXoff         , MUIA_Selected)));
      FPrintf(fh, "OwnDevUnit      %ls\n\n", yes_no(xget(modem_data->CH_OwnDevUnit    , MUIA_Selected)));
      FPrintf(fh, "QuickReconnect  %ls\n", yes_no(xget(dialer_data->CH_QuickReconnect , MUIA_Selected)));
      FPrintf(fh, "Debug           %ls\n", yes_no(xget(dialer_data->CH_Debug          , MUIA_Selected)));
      FPrintf(fh, "ConfirmOffline  %ls\n", yes_no(xget(dialer_data->CH_ConfirmOffline , MUIA_Selected)));
      FPrintf(fh, "ShowLog         %ls\n", yes_no(xget(dialer_data->CH_ShowLog        , MUIA_Selected)));
      FPrintf(fh, "ShowLamps       %ls\n", yes_no(xget(dialer_data->CH_ShowLamps      , MUIA_Selected)));
      FPrintf(fh, "ShowConnect     %ls\n", yes_no(xget(dialer_data->CH_ShowConnect    , MUIA_Selected)));
      FPrintf(fh, "ShowOnlineTime  %ls\n", yes_no(xget(dialer_data->CH_ShowOnlineTime , MUIA_Selected)));
      FPrintf(fh, "ShowButtons     %ls\n", yes_no(xget(dialer_data->CH_ShowButtons    , MUIA_Selected)));
      FPrintf(fh, "ShowNetwork     %ls\n", yes_no(xget(dialer_data->CH_ShowNetwork    , MUIA_Selected)));
      FPrintf(fh, "ShowUser        %ls\n", yes_no(xget(dialer_data->CH_ShowUser       , MUIA_Selected)));
      FPrintf(fh, "ShowStatusWin   %ls\n", yes_no(xget(dialer_data->CH_ShowStatusWin  , MUIA_Selected)));
      FPrintf(fh, "ShowSerialInput %ls\n", yes_no(xget(dialer_data->CH_ShowSerialInput, MUIA_Selected)));
      FPrintf(fh, "StartupOpenWin  %ls\n", yes_no(xget(dialer_data->CH_StartupOpenWin , MUIA_Selected)));
      FPrintf(fh, "StartupIconify  %ls\n", yes_no(xget(dialer_data->CH_StartupIconify , MUIA_Selected)));

      if(strlen((STRPTR)xget(dialer_data->STR_Startup      , MUIA_String_Contents)))
         FPrintf(fh, "Startup         %ls\n", xget(dialer_data->STR_Startup       , MUIA_String_Contents));
      if(strlen((STRPTR)xget(dialer_data->STR_Shutdown     , MUIA_String_Contents)))
         FPrintf(fh, "Shutdown        %ls\n\n", xget(dialer_data->STR_Shutdown    , MUIA_String_Contents));

      FPrintf(fh, "\n## ISP, user and interface configuration\n##\n\n");

      pos = 0;
      FOREVER
      {
         DoMethod(provider_data->LI_ISP, MUIM_List_GetEntry, pos++, &isp);
         if(!isp)
            break;

         FPrintf(fh, "\nISP\n");
         FPrintf(fh, "Name              %ls\n", (*isp->isp_name ? isp->isp_name : "noname"));
         if(*isp->isp_comment)
            FPrintf(fh, "Comment           %ls\n", isp->isp_comment);
         if(*isp->isp_login)
            FPrintf(fh, "Login             %ls\n", isp->isp_login);
         if(*isp->isp_password)
         {
            encrypt(isp->isp_password, buff);
            FPrintf(fh, "Password          \"%ls\"\n", buff);
         }
         if(*isp->isp_organisation)
            FPrintf(fh, "Organisation      %ls\n", isp->isp_organisation);
         if(*isp->isp_phonenumber)
            FPrintf(fh, "Phone             %ls\n", isp->isp_phonenumber);
         if(*isp->isp_bootp)
            FPrintf(fh, "BOOTPServer       %ls\n", isp->isp_bootp);
         if(*isp->isp_hostname)
            FPrintf(fh, "HostName          %ls\n", isp->isp_hostname);
         if(*isp->isp_timename)
            FPrintf(fh, "TimeServer        %ls\n", isp->isp_timename);

         if(isp->isp_flags & ISF_UseBootp)
            FPrintf(fh, "UseBOOTP          yes\n");
         if(isp->isp_flags & ISF_DontQueryHostname)
            FPrintf(fh, "DontQueryHostname yes\n");
         if(isp->isp_flags & ISF_GetTime)
            FPrintf(fh, "GetTime           yes\n");
         if(isp->isp_flags & ISF_SaveTime)
            FPrintf(fh, "SaveTime          yes\n");

         if(isp->isp_nameservers.mlh_TailPred != (struct MinNode *)&isp->isp_nameservers)
         {
            struct ServerEntry *server;

            server = (struct ServerEntry *)isp->isp_nameservers.mlh_Head;
            while(server->se_node.mln_Succ)
            {
               FPrintf(fh, "NameServer        %ls\n", server->se_name);
               server = (struct ServerEntry *)server->se_node.mln_Succ;
            }
         }
         if(isp->isp_domainnames.mlh_TailPred != (struct MinNode *)&isp->isp_domainnames)
         {
            struct ServerEntry *server;

            server = (struct ServerEntry *)isp->isp_domainnames.mlh_Head;
            while(server->se_node.mln_Succ)
            {
               FPrintf(fh, "DomainName        %ls\n", server->se_name);
               server = (struct ServerEntry *)server->se_node.mln_Succ;
            }
         }

         // interfaces

         if(isp->isp_ifaces.mlh_TailPred != (struct MinNode *)&isp->isp_ifaces)
         {
            struct Interface *iface;
            struct PrefsPPPIface *ppp_if;

            iface = (struct Interface *)isp->isp_ifaces.mlh_Head;
            while(iface->if_node.mln_Succ)
            {
               ppp_if = iface->if_userdata;

               FPrintf(fh, "\nINTERFACE\n");
               FPrintf(fh, "IfName          %ls\n", (*iface->if_name ? iface->if_name : "xxx"));
               FPrintf(fh, "Sana2Device     %ls\n", iface->if_sana2device);
               FPrintf(fh, "Sana2Unit       %ld\n", iface->if_sana2unit);
               FPrintf(fh, "Sana2Config     %ls\n", iface->if_sana2config);
               if(iface->if_flags & IFL_PPP)
               {
                  FPrintf(fh, "Sana2ConfigText sername %ls\\nserunit %ld\\nserbaud %ld\\nlocalipaddress %%a\\n",
                     xget(modem_data->STR_SerialDevice, MUIA_String_Contents),
                     xget(modem_data->STR_SerialUnit, MUIA_String_Integer),
                     xget(modem_data->STR_BaudRate, MUIA_String_Integer));
                  if(*isp->isp_login)
                     FPrintf(fh, "user %%u\\n");
                  if(*isp->isp_password)
                     FPrintf(fh, "secret %%p\\n");
                  if(xget(dialer_data->CH_Debug, MUIA_Selected))
                     FPrintf(fh, "debug = 1\\ndebug_window = 1\\nerror_requesters = 0\\nlog_file = t:appp.log\\n");
                  if(xget(modem_data->CH_OwnDevUnit, MUIA_Selected))
                     FPrintf(fh, "useodu yes\\n");
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
                  FPrintf(fh, "\n");
               }
               else if(iface->if_flags & IFL_SLIP)
               {
                  FPrintf(fh, "Sana2ConfigText %ls %ld %ld Shared %%a MTU=%ld",
                     xget(modem_data->STR_SerialDevice, MUIA_String_Contents),
                     xget(modem_data->STR_SerialUnit, MUIA_String_Integer),
                     xget(modem_data->STR_BaudRate, MUIA_String_Integer),
                     iface->if_MTU);

                  if(xget(modem_data->CH_7Wire, MUIA_Selected))
                     FPrintf(fh, " 7Wire");
                  if(ppp_if)
                  {
                     if(ppp_if->ppp_carrierdetect)
                        FPrintf(fh, " CD");
                  }
                  FPrintf(fh, "\n");
               }
               else if(iface->if_sana2configtext && *iface->if_sana2configtext)
               {
                  FPrintf(fh, "Sana2ConfigText ");

                  ptr = iface->if_sana2configtext;
                  while(*ptr)
                  {
                     if(*ptr == '\n')
                        FPrintf(fh, "\\n");
                     else
                        FPrintf(fh, "%lc", *ptr);
                     ptr++;
                  }
                  FPrintf(fh, "\n");
               }
               if(iface->if_configparams && *iface->if_configparams)
                  FPrintf(fh, "IfConfigParams  %ls\n", iface->if_configparams);
               FPrintf(fh, "MTU             %ld\n", iface->if_MTU);

               if(*iface->if_addr)
                  FPrintf(fh, "IPAddr          %ls\n", iface->if_addr);
               if(*iface->if_dst)
                  FPrintf(fh, "DestIP          %ls\n", iface->if_dst);
               if(*iface->if_gateway)
                  FPrintf(fh, "Gateway         %ls\n", iface->if_gateway);
               if(*iface->if_netmask)
                  FPrintf(fh, "Netmask         %ls\n", iface->if_netmask);

               if(iface->if_flags & IFL_AlwaysOnline)
                  FPrintf(fh, "AlwaysOnline\n");
               if(iface->if_flags & IFL_PPP)
                  FPrintf(fh, "DefaultPPP\n");
               if(iface->if_flags & IFL_SLIP)
                  FPrintf(fh, "DefaultSLIP\n");
               if(iface->if_keepalive)
                  FPrintf(fh, "KeepAlive       %ld\n", iface->if_keepalive);

               if(ppp_if && ((iface->if_flags & IFL_PPP) || (iface->if_flags & IFL_SLIP)))
               {
                  FPrintf(fh, "CarrierDetect   %ls\n", yes_no(ppp_if->ppp_carrierdetect));
                  if(iface->if_flags & IFL_PPP)
                  {
                     if(ppp_if->ppp_connecttimeout)
                        FPrintf(fh, "ConnectTimeout  %ld\n", ppp_if->ppp_connecttimeout);
                     if(*ppp_if->ppp_callback)
                        FPrintf(fh, "Callback        %ls\n", ppp_if->ppp_callback);
                     FPrintf(fh, "MPPCompression     %ls\n", yes_no(ppp_if->ppp_mppcomp));
                     FPrintf(fh, "VJCompression      %ls\n", yes_no(ppp_if->ppp_vjcomp));
                     FPrintf(fh, "BSDCompression     %ls\n", yes_no(ppp_if->ppp_bsdcomp));
                     FPrintf(fh, "DeflateCompression %ls\n", yes_no(ppp_if->ppp_deflatecomp));
                     FPrintf(fh, "EOF                %ls\n", yes_no(ppp_if->ppp_eof));
                  }
               }

               if(iface->if_events.mlh_TailPred != (struct MinNode *)&iface->if_events)
               {
                  struct ScriptLine *event;

                  event = (struct ScriptLine *)iface->if_events.mlh_Head;
                  while(event->sl_node.mln_Succ)
                  {
                     FPrintf(fh, "%ls      %ld %ls\n", event_commands[event->sl_command], event->sl_userdata, event->sl_contents);
                     event = (struct ScriptLine *)event->sl_node.mln_Succ;
                  }
               }

               iface = (struct Interface *)iface->if_node.mln_Succ;
            }
         }

         if(isp->isp_loginscript.mlh_TailPred != (struct MinNode *)&isp->isp_loginscript)
         {
            struct ScriptLine *script_line;

            FPrintf(fh, "\nLOGINSCRIPT\n");
            script_line = (struct ScriptLine *)isp->isp_loginscript.mlh_Head;
            while(script_line->sl_node.mln_Succ)
            {
               if(*script_line->sl_contents)
                  FPrintf(fh, "%ls \"%ls\"\n", script_commands[script_line->sl_command], script_line->sl_contents);
               else
                  FPrintf(fh, "%ls\n", script_commands[script_line->sl_command]);

               script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
            }
         }
      }
      pos = 0;
      FOREVER
      {
         DoMethod(user_data->LI_User, MUIM_List_GetEntry, pos++, &user);
         if(!user)
            break;

         if(!strcmp(user->us_login, "root"))
            got_root = TRUE;

         FPrintf(fh, "\nUSER\n");
         if(*user->us_login)
            FPrintf(fh, "Name         %ls\n", user->us_login);
         if(*user->us_realname)
            FPrintf(fh, "RealName     %ls\n", user->us_realname);
         if(*user->us_password)
            FPrintf(fh, "Password     \"%ls\"\n", (user->us_flags & UFL_Disabled ? "*" : user->us_password));
         FPrintf(fh, "UID          %ls\n", user->us_uid);
         FPrintf(fh, "GID          %ls\n", user->us_gid);
         if(*user->us_homedir)
            FPrintf(fh, "HomeDir      %ls\n", user->us_homedir);
         if(*user->us_shell)
            FPrintf(fh, "Shell        %ls\n", user->us_shell);
         if(*user->us_maillogin)
            FPrintf(fh, "MailLogin     %ls\n", user->us_maillogin);
         if(*user->us_mailpassword)
         {
            encrypt(user->us_mailpassword, buff);
            FPrintf(fh, "MailPassword  \"%ls\"\n", buff);
         }
         if(*user->us_email)
            FPrintf(fh, "EMail        %ls\n", user->us_email);
         if(*user->us_mailserver)
            FPrintf(fh, "MailServer    %ls\n", user->us_mailserver);
      }
      if(!got_root)
         FPrintf(fh, "\nUSER\nName root\nRealName The Judge\nPassword *\nUID 0\nGID 0\nHomeDir SYS:\nShell shell\n");

      Close(fh);
   }

//   if(changed_passwd)
   {
      if(fh = Open("AmiTCP:db/passwd", MODE_NEWFILE))
      {
         struct User *user;

         pos = 0;
         FOREVER
         {
            DoMethod(user_data->LI_User, MUIM_List_GetEntry, pos++, &user);
            if(!user)
               break;
            FPrintf(fh, "%ls|%ls|%ld|%ld|%ls|%ls|%ls\n", user->us_login, (user->us_flags & UFL_Disabled ? "*" : user->us_password), user->us_uid, user->us_gid, user->us_realname, user->us_homedir, user->us_shell);
         }
         if(!got_root)
            FPrintf(fh, "root|*|0|0|The Judge|SYS:|shell\n");
         Close(fh);
      }
      changed_passwd = FALSE;
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
            FPrintf(fh, "%ls|%ls|%ld|%ls\n", group->Name, "*", group->ID, group->Members);
         }
         Close(fh);
      }
      changed_group = FALSE;
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
      changed_hosts = FALSE;
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
      changed_protocols = FALSE;
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
      changed_services = FALSE;
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
      changed_inetaccess = FALSE;
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
      changed_inetd = FALSE;
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
      changed_networks = FALSE;
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
      changed_rpc = FALSE;
   }

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
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   STRPTR file;

   if(file = getfilename(obj, "Choose config file to load..", config_file, FALSE))
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
   struct MainWindow_Data *data = INST_DATA(cl, obj);

   STRPTR file;

   if(file = getfilename(obj, "Choose config file to import..", config_file, FALSE))
      DoMethod(obj, MUIM_MainWindow_LoadConfig, file);

   return(NULL);
}

///
/// MainWindow_MenuSaveAs
ULONG MainWindow_MenuSaveAs(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   STRPTR file;

   if(file = getfilename(obj, "Choose filename to save config..", config_file, TRUE))
   {
      strncpy(config_file, file, MAXPATHLEN);
      DoMethod(obj, MUIM_MainWindow_SaveConfig, config_file);
   }

   return(NULL);
}

///

#ifdef DO_LISTTREE
/// MainWindow_InitGroups (listtree)
ULONG MainWindow_InitGroups(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data = INST_DATA(cl, obj);
   BOOL success = FALSE;

   data->GR_Provider       = data->GR_Dialer         = data->GR_Modem     =
   data->GR_Databases      = data->GR_User           = NULL;

   if(data->GR_Info)
   {
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Info"), data->GR_Info, NULL, NULL);
   }
   if(data->GR_Provider        = NewObject(CL_Provider->mcc_Class  , NULL, TAG_DONE))
   {
      struct Provider_Data *p_data = INST_DATA(CL_Provider->mcc_Class, data->GR_Provider);

      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Provider"), data->GR_Provider, NULL, NULL);
   }
   if(data->GR_User        = NewObject(CL_User->mcc_Class  , NULL, TAG_DONE))
   {
      struct User_Data *u_data = INST_DATA(CL_User->mcc_Class, data->GR_User);

      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  User"), data->GR_User, NULL, NULL);
   }
   if(data->GR_Dialer       = NewObject(CL_Dialer->mcc_Class     , NULL, TAG_DONE))
   {
      struct Dialer_Data *d_data = INST_DATA(CL_Dialer->mcc_Class, data->GR_Dialer);

      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Dialer"), data->GR_Dialer, NULL, GPE_LIST);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Events"), d_data->GR_Events , data->GR_Dialer, NULL);
   }
   if(data->GR_Modem       = NewObject(CL_Modem->mcc_Class     , NULL, TAG_DONE))
   {
      struct Modem_Data *m_data = INST_DATA(CL_Modem->mcc_Class, data->GR_Modem);

      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr("  Modem / TA"), m_data->GR_Modem , NULL, GPE_LIST);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertGroup, GetStr(MSG_ModemRegister2), m_data->GR_Serial , data->GR_Modem, NULL);
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

   if(data->GR_Provider && data->GR_Dialer && data->GR_User &&
      data->GR_Databases && data->GR_Modem)
   {
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
   static STRPTR modem_titles[] = { "Modem / TA", "Device", NULL };
   static STRPTR dialer_titles[] = { "Misc", "Events", NULL };

   struct MainWindow_Data *data = INST_DATA(cl, obj);
   Object *tmp_group, *bodychunk;
   BOOL success = FALSE;

   data->GR_Provider       = data->GR_Dialer         = data->GR_Modem      =
   data->GR_Databases      = data->GR_User           = NULL;

   if(data->GR_Info)
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&information_header, (ULONG *)information_colors , (UBYTE *)information_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr("  Info"), data->GR_Info, NULL, NULL, bodychunk);
   }
   if(data->GR_Provider        = NewObject(CL_Provider->mcc_Class      , NULL, TAG_DONE))
   {
      struct Provider_Data *p_data = INST_DATA(CL_Provider->mcc_Class, data->GR_Provider);

      bodychunk = create_bodychunk((struct BitMapHeader *)&provider_header, (ULONG *)provider_colors , (UBYTE *)provider_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr("  Provider"), data->GR_Provider, NULL, NULL, bodychunk);
   }
   if(data->GR_User            = NewObject(CL_User->mcc_Class      , NULL, TAG_DONE))
   {
      struct User_Data *u_data = INST_DATA(CL_User->mcc_Class, data->GR_User);

      bodychunk = create_bodychunk((struct BitMapHeader *)&user_header, (ULONG *)user_colors , (UBYTE *)user_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr("  User"), data->GR_User, NULL, NULL, bodychunk);
   }
   if(data->GR_Dialer       = NewObject(CL_Dialer->mcc_Class     , NULL, TAG_DONE))
   {
      struct Dialer_Data *d_data = INST_DATA(CL_Dialer->mcc_Class, data->GR_Dialer);

      bodychunk = create_bodychunk((struct BitMapHeader *)&dialer_header, (ULONG *)dialer_colors , (UBYTE *)dialer_body);
      if(tmp_group = RegisterGroup(dialer_titles),
         MUIA_CycleChain      , 1,
         Child, d_data->GR_Misc,
         Child, d_data->GR_Events,
      End)
         DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr("  Options"), tmp_group, NULL, NULL, bodychunk);
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
         DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr("  Modem/TA"), tmp_group, NULL, NULL, bodychunk);
   }
   if(data->GR_Databases   = NewObject(CL_Databases->mcc_Class , NULL, TAG_DONE))
   {
      struct Databases_Data *d_data = INST_DATA(CL_Databases->mcc_Class, data->GR_Databases);

      bodychunk = create_bodychunk((struct BitMapHeader *)&databases_header, (ULONG *)databases_colors , (UBYTE *)databases_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr("  Database"), data->GR_Databases, NULL, NULL, bodychunk);
   }

   if(data->GR_Provider && data->GR_Dialer && data->GR_Databases && data->GR_Modem && data->GR_User)
      success = TRUE;

   return(success);
}

///
#endif

/// MainWindow_New
ULONG MainWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct MainWindow_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , "GenesisPrefs © 1997,98 by Michael Neuweiler, Active Software",
      MUIA_Window_ID       , MAKE_ID('A','R','E','F'),
      MUIA_Window_AppWindow, TRUE,
      MUIA_Window_Menustrip, tmp.MN_Strip = MUI_MakeObject(MUIO_MenustripNM, MainWindowMenu, 0),
      WindowContents       , VGroup,
         Child, tmp.GR_Pager = GrouppagerObject,
            MUIA_Grouppager_ListHorizWeight  , 40,
            MUIA_Grouppager_ListMinLineHeight, MAX(PROVIDER_HEIGHT, MAX(MODEM_HEIGHT, MAX(DIALER_HEIGHT, MAX(INFORMATION_HEIGHT, DATABASES_HEIGHT)))),
            MUIA_Grouppager_ListAdjustWidth  , TRUE,
            MUIA_Grouppager_DefaultGroup, VGroup,
               GroupFrame,
               MUIA_Background, MUII_GroupBack,
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
            End,
            Child, HVSpace,
         End,
         Child, CLabel("Preferences "VERSIONSTRING),
         Child, HVSpace,
#ifdef DEMO
         Child, CLabel(GetStr(MSG_TX_DemoVersion)),
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

      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_LOAD)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 1, MUIM_MainWindow_MenuLoad);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_IMPORT)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 1, MUIM_MainWindow_MenuImport);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_SAVE)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 2, MUIM_MainWindow_SaveConfig, config_file);
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_SAVEAS)     , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 1, MUIM_MainWindow_MenuSaveAs);

      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_MUI)         , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
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
      case MUIM_MainWindow_LoadDatabases: return(MainWindow_LoadDatabases (cl, obj, (APTR)msg));
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

