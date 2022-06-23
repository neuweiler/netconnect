/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "Strings.h"
#include "mui.h"
#include "mui_IfaceWindow.h"
#include "mui_Interfaces.h"
#include "mui_MainWindow.h"
#include "mui_Modems.h"
#include "protos.h"

///
/// external variables
extern struct Hook strobjhook, des_hook;
extern struct Library *GenesisBase;
extern struct MUI_CustomClass *CL_MainWindow, *CL_Modems;
extern Object *win;

///

/// set_ip_addr
VOID set_ip_addr(STRPTR src, Object *str_obj, Object *cy_obj, STRPTR def)
{
   if(src && *src && strcmp(src, (def ? def : (STRPTR)"0.0.0.0")))
   {
      setstring(str_obj , src);
      setcycle(cy_obj   , 1);
   }
   else
   {
      setstring(str_obj, NULL);
      setcycle(cy_obj  , 0);
   }
}

///

/// IfaceWindow_CopyData
ULONG IfaceWindow_CopyData(struct IClass *cl, Object *obj, Msg msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   ULONG value, pos;
   struct ScriptLine *script_line;
   struct ServerEntry *server;
   struct Modem *modem;

   if(data->iface)
   {
      struct PrefsPPPIface *ppp_if = data->iface->if_userdata;

      strncpy(data->iface->if_name, (STRPTR)xget(data->STR_Name, MUIA_String_Contents), sizeof(data->iface->if_name));
      strncpy(data->iface->if_comment, (STRPTR)xget(data->STR_Comment, MUIA_String_Contents), sizeof(data->iface->if_comment));
      strncpy(data->iface->if_addr, (STRPTR)xget(data->STR_Address, MUIA_String_Contents), sizeof(data->iface->if_addr));
      if(xget(data->CY_Address, MUIA_Cycle_Active))
         strncpy(data->iface->if_addr, (STRPTR)xget(data->STR_Address, MUIA_String_Contents), sizeof(data->iface->if_addr));
      else
         *data->iface->if_addr = NULL;
      if(xget(data->CY_Dest, MUIA_Cycle_Active))
         strncpy(data->iface->if_dst, (STRPTR)xget(data->STR_Dest, MUIA_String_Contents), sizeof(data->iface->if_dst));
      else
         *data->iface->if_dst = NULL;
      if(xget(data->CY_Gateway, MUIA_Cycle_Active))
         strncpy(data->iface->if_gateway, (STRPTR)xget(data->STR_Gateway, MUIA_String_Contents), sizeof(data->iface->if_gateway));
      else
         *data->iface->if_gateway = NULL;
      if(xget(data->CY_Netmask, MUIA_Cycle_Active))
         strncpy(data->iface->if_netmask, (STRPTR)xget(data->STR_Netmask, MUIA_String_Contents), sizeof(data->iface->if_netmask));
      else
         *data->iface->if_netmask = NULL;
      ReallocCopy((STRPTR *)&data->iface->if_configparams, (STRPTR)xget(data->STR_ConfigParams, MUIA_String_Contents));
      data->iface->if_MTU = xget(data->STR_MTU, MUIA_String_Integer);

      value = xget(data->CY_Sana2Device, MUIA_Cycle_Active);
      data->iface->if_flags &= ~IFL_PPP;
      data->iface->if_flags &= ~IFL_SLIP;

      switch(value)
      {
         case 0:  /** ppp **/
            strcpy(data->iface->if_sana2device, "DEVS:Networks/appp.device");
            data->iface->if_sana2unit = 0;
            strcpy(data->iface->if_sana2config, "ENV:Sana2/appp0.config");
            ReallocCopy((STRPTR *)&data->iface->if_sana2configtext, "");
            data->iface->if_flags |= IFL_PPP;
            break;

         case 1:  /** slip **/
            strcpy(data->iface->if_sana2device, "DEVS:Networks/aslip.device");
            data->iface->if_sana2unit = 0;
            strcpy(data->iface->if_sana2config, "ENV:Sana2/aslip0.config");
            ReallocCopy((STRPTR *)&data->iface->if_sana2configtext, "");
            data->iface->if_flags |= IFL_SLIP;
            break;

         default:
            strncpy(data->iface->if_sana2device, (STRPTR)xget(data->STR_Sana2Device, MUIA_String_Contents), sizeof(data->iface->if_sana2device));
            data->iface->if_sana2unit = xget(data->STR_Sana2Unit, MUIA_String_Integer);
            strncpy(data->iface->if_sana2config, (STRPTR)xget(data->STR_Sana2ConfigFile, MUIA_String_Contents), sizeof(data->iface->if_sana2config));
            ReallocCopy((STRPTR *)&data->iface->if_sana2configtext, (STRPTR)xget(data->TI_Sana2ConfigText, MUIA_String_Contents));
            break;
      }
      if(value < 2 && data->iface->if_configparams)
      {
         FreeVec(data->iface->if_configparams);
         data->iface->if_configparams = NULL;
      }

      DoMethod(data->LI_Modems, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem);
      if(modem && (modem != (APTR)-1))
      {
         data->iface->if_modemid = modem->mo_id;
         strncpy(data->iface->if_login        , (STRPTR)xget(data->STR_Login   , MUIA_String_Contents), sizeof(data->iface->if_login));
         strncpy(data->iface->if_password     , (STRPTR)xget(data->STR_Password, MUIA_String_Contents), sizeof(data->iface->if_password));
         strncpy(data->iface->if_phonenumber  , (STRPTR)xget(data->STR_Phone   , MUIA_String_Contents), sizeof(data->iface->if_phonenumber));

         clear_list(&data->iface->if_loginscript);
         pos = NULL;
         FOREVER
         {
            DoMethod(data->LI_LoginScript, MUIM_List_GetEntry, pos++, &script_line);
            if(!script_line)
               break;
            add_script_line(&data->iface->if_loginscript, script_line->sl_command, script_line->sl_contents, script_line->sl_userdata);
         }
      }
      else
      {
         data->iface->if_modemid        = 0;
         *data->iface->if_login         = NULL;
         *data->iface->if_password      = NULL;
         *data->iface->if_phonenumber   = NULL;
         clear_list(&data->iface->if_loginscript);
      }

      if(xget(data->CH_UseBOOTP, MUIA_Selected))
         data->iface->if_flags |= IFL_BOOTP;
      else
         data->iface->if_flags &= ~IFL_BOOTP;
      if(xget(data->CH_UseHostName, MUIA_Selected))
         data->iface->if_flags |= IFL_UseHostName;
      else
         data->iface->if_flags &= ~IFL_UseHostName;
      if(xget(data->CH_UseDomainName, MUIA_Selected))
         data->iface->if_flags |= IFL_UseDomainName;
      else
         data->iface->if_flags &= ~IFL_UseDomainName;
      if(xget(data->CH_GetTime, MUIA_Selected))
         data->iface->if_flags |= IFL_GetTime;
      else
         data->iface->if_flags &= ~IFL_GetTime;
      if(xget(data->CH_SaveTime, MUIA_Selected))
         data->iface->if_flags |= IFL_SaveTime;
      else
         data->iface->if_flags &= ~IFL_SaveTime;
      strncpy(data->iface->if_timename, (STRPTR)xget(data->STR_TimeServer, MUIA_String_Contents), sizeof(data->iface->if_timename));
      data->iface->if_keepalive = xget(data->SL_KeepAlive, MUIA_Numeric_Value);
      if(xget(data->CH_AutoOnline, MUIA_Selected))
         data->iface->if_flags |= IFL_AutoOnline;
      else
         data->iface->if_flags &= ~IFL_AutoOnline;
      if(xget(data->CH_DialIn, MUIA_Selected))
         data->iface->if_flags |= IFL_DialIn;
      else
         data->iface->if_flags &= ~IFL_DialIn;

      if(ppp_if)
      {
         ppp_if->ppp_carrierdetect  = xget(data->CH_CarrierDetect     , MUIA_Selected);
         if(xget(data->CH_Callback, MUIA_Selected))
         {
            ppp_if->ppp_connecttimeout = xget(data->SL_ConnectTimeout   , MUIA_Numeric_Value);
            strcpy(ppp_if->ppp_callback, (STRPTR)xget(data->STR_Callback , MUIA_String_Contents));
         }
         else
         {
            ppp_if->ppp_connecttimeout = 0;
            *ppp_if->ppp_callback = NULL;
         }
         ppp_if->ppp_mppcomp        = xget(data->CH_MPPCompression      , MUIA_Selected);
         ppp_if->ppp_vjcomp         = xget(data->CH_VJCompression       , MUIA_Selected);
         ppp_if->ppp_bsdcomp        = xget(data->CH_BSDCompression      , MUIA_Selected);
         ppp_if->ppp_deflatecomp    = xget(data->CH_DeflateCompression  , MUIA_Selected);
         ppp_if->ppp_eof            = !xget(data->CY_EOF                , MUIA_Cycle_Active);
         if(xget(data->CH_UseDNS, MUIA_Selected))
            data->iface->if_flags |= IFL_UseNameServer;
         else
            data->iface->if_flags &= ~IFL_UseNameServer;
      }

      clear_list(&data->iface->if_events);
      pos = NULL;
      FOREVER
      {
         DoMethod(data->LI_Online, MUIM_List_GetEntry, pos++, &script_line);
         if(!script_line)
            break;
         add_script_line(&data->iface->if_events, IFE_Online, script_line->sl_contents, script_line->sl_userdata);
      }
      pos = NULL;
      FOREVER
      {
         DoMethod(data->LI_OnlineFail, MUIM_List_GetEntry, pos++, &script_line);
         if(!script_line)
            break;
         add_script_line(&data->iface->if_events, IFE_OnlineFail, script_line->sl_contents, script_line->sl_userdata);
      }
      pos = NULL;
      FOREVER
      {
         DoMethod(data->LI_OfflineActive, MUIM_List_GetEntry, pos++, &script_line);
         if(!script_line)
            break;
         add_script_line(&data->iface->if_events, IFE_OfflineActive, script_line->sl_contents, script_line->sl_userdata);
      }
      pos = NULL;
      FOREVER
      {
         DoMethod(data->LI_OfflinePassive, MUIM_List_GetEntry, pos++, &script_line);
         if(!script_line)
            break;
         add_script_line(&data->iface->if_events, IFE_OfflinePassive, script_line->sl_contents, script_line->sl_userdata);
      }

      clear_list(&data->iface->if_nameservers);
      pos = NULL;
      FOREVER
      {
         DoMethod(data->LI_NameServers, MUIM_List_GetEntry, pos++, &server);
         if(!server)
            break;
         add_server(&data->iface->if_nameservers, server->se_name);
      }
      clear_list(&data->iface->if_domainnames);
      pos = NULL;
      FOREVER
      {
         DoMethod(data->LI_DomainNames, MUIM_List_GetEntry, pos++, &server);
         if(!server)
            break;
         add_server(&data->iface->if_domainnames, server->se_name);
      }
      if(xget(data->CH_UseHostName, MUIA_Selected))
         *data->iface->if_hostname = NULL;
      else
         strncpy(data->iface->if_hostname, (STRPTR)xget(data->STR_HostName, MUIA_String_Contents), sizeof(data->iface->if_name));
   }

   return(NULL);
}

