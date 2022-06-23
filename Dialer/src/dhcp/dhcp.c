/// includes & defines
#include "//Genesis.h"
#include "/protos.h"
#include "/Strings.h"
#include "/mui.h"
#include "/mui_MainWindow.h"
#include "//genesis.lib/pragmas/nc_lib.h"
#include "/iface.h"
#include "if.h"
#include "dhcp.h"
#include "signal-handler.h"
#include "error-handler.h"
#include "daemon.h"
#include "client.h"
#include "memory.h"

#define DEFAULT_IF   "eth0"

///
/// variables
int     BeRFC1541 = 0;        /* default is InternetDraft mode */
char *Hostname = NULL;        /* hostname in the DHCP msg for xmit */

extern Object *app, *win;
extern BOOL dialup;
#ifdef NETCONNECT
extern struct Library *NetConnectBase;
#endif

///

/// send_dhcp_msg
struct Interface *send_dhcp_msg(BOOL abort, BOOL success, struct Interface_Data **iface_data)
{
   struct Interface *iface = NULL;
   struct MsgPort *reply_port;
   struct DHCP_Msg *msg;
   ULONG sig;

   if(FindPort(DHCP_PORTNAME))
   {
      if(reply_port = CreatePort(NULL, 0))
      {
         if(msg = (struct DHCP_Msg *)AllocVec(sizeof(struct DHCP_Msg), MEMF_ANY | MEMF_CLEAR))
         {
            msg->dhcp_Msg.mn_Node.ln_Type = NT_MESSAGE;
            msg->dhcp_Msg.mn_Length       = sizeof(struct DHCP_Msg);
            msg->dhcp_Msg.mn_ReplyPort    = reply_port;
            msg->dhcp_abort               = abort;
            msg->dhcp_success             = success;

            if(safe_put_to_port((struct Message *)msg, DHCP_PORTNAME))
            {
               sig = Wait((1 << reply_port->mp_SigBit) | SIGBREAKF_CTRL_C);

               if(sig & (1 << reply_port->mp_SigBit))
               {
                  if(GetMsg(reply_port))
                  {
                     iface = msg->dhcp_iface;
                     if(iface_data)
                        *iface_data = msg->dhcp_iface_data;
                  }
               }
            }
            FreeVec(msg);
         }
         DeletePort(reply_port);
      }
   }
   return(iface);
}

///
/// DHCPHandler
SAVEDS ASM VOID DHCPHandler(register __a0 STRPTR args, register __d0 LONG arg_len)
{
   char *clientId = NULL;     /* ptr to client identifier user specified */
   struct Interface *iface;
   struct Interface_Data *iface_data, *src;
   struct Library *SocketBase;

   if(iface = send_dhcp_msg(FALSE, FALSE, &src))
   {
#ifdef NETCONNECT
      SocketBase = NCL_OpenSocket();
#else
      SocketBase = OpenLibrary("bsdsocket.library", 0);
#endif
      if(SocketBase)
      {
         if(iface_data = AllocVec(sizeof(struct Interface_Data), MEMF_ANY))
         {
            bcopy(src, iface_data, sizeof(struct Interface_Data));
            iface_data->ifd_fd = NULL;
            iface_data->ifd_s2 = NULL;

            srand((u_int)time(NULL));
            classIDsetup(NULL);        /* setup default class identifier */

            ifReset(SocketBase, iface, iface_data);        /* reset interface, 'Ifbuf' */

            if((iface_data->ifd_flags & IFF_POINTOPOINT) && !dialup)
            {
               if((iface_data->ifd_fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
               {
                  // Route "broadcasts" to the point-to-point destination
                  syslog_AmiTCP(SocketBase, LOG_DEBUG, "dhcp: adding route for broadcast address on p-t-p link.");
                  route_add(SocketBase, iface_data->ifd_fd, RTF_UP | RTF_GATEWAY|RTF_HOST, INADDR_BROADCAST, iface_data->ifd_dst, TRUE);

                  CloseSocket(iface_data->ifd_fd);
                  iface_data->ifd_fd = -1;
               }
               else
                  syslog_AmiTCP(SocketBase, LOG_ERR, "socket (DHCPHandler)");
            }

            if(clientIDsetup(SocketBase, clientId, iface_data))
            {
               dhcpMsgInit(SocketBase, iface->if_name);
               dhcpClient(SocketBase, iface, iface_data);
            }

            if((iface_data->ifd_flags & IFF_POINTOPOINT) && !dialup)
            {
               if((iface_data->ifd_fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
               {
                  // delete the added "broadcasts" route
                  syslog_AmiTCP(SocketBase, LOG_DEBUG, "dhcp: deleting route for broadcast address on p-t-p link");
                  route_delete(SocketBase, iface_data->ifd_fd, INADDR_BROADCAST);

                  CloseSocket(iface_data->ifd_fd);
                  iface_data->ifd_fd = -1;
               }
               else
                  syslog_AmiTCP(SocketBase, LOG_ERR, "socket (DHCPHandler)");
            }

            FreeVec(iface_data);
         }
      }
      else
         DoMethod(app, MUIM_Application_PushMethod, win, 3, MUIM_MainWindow_MUIRequest, GetStr(MSG_BT_Abort), GetStr(MSG_TX_ErrorBsdsocketLib));

      iface->if_dhcp_proc = NULL;
   }
   send_dhcp_msg(TRUE, FALSE, NULL);
}

///
