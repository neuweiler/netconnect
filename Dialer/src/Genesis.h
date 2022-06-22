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

/// Config
#define CFL_IgnoreDSR            (1)
#define CFL_7Wire                (1 << 1)    // default on
#define CFL_RadBoogie            (1 << 2)    // default on
#define CFL_XonXoff              (1 << 3)
#define CFL_OwnDevUnit           (1 << 4)
#define CFL_QuickReconnect       (1 << 5)
#define CFL_Debug                (1 << 6)
#define CFL_ConfirmOffline       (1 << 7)
#define CFL_ShowLog              (1 << 8)    // default on
#define CFL_ShowLamps            (1 << 9)    // default on
#define CFL_ShowConnect          (1 << 10)   // default on
#define CFL_ShowOnlineTime       (1 << 11)   // default on
#define CFL_ShowButtons          (1 << 12)   // default on
#define CFL_ShowNetwork          (1 << 13)   // default on
#define CFL_ShowUser             (1 << 14)   // default on
#define CFL_ShowStatusWin        (1 << 15)   // default on
#define CFL_ShowSerialInput      (1 << 16)   // default on
#define CFL_StartupOpenWin       (1 << 17)   // default on
#define CFL_StartupIconify       (1 << 18)


struct Config
{
   char    cnf_serialdevice[81];
   int     cnf_serialunit;
   ULONG   cnf_baudrate;
   ULONG   cnf_serbuflen;

   char    cnf_modemname[81];
   char    cnf_initstring[81];
   char    cnf_dialprefix[41];
   char    cnf_dialsuffix[41];
   int     cnf_redialattempts;
   int     cnf_redialdelay;
   ULONG   cnf_flags;

   char    *cnf_startup;
   char    *cnf_shutdown;
};
///
/// Interface
#define IFL_IsOnline       (1)         /* is the iface currently online ? */
#define IFL_PutOnline      (1 << 1)    /* shall the iface be put online in this pass ? */
#define IFL_PutOffline     (1 << 2)    /* shall the iface be put online in this pass ? */
#define IFL_AlwaysOnline   (1 << 3)    /* must iface be online as longa as genesis's running ? */

#define IFL_IPDynamic      (1 << 4)    /* was ip addr set to dynamic at startup ? */
#define IFL_DSTDynamic     (1 << 5)    /* was dst dynamic */
#define IFL_GWDynamic      (1 << 6)    /* was gateway dynamic */
#define IFL_NMDynamic      (1 << 7)    /* was netmask dynamic */
#define IFL_PPP            (1 << 8)    /* is it default ppp */
#define IFL_SLIP           (1 << 9)    /* is it default slip */

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
   char    if_sana2device[MAXPATHLEN];
   int     if_sana2unit;
   char    if_sana2config[MAXPATHLEN];
   char    *if_sana2configtext;
   char    *if_configparams;
   int     if_MTU;             /* maximum transfer unit */

   char    if_addr[16];        /* IP address */
   char    if_dst[16];         /* destination (or broadcast) address */
   char    if_gateway[16];     /* default gateway */
   char    if_netmask[16];     /* netmask */

   ULONG   if_flags;           /* flags for the iface */
   int     if_keepalive;       /* ping interval in minutes */

   struct MinList if_events;
   APTR    if_userdata;
};

///
/// ISP
enum { CNF_Assign_Static = 1, CNF_Assign_IFace, CNF_Assign_BootP, CNF_Assign_Root, CNF_Assign_DNSQuery };

#define ISF_UseBootp          (1)         /* Use bootp */
#define ISF_GetTime           (1 << 1)    /* Ask time at given server */
#define ISF_SaveTime          (1 << 2)    /* Save time to batt clock */
#define ISF_DontQueryHostname (1 << 3)    /* Don't query hostname => speedup */

struct ISP
{
   char   isp_name[41];
   char   isp_comment[41];
   char   isp_login[41];
   char   isp_password[41];

   char   isp_phonenumber[101];
   char   isp_organisation[41];

   char   isp_bootp[16];       /* bootp server IP address */
   char   isp_hostname[64];    /* space for fully qualified host name */
   char   isp_timename[64];
   UWORD  isp_flags;

   struct MinList isp_nameservers;
   struct MinList isp_domainnames;
   struct MinList isp_ifaces;
   struct MinList isp_loginscript;
};

///
/// User
#define UFL_Disabled       (1)    // I can't overwrite the password immediately when "disable" is selected because it might get deselected again

struct User
{
   char   us_login[41];
   char   us_realname[81];
   char   us_password[41];
   LONG   us_uid;
   LONG   us_gid;
   char   us_homedir[MAXPATHLEN];
   char   us_shell[81];
   USHORT us_flags;

   char   us_maillogin[41];
   char   us_mailpassword[41];
   char   us_email[81];
   char   us_mailserver[81];
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

/// ServerEntry   (41)
struct ServerEntry
{
   struct MinNode se_node;

   char se_name[41];
};

///
/// ScriptLine    (256)

enum { SL_Send = 0, SL_WaitFor, SL_Dial, SL_GoOnline, SL_SendLogin, SL_SendPassword, SL_SendBreak, SL_Exec, SL_Pause, SL_ParseIP, SL_ParsePasswd };
#ifdef USE_SCRIPT_COMMANDS
STRPTR script_commands[] = { "Send", "WaitFor", "Dial", "GoOnline", "SendLogin", "SendPassword", "SendBreak", "Exec", "Pause", "ParseIP", "ParsePasswd", NULL };
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
/// struct Modem
struct Modem
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
/// struct Network
struct Network
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




