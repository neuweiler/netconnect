/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Dialer.h"
#include "protos.h"

///
/// external variables
extern struct Hook des_hook;
extern struct Hook strobjhook;
extern struct Hook objstrhook;
extern Object *win;
extern Object *app;

///

/// Dialer_New
ULONG Dialer_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Dialer_Data tmp;
   static STRPTR ARR_Dialer_Register[4];
   static STRPTR ARR_MainWindow[4];

   ARR_Dialer_Register[0] = GetStr(MSG_DialerRegister1);
   ARR_Dialer_Register[1] = GetStr(MSG_DialerRegister2);
   ARR_Dialer_Register[2] = GetStr(MSG_DialerRegister3);
   ARR_Dialer_Register[3] = NULL;

   ARR_MainWindow[0] = GetStr(MSG_TX_OpenStartup);
   ARR_MainWindow[1] = GetStr(MSG_TX_IconifyStartup);
   ARR_MainWindow[2] = GetStr(MSG_TX_ClosedStartup);
   ARR_MainWindow[3] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Register_Titles, ARR_Dialer_Register,
      MUIA_CycleChain, 1,
      Child, VGroup,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_MainWindowDisplayOptions)),
         Child, ColGroup(5),
            Child, HVSpace,
            Child, ColGroup(2),
               Child, tmp.CH_ShowOnlineTime  = MakeKeyCheckMark(TRUE, MSG_CC_ShowOnlineTime),
               Child, KeyLLabel1(GetStr(MSG_LA_ShowOnlineTime), *GetStr(MSG_CC_ShowOnlineTime)),
               Child, tmp.CH_ShowConnect     = MakeKeyCheckMark(TRUE, MSG_CC_ShowSpeed),
               Child, KeyLLabel1(GetStr(MSG_LA_ShowSpeed), *GetStr(MSG_CC_ShowSpeed)),
               Child, tmp.CH_ShowNetwork     = MakeKeyCheckMark(TRUE, MSG_CC_ShowProviders),
               Child, KeyLLabel1(GetStr(MSG_LA_ShowProviders), *GetStr(MSG_CC_ShowProviders)),
               Child, tmp.CH_ShowButtons     = MakeKeyCheckMark(TRUE, MSG_CC_ShowButtons),
               Child, KeyLLabel1(GetStr(MSG_LA_ShowButtons), *GetStr(MSG_CC_ShowButtons)),
            End,
            Child, HVSpace,
            Child, VGroup,
               Child, ColGroup(2),
                  Child, tmp.CH_ShowLamps       = MakeKeyCheckMark(TRUE, MSG_CC_ShowLeds),
                  Child, KeyLLabel1(GetStr(MSG_LA_ShowLeds), *GetStr(MSG_CC_ShowLeds)),
                  Child, tmp.CH_ShowLog         = MakeKeyCheckMark(TRUE, MSG_CC_ShowLog),
                  Child, KeyLLabel1(GetStr(MSG_LA_ShowLog), *GetStr(MSG_CC_ShowLog)),
                  Child, tmp.CH_ShowUser        = MakeKeyCheckMark(TRUE, MSG_CC_ShowUsers),
                  Child, KeyLLabel1(GetStr(MSG_LA_ShowUsers), *GetStr(MSG_CC_ShowUsers)),
               End,
               Child, tmp.CY_MainWindow = Cycle(ARR_MainWindow),
            End,
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_StatusWindows)),
         Child, ColGroup(7),
            Child, HVSpace,
            Child, tmp.CH_ShowStatusWin   = MakeKeyCheckMark(TRUE, MSG_CC_StatusWindows),
            Child, KeyLLabel1(GetStr(MSG_LA_StatusWindows), *GetStr(MSG_CC_StatusWindows)),
            Child, HVSpace,
            Child, tmp.CH_ShowSerialInput = MakeKeyCheckMark(TRUE, MSG_CC_ShowSerialInput),
            Child, KeyLLabel1(GetStr(MSG_LA_ShowSerialInput), *GetStr(MSG_CC_ShowSerialInput)),
            Child, HVSpace,
         End,
         Child, HVSpace,
      End,

      Child, VGroup,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_Control)),
         Child, ColGroup(7),
            Child, HVSpace,
            Child, tmp.CH_QuickReconnect  = MakeKeyCheckMark(FALSE, MSG_CC_QuickReconnect),
            Child, KeyLLabel1(GetStr(MSG_LA_QuickReconnect), *GetStr(MSG_CC_QuickReconnect)),
            Child, HVSpace,
            Child, tmp.CH_ConfirmOffline  = MakeKeyCheckMark(FALSE, MSG_CC_ConfirmDisconnect),
            Child, KeyLLabel1(GetStr(MSG_LA_ConfirmDisconnect), *GetStr(MSG_CC_ConfirmDisconnect)),
            Child, HVSpace,
            Child, HVSpace,
            Child, tmp.CH_Debug           = MakeKeyCheckMark(FALSE, MSG_CC_DebugMode),
            Child, KeyLLabel1(GetStr(MSG_LA_DebugMode), *GetStr(MSG_CC_DebugMode)),
            Child, HVSpace,
            Child, VVSpace,
            Child, VVSpace,
            Child, HVSpace,
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_Startup)),
         Child, ColGroup(7),
            Child, HVSpace,
            Child, tmp.CH_StartupInetd  = MakeKeyCheckMark(TRUE, MSG_CC_StartupInetd),
            Child, KeyLLabel1(GetStr(MSG_LA_StartupInetd), *GetStr(MSG_CC_StartupInetd)),
            Child, HVSpace,
            Child, tmp.CH_StartupLoopback  = MakeKeyCheckMark(TRUE, MSG_CC_StartupLoopback),
            Child, KeyLLabel1(GetStr(MSG_LA_StartupLoopback), *GetStr(MSG_CC_StartupLoopback)),
            Child, HVSpace,
            Child, HVSpace,
            Child, tmp.CH_StartupTCP  = MakeKeyCheckMark(TRUE, MSG_CC_StartupTCP),
            Child, KeyLLabel1(GetStr(MSG_LA_StartupTCP), *GetStr(MSG_CC_StartupTCP)),
            Child, HVSpace,
            Child, HVSpace,
            Child, HVSpace,
            Child, HVSpace,
         End,
         Child, HVSpace,
      End,

      Child, VGroup,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_Startup)),
         Child, HGroup,
            Child, tmp.PA_Startup = MakePopAsl(tmp.STR_Startup = MakeKeyString(NULL, MAXPATHLEN, "  r"), MSG_TX_ChooseFile, FALSE),
            Child, tmp.CY_Startup = Cycle(exec_types),
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_Shutdown)),
         Child, HGroup,
            Child, tmp.PA_Shutdown = MakePopAsl(tmp.STR_Shutdown = MakeKeyString(NULL, MAXPATHLEN, "  d"), MSG_TX_ChooseFile, FALSE),
            Child, tmp.CY_Shutdown = Cycle(exec_types),
         End,
         Child, HVSpace,
      End,
   TAG_MORE, msg->ops_AttrList))
   {
      struct Dialer_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      set(data->CY_MainWindow , MUIA_CycleChain, 1);
      set(data->CY_Startup    , MUIA_CycleChain, 1);
      set(data->CY_Shutdown   , MUIA_CycleChain, 1);

      set(data->CY_Startup , MUIA_Weight, 0);
      set(data->CY_Shutdown, MUIA_Weight, 0);
set(data->CH_ConfirmOffline, MUIA_Disabled, TRUE);
set(data->CH_QuickReconnect, MUIA_Disabled, TRUE);

      set(data->CH_ShowOnlineTime   , MUIA_ShortHelp, GetStr(MSG_Help_ShowOnlineTime));
      set(data->CH_ShowConnect      , MUIA_ShortHelp, GetStr(MSG_Help_ShowSpeed));
      set(data->CH_ShowNetwork      , MUIA_ShortHelp, GetStr(MSG_Help_ShowProviders));
      set(data->CH_ShowButtons      , MUIA_ShortHelp, GetStr(MSG_Help_ShowButtons));
      set(data->CH_ShowLamps        , MUIA_ShortHelp, GetStr(MSG_Help_ShowLeds));
      set(data->CH_ShowLog          , MUIA_ShortHelp, GetStr(MSG_Help_ShowLog));
      set(data->CH_ShowUser         , MUIA_ShortHelp, GetStr(MSG_Help_ShowUsers));
      set(data->CY_MainWindow       , MUIA_ShortHelp, GetStr(MSG_Help_MainWindow));
      set(data->CH_ShowStatusWin    , MUIA_ShortHelp, GetStr(MSG_Help_StatusWindows));
      set(data->CH_ShowSerialInput  , MUIA_ShortHelp, GetStr(MSG_Help_ShowSerialInput));
      set(data->CH_QuickReconnect   , MUIA_ShortHelp, GetStr(MSG_Help_QuickReconnect));
      set(data->CH_ConfirmOffline   , MUIA_ShortHelp, GetStr(MSG_Help_ConfirmOffline));
      set(data->CH_Debug            , MUIA_ShortHelp, GetStr(MSG_Help_Debug));
      set(data->CH_StartupInetd     , MUIA_ShortHelp, GetStr(MSG_Help_StartupInetd));
      set(data->CH_StartupLoopback  , MUIA_ShortHelp, GetStr(MSG_Help_StartupLoopback));
      set(data->CH_StartupTCP       , MUIA_ShortHelp, GetStr(MSG_Help_StartupTCP));
      set(data->STR_Startup         , MUIA_ShortHelp, GetStr(MSG_Help_Startup));
      set(data->CY_Startup          , MUIA_ShortHelp, GetStr(MSG_Help_ExecType));
      set(data->STR_Shutdown        , MUIA_ShortHelp, GetStr(MSG_Help_Shutdown));
      set(data->CY_Shutdown         , MUIA_ShortHelp, GetStr(MSG_Help_ExecType));
   }
   return((ULONG)obj);
}

///
/// Dialer_Dispatcher
SAVEDS ASM ULONG Dialer_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(Dialer_New               (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///

