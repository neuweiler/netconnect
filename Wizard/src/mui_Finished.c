/// includes
#include "/includes.h"
#pragma header

#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "mui_Finished.h"
#include "mui_MainWindow.h"
#include "protos.h"

#include "images/setup_page6.h"
///
/// external variables
extern Object *win, *app;
extern int addr_assign, dst_assign, dns_assign, domainname_assign;
extern struct Hook deshook;
extern struct Config Config;
extern struct ISP ISP;
extern struct Interface Iface;
extern BOOL no_picture;

extern ULONG setup_page6_colors[];
extern UBYTE setup_page6_body[];

///

/// ScriptList_ConstructFunc
struct ScriptLine * SAVEDS ScriptList_ConstructFunc(register __a2 APTR pool, register __a1 struct ScriptLine *src)
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
SAVEDS LONG ScriptList_DisplayFunc(register __a2 char **array, register __a1 struct ScriptLine *script_line)
{
   if(script_line)
   {
      *array++ = script_commands[script_line->sl_command];
      *array = script_line->sl_contents;
   }
   else
   {
      *array++ = GetStr("  \033bCommand");
      *array   = GetStr("  \033bString");
   }
   return(NULL);
}
static const struct Hook ScriptList_DisplayHook= { { 0,0 }, (VOID *)ScriptList_DisplayFunc , NULL, NULL };

///
/// Finished_ShowConfig
ULONG Finished_ShowConfig(struct IClass *cl, Object *obj, Msg msg)
{
   struct Finished_Data *data = INST_DATA(cl, obj);
   Object *window, *script;
   char ip_info[41], dest_info[41], dns1_info[41], dns2_info[41], device_info[81];

   if(addr_assign == CNF_Assign_Static)
      sprintf(ip_info, "%ls (%ls)", Iface.if_addr, GetStr(MSG_TX_Static));
   else
   {
      strcpy(ip_info, GetStr(MSG_TX_Dynamic));
      strcat(ip_info, (addr_assign == CNF_Assign_BootP ? " (BOOTP)" : " (ICPC)"));
   }

   if(dst_assign == CNF_Assign_Static)
      sprintf(dest_info, "%ls (%ls)", Iface.if_dst, GetStr(MSG_TX_Static));
   else
   {
      strcpy(dest_info, GetStr(MSG_TX_Dynamic));
      strcat(dest_info, (dst_assign == CNF_Assign_BootP ? " (BOOTP)" : " (ICPC)"));
   }
/*
   if(*ISP.isp_dns1)
   {
      strcpy(dns1_info, ISP.isp_dns1);
      strcat(dns1_info, (dns_assign == CNF_Assign_BootP ? " (BOOTP)" : (dns_assign == CNF_Assign_Root ? " (ROOT)" : " (MSDNS)")));
   }
   else
      strcpy(dns1_info, GetStr(MSG_TX_Undefined));

   if(*ISP.isp_dns2)
   {
      strcpy(dns2_info, ISP.isp_dns2);
      strcat(dns2_info, (dns_assign == CNF_Assign_BootP ? " (BOOTP)" : (dns_assign == CNF_Assign_Root ? " (ROOT)" : " (MSDNS)")));
   }
   else
      strcpy(dns2_info, GetStr(MSG_TX_Undefined));
*/
   if(strcmp(Iface.if_name, "ppp") && strcmp(Iface.if_name, "slip"))
      sprintf(device_info, "%ls, unit %ld", FilePart(Iface.if_sana2device), Iface.if_sana2unit);
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
               Child, MakeText(ISP.isp_phonenumber),
               Child, Label(GetStr(MSG_BLA_LoginName)),
               Child, MakeText(ISP.isp_login),
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
               Child, VVSpace,
               Child, HVSpace,
            End,
            Child, ColGroup(2),
               GroupFrame,
               MUIA_Background, MUII_GroupBack,
               Child, Label(GetStr(MSG_BLA_LocalIPAddr)),
               Child, MakeText(ip_info),
               Child, Label(GetStr(MSG_BLA_RemoteIPAddr)),
               Child, MakeText(dest_info),
               Child, Label(GetStr(MSG_BLA_Netmask)),
               Child, MakeText((*Iface.if_netmask ? (STRPTR)Iface.if_netmask : GetStr(MSG_TX_Undefined))),
            End,
            Child, ColGroup(2),
               GroupFrame,
               MUIA_Background, MUII_GroupBack,
               Child, Label(GetStr(MSG_BLA_DNS1IPAddr)),
               Child, MakeText(dns1_info),
               Child, Label(GetStr(MSG_BLA_DNS2IPAddr)),
               Child, MakeText(dns2_info),
               Child, Label(GetStr(MSG_BLA_DomainName)),
Child, HVSpace,
//               Child, MakeText(ISP.isp_domainname),
            End,
         End,
         Child, script = ListviewObject,
            MUIA_ShowMe    , (!strcmp(Iface.if_name, "ppp") || !strcmp(Iface.if_name, "slip")),
            MUIA_FrameTitle, GetStr(MSG_LA_LoginScript_List),
            MUIA_Listview_Input, FALSE,
            MUIA_Listview_List, ListObject,
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


      if(ISP.isp_loginscript.mlh_TailPred != (struct MinNode *)&ISP.isp_loginscript)
      {
         struct ScriptLine *script_line;

         script_line = (struct ScriptLine *)ISP.isp_loginscript.mlh_Head;
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
      MUIA_Group_Horiz, TRUE,
      Child, tmp.GR_Picture = VGroup,
        MUIA_ShowMe, !no_picture,
        Child, BodychunkObject,
            GroupFrame,
            InnerSpacing(0, 0),
            MUIA_FixWidth             , SETUP_PAGE6_WIDTH,
            MUIA_FixHeight            , SETUP_PAGE6_HEIGHT,
            MUIA_Bitmap_Width         , SETUP_PAGE6_WIDTH ,
            MUIA_Bitmap_Height        , SETUP_PAGE6_HEIGHT,
            MUIA_Bodychunk_Depth      , SETUP_PAGE6_DEPTH ,
            MUIA_Bodychunk_Body       , (UBYTE *)setup_page6_body,
            MUIA_Bodychunk_Compression, SETUP_PAGE6_COMPRESSION,
            MUIA_Bodychunk_Masking    , SETUP_PAGE6_MASKING,
            MUIA_Bitmap_SourceColors  , (ULONG *)setup_page6_colors,
         End,
         Child, HVSpace,
      End,
      Child, VGroup,
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
            Child, MakePopAsl(tmp.STR_Info = MakeString("AmiTCP:log/GenesisWizard.log", MAXPATHLEN), MSG_TX_SaveInformation, FALSE),
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
      End,
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
SAVEDS ULONG Finished_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                   : return(Finished_New         (cl, obj, (APTR)msg));
      case MUIM_Finished_ShowConfig : return(Finished_ShowConfig  (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///

