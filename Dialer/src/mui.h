#define MUISERIALNR_GENESIS 1
#define TAGBASE_GENESIS (TAG_USER | (MUISERIALNR_GENESIS << 16))

#define VVSpace MUI_NewObject(MUIC_Rectangle, MUIA_FixWidth, 1, TAG_DONE)
enum { MEN_ABOUT=1, MEN_REPORT, MEN_NETINFO, MEN_ABOUT_MUI, MEN_QUIT, MEN_MUI,
       MEN_GENESIS, MEN_DISPLAY, MEN_TIMEONLINE, MEN_LEDS, MEN_CONNECT, MEN_INTERFACE,
       MEN_USER, MEN_LOG, MEN_BUTTONS, MEN_STATUS, MEN_SERIAL, MEN_MAINWINDOW };
enum { ID_DOUBLESTART=40 };

#define MUIV_Background_Red      "2:cccccccc,60606060,60606060"
#define MUIV_Background_Orange   "2:cccccccc,a0a0a0a0,70707070"
#define MUIV_Background_Yellow   "2:cccccccc,cccccccc,70707070"
#define MUIV_Background_Green    "2:70707070,cccccccc,70707070"

/*
MainWindow: 1000
Online    : 1010
Genesis   : 1020
IfaceReq  : 1030
NetInfo   : 1040
MainWindow: 1050
*/

/// MainMessage
struct MainMessage
{
   struct Message msg;

   Object *obj;
   LONG MethodID;
   APTR data1;
   APTR data2;
   APTR data3;
   ULONG result;
};

///
/// LogEntry
struct LogEntry
{
   char time[31];
   char type[21];
   char info[81];
};

///

#define MUIM_Genesis_Handshake                   (TAGBASE_GENESIS | 0x1020)
#define MUIM_Genesis_Get                         (TAGBASE_GENESIS | 0x1021)


