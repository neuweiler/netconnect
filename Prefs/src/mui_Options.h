struct LogLevel_Data
{
   char buf[20];
};


struct Options_Data
{
   Object *PA_Startup;
   Object *STR_Startup;
   Object *CY_Startup;
   Object *PA_Shutdown;
   Object *STR_Shutdown;
   Object *CY_Shutdown;

   Object *CH_ConfirmOffline;
   Object *CH_Debug;
   Object *CH_FlushUserOnExit;
   Object *CH_NoAutoTraffic;
   Object *CH_StartupInetd;
   Object *CH_StartupLoopback;
   Object *CH_StartupTCP;

   Object *CY_MainWindow;
   Object *CH_ShowLog;
   Object *CH_ShowLamps;
   Object *CH_ShowConnect;
   Object *CH_ShowOnlineTime;
   Object *CH_ShowIface;
   Object *CH_ShowUser;
   Object *CH_ShowButtons;
   Object *CH_ShowStatusWin;
   Object *CH_ShowSerialInput;

   Object *SL_KernelPriority;
   Object *PA_LogFile;
   Object *STR_LogFile;
   Object *SL_LogLevel;
   Object *SL_LogFileLevel;
   Object *STR_MBufInitial;
   Object *STR_MBufChunk;
   Object *STR_MBufClChunk;
   Object *STR_MBufMaxMem;
   Object *STR_MBufClusterSize;
   Object *CH_DebugSana;
   Object *CH_DebugICMP;
   Object *CH_DebugIP;
   Object *CH_BeGateway;
   Object *CH_IPSendRedirects;
   Object *STR_TCPSendSpace;
   Object *STR_TCPRecvSpace;
};

