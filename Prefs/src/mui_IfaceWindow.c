/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/genesis_lib.h"
#include "Strings.h"
#include "mui.h"
#include "mui_IfaceWindow.h"
#include "mui_ProviderWindow.h"
#include "protos.h"

///
/// external variables
extern struct Hook strobjhook, des_hook;
extern Object *win;

///

/// IfaceWindow_CopyData
ULONG IfaceWindow_CopyData(struct IClass *cl, Object *obj, Msg msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   ULONG value;

   if(data->iface)
   {
      struct PrefsPPPIface *ppp_if = data->iface->if_userdata;

      strncpy(data->iface->if_name, (STRPTR)xget(data->STR_IfaceName, MUIA_String_Contents), sizeof(data->iface->if_name));
      strncpy(data->iface->if_addr, (STRPTR)xget(data->STR_Address, MUIA_String_Contents), sizeof(data->iface->if_addr));
      if(xget(data->CY_Address, MUIA_Cycle_Active))
      {
         strncpy(data->iface->if_addr, (STRPTR)xget(data->STR_Address, MUIA_String_Contents), sizeof(data->iface->if_addr));
         set(data->STR_Address, MUIA_Disabled, FALSE);
      }
      else
      {
         *data->iface->if_addr = NULL;
         set(data->STR_Address, MUIA_Disabled, TRUE);
      }
      strncpy(data->iface->if_dst, (STRPTR)xget(data->STR_Dest, MUIA_String_Contents), sizeof(data->iface->if_dst));
      strncpy(data->iface->if_gateway, (STRPTR)xget(data->STR_Gateway, MUIA_String_Contents), sizeof(data->iface->if_gateway));
      strncpy(data->iface->if_netmask, (STRPTR)xget(data->STR_Netmask, MUIA_String_Contents), sizeof(data->iface->if_netmask));
      realloc_copy((STRPTR *)&data->iface->if_configparams, (STRPTR)xget(data->STR_ConfigParams, MUIA_String_Contents));
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
            realloc_copy((STRPTR *)&data->iface->if_sana2configtext, "");
            strcpy(data->iface->if_name, "ppp");
            data->iface->if_flags |= IFL_PPP;
            break;

         case 1:  /** slip **/
            strcpy(data->iface->if_sana2device, "DEVS:Networks/aslip.device");
            data->iface->if_sana2unit = 0;
            strcpy(data->iface->if_sana2config, "ENV:Sana2/aslip0.config");
            realloc_copy((STRPTR *)&data->iface->if_sana2configtext, "");
            strcpy(data->iface->if_name, "slip");
            data->iface->if_flags |= IFL_SLIP;
            break;

         default:
            strncpy(data->iface->if_sana2device, (STRPTR)xget(data->STR_Sana2Device, MUIA_String_Contents), sizeof(data->iface->if_sana2device));
            data->iface->if_sana2unit = xget(data->STR_Sana2Unit, MUIA_String_Integer);
            strncpy(data->iface->if_sana2config, (STRPTR)xget(data->STR_Sana2ConfigFile, MUIA_String_Contents), sizeof(data->iface->if_sana2config));
            realloc_copy((STRPTR *)&data->iface->if_sana2configtext, (STRPTR)xget(data->TI_Sana2ConfigText, MUIA_String_Contents));
            break;
      }
      if(value < 2 && data->iface->if_configparams)
      {
         FreeVec(data->iface->if_configparams);
         data->iface->if_configparams = NULL;
      }

      data->iface->if_keepalive = xget(data->SL_KeepAlive, MUIA_Numeric_Value);
      if(xget(data->CH_AlwaysOnline, MUIA_Selected))
         data->iface->if_flags |= IFL_AlwaysOnline;
      else
         data->iface->if_flags &= ~IFL_AlwaysOnline;

      if(ppp_if)
      {
         ppp_if->ppp_carrierdetect  = xget(data->CH_CarrierDetect     , MUIA_Selected);
         ppp_if->ppp_connecttimeout = xget(data->STR_ConnectTimeout   , MUIA_String_Integer);
         strcpy(ppp_if->ppp_callback, (STRPTR)xget(data->STR_Callback , MUIA_String_Contents));
         ppp_if->ppp_mppcomp        = xget(data->CH_MPPCompression    , MUIA_Selected);
         ppp_if->ppp_vjcomp         = xget(data->CH_VJCompression     , MUIA_Selected);
         ppp_if->ppp_bsdcomp        = xget(data->CH_BSDCompression    , MUIA_Selected);
         ppp_if->ppp_deflatecomp    = xget(data->CH_DeflateCompression, MUIA_Selected);
         ppp_if->ppp_eof            = xget(data->CY_EOF               , MUIA_Cycle_Active);
      }
   }
   return(NULL);
}

