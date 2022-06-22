#include "globals.c"
#include "protos.h"

/// Provider_Sana2Cycle
ULONG Provider_Sana2Cycle(struct IClass *cl, Object *obj, Msg msg)
{
   struct Provider_Data *data = INST_DATA(cl, obj);
   LONG value;

   value = xget(data->CY_Sana2Device, MUIA_Cycle_Active);

   DoMethod(data->STR_Sana2Device, MUIM_MultiSet, MUIA_Disabled, value != 2,
     data->PA_Sana2Device, data->STR_Sana2Unit, data->PA_Sana2ConfigFile, data->TI_Sana2ConfigContents, NULL);

   switch(value)
   {
      case 0:  /** ppp **/
         setstring(data->STR_Sana2Device, "DEVS:Networks/appp.device");
         set(data->STR_Sana2Unit, MUIA_String_Integer, 0);
         setstring(data->STR_Sana2ConfigFile, "ENV:Sana2/appp0.config");
         setstring(data->TI_Sana2ConfigContents, get_configcontents(TRUE));
         setstring(data->STR_IfaceName, "ppp");
         setstring(data->STR_IfaceConfigParams, "");
         break;

      case 1:  /** slip **/
         setstring(data->STR_Sana2Device, "DEVS:Networks/aslip.device");
         set(data->STR_Sana2Unit, MUIA_String_Integer, 0);
         setstring(data->STR_Sana2ConfigFile, "ENV:Sana2/aslip0.config");
         setstring(data->TI_Sana2ConfigContents, get_configcontents(FALSE));
         setstring(data->STR_IfaceName, "slip");
         setstring(data->STR_IfaceConfigParams, "");
         break;
   }

   return(NULL);
}

///
/// Provider_Reset
#ifdef FIX_PROVIDER
ULONG Provider_Reset(struct IClass *cl, Object *obj, Msg msg)
{
   struct Provider_Data *data = INST_DATA(cl, obj);

   set(data->STR_DomainName   , MUIA_String_Contents, "demon.co.uk");
   set(data->STR_NameServer1  , MUIA_String_Contents, "158.152.1.58");
   set(data->STR_NameServer2  , MUIA_String_Contents, "158.152.1.43");
   set(data->CY_Authentication, MUIA_Cycle_Active, 0);
   set(data->STR_MailServer   , MUIA_String_Contents, "post.demon.co.uk");
   set(data->STR_POPServer    , MUIA_String_Contents, "sdps.demon.co.uk");
   set(data->STR_NewsServer   , MUIA_String_Contents, "news.demon.co.uk");
   set(data->STR_WWWServer    , MUIA_String_Contents, "www.demon.net.uk");
   set(data->STR_FTPServer    , MUIA_String_Contents, "ftp.demon.net.uk");
   set(data->STR_IRCServer    , MUIA_String_Contents, "irc.demon.co.uk");
   set(data->STR_ProxyServer  , MUIA_String_Contents, "www-cache.demon.co.uk");
   set(data->STR_ProxyPort    , MUIA_String_Integer, 8080);
   set(data->STR_TimeServer   , MUIA_String_Contents, "158.152.1.58");

   return(NULL);
}
#endif

///
/// Provider_PopString_Close
ULONG Provider_PopString_Close(struct IClass *cl, Object *obj, struct MUIP_Provider_PopString_Close *msg)
{
   struct Provider_Data *data = INST_DATA(cl, obj);
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
      "IPTYPE=2048 NOARP P2P IPREQ=8 WRITEREQ=8"     // ariadneliana0
      };

   if(msg->flags == MUIV_Provider_PopString_IfaceName)
   {
      if((pos = xget(data->LV_IfaceNames, MUIA_List_Active)) != MUIV_List_Active_Off)
      {
         DoMethod(data->LV_IfaceNames, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
         if(ptr)
         {
            setstring(data->STR_IfaceName, ptr);
            setstring(data->STR_IfaceConfigParams, config_strings[pos]);
         }
         DoMethod(data->PO_IfaceName, MUIM_Popstring_Close, TRUE);
      }
   }

   return(NULL);
}


/// Provider_New