///
/// IfaceWindow_Init
ULONG IfaceWindow_Init(struct IClass *cl, Object *obj, struct MUIP_IfaceWindow_Init *msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
   struct Modems_Data *mo_data = INST_DATA(CL_Modems->mcc_Class, mw_data->GR_Modems);
   BOOL specify_sana2 = FALSE;
   struct Modem *modem;
   struct ScriptLine *script_line;
   struct ServerEntry *server;
   ULONG pos;

   if(data->iface = msg->iface)
   {
      struct PrefsPPPIface *ppp_if = data->iface->if_userdata;

      setstring(data->STR_Name     , data->iface->if_name);
      setstring(data->STR_Comment  , data->iface->if_comment);
      set_ip_addr(data->iface->if_addr    , data->STR_Address, data->CY_Address, NULL);
      set_ip_addr(data->iface->if_dst     , data->STR_Dest   , data->CY_Dest   , NULL);
      set_ip_addr(data->iface->if_gateway , data->STR_Gateway, data->CY_Gateway, NULL);
      set_ip_addr(data->iface->if_netmask , data->STR_Netmask, data->CY_Netmask, "255.255.255.255");
      setstring(data->STR_ConfigParams  , data->iface->if_configparams);
      set(data->STR_MTU                 , MUIA_String_Integer, data->iface->if_MTU);

      if(data->iface->if_flags & IFL_PPP)
         setcycle(data->CY_Sana2Device, 0);
      else if(data->iface->if_flags & IFL_SLIP)
         setcycle(data->CY_Sana2Device, 1);
      else
      {
         setcycle(data->CY_Sana2Device, 2);
         specify_sana2 = TRUE;
      }
      setstring(data->STR_Sana2Device    , data->iface->if_sana2device);
      set(data->STR_Sana2Unit            , MUIA_String_Integer, data->iface->if_sana2unit);
      setstring(data->STR_Sana2ConfigFile, data->iface->if_sana2config);
      setstring(data->TI_Sana2ConfigText , (data->iface->if_sana2configtext ? data->iface->if_sana2configtext : NULL));

      setcheckmark(data->CH_UseHostName   , data->iface->if_flags & IFL_UseHostName);
      setcheckmark(data->CH_UseDomainName , data->iface->if_flags & IFL_UseDomainName);
      setcheckmark(data->CH_UseBOOTP      , data->iface->if_flags & IFL_BOOTP);
      setcheckmark(data->CH_AutoOnline    , data->iface->if_flags & IFL_AutoOnline);
      setcheckmark(data->CH_DialIn        , data->iface->if_flags & IFL_DialIn);
      setcheckmark(data->CH_GetTime       , data->iface->if_flags & IFL_GetTime);
      setcheckmark(data->CH_SaveTime      , ((data->iface->if_flags & IFL_GetTime) && (data->iface->if_flags & IFL_SaveTime)));
      setstring(data->STR_TimeServer      , ((data->iface->if_flags & IFL_GetTime) ? data->iface->if_timename : NULL));
      setslider(data->SL_KeepAlive        , data->iface->if_keepalive);

      setcheckmark(data->CH_CarrierDetect     , ppp_if->ppp_carrierdetect);
      if(*ppp_if->ppp_callback)
      {
         setcheckmark(data->CH_Callback       , TRUE);
         setstring(data->STR_Callback         , ppp_if->ppp_callback);
         setslider(data->SL_ConnectTimeout    , ppp_if->ppp_connecttimeout);
      }
      setcheckmark(data->CH_MPPCompression    , ppp_if->ppp_mppcomp);
      setcheckmark(data->CH_VJCompression     , ppp_if->ppp_vjcomp);
      setcheckmark(data->CH_BSDCompression    , ppp_if->ppp_bsdcomp);
      setcheckmark(data->CH_DeflateCompression, ppp_if->ppp_deflatecomp);
      setcheckmark(data->CH_UseDNS            , (data->iface->if_flags & IFL_UseNameServer));
      setcycle(data->CY_EOF                   , !ppp_if->ppp_eof);

      if(data->iface->if_events.mlh_TailPred != (struct MinNode *)&data->iface->if_events)
      {
         script_line = (struct ScriptLine *)data->iface->if_events.mlh_Head;
         while(script_line->sl_node.mln_Succ)
         {
            switch(script_line->sl_command)
            {
               case IFE_Online:
                  DoMethod(data->LI_Online, MUIM_List_InsertSingle, script_line, MUIV_List_Insert_Bottom);
                  break;
               case IFE_OnlineFail:
                  DoMethod(data->LI_OnlineFail, MUIM_List_InsertSingle, script_line, MUIV_List_Insert_Bottom);
                  break;
               case IFE_OfflineActive:
                  DoMethod(data->LI_OfflineActive, MUIM_List_InsertSingle, script_line, MUIV_List_Insert_Bottom);
                  break;
               case IFE_OfflinePassive:
                  DoMethod(data->LI_OfflinePassive, MUIM_List_InsertSingle, script_line, MUIV_List_Insert_Bottom);
                  break;
            }
            script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
         }
      }

      DoMethod(data->LI_Modems, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Top);
      pos = 0;
      FOREVER
      {
         DoMethod(mo_data->LI_Modems, MUIM_List_GetEntry, pos++, &modem);
         if(!modem)
            break;
         DoMethod(data->LI_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Bottom);
         if(modem->mo_id == data->iface->if_modemid)
            set(data->LI_Modems, MUIA_List_Active, pos);
      }
      if(xget(data->LI_Modems, MUIA_List_Active) == MUIV_List_Active_Off)
         set(data->LI_Modems, MUIA_List_Active, ((specify_sana2 || (pos < 2)) ? MUIV_List_Active_Top : 1));

      setstring(data->STR_Phone, data->iface->if_phonenumber);
      setstring(data->STR_Login, data->iface->if_login);
      setstring(data->STR_Password, data->iface->if_password);

      if(data->iface->if_loginscript.mlh_TailPred != (struct MinNode *)&data->iface->if_loginscript)
      {
         script_line = (struct ScriptLine *)data->iface->if_loginscript.mlh_Head;
         while(script_line->sl_node.mln_Succ)
         {
            DoMethod(data->LI_LoginScript, MUIM_List_InsertSingle, script_line, MUIV_List_Insert_Bottom);
            script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
         }
      }
      if(data->iface->if_nameservers.mlh_TailPred != (struct MinNode *)&data->iface->if_nameservers)
      {
         server = (struct ServerEntry *)data->iface->if_nameservers.mlh_Head;
         while(server->se_node.mln_Succ)
         {
            DoMethod(data->LI_NameServers, MUIM_List_InsertSingle, server, MUIV_List_Insert_Bottom);
            server = (struct ServerEntry *)server->se_node.mln_Succ;
         }
      }
      if(data->iface->if_domainnames.mlh_TailPred != (struct MinNode *)&data->iface->if_domainnames)
      {
         server = (struct ServerEntry *)data->iface->if_domainnames.mlh_Head;
         while(server->se_node.mln_Succ)
         {
            DoMethod(data->LI_DomainNames, MUIM_List_InsertSingle, server, MUIV_List_Insert_Bottom);
            server = (struct ServerEntry *)server->se_node.mln_Succ;
         }
      }
      if(*data->iface->if_hostname)
         setstring(data->STR_HostName, data->iface->if_hostname);
   }
   return(NULL);
}

///
/// IfaceWindow_PopString_Close
ULONG IfaceWindow_PopString_Close(struct IClass *cl, Object *obj, struct MUIP_IfaceWindow_PopString_Close *msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   STRPTR ptr;
   LONG pos;
   STRPTR config_strings[] = {
      "",                                            // ppp
      "",                                            // slip
      "IPTYPE=2048 NOARP P2P IPREQ=8 WRITEREQ=8",    // plip0
      "IPTYPE=2048 NOARP P2P IPREQ=32 WRITEREQ=32",  // magplip0
      "",                                            // ether0
      "",                                            // a2065
      "",                                            // a4066
      "",                                            // hydra
      "",                                            // wd80xx
      "",                                            // eb920
      "NOQUICKIO",                                   // quicknet
      "",                                            // gg_3c503
      "",                                            // gg_ne1000
      "",                                            // gg_smc
      "NOTRACKING",                                  // arcnet0
      "NOTRACKING",                                  // a2060
      "IPTYPE=204 ARPTYPE=205 ARPREQ=3 IPREQ=16 WRITEREQ=16",  // ax25
      "IPTYPE=2048 NOARP P2P IPREQ=8 WRITEREQ=8",    // liana0
      "",                                            // ariadne
      "IPTYPE=2048 NOARP P2P IPREQ=8 WRITEREQ=8"     // ariadneliana0
      };

   if(msg->flags == MUIV_IfaceWindow_PopString_IfaceName)
   {
      if((pos = xget(data->LV_IfaceNames, MUIA_List_Active)) != MUIV_List_Active_Off)
      {
         DoMethod(data->LV_IfaceNames, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
         if(ptr)
         {
            setstring(data->STR_Name, ptr);
            setstring(data->STR_ConfigParams, config_strings[pos]);
         }
         DoMethod(data->PO_Name, MUIM_Popstring_Close, TRUE);
      }
   }

   return(NULL);
}

///
/// IfaceWindow_SanaActive
ULONG IfaceWindow_SanaActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   ULONG value;

   value = xget(data->CY_Sana2Device, MUIA_Cycle_Active);
   DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, value < 2,
      data->PA_Sana2Device, data->STR_Sana2Unit, data->PA_Sana2ConfigFile,
      data->TI_Sana2ConfigText, NULL);

   DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, value != 0,
      data->CH_MPPCompression, data->CH_VJCompression, data->CH_BSDCompression,
      data->CH_DeflateCompression, data->CY_EOF, data->CH_Callback, data->CH_UseDNS, NULL);

   DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, ((value != 0) || (!xget(data->CH_Callback, MUIA_Selected))),
      data->SL_ConnectTimeout, data->STR_Callback, NULL);

   set(data->CH_CarrierDetect, MUIA_Disabled, value > 1);

   return(NULL);
}