///
/// IfaceWindow_Init
ULONG IfaceWindow_Init(struct IClass *cl, Object *obj, struct MUIP_IfaceWindow_Init *msg)
{
   struct IfaceWindow_Data *data = INST_DATA(cl, obj);
   BOOL specify_sana2 = FALSE;

   if(data->iface = msg->iface)
   {
      struct PrefsPPPIface *ppp_if = data->iface->if_userdata;

      setstring(data->STR_IfaceName      , data->iface->if_name);
      if(*data->iface->if_addr && strcmp(data->iface->if_addr, "0.0.0.0"))
      {
         setstring(data->STR_Address , data->iface->if_addr);
         setcycle(data->CY_Address   , 1);
         set(data->STR_Address       , MUIA_Disabled, FALSE);
      }
      else
      {
         setstring(data->STR_Address, NULL);
         setcycle(data->CY_Address  , 0);
         set(data->STR_Address      , MUIA_Disabled, TRUE);
      }
      setstring(data->STR_Dest          , data->iface->if_dst);
      setstring(data->STR_Gateway       , data->iface->if_gateway);
      setstring(data->STR_Netmask       , data->iface->if_netmask);
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

      setslider(data->SL_KeepAlive       , data->iface->if_keepalive);
      setcheckmark(data->CH_AlwaysOnline , data->iface->if_flags & IFL_AlwaysOnline);

      setcheckmark(data->CH_CarrierDetect     , ppp_if->ppp_carrierdetect);
      set(data->STR_ConnectTimeout            , MUIA_String_Integer   , ppp_if->ppp_connecttimeout);
      setstring(data->STR_Callback            , ppp_if->ppp_callback);
      setcheckmark(data->CH_MPPCompression    , ppp_if->ppp_mppcomp);
      setcheckmark(data->CH_VJCompression     , ppp_if->ppp_vjcomp);
      setcheckmark(data->CH_BSDCompression    , ppp_if->ppp_bsdcomp);
      setcheckmark(data->CH_DeflateCompression, ppp_if->ppp_deflatecomp);
      setcycle(data->CY_EOF                   , ppp_if->ppp_eof);
   }
   set(data->PA_Sana2Device         , MUIA_Disabled, !specify_sana2);
   set(data->STR_Sana2Unit          , MUIA_Disabled, !specify_sana2);
   set(data->PA_Sana2ConfigFile     , MUIA_Disabled, !specify_sana2);
   set(data->TI_Sana2ConfigText     , MUIA_Disabled, !specify_sana2);

   set(data->CH_CarrierDetect       , MUIA_Disabled, !((data->iface->if_flags & IFL_PPP) || (data->iface->if_flags & IFL_SLIP)));
   set(data->CH_MPPCompression      , MUIA_Disabled, !(data->iface->if_flags & IFL_PPP));
   set(data->CH_VJCompression       , MUIA_Disabled, !(data->iface->if_flags & IFL_PPP));
   set(data->CH_BSDCompression      , MUIA_Disabled, !(data->iface->if_flags & IFL_PPP));
   set(data->CH_DeflateCompression  , MUIA_Disabled, !(data->iface->if_flags & IFL_PPP));
   set(data->CY_EOF                 , MUIA_Disabled, !(data->iface->if_flags & IFL_PPP));
   set(data->STR_ConnectTimeout     , MUIA_Disabled, !(data->iface->if_flags & IFL_PPP));
   set(data->STR_Callback           , MUIA_Disabled, !(data->iface->if_flags & IFL_PPP));

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
      "IPTYPE=2048 NOARP P2P IPREQ=8 WRITEREQ=8"     // ariadneliana0
      };

   if(msg->flags == MUIV_IfaceWindow_PopString_IfaceName)
   {
      if((pos = xget(data->LV_IfaceNames, MUIA_List_Active)) != MUIV_List_Active_Off)
      {
         DoMethod(data->LV_IfaceNames, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &ptr);
         if(ptr)
         {
            setstring(data->STR_IfaceName, ptr);
            setstring(data->STR_ConfigParams, config_strings[pos]);
         }
         DoMethod(data->PO_IfaceName, MUIM_Popstring_Close, TRUE);
      }
   }

   return(NULL);
}

///

