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

/// Config
enum { CNF_Assign_Static = 1, CNF_Assign_IFace, CNF_Assign_BootP, CNF_Assign_Root, CNF_Assign_DNSQuery };
#define DEFAULT_CONFIGFILE "AmiTCP:config/Default.config"

struct config
{
   char    cnf_loginname[81]; // for login at provider
   char    cnf_username[81];  // for env:LOGNAME
   char    cnf_password[81];
   char    cnf_realname[81];
   char    cnf_email[81];
   char    cnf_organisation[81];

   char    cnf_phonenumber[256];

   char    cnf_serialdevice[81];
   int     cnf_serialunit;
   ULONG   cnf_baudrate;
   USHORT  cnf_carrierdetect;
   USHORT  cnf_7wire;
   ULONG   cnf_serbuflen;

   char    cnf_modemname[81];
   char    cnf_initstring[81];
   char    cnf_dialprefix[81];
   char    cnf_dialsuffix[81];
   int     cnf_redialattempts;
   int     cnf_redialdelay;

   char    cnf_sana2device[MAXPATHLEN];
   int     cnf_sana2unit;
   char    cnf_sana2config[MAXPATHLEN];
   char    *cnf_sana2configtext;
   char    cnf_ifname[21];
   char    *cnf_ifconfigparams;
   int     cnf_MTU;           /* maximum transfer unit */
   int     cnf_keepalive;     /* do nothing, icmp ping (1) or ppp ping (2) */
   int     cnf_pinginterval;  /* interval in minutes */

   ULONG  cnf_addr;          /* IP address */
   ULONG  cnf_dst;           /* destination (or broadcast) address */
   ULONG  cnf_netmask;       /* netmask */
   ULONG  cnf_gateway;       /* default gateway */
   ULONG  cnf_dns1;
   ULONG  cnf_dns2;
   ULONG  cnf_bootpserver;   /* bootp server IP address */
   USHORT cnf_use_bootp;

   char    cnf_hostname[64]; /* space for fully qualified host name */
   char    cnf_domainname[64];  /* space for our domain name */
   char    cnf_timename[64];

   USHORT cnf_autologin;
   USHORT cnf_onlineonstartup;
   USHORT cnf_quickreconnect;
   USHORT cnf_synclock;
   USHORT cnf_showstatus;
   USHORT cnf_showspeed;
   USHORT cnf_showonlinetime;
   USHORT cnf_showbuttons;

   char    *cnf_online;             /* pointer to file to be executed */
   char    *cnf_onlinefail;
   char    *cnf_offlineactive;
   char    *cnf_offlinepassive;
   char    *cnf_startup;
   char    *cnf_shutdown;
   USHORT cnf_winonline;           /* 0= do nothing, 1= open, 2= close */
   USHORT cnf_winonlinefail;
   USHORT cnf_winofflineactive;
   USHORT cnf_winofflinepassive;
   USHORT cnf_winstartup;

/** private stuff **/

   ULONG  cnf_connectspeed;   /* baudrate we're connected */
   USHORT cnf_use_hwtype;  /* use hardware type on BOOTP? */

};
///

/// ScriptLine

enum { SL_Send = 0, SL_WaitFor, SL_Dial, SL_GoOnline, SL_SendLogin, SL_SendPassword, SL_SendBreak, SL_Exec, SL_Pause };
#ifdef USE_SCRIPT_COMMANDS
STRPTR script_commands[] = { "Send", "WaitFor", "Dial", "GoOnline", "SendLogin", "SendPassword", "SendBreak", "Exec", "Pause", NULL };
#else
extern STRPTR script_commands[];
#endif

struct ScriptLine
{
   struct MinNode sl_node;

   int sl_command;
   char sl_contents[256];
};
///
/// struct pc_Data
struct pc_Data
{
   STRPTR Buffer;    /* buffer holding the file (internal use only) */
   LONG Size;        /* variable holding the size of the buffer (internal use only) */
   STRPTR Current;   /* pointer to the current position (internal use only) */

   STRPTR Argument;  /* pointer to the argument */
   STRPTR Contents;  /* pointer to its contents */
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
/// struct User
struct User
{
   char Login[41];
   char Password[41];
   LONG UserID;
   LONG GroupID;
   char Name[81];
   char HomeDir[MAXPATHLEN];
   char Shell[81];
   BOOL Disabled;       // I can't overwrite the password immediately when "disable" is selected because it might get deselected again
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




