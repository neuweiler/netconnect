/// includes
#include "/includes.h"

#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_Finished.h"
#include "mui_MainWindow.h"
#include "protos.h"

///
/// external variables
extern struct Library *MUIMasterBase;
extern Object *win, *app;
extern int addr_assign, dst_assign, dns_assign, domainname_assign, gw_assign;
extern struct Hook deshook;
extern struct Config Config;
extern struct ISP ISP;
extern struct Interface Iface;
extern BOOL use_modem;

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
      *array++ = GetStr(MSG_TX_LoginScriptTitleCommand);
      *array   = GetStr(MSG_TX_LoginScriptTitleString);
   }
   return(NULL);
}
static const struct Hook ScriptList_DisplayHook= { { 0,0 }, (VOID *)ScriptList_DisplayFunc , NULL, NULL };

///
/// Finished_ShowConfig
ULONG Finished_ShowConfig(struct IClass *cl, Object *obj, Msg msg)
{
//   struct Finished_Data *data = INST_DATA(cl, obj);
   struct ScriptLine *script_line;
   struct ServerEntry *server;
   Object *window, *script;
   char ip_info[41], dest_info[41], gw_info[41], dns1_info[41], dns2_info[41], device_info[81], domain_info[81];

   if(addr_assign == ASSIGN_Static)
      sprintf(ip_info, "%ls (%ls)", Iface.if_addr, GetStr(MSG_TX_Static));
   else
   {
      strcpy(ip_info, GetStr(MSG_TX_Dynamic));
      strcat(ip_info, (addr_assign == ASSIGN_BOOTP ? " (BOOTP)" : " (ICPC)"));
   }

   if(dst_assign == ASSIGN_Static)
      sprintf(dest_info, "%ls (%ls)", Iface.if_dst, GetStr(MSG_TX_Static));
   else
   {
      strcpy(dest_info, GetStr(MSG_TX_Dynamic));
      strcat(dest_info, (dst_assign == ASSIGN_BOOTP ? " (BOOTP)" : " (ICPC)"));
   }

   if(gw_assign == ASSIGN_Static)
      sprintf(gw_info, "%ls (%ls)", Iface.if_gateway, GetStr(MSG_TX_Static));
   else
   {
      strcpy(gw_info, GetStr(MSG_TX_Dynamic));
      strcat(gw_info, (gw_assign == ASSIGN_BOOTP ? " (BOOTP)" : " (ICPC)"));
   }

   *dns1_info = *dns2_info = NULL;
   if(Iface.if_nameservers.mlh_TailPred != (struct MinNode *)&Iface.if_nameservers)
   {
      server = (struct ServerEntry *)Iface.if_nameservers.mlh_TailPred;
      if(server->se_node.mln_Pred)
      {
         strcpy(dns1_info, server->se_name);
         strcat(dns1_info, (dns_assign == ASSIGN_BOOTP ? " (BOOTP)" : (dns_assign == ASSIGN_Root ? " (ROOT)" : " (MSDNS)")));
         server = (struct ServerEntry *)server->se_node.mln_Pred;
         if(server->se_node.mln_Pred)
         {
            strcpy(dns2_info, server->se_name);
            strcat(dns2_info, (dns_assign == ASSIGN_BOOTP ? " (BOOTP)" : (dns_assign == ASSIGN_Root ? " (ROOT)" : " (MSDNS)")));
         }
      }
   }
   if(!*dns1_info)
      strcpy(dns1_info, GetStr(MSG_TX_Undefined));
   if(!*dns2_info)
      strcpy(dns2_info, GetStr(MSG_TX_Undefined));

   *domain_info = NULL;
   if(Iface.if_domainnames.mlh_TailPred != (struct MinNode *)&Iface.if_domainnames)
   {
      server = (struct ServerEntry *)Iface.if_domainnames.mlh_TailPred;
      if(server->se_node.mln_Pred)
         strcpy(domain_info, server->se_name);
   }
   if(!*domain_info)
      strcpy(domain_info, GetStr(MSG_TX_Undefined));

   sprintf(device_info, "%ls, unit %ld", FilePart(Iface.if_sana2device), Iface.if_sana2unit);

   if(window = WindowObject,
      MUIA_Window_Title , GetStr(MSG_TX_ShowConfigWindowTitle),
      MUIA_Window_ID    , MAKE_ID('N','C','O','F'),
      WindowContents    , VGroup,
         Child, CLabel(GetStr(MSG_LA_ShowConfigTitle)),
         Child, ColGroup(2),
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, Label(GetStr(MSG_LA_IfaceName)),
            Child, MakeText(Iface.if_name),
            Child, Label(GetStr(MSG_LA_Sana2Device)),
            Child, MakeText(device_info),
         End,
         Child, ColGroup(2),
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, Label(GetStr(MSG_LA_IPAddr)),
            Child, MakeText(ip_info),
            Child, Label(GetStr(MSG_LA_Destination)),
            Child, MakeText(dest_info),
            Child, Label(GetStr(MSG_LA_Gateway)),
            Child, MakeText(gw_info),
            Child, Label(GetStr(MSG_LA_Netmask)),
            Child, MakeText((*Iface.if_netmask ? (STRPTR)Iface.if_netmask : GetStr(MSG_TX_Undefined))),
         End,
         Child, ColGroup(2),
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, Label(GetStr(MSG_LA_DNS1IPAddr)),
            Child, MakeText(dns1_info),
            Child, Label(GetStr(MSG_LA_DNS2IPAddr)),
            Child, MakeText(dns2_info),
            Child, Label(GetStr(MSG_LA_DomainName)),
            Child, MakeText(domain_info),
            Child, Label(GetStr(MSG_LA_HostName)),
            Child, MakeText((*Iface.if_hostname ? (STRPTR)Iface.if_hostname : GetStr(MSG_TX_Undefined))),
         End,
         Child, script = ListviewObject,
            MUIA_ShowMe          , use_modem,
            MUIA_FrameTitle      , GetStr(MSG_LA_LoginScript_List),
            MUIA_Listview_Input  , FALSE,
            MUIA_Listview_List   , ListObject,
               ReadListFrame,
               MUIA_List_ConstructHook, &ScriptList_ConstructHook,
               MUIA_List_DisplayHook  , &ScriptList_DisplayHook,
               MUIA_List_DestructHook , &deshook,
               MUIA_List_Format       , "BAR,",
               MUIA_List_Title        , TRUE,
            End,
         End,
      End,
   End)
   {
      DoMethod(window, MUIM_Notify, MUIA_Window_CloseRequest, TRUE ,
         MUIV_Notify_Application, 5, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, window);

      set(app, MUIA_Application_Sleep, TRUE);
      DoMethod(app, OM_ADDMEMBER, window);

      if(Iface.if_loginscript.mlh_TailPred != (struct MinNode *)&Iface.if_loginscript)
      {
         script_line = (struct ScriptLine *)Iface.if_loginscript.mlh_Head;
         while(script_line->sl_node.mln_Succ)
         {
            DoMethod(script, MUIM_List_InsertSingle, script_line, MUIV_List_Insert_Bottom);
            script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
         }
      }

      set(window, MUIA_Window_Open, TRUE);
   }
   return(NULL);
}