/// IfaceWindow_New
ULONG IfaceWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct IfaceWindow_Data tmp;
   static STRPTR STR_CY_Ping[4];
   static STRPTR STR_CY_Dynamic[3];
   static STRPTR STR_CY_SanaDevice[4];
   static STRPTR STR_CY_EOF[3];
   static STRPTR ARR_IfacePages[] = { "Interface", "Sana II", "Misc", "PPP", NULL };
   static STRPTR STR_PO_Interfaces[] = { "ppp", "slip", "plip0", "magplip0", "ether0", "a2065", "a4066",
         "hydra", "wd80xx", "eb920", "quicknet","gg_3c503", "gg_ne1000", "gg_smc", "arcnet0", "a2060",
         "ax25", "liana0", "ariadne", "ariadneliana0", NULL };
   Object *originator;

   originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

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

   STR_CY_EOF[0] = GetStr("  auto");
   STR_CY_EOF[1] = GetStr("  off");
   STR_CY_EOF[2] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , "Edit Interface",
      MUIA_Window_ID       , MAKE_ID('I','F','A','E'),
      MUIA_Window_Height   , MUIV_Window_Height_MinMax(0),
      WindowContents       , VGroup,
         Child, RegisterGroup(ARR_IfacePages),
            Child, VGroup,
               Child, ColGroup(2),
                  Child, MakeKeyLabel2("  Name:", "  n"),
                  Child, tmp.PO_IfaceName = PopobjectObject,
                     MUIA_Popstring_String      , tmp.STR_IfaceName = MakeKeyString(NULL, 16, "  n"),
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
                  Child, MakeKeyLabel2(MSG_LA_Address, MSG_CC_Address),
                  Child, HGroup,
                     Child, tmp.STR_Address = TextinputObject,
                        StringFrame,
                        MUIA_ControlChar     , *GetStr(MSG_CC_Address),
                        MUIA_CycleChain      , 1,
                        MUIA_String_Accept   , "0123456789.",
                        MUIA_String_MaxLen   , 18,
                     End,
                     Child, tmp.CY_Address = Cycle(STR_CY_Dynamic),
                  End,
                  Child, MakeKeyLabel2("  Destination:", "  d"),
                  Child, tmp.STR_Dest = TextinputObject,
                     StringFrame,
                     MUIA_ControlChar     , *GetStr("  d"),
                     MUIA_CycleChain      , 1,
                     MUIA_String_Accept   , "0123456789.",
                     MUIA_String_MaxLen   , 18,
                  End,
                  Child, MakeKeyLabel2("  Gateway:", "  y"),
                  Child, tmp.STR_Gateway = TextinputObject,
                     StringFrame,
                     MUIA_ControlChar     , *GetStr("  y"),
                     MUIA_CycleChain      , 1,
                     MUIA_String_Accept   , "0123456789.",
                     MUIA_String_MaxLen   , 18,
                  End,
                  Child, MakeKeyLabel2("  Netmask:", "  e"),
                  Child, tmp.STR_Netmask = TextinputObject,
                     StringFrame,
                     MUIA_ControlChar     , *GetStr("  e"),
                     MUIA_CycleChain      , 1,
                     MUIA_String_Accept   , "0123456789.",
                     MUIA_String_MaxLen   , 18,
                  End,
                  Child, MakeKeyLabel2(MSG_LA_MTU, MSG_CC_MTU),
                  Child, HGroup,
                     Child, tmp.STR_MTU = TextinputObject,
                        StringFrame,
                        MUIA_Weight, 30,
                        MUIA_ControlChar     , *GetStr(MSG_CC_MTU),
                        MUIA_CycleChain      , 1,
                        MUIA_String_MaxLen   , 6,
                        MUIA_String_Accept   , "1234567890",
                     End,
                     Child, MakeKeyLabel2("  Params:", "  p"),
                     Child, tmp.STR_ConfigParams = MakeKeyString(NULL, 1024, "  p"),
                  End,
               End,
            End,
            Child, VGroup,
               Child, tmp.CY_Sana2Device = Cycle(STR_CY_SanaDevice),
               Child, HGroup,
                  Child, tmp.PA_Sana2Device = MakePopAsl(tmp.STR_Sana2Device = MakeKeyString(NULL, MAXPATHLEN, "  d"), "Choose sana II device", FALSE),
                  Child, MakeKeyLabel2("  Unit:", "  u"),
                  Child, tmp.STR_Sana2Unit = TextinputObject,
                     StringFrame,
                     MUIA_Weight          , 2,
                     MUIA_ControlChar     , *GetStr("  u"),
                     MUIA_CycleChain      , 1,
                     MUIA_String_MaxLen   , 5,
                     MUIA_String_Accept   , "1234567890",
                  End,
               End,
               Child, MUI_MakeObject(MUIO_BarTitle, "Config file"),
               Child, tmp.PA_Sana2ConfigFile = MakePopAsl(tmp.STR_Sana2ConfigFile = MakeKeyString(NULL, MAXPATHLEN, "  f"), "Choose name for sana II config file", FALSE),
               Child, tmp.TI_Sana2ConfigText = TextinputscrollObject,
                  MUIA_Weight, 5,
                  StringFrame,
                  MUIA_CycleChain, 1,
                  MUIA_ControlChar, GetStr("  o"),
                  MUIA_Textinput_Multiline, TRUE,
                  MUIA_Textinput_WordWrap, 512,
                  MUIA_Textinput_AutoExpand, TRUE,
               End,
            End,
            Child, VGroup,
               Child, ColGroup(2),
                  Child, Label("  Keep alive:"),
                  Child, tmp.SL_KeepAlive = MUI_MakeObject(MUIO_NumericButton, NULL, 0, 60, "Ping every %2ld min"),
                  Child, MakeKeyLabel2("  Is always online:", "  o"),
                  Child, HGroup,
                     Child, tmp.CH_AlwaysOnline = MakeKeyCheckMark(FALSE, "  o"),
                     Child, HVSpace,
                  End,
               End,
               Child, HVSpace,
            End,
            Child, VGroup, // ppp
               Child, HVSpace,
               Child, ColGroup(2),
                  Child, KeyLLabel1(GetStr(MSG_LA_CarrierDetect), *GetStr(MSG_CC_CarrierDetect)),
                  Child, tmp.CH_CarrierDetect = MakeKeyCheckMark(FALSE, MSG_CC_CarrierDetect),
                  Child, MakeKeyLabel2("  MPP Compression:", "  m"),
                  Child, tmp.CH_MPPCompression = MakeKeyCheckMark(FALSE, "  m"),
                  Child, MakeKeyLabel2("  VJ Compression:", "  v"),
                  Child, tmp.CH_VJCompression = MakeKeyCheckMark(FALSE, "  v"),
                  Child, MakeKeyLabel2("  BSD Compression:", "  b"),
                  Child, tmp.CH_BSDCompression = MakeKeyCheckMark(FALSE, "  b"),
                  Child, MakeKeyLabel2("  Deflate Compression:", "  d"),
                  Child, tmp.CH_DeflateCompression = MakeKeyCheckMark(FALSE, "  d"),
                  Child, MakeKeyLabel2("  EOF Mode:", "  e"),
                  Child, tmp.CY_EOF = KeyCycle(STR_CY_EOF, GetStr("  e")),
                  Child, MakeKeyLabel2("  Connect timeout:", "  t"),
                  Child, tmp.STR_ConnectTimeout = MakeKeyString(NULL, 16, "  t"),
                  Child, MakeKeyLabel2("  Callback:", "  a"),
                  Child, tmp.STR_Callback = MakeKeyString(NULL, 80, "  a"),
               End,
               Child, HVSpace,
            End,
         End,
         Child, HGroup,
            Child, tmp.BT_Okay   = MakeButton("  _Okay"),
            Child, tmp.BT_Cancel = MakeButton("  _Cancel"),
         End,
      End,
      TAG_MORE, msg->ops_AttrList))
   {
      struct IfaceWindow_Data *data = INST_DATA(cl, obj);

      *data = tmp;

      data->iface = NULL;

      set(data->CY_Address, MUIA_Weight, 0);

      DoMethod(obj                 , MUIM_Notify, MUIA_Window_CloseRequest, TRUE            , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_ProviderWindow_EditIfaceFinish, obj, 0);
      DoMethod(data->BT_Cancel     , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_ProviderWindow_EditIfaceFinish, obj, 0);
      DoMethod(data->BT_Okay       , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_ProviderWindow_EditIfaceFinish, obj, 1);

      DoMethod(data->LV_IfaceNames , MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime , obj, 2, MUIM_IfaceWindow_PopString_Close, MUIV_IfaceWindow_PopString_IfaceName);
   }
   return((ULONG)obj);
}

///
/// IfaceWindow_Dispatcher
SAVEDS ULONG IfaceWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                            : return(IfaceWindow_New            (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_Init             : return(IfaceWindow_Init           (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_CopyData         : return(IfaceWindow_CopyData       (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_PopString_Close  : return(IfaceWindow_PopString_Close(cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///
