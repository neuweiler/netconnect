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
extern Object *win, *app, *li_script;
extern int addr_assign, dst_assign, dns_assign, domainname_assign;
extern char ip[], dest[], dns1[], dns2[], mask[];
extern struct config Config;
extern BOOL no_picture;

extern ULONG setup_page6_colors[];
extern UBYTE setup_page6_body[];

///

/// Finished_ShowConfig
ULONG Finished_ShowConfig(struct IClass *cl, Object *obj, Msg msg)
{
   struct Finished_Data *data = INST_DATA(cl, obj);
   Object *window, *script;
   char ip_info[41], dest_info[41], dns1_info[41], dns2_info[41], device_info[81];

   if(addr_assign == CNF_Assign_Static)
      sprintf(ip_info, "%ls (%ls)", ip, GetStr(MSG_TX_Static));
   else
   {
      if(addr_assign == CNF_Assign_BootP)
         sprintf(ip_info, "%ls (BOOTP)", GetStr(MSG_TX_Dynamic));
      else
         sprintf(ip_info, "%ls (ICPC)", GetStr(MSG_TX_Dynamic));
   }

   if(*dest)
   {
      if(dst_assign == CNF_Assign_BootP)
         sprintf(dest_info, "%ls (BOOTP)", dest);
      else
         sprintf(dest_info, "%ls (ICPC)", dest);
   }
   else
      strcpy(dest_info, GetStr(MSG_TX_Undefined));

   if(*dns1)
   {
      if(dns_assign == CNF_Assign_BootP)
         sprintf(dns1_info, "%ls (BOOTP)", dns1);
      else if(dns_assign == CNF_Assign_Root)
         sprintf(dns1_info, "%ls (ROOT)", dns1);
      else
         sprintf(dns1_info, "%ls (MSDNS)", dns1);
   }
   else
      strcpy(dns1_info, GetStr(MSG_TX_Undefined));

   if(*dns2)
   {
      if(dns_assign == CNF_Assign_BootP)
         sprintf(dns2_info, "%ls (BOOTP)", dns2);
      else if(dns_assign == CNF_Assign_Root)
         sprintf(dns2_info, "%ls (ROOT)", dns2);
      else
         sprintf(dns2_info, "%ls (MSDNS)", dns2);
   }
   else
      strcpy(dns2_info, GetStr(MSG_TX_Undefined));

   if(strcmp(Config.cnf_ifname, "ppp") && strcmp(Config.cnf_ifname, "slip"))
      sprintf(device_info, "%ls, unit %ld", FilePart(Config.cnf_sana2device), Config.cnf_sana2unit);
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
               Child, MakeText(Config.cnf_phonenumber),
               Child, Label(GetStr(MSG_BLA_LoginName)),
               Child, MakeText(Config.cnf_loginname),
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
               Child, MakeText((*mask ? (STRPTR)mask : GetStr(MSG_TX_Undefined))),
            End,
            Child, ColGroup(2),
               GroupFrame,
               MUIA_Background, MUII_GroupBack,
               Child, Label(GetStr(MSG_BLA_DNS1IPAddr)),
               Child, MakeText(dns1_info),
               Child, Label(GetStr(MSG_BLA_DNS2IPAddr)),
               Child, MakeText(dns2_info),
               Child, Label(GetStr(MSG_BLA_DomainName)),
               Child, MakeText(Config.cnf_domainname),
            End,
         End,
         Child, script = ListviewObject,
            MUIA_ShowMe    , (!strcmp(Config.cnf_ifname, "ppp") || !strcmp(Config.cnf_ifname, "slip")),
            MUIA_FrameTitle, GetStr(MSG_LA_LoginScript_List),
            MUIA_Listview_Input, FALSE,
            MUIA_Listview_List, ListObject,
               ReadListFrame,
               MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
               MUIA_List_DestructHook, MUIV_List_DestructHook_String,
            End,
         End,
      End,
   End)
   {
      STRPTR ptr;
      LONG pos;

      DoMethod(window, MUIM_Notify, MUIA_Window_CloseRequest, TRUE ,
         MUIV_Notify_Application, 5, MUIM_Application_PushMethod, win, 2, MUIM_MainWindow_DisposeWindow, window);

      set(app, MUIA_Application_Sleep, TRUE);
      DoMethod(app, OM_ADDMEMBER, window);

      pos = 0;
      FOREVER
      {
         DoMethod(li_script, MUIM_List_GetEntry, pos++, &ptr);
         if(!ptr)
            break;
         DoMethod(script, MUIM_List_InsertSingle, ptr, MUIV_List_Insert_Bottom);
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
            Child, MakePopAsl(tmp.STR_Config = MakeString("AmiTCP:config/GenesisWizard.config", MAXPATHLEN), MSG_TX_SaveConfiguration, FALSE),
         End,
         Child, HVSpace,
         Child, MakeText(GetStr(MSG_TX_InfoSaveInfo)),
         Child, HGroup,
            Child, tmp.CH_Info = CheckMark(TRUE),
            Child, MakePopAsl(tmp.STR_Info = MakeString("PROGDIR:GenesisWizard.log", MAXPATHLEN), MSG_TX_SaveInformation, FALSE),
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