///
/// IfaceWindow_EventActive
ULONG IfaceWindow_EventActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   ULONG value;

   value = xget(data->CY_EventPage, MUIA_Cycle_Active);
   set(data->GR_EventPages, MUIA_Group_ActivePage, value);
   switch(value)
   {
      case IFE_Online:
         data->LI_Active = data->LI_Online;
         break;
      case IFE_OnlineFail:
         data->LI_Active = data->LI_OnlineFail;
         break;
      case IFE_OfflineActive:
         data->LI_Active = data->LI_OfflineActive;
         break;
      case IFE_OfflinePassive:
         data->LI_Active = data->LI_OfflinePassive;
         break;
   }
   DoMethod(obj, MUIM_IfaceWindow_SetEventStates);

   return(NULL);
}

///
/// IfaceWindow_SetEventStates
ULONG IfaceWindow_SetEventStates(struct IClass *cl, Object *obj, Msg msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   struct ScriptLine *event = NULL;

   if(data->LI_Active)
      DoMethod(data->LI_Active, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &event);

   if(event)
   {
      nnset(data->STR_Event   , MUIA_String_Contents  , event->sl_contents);
      nnset(data->CY_ExecType , MUIA_Cycle_Active     , event->sl_userdata);
   }
   else
   {
      nnset(data->STR_Event   , MUIA_String_Contents  , NULL);
      nnset(data->CY_ExecType , MUIA_Cycle_Active     , 0);
   }
   set(data->BT_RemoveEvent      , MUIA_Disabled, !event);
   set(data->PA_Event            , MUIA_Disabled, !event);
   set(data->CY_ExecType         , MUIA_Disabled, !event);
   set(win, MUIA_Window_ActiveObject, data->STR_Event);

   return(NULL);
}

///
/// IfaceWindow_NameserversActive
ULONG IfaceWindow_NameserversActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   struct ServerEntry *server;

   DoMethod(data->LI_NameServers, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &server);
   nnset(data->STR_NameServer, MUIA_String_Contents, (server ? server->se_name : NULL));

   set(data->STR_NameServer      , MUIA_Disabled, !server);
   set(data->BT_RemoveNameServer , MUIA_Disabled, !server);
   if(server)
      set(obj, MUIA_Window_ActiveObject, data->STR_NameServer);

   return(NULL);
}

///
/// IfaceWindow_DomainnamesActive
ULONG IfaceWindow_DomainnamesActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   struct ServerEntry *server;

   DoMethod(data->LI_DomainNames, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &server);
   nnset(data->STR_DomainName, MUIA_String_Contents, (server ? server->se_name : NULL));

   set(data->STR_DomainName      , MUIA_Disabled, !server);
   set(data->BT_RemoveDomainName , MUIA_Disabled, !server);
   if(server)
      set(obj, MUIA_Window_ActiveObject, data->STR_DomainName);
   return(NULL);
}

