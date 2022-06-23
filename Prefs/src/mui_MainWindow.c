/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
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
#include "mui/grouppager_mcc.h"
#include "images/information.h"
#include "images/databases.h"
#include "images/dialer.h"
#include "images/modem.h"
#include "images/provider.h"
#include "images/logo.h"
#include "mcc.h"

///
/// external variables
extern struct GenesisBase *GenesisBase;
extern struct MUI_CustomClass  *CL_User, *CL_Provider, *CL_ProviderWindow, *CL_Dialer,
                               *CL_Modem, *CL_Databases, *CL_About;
extern char config_file[];
extern BOOL changed_passwd, changed_group, changed_hosts, changed_protocols,
            changed_services, changed_inetd, changed_networks, changed_rpc, changed_inetaccess;
extern Object *win, *app;
extern struct NewMenu MainWindowMenu[];
extern struct MinList McpList;
extern struct User *current_user;

extern struct BitMapHeader information_header, provider_header, user_header, modem_header, dialer_header, databases_header, logo_header, default_header;
extern ULONG information_colors[], provider_colors[], user_colors[], modem_colors[], dialer_colors[], databases_colors[], logo_colors[], default_colors[];
extern UBYTE information_body[], provider_body[], user_body[], modem_body[], dialer_body[], databases_body[], logo_body[], default_body[];

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

enum { type_ISP=1, type_Iface, type_LoginScript };

/// MainWindow_ClearConfig
ULONG MainWindow_ClearConfig(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data              = INST_DATA(cl                      , obj);
   struct Provider_Data    *provider_data    = INST_DATA(CL_Provider->mcc_Class  , data->GR_Provider);
   struct Modem_Data       *modem_data       = INST_DATA(CL_Modem->mcc_Class     , data->GR_Modem);
   struct Dialer_Data      *dialer_data      = INST_DATA(CL_Dialer->mcc_Class    , data->GR_Dialer);
#ifdef INTERNAL_USER
   struct User_Data        *user_data        = INST_DATA(CL_User->mcc_Class      , data->GR_User);
#endif

   DoMethod(provider_data->LI_ISP, MUIM_List_Clear);
#ifdef INTERNAL_USER
   DoMethod(user_data->LI_User, MUIM_List_Clear);
#endif

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
   setcheckmark(modem_data->CH_RadBoogie, TRUE);
   setcheckmark(modem_data->CH_OwnDevUnit, FALSE);
   setcycle(modem_data->CY_Handshake, 0);

   setcheckmark(dialer_data->CH_ShowLamps, TRUE);
   setcheckmark(dialer_data->CH_ShowLog, TRUE);
   setcheckmark(dialer_data->CH_ShowConnect, TRUE);
   setcheckmark(dialer_data->CH_ShowOnlineTime, TRUE);
   setcheckmark(dialer_data->CH_ShowButtons, TRUE);
   setcheckmark(dialer_data->CH_ShowNetwork, TRUE);
   setcheckmark(dialer_data->CH_ShowUser, TRUE);
   setcycle(dialer_data->CY_MainWindow, 0);
   setcheckmark(dialer_data->CH_ShowStatusWin, TRUE);
   setcheckmark(dialer_data->CH_ShowSerialInput, TRUE);

   setcheckmark(dialer_data->CH_ConfirmOffline, FALSE);
   setcheckmark(dialer_data->CH_QuickReconnect, FALSE);
   setcheckmark(dialer_data->CH_Debug, FALSE);
   setcheckmark(dialer_data->CH_StartupInetd, TRUE);
   setcheckmark(dialer_data->CH_StartupLoopback, TRUE);
   setcheckmark(dialer_data->CH_StartupTCP, TRUE);

   setstring(dialer_data->STR_Startup, NULL);
   setstring(dialer_data->STR_Shutdown, NULL);

   return(NULL);
}

