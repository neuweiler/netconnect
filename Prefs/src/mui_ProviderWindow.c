/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "Strings.h"
#include "mui.h"
#include "mui_ProviderWindow.h"
#include "mui_IfaceWindow.h"
#include "mui_Provider.h"
#include "protos.h"

///
/// external variables
extern struct Hook strobjhook, des_hook;
extern Object *win, *app;
extern struct MUI_CustomClass  *CL_IfaceWindow;
extern struct Library *GenesisBase;

///

/// ProviderWindow_PopString_AddPhone
ULONG ProviderWindow_PopString_AddPhone(struct IClass *cl, Object *obj, struct MUIP_ProviderWindow_PopString_AddPhone *msg)
{
   struct ProviderWindow_Data *data = INST_DATA(cl, obj);

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
/// ProviderWindow_Init
ULONG ProviderWindow_Init(struct IClass *cl, Object *obj, struct MUIP_ProviderWindow_Init *msg)
{
   struct ProviderWindow_Data *data = INST_DATA(cl, obj);
   struct Interface *iface = NULL;
   struct ScriptLine *script_line = NULL;

   data->isp = msg->isp;

   set(data->STR_Name      , MUIA_String_Contents, data->isp->isp_name);
   set(data->STR_Comment   , MUIA_String_Contents, data->isp->isp_comment);
   set(data->STR_Login     , MUIA_String_Contents, data->isp->isp_login);
   set(data->STR_Password  , MUIA_String_Contents, data->isp->isp_password);
// isp_organisation;

   if(data->isp->isp_nameservers.mlh_TailPred != (struct MinNode *)&data->isp->isp_nameservers ||
      data->isp->isp_domainnames.mlh_TailPred != (struct MinNode *)&data->isp->isp_domainnames)
   {
      setcycle(data->CY_Resolv, 1);

      if(data->isp->isp_nameservers.mlh_TailPred != (struct MinNode *)&data->isp->isp_nameservers)
      {
         struct ServerEntry *server;

         server = (struct ServerEntry *)data->isp->isp_nameservers.mlh_Head;
         while(server->se_node.mln_Succ)
         {
            DoMethod(data->LI_NameServers, MUIM_List_InsertSingle, server, MUIV_List_Insert_Bottom);
            server = (struct ServerEntry *)server->se_node.mln_Succ;
         }
      }
      if(data->isp->isp_domainnames.mlh_TailPred != (struct MinNode *)&data->isp->isp_domainnames)
      {
         struct ServerEntry *server;

         server = (struct ServerEntry *)data->isp->isp_domainnames.mlh_Head;
         while(server->se_node.mln_Succ)
         {
            DoMethod(data->LI_DomainNames, MUIM_List_InsertSingle, server, MUIV_List_Insert_Bottom);
            server = (struct ServerEntry *)server->se_node.mln_Succ;
         }
      }
   }
   else
   {
      setcycle(data->CY_Resolv, 0);
      DoMethod(data->LI_NameServers, MUIM_List_Clear);
      DoMethod(data->LI_DomainNames, MUIM_List_Clear);
      setstring(data->STR_NameServer, NULL);
      setstring(data->STR_DomainName, NULL);
   }
   setstring(data->STR_HostName, (*data->isp->isp_hostname ? data->isp->isp_hostname : NULL));
   setcycle(data->CY_HostName, (*data->isp->isp_hostname ? 1 : 0));
   setcheckmark(data->CH_DontQueryHostname, data->isp->isp_flags & ISF_DontQueryHostname);

   if(data->isp->isp_ifaces.mlh_TailPred != (struct MinNode *)&data->isp->isp_ifaces)
   {
      iface = (struct Interface *)data->isp->isp_ifaces.mlh_Head;
      while(iface->if_node.mln_Succ)
      {
         DoMethod(data->LI_Interfaces, MUIM_List_InsertSingle, iface, MUIV_List_Insert_Bottom);
         iface = (struct Interface *)iface->if_node.mln_Succ;
      }
   }

   setcheckmark(data->CH_GetTime, data->isp->isp_flags & ISF_GetTime);
   setcheckmark(data->CH_SaveTime, data->isp->isp_flags & ISF_SaveTime);
   setstring(data->STR_TimeServer, data->isp->isp_timename);
   setstring(data->STR_BootpServer, data->isp->isp_bootp);
   setcheckmark(data->CH_BOOTP, data->isp->isp_flags & ISF_UseBootp);
   set(data->STR_BootpServer, MUIA_Disabled, !(data->isp->isp_flags & ISF_UseBootp));

   if(data->isp->isp_loginscript.mlh_TailPred != (struct MinNode *)&data->isp->isp_loginscript)
   {
      script_line = (struct ScriptLine *)data->isp->isp_loginscript.mlh_Head;
      while(script_line->sl_node.mln_Succ)
      {
         DoMethod(data->LI_Script, MUIM_List_InsertSingle, script_line, MUIV_List_Insert_Bottom);
         script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
      }
   }

   setstring(data->STR_Phone, data->isp->isp_phonenumber);

   return(NULL);
}
///
/// ProviderWindow_CopyData
ULONG ProviderWindow_CopyData(struct IClass *cl, Object *obj, Msg msg)
{
   struct ProviderWindow_Data *data = INST_DATA(cl, obj);
   struct Interface *iface1 = NULL, *iface2 = NULL;
   struct ScriptLine *script_line1 = NULL, *script_line2 = NULL;
   LONG pos;
   struct ServerEntry *server1, *server2;

   if(data->isp)
   {
      strncpy(data->isp->isp_name      , (STRPTR)xget(data->STR_Name    , MUIA_String_Contents), sizeof(data->isp->isp_name));
      strncpy(data->isp->isp_comment   , (STRPTR)xget(data->STR_Comment , MUIA_String_Contents), sizeof(data->isp->isp_comment));
      strncpy(data->isp->isp_login     , (STRPTR)xget(data->STR_Login   , MUIA_String_Contents), sizeof(data->isp->isp_login));
      strncpy(data->isp->isp_password  , (STRPTR)xget(data->STR_Password, MUIA_String_Contents), sizeof(data->isp->isp_password));
//  isp_organisation;

      if(data->isp->isp_nameservers.mlh_TailPred != (struct MinNode *)&data->isp->isp_nameservers)
      {
         server1 = (struct ServerEntry *)data->isp->isp_nameservers.mlh_Head;
         while(server2 = (struct ServerEntry *)server1->se_node.mln_Succ)
         {
            Remove((struct Node *)server1);
            FreeVec(server1);
            server1 = server2;
         }
      }
      if(data->isp->isp_domainnames.mlh_TailPred != (struct MinNode *)&data->isp->isp_domainnames)
      {
         server1 = (struct ServerEntry *)data->isp->isp_domainnames.mlh_Head;
         while(server2 = (struct ServerEntry *)server1->se_node.mln_Succ)
         {
            Remove((struct Node *)server1);
            FreeVec(server1);
            server1 = server2;
         }
      }
      if(xget(data->CY_Resolv, MUIA_Cycle_Active))
      {
         pos = 0;
         FOREVER
         {
            DoMethod(data->LI_NameServers, MUIM_List_GetEntry, pos++, &server1);
            if(!server1)
               break;
            if(server2 = AllocVec(sizeof(struct ServerEntry), MEMF_ANY | MEMF_CLEAR))
            {
               strncpy(server2->se_name, server1->se_name, 40);
               AddTail((struct List *)&data->isp->isp_nameservers, (struct Node *)server2);
            }
         }
         pos = 0;
         FOREVER
         {
            DoMethod(data->LI_DomainNames, MUIM_List_GetEntry, pos++, &server1);
            if(!server1)
               break;
            if(server2 = AllocVec(sizeof(struct ServerEntry), MEMF_ANY | MEMF_CLEAR))
            {
               strncpy(server2->se_name, server1->se_name, 40);
               AddTail((struct List *)&data->isp->isp_domainnames, (struct Node *)server2);
            }
         }
      }

      if(xget(data->CY_HostName, MUIA_Cycle_Active))
         strncpy(data->isp->isp_hostname, (STRPTR)xget(data->STR_HostName  , MUIA_String_Contents), sizeof(data->isp->isp_hostname));
      else
         *data->isp->isp_hostname = NULL;
      if(xget(data->CH_DontQueryHostname, MUIA_Selected))
         data->isp->isp_flags |= ISF_DontQueryHostname;
      else
         data->isp->isp_flags &= ~ISF_DontQueryHostname;

      if(data->isp->isp_ifaces.mlh_TailPred != (struct MinNode *)&data->isp->isp_ifaces)
      {
         iface1 = (struct Interface *)data->isp->isp_ifaces.mlh_Head;
         while(iface2 = (struct Interface *)iface1->if_node.mln_Succ)
         {
            Remove((struct Node *)iface1);
            if(iface1->if_sana2configtext)
               FreeVec(iface1->if_sana2configtext);
            if(iface1->if_configparams)
               FreeVec(iface1->if_configparams);
            if(iface1->if_userdata)
               FreeVec(iface1->if_userdata);
// remove events
            FreeVec(iface1);
            iface1 = iface2;
         }
      }
      pos = 0;
      FOREVER
      {
         DoMethod(data->LI_Interfaces, MUIM_List_GetEntry, pos++, &iface1);
         if(!iface1)
            break;
         if(iface2 = AllocVec(sizeof(struct Interface), MEMF_ANY))
         {
            memcpy(iface2, iface1, sizeof(struct Interface));
            iface2->if_sana2configtext = NULL;
            if(iface1->if_sana2configtext)
               ReallocCopy((STRPTR *)&iface2->if_sana2configtext, iface1->if_sana2configtext);
            iface2->if_configparams = NULL;
            if(iface1->if_configparams)
               ReallocCopy((STRPTR *)&iface2->if_configparams, iface1->if_configparams);
            if(iface2->if_userdata = (APTR)AllocVec(sizeof(struct PrefsPPPIface), MEMF_ANY))
               memcpy(iface2->if_userdata, iface1->if_userdata, sizeof(struct PrefsPPPIface));
            NewList((struct List *)&iface2->if_events);
// copy events

            AddTail((struct List *)&data->isp->isp_ifaces, (struct Node *)iface2);
         }
      }

      if(xget(data->CH_GetTime, MUIA_Selected))
         data->isp->isp_flags |= ISF_GetTime;
      else
         data->isp->isp_flags &= ~ISF_GetTime;
      if(xget(data->CH_SaveTime, MUIA_Selected))
         data->isp->isp_flags |= ISF_SaveTime;
      else
         data->isp->isp_flags &= ~ISF_SaveTime;
      strncpy(data->isp->isp_timename , (STRPTR)xget(data->STR_TimeServer, MUIA_String_Contents) , sizeof(data->isp->isp_timename));
      strncpy(data->isp->isp_bootp, (STRPTR)xget(data->STR_BootpServer, MUIA_String_Contents), sizeof(data->isp->isp_bootp));
      if(xget(data->CH_BOOTP, MUIA_Selected))
         data->isp->isp_flags |= ISF_UseBootp;
      else
         data->isp->isp_flags &= ~ISF_UseBootp;

      if(data->isp->isp_loginscript.mlh_TailPred != (struct MinNode *)&data->isp->isp_loginscript)
      {
         script_line1 = (struct ScriptLine *)data->isp->isp_loginscript.mlh_Head;
         while(script_line2 = (struct ScriptLine *)script_line1->sl_node.mln_Succ)
         {
            Remove((struct Node *)script_line1);
            FreeVec(script_line1);
            script_line1 = script_line2;
         }
      }
      pos = 0;
      FOREVER
      {
         DoMethod(data->LI_Script, MUIM_List_GetEntry, pos++, &script_line1);
         if(!script_line1)
            break;
         if(script_line2 = AllocVec(sizeof(struct ScriptLine), MEMF_ANY))
         {
            memcpy(script_line2, script_line1, sizeof(struct ScriptLine));
            AddTail((struct List *)&data->isp->isp_loginscript, (struct Node *)script_line2);
         }
      }
      strncpy(data->isp->isp_phonenumber, (STRPTR)xget(data->STR_Phone, MUIA_String_Contents), sizeof(data->isp->isp_phonenumber));
   }

   return(NULL);
}
///

/// ProviderWindow_EditIface
ULONG ProviderWindow_EditIface(struct IClass *cl, Object *obj, Msg msg)
{
   struct ProviderWindow_Data *data = INST_DATA(cl, obj);
   struct Interface *iface;

   DoMethod(data->LI_Interfaces, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &iface);
   if(iface)
   {
      Object *window;

      set(app, MUIA_Application_Sleep, TRUE);
      if(window = NewObject(CL_IfaceWindow->mcc_Class, NULL, MUIA_Genesis_Originator, obj, TAG_DONE))
      {
         DoMethod(app, OM_ADDMEMBER, window);
         DoMethod(window, MUIM_IfaceWindow_Init, iface);
         set(window, MUIA_Window_Title, iface->if_name);
         set(window, MUIA_Window_Open, TRUE);
      }
   }
   else
      DisplayBeep(NULL);

   return(NULL);
}

///
/// ProviderWindow_EditIfaceFinish
ULONG ProviderWindow_EditIfaceFinish(struct IClass *cl, Object *obj, struct MUIP_ProviderWindow_EditIfaceFinish *msg)
{
//   struct Provider_Data *data = INST_DATA(cl, obj);

   set(msg->win, MUIA_Window_Open, FALSE);
   if(msg->ok)
      DoMethod(msg->win, MUIM_IfaceWindow_CopyData);
   DoMethod(app, OM_REMMEMBER, msg->win);
   MUI_DisposeObject(msg->win);
   set(app, MUIA_Application_Sleep, FALSE);

   return(NULL);
}

///

/// ProviderWindow_NameserversActive
ULONG ProviderWindow_NameserversActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct ProviderWindow_Data *data = INST_DATA(cl, obj);
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
/// ProviderWindow_DomainnamesActive
ULONG ProviderWindow_DomainnamesActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct ProviderWindow_Data *data = INST_DATA(cl, obj);
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
/// ProviderWindow_ScriptActive
ULONG ProviderWindow_ScriptActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct ProviderWindow_Data *data = INST_DATA(cl, obj);
   struct ScriptLine *script_line;

   DoMethod(data->LI_Script, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &script_line);
   if(script_line)
   {
      nnset(data->CY_Command, MUIA_Cycle_Active, script_line->sl_command);
      if(script_line->sl_command > SL_WaitFor && script_line->sl_command < SL_Exec)
      {
         nnset(data->STR_String, MUIA_String_Contents, NULL);
         set(data->STR_String, MUIA_Disabled, TRUE);
      }
      else
      {
         nnset(data->STR_String, MUIA_String_Contents, script_line->sl_contents);
         set(data->STR_String, MUIA_Disabled, FALSE);
         set(obj, MUIA_Window_ActiveObject, data->STR_String);
      }
      set(data->CY_Command, MUIA_Disabled, FALSE);
      set(data->BT_Remove, MUIA_Disabled, FALSE);
   }
   else
   {
      nnset(data->STR_String, MUIA_String_Contents, NULL);
      set(data->STR_String, MUIA_Disabled, TRUE);
      set(data->CY_Command, MUIA_Disabled, TRUE);
      set(data->BT_Remove, MUIA_Disabled, TRUE);
   }
   return(NULL);
}

