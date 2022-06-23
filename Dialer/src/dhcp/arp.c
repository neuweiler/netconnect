/// includes
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "//Genesis.h"
#include "/iface.h"
#include "dhcp.h"
#include "if.h"
#include "arp.h"
#include "socket-if.h"
#include "error-handler.h"
#include "/protos.h"

///

/// arpCheck
int arpCheck(struct Library *SocketBase, u_long inaddr, struct Interface_Data *iface_data, long timeout)
{
   int            s;          /* socket */
   int            rv;         /* return value */
   struct sockaddr addr;      /* for interface name */
   struct arpMsg  arp;
   fd_set         fdset;
   struct timeval tm;
   time_t         prevTime;

   rv = 1;
   if(openRawSocket(SocketBase, &s, ETH_P_ARP))
   {
      /* send arp request */
      mkArpMsg(ARPOP_REQUEST, inaddr, NULL, iface_data->ifd_addr, iface_data->ifd_haddr, &arp);
      bzero(&addr, sizeof(addr));
      strcpy(addr.sa_data, iface_data->ifd_name);
      if(sendto(s, (UBYTE const *)&arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0)
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, "sendto (arpCheck)");
         rv = 0;
      }

      /* wait arp reply, and check it */
      tm.tv_usec = 0;
      time(&prevTime);
      while(timeout > 0)
      {
         FD_ZERO(&fdset);
         FD_SET(s, &fdset);
         tm.tv_sec  = timeout;
         if(WaitSelect(s+1, &fdset, (fd_set *)NULL, (fd_set *)NULL, &tm, NULL) < 0)
         {
            syslog_AmiTCP(SocketBase, LOG_ERR, "WaitSelect (arpCheck)");
            rv = 0;
         }
         if(FD_ISSET(s, &fdset))
         {
            if(recv(s, (UBYTE *)&arp, sizeof(arp), 0) < 0)
            {
               syslog_AmiTCP(SocketBase, LOG_ERR, "recv (arpCheck)");
               rv = 0;
            }
            if(arp.operation == htons(ARPOP_REPLY) &&
               bcmp(arp.tHaddr, iface_data->ifd_haddr, 6) == 0 &&
               *((u_int *)arp.sInaddr) == inaddr)
            {
               rv = 0;
               break;
            }
         }
         timeout -= time(NULL) - prevTime;
         time(&prevTime);
      }
      CloseSocket(s);
   }
   else
      rv = 0;

   return(rv);
}

///
/// sendArpReply
void sendArpReply(struct Library *SocketBase, u_char *thaddr, u_long tinaddr, struct Interface_Data *iface_data)
{
   int            s;       /* socket */
   struct sockaddr addr;      /* for interface name */
   struct arpMsg  arp;

   if(openRawSocket(SocketBase, &s, htons(ETH_P_ARP)))
   {
      /* send arp reply */
      mkArpMsg(ARPOP_REPLY, tinaddr, thaddr, iface_data->ifd_addr, iface_data->ifd_haddr, &arp);
      bzero(&addr, sizeof(addr));
      strcpy(addr.sa_data, iface_data->ifd_name);
      if(sendto(s, (UBYTE const *)&arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0)
         syslog_AmiTCP(SocketBase, LOG_ERR, "sendto (arpCheck)");
   }
   else
      syslog_AmiTCP(SocketBase, LOG_ERR, "openRawSocket (arpCheck)");
}

///
/// mkArpMsg
void mkArpMsg(int opcode, u_long tInaddr, u_char *tHaddr, u_long sInaddr, u_char *sHaddr, struct arpMsg *msg)
{
   bzero(msg, sizeof(*msg));
   bcopy(MAC_BCAST_ADDR, msg->ethhdr.h_dest, 6);   /* MAC DA */
   bcopy(sHaddr, msg->ethhdr.h_source, 6);         /* MAC SA */
   msg->ethhdr.h_proto = htons(ETH_P_ARP);         /* protocol type (Ethernet) */
   msg->htype = htons(ARPHRD_ETHER);               /* hardware type */
   msg->ptype = htons(ETH_P_IP);                   /* protocol type (ARP message) */
   msg->hlen = 6;                                  /* hardware address length */
   msg->plen = 4;                                  /* protocol address length */
   msg->operation = htons(opcode);                 /* ARP op code */
   *((u_int *)msg->sInaddr) = sInaddr;             /* source IP address */
   bcopy(sHaddr, msg->sHaddr, 6);                  /* source hardware address */
   *((u_int *)msg->tInaddr) = tInaddr;             /* target IP address */
   if (opcode == ARPOP_REPLY)
      bcopy(tHaddr, msg->tHaddr, 6);               /* target hardware address */
}

///