///
/// MainWindow_LoadDatabases
ULONG MainWindow_LoadDatabases(struct IClass *cl, Object *obj, Msg msg)
{
   struct MainWindow_Data *data              = INST_DATA(cl                      , obj);
//   struct Provider_Data    *provider_data    = INST_DATA(CL_Provider->mcc_Class  , data->GR_Provider);
   struct Modem_Data       *modem_data       = INST_DATA(CL_Modem->mcc_Class     , data->GR_Modem);
//   struct Dialer_Data      *dialer_data      = INST_DATA(CL_Dialer->mcc_Class    , data->GR_Dialer);
   struct Databases_Data   *db_data          = INST_DATA(CL_Databases->mcc_Class , data->GR_Databases);
#ifdef INTERNAL_USER
   struct User_Data        *user_data        = INST_DATA(CL_User->mcc_Class      , data->GR_User);
#endif
   struct ParseConfig_Data pc_data;
   char buffer[41];
   STRPTR ptr;

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
            if(!stricmp(pc_data.pc_argument, "Modem"))
            {
               if(*modem->Name)
               {
                  modem->ProtocolName[protocol_nr][0] = NULL;
                  DoMethod(modem_data->LI_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Sorted);
                  bzero(modem, sizeof(struct Modem));
               }
               strncpy(modem->Name, pc_data.pc_contents, sizeof(modem->Name));
               protocol_nr = 0;
            }
            if(!stricmp(pc_data.pc_argument, "Protocol"))
               strncpy(modem->ProtocolName[protocol_nr], pc_data.pc_contents, 20);

            if(!stricmp(pc_data.pc_argument, "InitString"))
            {
               strncpy(modem->InitString[protocol_nr], pc_data.pc_contents, 40);
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

#ifdef INTERNAL_USER
   /**** parse the passwd file ****/

   set(user_data->LI_User, MUIA_List_Quiet, TRUE);
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
                     inet_access->Access = (stricmp(pc_data.pc_contents, "allow") ? 0 : 1);
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
            if(ptr = strchr(pc_data.pc_contents, '#'))
               *ptr = NULL;
            if(ptr = strchr(pc_data.pc_contents, ';'))
               *ptr = NULL;

            if(pc_data.pc_contents = extract_arg(pc_data.pc_contents, network->Name, sizeof(network->Name), NULL))
            {
               pc_data.pc_contents = extract_arg(pc_data.pc_contents, buffer, sizeof(buffer), NULL);
               network->Number = atol(buffer);

               if(pc_data.pc_contents)
                  strncpy(network->Aliases, pc_data.pc_contents, sizeof(network->Aliases));
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
   DoMethod(data->GR_Databases, MUIM_Databases_SetStates, MUIV_Databases_SetStates_Rpcs);

   return(NULL);
}

///

/// MainWindow_LoadConfig
ULONG MainWindow_LoadConfig(struct IClass *cl, Object *obj, struct MUIP_MainWindow_LoadConfig *msg)
{
   struct MainWindow_Data *data              = INST_DATA(cl                      , obj);
   struct Provider_Data    *provider_data    = INST_DATA(CL_Provider->mcc_Class  , data->GR_Provider);
   struct Modem_Data       *modem_data       = INST_DATA(CL_Modem->mcc_Class     , data->GR_Modem);
   struct Dialer_Data      *dialer_data      = INST_DATA(CL_Dialer->mcc_Class    , data->GR_Dialer);
   struct ParseConfig_Data pc_data;
   STRPTR ptr;
   struct ISP *isp = NULL;
   struct Interface *iface = NULL;
   struct PrefsPPPIface *ppp_if = NULL;
   int current_type = NULL;

   /**** load providers ****/
   if(ParseConfig((msg->file ? msg->file : (STRPTR)config_file), &pc_data))
   {
      while(ParseNext(&pc_data))
      {
         if(!stricmp(pc_data.pc_argument, "ISP"))
         {
            current_type = type_ISP;
            DoMethod(provider_data->LI_ISP, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
            DoMethod(provider_data->LI_ISP, MUIM_List_GetEntry, xget(provider_data->LI_ISP, MUIA_List_Entries) - 1 , &isp);
            continue;
         }
         else if(!stricmp(pc_data.pc_argument, "INTERFACE"))
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
         else if(!stricmp(pc_data.pc_argument, "LOGINSCRIPT"))
         {
            current_type = type_LoginScript;
            continue;
         }

         else if(!stricmp(pc_data.pc_argument, "SerialDevice"))
            setstring(modem_data->STR_SerialDevice, pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "SerialUnit"))
            set(modem_data->STR_SerialUnit, MUIA_String_Integer, atol(pc_data.pc_contents));
         else if(!stricmp(pc_data.pc_argument, "BaudRate"))
            set(modem_data->STR_BaudRate, MUIA_String_Integer, atol(pc_data.pc_contents));
         else if(!stricmp(pc_data.pc_argument, "SerBufLen"))
            set(modem_data->STR_SerBufLen, MUIA_String_Integer, atol(pc_data.pc_contents));

         else if(!stricmp(pc_data.pc_argument, "Modem"))
            setstring(modem_data->STR_Modem, pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "InitString"))
            setstring(modem_data->STR_InitString, pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "DialPrefix"))
            setstring(modem_data->STR_DialPrefix, pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "DialSuffix"))
            setstring(modem_data->STR_DialSuffix, pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "RedialAttempts"))
            setslider(modem_data->SL_RedialAttempts, atol(pc_data.pc_contents));
         else if(!stricmp(pc_data.pc_argument, "RedialDelay"))
            setslider(modem_data->SL_RedialDelay, atol(pc_data.pc_contents));

         else if(!stricmp(pc_data.pc_argument, "IgnoreDSR") && is_true(&pc_data))
            setcheckmark(modem_data->CH_IgnoreDSR, TRUE);
         else if(!stricmp(pc_data.pc_argument, "7Wire"))
               setcycle(modem_data->CY_Handshake, (is_false(&pc_data) ? 2 : 0));
         else if(!stricmp(pc_data.pc_argument, "RadBoogie") && is_false(&pc_data))
            setcheckmark(modem_data->CH_RadBoogie, FALSE);
         else if(!stricmp(pc_data.pc_argument, "XonXoff") && is_true(&pc_data))
            setcycle(modem_data->CY_Handshake, 1);
         else if(!stricmp(pc_data.pc_argument, "OwnDevUnit") && is_true(&pc_data))
            setcheckmark(modem_data->CH_OwnDevUnit, TRUE);

         else if(!stricmp(pc_data.pc_argument, "QuickReconnect") && is_true(&pc_data))
            setcheckmark(dialer_data->CH_QuickReconnect, TRUE);
         else if(!stricmp(pc_data.pc_argument, "Debug") && is_true(&pc_data))
            setcheckmark(dialer_data->CH_Debug, TRUE);
         else if(!stricmp(pc_data.pc_argument, "ConfirmOffline") && is_true(&pc_data))
           setcheckmark(dialer_data->CH_ConfirmOffline, TRUE);
         else if(!stricmp(pc_data.pc_argument, "ShowLog") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowLog, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowLamps") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowLamps, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowConnect") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowConnect, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowOnlineTime") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowOnlineTime, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowButtons") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowButtons, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowNetwork") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowNetwork, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowUser") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowUser, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowStatusWin") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowStatusWin, FALSE);
         else if(!stricmp(pc_data.pc_argument, "ShowSerialInput") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_ShowSerialInput, FALSE);
         else if(!stricmp(pc_data.pc_argument, "StartupOpenWin") && is_false(&pc_data))
            setcycle(dialer_data->CY_MainWindow, 2);
         else if(!stricmp(pc_data.pc_argument, "StartupIconify") && is_true(&pc_data))
            setcycle(dialer_data->CY_MainWindow, 1);
         else if(!stricmp(pc_data.pc_argument, "StartupInetd") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_StartupInetd, FALSE);
         else if(!stricmp(pc_data.pc_argument, "StartupLoopback") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_StartupLoopback, FALSE);
         else if(!stricmp(pc_data.pc_argument, "StartupTCP") && is_false(&pc_data))
            setcheckmark(dialer_data->CH_StartupTCP, FALSE);

         else if(!stricmp(pc_data.pc_argument, "Startup") && strlen(pc_data.pc_contents) > 2)
         {
            setstring(dialer_data->STR_Startup, &pc_data.pc_contents[2]);
            setcycle(dialer_data->CY_Startup, *pc_data.pc_contents - 48);
         }
         else if(!stricmp(pc_data.pc_argument, "Shutdown") && strlen(pc_data.pc_contents) > 2)
         {
            setstring(dialer_data->STR_Shutdown, &pc_data.pc_contents[2]);
            setcycle(dialer_data->CY_Shutdown, *pc_data.pc_contents - 48);
         }

         else switch(current_type)
         {
            case type_ISP:
               if(isp)
               {
                  if(!stricmp(pc_data.pc_argument, "Name"))
                     strncpy(isp->isp_name, (*pc_data.pc_contents ? pc_data.pc_contents : (STRPTR)"noname ISP"), sizeof(isp->isp_name));
                  else if(!stricmp(pc_data.pc_argument, "Comment"))
                     strncpy(isp->isp_comment, pc_data.pc_contents, sizeof(isp->isp_comment));
                  else if(!stricmp(pc_data.pc_argument, "Login"))
                     strncpy(isp->isp_login, pc_data.pc_contents, sizeof(isp->isp_login));
                  else if(!stricmp(pc_data.pc_argument, "Password"))
                     decrypt(pc_data.pc_contents, isp->isp_password);

                  else if(!stricmp(pc_data.pc_argument, "Organisation"))
                     strncpy(isp->isp_organisation  , pc_data.pc_contents, sizeof(isp->isp_organisation));
                  else if(!stricmp(pc_data.pc_argument, "Phone"))
                     strncpy(isp->isp_phonenumber, pc_data.pc_contents, sizeof(isp->isp_phonenumber));

                  else if(!stricmp(pc_data.pc_argument, "HostName"))
                     strncpy(isp->isp_hostname, pc_data.pc_contents, sizeof(isp->isp_hostname));
                  else if(!stricmp(pc_data.pc_argument, "TimeServer"))
                     strncpy(isp->isp_timename, pc_data.pc_contents, sizeof(isp->isp_timename));
                  else if(!stricmp(pc_data.pc_argument, "DontQueryHostname") && is_true(&pc_data))
                     isp->isp_flags |= ISF_DontQueryHostname;
                  else if(!stricmp(pc_data.pc_argument, "GetTime") && is_true(&pc_data))
                     isp->isp_flags |= ISF_GetTime;
                  else if(!stricmp(pc_data.pc_argument, "SaveTime") && is_true(&pc_data))
                     isp->isp_flags |= ISF_SaveTime;
                  else if(!stricmp(pc_data.pc_argument, "NameServer"))
                  {
                     struct ServerEntry *server;

                     if(server = AllocVec(sizeof(struct ServerEntry), MEMF_ANY))
                     {
                        strncpy(server->se_name, pc_data.pc_contents, sizeof(server->se_name));
                        AddTail((struct List *)&isp->isp_nameservers, (struct Node *)server);
                     }
                  }
                  else if(!stricmp(pc_data.pc_argument, "DomainName"))
                  {
                     struct ServerEntry *server;

                     if(server = AllocVec(sizeof(struct ServerEntry), MEMF_ANY))
                     {
                        strncpy(server->se_name, pc_data.pc_contents, sizeof(server->se_name));
                        AddTail((struct List *)&isp->isp_domainnames, (struct Node *)server);
                     }
                  }
               }
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
                  else if(!stricmp(pc_data.pc_argument, "MTU"))
                     iface->if_MTU = atol(pc_data.pc_contents);
                  else if(!stricmp(pc_data.pc_argument, "IPAddr"))
                     strncpy(iface->if_addr, pc_data.pc_contents, sizeof(iface->if_addr));
                  else if(!stricmp(pc_data.pc_argument, "DestIP"))
                     strncpy(iface->if_dst, pc_data.pc_contents, sizeof(iface->if_dst));
                  else if(!stricmp(pc_data.pc_argument, "Gateway"))
                     strncpy(iface->if_gateway, pc_data.pc_contents, sizeof(iface->if_gateway));
                  else if(!stricmp(pc_data.pc_argument, "Netmask"))
                     strncpy(iface->if_netmask, pc_data.pc_contents, sizeof(iface->if_netmask));

                  else if(!stricmp(pc_data.pc_argument, "UseBOOTP") && is_true(&pc_data))
                     iface->if_flags |= IFL_BOOTP;
                  else if((!stricmp(pc_data.pc_argument, "AutoOnline") || !stricmp(pc_data.pc_argument, "AlwaysOnline")) && is_true(&pc_data))
                     iface->if_flags |= IFL_AutoOnline;
                  else if(!stricmp(pc_data.pc_argument, "DefaultPPP") && is_true(&pc_data))
                     iface->if_flags |= IFL_PPP;
                  else if(!stricmp(pc_data.pc_argument, "DefaultSLIP") && is_true(&pc_data))
                     iface->if_flags |= IFL_SLIP;
                  else if(!stricmp(pc_data.pc_argument, "KeepAlive"))
                     iface->if_keepalive = atol(pc_data.pc_contents);

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
                           strncpy(script_line->sl_contents, &pc_data.pc_contents[2], sizeof(script_line->sl_contents));
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
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_ParseIP]))
                  command = SL_ParseIP;
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_ParsePasswd]))
                  command = SL_ParsePasswd;
               else
                  command = -1;

               if(command != -1 && isp)
               {
                  if(script_line = AllocVec(sizeof(struct ScriptLine), MEMF_ANY))
                  {
                     script_line->sl_command = command;
                     strncpy(script_line->sl_contents, pc_data.pc_contents, sizeof(script_line->sl_contents));

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
   struct Modem_Data       *modem_data       = INST_DATA(CL_Modem->mcc_Class     , data->GR_Modem);
   struct Dialer_Data      *dialer_data      = INST_DATA(CL_Dialer->mcc_Class    , data->GR_Dialer);
   struct Databases_Data   *db_data          = INST_DATA(CL_Databases->mcc_Class , data->GR_Databases);
#ifdef INTERNAL_USER
   struct User_Data        *user_data        = INST_DATA(CL_User->mcc_Class      , data->GR_User);
#endif
//   struct User *user = NULL;
   struct ISP *isp = NULL;
   BPTR fh;
   char buff[110];
   STRPTR ptr;
   LONG pos;

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

   // save order is: services, passwd, group, genesis.conf (which initiates a RESET), inets.conf

   if(changed_services)
   {
      if(fh = Open("AmiTCP:db/services", MODE_NEWFILE))
      {
         struct Service *service;

         FPrintf(fh, "## This file was generated by GENESiS Prefs\n##\n");
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
      FPrintf(fh, "## This file was generated by GENESiS Prefs\n##\n\n");

      FPrintf(fh, "SerialDevice       %ls\n", xget(modem_data->STR_SerialDevice  , MUIA_String_Contents));
      FPrintf(fh, "SerialUnit         %ls\n", xget(modem_data->STR_SerialUnit    , MUIA_String_Contents));
      FPrintf(fh, "BaudRate           %ls\n", xget(modem_data->STR_BaudRate      , MUIA_String_Contents));
      FPrintf(fh, "SerBufLen          %ls\n\n", xget(modem_data->STR_SerBufLen   , MUIA_String_Contents));

      FPrintf(fh, "Modem              %ls\n", xget(modem_data->STR_Modem         , MUIA_String_Contents));
      FPrintf(fh, "InitString         %ls\n", xget(modem_data->STR_InitString    , MUIA_String_Contents));
      FPrintf(fh, "DialPrefix         %ls\n", xget(modem_data->STR_DialPrefix    , MUIA_String_Contents));
      FPrintf(fh, "DialSuffix         %ls\n", xget(modem_data->STR_DialSuffix    , MUIA_String_Contents));
      FPrintf(fh, "RedialAttempts     %ld\n", xget(modem_data->SL_RedialAttempts , MUIA_Numeric_Value));
      FPrintf(fh, "RedialDelay        %ld\n", xget(modem_data->SL_RedialDelay    , MUIA_Numeric_Value));
      FPrintf(fh, "IgnoreDSR          %ls\n", yes_no(xget(modem_data->CH_IgnoreDSR       , MUIA_Selected)));
      switch(xget(modem_data->CY_Handshake, MUIA_Cycle_Active))
      {
         case 0:
            FPrintf(fh, "7Wire              yes\n");
            break;
         case 1:
            FPrintf(fh, "7Wire              no\n");
            FPrintf(fh, "XonXoff            yes\n");
            break;
         case 2:
            FPrintf(fh, "7Wire              no\n");
            FPrintf(fh, "XonXoff            no\n");
            break;
      }
      FPrintf(fh, "RadBoogie          %ls\n", yes_no(xget(modem_data->CH_RadBoogie       , MUIA_Selected)));
      FPrintf(fh, "OwnDevUnit         %ls\n\n", yes_no(xget(modem_data->CH_OwnDevUnit    , MUIA_Selected)));
      FPrintf(fh, "QuickReconnect     %ls\n", yes_no(xget(dialer_data->CH_QuickReconnect , MUIA_Selected)));
      FPrintf(fh, "Debug              %ls\n", yes_no(xget(dialer_data->CH_Debug          , MUIA_Selected)));
      FPrintf(fh, "ConfirmOffline     %ls\n", yes_no(xget(dialer_data->CH_ConfirmOffline , MUIA_Selected)));
      FPrintf(fh, "ShowLog            %ls\n", yes_no(xget(dialer_data->CH_ShowLog        , MUIA_Selected)));
      FPrintf(fh, "ShowLamps          %ls\n", yes_no(xget(dialer_data->CH_ShowLamps      , MUIA_Selected)));
      FPrintf(fh, "ShowConnect        %ls\n", yes_no(xget(dialer_data->CH_ShowConnect    , MUIA_Selected)));
      FPrintf(fh, "ShowOnlineTime     %ls\n", yes_no(xget(dialer_data->CH_ShowOnlineTime , MUIA_Selected)));
      FPrintf(fh, "ShowButtons        %ls\n", yes_no(xget(dialer_data->CH_ShowButtons    , MUIA_Selected)));
      FPrintf(fh, "ShowNetwork        %ls\n", yes_no(xget(dialer_data->CH_ShowNetwork    , MUIA_Selected)));
      FPrintf(fh, "ShowUser           %ls\n", yes_no(xget(dialer_data->CH_ShowUser       , MUIA_Selected)));
      FPrintf(fh, "ShowStatusWin      %ls\n", yes_no(xget(dialer_data->CH_ShowStatusWin  , MUIA_Selected)));
      FPrintf(fh, "ShowSerialInput    %ls\n", yes_no(xget(dialer_data->CH_ShowSerialInput, MUIA_Selected)));
      switch(xget(dialer_data->CY_MainWindow, MUIA_Cycle_Active))
      {
         case 1:
            FPrintf(fh, "StartupIconify     yes\n");
            break;
         case 2:
            FPrintf(fh, "StartupOpenWin     no\n");
            break;
      }

      if(strlen((STRPTR)xget(dialer_data->STR_Startup      , MUIA_String_Contents)))
         FPrintf(fh, "Startup            %ld %ls\n", xget(dialer_data->CY_Startup, MUIA_Cycle_Active), xget(dialer_data->STR_Startup, MUIA_String_Contents));
      if(strlen((STRPTR)xget(dialer_data->STR_Shutdown     , MUIA_String_Contents)))
         FPrintf(fh, "Shutdown           %ld %ls\n", xget(dialer_data->CY_Shutdown, MUIA_Cycle_Active), xget(dialer_data->STR_Shutdown    , MUIA_String_Contents));
      FPrintf(fh, "StartupInetd       %ls\n", yes_no(xget(dialer_data->CH_StartupInetd  , MUIA_Selected)));
      FPrintf(fh, "StartupLoopback    %ls\n", yes_no(xget(dialer_data->CH_StartupLoopback, MUIA_Selected)));
      FPrintf(fh, "StartupTCP         %ls\n\n", yes_no(xget(dialer_data->CH_StartupTCP, MUIA_Selected)));

      FPrintf(fh, "\n\n## ISP and interface configuration\n##\n");

      pos = 0;
      FOREVER
      {
         DoMethod(provider_data->LI_ISP, MUIM_List_GetEntry, pos++, &isp);
         if(!isp)
            break;

         FPrintf(fh, "\nISP\n");
         FPrintf(fh, "Name               %ls\n", (*isp->isp_name ? isp->isp_name : "noname"));
         if(*isp->isp_comment)
            FPrintf(fh, "Comment            %ls\n", isp->isp_comment);
         if(*isp->isp_login)
            FPrintf(fh, "Login              %ls\n", isp->isp_login);
         if(*isp->isp_password)
         {
            encrypt(isp->isp_password, buff);
            FPrintf(fh, "Password           \"%ls\"\n", buff);
         }
         if(*isp->isp_organisation)
            FPrintf(fh, "Organisation       %ls\n", isp->isp_organisation);
         if(*isp->isp_phonenumber)
            FPrintf(fh, "Phone              %ls\n", isp->isp_phonenumber);
         if(*isp->isp_hostname)
            FPrintf(fh, "HostName           %ls\n", isp->isp_hostname);
         if(*isp->isp_timename)
            FPrintf(fh, "TimeServer         %ls\n", isp->isp_timename);

         if(isp->isp_flags & ISF_DontQueryHostname)
            FPrintf(fh, "DontQueryHostname  yes\n");
         if(isp->isp_flags & ISF_GetTime)
            FPrintf(fh, "GetTime            yes\n");
         if(isp->isp_flags & ISF_SaveTime)
            FPrintf(fh, "SaveTime           yes\n");

         if(isp->isp_nameservers.mlh_TailPred != (struct MinNode *)&isp->isp_nameservers)
         {
            struct ServerEntry *server;

            server = (struct ServerEntry *)isp->isp_nameservers.mlh_Head;
            while(server->se_node.mln_Succ)
            {
               FPrintf(fh, "NameServer         %ls\n", server->se_name);
               server = (struct ServerEntry *)server->se_node.mln_Succ;
            }
         }
         if(isp->isp_domainnames.mlh_TailPred != (struct MinNode *)&isp->isp_domainnames)
         {
            struct ServerEntry *server;

            server = (struct ServerEntry *)isp->isp_domainnames.mlh_Head;
            while(server->se_node.mln_Succ)
            {
               FPrintf(fh, "DomainName         %ls\n", server->se_name);
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
               FPrintf(fh, "IfName             %ls\n", (*iface->if_name ? iface->if_name : "xxx"));
               FPrintf(fh, "Sana2Device        %ls\n", iface->if_sana2device);
               FPrintf(fh, "Sana2Unit          %ld\n", iface->if_sana2unit);
               FPrintf(fh, "Sana2Config        %ls\n", iface->if_sana2config);
               if(iface->if_flags & IFL_PPP)
               {
                  FPrintf(fh, "Sana2ConfigText    \"sername %ls\\nserunit %ld\\nserbaud %ld\\nlocalipaddress %%a\\n",
                     xget(modem_data->STR_SerialDevice, MUIA_String_Contents),
                     xget(modem_data->STR_SerialUnit, MUIA_String_Integer),
                     xget(modem_data->STR_BaudRate, MUIA_String_Integer));
                  if(*isp->isp_login)
                     FPrintf(fh, "user \"%%u\"\\n");
                  if(*isp->isp_password)
                     FPrintf(fh, "secret \"%%p\"\\n");
                  if(xget(dialer_data->CH_Debug, MUIA_Selected))
                     FPrintf(fh, "debug = 1\\ndebug_window = 1\\nerror_requesters = 0\\nlog_file = t:appp.log\\n");
                  switch(xget(modem_data->CY_Handshake, MUIA_Cycle_Active))
                  {
                     case 1:
                        FPrintf(fh, "sevenwire      0\\n");
                        FPrintf(fh, "xonxoff        1\\n");
                        break;
                     case 2:
                        FPrintf(fh, "sevenwire      0\\n");
                        FPrintf(fh, "xonxoff        0\\n");
                        break;
                  }
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
                  FPrintf(fh, "Sana2ConfigText    %ls %ld %ld Shared %%a MTU=%ld",
                     xget(modem_data->STR_SerialDevice, MUIA_String_Contents),
                     xget(modem_data->STR_SerialUnit, MUIA_String_Integer),
                     xget(modem_data->STR_BaudRate, MUIA_String_Integer),
                     iface->if_MTU);

                  if(!xget(modem_data->CY_Handshake, MUIA_Cycle_Active))
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
                  FPrintf(fh, "Sana2ConfigText    ");

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
                  FPrintf(fh, "IfConfigParams     %ls\n", iface->if_configparams);
               FPrintf(fh, "MTU                %ld\n", iface->if_MTU);

               if(*iface->if_addr)
                  FPrintf(fh, "IPAddr             %ls\n", iface->if_addr);
               if(*iface->if_dst)
                  FPrintf(fh, "DestIP             %ls\n", iface->if_dst);
               if(*iface->if_gateway)
                  FPrintf(fh, "Gateway            %ls\n", iface->if_gateway);
               if(*iface->if_netmask)
                  FPrintf(fh, "Netmask            %ls\n", iface->if_netmask);

               if(iface->if_flags & IFL_BOOTP)
                  FPrintf(fh, "UseBOOTP\n");
               if(iface->if_flags & IFL_AutoOnline)
                  FPrintf(fh, "AutoOnline\n");
               if(iface->if_flags & IFL_PPP)
                  FPrintf(fh, "DefaultPPP\n");
               if(iface->if_flags & IFL_SLIP)
                  FPrintf(fh, "DefaultSLIP\n");
               if(iface->if_keepalive)
                  FPrintf(fh, "KeepAlive          %ld\n", iface->if_keepalive);

               if(ppp_if && ((iface->if_flags & IFL_PPP) || (iface->if_flags & IFL_SLIP)))
               {
                  FPrintf(fh, "CarrierDetect      %ls\n", yes_no(ppp_if->ppp_carrierdetect));
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
                     FPrintf(fh, "%ls %ld %ls\n", event_commands[event->sl_command], event->sl_userdata, event->sl_contents);
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
      Close(fh);
   }

   if(changed_hosts)
   {
      if(fh = Open("AmiTCP:db/hosts", MODE_NEWFILE))
      {
         struct Host *host;

         FPrintf(fh, "## This file was generated by GENESiS Prefs\n##\n");
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

         FPrintf(fh, "## This file was generated by GENESiS Prefs\n##\n");
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

         FPrintf(fh, "## This file was generated by GENESiS Prefs\n##\n");
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

         FPrintf(fh, "## This file was generated by GENESiS Prefs\n##\n");
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
         struct Network *network;

         FPrintf(fh, "## This file was generated by GENESiS Prefs\n##\n");
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

         FPrintf(fh, "## This file was generated by GENESiS Prefs\n##\n");
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
/// MainWindow_LoadOldConfig
ULONG MainWindow_LoadOldConfig(struct IClass *cl, Object *obj, struct MUIP_MainWindow_LoadOldConfig *msg)
{
   struct MainWindow_Data *data              = INST_DATA(cl                      , obj);
   struct Provider_Data    *provider_data    = INST_DATA(CL_Provider->mcc_Class  , data->GR_Provider);
   struct Modem_Data       *modem_data       = INST_DATA(CL_Modem->mcc_Class     , data->GR_Modem);
#ifdef INTERNAL_USER
   struct User_Data        *user_data        = INST_DATA(CL_User->mcc_Class      , data->GR_User);
   struct Prefs_User *p_user;
   BOOL found;
#endif
   struct ParseConfig_Data pc_data;
   struct ISP *isp = NULL;
   struct Interface *iface = NULL;
   struct PrefsPPPIface *ppp_if = NULL;
   char real_name[81];
   LONG pos;

   *real_name = NULL;
   DoMethod(provider_data->LI_ISP, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
   DoMethod(provider_data->LI_ISP, MUIM_List_GetEntry, xget(provider_data->LI_ISP, MUIA_List_Entries) - 1 , &isp);
   if(isp)
   {
      if(iface = AllocVec(sizeof(struct Interface), MEMF_ANY | MEMF_CLEAR))
      {
         NewList((struct List *)&iface->if_events);
         ppp_if = iface->if_userdata = AllocVec(sizeof(struct PrefsPPPIface), MEMF_ANY | MEMF_CLEAR);
         AddTail((struct List *)&isp->isp_ifaces, (struct Node *)iface);

         strcpy(isp->isp_comment, "old configuration");

         if(!msg->file)
            msg->file = "ENV:NetConfig/Provider.conf";

         if(ParseConfig(msg->file, &pc_data))
         {
            while(ParseNext(&pc_data))
            {
               if(!stricmp(pc_data.pc_argument, "Provider"))
                  strncpy(isp->isp_name, (*pc_data.pc_contents ? pc_data.pc_contents : (STRPTR)"noname ISP"), sizeof(isp->isp_name));
               else if(!stricmp(pc_data.pc_argument, "Phone"))
                  strncpy(isp->isp_phonenumber, pc_data.pc_contents, sizeof(isp->isp_phonenumber));
               else if(!stricmp(pc_data.pc_argument, "HostName"))
                  strncpy(isp->isp_hostname, pc_data.pc_contents, sizeof(isp->isp_hostname));
               else if(!stricmp(pc_data.pc_argument, "TimeServer"))
                  strncpy(isp->isp_timename, pc_data.pc_contents, sizeof(isp->isp_timename));
               else if(!stricmp(pc_data.pc_argument, "NameServer"))
               {
                  struct ServerEntry *server;

                  if(server = AllocVec(sizeof(struct ServerEntry), MEMF_ANY))
                  {
                     strncpy(server->se_name, pc_data.pc_contents, sizeof(server->se_name));
                     AddTail((struct List *)&isp->isp_nameservers, (struct Node *)server);
                  }
               }
               else if(!stricmp(pc_data.pc_argument, "DomainName"))
               {
                  struct ServerEntry *server;

                  if(server = AllocVec(sizeof(struct ServerEntry), MEMF_ANY))
                  {
                     strncpy(server->se_name, pc_data.pc_contents, sizeof(server->se_name));
                     AddTail((struct List *)&isp->isp_domainnames, (struct Node *)server);
                  }
               }
               if(!stricmp(pc_data.pc_argument, "Interface"))
                  strncpy(iface->if_name, pc_data.pc_contents, sizeof(iface->if_name));
               else if(!stricmp(pc_data.pc_argument, "IfnterfaceConfig") && !iface->if_configparams)
                  ReallocCopy((STRPTR *)&iface->if_configparams, pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "MTU"))
                  iface->if_MTU = atol(pc_data.pc_contents);
               else if(!stricmp(pc_data.pc_argument, "IPAddr"))
                  strncpy(iface->if_addr, pc_data.pc_contents, sizeof(iface->if_addr));
               else if(!stricmp(pc_data.pc_argument, "DestIP"))
                  strncpy(iface->if_dst, pc_data.pc_contents, sizeof(iface->if_dst));
            }
            ParseEnd(&pc_data);

            if(ParseConfig("ENV:NetConfig/User.conf", &pc_data))
            {
               while(ParseNext(&pc_data))
               {
                  if(!stricmp(pc_data.pc_argument, "LoginName"))
                     strncpy(isp->isp_login, pc_data.pc_contents, sizeof(isp->isp_login));
                  else if(!stricmp(pc_data.pc_argument, "Password"))
                     strncpy(isp->isp_password, pc_data.pc_contents, sizeof(isp->isp_password));
                  else if(!stricmp(pc_data.pc_argument, "RealName"))
                     strncpy(real_name, pc_data.pc_contents, sizeof(real_name));
                  else if(!stricmp(pc_data.pc_argument, "Device"))
                     setstring(modem_data->STR_SerialDevice, pc_data.pc_contents);
                  else if(!stricmp(pc_data.pc_argument, "Unit"))
                     set(modem_data->STR_SerialUnit, MUIA_String_Integer, atol(pc_data.pc_contents));
                  else if(!stricmp(pc_data.pc_argument, "Baud"))
                     set(modem_data->STR_BaudRate, MUIA_String_Integer, atol(pc_data.pc_contents));
                  else if(!stricmp(pc_data.pc_argument, "Modem"))
                     setstring(modem_data->STR_Modem, pc_data.pc_contents);
                  else if(!stricmp(pc_data.pc_argument, "ModemInit"))
                     setstring(modem_data->STR_InitString, pc_data.pc_contents);
                  else if(!stricmp(pc_data.pc_argument, "DialPrefix"))
                     setstring(modem_data->STR_DialPrefix, pc_data.pc_contents);
                  else if(!stricmp(pc_data.pc_argument, "RedialAttempts"))
                     setslider(modem_data->SL_RedialAttempts, atol(pc_data.pc_contents));
                  else if(!stricmp(pc_data.pc_argument, "RedialDelay"))
                     setslider(modem_data->SL_RedialDelay, atol(pc_data.pc_contents));
                  else if(!stricmp(pc_data.pc_argument, "7Wire"))
                     setcycle(modem_data->CY_Handshake, (is_false(&pc_data) ? 2 : 0));
                  else if(!stricmp(pc_data.pc_argument, "CarrierDetect") && is_true(&pc_data) && ppp_if)
                     ppp_if->ppp_carrierdetect = TRUE;
                  else if(!stricmp(pc_data.pc_argument, "OwnDevUnit") && is_true(&pc_data))
                     setcheckmark(modem_data->CH_OwnDevUnit, TRUE);
                  else if(!stricmp(pc_data.pc_argument, "Organisation"))
                     strncpy(isp->isp_organisation  , pc_data.pc_contents, sizeof(isp->isp_organisation));
               }
               ParseEnd(&pc_data);
            }
            else
               MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_CouldNotOpenX), "ENV:NetConfig/User.conf");

            if(!strstr(iface->if_name, "ppp") || !stricmp(iface->if_name, "PPP"))
            {
               iface->if_flags |= IFL_PPP;
               strcpy(iface->if_sana2device, "DEVS:Networks/appp.device");
               iface->if_sana2unit = 0;
               strcpy(iface->if_sana2config, "ENV:Sana2/appp0.config");
            }
            else if(!strstr(iface->if_name, "slip") || !stricmp(iface->if_name, "SLIP"))
            {
               iface->if_flags |= IFL_SLIP;
               strcpy(iface->if_sana2device, "DEVS:Networks/aslip.device");
               iface->if_sana2unit = 0;
               strcpy(iface->if_sana2config, "ENV:Sana2/aslip0.config");
            }
         }
         else
            MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_CouldNotOpenX), msg->file);

         if(ParseConfig("ENV:NetConfig/LoginScript", &pc_data))
         {
            struct ScriptLine *script_line;
            int command;

            if(script_line = AllocVec(sizeof(struct ScriptLine), MEMF_ANY))
            {
               script_line->sl_command = SL_Dial;
               *script_line->sl_contents = NULL;
               AddTail((struct List *)&isp->isp_loginscript, (struct Node *)script_line);
            }

            while(ParseNext(&pc_data))
            {
               if(!stricmp(pc_data.pc_argument, "SendLn") || !stricmp(pc_data.pc_argument, script_commands[SL_Send]))
                  command = SL_Send;
               else if(!stricmp(pc_data.pc_argument, script_commands[SL_WaitFor]))
                  command = SL_WaitFor;
               else if(strstr(pc_data.pc_contents, "send username"))
               {
                  command = SL_SendLogin;
                  ParseNext(&pc_data);
               }
               else if(strstr(pc_data.pc_contents, "send password"))
               {
                  command = SL_SendPassword;
                  ParseNext(&pc_data);
               }
               else
                  command = -1;

               if(command != -1)
               {
                  if(script_line = AllocVec(sizeof(struct ScriptLine), MEMF_ANY))
                  {
                     script_line->sl_command = command;
                     if(command == SL_WaitFor || command == SL_Send)
                        strncpy(script_line->sl_contents, pc_data.pc_contents, sizeof(script_line->sl_contents));
                     else
                        *script_line->sl_contents = NULL;

                     AddTail((struct List *)&isp->isp_loginscript, (struct Node *)script_line);
                  }
               }
            }
            if(script_line = AllocVec(sizeof(struct ScriptLine), MEMF_ANY))
            {
               script_line->sl_command = SL_GoOnline;
               *script_line->sl_contents = NULL;
               AddTail((struct List *)&isp->isp_loginscript, (struct Node *)script_line);
            }
            ParseEnd(&pc_data);
         }
         else
            MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_Okay), GetStr(MSG_TX_CouldNotOpenX), "ENV:NetConfig/LoginScript");
      }
      pos = 0;
#ifdef INTERNAL_USER
      found = FALSE;
      while(!found)
      {
         DoMethod(user_data->LI_User, MUIM_List_GetEntry, pos++, &p_user);
         if(!p_user)
            break;
         if(!stricmp(p_user->pu_login, isp->isp_login))
            found = TRUE;
      }
      if(!found)
      {
         if(p_user = AllocVec(sizeof(struct Prefs_User), MEMF_ANY | MEMF_CLEAR))
         {
            if(!UserGroupBase)
               UserGroupBase = OpenLibrary(USERGROUPNAME, 0);

            if(UserGroupBase)
            {
               strcpy(p_user->pu_password, crypt(isp->isp_password, isp->isp_login));
               CloseLibrary(UserGroupBase);
               UserGroupBase = NULL;
            }
            strcpy(p_user->pu_login, isp->isp_login);
            strcpy(p_user->pu_realname, real_name);
            p_user->pu_uid = 199;
            p_user->pu_gid = 100;
            sprintf(p_user->pu_homedir, "AmiTCP:Home/%ls", isp->isp_login);
            strcpy(p_user->pu_shell, "noshell");

            DoMethod(user_data->LI_User, MUIM_List_InsertSingle, p_user, MUIV_List_Insert_Bottom);
            changed_passwd = TRUE;

            FreeVec(p_user);
         }
      }
#endif
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
/// MainWindow_MenuImportOld
ULONG MainWindow_MenuImportOld(struct IClass *cl, Object *obj, Msg msg)
{
   STRPTR file;

   if(MUI_Request(app, win, NULL, NULL, GetStr(MSG_ReqBT_LoadCancel), GetStr(MSG_TX_LoadNC1Config)))
      if(file = getfilename(obj, GetStr(MSG_TX_ChooseConfigFileImport), "ENV:NetConfig/Provider.conf", FALSE))
         DoMethod(obj, MUIM_MainWindow_LoadOldConfig, file);

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

   data->GR_Provider       = data->GR_Dialer         = data->GR_Modem      =
   data->GR_Databases      = NULL;
#ifdef INTERNAL_USER
   data->GR_User           = NULL;
#endif

   if(data->GR_Info)
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&information_header, (ULONG *)information_colors , (UBYTE *)information_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_TX_GroupNames1), data->GR_Info, NULL, NULL, bodychunk);
   }
   if(data->GR_Provider        = NewObject(CL_Provider->mcc_Class      , NULL, TAG_DONE))
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&provider_header, (ULONG *)provider_colors , (UBYTE *)provider_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_TX_GroupNames2), data->GR_Provider, NULL, NULL, bodychunk);
   }
#ifdef INTERNAL_USER
   if(data->GR_User            = NewObject(CL_User->mcc_Class      , NULL, TAG_DONE))
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&user_header, (ULONG *)user_colors , (UBYTE *)user_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_TX_GroupNames3), data->GR_User, NULL, NULL, bodychunk);
   }
