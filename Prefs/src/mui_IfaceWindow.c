/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include "Strings.h"
#include "mui.h"
#include "mui_IfaceWindow.h"
#include "mui_ProviderWindow.h"
#include "protos.h"

///
/// external variables
extern struct Hook strobjhook, des_hook;
extern struct Library *GenesisBase;
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
   ULONG value;

   if(data->iface)
   {
      struct PrefsPPPIface *ppp_if = data->iface->if_userdata;

      strncpy(data->iface->if_name, (STRPTR)xget(data->STR_IfaceName, MUIA_String_Contents), sizeof(data->iface->if_name));
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
            strcpy(data->iface->if_name, "ppp");
            data->iface->if_flags |= IFL_PPP;
            break;

         case 1:  /** slip **/
            strcpy(data->iface->if_sana2device, "DEVS:Networks/aslip.device");
            data->iface->if_sana2unit = 0;
            strcpy(data->iface->if_sana2config, "ENV:Sana2/aslip0.config");
            ReallocCopy((STRPTR *)&data->iface->if_sana2configtext, "");
            strcpy(data->iface->if_name, "slip");
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

      data->iface->if_keepalive = xget(data->SL_KeepAlive, MUIA_Numeric_Value);
      if(xget(data->CH_AlwaysOnline, MUIA_Selected))
         data->iface->if_flags |= IFL_AlwaysOnline;
      else
         data->iface->if_flags &= ~IFL_AlwaysOnline;

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

      setstring(data->STR_IfaceName     , data->iface->if_name);
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

      setslider(data->SL_KeepAlive       , data->iface->if_keepalive);
      setcheckmark(data->CH_AlwaysOnline , data->iface->if_flags & IFL_AlwaysOnline);

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
      setcycle(data->CY_EOF                   , ppp_if->ppp_eof);
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
      "IPTYPE=2048 NOARP P2P IPREQ=8 WRITEREQ=8"     // ariadne
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
      data->CH_DeflateCompression, data->CY_EOF, data->CH_Callback, NULL);

   DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, ((value != 0) || (!xget(data->CH_Callback, MUIA_Selected))),
      data->SL_ConnectTimeout, data->STR_Callback, NULL);

   set(data->CH_CarrierDetect, MUIA_Disabled, value > 1);

   return(NULL);
}

///