///
/// ProviderWindow_IfacesActive
ULONG ProviderWindow_IfacesActive(struct IClass *cl, Object *obj, Msg msg)
{
   struct ProviderWindow_Data *data = INST_DATA(cl, obj);
   struct Interface *iface;

   DoMethod(data->LI_Interfaces, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &iface);
   set(data->BT_EditIface  , MUIA_Disabled, !iface);
   set(data->BT_DeleteIface, MUIA_Disabled, !iface);

   return(NULL);
}

///
/// ProviderWindow_Modification
ULONG ProviderWindow_Modification(struct IClass *cl, Object *obj, struct MUIP_ProviderWindow_Modification *msg)
{
   struct ProviderWindow_Data *data = INST_DATA(cl, obj);

   switch(msg->what)
   {
      case MUIV_ProviderWindow_Modification_AddNameServer:
         DoMethod(data->LI_NameServers, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
         set(data->LI_NameServers, MUIA_List_Active, MUIV_List_Active_Bottom);
         break;

      case MUIV_ProviderWindow_Modification_NameServer:
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

      case MUIV_ProviderWindow_Modification_AddDomainName:
         DoMethod(data->LI_DomainNames, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
         set(data->LI_DomainNames, MUIA_List_Active, MUIV_List_Active_Bottom);
         break;

      case MUIV_ProviderWindow_Modification_DomainName:
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


      case MUIV_ProviderWindow_Modification_AddInterface:
         DoMethod(data->LI_Interfaces, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
         set(data->LI_Interfaces, MUIA_List_Active, MUIV_List_Active_Bottom);
         DoMethod(obj, MUIM_ProviderWindow_EditIface);
         break;

      case MUIV_ProviderWindow_Modification_AddLine:
         DoMethod(data->LI_Script, MUIM_List_InsertSingle, -1, MUIV_List_Insert_Bottom);
         set(data->LI_Script, MUIA_List_Active, MUIV_List_Active_Bottom);
         break;

      case MUIV_ProviderWindow_Modification_ScriptLine:
      {
         struct ScriptLine *script_line;

         DoMethod(data->LI_Script, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &script_line);
         if(script_line)
         {
            script_line->sl_command = xget(data->CY_Command, MUIA_Cycle_Active);
            strncpy(script_line->sl_contents, (STRPTR)xget(data->STR_String, MUIA_String_Contents), MAXPATHLEN);
            DoMethod(data->LI_Script, MUIM_List_Redraw, MUIV_List_Redraw_Active);
            if(script_line->sl_command > SL_WaitFor && script_line->sl_command < SL_Exec)
               set(data->STR_String, MUIA_Disabled, TRUE);
            else
               set(data->STR_String, MUIA_Disabled, FALSE);
         }
      }
      break;
   }
   return(NULL);
}

///

/// ScriptList_ConstructFunc
SAVEDS ASM struct ScriptLine *ScriptList_ConstructFunc(register __a2 APTR pool, register __a1 struct ScriptLine *src)
{
   struct ScriptLine *new;

   if((new = (struct ScriptLine *)AllocVec(sizeof(struct ScriptLine), MEMF_ANY | MEMF_CLEAR)) && src && (src != (APTR)-1))
      memcpy(new, src, sizeof(struct ScriptLine));
   else if(new)
      new->sl_command = SL_Send;

   return(new);
}
static const struct Hook ScriptList_ConstructHook= { { 0,0 }, (VOID *)ScriptList_ConstructFunc , NULL, NULL };

///
/// ScriptList_DisplayFunc
SAVEDS ASM LONG ScriptList_DisplayFunc(register __a2 char **array, register __a1 struct ScriptLine *script_line)
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
static const struct Hook ScriptList_DisplayHook= { { 0,0 }, (VOID *)ScriptList_DisplayFunc , NULL, NULL };

///
/// ServerList_ConstructFunc
SAVEDS ASM struct ServerEntry *ServerList_ConstructFunc(register __a2 APTR pool, register __a1 struct ServerEntry *src)
{
   struct ServerEntry *new;

   if((new = (struct ServerEntry *)AllocVec(sizeof(struct ServerEntry), MEMF_ANY | MEMF_CLEAR)) && src && (src != (APTR)-1))
      memcpy(new, src, sizeof(struct ServerEntry));

   return(new);
}
static const struct Hook ServerList_ConstructHook= { { 0,0 }, (VOID *)ServerList_ConstructFunc , NULL, NULL };

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
static const struct Hook ServerList_DisplayHook= { { 0,0 }, (VOID *)ServerList_DisplayFunc , NULL, NULL };

///
/// IfaceList_ConstructFunc
SAVEDS ASM struct Interface *IfaceList_ConstructFunc(register __a2 APTR pool, register __a1 struct Interface *src)
{
   struct Interface *new;

   if(new = (struct Interface *)AllocVec(sizeof(struct Interface), MEMF_ANY | MEMF_CLEAR))
   {
      if(src && (src != (APTR)-1))
      {
         memcpy(new, src, sizeof(struct Interface));
         if(src->if_sana2configtext)
         {
            new->if_sana2configtext = NULL;
            ReallocCopy((STRPTR *)&new->if_sana2configtext, src->if_sana2configtext);
         }
         if(src->if_configparams)
         {
            new->if_configparams = NULL;
            ReallocCopy((STRPTR *)&new->if_configparams, src->if_configparams);
         }
         NewList((struct List *)&new->if_events);
         if(src->if_events.mlh_TailPred != (struct MinNode *)&src->if_events)
         {
            struct ScriptLine *event1, *event2;

            event1 = (struct ScriptLine *)src->if_events.mlh_Head;
            while(event1->sl_node.mln_Succ)
            {
               if(event2 = AllocVec(sizeof(struct ScriptLine), MEMF_ANY))
               {
                  memcpy(event2, event1, sizeof(struct ScriptLine));
                  AddTail((struct List *)&new->if_events, (struct Node *)event2);
               }
               event1 = (struct ScriptLine *)event1->sl_node.mln_Succ;
            }
         }

      }
      else
      {
         strcpy(new->if_name, "ppp");
         strcpy(new->if_sana2device, "DEVS:Networks/");
         strcpy(new->if_sana2config, "ENV:Sana2/");
         new->if_flags = IFL_PPP;
         new->if_MTU = 1500;
         NewList((struct List *)&new->if_events);
      }


      if(new->if_userdata = AllocVec(sizeof(struct PrefsPPPIface), MEMF_ANY | MEMF_CLEAR))
      {
         struct PrefsPPPIface *ppp_if = (struct PrefsPPPIface *)new->if_userdata;

         if(src && (src != (APTR)-1) && src->if_userdata)
            memcpy(new->if_userdata, src->if_userdata, sizeof(struct PrefsPPPIface));
         else
            ppp_if->ppp_carrierdetect = TRUE;
      }
   }
   return(new);
}
static const struct Hook IfaceList_ConstructHook= { { 0,0 }, (VOID *)IfaceList_ConstructFunc , NULL, NULL };

///
/// IfaceList_DestructFunc
SAVEDS ASM VOID IfaceList_DestructFunc(register __a2 APTR pool, register __a1 struct Interface *iface)
{
   if(iface)
   {
      if(iface->if_sana2configtext)
         FreeVec(iface->if_sana2configtext);
      if(iface->if_configparams)
         FreeVec(iface->if_configparams);
      if(iface->if_userdata)
         FreeVec(iface->if_userdata);

      FreeVec(iface);
   }
}
static const struct Hook IfaceList_DestructHook= { { 0,0 }, (VOID *)IfaceList_DestructFunc , NULL, NULL };

///
/// IfaceList_DisplayFunc
SAVEDS ASM LONG IfaceList_DisplayFunc(register __a2 char **array, register __a1 struct Interface *iface)
{
   if(iface)
   {
      static char buf[5];

      sprintf(buf, "%ld", iface->if_sana2unit);
      *array++ = iface->if_name;
      *array++ = iface->if_sana2device;
      *array++ = buf;
      *array   = (*iface->if_addr ? (STRPTR)iface->if_addr : GetStr(MSG_TX_Dynamic));
   }
   else
   {
      *array++ = GetStr(MSG_TX_Name);
      *array++ = GetStr(MSG_TX_Device);
      *array++ = GetStr(MSG_TX_Unit);
      *array   = GetStr(MSG_TX_Address);
   }
   return(NULL);
}
static const struct Hook IfaceList_DisplayHook= { { 0,0 }, (VOID *)IfaceList_DisplayFunc , NULL, NULL };

///

/// ProviderWindow_New
ULONG ProviderWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   static STRPTR STR_CY_Dynamic[3];
   static STRPTR ARR_Pages[6];
   struct ProviderWindow_Data tmp;
   Object *originator;

   originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

   ARR_Pages[0] = GetStr(MSG_TX_ProviderPage1);
   ARR_Pages[1] = GetStr(MSG_TX_ProviderPage2);
   ARR_Pages[2] = GetStr(MSG_TX_ProviderPage3);
   ARR_Pages[3] = GetStr(MSG_TX_ProviderPage4);
   ARR_Pages[4] = GetStr(MSG_TX_ProviderPage5);
   ARR_Pages[5] = NULL;

   STR_CY_Dynamic[0] = GetStr(MSG_TX_Dynamic);
   STR_CY_Dynamic[1] = GetStr(MSG_TX_Static);
   STR_CY_Dynamic[2] = NULL;

   if(obj = tmp.GR_TCPIP = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , GetStr(MSG_TX_EditISP),
      MUIA_Window_ID       , MAKE_ID('I','S','P','E'),
      MUIA_Window_Height   , MUIV_Window_Height_MinMax(0),
      WindowContents       , VGroup,
         Child, RegisterGroup(ARR_Pages),
            MUIA_CycleChain, 1,
            Child, VGroup,
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_ISPInformation)),
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_Name, MSG_CC_Name),
                  Child, tmp.STR_Name    = MakeKeyString(NULL, 40, MSG_CC_Name),
                  Child, MakeKeyLabel2(MSG_LA_Comment, MSG_CC_Comment),
                  Child, tmp.STR_Comment = MakeKeyString(NULL, 40, MSG_CC_Comment),
               End,
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_Authentication)),
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_Login, MSG_CC_Login),
                  Child, tmp.STR_Login    = MakeKeyString(NULL, 40, MSG_CC_Login),
                  Child, MakeKeyLabel2(MSG_LA_Password, MSG_CC_Password),
                  Child, tmp.STR_Password = TextinputObject,
                     StringFrame,
                     MUIA_ControlChar     , *GetStr(MSG_CC_Password),
                     MUIA_CycleChain      , 1,
                     MUIA_String_Secret   , TRUE,
                     MUIA_String_MaxLen   , 40,
                  End,
               End,
               Child, HVSpace,
            End,
            Child, VGroup,
               Child, HVSpace,
               Child, ColGroup(2),
                  Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_DNS)),
                  Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_DomainNames)),
                  Child, VGroup,
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
                  Child, VGroup,
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
               Child, HGroup,
                  Child, MakeKeyLabel2(MSG_LA_Configuration, MSG_CC_Configuration),
                  Child, tmp.CY_Resolv = MakeKeyCycle(STR_CY_Dynamic, MSG_CC_Configuration),
                  Child, HVSpace,
               End,
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_LocalConfiguration)),
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_HostName, MSG_CC_HostName),
                  Child, HGroup,
                     Child, tmp.STR_HostName = MakeKeyString(NULL, 64, MSG_CC_HostName),
                     Child, tmp.CY_HostName  = Cycle(STR_CY_Dynamic),
                  End,
               End,
               Child, HGroup,
                  Child, MakeKeyLabel2(MSG_LA_DontQueryHostname, MSG_CC_DontQueryHostname),
                  Child, tmp.CH_DontQueryHostname = MakeKeyCheckMark(FALSE, MSG_CC_DontQueryHostname),
                  Child, HVSpace,
               End,
               Child, HVSpace,
            End,

            Child, tmp.GR_Interface = VGroup,
               Child, VGroup,
                  GroupSpacing(0),
                  Child, tmp.LV_Interfaces = NListviewObject,
                     MUIA_CycleChain      , 1,
                     MUIA_NListview_NList , tmp.LI_Interfaces = NListObject,
                        MUIA_Frame              , MUIV_Frame_InputList,
                        MUIA_NList_DragType     , MUIV_NList_DragType_Default,
                        MUIA_NList_ConstructHook, &IfaceList_ConstructHook,
                        MUIA_NList_DisplayHook  , &IfaceList_DisplayHook,
                        MUIA_NList_DestructHook , &IfaceList_DestructHook,
                        MUIA_NList_DragSortable , TRUE,
                        MUIA_NList_DoubleClick  , TRUE,
                        MUIA_NList_Format       , "BAR,BAR,BAR,",
                        MUIA_NList_Title        , TRUE,
                     End,
                  End,
                  Child, HGroup,
                     GroupSpacing(0),
                     Child, tmp.BT_AddIface    = MakeButton(MSG_BT_New),
                     Child, tmp.BT_DeleteIface = MakeButton(MSG_BT_Delete),
                     Child, tmp.BT_EditIface   = MakeButton(MSG_BT_Edit),
                  End,
               End,
            End,

            Child, tmp.GR_Server = VGroup,
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_Time)),
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_SyncClock, MSG_CC_SyncClock),
                  Child, HGroup,
                     Child, tmp.CH_GetTime = MakeKeyCheckMark(FALSE, MSG_CC_SyncClock),
                     Child, HVSpace,
                     Child, MakeKeyLabel2(MSG_LA_SaveTime, MSG_CC_SaveTime),
                     Child, tmp.CH_SaveTime = MakeKeyCheckMark(FALSE, MSG_CC_SaveTime),
                  End,
                  Child, MakeKeyLabel2(MSG_LA_TimeServer, MSG_CC_TimeServer),
                  Child, tmp.STR_TimeServer     = MakeKeyString(NULL, 64, MSG_CC_TimeServer),
               End,
               Child, HVSpace,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_Bootp)),
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_UseBootp, MSG_CC_UseBootp),
                  Child, HGroup,
                     Child, tmp.CH_BOOTP = MakeKeyCheckMark(FALSE, MSG_CC_UseBootp),
                     Child, HVSpace,
                  End,
                  Child, MakeKeyLabel2(MSG_LA_BootpServer, MSG_CC_BootpServer),
                  Child, tmp.STR_BootpServer    = TextinputObject,
                     StringFrame,
                     MUIA_ControlChar     , *GetStr(MSG_CC_BootpServer),
                     MUIA_CycleChain      , 1,
                     MUIA_String_Accept   , "0123456789.",
                     MUIA_String_MaxLen   , 16,
                  End,
               End,
               Child, HVSpace,
            End,

            Child, tmp.GR_Script = VGroup,
               Child, VGroup,
                  GroupSpacing(0),
                  Child, tmp.LV_Script = NListviewObject,
                     MUIA_CycleChain      , 1,
                     MUIA_NListview_NList , tmp.LI_Script = NListObject,
                        MUIA_Frame              , MUIV_Frame_InputList,
                        MUIA_NList_DragType     , MUIV_NList_DragType_Default,
                        MUIA_NList_ConstructHook, &ScriptList_ConstructHook,
                        MUIA_NList_DisplayHook  , &ScriptList_DisplayHook,
                        MUIA_NList_DestructHook , &des_hook,
                        MUIA_NList_DragSortable , TRUE,
                        MUIA_NList_Format       , "BAR,",
                        MUIA_NList_Title        , TRUE,
                     End,
                  End,
                  Child, HGroup,
                     GroupSpacing(0),
                     Child, tmp.BT_Add    = MakeButton(MSG_BT_Add),
                     Child, tmp.BT_Remove = MakeButton(MSG_BT_Remove),
                  End,
               End,
               Child, HGroup,
                  Child, tmp.CY_Command = Cycle(script_commands),
                  Child, tmp.STR_String = MakeKeyString(NULL, MAXPATHLEN, "  l"),
               End,
               Child, HGroup,
                  Child, MakeKeyLabel2(MSG_LA_PhoneNumbers, MSG_CC_PhoneNumbers),
                  Child, tmp.PO_Phone = PopobjectObject,
                     MUIA_Popstring_String      , tmp.STR_Phone = MakeKeyString(NULL, 100, MSG_CC_PhoneNumbers),
                     MUIA_Popstring_Button      , PopButton(MUII_PopUp),
                     MUIA_Popobject_Object      , VGroup,
                        MUIA_Frame, MUIV_Frame_Group,
                        Child, KeyLLabel(GetStr(MSG_LA_AddNumber), *GetStr(MSG_CC_AddNumber)),
                        Child, tmp.STR_AddPhone = MakeKeyString(NULL, 30, MSG_CC_AddNumber),
                        Child, HGroup,
                           Child, tmp.BT_AddPhone = MakeButton(MSG_BT_Add),
                           Child, tmp.BT_CancelPhone = MakeButton(MSG_BT_Cancel),
                        End,
                     End,
                  End,
               End,
            End,

         End,
         Child, HGroup,
            Child, tmp.BT_Okay   = MakeButton(MSG_BT_Okay),
            Child, tmp.BT_Cancel = MakeButton(MSG_BT_Cancel),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct ProviderWindow_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      data->isp = NULL;

      set(data->CY_HostName        , MUIA_Weight, 0);
      set(data->CY_Command         , MUIA_Weight, 0);

      set(data->LV_NameServers     , MUIA_Disabled, TRUE);
      set(data->BT_AddNameServer   , MUIA_Disabled, TRUE);
      set(data->BT_RemoveNameServer, MUIA_Disabled, TRUE);
      set(data->STR_NameServer     , MUIA_Disabled, TRUE);
      set(data->LV_DomainNames     , MUIA_Disabled, TRUE);
      set(data->BT_AddDomainName   , MUIA_Disabled, TRUE);
      set(data->BT_RemoveDomainName, MUIA_Disabled, TRUE);
      set(data->STR_DomainName     , MUIA_Disabled, TRUE);
      set(data->STR_HostName       , MUIA_Disabled, TRUE);
      set(data->BT_DeleteIface     , MUIA_Disabled, TRUE);
      set(data->BT_EditIface       , MUIA_Disabled, TRUE);
      set(data->CH_SaveTime        , MUIA_Disabled, TRUE);
      set(data->STR_TimeServer     , MUIA_Disabled, TRUE);
      set(data->CY_Command         , MUIA_Disabled, TRUE);
      set(data->STR_String         , MUIA_Disabled, TRUE);
      set(data->BT_Remove          , MUIA_Disabled, TRUE);

      set(data->STR_String         , MUIA_String_AttachedList, data->LV_Script);

      set(data->CH_BOOTP        , MUIA_CycleChain, 1);
      set(data->CY_Resolv       , MUIA_CycleChain, 1);
      set(data->CY_HostName     , MUIA_CycleChain, 1);
      set(data->CY_Command      , MUIA_CycleChain, 1);

      set(data->STR_Login       , MUIA_ShortHelp, GetStr(MSG_Help_LoginName));
