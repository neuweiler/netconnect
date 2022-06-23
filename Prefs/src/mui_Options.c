/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_Options.h"
#include "protos.h"

///
/// external variables
extern struct Hook des_hook;
extern struct Hook strobjhook;
extern struct Hook objstrhook;
extern Object *win;
extern Object *app;
extern struct MUI_CustomClass *CL_LogLevel;

///

/// Options_New
ULONG Options_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct Options_Data tmp;
   static STRPTR ARR_Options_Register[5];
   static STRPTR ARR_MainWindow[4];

   ARR_Options_Register[0] = GetStr(MSG_OptionsRegister1);
   ARR_Options_Register[1] = GetStr(MSG_OptionsRegister2);
   ARR_Options_Register[2] = GetStr(MSG_OptionsRegister3);
   ARR_Options_Register[3] = GetStr(MSG_OptionsRegister4);
   ARR_Options_Register[4] = NULL;

   ARR_MainWindow[0] = GetStr(MSG_CY_OpenStartup);
   ARR_MainWindow[1] = GetStr(MSG_CY_IconifyStartup);
   ARR_MainWindow[2] = GetStr(MSG_CY_ClosedStartup);
   ARR_MainWindow[3] = NULL;

   if(obj = (Object *)DoSuperNew(cl, obj,
      MUIA_Register_Titles, ARR_Options_Register,
      MUIA_CycleChain, 1,
      Child, VGroup,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_MainWindowDisplayOptionsTitle)),
         Child, ColGroup(5),
            Child, HVSpace,
            Child, ColGroup(2),
               Child, tmp.CH_ShowOnlineTime  = MakeKeyCheckMark(TRUE, MSG_CC_ShowOnlineTime),
               Child, KeyLLabel1(GetStr(MSG_LA_ShowOnlineTime), *GetStr(MSG_CC_ShowOnlineTime)),
               Child, tmp.CH_ShowConnect     = MakeKeyCheckMark(TRUE, MSG_CC_ShowSpeed),
               Child, KeyLLabel1(GetStr(MSG_LA_ShowSpeed), *GetStr(MSG_CC_ShowSpeed)),
               Child, tmp.CH_ShowIface       = MakeKeyCheckMark(TRUE, MSG_CC_ShowIfaces),
               Child, KeyLLabel1(GetStr(MSG_LA_ShowIfaces), *GetStr(MSG_CC_ShowIfaces)),
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
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_StatusWindowsTitle)),
         Child, ColGroup(7),
            Child, HVSpace,
            Child, tmp.CH_ShowStatusWin   = MakeKeyCheckMark(TRUE, MSG_CC_StatusWindows),
            Child, KeyLLabel1(GetStr(MSG_LA_StatusWindows), *GetStr(MSG_CC_StatusWindows)),
            Child, HVSpace,
            Child, tmp.CH_ShowSerialInput = MakeKeyCheckMark(TRUE, MSG_CC_ShowSerialInput),
            Child, KeyLLabel1(GetStr(MSG_LA_ShowSerialInput), *GetStr(MSG_CC_ShowSerialInput)),
            Child, HVSpace,
            Child, HVSpace,
            Child, tmp.CH_StartupNetInfo     = MakeKeyCheckMark(TRUE, MSG_CC_StartupNetInfo),
            Child, KeyLLabel1(GetStr(MSG_LA_StartupNetInfo), *GetStr(MSG_CC_StartupNetInfo)),
            Child, HVSpace,
            Child, VVSpace,
            Child, HVSpace,
            Child, HVSpace,
         End,
         Child, HVSpace,
      End,

      Child, VGroup, // misc
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_ControlTitle)),
         Child, ColGroup(7),
            Child, HVSpace,
            Child, tmp.CH_ConfirmOffline  = MakeKeyCheckMark(FALSE, MSG_CC_ConfirmDisconnect),
            Child, KeyLLabel1(GetStr(MSG_LA_ConfirmDisconnect), *GetStr(MSG_CC_ConfirmDisconnect)),
            Child, HVSpace,
            Child, tmp.CH_Debug           = MakeKeyCheckMark(FALSE, MSG_CC_DebugMode),
            Child, KeyLLabel1(GetStr(MSG_LA_DebugMode), *GetStr(MSG_CC_DebugMode)),
            Child, HVSpace,
            Child, HVSpace,
            Child, tmp.CH_FlushUserOnExit  = MakeKeyCheckMark(FALSE, MSG_CC_FlushUserOnExit),
            Child, KeyLLabel1(GetStr(MSG_LA_FlushUserOnExit), *GetStr(MSG_CC_FlushUserOnExit)),
            Child, HVSpace,
            Child, tmp.CH_NoAutoTraffic    = MakeKeyCheckMark(FALSE, MSG_CC_NoAutoTraffic),
            Child, KeyLLabel1(GetStr(MSG_LA_NoAutoTraffic), *GetStr(MSG_CC_NoAutoTraffic)),
            Child, HVSpace,
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_LogSettingsTitle)),
         Child, ColGroup(2),
            Child, MakeKeyLabel2(MSG_LA_LogLevel, MSG_CC_LogLevel),
            Child, HGroup,
               Child, tmp.SL_LogLevel = NewObject(CL_LogLevel->mcc_Class, 0,
                  MUIA_Numeric_Min  , 0,
                  MUIA_Numeric_Max  , 79,
                  MUIA_CycleChain   , 1,
                  MUIA_ControlChar  , *GetStr(MSG_CC_LogLevel),
               TAG_DONE),
               Child, HVSpace,
            End,
            Child, MakeKeyLabel2(MSG_LA_LogFileLevel, MSG_CC_LogFileLevel),
            Child, HGroup,
               Child, tmp.SL_LogFileLevel = NewObject(CL_LogLevel->mcc_Class, 0,
                  MUIA_Numeric_Min  , 0,
                  MUIA_Numeric_Max  , 79,
                  MUIA_CycleChain   , 1,
                  MUIA_ControlChar  , *GetStr(MSG_CC_LogFileLevel),
               TAG_DONE),
               Child, HVSpace,
            End,
            Child, MakeKeyLabel2(MSG_LA_LogFile, MSG_CC_LogFile),
            Child, tmp.PA_LogFile = MakePopAsl(tmp.STR_LogFile = MakeKeyString(MAXPATHLEN, MSG_CC_LogFile), MSG_TX_ChooseFile, FALSE),
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_StartupTitle)),
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

      Child, VGroup, // exec
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_StartupTitle)),
         Child, HGroup,
            Child, tmp.PA_Startup = MakePopAsl(tmp.STR_Startup = MakeKeyString(MAXPATHLEN, "  r"), MSG_TX_ChooseFile, FALSE),
            Child, tmp.CY_Startup = Cycle(exec_types),
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_ShutdownTitle)),
         Child, HGroup,
            Child, tmp.PA_Shutdown = MakePopAsl(tmp.STR_Shutdown = MakeKeyString(MAXPATHLEN, "  d"), MSG_TX_ChooseFile, FALSE),
            Child, tmp.CY_Shutdown = Cycle(exec_types),
         End,
         Child, HVSpace,
      End,

      Child, VGroup, // advanced
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_KernelOptionsTitle)),
         Child, ColGroup(7),
            Child, HVSpace,
            Child, MakeKeyLabel2(MSG_LA_BeGateway, MSG_CC_BeGateway),
            Child, tmp.CH_BeGateway = MakeKeyCheckMark(FALSE, MSG_CC_BeGateway),
            Child, HVSpace,
            Child, MakeKeyLabel2(MSG_LA_DebugSana, MSG_CC_DebugSana),
            Child, tmp.CH_DebugSana = MakeKeyCheckMark(FALSE, MSG_CC_DebugSana),
            Child, HVSpace,
            Child, HVSpace,
            Child, MakeKeyLabel2(MSG_LA_IPSendRedirects, MSG_CC_IPSendRedirects),
            Child, tmp.CH_IPSendRedirects = MakeKeyCheckMark(FALSE, MSG_CC_IPSendRedirects),
            Child, HVSpace,
            Child, MakeKeyLabel2(MSG_LA_DebugICMP, MSG_CC_DebugICMP),
            Child, tmp.CH_DebugICMP = MakeKeyCheckMark(FALSE, MSG_CC_DebugICMP),
            Child, HVSpace,
            Child, HVSpace,
            Child, MakeKeyLabel2(MSG_LA_KernelPriority, MSG_CC_KernelPriority),
            Child, HGroup,
               Child, tmp.SL_KernelPriority = MUI_MakeObject(MUIO_NumericButton, NULL, -9, 10, "%ld"),
               Child, HVSpace,
            End,
            Child, HVSpace,
            Child, MakeKeyLabel2(MSG_LA_DebugIP, MSG_CC_DebugIP),
            Child, tmp.CH_DebugIP = MakeKeyCheckMark(FALSE, MSG_CC_DebugIP),
            Child, HVSpace,
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_MBufTitle)),
         Child, ColGroup(4),
            Child, MakeKeyLabel2(MSG_LA_MBufInitial, MSG_CC_MBufInitial),
            Child, tmp.STR_MBufInitial = MakeKeyInteger(6, MSG_CC_MBufInitial),
            Child, MakeKeyLabel2(MSG_LA_MBufChunk, MSG_CC_MBufChunk),
            Child, tmp.STR_MBufChunk = MakeKeyInteger(6, MSG_CC_MBufChunk),
            Child, MakeKeyLabel2(MSG_LA_MBufClChunk, MSG_CC_MBufClChunk),
            Child, tmp.STR_MBufClChunk = MakeKeyInteger(6, MSG_CC_MBufClChunk),
            Child, MakeKeyLabel2(MSG_LA_MBufClusterSize, MSG_CC_MBufClusterSize),
            Child, tmp.STR_MBufClusterSize = MakeKeyInteger(6, MSG_CC_MBufClusterSize),
            Child, MakeKeyLabel2(MSG_LA_MBufMaxMem, MSG_CC_MBufMaxMem),
            Child, tmp.STR_MBufMaxMem = MakeKeyInteger(6, MSG_CC_MBufMaxMem),
            Child, VVSpace,
            Child, HVSpace,
         End,
         Child, HVSpace,
         Child, MUI_MakeObject(MUIO_BarTitle, GetStr(MSG_TX_TCPSpaceTitle)),
         Child, ColGroup(4),
            Child, MakeKeyLabel2(MSG_LA_TCPSendSpace, MSG_CC_TCPSendSpace),
            Child, tmp.STR_TCPSendSpace = MakeKeyInteger(6, MSG_CC_TCPSendSpace),
            Child, MakeKeyLabel2(MSG_LA_TCPRecvSpace, MSG_CC_TCPSendSpace),
            Child, tmp.STR_TCPRecvSpace = MakeKeyInteger(6, MSG_CC_TCPRecvSpace),
         End,
         Child, HVSpace,
      End,
   TAG_MORE, msg->ops_AttrList))
   {
      struct Options_Data *data = INST_DATA(cl,obj);

      *data = tmp;

      set(data->CY_MainWindow , MUIA_CycleChain, 1);
      set(data->CY_Startup    , MUIA_CycleChain, 1);
      set(data->CY_Shutdown   , MUIA_CycleChain, 1);

      set(data->CY_Startup , MUIA_Weight, 0);
      set(data->CY_Shutdown, MUIA_Weight, 0);
set(data->CH_DebugICMP     , MUIA_Disabled, TRUE);

      set(data->CH_ShowOnlineTime   , MUIA_ShortHelp, GetStr(MSG_Help_ShowOnlineTime));
      set(data->CH_ShowConnect      , MUIA_ShortHelp, GetStr(MSG_Help_ShowSpeed));
      set(data->CH_ShowIface        , MUIA_ShortHelp, GetStr(MSG_Help_ShowIfaces));
      set(data->CH_ShowButtons      , MUIA_ShortHelp, GetStr(MSG_Help_ShowButtons));
      set(data->CH_ShowLamps        , MUIA_ShortHelp, GetStr(MSG_Help_ShowLeds));
      set(data->CH_ShowLog          , MUIA_ShortHelp, GetStr(MSG_Help_ShowLog));
      set(data->CH_ShowUser         , MUIA_ShortHelp, GetStr(MSG_Help_ShowUsers));
      set(data->CY_MainWindow       , MUIA_ShortHelp, GetStr(MSG_Help_MainWindow));
      set(data->CH_ShowStatusWin    , MUIA_ShortHelp, GetStr(MSG_Help_StatusWindows));
      set(data->CH_ShowSerialInput  , MUIA_ShortHelp, GetStr(MSG_Help_ShowSerialInput));
      set(data->CH_StartupNetInfo   , MUIA_ShortHelp, GetStr(MSG_Help_StartupNetInfo));
      set(data->CH_ConfirmOffline   , MUIA_ShortHelp, GetStr(MSG_Help_ConfirmOffline));
      set(data->CH_Debug            , MUIA_ShortHelp, GetStr(MSG_Help_Debug));
      set(data->CH_FlushUserOnExit  , MUIA_ShortHelp, GetStr(MSG_Help_FlushUserOnExit));
      set(data->CH_NoAutoTraffic    , MUIA_ShortHelp, GetStr(MSG_Help_NoAutoTraffic));
      set(data->CH_StartupInetd     , MUIA_ShortHelp, GetStr(MSG_Help_StartupInetd));
      set(data->CH_StartupLoopback  , MUIA_ShortHelp, GetStr(MSG_Help_StartupLoopback));
      set(data->CH_StartupTCP       , MUIA_ShortHelp, GetStr(MSG_Help_StartupTCP));
      set(data->STR_Startup         , MUIA_ShortHelp, GetStr(MSG_Help_Startup));
      set(data->CY_Startup          , MUIA_ShortHelp, GetStr(MSG_Help_ExecType));
      set(data->STR_Shutdown        , MUIA_ShortHelp, GetStr(MSG_Help_Shutdown));
      set(data->CY_Shutdown         , MUIA_ShortHelp, GetStr(MSG_Help_ExecType));
      set(data->SL_KernelPriority   , MUIA_ShortHelp, GetStr(MSG_Help_KernelPriority));
      set(data->STR_LogFile         , MUIA_ShortHelp, GetStr(MSG_Help_LogFile));
      set(data->SL_LogLevel         , MUIA_ShortHelp, GetStr(MSG_Help_LogLevel));
      set(data->SL_LogFileLevel     , MUIA_ShortHelp, GetStr(MSG_Help_LogFileLevel));
      set(data->STR_MBufInitial     , MUIA_ShortHelp, GetStr(MSG_Help_MBufInitial));
      set(data->STR_MBufChunk       , MUIA_ShortHelp, GetStr(MSG_Help_MBufChunk));
      set(data->STR_MBufClChunk     , MUIA_ShortHelp, GetStr(MSG_Help_MBufClChunk));
      set(data->STR_MBufMaxMem      , MUIA_ShortHelp, GetStr(MSG_Help_MBufMaxMem));
      set(data->STR_MBufClusterSize , MUIA_ShortHelp, GetStr(MSG_Help_MBufClusterSize));
      set(data->CH_DebugSana        , MUIA_ShortHelp, GetStr(MSG_Help_DebugSana));
      set(data->CH_DebugICMP        , MUIA_ShortHelp, GetStr(MSG_Help_DebugICMP));
      set(data->CH_DebugIP          , MUIA_ShortHelp, GetStr(MSG_Help_DebugIP));
      set(data->CH_BeGateway        , MUIA_ShortHelp, GetStr(MSG_Help_BeGateway));
      set(data->CH_IPSendRedirects  , MUIA_ShortHelp, GetStr(MSG_Help_IPSendRedirects));
      set(data->STR_TCPSendSpace    , MUIA_ShortHelp, GetStr(MSG_Help_TCPSendSpace));
      set(data->STR_TCPRecvSpace    , MUIA_ShortHelp, GetStr(MSG_Help_TCPRecvSpace));
   }
   return((ULONG)obj);
}