/// IfaceWindow_New
ULONG IfaceWindow_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct IfaceWindow_Data tmp;
   static STRPTR STR_CY_Dynamic[3];
   static STRPTR STR_CY_Netmask[3];
   static STRPTR STR_CY_SanaDevice[4];
   static STRPTR STR_CY_EOF[3];
   static STRPTR ARR_IfacePages[5];
   static STRPTR STR_PO_Interfaces[] = { "ppp", "slip", "plip0", "magplip0", "ether0", "a2065", "a4066",
         "hydra", "wd80xx", "eb920", "quicknet","gg_3c503", "gg_ne1000", "gg_smc", "arcnet0", "a2060",
         "ax25", "liana0", "ariadne", "ariadneliana0", NULL };
   Object *originator;

   originator = (Object *)GetTagData(MUIA_Genesis_Originator, 0, msg->ops_AttrList);

   ARR_IfacePages[0] = GetStr(MSG_TX_IfaceWindowRegister1);
   ARR_IfacePages[1] = GetStr(MSG_TX_IfaceWindowRegister2);
   ARR_IfacePages[2] = GetStr(MSG_TX_IfaceWindowRegister3);
   ARR_IfacePages[3] = GetStr(MSG_TX_IfaceWindowRegister4);
   ARR_IfacePages[4] = NULL;

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

   STR_CY_EOF[0] = GetStr(MSG_CY_EOFMode1);
   STR_CY_EOF[1] = GetStr(MSG_CY_EOFMode2);
   STR_CY_EOF[2] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Window_Title    , GetStr(MSG_TX_EditInterface),
      MUIA_Window_ID       , MAKE_ID('I','F','A','E'),
      MUIA_Window_Height   , MUIV_Window_Height_MinMax(0),
      WindowContents       , VGroup,
         Child, RegisterGroup(ARR_IfacePages),
            MUIA_CycleChain, 1,
            Child, VGroup,
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_Name, MSG_CC_Name),
                  Child, tmp.PO_IfaceName = PopobjectObject,
                     MUIA_Popstring_String      , tmp.STR_IfaceName = MakeKeyString(NULL, 16, MSG_CC_Name),
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
                     Child, tmp.STR_MTU = TextinputObject,
                        StringFrame,
                        MUIA_Weight, 30,
                        MUIA_ControlChar     , *GetStr(MSG_CC_MTU),
                        MUIA_CycleChain      , 1,
                        MUIA_String_MaxLen   , 6,
                        MUIA_String_Accept   , "1234567890",
                     End,
                     Child, MakeKeyLabel2(MSG_LA_IfaceParams, MSG_CC_IfaceParams),
                     Child, tmp.STR_ConfigParams = MakeKeyString(NULL, 1024, MSG_CC_IfaceParams),
                  End,
               End,
            End,
            Child, VGroup,
               Child, tmp.CY_Sana2Device = Cycle(STR_CY_SanaDevice),
               Child, HGroup,
                  Child, tmp.PA_Sana2Device = MakePopAsl(tmp.STR_Sana2Device = MakeKeyString(NULL, MAXPATHLEN, "   "), GetStr(MSG_TX_ChooseSanaDevice), FALSE),
                  Child, MakeKeyLabel2(MSG_LA_Unit, MSG_CC_Unit),
                  Child, tmp.STR_Sana2Unit = TextinputObject,
                     StringFrame,
                     MUIA_Weight          , 2,
                     MUIA_ControlChar     , *GetStr(MSG_CC_Unit),
                     MUIA_CycleChain      , 1,
                     MUIA_String_MaxLen   , 5,
                     MUIA_String_Accept   , "1234567890",
                  End,
               End,
               Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_ConfigFile)),
               Child, tmp.PA_Sana2ConfigFile = MakePopAsl(tmp.STR_Sana2ConfigFile = MakeKeyString(NULL, MAXPATHLEN, "  f"), GetStr(MSG_TX_ChooseSanaConfigFile), FALSE),
               Child, tmp.TI_Sana2ConfigText = TextinputscrollObject,
                  MUIA_Weight, 5,
                  StringFrame,
                  MUIA_CycleChain, 1,
                  MUIA_Textinput_Multiline, TRUE,
                  MUIA_Textinput_WordWrap, 512,
                  MUIA_Textinput_AutoExpand, TRUE,
               End,
            End,
            Child, VGroup,
               Child, HVSpace,
               Child, ColGroup(2),
                  Child, Label(GetStr(MSG_LA_KeepAlive)),
                  Child, HGroup,
                     Child, Label(GetStr(MSG_TX_PingEvery)),
                     Child, tmp.SL_KeepAlive = MUI_MakeObject(MUIO_NumericButton, NULL, 0, 60, "%2ld"),
                     Child, Label(GetStr(MSG_TX_Minutes)),
                  End,
                  Child, MakeKeyLabel2(MSG_LA_AlwaysOnline, MSG_CC_AlwaysOnline),
                  Child, HGroup,
                     Child, tmp.CH_AlwaysOnline = MakeKeyCheckMark(FALSE, MSG_CC_AlwaysOnline),
                     Child, HVSpace,
                  End,
               End,
               Child, HVSpace,
            End,
            Child, VGroup, // ppp
               Child, HVSpace,
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
               End,
               Child, HVSpace,
               Child, ColGroup(2),
                  Child, MakeKeyLabel2(MSG_LA_Callback, MSG_CC_Callback),
                  Child, HGroup,
                     GroupSpacing(1),
                     Child, tmp.CH_Callback = MakeKeyCheckMark(FALSE, MSG_CC_Callback),
                     Child, tmp.STR_Callback = MakeKeyString(NULL, 80, MSG_CC_Callback),
                  End,
                  Child, MakeKeyLabel2(MSG_LA_Timeout, MSG_CC_Timeout),
                  Child, HGroup,
                     Child, tmp.SL_ConnectTimeout = MUI_MakeObject(MUIO_NumericButton, NULL, 5, 360, GetStr(MSG_TX_TimeoutSlider)),
                     Child, HVSpace,
                  End,
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

      set(data->SL_ConnectTimeout, MUIA_ControlChar, GetStr(MSG_CC_Timeout));

      set(data->CY_Address, MUIA_Weight, 0);
      set(data->CY_Dest   , MUIA_Weight, 0);
      set(data->CY_Gateway, MUIA_Weight, 0);
      set(data->CY_Netmask, MUIA_Weight, 0);

      set(data->CY_Address       , MUIA_CycleChain, 1);
      set(data->CY_Dest          , MUIA_CycleChain, 1);
      set(data->CY_Gateway       , MUIA_CycleChain, 1);
      set(data->CY_Netmask       , MUIA_CycleChain, 1);
      set(data->CY_Sana2Device   , MUIA_CycleChain, 1);
      set(data->SL_KeepAlive     , MUIA_CycleChain, 1);
      set(data->SL_ConnectTimeout, MUIA_CycleChain, 1);

      set(data->STR_Address        , MUIA_Disabled, TRUE);
      set(data->STR_Dest           , MUIA_Disabled, TRUE);
      set(data->STR_Gateway        , MUIA_Disabled, TRUE);
      set(data->STR_Netmask        , MUIA_Disabled, TRUE);
      set(data->PA_Sana2Device    , MUIA_Disabled, TRUE);
      set(data->STR_Sana2Unit     , MUIA_Disabled, TRUE);
      set(data->PA_Sana2ConfigFile, MUIA_Disabled, TRUE);
      set(data->TI_Sana2ConfigText, MUIA_Disabled, TRUE);
      set(data->STR_Callback      , MUIA_Disabled, TRUE);
      set(data->SL_ConnectTimeout , MUIA_Disabled, TRUE);

      DoMethod(obj                 , MUIM_Notify, MUIA_Window_CloseRequest, TRUE            , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_ProviderWindow_EditIfaceFinish, obj, 0);
      DoMethod(data->BT_Cancel     , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_ProviderWindow_EditIfaceFinish, obj, 0);
      DoMethod(data->BT_Okay       , MUIM_Notify, MUIA_Pressed            , FALSE           , MUIV_Notify_Application, 6, MUIM_Application_PushMethod, originator, 3, MUIM_ProviderWindow_EditIfaceFinish, obj, 1);

      DoMethod(data->CH_Callback   , MUIM_Notify, MUIA_Selected           , MUIV_EveryTime  , obj, 6, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, data->STR_Callback, data->SL_ConnectTimeout, NULL);

      DoMethod(data->LV_IfaceNames , MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime , obj, 2, MUIM_IfaceWindow_PopString_Close, MUIV_IfaceWindow_PopString_IfaceName);
      DoMethod(data->CY_Address    , MUIM_Notify, MUIA_Cycle_Active        , MUIV_EveryTime , data->STR_Address , 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->CY_Dest       , MUIM_Notify, MUIA_Cycle_Active        , MUIV_EveryTime , data->STR_Dest    , 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->CY_Gateway    , MUIM_Notify, MUIA_Cycle_Active        , MUIV_EveryTime , data->STR_Gateway , 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->CY_Netmask    , MUIM_Notify, MUIA_Cycle_Active        , MUIV_EveryTime , data->STR_Netmask , 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->CY_Sana2Device, MUIM_Notify, MUIA_Cycle_Active        , MUIV_EveryTime , obj, 1, MUIM_IfaceWindow_SanaActive);
   }
   return((ULONG)obj);
}

///
/// IfaceWindow_Dispatcher
SAVEDS ASM ULONG IfaceWindow_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   switch((ULONG)msg->MethodID)
   {
      case OM_NEW                            : return(IfaceWindow_New            (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_Init             : return(IfaceWindow_Init           (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_CopyData         : return(IfaceWindow_CopyData       (cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_PopString_Close  : return(IfaceWindow_PopString_Close(cl, obj, (APTR)msg));
      case MUIM_IfaceWindow_SanaActive       : return(IfaceWindow_SanaActive     (cl, obj, (APTR)msg));
   }

   return(DoSuperMethodA(cl, obj, msg));
}

///