///
/// IfaceWindow_Modification
ULONG IfaceWindow_Modification(struct IClass *cl, Object *obj, struct MUIP_IfaceWindow_Modification *msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   struct ScriptLine *event = NULL;

   switch(msg->what)
   {
      case MUIV_IfaceWindow_Modification_AddEvent:
         if(data->LI_Active)
         {
            DoMethod(data->LI_Active, MUIM_NList_InsertSingle, -1, MUIV_NList_Insert_Bottom);
            set(data->LI_Active, MUIA_NList_Active, MUIV_NList_Active_Bottom);
         }
         break;

      case MUIV_IfaceWindow_Modification_RemoveEvent:
         if(data->LI_Active)
            DoMethod(data->LI_Active, MUIM_NList_Remove, MUIV_NList_Remove_Active);
         break;

      case MUIV_IfaceWindow_Modification_Event:
         if(data->LI_Active)
            DoMethod(data->LI_Active, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &event);
         if(event)
         {
            strcpy(event->sl_contents, (STRPTR)xget(data->STR_Event, MUIA_String_Contents));
            DoMethod(data->LI_Active, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
         }
         break;

      case MUIV_IfaceWindow_Modification_ExecType:
         if(data->LI_Active)
            DoMethod(data->LI_Active, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &event);
         if(event)
         {
            event->sl_userdata = xget(data->CY_ExecType, MUIA_Cycle_Active);
            DoMethod(data->LI_Active, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
         }
         break;

      case MUIV_IfaceWindow_Modification_AddScriptLine:
         DoMethod(data->LI_LoginScript, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
         set(data->LI_LoginScript, MUIA_List_Active, MUIV_List_Active_Bottom);
         break;

      case MUIV_IfaceWindow_Modification_ScriptLine:
      {
         struct ScriptLine *script_line;

         DoMethod(data->LI_LoginScript, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &script_line);
         if(script_line)
         {
            script_line->sl_command = xget(data->CY_ScriptCommand, MUIA_Cycle_Active);
            strncpy(script_line->sl_contents, (STRPTR)xget(data->STR_ScriptContents, MUIA_String_Contents), MAXPATHLEN);
            DoMethod(data->LI_LoginScript, MUIM_List_Redraw, MUIV_List_Redraw_Active);
            if(script_line->sl_command > SL_WaitFor && script_line->sl_command < SL_Exec)
               set(data->STR_ScriptContents, MUIA_Disabled, TRUE);
            else
               set(data->STR_ScriptContents, MUIA_Disabled, FALSE);
         }
      }
      break;

      case MUIV_IfaceWindow_Modification_NewModem:
      {
         struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
         struct Modems_Data *mo_data = INST_DATA(CL_Modems->mcc_Class, mw_data->GR_Modems);
         struct Modem *modem;

         DoMethod(mw_data->GR_Modems, MUIM_Modems_NewModem);
         DoMethod(mo_data->LI_Modems, MUIM_List_GetEntry, xget(mo_data->LI_Modems, MUIA_List_Entries) - 1, &modem);
         if(modem)
            DoMethod(data->LI_Modems, MUIM_List_InsertSingle, modem, MUIV_List_Insert_Bottom);
      }
      break;

      case MUIV_IfaceWindow_Modification_EditModem:
      case MUIV_IfaceWindow_Modification_DeleteModem:
      {
         struct MainWindow_Data *mw_data = INST_DATA(CL_MainWindow->mcc_Class, win);
         struct Modems_Data *mo_data = INST_DATA(CL_Modems->mcc_Class, mw_data->GR_Modems);
         struct Modem *modem1, *modem2;
         ULONG pos;

         DoMethod(data->LI_Modems, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem1);
         if(modem1 && (modem1 != (APTR)-1))
         {
            pos = 0;
            FOREVER
            {
               DoMethod(mo_data->LI_Modems, MUIM_List_GetEntry, pos++, &modem2);
               if(!modem2)
                  break;
               if(modem2->mo_id == modem1->mo_id)
               {
                  set(mo_data->LI_Modems, MUIA_List_Active, pos - 1);
                  if(msg->what == MUIV_IfaceWindow_Modification_DeleteModem)
                  {
                     DoMethod(data->LI_Modems, MUIM_List_Remove, MUIV_List_Remove_Active);
                     DoMethod(mo_data->LI_Modems, MUIM_List_Remove, MUIV_List_Remove_Active);
                  }
                  else
                     DoMethod(mw_data->GR_Modems, MUIM_Modems_Edit);
                  break;
               }
            }
         }
      }
      break;

      case MUIV_IfaceWindow_Modification_AddNameServer:
         DoMethod(data->LI_NameServers, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
         set(data->LI_NameServers, MUIA_List_Active, MUIV_List_Active_Bottom);
         break;

      case MUIV_IfaceWindow_Modification_NameServer:
      {
         struct ServerEntry *server;

         DoMethod(data->LI_NameServers, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &server);
         if(server)
         {
            strncpy(server->se_name, (STRPTR)xget(data->STR_NameServer, MUIA_String_Contents), 18);
            DoMethod(data->LI_NameServers, MUIM_List_Redraw, MUIV_List_Redraw_Active);
         }
      }
      break;

      case MUIV_IfaceWindow_Modification_AddDomainName:
         DoMethod(data->LI_DomainNames, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
         set(data->LI_DomainNames, MUIA_List_Active, MUIV_List_Active_Bottom);
         break;

      case MUIV_IfaceWindow_Modification_DomainName:
      {
         struct ServerEntry *server;

         DoMethod(data->LI_DomainNames, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &server);
         if(server)
         {
            strncpy(server->se_name, (STRPTR)xget(data->STR_DomainName, MUIA_String_Contents), 40);
            DoMethod(data->LI_DomainNames, MUIM_List_Redraw, MUIV_List_Redraw_Active);
         }
      }
      break;

   }

   return(NULL);
}   

///
/// IfaceWindow_ModemActive
ULONG IfaceWindow_ModemActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   struct Modem *modem;

   DoMethod(data->LI_Modems, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &modem);
   DoMethod(data->PO_Phone, MUIM_MultiSet, MUIA_Disabled, (!modem || (modem == (APTR)-1)),
      data->PO_Phone, data->STR_Login, data->STR_Password, data->BT_DeleteModem,
      data->BT_EditModem, data->LV_LoginScript, data->BT_AddScriptLine, NULL);
   DoMethod(data->BT_RemoveScriptLine, MUIM_MultiSet, MUIA_Disabled, (!modem || (modem == (APTR)-1) || (xget(data->LI_LoginScript, MUIA_List_Active) == MUIV_List_Active_Off)),
      data->BT_RemoveScriptLine, data->CY_ScriptCommand, data->STR_ScriptContents, NULL);

   return(NULL);
}

///
/// IfaceWindow_LoginScriptActive
ULONG IfaceWindow_LoginScriptActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   struct ScriptLine *script_line;

   DoMethod(data->LI_LoginScript, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &script_line);
   if(script_line)
   {
      nnset(data->CY_ScriptCommand, MUIA_Cycle_Active, script_line->sl_command);
      if(script_line->sl_command > SL_WaitFor && script_line->sl_command < SL_Exec)
      {
         nnset(data->STR_ScriptContents, MUIA_String_Contents, NULL);
         set(data->STR_ScriptContents, MUIA_Disabled, TRUE);
      }
      else
      {
         nnset(data->STR_ScriptContents, MUIA_String_Contents, script_line->sl_contents);
         set(data->STR_ScriptContents, MUIA_Disabled, FALSE);
         set(obj, MUIA_Window_ActiveObject, data->STR_ScriptContents);
      }
      set(data->CY_ScriptCommand, MUIA_Disabled, FALSE);
      set(data->BT_RemoveScriptLine, MUIA_Disabled, FALSE);
   }
   else
   {
      nnset(data->STR_ScriptContents, MUIA_String_Contents, NULL);
      set(data->STR_ScriptContents, MUIA_Disabled, TRUE);
      set(data->CY_ScriptCommand, MUIA_Disabled, TRUE);
      set(data->BT_RemoveScriptLine, MUIA_Disabled, TRUE);
   }
   return(NULL);
}

///
/// IfaceWindow_PopString_AddPhone
ULONG IfaceWindow_PopString_AddPhone(struct IClass *cl, Object *obj, struct MUIP_IfaceWindow_PopString_AddPhone *msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);

   if(msg->doit)
   {
      STRPTR ptr;

      if(ptr = (STRPTR)xget(data->STR_AddPhone, MUIA_String_Contents))
      {
         if(*ptr)
         {
            if(strlen((STRPTR)xget(data->STR_Phone, MUIA_String_Contents)))
               DoMethod(data->STR_Phone, MUIM_Textinput_AppendText, " | ", 3);
            DoMethod(data->STR_Phone, MUIM_Textinput_AppendText, ptr, strlen(ptr));
         }
      }
   }
   DoMethod(data->PO_Phone, MUIM_Popstring_Close, TRUE);
   set(data->STR_AddPhone, MUIA_String_Contents, NULL);

   return(NULL);
}


///

/// LoginScript_ConstructFunc
SAVEDS ASM struct ScriptLine *LoginScript_ConstructFunc(register __a2 APTR pool, register __a1 struct ScriptLine *src)
{
   struct ScriptLine *new;

   if((new = (struct ScriptLine *)AllocVec(sizeof(struct ScriptLine), MEMF_ANY | MEMF_CLEAR)) && src && (src != (APTR)-1))
      memcpy(new, src, sizeof(struct ScriptLine));
   else if(new)
      new->sl_command = SL_Send;

   return(new);
}
struct Hook LoginScript_ConstructHook= { { 0,0 }, (VOID *)LoginScript_ConstructFunc , NULL, NULL };

///
/// LoginScript_DisplayFunc
SAVEDS ASM LONG LoginScript_DisplayFunc(register __a2 char **array, register __a1 struct ScriptLine *script_line)
{
   if(script_line)
   {
      *array++ = script_commands[script_line->sl_command];
      *array = script_line->sl_contents;
   }
   else
   {
      *array++ = GetStr(MSG_TX_Command);
      *array   = GetStr(MSG_TX_String);
   }
   return(NULL);
}
struct Hook LoginScript_DisplayHook= { { 0,0 }, (VOID *)LoginScript_DisplayFunc , NULL, NULL };

///
/// EventList_ConstructFunc
SAVEDS ASM struct ScriptLine *EventList_ConstructFunc(register __a2 APTR pool, register __a1 struct ScriptLine *src)
{
   struct ScriptLine *new;

   if((new = (struct ScriptLine *)AllocVec(sizeof(struct ScriptLine), MEMF_ANY | MEMF_CLEAR)) && src && src != (APTR)-1)
      memcpy(new, src, sizeof(struct ScriptLine));
   return(new);
}
struct Hook EventList_ConstructHook= { { 0,0 }, (VOID *)EventList_ConstructFunc , NULL, NULL };

///
/// EventList_DisplayFunc
SAVEDS ASM LONG EventList_DisplayFunc(register __a2 char **array, register __a1 struct ScriptLine *event)
{
   if(event)
   {
      if(event->sl_userdata > EXEC_ARexx || event->sl_userdata < 0)
         event->sl_userdata = EXEC_CLI;

      *array++ = exec_types[event->sl_userdata];
      *array   = event->sl_contents;
   }
   return(NULL);
}
struct Hook EventList_DisplayHook= { { 0,0 }, (VOID *)EventList_DisplayFunc , NULL, NULL };

///
/// Modems_DisplayFunc
SAVEDS ASM LONG Modems_DisplayFunc(register __a2 char **array, register __a1 struct Modem *modem)
{
   if(modem)
   {
      if(modem == (APTR)-1)
      {
         *array++ = GetStr(MSG_TX_NoModem);
         *array   = GetStr(MSG_TX_NoModemComment);
      }
      else
      {
         *array++ = modem->mo_name;
         *array   = modem->mo_comment;
      }
   }
   else
   {
      *array++ = GetStr(MSG_TX_Name);
      *array   = GetStr(MSG_TX_Comment);
   }
   return(NULL);
}
struct Hook Modems_DisplayHook= { { 0,0 }, (VOID *)Modems_DisplayFunc , NULL, NULL };

///
/// ServerList_ConstructFunc
SAVEDS ASM struct ServerEntry *ServerList_ConstructFunc(register __a2 APTR pool, register __a1 struct ServerEntry *src)
{
   struct ServerEntry *new;

   if((new = (struct ServerEntry *)AllocVec(sizeof(struct ServerEntry), MEMF_ANY | MEMF_CLEAR)) && src && (src != (APTR)-1))
      memcpy(new, src, sizeof(struct ServerEntry));

   return(new);
}
struct Hook ServerList_ConstructHook= { { 0,0 }, (VOID *)ServerList_ConstructFunc , NULL, NULL };

///
/// ServerList_DisplayFunc
SAVEDS ASM LONG ServerList_DisplayFunc(register __a2 char **array, register __a1 struct ServerEntry *server)
{
   if(server)
      *array = server->se_name;
   else
      *array = "";

   return(NULL);
}
struct Hook ServerList_DisplayHook= { { 0,0 }, (VOID *)ServerList_DisplayFunc , NULL, NULL };

///

/// IfaceWindow_New
ULONG IfaceWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct IfaceWindow_Data tmp;
   static STRPTR STR_CY_Dynamic[3];
   static STRPTR STR_CY_Netmask[3];
   static STRPTR STR_CY_SanaDevice[4];
   static STRPTR STR_CY_EventPage[5];
   static STRPTR STR_CY_EOF[3];
   static STRPTR ARR_IfacePages[8];
   static STRPTR STR_PO_Interfaces[] = { "ppp", "slip", "plip0", "magplip0", "ether0", "a2065", "a4066",
         "hydra", "wd80xx", "eb920", "quicknet","gg_3c503", "gg_ne1000", "gg_smc", "arcnet0", "a2060",
         "ax25", "liana0", "ariadne", "ariadneliana0", NULL };
   Object *originator;

   originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

   ARR_IfacePages[0] = GetStr(MSG_TX_IfaceWindowRegister1);
   ARR_IfacePages[1] = GetStr(MSG_TX_IfaceWindowRegister2);
   ARR_IfacePages[2] = GetStr(MSG_TX_IfaceWindowRegister3);
   ARR_IfacePages[3] = GetStr(MSG_TX_IfaceWindowRegister4);
   ARR_IfacePages[4] = GetStr(MSG_TX_IfaceWindowRegister5);
   ARR_IfacePages[5] = GetStr(MSG_TX_IfaceWindowRegister6);
   ARR_IfacePages[6] = GetStr(MSG_TX_IfaceWindowRegister7);
   ARR_IfacePages[7] = NULL;

   STR_CY_Dynamic[0] = GetStr(MSG_TX_Dynamic);
   STR_CY_Dynamic[1] = GetStr(MSG_TX_Static);
   STR_CY_Dynamic[2] = NULL;

   STR_CY_Netmask[0] = GetStr(MSG_TX_Default);
   STR_CY_Netmask[1] = GetStr(MSG_TX_Specify);
   STR_CY_Netmask[2] = NULL;

   STR_CY_SanaDevice[0] = "PPP";
   STR_CY_SanaDevice[1] = "SLIP";
   STR_CY_SanaDevice[2] = GetStr(MSG_CY_SpecifySana);
   STR_CY_SanaDevice[3] = NULL;

   STR_CY_EventPage[0] = GetStr(MSG_CY_Online);
   STR_CY_EventPage[1] = GetStr(MSG_CY_OnlineFailed);
   STR_CY_EventPage[2] = GetStr(MSG_CY_OfflineActive);
   STR_CY_EventPage[3] = GetStr(MSG_CY_OfflinePassive);
   STR_CY_EventPage[4] = NULL;

   STR_CY_EOF[0] = GetStr(MSG_CY_EOFMode1);
   STR_CY_EOF[1] = GetStr(MSG_CY_EOFMode2);
   STR_CY_EOF[2] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_ID       , MAKE_ID('I','F','A','E'),
      MUIA_Window_Height   , MUIV_Window_Height_MinMax(0),
      MUIA_Window_Width    , MUIV_Window_Width_MinMax(0),
      WindowContents       , VGroup,
         Child, RegisterGroup(ARR_IfacePages),
            MUIA_CycleChain, 1,
            Child, VGroup,
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_Name, MSG_CC_Name),
                  Child, tmp.PO_Name = PopobjectObject,
                     MUIA_Popstring_String      , tmp.STR_Name = MakeKeyString(16, MSG_CC_Name),
                     MUIA_Popstring_Button      , PopButton(MUII_PopUp),
                     MUIA_Popobject_StrObjHook  , &strobjhook,
                     MUIA_Popobject_Object      , tmp.LV_IfaceNames = ListviewObject,
                        MUIA_Listview_DoubleClick  , TRUE,
                        MUIA_Listview_List         , ListObject,
                           MUIA_Frame              , MUIV_Frame_InputList,
                           MUIA_List_AutoVisible   , TRUE,
                           MUIA_List_SourceArray   , STR_PO_Interfaces,
                        End,
                     End,
                  End,
                  Child, MakeKeyLabel2(MSG_LA_Comment, MSG_CC_Comment),
                  Child, tmp.STR_Comment = MakeKeyString(40, MSG_CC_Comment),
                  Child, VGroup,
                     Child, MakeKeyLabel2(MSG_LA_Address    , MSG_CC_Address),
                     Child, MakeKeyLabel2(MSG_LA_Destination, MSG_CC_Destination),
                     Child, MakeKeyLabel2(MSG_LA_Gateway    , MSG_CC_Gateway),
                     Child, MakeKeyLabel2(MSG_LA_Netmask    , MSG_CC_Netmask),
                  End,
                  Child, ColGroup(2),
                     Child, tmp.STR_Address = TextinputObject,
                        StringFrame,
                        MUIA_ControlChar     , *GetStr(MSG_CC_Address),
                        MUIA_CycleChain      , 1,
                        MUIA_String_Accept   , "0123456789.",
                        MUIA_String_MaxLen   , 16,
                     End,
                     Child, tmp.CY_Address = Cycle(STR_CY_Dynamic),
                     Child, tmp.STR_Dest = TextinputObject,
                        StringFrame,
                        MUIA_ControlChar     , *GetStr(MSG_CC_Destination),
                        MUIA_CycleChain      , 1,
                        MUIA_String_Accept   , "0123456789.",
                        MUIA_String_MaxLen   , 16,
                     End,
                     Child, tmp.CY_Dest = Cycle(STR_CY_Dynamic),
                     Child, tmp.STR_Gateway = TextinputObject,
                        StringFrame,
                        MUIA_ControlChar     , *GetStr(MSG_CC_Gateway),
                        MUIA_CycleChain      , 1,
                        MUIA_String_Accept   , "0123456789.",
                        MUIA_String_MaxLen   , 16,
                     End,
                     Child, tmp.CY_Gateway = Cycle(STR_CY_Dynamic),
                     Child, tmp.STR_Netmask = TextinputObject,
                        StringFrame,
                        MUIA_ControlChar     , *GetStr(MSG_CC_Netmask),
                        MUIA_CycleChain      , 1,
                        MUIA_String_Accept   , "0123456789.",
                        MUIA_String_MaxLen   , 16,
                     End,
                     Child, tmp.CY_Netmask = Cycle(STR_CY_Netmask),
                  End,
                  Child, MakeKeyLabel2(MSG_LA_MTU, MSG_CC_MTU),
                  Child, HGroup,
                     Child, tmp.STR_MTU = MakeKeyInteger(6, MSG_CC_MTU),
                     Child, MakeKeyLabel2(MSG_LA_IfaceParams, MSG_CC_IfaceParams),
                     Child, tmp.STR_ConfigParams = MakeKeyString(1024, MSG_CC_IfaceParams),
                  End,
               End,
            End,

            Child, VGroup, // sana2
               Child, tmp.CY_Sana2Device = Cycle(STR_CY_SanaDevice),
               Child, HGroup,
                  Child, tmp.PA_Sana2Device = MakePopAsl(tmp.STR_Sana2Device = MakeKeyString(MAXPATHLEN, "   "), GetStr(MSG_TX_ChooseSanaDevice), FALSE),
                  Child, MakeKeyLabel2(MSG_LA_Unit, MSG_CC_Unit),
                  Child, tmp.STR_Sana2Unit = MakeKeyInteger(5, MSG_CC_Unit),
               End,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_ConfigFileTitle)),
               Child, tmp.PA_Sana2ConfigFile = MakePopAsl(tmp.STR_Sana2ConfigFile = MakeKeyString(MAXPATHLEN, "  f"), GetStr(MSG_TX_ChooseSanaConfigFile), FALSE),
               Child, tmp.TI_Sana2ConfigText = TextinputscrollObject,
                  MUIA_Weight, 5,
                  StringFrame,
                  MUIA_CycleChain, 1,
                  MUIA_Textinput_Multiline, TRUE,
                  MUIA_Textinput_WordWrap, 512,
                  MUIA_Textinput_AutoExpand, TRUE,
               End,
            End,

            Child, VGroup, // Modem
               Child, HGroup,
                  Child, VGroup,
                     Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_ModemTATitle)),
                     Child, VGroup,
                        GroupSpacing(0),
                        Child, tmp.LV_Modems = NListviewObject,
                           MUIA_CycleChain      , 1,
                           MUIA_NListview_NList , tmp.LI_Modems = NListObject,
                              MUIA_Frame              , MUIV_Frame_InputList,
                              MUIA_NList_DisplayHook  , &Modems_DisplayHook,
                              MUIA_NList_Format       ,"BAR,",
                              MUIA_NList_Title        , TRUE,
                           End,
                        End,
                        Child, HGroup,
                           GroupSpacing(0),
                           Child, tmp.BT_NewModem  = MakeButton(MSG_BT_New),
                           Child, tmp.BT_DeleteModem  = MakeButton(MSG_BT_Delete),
                           Child, tmp.BT_EditModem = MakeButton(MSG_BT_Edit),
                        End,
                     End,
                     Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_ProviderInfoTitle)),
                     Child, ColGroup(2),
                        Child, MakeKeyLabel2(MSG_LA_Phone, MSG_CC_Phone),
                        Child, tmp.PO_Phone = PopobjectObject,
                           MUIA_Popstring_String      , tmp.STR_Phone = MakeKeyString(100, MSG_CC_Phone),
                           MUIA_Popstring_Button      , PopButton(MUII_PopUp),
                           MUIA_Popobject_Object      , VGroup,
                              MUIA_Frame, MUIV_Frame_Group,
                              Child, KeyLLabel(GetStr(MSG_LA_AddNumber), *GetStr(MSG_CC_AddNumber)),
                              Child, tmp.STR_AddPhone = MakeKeyString(30, MSG_CC_AddNumber),
                              Child, HGroup,
                                 Child, tmp.BT_AddPhone = MakeButton(MSG_BT_Add),
                                 Child, tmp.BT_CancelPhone = MakeButton(MSG_BT_Cancel),
                              End,
                           End,
                        End,
                        Child, MakeKeyLabel2(MSG_LA_Login, MSG_CC_Login),
                        Child, tmp.STR_Login    = MakeKeyString(40, MSG_CC_Login),
                        Child, MakeKeyLabel2(MSG_LA_Password, MSG_CC_Password),
                        Child, tmp.STR_Password = TextinputObject,
                           StringFrame,
                           MUIA_ControlChar     , *GetStr(MSG_CC_Password),
                           MUIA_CycleChain      , 1,
                           MUIA_String_Secret   , TRUE,
                           MUIA_String_MaxLen   , 40,
                        End,
                     End,
                  End,
                  Child, BalanceObject, End,
                  Child, VGroup,
                     Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_LoginScriptTitle)),
                     Child, VGroup,
                        GroupSpacing(0),
                        Child, tmp.LV_LoginScript = NListviewObject,
                           MUIA_CycleChain      , 1,
                           MUIA_NListview_NList , tmp.LI_LoginScript = NListObject,
                              MUIA_Frame              , MUIV_Frame_InputList,
                              MUIA_NList_DragType     , MUIV_NList_DragType_Default,
                              MUIA_NList_ConstructHook, &LoginScript_ConstructHook,
                              MUIA_NList_DisplayHook  , &LoginScript_DisplayHook,
                              MUIA_NList_DestructHook , &des_hook,
                              MUIA_NList_DragSortable , TRUE,
                              MUIA_NList_Format       , "BAR,",
                              MUIA_NList_Title        , TRUE,
                           End,
                        End,
                        Child, HGroup,
                           GroupSpacing(0),
                           Child, tmp.BT_AddScriptLine    = MakeButton(MSG_BT_Add),
                           Child, tmp.BT_RemoveScriptLine = MakeButton(MSG_BT_Remove),
                        End,
                     End,
                     Child, HGroup,
                        Child, tmp.CY_ScriptCommand = Cycle(script_commands),
                        Child, tmp.STR_ScriptContents = MakeKeyString(MAXPATHLEN, "  l"),
                     End,
                  End,
               End,
            End,

            Child, VGroup, // ppp
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_PPPOptionsTitle)),
               Child, ColGroup(5),
                  Child, MakeKeyLabel2(MSG_LA_CarrierDetect, MSG_CC_CarrierDetect),
                  Child, tmp.CH_CarrierDetect = MakeKeyCheckMark(FALSE, MSG_CC_CarrierDetect),
                  Child, HVSpace,
                  Child, MakeKeyLabel2(MSG_LA_EOFMode, MSG_CC_EOFMode),
                  Child, tmp.CY_EOF = MakeKeyCycle(STR_CY_EOF, MSG_CC_EOFMode),
                  Child, MakeKeyLabel2(MSG_LA_MPPCompression, MSG_CC_MPPCompression),
                  Child, tmp.CH_MPPCompression = MakeKeyCheckMark(FALSE, MSG_CC_MPPCompression),
                  Child, HVSpace,
                  Child, MakeKeyLabel2(MSG_LA_BSDCompression, MSG_CC_BSDCompression),
                  Child, HGroup,
                     Child, tmp.CH_BSDCompression = MakeKeyCheckMark(FALSE, MSG_CC_BSDCompression),
                     Child, HVSpace,
                  End,
                  Child, MakeKeyLabel2(MSG_LA_VJCompression, MSG_CC_VJCompression),
                  Child, tmp.CH_VJCompression = MakeKeyCheckMark(FALSE, MSG_CC_VJCompression),
                  Child, HVSpace,
                  Child, MakeKeyLabel2(MSG_LA_DeflateCompression, MSG_CC_DeflateCompression),
                  Child, HGroup,
                     Child, tmp.CH_DeflateCompression = MakeKeyCheckMark(FALSE, MSG_CC_DeflateCompression),
                     Child, HVSpace,
                  End,
                  Child, MakeKeyLabel2(MSG_LA_UseDNSAddress, MSG_CC_UseDNSAddress),
                  Child, tmp.CH_UseDNS = MakeKeyCheckMark(FALSE, MSG_CC_UseDNSAddress),
                  Child, HVSpace,
                  Child, VVSpace,
                  Child, HVSpace,
               End,
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_CallbackSettingsTitle)),
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_Callback, MSG_CC_Callback),
                  Child, HGroup,
                     GroupSpacing(1),
                     Child, tmp.CH_Callback = MakeKeyCheckMark(FALSE, MSG_CC_Callback),
                     Child, tmp.STR_Callback = MakeKeyString(80, MSG_CC_Callback),
                  End,
                  Child, MakeKeyLabel2(MSG_LA_Timeout, MSG_CC_Timeout),
                  Child, HGroup,
                     Child, tmp.SL_ConnectTimeout = MUI_MakeObject(MUIO_NumericButton, NULL, 5, 360, GetStr(MSG_TX_TimeoutSlider)),
                     Child, HVSpace,
                  End,
               End,
               Child, HVSpace,
            End,

            Child, VGroup, // Services
               Child, HVSpace,
               Child, ColGroup(2),
                  Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_TimeTitle)),
                  Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_MiscTitle)),
                  Child, ColGroup(2),
                     Child, MakeKeyLabel2(MSG_LA_SyncClock, MSG_CC_SyncClock),
                     Child, HGroup,
                        Child, tmp.CH_GetTime = MakeKeyCheckMark(FALSE, MSG_CC_SyncClock),
                        Child, HVSpace,
                     End,
                     Child, MakeKeyLabel2(MSG_LA_SaveTime, MSG_CC_SaveTime),
                     Child, HGroup,
                        Child, tmp.CH_SaveTime = MakeKeyCheckMark(FALSE, MSG_CC_SaveTime),
                        Child, HVSpace,
                     End,
                     Child, MakeKeyLabel2(MSG_LA_TimeServer, MSG_CC_TimeServer),
                     Child, tmp.STR_TimeServer     = MakeKeyString(64, MSG_CC_TimeServer),
                  End,
                  Child, VGroup,
                     MUIA_Weight, 10,
                     Child, ColGroup(4),
                        Child, HVSpace,
                        Child, MakeKeyLabel2(MSG_LA_UseBOOTP, MSG_CC_UseBOOTP),
                        Child, tmp.CH_UseBOOTP = MakeKeyCheckMark(FALSE, MSG_CC_UseBOOTP),
                        Child, HVSpace,
                        Child, HVSpace,
                        Child, MakeKeyLabel2(MSG_LA_AutoOnline, MSG_CC_AutoOnline),
                        Child, tmp.CH_AutoOnline = MakeKeyCheckMark(FALSE, MSG_CC_AutoOnline),
                        Child, HVSpace,
                        Child, HVSpace,
                        Child, MakeKeyLabel2(MSG_LA_DialIn, MSG_CC_DialIn),
                        Child, tmp.CH_DialIn = MakeKeyCheckMark(FALSE, MSG_CC_DialIn),
                        Child, HVSpace,
                     End,
                     Child, HVSpace,
                  End,
               End,
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_KeepAliveTitle)),
               Child, HGroup,
                  Child, Label(GetStr(MSG_TX_PingEvery)),
                  Child, tmp.SL_KeepAlive = MUI_MakeObject(MUIO_NumericButton, NULL, 0, 60, "  %ld"),
                  Child, Label(GetStr(MSG_TX_Minutes)),
                  Child, HVSpace,
               End,
               Child, HVSpace,
            End,

            Child, VGroup, // Events
               Child, tmp.CY_EventPage = Cycle(STR_CY_EventPage),
               Child, VGroup,
                  GroupSpacing(0),
                  Child, tmp.GR_EventPages = VGroup,
                     MUIA_Group_PageMode, TRUE,
                     Child, tmp.LV_Online = NListviewObject,
                        MUIA_CycleChain      , 1,
                        MUIA_NListview_NList , tmp.LI_Active = tmp.LI_Online = NListObject,
                           MUIA_Frame               , MUIV_Frame_InputList,
                           MUIA_NList_DragType      , MUIV_NList_DragType_Default,
                           MUIA_NList_DragSortable  , TRUE,
                           MUIA_NList_DisplayHook   , &EventList_DisplayHook,
                           MUIA_NList_ConstructHook , &EventList_ConstructHook,
                           MUIA_NList_DestructHook  , &des_hook,
                           MUIA_NList_Format        , "BAR,",
                           MUIA_NList_Title         , FALSE,
                        End,
                     End,
                     Child, tmp.LV_OnlineFail = NListviewObject,
                        MUIA_CycleChain      , 1,
                        MUIA_NListview_NList , tmp.LI_OnlineFail = NListObject,
                           MUIA_Frame               , MUIV_Frame_InputList,
                           MUIA_NList_DragType      , MUIV_NList_DragType_Default,
                           MUIA_NList_DragSortable  , TRUE,
                           MUIA_NList_DisplayHook   , &EventList_DisplayHook,
                           MUIA_NList_ConstructHook , &EventList_ConstructHook,
                           MUIA_NList_DestructHook  , &des_hook,
                           MUIA_NList_Format        , "BAR,",
                           MUIA_NList_Title         , FALSE,
                        End,
                     End,
                     Child, tmp.LV_OfflineActive = NListviewObject,
                        MUIA_CycleChain      , 1,
                        MUIA_NListview_NList , tmp.LI_OfflineActive = NListObject,
                           MUIA_Frame               , MUIV_Frame_InputList,
                           MUIA_NList_DragType      , MUIV_NList_DragType_Default,
                           MUIA_NList_DragSortable  , TRUE,
                           MUIA_NList_DisplayHook   , &EventList_DisplayHook,
                           MUIA_NList_ConstructHook , &EventList_ConstructHook,
                           MUIA_NList_DestructHook  , &des_hook,
                           MUIA_NList_Format        , "BAR,",
                           MUIA_NList_Title         , FALSE,
                        End,
                     End,
                     Child, tmp.LV_OfflinePassive = NListviewObject,
                        MUIA_CycleChain      , 1,
                        MUIA_NListview_NList , tmp.LI_OfflinePassive = NListObject,
                           MUIA_Frame               , MUIV_Frame_InputList,
                           MUIA_NList_DragType      , MUIV_NList_DragType_Default,
                           MUIA_NList_DragSortable  , TRUE,
                           MUIA_NList_DisplayHook   , &EventList_DisplayHook,
                           MUIA_NList_ConstructHook , &EventList_ConstructHook,
                           MUIA_NList_DestructHook  , &des_hook,
                           MUIA_NList_Format        , "BAR,",
                           MUIA_NList_Title         , FALSE,
                        End,
                     End,
                  End,
                  Child, HGroup,
                     GroupSpacing(0),
                     Child, tmp.BT_AddEvent     = MakeButton(MSG_BT_Add),
                     Child, tmp.BT_RemoveEvent  = MakeButton(MSG_BT_Remove),
                  End,
               End,
               Child, HGroup,
                  Child, tmp.PA_Event = MakePopAsl(tmp.STR_Event = MakeKeyString(MAXPATHLEN, "  x"), MSG_TX_ChooseFile, FALSE),
                  Child, tmp.CY_ExecType = Cycle(exec_types),
               End,
            End,

            Child, VGroup, // resolv
               Child, ColGroup(2),
                  MUIA_Weight, 500,
                  Child, VGroup,
                     Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_DomainNameServersTitle)),
                     Child, VGroup, // Nameservers
                        GroupSpacing(0),
                        Child, tmp.LV_NameServers = ListviewObject,
                           MUIA_CycleChain         , 1,
                           MUIA_Listview_DragType  , MUIV_Listview_DragType_Immediate,
                           MUIA_Listview_List      , tmp.LI_NameServers = ListObject,
                              MUIA_Frame             , MUIV_Frame_InputList,
                              MUIA_List_ConstructHook, &ServerList_ConstructHook,
                              MUIA_List_DestructHook , &des_hook,
                              MUIA_List_DisplayHook  , &ServerList_DisplayHook,
                              MUIA_List_DragSortable , TRUE,
                           End,
                        End,
                        Child, HGroup,
                           GroupSpacing(0),
                           Child, tmp.BT_AddNameServer    = MakeButton(MSG_BT_Add),
                           Child, tmp.BT_RemoveNameServer = MakeButton(MSG_BT_Remove),
                        End,
                        Child, tmp.STR_NameServer = TextinputObject,
                           StringFrame,
                           MUIA_CycleChain      , 1,
                           MUIA_String_Accept   , "0123456789.",
                           MUIA_String_MaxLen   , 16,
                        End,
                     End,
                  End,
                  Child, VGroup,
                     Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_DomainNamesTitle)),
                     Child, VGroup, // Domainnames
                        GroupSpacing(0),
                        Child, tmp.LV_DomainNames = ListviewObject,
                           MUIA_CycleChain         , 1,
                           MUIA_Listview_DragType  , MUIV_Listview_DragType_Immediate,
                           MUIA_Listview_List      , tmp.LI_DomainNames = ListObject,
                              MUIA_Frame             , MUIV_Frame_InputList,
                              MUIA_List_ConstructHook, &ServerList_ConstructHook,
                              MUIA_List_DestructHook , &des_hook,
                              MUIA_List_DisplayHook  , &ServerList_DisplayHook,
                              MUIA_List_DragSortable , TRUE,
                           End,
                        End,
                        Child, HGroup,
                           GroupSpacing(0),
                           Child, tmp.BT_AddDomainName    = MakeButton(MSG_BT_Add2),
                           Child, tmp.BT_RemoveDomainName = MakeButton(MSG_BT_Remove2),
                        End,
                        Child, tmp.STR_DomainName = TextinputObject,
                           StringFrame,
                           MUIA_CycleChain      , 1,
                           MUIA_String_MaxLen   , 40,
                        End,
                     End,
                  End,
               End,
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_LocalHostNameTitle)),
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_QueryHostName, MSG_CC_QueryHostName),
                  Child, HGroup,
                     Child, tmp.CH_UseHostName = MakeKeyCheckMark(FALSE, MSG_CC_QueryHostName),
                     Child, HVSpace,
                     Child, MakeKeyLabel2(MSG_LA_QueryDomainName, MSG_CC_QueryDomainName),
                     Child, tmp.CH_UseDomainName = MakeKeyCheckMark(FALSE, MSG_CC_QueryDomainName),
                  End,
                  Child, MakeKeyLabel2(MSG_LA_HostName, MSG_CC_HostName),
                  Child, tmp.STR_HostName = MakeKeyString(64, MSG_CC_HostName),
               End,
               Child, HVSpace,
            End,
         End,
         Child, HGroup,
            Child, tmp.BT_Okay   = MakeButton(MSG_BT_Okay),
            Child, tmp.BT_Cancel = MakeButton(MSG_BT_Cancel),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct IfaceWindow_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->iface = NULL;

