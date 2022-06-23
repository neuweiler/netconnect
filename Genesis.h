/// includes
#include <exec/types.h>
#include <exec/nodes.h>
///

/// general defines
//#define REG(x) register __##x
//#define SAVEDS __saveds
//#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define MAXPATHLEN 256
#define BUFSIZE      512

#define IO_SIGBIT(req)  ((LONG)(((struct IORequest *)req)->io_Message.mn_ReplyPort->mp_SigBit))
#define IO_SIGMASK(req) ((LONG)(1L<<IO_SIGBIT(req)))

///

#define DEFAULT_CONFIGFILE "AmiTCP:db/genesis.conf"
enum { ASSIGN_Static = 1, ASSIGN_IFace, ASSIGN_BOOTP, ASSIGN_Root, ASSIGN_DNSQuery, ASSIGN_ICMP };

#ifdef DEMO
#define MAX_DAYS 100
#define WARN_DAYS 90
#endif

/// Config
#define CFL_Debug                (1 << 1)
#define CFL_ConfirmOffline       (1 << 2)
#define CFL_ShowLog              (1 << 3)    // default on
#define CFL_ShowLeds             (1 << 4)    // default on
#define CFL_ShowConnect          (1 << 5)    // default on
#define CFL_ShowOnlineTime       (1 << 6)    // default on
#define CFL_ShowButtons          (1 << 7)    // default on
#define CFL_ShowInterface        (1 << 8)    // default on
#define CFL_ShowUser             (1 << 9)    // default on
#define CFL_ShowStatusWin        (1 << 10)   // default on
#define CFL_ShowSerialInput      (1 << 11)   // default on
#define CFL_StartupOpenWin       (1 << 12)   // default on
#define CFL_StartupIconify       (1 << 13)
#define CFL_StartupInetd         (1 << 14)   // default on
#define CFL_StartupLoopback      (1 << 15)   // default on
#define CFL_StartupTCP           (1 << 16)   // default on
#define CFL_FlushUserOnExit      (1 << 17)
#define CFL_NoAutoTraffic        (1 << 18)

struct Config
{
   ULONG   cnf_flags;

   struct MinList cnf_ifaces;
   struct MinList cnf_modems;

   char    *cnf_startup;
   int     cnf_startuptype;
   char    *cnf_shutdown;
   int     cnf_shutdowntype;
};
///
/// Modem
#define MFL_IgnoreDSR            (1 << 0)
#define MFL_7Wire                (1 << 1) // default on
#define MFL_RadBoogie            (1 << 2) // default on
#define MFL_XonXoff              (1 << 3)
#define MFL_OwnDevUnit           (1 << 4)
#define MFL_DropDTR              (1 << 6) // drop DTR to hangup, default on
#define MFL_SerialLocked         (1 << 7) // ser.dev locked by owndevunit

struct Modem
{
   struct MinNode mo_node;

   int      mo_id;      // a unique ID#, used for iface_serial;
   char     mo_name[41];
   char     mo_comment[41];

   char     mo_device[MAXPATHLEN];
   int      mo_unit;
   ULONG    mo_baudrate;
   ULONG    mo_serbuflen;
   ULONG    mo_flags;

   char     mo_init[81];
   char     mo_dialprefix[41];
   char     mo_dialsuffix[41];
   char     mo_answer[41];
   char     mo_hangup[41];

   char     mo_ring[21];
   char     mo_connect[21];
   char     mo_nocarrier[21];
   char     mo_nodialtone[21];
   char     mo_busy[21];
   char     mo_ok[21];
   char     mo_error[21];

   int      mo_redialattempts;
   int      mo_redialdelay;
   int      mo_commanddelay;  // delay after each command sent to modem

   struct   IOExtSer *mo_serreq;
   struct   MsgPort  *mo_serport;
};

///
/// Interface
#define IFL_IsOnline       (1 << 0)    /* is the iface currently online ? */
#define IFL_PutOnline      (1 << 1)    /* shall the iface be put online  ? */
#define IFL_PutOffline     (1 << 2)    /* shall the iface be put offline ? */
#define IFL_AutoOnline     (1 << 3)    /* put iface online automatically ? */
#define IFL_IPDynamic      (1 << 4)    /* was ip addr set to dynamic at startup ? */
#define IFL_DSTDynamic     (1 << 5)    /* was dst dynamic */
#define IFL_GWDynamic      (1 << 6)    /* was gateway dynamic */
#define IFL_NMDynamic      (1 << 7)    /* was netmask dynamic */
#define IFL_PPP            (1 << 8)    /* is it default ppp */
#define IFL_SLIP           (1 << 9)    /* is it default slip */
#define IFL_BOOTP          (1 << 10)   /* use BOOTP for this iface */
#define IFL_IsDefaultGW    (1 << 11)   /* is this iface used for default gateway atm ? */
#define IFL_GetTime        (1 << 12)   /* Ask time at given server */
#define IFL_SaveTime       (1 << 13)   /* Save time to batt clock */
#define IFL_UseDomainName  (1 << 14)   /* should domainname be retrieved and used ? */
#define IFL_UseHostName    (1 << 15)   /* should hostname be retrieved and used ? */
#define IFL_UseNameServer  (1 << 16)   /* should appp's ms-dns be used ? */
#define IFL_DialIn         (1 << 17)   /* is this iface used as a dial-in ? */

