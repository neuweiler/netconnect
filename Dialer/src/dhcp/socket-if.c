/// includes
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dhcp.h"
#include "dhcp-options.h"
#include "error-handler.h"
#include "if.h"
#include "client.h"
#include "/protos.h"

///

/// setSockAddrIn
void setSockAddrIn(u_short port, u_long inaddr, struct sockaddr_in *saddr)
{
   bzero((char *)saddr, sizeof(*saddr));
   saddr->sin_family    = AF_INET;
   saddr->sin_addr.s_addr  = inaddr;
   saddr->sin_port         = port;
}

///
/// openSendSocket
BOOL openSendSocket(struct Library *SocketBase, struct sockaddr_in *addr, int *s)
{
   int optval = 1;

   if((*s = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
   {
      if(setsockopt(*s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) >= 0)
      {
         /* use DHCP client port for the source port because
          * some servers do NOT use the DHCP client port
          * BUT use the UDP source  port number of the received
          * datagram for the destination UDP port when it responds
          * to clients.
          */
         addr->sin_port = htons(DHCP_CLIENT_PORT);
         if(bind(*s, (struct sockaddr *)addr, sizeof(*addr)) >= 0)
            return(TRUE);
         else
            syslog_AmiTCP(SocketBase, LOG_ERR, "bind (openSendSocket)");
      }
      else
         syslog_AmiTCP(SocketBase, LOG_ERR, "setsockopt (openSendSocket)");
   }
   else
      syslog_AmiTCP(SocketBase, LOG_ERR, "socket (openSendSocket)");

   return(FALSE);
}

///
/// openRecvSocket
BOOL openRecvSocket(struct Library *SocketBase, struct sockaddr_in *addr, int *s)
{
   int optval = 1;

   if((*s = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
   {
      if(setsockopt(*s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) >= 0)
      {
         addr->sin_port = htons(DHCP_CLIENT_PORT);
         if(bind(*s, (struct sockaddr *)addr, sizeof(*addr)) >= 0)
            return(TRUE);
         else
            syslog_AmiTCP(SocketBase, LOG_ERR, "bind (openRecvSocket)");
      }
      else
         syslog_AmiTCP(SocketBase, LOG_ERR, "setsockopt (openRecvSocket)");
   }
   else
      syslog_AmiTCP(SocketBase, LOG_ERR, "socket (openRecvSocket)");

   return(FALSE);
}

///
/// openRawSocket
BOOL openRawSocket(struct Library *SocketBase, int *s, u_short type)
{
   int optval = 1;

   if((*s = socket(AF_INET, SOCK_RAW, htons(type))) >= 0)
   {
      if ( setsockopt(*s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) >= 0 )
         return(TRUE);
      else
         syslog_AmiTCP(SocketBase, LOG_ERR, "setsockopt (openRawSocket)");
   }
   else
      syslog_AmiTCP(SocketBase, LOG_ERR, "socket (openRawSocket)");

   return(FALSE);
}

///
/// rcvAndCheckDhcpMsg
int rcvAndCheckDhcpMsg(struct Library *SocketBase, int s, dhcpMessage *msg, u_long waitMsgType, u_char *optp[], long timeout)
{
   int readfds, n, len;
   time_t prevTime;
   struct sockaddr_in addr;
   struct timeval tm;

   bzero((char *)msg, sizeof(*msg));
   bzero((char *)&addr, sizeof(addr));
   tm.tv_sec = 0;
   time(&prevTime); 
   while(timeout > 0)
   {
//      FD_ZERO(&fdset);
//      FD_SET(s, &fdset);
      tm.tv_usec  = timeout;
      readfds = (1 << s);
Printf("s: %ld\n", s);
      if(n = WaitSelect(s+1, (fd_set *)&readfds, (fd_set *)NULL, (fd_set *)NULL, &tm, NULL) < 0)
      {
Printf("errno: %ld,  ebadf=%ld  eintr=%ld   einval=%ld\n", errno, EBADF, EINTR, EINVAL);
         syslog_AmiTCP(SocketBase, LOG_ERR, "select (rcvAndCheckDhcpMsg)");
         return(0);         /* receive unsuccessful */
      }
      len = sizeof(*msg);
Printf("n=%ld\n", n);
      if(n)
      {
         if(recvfrom(s, (char *)msg, len, 0, (struct sockaddr *)&addr, (LONG *)&len) < 0)
         {
            syslog_AmiTCP(SocketBase, LOG_ERR, "recvfrom (rcvAndCheckDhcpMsg)");
            return(0);      /* receive unsuccessful */
         }
         if(parseDhcpMsg(optp, msg))
         {
            if(waitMsgType & (1 << (*(optp[OmsgType]+1)-1)))
               return(1);
         }
      }
      timeout -= (time(NULL) - prevTime)*1000000;
      time(&prevTime);
   }
   return(0);               /* receive timeout */
}

///
/// waitChkReXmitMsg
int waitChkReXmitMsg(struct Library *SocketBase, int sRecv, dhcpMessage *pMsgRecv, int sSend, dhcpMessage *pMsgSend, struct sockaddr_in *addr, u_long waitMsgType, u_char *optp[], int nretry)
{
   long  tm;

   tm = getNextTimeout(INIT_TIMEOUT);
   while(nretry--)
   {
      if(rcvAndCheckDhcpMsg(SocketBase, sRecv, pMsgRecv, waitMsgType, optp, tm))
         return(1);
      if(sendto(sSend, (char *)pMsgSend, sizeof(*pMsgSend), 0, (struct sockaddr *)addr, sizeof(*addr)) < 0)
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, "sendto (waitChkReXmitMsg)");
         break;
      }
      tm = getNextTimeout(NEXT_TIMEOUT);
   }
   return(0);
}

///
/// setWaitMsgType
void setWaitMsgType(int type, u_int *ptype)
{
   *ptype |= (1 << (type-1));
}

///