set(data->CH_DialIn, MUIA_Disabled, TRUE);
      set(data->SL_ConnectTimeout, MUIA_ControlChar, GetStr(MSG_CC_Timeout));
      setslider(data->SL_ConnectTimeout, 50);

      set(data->CY_Address       , MUIA_Weight, 0);
      set(data->CY_Dest          , MUIA_Weight, 0);
      set(data->CY_Gateway       , MUIA_Weight, 0);
      set(data->CY_Netmask       , MUIA_Weight, 0);
      set(data->CY_ScriptCommand , MUIA_Weight, 0);
      set(data->CY_ExecType      , MUIA_Weight, 0);
      set(data->STR_MTU          , MUIA_Weight, 30);
      set(data->STR_Sana2Unit    , MUIA_Weight, 2);

      set(data->CY_Address       , MUIA_CycleChain, 1);
      set(data->CY_Dest          , MUIA_CycleChain, 1);
      set(data->CY_Gateway       , MUIA_CycleChain, 1);
      set(data->CY_Netmask       , MUIA_CycleChain, 1);
      set(data->CY_Sana2Device   , MUIA_CycleChain, 1);
      set(data->SL_KeepAlive     , MUIA_CycleChain, 1);
      set(data->SL_ConnectTimeout, MUIA_CycleChain, 1);
      set(data->CY_EventPage     , MUIA_CycleChain, 1);
      set(data->CY_ExecType      , MUIA_CycleChain, 1);

      set(data->STR_Address         , MUIA_Disabled, TRUE);
      set(data->STR_Dest            , MUIA_Disabled, TRUE);
      set(data->STR_Gateway         , MUIA_Disabled, TRUE);
      set(data->STR_Netmask         , MUIA_Disabled, TRUE);
      set(data->PA_Sana2Device      , MUIA_Disabled, TRUE);
      set(data->STR_Sana2Unit       , MUIA_Disabled, TRUE);
      set(data->PA_Sana2ConfigFile  , MUIA_Disabled, TRUE);
      set(data->TI_Sana2ConfigText  , MUIA_Disabled, TRUE);
      set(data->BT_RemoveScriptLine , MUIA_Disabled, TRUE);
      set(data->CY_ScriptCommand    , MUIA_Disabled, TRUE);
      set(data->STR_ScriptContents  , MUIA_Disabled, TRUE);
      set(data->STR_Callback        , MUIA_Disabled, TRUE);
      set(data->SL_ConnectTimeout   , MUIA_Disabled, TRUE);
      set(data->BT_RemoveEvent      , MUIA_Disabled, TRUE);
      set(data->PA_Event            , MUIA_Disabled, TRUE);
      set(data->CY_ExecType         , MUIA_Disabled, TRUE);
      set(data->BT_RemoveNameServer , MUIA_Disabled, TRUE);
      set(data->STR_NameServer      , MUIA_Disabled, TRUE);
      set(data->BT_RemoveDomainName , MUIA_Disabled, TRUE);
      set(data->STR_DomainName      , MUIA_Disabled, TRUE);
      set(data->STR_TimeServer      , MUIA_Disabled, TRUE);
      set(data->CH_SaveTime         , MUIA_Disabled, TRUE);

      set(data->PO_Name                , MUIA_ShortHelp, GetStr(MSG_Help_IfaceName));
      set(data->STR_Comment            , MUIA_ShortHelp, GetStr(MSG_Help_Comment));
      set(data->STR_Address            , MUIA_ShortHelp, GetStr(MSG_Help_Address));
      set(data->CY_Address             , MUIA_ShortHelp, GetStr(MSG_Help_StaticDynamic));
      set(data->STR_Dest               , MUIA_ShortHelp, GetStr(MSG_Help_Dest));
      set(data->CY_Dest                , MUIA_ShortHelp, GetStr(MSG_Help_StaticDynamic));
      set(data->STR_Gateway            , MUIA_ShortHelp, GetStr(MSG_Help_Gateway));
      set(data->CY_Gateway             , MUIA_ShortHelp, GetStr(MSG_Help_StaticDynamic));
      set(data->STR_Netmask            , MUIA_ShortHelp, GetStr(MSG_Help_Netmask));
      set(data->CY_Netmask             , MUIA_ShortHelp, GetStr(MSG_Help_DefaultNetmask));
      set(data->STR_MTU                , MUIA_ShortHelp, GetStr(MSG_Help_MTU));
      set(data->STR_ConfigParams       , MUIA_ShortHelp, GetStr(MSG_Help_ConfigParams));

      set(data->CY_Sana2Device         , MUIA_ShortHelp, GetStr(MSG_Help_Sana2DeviceCycle));
      set(data->PA_Sana2Device         , MUIA_ShortHelp, GetStr(MSG_Help_Sana2Device));
      set(data->STR_Sana2Unit          , MUIA_ShortHelp, GetStr(MSG_Help_Sana2Unit));
      set(data->PA_Sana2ConfigFile     , MUIA_ShortHelp, GetStr(MSG_Help_Sana2ConfigFile));
      set(data->TI_Sana2ConfigText     , MUIA_ShortHelp, GetStr(MSG_Help_Sana2ConfigText));

      set(data->LV_Modems              , MUIA_ShortHelp, GetStr(MSG_Help_IfaceChooseModem));
      set(data->BT_NewModem            , MUIA_ShortHelp, GetStr(MSG_Help_New));
      set(data->BT_DeleteModem         , MUIA_ShortHelp, GetStr(MSG_Help_Delete));
      set(data->BT_EditModem           , MUIA_ShortHelp, GetStr(MSG_Help_Edit));
      set(data->PO_Phone               , MUIA_ShortHelp, GetStr(MSG_Help_Phone));
      set(data->STR_Login              , MUIA_ShortHelp, GetStr(MSG_Help_LoginName));
      set(data->STR_Password           , MUIA_ShortHelp, GetStr(MSG_Help_Password));
      set(data->LV_LoginScript         , MUIA_ShortHelp, GetStr(MSG_Help_LoginScript));
      set(data->BT_AddScriptLine       , MUIA_ShortHelp, GetStr(MSG_Help_New));
      set(data->BT_RemoveScriptLine    , MUIA_ShortHelp, GetStr(MSG_Help_Remove));
      set(data->CY_ScriptCommand       , MUIA_ShortHelp, GetStr(MSG_Help_ScriptCommand));
      set(data->STR_ScriptContents     , MUIA_ShortHelp, GetStr(MSG_Help_ScriptContents));

      set(data->CH_GetTime             , MUIA_ShortHelp, GetStr(MSG_Help_SyncClock));
      set(data->CH_SaveTime            , MUIA_ShortHelp, GetStr(MSG_Help_SaveTime));
      set(data->STR_TimeServer         , MUIA_ShortHelp, GetStr(MSG_Help_TimeServer));
      set(data->CH_UseBOOTP            , MUIA_ShortHelp, GetStr(MSG_Help_UseBOOTP));
      set(data->SL_KeepAlive           , MUIA_ShortHelp, GetStr(MSG_Help_KeepAlive));
      set(data->CH_AutoOnline          , MUIA_ShortHelp, GetStr(MSG_Help_AutoOnline));

      set(data->CY_EventPage           , MUIA_ShortHelp, GetStr(MSG_Help_EventPage));
      set(data->LV_Online              , MUIA_ShortHelp, GetStr(MSG_Help_Online));
      set(data->LV_OnlineFail          , MUIA_ShortHelp, GetStr(MSG_Help_OnlineFail));
      set(data->LV_OfflineActive       , MUIA_ShortHelp, GetStr(MSG_Help_OfflineActive));
      set(data->LV_OfflinePassive      , MUIA_ShortHelp, GetStr(MSG_Help_OfflinePassive));
      set(data->BT_AddEvent            , MUIA_ShortHelp, GetStr(MSG_Help_New));
      set(data->BT_RemoveEvent         , MUIA_ShortHelp, GetStr(MSG_Help_Remove));
      set(data->PA_Event               , MUIA_ShortHelp, GetStr(MSG_Help_Event));
      set(data->CY_ExecType            , MUIA_ShortHelp, GetStr(MSG_Help_ExecType));

      set(data->CH_CarrierDetect       , MUIA_ShortHelp, GetStr(MSG_Help_CarrierDetect));
      set(data->SL_ConnectTimeout      , MUIA_ShortHelp, GetStr(MSG_Help_ConnectTimeout));
      set(data->STR_Callback           , MUIA_ShortHelp, GetStr(MSG_Help_Callback));
      set(data->CH_Callback            , MUIA_ShortHelp, GetStr(MSG_Help_Callback));
      set(data->CH_MPPCompression      , MUIA_ShortHelp, GetStr(MSG_Help_MPPCompression));
      set(data->CH_VJCompression       , MUIA_ShortHelp, GetStr(MSG_Help_VJCompression));
      set(data->CH_BSDCompression      , MUIA_ShortHelp, GetStr(MSG_Help_BSDCompression));
      set(data->CH_DeflateCompression  , MUIA_ShortHelp, GetStr(MSG_Help_DeflateCompression));
      set(data->CY_EOF                 , MUIA_ShortHelp, GetStr(MSG_Help_EOF));
      set(data->CH_UseDNS              , MUIA_ShortHelp, GetStr(MSG_Help_UseNameserver));

      set(data->LV_NameServers         , MUIA_ShortHelp, GetStr(MSG_Help_NameServers));
      set(data->LV_DomainNames         , MUIA_ShortHelp, GetStr(MSG_Help_DomainNames));
      set(data->BT_AddNameServer       , MUIA_ShortHelp, GetStr(MSG_Help_New));
      set(data->BT_RemoveNameServer    , MUIA_ShortHelp, GetStr(MSG_Help_Remove));
      set(data->BT_AddDomainName       , MUIA_ShortHelp, GetStr(MSG_Help_New));
      set(data->BT_RemoveDomainName    , MUIA_ShortHelp, GetStr(MSG_Help_Remove));
      set(data->CH_UseHostName         , MUIA_ShortHelp, GetStr(MSG_Help_QueryHostName));
      set(data->CH_UseDomainName       , MUIA_ShortHelp, GetStr(MSG_Help_QueryDomainName));
      set(data->STR_HostName           , MUIA_ShortHelp, GetStr(MSG_Help_HostName));

      set(data->BT_Okay                , MUIA_ShortHelp, GetStr(MSG_Help_Okay));
      set(data->BT_Cancel              , MUIA_ShortHelp, GetStr(MSG_Help_Cancel));

      DoMethod(obj                 , MUIM_Notify, MUIA_Window_CloseRequest, TRUE            , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_Interfaces_EditFinish, obj, 0);
      DoMethod(data->BT_Cancel     , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_Interfaces_EditFinish, obj, 0);
      DoMethod(data->BT_Okay       , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_Interfaces_EditFinish, obj, 1);

      DoMethod(data->CH_Callback   , MUIM_Notify, MUIA_Selected           , MUIV_EveryTime  , obj, 6, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, data->STR_Callback, data->SL_ConnectTimeout, NULL);

      DoMethod(data->LV_IfaceNames , MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime , obj, 2, MUIM_IfaceWindow_PopString_Close, MUIV_IfaceWindow_PopString_IfaceName);
      DoMethod(data->CY_Address    , MUIM_Notify, MUIA_Cycle_Active        , MUIV_EveryTime , data->STR_Address , 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->CY_Dest       , MUIM_Notify, MUIA_Cycle_Active        , MUIV_EveryTime , data->STR_Dest    , 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->CY_Gateway    , MUIM_Notify, MUIA_Cycle_Active        , MUIV_EveryTime , data->STR_Gateway , 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->CY_Netmask    , MUIM_Notify, MUIA_Cycle_Active        , MUIV_EveryTime , data->STR_Netmask , 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->CY_Sana2Device, MUIM_Notify, MUIA_Cycle_Active        , MUIV_EveryTime , obj, 1, MUIM_IfaceWindow_SanaActive);

      DoMethod(data->LI_Modems   , MUIM_Notify, MUIA_NList_Active          , MUIV_EveryTime , obj, 1, MUIM_IfaceWindow_ModemActive);
      DoMethod(data->LI_Modems   , MUIM_Notify, MUIA_NList_DoubleClick     , MUIV_EveryTime, obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_EditModem);
      DoMethod(data->BT_NewModem , MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_NewModem);
      DoMethod(data->BT_DeleteModem, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_DeleteModem);
      DoMethod(data->BT_EditModem, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_EditModem);

      DoMethod(data->LV_LoginScript     , MUIM_Notify, MUIA_List_Active     , MUIV_EveryTime , obj, 1, MUIM_IfaceWindow_LoginScriptActive);
      DoMethod(data->BT_AddScriptLine   , MUIM_Notify, MUIA_Pressed         , FALSE          , obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_AddScriptLine);
      DoMethod(data->BT_RemoveScriptLine, MUIM_Notify, MUIA_Pressed         , FALSE          , data->LI_LoginScript, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(data->CY_ScriptCommand   , MUIM_Notify, MUIA_Cycle_Active    , MUIV_EveryTime , obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_ScriptLine);
      DoMethod(data->STR_ScriptContents , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime , obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_ScriptLine);
      DoMethod(data->STR_Phone          , MUIM_Notify, MUIA_String_Acknowledge,MUIV_EveryTime, obj, 2, MUIM_IfaceWindow_PopString_AddPhone, TRUE);
      DoMethod(data->BT_AddPhone        , MUIM_Notify, MUIA_Pressed         , FALSE          , obj, 2, MUIM_IfaceWindow_PopString_AddPhone, TRUE);
      DoMethod(data->BT_CancelPhone     , MUIM_Notify, MUIA_Pressed         , FALSE          , obj, 2, MUIM_IfaceWindow_PopString_AddPhone, FALSE);

      DoMethod(data->CY_EventPage      , MUIM_Notify, MUIA_Cycle_Active    , MUIV_EveryTime , obj, 1, MUIM_IfaceWindow_EventActive);
      DoMethod(data->LV_Online         , MUIM_Notify, MUIA_NList_Active    , MUIV_EveryTime , obj, 1, MUIM_IfaceWindow_SetEventStates);
      DoMethod(data->LV_OnlineFail     , MUIM_Notify, MUIA_NList_Active    , MUIV_EveryTime , obj, 1, MUIM_IfaceWindow_SetEventStates);
      DoMethod(data->LV_OfflineActive  , MUIM_Notify, MUIA_NList_Active    , MUIV_EveryTime , obj, 1, MUIM_IfaceWindow_SetEventStates);
      DoMethod(data->LV_OfflinePassive , MUIM_Notify, MUIA_NList_Active    , MUIV_EveryTime , obj, 1, MUIM_IfaceWindow_SetEventStates);
      DoMethod(data->BT_AddEvent       , MUIM_Notify, MUIA_Pressed         , FALSE          , obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_AddEvent);
      DoMethod(data->BT_RemoveEvent    , MUIM_Notify, MUIA_Pressed         , FALSE          , obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_RemoveEvent);
      DoMethod(data->STR_Event         , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime , obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_Event);
      DoMethod(data->CY_ExecType       , MUIM_Notify, MUIA_Cycle_Active    , MUIV_EveryTime , obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_ExecType);

      DoMethod(data->CH_GetTime        , MUIM_Notify, MUIA_Selected        , MUIV_EveryTime  , data->STR_TimeServer, 6, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, data->CH_SaveTime, data->STR_TimeServer, NULL);

      DoMethod(data->LI_NameServers       , MUIM_Notify, MUIA_List_Active     , MUIV_EveryTime  , obj, 1, MUIM_IfaceWindow_NameserversActive);
      DoMethod(data->BT_AddNameServer     , MUIM_Notify, MUIA_Pressed         , FALSE           , obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_AddNameServer);
      DoMethod(data->BT_RemoveNameServer  , MUIM_Notify, MUIA_Pressed         , FALSE           , data->LI_NameServers, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(data->STR_NameServer       , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime  , obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_NameServer);
      DoMethod(data->LI_DomainNames       , MUIM_Notify, MUIA_List_Active     , MUIV_EveryTime  , obj, 1, MUIM_IfaceWindow_DomainnamesActive);
      DoMethod(data->BT_AddDomainName     , MUIM_Notify, MUIA_Pressed         , FALSE           , obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_AddDomainName);
      DoMethod(data->BT_RemoveDomainName  , MUIM_Notify, MUIA_Pressed         , FALSE           , data->LI_DomainNames, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(data->STR_DomainName       , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime  , obj, 2, MUIM_IfaceWindow_Modification, MUIV_IfaceWindow_Modification_DomainName);
      DoMethod(data->CH_UseHostName       , MUIM_Notify, MUIA_Selected        , MUIV_EveryTime  , data->STR_HostName, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);
   }
   return((ULONG)obj);
}

///
/// IfaceWindow_Dispatcher
SAVEDS ASM ULONG IfaceWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                               : return(IfaceWindow_New               (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_Init                : return(IfaceWindow_Init              (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_CopyData            : return(IfaceWindow_CopyData          (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_PopString_Close     : return(IfaceWindow_PopString_Close   (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_SanaActive          : return(IfaceWindow_SanaActive        (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_SetEventStates      : return(IfaceWindow_SetEventStates    (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_Modification        : return(IfaceWindow_Modification      (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_EventActive         : return(IfaceWindow_EventActive       (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_ModemActive         : return(IfaceWindow_ModemActive       (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_LoginScriptActive   : return(IfaceWindow_LoginScriptActive (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_PopString_AddPhone  : return(IfaceWindow_PopString_AddPhone(cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_NameserversActive   : return(IfaceWindow_NameserversActive (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_DomainnamesActive   : return(IfaceWindow_DomainnamesActive (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