/// Provider_New
ULONG Provider_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   static STRPTR STR_CY_Dynamic[3];
   static STRPTR STR_CY_SanaDevice[4];
   static STRPTR STR_CY_Ping[4];
   static STRPTR STR_PO_Interfaces[] = { "ppp", "slip", "plip0", "magplip0", "ether0", "a2065", "a4066",
         "hydra", "wd80xx", "eb920", "quicknet","gg_3c503", "gg_ne1000", "gg_smc", "arcnet0", "a2060",
         "ax25", "liana0", "ariadneliana0", NULL };
   struct Provider_Data tmp;

   STR_CY_Dynamic[0] = GetStr(MSG_CY_Address0);
   STR_CY_Dynamic[1] = GetStr(MSG_CY_Address1);
   STR_CY_Dynamic[2] = NULL;

   STR_CY_SanaDevice[0] = GetStr("  PPP");
   STR_CY_SanaDevice[1] = GetStr("  SLIP");
   STR_CY_SanaDevice[2] = GetStr("  specify sana-II device");
   STR_CY_SanaDevice[3] = NULL;

   STR_CY_Ping[0] = GetStr("  disabled");
   STR_CY_Ping[1] = GetStr("  ICMP ping");
   STR_CY_Ping[2] = GetStr("  PPP ping");
   STR_CY_Ping[3] = NULL;

   if(obj = tmp.GR_TCPIP = (Object *)DoSuperNew(cl, obj,
      MUIA_InnerLeft, 0,
      MUIA_InnerRight, 0,
      MUIA_InnerBottom, 0,
      MUIA_InnerTop, 0,
      Child, HVSpace,
      Child, MUI_MakeObject(MUIO_BarTitle, "Domain configuration"),
      Child, ColGroup(2),
         Child, MakeKeyLabel2(MSG_LA_DomainName, MSG_CC_DomainName),
         Child, tmp.STR_DomainName = MakeKeyString("", 80, MSG_CC_DomainName),
         Child, MakeKeyLabel2(MSG_LA_NameServer1, MSG_CC_NameServer1),
         Child, tmp.STR_NameServer1 = TextinputObject,
            MUIA_ControlChar     , *GetStr(MSG_CC_NameServer1),
            MUIA_CycleChain      , 1,
            MUIA_Frame           , MUIV_Frame_String,
            MUIA_String_Accept   , "0123456789.",
            MUIA_String_MaxLen   , 18,
         End,
         Child, MakeKeyLabel2(MSG_LA_NameServer2, MSG_CC_NameServer2),
         Child, tmp.STR_NameServer2 = TextinputObject,
            MUIA_ControlChar     , *GetStr(MSG_CC_NameServer2),
            MUIA_CycleChain      , 1,
            MUIA_Frame           , MUIV_Frame_String,
            MUIA_String_Accept   , "0123456789.",
            MUIA_String_MaxLen   , 18,
         End,
         Child, MakeKeyLabel2("  Configuration:", "  o"),
         Child, HGroup,
            Child, tmp.CY_Resolv = MakeKeyCycle(STR_CY_Dynamic, "  o"),
            Child, HVSpace,
         End,
      End,
      Child, HVSpace,
      Child, MUI_MakeObject(MUIO_BarTitle, "Local host"),
      Child, ColGroup(2),
         Child, MakeKeyLabel2(MSG_LA_Address, MSG_CC_Address),
         Child, HGroup,
            Child, tmp.STR_IP_Address = TextinputObject,
               MUIA_ControlChar     , *GetStr(MSG_CC_Address),
               MUIA_CycleChain      , 1,
               MUIA_Frame           , MUIV_Frame_String,
               MUIA_String_Contents , "0.0.0.0",
               MUIA_String_Accept   , "0123456789.",
               MUIA_String_MaxLen   , 18,
            End,
            Child, tmp.CY_Address = Cycle(STR_CY_Dynamic),
         End,
         Child, MakeKeyLabel2(MSG_LA_HostName, MSG_CC_HostName),
         Child, tmp.STR_HostName = MakeKeyString("", 80, MSG_CC_HostName),
      End,
      Child, HVSpace,
      TAG_MORE, msg->ops_AttrList))
   {
      struct Provider_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      data->GR_Sana2 = VGroup,
         MUIA_InnerLeft, 0,
         MUIA_InnerRight, 0,
         MUIA_InnerBottom, 0,
         MUIA_InnerTop, 0,
         Child, VGroup,
            Child, MUI_MakeObject(MUIO_BarTitle, "Device driver"),
            Child, data->CY_Sana2Device = Cycle(STR_CY_SanaDevice),
            Child, HGroup,
               Child, data->PA_Sana2Device = MakePopAsl(data->STR_Sana2Device = MakeKeyString("DEVS:Networks/", MAXPATHLEN, "  d"), "Choose sana II device", FALSE),
               Child, MakeKeyLabel2("  Unit:", "  u"),
               Child, data->STR_Sana2Unit = TextinputObject,
                  MUIA_Weight          , 5,
                  MUIA_ControlChar     , *GetStr("  u"),
                  MUIA_CycleChain      , 1,
                  StringFrame,
                  MUIA_String_MaxLen   , 5,
                  MUIA_String_Integer  , 0,
                  MUIA_String_Accept   , "1234567890",
               End,
            End,
            Child, RectangleObject, MUIA_Weight, 0, End,
            Child, MUI_MakeObject(MUIO_BarTitle, "Config file"),
            Child, data->PA_Sana2ConfigFile = MakePopAsl(data->STR_Sana2ConfigFile = MakeKeyString("ENV:Sana2/", MAXPATHLEN, "  f"), "Choose name for sana II config file", FALSE),
            Child, data->TI_Sana2ConfigContents = TextinputscrollObject,
               StringFrame,
               MUIA_CycleChain, 1,
               MUIA_ControlChar, GetStr("  o"),
               MUIA_Textinput_Multiline, TRUE,
               MUIA_Textinput_WordWrap, 512,
               MUIA_Textinput_AutoExpand, TRUE,
            End,
         End,
      End;

      data->GR_Interface = VGroup,
MUIA_InnerLeft, 0,
MUIA_InnerRight, 0,
MUIA_InnerBottom, 0,
MUIA_InnerTop, 0,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Interface configuration"),
         Child, ColGroup(2),
            Child, MakeKeyLabel2("  Name:", "  n"),
            Child, data->PO_IfaceName = PopobjectObject,
               MUIA_Popstring_String      , data->STR_IfaceName = MakeKeyString("ppp", 16, "  n"),
               MUIA_Popstring_Button      , PopButton(MUII_PopUp),
               MUIA_Popobject_StrObjHook  , &strobjhook,
               MUIA_Popobject_Object      , data->LV_IfaceNames = ListviewObject,
                  MUIA_Listview_DoubleClick  , TRUE,
                  MUIA_Listview_List         , ListObject,
                     MUIA_Frame              , MUIV_Frame_InputList,
                     MUIA_List_AutoVisible   , TRUE,
                     MUIA_List_SourceArray   , STR_PO_Interfaces,
                  End,
               End,
            End,
            Child, MakeKeyLabel2("  Config:", "  p"),
            Child, data->STR_IfaceConfigParams = MakeKeyString(NULL, 1024, "  p"),
            Child, MakeKeyLabel2(MSG_LA_MTU, MSG_CC_MTU),
            Child, HGroup,
               Child, data->STR_MTU = TextinputObject,
                  MUIA_ControlChar     , *GetStr(MSG_CC_MTU),
                  MUIA_CycleChain      , 1,
                  StringFrame,
                  MUIA_String_MaxLen   , 6,
                  MUIA_String_Integer  , 1500,
                  MUIA_String_Accept   , "1234567890",
               End,
               Child, HVSpace,
            End,
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Options"),
         Child, ColGroup(2),
            Child, MakeKeyLabel2("  Keep alive:", "  k"),
            Child, HGroup,
               Child, data->CY_Ping = MakeKeyCycle(STR_CY_Ping, "  k"),
               Child, data->SL_Ping = NumericbuttonObject,
                  MUIA_CycleChain      , 1,
                  MUIA_Numeric_Min     , 1,
                  MUIA_Numeric_Max     , 60,
                  MUIA_Numeric_Value   , 10,
               End,
               Child, LLabel("minutes to avoid timeout"),
            End,
            Child, MakeKeyLabel2("  Use BOOTP:", MSG_CC_BOOTP),
            Child, HGroup,
               Child, data->CH_BOOTP = MakeKeyCheckMark(FALSE, MSG_CC_BOOTP),
               Child, HVSpace,
            End,
         End,
         Child, HVSpace,
      End;

      data->GR_Server = VGroup,
MUIA_InnerLeft, 0,
MUIA_InnerRight, 0,
MUIA_InnerBottom, 0,
MUIA_InnerTop, 0,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Communication"),
         Child, ColGroup(2),
            Child, MakeKeyLabel2(MSG_LA_MailServer, MSG_CC_MailServer),
            Child, data->STR_MailServer = MakeKeyString("", 80, MSG_CC_MailServer),
            Child, MakeKeyLabel2(MSG_LA_POP3Server, MSG_CC_POP3Server),
            Child, data->STR_POPServer = MakeKeyString("", 80, MSG_CC_POP3Server),
            Child, MakeKeyLabel2(MSG_LA_NewsServer, MSG_CC_NewsServer),
            Child, data->STR_NewsServer = MakeKeyString("", 80, MSG_CC_NewsServer),
            Child, MakeKeyLabel2(MSG_LA_IRCServer, MSG_CC_IRCServer),
            Child, HGroup,
               Child, data->STR_IRCServer = MakeKeyString("", 80, MSG_CC_IRCServer),
               Child, MakeKeyLabel2(MSG_LA_IRCPort, MSG_CC_IRCPort),
               Child, data->STR_IRCPort = TextinputObject,
                  MUIA_Weight          , 50,
                  MUIA_ControlChar     , *GetStr(MSG_CC_IRCPort),
                  MUIA_CycleChain      , 1,
                  MUIA_Frame           , MUIV_Frame_String,
                  MUIA_String_Accept   , "0123456789",
                  MUIA_String_MaxLen   , 10,
               End,
            End,
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, "Control"),
         Child, ColGroup(2),
            Child, MakeKeyLabel2(MSG_LA_TimeServer, MSG_CC_TimeServer),
            Child, data->STR_TimeServer = MakeKeyString("", 80, MSG_CC_TimeServer),
            Child, MakeKeyLabel2(MSG_LA_ProxyServer, MSG_CC_ProxyServer),
            Child, HGroup,
               Child, data->STR_ProxyServer = MakeKeyString("", 80, MSG_CC_ProxyServer),
               Child, MakeKeyLabel2(MSG_LA_ProxyPort, MSG_CC_ProxyPort),
               Child, data->STR_ProxyPort = TextinputObject,
                  MUIA_Weight          , 50,
                  MUIA_ControlChar     , *GetStr(MSG_CC_ProxyPort),
                  MUIA_CycleChain      , 1,
                  MUIA_Frame           , MUIV_Frame_String,
                  MUIA_String_Accept   , "0123456789",
                  MUIA_String_MaxLen   , 10,
               End,
            End,
         End,
         Child, HVSpace,
      End;

      if(data->GR_Interface && data->GR_Server)
      {
         set(data->STR_DomainName        , MUIA_Disabled, TRUE);
         set(data->STR_NameServer1       , MUIA_Disabled, TRUE);
         set(data->STR_NameServer2       , MUIA_Disabled, TRUE);
         set(data->STR_IP_Address        , MUIA_Disabled, TRUE);
         set(data->STR_HostName          , MUIA_Disabled, TRUE);
         set(data->PA_Sana2Device        , MUIA_Disabled, TRUE);
         set(data->STR_Sana2Unit         , MUIA_Disabled, TRUE);
         set(data->PA_Sana2ConfigFile    , MUIA_Disabled, TRUE);
         set(data->TI_Sana2ConfigContents, MUIA_Disabled, TRUE);
         set(data->SL_Ping               , MUIA_Disabled, TRUE);
         set(data->CY_Address       , MUIA_Weight, 0);

         set(data->CH_BOOTP        , MUIA_CycleChain, 1);
         set(data->CY_Resolv       , MUIA_CycleChain, 1);
         set(data->CY_Address      , MUIA_CycleChain, 1);
         set(data->CY_Sana2Device  , MUIA_CycleChain, 1);

         set(data->STR_DomainName  , MUIA_ShortHelp, GetStr(MSG_Help_DomainName));
         set(data->STR_NameServer1 , MUIA_ShortHelp, GetStr(MSG_Help_NameServer1));
         set(data->STR_NameServer2 , MUIA_ShortHelp, GetStr(MSG_Help_NameServer2));
         set(data->STR_ProxyServer , MUIA_ShortHelp, GetStr(MSG_Help_ProxyServer));
         set(data->STR_ProxyPort   , MUIA_ShortHelp, GetStr(MSG_Help_ProxyPort));
         set(data->STR_HostName    , MUIA_ShortHelp, GetStr(MSG_Help_HostName));
         set(data->STR_IP_Address  , MUIA_ShortHelp, GetStr(MSG_Help_IP_Address));
         set(data->CY_Address      , MUIA_ShortHelp, GetStr(MSG_Help_Address));
         set(data->CH_BOOTP        , MUIA_ShortHelp, GetStr(MSG_Help_BOOTP));
         set(data->STR_MTU         , MUIA_ShortHelp, GetStr(MSG_Help_MTU));

         set(data->STR_MailServer  , MUIA_ShortHelp, GetStr(MSG_Help_MailServer));
         set(data->STR_POPServer   , MUIA_ShortHelp, GetStr(MSG_Help_POPServer));
         set(data->STR_NewsServer  , MUIA_ShortHelp, GetStr(MSG_Help_NewsServer));
         set(data->STR_IRCServer   , MUIA_ShortHelp, GetStr(MSG_Help_IRCServer));
         set(data->STR_IRCPort     , MUIA_ShortHelp, GetStr(MSG_Help_IRCPort));
         set(data->STR_TimeServer  , MUIA_ShortHelp, GetStr(MSG_Help_TimeServer));

         DoMethod(data->CY_Resolv     , MUIM_Notify, MUIA_Cycle_Active       , MUIV_EveryTime  , data->STR_DomainName    , 7, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, data->STR_DomainName, data->STR_NameServer1, data->STR_NameServer2, NULL);
         DoMethod(data->CY_Address    , MUIM_Notify, MUIA_Cycle_Active       , MUIV_EveryTime  , data->STR_IP_Address    , 6, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, data->STR_IP_Address, data->STR_HostName, NULL);
         DoMethod(data->CY_Sana2Device, MUIM_Notify, MUIA_Cycle_Active       , MUIV_EveryTime  , obj, 1, MUIM_Provider_Sana2Cycle);
         DoMethod(data->LV_IfaceNames , MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime , obj, 2, MUIM_Provider_PopString_Close, MUIV_Provider_PopString_IfaceName);
         DoMethod(data->CY_Ping       , MUIM_Notify, MUIA_Cycle_Active       , MUIV_EveryTime  , data->SL_Ping           , 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

         DoMethod(data->STR_DomainName       , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_NameServer1);
         DoMethod(data->STR_NameServer1      , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_NameServer2);
         DoMethod(data->STR_NameServer2      , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_IP_Address);
         DoMethod(data->STR_IP_Address       , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_HostName);
         DoMethod(data->STR_Sana2Device      , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Sana2Unit);
         DoMethod(data->STR_Sana2Unit        , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_Sana2ConfigFile);
         DoMethod(data->STR_IfaceName        , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_IfaceConfigParams);
         DoMethod(data->STR_IfaceConfigParams, MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_MTU);
         DoMethod(data->STR_MailServer       , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_POPServer);
         DoMethod(data->STR_POPServer        , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_NewsServer);
         DoMethod(data->STR_NewsServer       , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_TimeServer);
         DoMethod(data->STR_TimeServer       , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_IRCServer);
         DoMethod(data->STR_IRCServer        , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_IRCPort);
         DoMethod(data->STR_IRCPort          , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_ProxyServer);
         DoMethod(data->STR_ProxyServer      , MUIM_Notify, MUIA_String_Acknowledge , MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, data->STR_ProxyPort);

#ifdef FIX_PROVIDER
         DoMethod(data->STR_DomainName   , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
         DoMethod(data->STR_NameServer1  , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
         DoMethod(data->STR_NameServer2  , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
         DoMethod(data->STR_MailServer   , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
         DoMethod(data->STR_POPServer    , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
         DoMethod(data->STR_NewsServer   , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
         DoMethod(data->STR_IRCServer    , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
         DoMethod(data->STR_ProxyServer  , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
         DoMethod(data->STR_ProxyPort    , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
         DoMethod(data->STR_TimeServer   , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, obj, 1, MUIM_Provider_Reset);
#endif
      }
      else
         obj = NULL;
   }
   return((ULONG)obj);
}

///
/// Provider_Dispatcher
SAVEDS ASM ULONG Provider_Dispatcher(REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)
{
   switch (msg->MethodID)
   {
      case OM_NEW                                  : return(Provider_New                        (cl, obj, (APTR)msg));
      case MUIM_Provider_Sana2Cycle                : return(Provider_Sana2Cycle                 (cl, obj, (APTR)msg));
      case MUIM_Provider_PopString_Close           : return(Provider_PopString_Close            (cl, obj, (APTR)msg));
#ifdef FIX_PROVIDER
      case MUIM_Provider_Reset                     : return(Provider_Reset                      (cl, obj, (APTR)msg));
#endif
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///