///
/// Options_Dispatcher
SAVEDS ASM ULONG Options_Dispatcher(register __a0 struct IClass *cl, register __a2 Object *obj, register __a1 Msg msg)
{
   if(msg->MethodID == OM_NEW)
      return(Options_New               (cl, obj, (APTR)msg));

   return(DoSuperMethodA(cl, obj, msg));
}

///
/// LogLevel_Dispatcher
SAVEDS ASM ULONG LogLevel_Dispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg)
{
   if(msg->MethodID == MUIM_Numeric_Stringify)
   {
      struct LogLevel_Data *data = INST_DATA(cl,obj);
      struct MUIP_Numeric_Stringify *m = (APTR)msg;

      switch(7 - (m->value / 10))
      {
         case LOG_EMERG:
            strcpy(data->buf, "\033cemergency");
            break;
         case LOG_ALERT:
            strcpy(data->buf, "\033calert");
            break;
         case LOG_CRIT:
            strcpy(data->buf, "\033ccritical");
            break;
         case LOG_ERR:
            strcpy(data->buf, "\033cerror");
            break;
         case LOG_WARNING:
            strcpy(data->buf, "\033cwarning");
            break;
         case LOG_NOTICE:
            strcpy(data->buf, "\033cnotice");
            break;
         case LOG_INFO:
            strcpy(data->buf, "\033cinfo");
            break;
         default:
            strcpy(data->buf, "\033cdebug");
            break;
      }
      return((ULONG)data->buf);
   }
   return(DoSuperMethodA(cl, obj, msg));
}

///