///

/// Finished_New
ULONG Finished_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Finished_Data tmp;

   if(obj = (Object *)DoSuperNew(cl, obj,
      GroupFrame,
      MUIA_Background, MUII_TextBack,
      Child, HVSpace,
      Child, MakeText(GetStr(MSG_TX_InfoSaveConfig)),
      Child, HGroup,
         Child, tmp.CH_Config = CheckMark(TRUE),
         Child, MakePopAsl(tmp.STR_Config = MakeString(DEFAULT_CONFIGFILE, MAXPATHLEN), MSG_TX_SaveConfiguration, FALSE),
      End,
      Child, HVSpace,
      Child, MakeText(GetStr(MSG_TX_InfoSaveInfo)),
      Child, HGroup,
         Child, tmp.CH_Info = CheckMark(TRUE),
         Child, MakePopAsl(tmp.STR_Info = MakeString("AmiTCP:log/GENESiSWizard.log", MAXPATHLEN), MSG_TX_SaveInformation, FALSE),
      End,
      Child, HVSpace,
      Child, MakeText(GetStr(MSG_TX_InfoPrint)),
      Child, HGroup,
         Child, tmp.CH_Printer = CheckMark(FALSE),
         Child, tmp.STR_Printer = MakeString("PRT:", 80),
      End,
      Child, HVSpace,
      Child, HGroup,
         Child, HVSpace,
         Child, tmp.BT_ShowConfig = MakeButton(MSG_BT_ViewConfigInfo),
         Child, HVSpace,
      End,
      Child, HVSpace,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Finished_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      set(data->BT_ShowConfig, MUIA_Weight, 300);
      set(data->CH_Config , MUIA_CycleChain, 1);
      set(data->CH_Info   , MUIA_CycleChain, 1);
      set(data->CH_Printer, MUIA_CycleChain, 1);

      set(data->CH_Config        , MUIA_ShortHelp, GetStr(MSG_HELP_DoSaveConfig));
      set(data->CH_Info          , MUIA_ShortHelp, GetStr(MSG_HELP_DoSaveInfo));
      set(data->CH_Printer       , MUIA_ShortHelp, GetStr(MSG_HELP_DoPrint));
      set(data->STR_Config       , MUIA_ShortHelp, GetStr(MSG_HELP_ConfigFilename));
      set(data->STR_Info         , MUIA_ShortHelp, GetStr(MSG_HELP_InfoFilename));
      set(data->STR_Printer      , MUIA_ShortHelp, GetStr(MSG_HELP_PrinterDevice));
      set(data->BT_ShowConfig    , MUIA_ShortHelp, GetStr(MSG_HELP_ShowConfig));

      DoMethod(data->BT_ShowConfig       , MUIM_Notify, MUIA_Pressed   , FALSE, obj, 1, MUIM_Finished_ShowConfig);
   }

   return((ULONG)obj);
}

///
/// Finished_Dispatcher
SAVEDS ASM ULONG Finished_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                   : return(Finished_New         (cl, obj, (APTR)msg));
      case MUIM_Finished_ShowConfig : return(Finished_ShowConfig  (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