#endif
   if(data->GR_Dialer       = NewObject(CL_Dialer->mcc_Class     , NULL, TAG_DONE))
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&dialer_header, (ULONG *)dialer_colors , (UBYTE *)dialer_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_TX_GroupNames4), data->GR_Dialer, NULL, NULL, bodychunk);
   }
   if(data->GR_Modem      = NewObject(CL_Modem->mcc_Class    , NULL, TAG_DONE))
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&modem_header, (ULONG *)modem_colors , (UBYTE *)modem_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_TX_GroupNames5), data->GR_Modem, NULL, NULL, bodychunk);
   }
   if(data->GR_Databases   = NewObject(CL_Databases->mcc_Class , NULL, TAG_DONE))
   {
      bodychunk = create_bodychunk((struct BitMapHeader *)&databases_header, (ULONG *)databases_colors , (UBYTE *)databases_body);
      DoMethod(data->GR_Pager, MUIM_Grouppager_InsertImageGroup, GetStr(MSG_TX_GroupNames6), data->GR_Databases, NULL, NULL, bodychunk);
   }

#ifdef INTERNAL_USER
   if(data->GR_Provider && data->GR_Dialer && data->GR_Databases && data->GR_Modem && data->GR_User)
      success = TRUE;
#else
   if(data->GR_Provider && data->GR_Dialer && data->GR_Databases && data->GR_Modem)
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
      MUIA_Window_Title    , "GENESiSPrefs © 1997,98 by Michael Neuweiler & Active Technologies",
      MUIA_Window_ID       , MAKE_ID('G','P','R','F'),
      MUIA_Window_Height   , MUIV_Window_Height_MinMax(0),
      MUIA_Window_Width    , MUIV_Window_Width_MinMax(0),
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
      DoMethod((Object *)DoMethod(data->MN_Strip, MUIM_FindUData, MEN_IMPORTOLD) , MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
         obj, 1, MUIM_MainWindow_MenuImportOld);
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
      case MUIM_MainWindow_LoadOldConfig: return(MainWindow_LoadOldConfig (cl, obj, (APTR)msg));
      case MUIM_MainWindow_LoadDatabases: return(MainWindow_LoadDatabases (cl, obj, (APTR)msg));
      case MUIM_MainWindow_SaveConfig   : return(MainWindow_SaveConfig    (cl, obj, (APTR)msg));
      case MUIM_MainWindow_About        : return(MainWindow_About         (cl, obj, (APTR)msg));
      case MUIM_MainWindow_AboutFinish  : return(MainWindow_AboutFinish   (cl, obj, (APTR)msg));
      case MUIM_MainWindow_MenuLoad     : return(MainWindow_MenuLoad      (cl, obj, (APTR)msg));
      case MUIM_MainWindow_MenuImport   : return(MainWindow_MenuImport    (cl, obj, (APTR)msg));
      case MUIM_MainWindow_MenuImportOld: return(MainWindow_MenuImportOld (cl, obj, (APTR)msg));
      case MUIM_MainWindow_MenuSaveAs   : return(MainWindow_MenuSaveAs    (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