enum { IFE_Online = 0, IFE_OnlineFail, IFE_OfflineActive, IFE_OfflinePassive };
#ifdef USE_EVENT_COMMANDS
STRPTR event_commands[] = { "Online", "OnlineFail", "OfflineActive", "OfflinePassive", NULL };
#else
extern STRPTR event_commands[];
#endif

struct Interface
{
   struct MinNode if_node;

   char    if_name[21];
   char    if_comment[41];

   char    if_sana2device[MAXPATHLEN];
   int     if_sana2unit;
   char    if_sana2config[MAXPATHLEN];
   char    *if_sana2configtext;
   char    *if_configparams;

   char    if_hostname[64];
   char    if_addr[16];        /* IP address */
   char    if_dst[16];         /* destination (or broadcast) address */
   char    if_gateway[16];     /* default gateway */
   char    if_netmask[16];     /* netmask */
   int     if_MTU;             /* maximum transfer unit */

   ULONG   if_flags;           /* flags for the iface */
   int     if_keepalive;       /* ping interval in minutes */
   char    if_timename[64];    /* name of timeserver to query */

   int     if_modemid;      // see struct modem
   char    if_login[41];
   char    if_password[41];
   char    if_phonenumber[101];

   struct  MinList if_loginscript;
   struct  MinList if_events;
   struct  MinList if_nameservers;
   struct  MinList if_domainnames;

   struct  Process *if_dhcp;     /* pointer to dhcp handler */
   struct  Process *if_dialin;   /* pointer to dial-in handler */
   APTR    if_loggerhandle;      /* handle for genesislogger.library */
   APTR    if_userdata;
};

///
/// PrefsPPPIface
struct PrefsPPPIface
{
   BOOL ppp_carrierdetect;
   ULONG ppp_connecttimeout;
   char ppp_callback[81];
   BOOL ppp_mppcomp;
   BOOL ppp_vjcomp;
   BOOL ppp_bsdcomp;
   BOOL ppp_deflatecomp;
   BOOL ppp_eof;
};
///

/// DHCP_Msg
struct DHCP_Msg
{
   struct Message    dhcp_Msg;
   struct Interface  *dhcp_iface;
   struct Interface_Data *dhcp_iface_data;
   BOOL              dhcp_abort;
   BOOL              dhcp_success;
};

#define DHCP_PORTNAME "GenesisDHCP"

///

/// Exec Types

enum { EXEC_CLI = 0, EXEC_WB, EXEC_Script, EXEC_ARexx };
#ifdef USE_EXEC_TYPES
STRPTR exec_types[] = { "CLI", "WB", "Script", "AREXX", NULL };
#else
extern STRPTR exec_types[];
#endif

///

/// ServerEntry   (41)
struct ServerEntry
{
   struct MinNode se_node;

   char se_name[41];
};

///
/// ScriptLine    (256)

enum { SL_Send = 0, SL_WaitFor, SL_Dial, SL_GoOnline, SL_SendLogin, SL_SendPassword, SL_SendBreak, SL_Exec, SL_Pause, SL_ParseIP };
#ifdef USE_SCRIPT_COMMANDS
STRPTR script_commands[] = { "Send", "WaitFor", "Dial", "GoOnline", "SendLogin", "SendPassword", "SendBreak", "Exec", "Pause", "ParseIP", NULL };
#else
extern STRPTR script_commands[];
#endif

struct ScriptLine
{
   struct MinNode sl_node;

   int sl_command;
   char sl_contents[256];
   int sl_userdata;
};
///
/// struct ModemX
struct ModemX
{
   char Name[81];
   char ProtocolName[10][21];
   char InitString[10][41];
};

///
/// struct ModemProtocol
struct ModemProtocol
{
   char Name[41];
   char InitString[41];
};

///
/// struct Group
struct Group
{
   char Name[41];
   char Password[21];
   LONG ID;
   char Members[1024];
};

///
/// struct Protocol
struct Protocol
{
   char Name[41];
   WORD ID;
   char Aliases[81];
};

///
/// struct Service
struct Service
{
   char Name[41];
   WORD Port;
   char Protocol[41];
   char Aliases[81];
};

///
/// struct InetAccess
struct InetAccess
{
   char Service[41];
   char Host[81];
   BOOL Access;
   BOOL Log;
};

///
/// struct Inetd
struct Inetd
{
   char Service[41];
   BYTE Socket;
   char Protocol[41];
   BYTE Wait;
   char User[41];
   char Server[81];
   char CliName[81];
   char Args[81];

   BOOL Active;
};

///
/// struct Host
struct Host
{
   char Addr[21];
   char Name[41];
   char Aliases[81];
};

///
/// struct DB_Network
struct DB_Network
{
   char Name[41];
   ULONG Number;
   char Aliases[81];
};

///
/// struct Rpc
struct Rpc
{
   char Name[41];
   ULONG Number;
   char Aliases[81];
};

///