//      set(data->STR_Comment     , MUIA_ShortHelp, GetStr(MSG_Help_Comment));
      set(data->STR_Password    , MUIA_ShortHelp, GetStr(MSG_Help_Password));
      set(data->STR_HostName    , MUIA_ShortHelp, GetStr(MSG_Help_HostName));
      set(data->STR_TimeServer  , MUIA_ShortHelp, GetStr(MSG_Help_TimeServer));
      set(data->STR_Phone       , MUIA_ShortHelp, GetStr(MSG_Help_Phone));

      DoMethod(obj                 , MUIM_Notify, MUIA_Window_CloseRequest, TRUE            , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_Provider_EditISPFinish, obj, 0);
      DoMethod(data->BT_Cancel     , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_Provider_EditISPFinish, obj, 0);
      DoMethod(data->BT_Okay       , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_Provider_EditISPFinish, obj, 1);

      DoMethod(data->LI_NameServers       , MUIM_Notify, MUIA_List_Active     , MUIV_EveryTime  , obj, 1, MUIM_ProviderWindow_NameserversActive);
      DoMethod(data->BT_AddNameServer     , MUIM_Notify, MUIA_Pressed         , FALSE           , obj, 2, MUIM_ProviderWindow_Modification, MUIV_ProviderWindow_Modification_AddNameServer);
      DoMethod(data->BT_RemoveNameServer  , MUIM_Notify, MUIA_Pressed         , FALSE           , data->LI_NameServers, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(data->STR_NameServer       , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime  , obj, 2, MUIM_ProviderWindow_Modification, MUIV_ProviderWindow_Modification_NameServer);
      DoMethod(data->LI_DomainNames       , MUIM_Notify, MUIA_List_Active     , MUIV_EveryTime  , obj, 1, MUIM_ProviderWindow_DomainnamesActive);
      DoMethod(data->BT_AddDomainName     , MUIM_Notify, MUIA_Pressed         , FALSE           , obj, 2, MUIM_ProviderWindow_Modification, MUIV_ProviderWindow_Modification_AddDomainName);
      DoMethod(data->BT_RemoveDomainName  , MUIM_Notify, MUIA_Pressed         , FALSE           , data->LI_DomainNames, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(data->STR_DomainName       , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime  , obj, 2, MUIM_ProviderWindow_Modification, MUIV_ProviderWindow_Modification_DomainName);
      DoMethod(data->CY_Resolv            , MUIM_Notify, MUIA_Cycle_Active    , MUIV_EveryTime  , data->STR_DomainName    , 12, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, data->LV_NameServers, data->BT_AddNameServer, data->BT_RemoveNameServer, data->STR_NameServer, data->LV_DomainNames, data->BT_AddDomainName, data->BT_RemoveDomainName, data->STR_DomainName, NULL);
      DoMethod(data->CY_HostName          , MUIM_Notify, MUIA_Cycle_Active    , MUIV_EveryTime  , data->STR_HostName      , 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->CH_GetTime           , MUIM_Notify, MUIA_Selected        , MUIV_EveryTime  , data->STR_TimeServer    , 6, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, data->CH_SaveTime, data->STR_TimeServer, NULL);
      DoMethod(data->CH_BOOTP             , MUIM_Notify, MUIA_Selected        , MUIV_EveryTime  , data->STR_BootpServer   , 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

      DoMethod(data->STR_Name             , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Comment);
      DoMethod(data->STR_Comment          , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Login);
      DoMethod(data->STR_Login            , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Password);
      DoMethod(data->STR_Password         , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, obj, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Name);

      DoMethod(data->LI_Interfaces      , MUIM_Notify, MUIA_List_Active     , MUIV_EveryTime , obj, 1, MUIM_ProviderWindow_IfacesActive);
      DoMethod(data->LI_Interfaces      , MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, obj, 1, MUIM_ProviderWindow_EditIface);
      DoMethod(data->BT_AddIface        , MUIM_Notify, MUIA_Pressed         , FALSE          , obj, 2, MUIM_ProviderWindow_Modification, MUIV_ProviderWindow_Modification_AddInterface);
      DoMethod(data->BT_DeleteIface     , MUIM_Notify, MUIA_Pressed         , FALSE          , data->LI_Interfaces, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(data->BT_EditIface       , MUIM_Notify, MUIA_Pressed         , FALSE          , obj, 1, MUIM_ProviderWindow_EditIface);
      DoMethod(data->LV_Script          , MUIM_Notify, MUIA_List_Active     , MUIV_EveryTime , obj, 1, MUIM_ProviderWindow_ScriptActive);
      DoMethod(data->BT_Add             , MUIM_Notify, MUIA_Pressed         , FALSE          , obj, 2, MUIM_ProviderWindow_Modification, MUIV_ProviderWindow_Modification_AddLine);
      DoMethod(data->BT_Remove          , MUIM_Notify, MUIA_Pressed         , FALSE          , data->LI_Script, 2, MUIM_List_Remove, MUIV_List_Remove_Active);
      DoMethod(data->CY_Command         , MUIM_Notify, MUIA_Cycle_Active    , MUIV_EveryTime , obj, 2, MUIM_ProviderWindow_Modification, MUIV_ProviderWindow_Modification_ScriptLine);
      DoMethod(data->STR_String         , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime , obj, 2, MUIM_ProviderWindow_Modification, MUIV_ProviderWindow_Modification_ScriptLine);
      DoMethod(data->STR_Phone          , MUIM_Notify, MUIA_String_Acknowledge,MUIV_EveryTime, obj, 2, MUIM_ProviderWindow_PopString_AddPhone, TRUE);
      DoMethod(data->BT_AddPhone        , MUIM_Notify, MUIA_Pressed         , FALSE          , obj, 2, MUIM_ProviderWindow_PopString_AddPhone, TRUE);
      DoMethod(data->BT_CancelPhone     , MUIM_Notify, MUIA_Pressed         , FALSE          , obj, 2, MUIM_ProviderWindow_PopString_AddPhone, FALSE);
   }
   return((ULONG)obj);
}

///
/// ProviderWindow_Dispatcher
SAVEDS ASM ULONG ProviderWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                                  : return(ProviderWindow_New               (cl, obj, (APTR)msg));
      case MUIM_ProviderWindow_Init                : return(ProviderWindow_Init              (cl, obj, (APTR)msg));
      case MUIM_ProviderWindow_CopyData            : return(ProviderWindow_CopyData          (cl, obj, (APTR)msg));
      case MUIM_ProviderWindow_NameserversActive   : return(ProviderWindow_NameserversActive (cl, obj, (APTR)msg));
      case MUIM_ProviderWindow_DomainnamesActive   : return(ProviderWindow_DomainnamesActive (cl, obj, (APTR)msg));
      case MUIM_ProviderWindow_ScriptActive        : return(ProviderWindow_ScriptActive      (cl, obj, (APTR)msg));
      case MUIM_ProviderWindow_IfacesActive        : return(ProviderWindow_IfacesActive      (cl, obj, (APTR)msg));
      case MUIM_ProviderWindow_PopString_AddPhone  : return(ProviderWindow_PopString_AddPhone(cl, obj, (APTR)msg));
      case MUIM_ProviderWindow_Modification        : return(ProviderWindow_Modification      (cl, obj, (APTR)msg));
      case MUIM_ProviderWindow_EditIface           : return(ProviderWindow_EditIface         (cl, obj, (APTR)msg));
      case MUIM_ProviderWindow_EditIfaceFinish     : return(ProviderWindow_EditIfaceFinish   (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

