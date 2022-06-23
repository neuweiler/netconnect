/// includes
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "if.h"
#include "//Genesis.h"
#include "/iface.h"
#include "dhcp.h"
#include "dhcp-options.h"
#include "socket-if.h"
#include "arp.h"
#include "error-handler.h"
#include "hostinfo.h"
#include "client.h"
#include "/protos.h"

///
/// variables
extern int BeRFC1541;         /* if 1, Be RFC154 compliant */
extern char *Hostname;        /* if !NULL, put hostname into DHCP msg */
extern struct ExecBase *SysBase;

int (*Fsm[MAX_STATES])(struct Library *SocketBase, struct Interface *iface, struct Interface_Data *iface_data);     /* finite state machine */
int CurrState;             /* current state */
int PrevState;             /* previous state */

dhcpMessage DhcpMsgSend;      /* DHCP message to send */
dhcpMessage DhcpMsgRecv;      /* DHCP message received */

int Ssend;                 /* socket fd for send */
int Srecv;                 /* socket fd for receive */

time_t ReqSentTime;           /* time when DHCPREQUEST message is sent */
u_long SuggestLeaseTime = 0;  /* lease time suggested by the user */

/* DHCP information */
u_long ServerInaddr;       /* Server's IP address (network byte order) */
u_long LeaseTime;          /* lease time (network byte order) */
u_long RenewTime;          /* T1 time (network byte order) */
u_long RebindTime;            /* T2 time (network byte order) */

static u_char *OptPtr[MAXNOPT];  /* array of ptrs to DHCP option element */
static u_char *OptOffer[MAXNOPT];   /* same as above in DHCPOFFER msg */
static u_char ClassId[MAXIDCHARS];  /* class identifier */
static u_char ClientId[MAXIDCHARS]; /* client identifier */

///

/// classIDsetup
void classIDsetup(char *id)
{
   if(id != NULL)
   {
      strncpy(ClassId, id, sizeof(ClassId));
      return;
   }
   /* setup default class identifier if id is NULL */
   sprintf(ClassId, "AmigaOS %ld.%ld amiga", SysBase->LibNode.lib_Version, SysBase->SoftVer);
}

///
/// clientIDsetup
BOOL clientIDsetup(struct Library *SocketBase, char *id, struct Interface_Data *iface_data)
{
   u_char *s;
   int      len;

   s = ClientId;
   *s++ = 61;              /* type value of client identifier */

   if(id != NULL)
   {
      len = strlen(id);
      if(len > sizeof(ClientId) - 4)
      {
         /* 4: code, len, type, EOS */
         Printf("clientIDsetup: too long client ID string\n");
         return(FALSE);
      }
      *s++ = len + 1;         /* 1 for the # field */
      *s++ = 0;               /* type: string */
      strcpy(s, id);
      return(TRUE);
   }

   /* setup default client identifier if id is NULL */
   *s++ = 7;               /* length: 6 (MAC Addr) + 1 (# field) */
   *s++ = ARPHRD_ETHER;    /* type: Ethernet address */
   bcopy(iface_data->ifd_haddr, s, sizeof(iface_data->ifd_haddr));
   return(TRUE);
}

///
/// dhcpMsgInit
BOOL dhcpMsgInit(struct Library *SocketBase, u_char *ifname)
{
   struct sockaddr_in   addr;

#if 0
   /* open and bind socket for receiving */
   setSockAddrIn(htons(0), htonl(INADDR_ANY), &addr);
   openRecvSocket(&addr, &Srecv);
#endif

   /* open and bind socket for sending */
   setSockAddrIn(htons(DHCP_CLIENT_PORT), htonl(INADDR_ANY), &addr);
   if(openSendSocket(SocketBase, &addr, &Ssend))
   {
      Srecv = Ssend;
      return(TRUE);
   }
   return(FALSE);
}

///
/// dhcpClient
void dhcpClient(struct Library *SocketBase, struct Interface *iface, struct Interface_Data *iface_data)
{
   int next_state;

   Fsm[INIT_REBOOT]= initReboot;
   Fsm[INIT]       = init;
   Fsm[REBOOTING]  = rebooting;
   Fsm[SELECTING]  = selecting;
   Fsm[REQUESTING] = requesting;
   Fsm[BOUND]      = bound;
   Fsm[RENEWING]   = renewing;
   Fsm[REBINDING]  = rebinding;

   CurrState = PrevState = INIT_REBOOT;

   FOREVER
   {
Printf("exec state: %ld\n", CurrState);
Delay(100);
      next_state = (*Fsm[CurrState])(SocketBase, iface, iface_data);
Printf("next state: %ld\n", next_state);
      if(next_state == EXCEPTION)
      {
         syslog_AmiTCP(SocketBase, LOG_CRIT, "Exception occured in the fsm (dhcpClient)");
         break;
      }
      if(next_state == EXIT)
         break;
      PrevState = CurrState;
      CurrState  = next_state;
   }

   if(Srecv >= 0)
      CloseSocket(Srecv);
   if(Ssend >= 0)
      CloseSocket(Ssend);
   Ssend = Srecv = -1;
}

///
/// initReboot
int initReboot(struct Library *SocketBase, struct Interface *iface, struct Interface_Data *iface_data)
{
   int               fd;
   int               nextState = REBOOTING;
   char           filename[IFNAMSIZ + 128];
   struct sockaddr_in   addr;

   bzero((char *)&addr, sizeof(addr));
   strcpy(filename, DHCP_CACHE_FILE);
   strcat(filename, iface->if_name);
   if((fd = open(filename, O_RDONLY)) < 0)
      return(INIT);

   /* try to open cache file */
   if(read(fd, (char *)&addr.sin_addr.s_addr, sizeof(addr.sin_addr.s_addr)) < 0)
      nextState = INIT;
   close(fd);
   unlink(filename);
   if(nextState != REBOOTING)
      return(nextState);

   /* found cache file */
   mkDhcpDiscoverMsg(iface_data->ifd_haddr, &DhcpMsgSend); /* set up MAC adddr, etc. */
   /* TODO: cache file should have the lease time previously used */
   mkDhcpRequestMsg(REBOOTING, 0, 0, rand(), addr.sin_addr.s_addr, &DhcpMsgSend);
   setSockAddrIn(htons(DHCP_SERVER_PORT), htonl(INADDR_BROADCAST), &addr);
   time(&ReqSentTime);
#ifdef NEED_BCAST_RESPONSE
   DhcpMsgSend.flags = htons(F_BROADCAST);
#endif
   if(sendto(Ssend, (char *)&DhcpMsgSend, sizeof(DhcpMsgSend), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "sendto (initReboot)");
      return(EXIT);
   }
   return(REBOOTING);
}

///
/// init
int init(struct Library *SocketBase, struct Interface *iface, struct Interface_Data *iface_data)
{
   char ifname[IFNAMSIZ];
   int waitTime = 0;
   struct sockaddr_in   addr;

   freeOptInfo(OptOffer);     /* clear up 'OptOffer' */
   if(PrevState == RENEWING || PrevState == REBINDING)
   {
      strcpy(ifname, iface->if_name);
      ifReset(SocketBase, iface, iface_data);
      if(!dhcpMsgInit(SocketBase, iface->if_name))
         return(EXIT);
   }
   mkDhcpDiscoverMsg(iface_data->ifd_haddr, &DhcpMsgSend);
   setSockAddrIn(htons(DHCP_SERVER_PORT), htonl(INADDR_BROADCAST), &addr);

   do
   {
      waitTime = rand() % 10;
   } while (waitTime == 0);
   Delay(waitTime * 50);

   time(&ReqSentTime);        /* record local time not @ REQUEST but @ DISCOVER (RFC1541) */
   if(sendto(Ssend, (char *)&DhcpMsgSend, sizeof(DhcpMsgSend), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "sendto (init)");
      return(EXIT);
   }
   return(SELECTING);
}

///
/// rebooting
int rebooting(struct Library *SocketBase, struct Interface *iface, struct Interface_Data *iface_data)
{
   int                nextState;
   struct sockaddr_in    addr;
   u_int           waitMsgType;

   setSockAddrIn(htons(DHCP_SERVER_PORT), htonl(INADDR_BROADCAST), &addr);
   waitMsgType        = 0;
   setWaitMsgType(DHCP_ACK, &waitMsgType);
   setWaitMsgType(DHCP_NAK, &waitMsgType);

   /* DhcpMsgSend should contain DHCPREQUEST message (created in initReboot). */
   if(!waitChkReXmitMsg(SocketBase, Srecv, &DhcpMsgRecv, Ssend, &DhcpMsgSend, &addr, waitMsgType, OptPtr, N_REXMIT_REQ_REBOOT))
   {
      Printf("REBOOTING: timeout. Fall back to INIT\n");
      return(INIT);
   }
   nextState = setDhcpInfo(OptPtr, &DhcpMsgRecv);
   if(nextState == BOUND)
   {
      /* check if yiaddr is already used */
      if(!arpCheck(SocketBase, DhcpMsgRecv.yiaddr, iface_data, ARP_REPLY_TIMEOUT))
      {
         if(!sendDhcpDecline(SocketBase, DHCP_DECLINE, *((u_int *)(OptPtr[OserverInaddr]+1)), DhcpMsgRecv.yiaddr))
            return(EXIT);
         Delay(10);
         Printf("REBOOTING: %ls is already used. Fall back to INIT\n", Inet_NtoA(DhcpMsgRecv.yiaddr));
         return(INIT);
      }
      /* now the client is initialized */

      /* initHost uses the values of iface_data->ifd_dst and iface_data->ifd_netmask as is
       * under the following condition:
       *   1. the DHCP ACK msg does not includes bcast or subnetmask option
       *   2. iface_data->ifd_dst or iface_data->ifd_netmask is not 0.
       * So we have to set these values to 0 not to confuse initHost.
       */
      iface_data->ifd_dst = iface_data->ifd_netmask = 0;
      if(!initHost(SocketBase, iface_data, DhcpMsgRecv.yiaddr))
         return(EXIT);
      execCommandFile();
      if(LeaseTime == INFINITE_LEASE_TIME)
      {
         Printf("got the INFINITE lease time. exit(0).\n");
         return(EXIT);
      }
   }
   else if(nextState == INIT)
      syslog_AmiTCP(SocketBase, LOG_NOTICE, "REBOOTING: got DHCPNAK. Fall back to INIT");

   return(nextState);
}

///
/// selecting
int selecting(struct Library *SocketBase, struct Interface *iface, struct Interface_Data *iface_data)
{
   time_t             prevTime;
   struct sockaddr_in    addr;
   u_int           waitMsgType;

   time(&prevTime);

   setSockAddrIn(htons(DHCP_SERVER_PORT), htonl(INADDR_BROADCAST), &addr);

   /* DhcpMsgSend shoud contain DHCPDISCOVER message (created in 'init') */
   waitMsgType        = 0;
   setWaitMsgType(DHCP_OFFER, &waitMsgType);
   if(waitChkReXmitMsg(SocketBase, Srecv, &DhcpMsgRecv, Ssend, &DhcpMsgSend, &addr, waitMsgType, OptPtr, N_REXMIT_DISCOVER))
   {
      /* send DHCPREQUEST msessage */
      ServerInaddr = *((u_int *)(OptPtr[OserverInaddr]+1));
      LeaseTime    = *((u_int *)(OptPtr[OleaseTime]+1));
      mkDhcpRequestMsg(SELECTING, ServerInaddr, LeaseTime, DhcpMsgRecv.xid, DhcpMsgRecv.yiaddr, &DhcpMsgSend);
#ifdef NEED_BCAST_RESPONSE
      DhcpMsgSend.flags = htons(F_BROADCAST);
#endif
      if(sendto(Ssend, (char *)&DhcpMsgSend, sizeof(DhcpMsgSend), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, "sendto (selecting)");
         return(EXIT);
      }
      /* record option values in the DHCPOFFER message because NT server
         does not include those values in the DHCPACK message
       */
      setupIfInfo(iface_data, DhcpMsgRecv.yiaddr, OptPtr);
      setupOptInfo(OptOffer, (const u_char **)OptPtr);
      return(REQUESTING);
   }
   ifDown(SocketBase, iface_data);
   Printf("no DHCPOFFER messages\n");
   return EXCEPTION;       /* should not happen */
}

///
/// requesting
int requesting(struct Library *SocketBase, struct Interface *iface, struct Interface_Data *iface_data)
{
   struct sockaddr_in    addr;
   int                nextState;
   u_int           waitMsgType;

   /* wait DHCPACK/DHCPNAK, rexmit RHCPREQUEST if necessary */
   setSockAddrIn(htons(DHCP_SERVER_PORT), htonl(INADDR_BROADCAST), &addr);
   waitMsgType = 0;
   setWaitMsgType(DHCP_ACK, &waitMsgType);
   setWaitMsgType(DHCP_NAK, &waitMsgType);
   if(waitChkReXmitMsg(SocketBase, Srecv, &DhcpMsgRecv, Ssend, &DhcpMsgSend, &addr, waitMsgType, OptPtr, N_REXMIT_REQUEST))
   {
      if((nextState = setDhcpInfo(OptPtr, &DhcpMsgRecv)) != EXCEPTION)
      {
         if(nextState == BOUND)
         {
            /* check if yiaddr is already used */
            iface_data->ifd_addr = htonl(0);  /* sender's IP address must be 0 */
            if(!arpCheck(SocketBase, DhcpMsgRecv.yiaddr, iface_data, ARP_REPLY_TIMEOUT))
            {
               if(!sendDhcpDecline(SocketBase, DHCP_DECLINE, *((u_int *)(OptPtr[OserverInaddr]+1)), DhcpMsgRecv.yiaddr))
                  return(EXIT);
               Printf("REQUESTING: %ls is already used. Fall back to INIT\n", Inet_NtoA(DhcpMsgRecv.yiaddr));
               Delay(10);
               return(INIT);
            }
            /* now the client is initialized */
            setupOptInfo(OptOffer, (const u_char **)OptPtr);
            if(!initHost(SocketBase, iface_data, DhcpMsgRecv.yiaddr))
               return(EXIT);
            sendArpReply(SocketBase, MAC_BCAST_ADDR, iface_data->ifd_dst, iface_data);
            if(LeaseTime == INFINITE_LEASE_TIME)
            {
               Printf("got the INFINITE lease time. exit(0).\n");
               return(EXIT);
            }
         }
         return(nextState);
      }
   }
   Printf("no response to DHCPREEQUEST message. move to INIT state\n");
   return(INIT);
}

///
// bound
int bound(struct Library *SocketBase, struct Interface *iface, struct Interface_Data *iface_data)
{
   Printf("got in BOUND state\n");
   send_dhcp_msg(FALSE, TRUE, NULL);
   Delay(ntohl(RenewTime) * 50);
   return(RENEWING);
}

//
/// renewing
int renewing(struct Library *SocketBase, struct Interface *iface, struct Interface_Data *iface_data)
{
   struct sockaddr_in    addr;
   int                nextState;
   time_t             sendTime;
   time_t             prevTime;
   long            timeout;
   long            tm;
   u_int           waitMsgType;

   Printf("got in RENEWING state\n");

   /* setup for sending unicast DHCPREQUEST message */
   setSockAddrIn(htons(DHCP_SERVER_PORT), ServerInaddr, &addr);
   mkDhcpRequestMsg(RENEWING, ServerInaddr, LeaseTime, rand(), iface_data->ifd_addr, &DhcpMsgSend);
   /* send DHCPREQUESTvia unicast, and
    * wait server's response (DHCPACK/DHCPNAK)
    */
   timeout = ReqSentTime + ntohl(RebindTime) - time(NULL);
   /* timeout     = ntohl(RebindTime) - ntohl(RenewTime); */
   nextState   = REBINDING;
   waitMsgType = 0;
   setWaitMsgType(DHCP_ACK, &waitMsgType);   /* wait DHCPACK, DHCPNAK */
   setWaitMsgType(DHCP_NAK, &waitMsgType);
   tm = getNextTimeout(INIT_TIMEOUT);
   time(&prevTime);
   while(timeout > 0)
   {
      /* send DHCPREQUEST via unicast */
      time(&sendTime);
      if(sendto(Ssend, (char *)&DhcpMsgSend, sizeof(DhcpMsgSend), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
      {
         syslog_AmiTCP(SocketBase, LOG_WARNING, "sendto (renewing)");
         Delay(tm/1000000);
      }
      else
      {
         /* wait server's response */
         if(rcvAndCheckDhcpMsg(SocketBase, Srecv, &DhcpMsgRecv, waitMsgType, OptPtr, tm))
         {
            nextState = setDhcpInfo(OptPtr, &DhcpMsgRecv);
            if((nextState == EXCEPTION) || (nextState == EXIT))
               return(nextState);
            break;
         }
      }
      tm = getNextTimeout(NEXT_TIMEOUT);  /* renew response timeout value */
      timeout -= time(NULL) - prevTime;
      time(&prevTime);
   }
   if(nextState == BOUND)
   {
      ReqSentTime = sendTime;       /* renew time when DHCPREQ is sent */
   }
   else if(nextState == INIT)   /* got DHCPNAK */
   {
      Printf("RENEWING: got DHCPNAK. Fall back to INIT\n");
      CloseSocket(Ssend);
      CloseSocket(Srecv);
      Ssend = Srecv = -1;
      ifDown(SocketBase, iface_data);
   }
   return(nextState);
}

///
/// rebinding
int rebinding(struct Library *SocketBase, struct Interface *iface, struct Interface_Data *iface_data)
{
   struct sockaddr_in    addr;
   int                nextState;
   time_t             sendTime;
   time_t             prevTime;
   long            timeout;
   long            tm;
   u_int           waitMsgType;

   Printf("got in REBINDING state\n");

   nextState = INIT;       /* init nextState */

   /* setup for sending broadcast DHCPREQUEST message */
   setSockAddrIn(htons(DHCP_SERVER_PORT), iface_data->ifd_dst, &addr);
   mkDhcpRequestMsg(REBINDING, ServerInaddr, LeaseTime, rand(), iface_data->ifd_addr, &DhcpMsgSend);

   /* send DHCPREQUEST via broadcast, and
    * wait server's response (DHCPACK/DHCPNAK)
    */
   timeout = ReqSentTime + ntohl(LeaseTime) - time(NULL);
   /* timeout     = ntohl(LeaseTime) - ntohl(RebindTime); */
   waitMsgType = 0;
   setWaitMsgType(DHCP_ACK, &waitMsgType);   /* wait DHCPACK, DHCPNAK */
   setWaitMsgType(DHCP_NAK, &waitMsgType);
   tm = getNextTimeout(INIT_TIMEOUT);
   time(&prevTime);
   while(timeout > 0)
   {
      time(&sendTime);
      if(sendto(Ssend, (char *)&DhcpMsgSend, sizeof(DhcpMsgSend), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
      {
         syslog_AmiTCP(SocketBase, LOG_WARNING, "sendto (rebinding)");
         Delay(tm/1000000);
      }
      else
      {
         if(rcvAndCheckDhcpMsg(SocketBase, Srecv, &DhcpMsgRecv, waitMsgType, OptPtr, tm))
         {
            nextState = setDhcpInfo(OptPtr, &DhcpMsgRecv);
            if((nextState == EXCEPTION) || nextState == EXIT)
               return(nextState);
            break;
         }
      }
      tm = getNextTimeout(NEXT_TIMEOUT);
      timeout -= time(NULL) - prevTime;
      time(&prevTime);
   }
   if(nextState == BOUND)
   {
      ReqSentTime = sendTime; /* renew time when DHCPREQ is sent */
      return(nextState);
   }
   /* Lease expired. halt network */
   Printf("REBINDING: Lease time expired. Fall back to INIT\n");
   CloseSocket(Ssend);
   CloseSocket(Srecv);
   Ssend = Srecv = -1;
   ifDown(SocketBase, iface_data);
   return(INIT);
}

///

/// mkDhcpDiscoverMsg
void mkDhcpDiscoverMsg(u_char *haddr, dhcpMessage *msg)
{
   u_char *p = msg->options + 4; /* just after the magic cookie */

   bzero((char *)msg, sizeof(*msg));
   msg->htype = HTYPE_ETHER;  /* supports Etherenet only */
   msg->hlen  = 6;
   bcopy(haddr, msg->chaddr, 6);
   msg->op     = BOOTREQUEST;
#ifdef NEED_BCAST_RESPONSE
   msg->flags = htons(F_BROADCAST);
#endif
   msg->xid   = htonl(rand());

   /* make DHCP message option field */
   *((u_long *)msg->options) = htonl(MAGIC_COOKIE);
   *p++ = dhcpMessageType;    /* DHCP message type */
   *p++ = 1;
   *p++ = DHCP_DISCOVER;
   *p++ = dhcpMaxMsgSize;     /* Maximum DHCP message size */
   *p++ = 2;
   *((u_short *)p) = htons(sizeof(*msg));
   p += sizeof(u_short);
   if(SuggestLeaseTime)
   {
      *p++ = dhcpIPaddrLeaseTime;   /* IP address lease time */
      *p++ = 4;
      *((u_int *)p) = htonl(SuggestLeaseTime);
      p += sizeof(long);
   }
   *p++ = dhcpParamRequest;   /* Parameter Request List */
   *p++ = 8;               /* number of requests */
   *p++ = subnetMask;
   *p++ = routersOnSubnet;
   *p++ = dns;
   *p++ = hostName;
   *p++ = domainName;
   *p++ = broadcastAddr;
   *p++ = ntpServers;
   *p++ = nisDomainName;

   if(Hostname != NULL)
   {
      int len;

      len = strlen(Hostname);
      *p++ = hostName;
      *p++ = len;
      strncpy(p, Hostname, len);
      p += len;
   }
   *p++ = dhcpClassIdentifier;   /* class identifier */
   *p++ = strlen(ClassId);
   strcpy(p, ClassId);
   p += strlen(ClassId);
   bcopy(ClientId, p, ClientId[1]+2); /* client identifier */
   p += ClientId[1] + 2;
   *p = endOption;            /* end */
}

///
/// mkDhcpRequestMsg
void mkDhcpRequestMsg(int flag, u_long serverInaddr, u_long leaseTime, u_long xid, u_long ciaddr, dhcpMessage *msg)
{
   u_char *p = msg->options + 4; /* just after the magic cookie */

   msg->xid = xid;
   msg->ciaddr = ciaddr;
   msg->flags = htons(0);     /* do not set the broadcast flag here */
   bzero((char *)p, sizeof(msg->options) - 4);  /* clear DHCP option field */

   /* 1. Requested IP address must not be in the DHCPREQUEST message
    *    under the RFC1541 mode.
    * 2. Requested IP address must be in the DHCPREQUEST message
    *    sent in the SELECTING or INIT-REBOOT state under the Internet
    *    Draft mode
    */
   if(!BeRFC1541)
   {
      if(flag == REBOOTING || flag == SELECTING)
      {
         /* ciaddr must be 0 in REBOOTING & SELECTING */
         msg->ciaddr = htonl(0);
         *p++ = dhcpRequestedIPaddr;
         *p++ = 4;
         *((u_int *)p) = ciaddr;
         p += sizeof(u_int);
      }
   }
   *p++ = dhcpMessageType;          /* DHCP message type */
   *p++ = 1;
   *p++ = DHCP_REQUEST;
   *p++ = dhcpMaxMsgSize;           /* Maximum DHCP message size */
   *p++ = 2;
   *((u_short *)p) = htons(sizeof(*msg));
   p += sizeof(u_short);
   if(flag == REBOOTING && SuggestLeaseTime )
   {
      *p++ = dhcpIPaddrLeaseTime;   /* IP address lease time */
      *p++ = 4;
      *((u_int *)p) = htonl(SuggestLeaseTime);
      p += sizeof(long);
   }
   if(flag == SELECTING || flag == REQUESTING)
   {
      *p++ = dhcpServerIdentifier;  /* server identifier */
      *p++ = 4;
      *((u_int *)p) = serverInaddr;
      p += sizeof(u_int);
   }
   if(leaseTime != 0)
   {
      *p++ = dhcpIPaddrLeaseTime;      /* IP address lease time */
      *p++ = 4;
      *((u_int *)p) = leaseTime;
      p += sizeof(u_int);
   }
   *p++ = dhcpParamRequest;   /* Parameter Request List */
   *p++ = 8;               /* number of requests */
   *p++ = subnetMask;
   *p++ = routersOnSubnet;
   *p++ = dns;
   *p++ = hostName;
   *p++ = domainName;
   *p++ = broadcastAddr;
   *p++ = ntpServers;
   *p++ = nisDomainName;

   if(Hostname != NULL)
   {
      int len;

      len = strlen(Hostname);
      *p++ = hostName;
      *p++ = len;
      strncpy(p, Hostname, len);
      p += len;
   }
   *p++ = dhcpClassIdentifier;         /* class identifier */
   *p++ = strlen(ClassId);
   strcpy(p, ClassId);
   p += strlen(ClassId);
   bcopy(ClientId, p, ClientId[1]+2);  /* client identifier */
   p += ClientId[1] + 2;
   *p = endOption;                  /* end */
}

///
/// mkDhcpDeclineMsg
void mkDhcpDeclineMsg(int flag, u_long serverInaddr, u_long ciaddr, dhcpMessage *msg)
{
   u_char *p = msg->options + 4; /* just after the magic cookie */

   msg->xid = rand();
   if(flag == DHCP_RELEASE) /* make a DHCPRELEASE msg */
      msg->ciaddr = ciaddr;
   else
   {                        /* make a DHCPDECLINE msg */
      if(BeRFC1541)         /* use ciaddr in RFC1541 compliant mode */
         msg->ciaddr = ciaddr;
      else
         msg->ciaddr = 0;
   }

   bzero((char *)p, sizeof(msg->options) - 4);
   *p++ = dhcpMessageType;    /* DHCP message type */
   *p++ = 1;
   *p++ = (u_char)flag;
   *p++ = dhcpServerIdentifier; /* server identifier */
   *p++ = 4;
   *((u_int *)p) = serverInaddr;
   p += sizeof(long);
   if(flag == DHCP_DECLINE && !BeRFC1541)
   {
      /* use the requested IP address option
       * in the Internet Draft compliant mode
       */
      *p++ = dhcpRequestedIPaddr;
      *p++ = 4;
      *((u_int *)p) = ciaddr;
      p += sizeof(u_int);
   }
   if(Hostname != NULL)
   {
      int len;

      len = strlen(Hostname);
      *p++ = hostName;
      *p++ = len;
      strncpy(p, Hostname, len);
      p += len;
   }
   bcopy(ClientId, p, ClientId[1]+2); /* client identifier */
   p += ClientId[1] + 2;
   *p = endOption;            /* end */
}

///
/// sendDhcpDecline
BOOL sendDhcpDecline(struct Library *SocketBase, int flag, u_long serverInaddr, u_long ciaddr)
{
   struct sockaddr_in    addr;

   bzero((char *)&addr, sizeof(addr));
   addr.sin_family    = AF_INET;
   addr.sin_port      = htons(DHCP_SERVER_PORT);
   switch(flag)
   {
      case DHCP_DECLINE:
         addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
         break;
      case DHCP_RELEASE:
         addr.sin_addr.s_addr = serverInaddr;
         break;
      default:
         Printf("illegal flag value (sendDhcpDecline)\n");
         break;
   }
   mkDhcpDeclineMsg(flag, serverInaddr, ciaddr, &DhcpMsgSend);
   if(sendto(Ssend, (char *)&DhcpMsgSend, sizeof(DhcpMsgSend), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "sendto (sendDhcpDecline)");
      return(FALSE);
   }
   return(TRUE);
}

///

/// setDhcpInfo
int setDhcpInfo(u_char *optp[], dhcpMessage *msg)
{
   switch(*(optp[OmsgType]+1))
   {
      case DHCP_NAK:
         return(INIT);
      case DHCP_ACK:
         ServerInaddr = *((u_long *)(optp[OserverInaddr]+1));
         LeaseTime    = *((u_long *)(optp[OleaseTime]+1));
         if(optp[OrenewalTime] == NULL)
            RenewTime = htonl(ntohl(LeaseTime) / 2);     /* default T1 time */
         else
            RenewTime = *((u_long *)(optp[OrenewalTime]+1));
         if(optp[OrebindTime] == NULL)
            RebindTime = htonl(ntohl(LeaseTime) / 8 * 7);   /* default T2 time */
         else
            RebindTime = *((u_long *)(optp[OrebindTime]+1));
         return(BOUND);
      default:
         return(EXCEPTION);    /* should not happen */
   }
}

///
/// setupIfInfo
/* NOTE: this function is called from 'selecting()', and sets up the network
 *       interface information. It sets subnetmask and broadcast to 0 unless
 *       it receives these values. These values are set correctly in
 *       'initHost()'. This is for the following case: these values were in
 *       the DHCPOFFER message, and were not in the DHCPACK message. In this
 *       case, dhcpcd must not override these values with the 'natural'
 *       subenetmask and broadcast.
 */
void setupIfInfo(struct Interface_Data *iface_data, u_long yiaddr, u_char *optp[])
{
   iface_data->ifd_addr = yiaddr;
   if(optp[Onetmask] != NULL)
      iface_data->ifd_netmask = *((u_int *)(optp[Onetmask]+1));
   else
      iface_data->ifd_netmask = 0;

   if(optp[ObcastInaddr] != NULL)
      iface_data->ifd_dst = *((u_int *)(optp[ObcastInaddr]+1));
   else
      iface_data->ifd_dst = 0;
}

///
/// initHost
BOOL initHost(struct Library *SocketBase, struct Interface_Data *iface_data, u_long yiaddr)
{
   BOOL success = FALSE;

   /* configure interface */
   iface_data->ifd_addr  = yiaddr;
   Printf("assigned IP address:        %ls\n", Inet_NtoA(iface_data->ifd_addr));
   if(OptPtr[Onetmask] == NULL)
   {
      if(iface_data->ifd_netmask == 0)
      {
         /* if iface_data->ifd_netmask != 0, subnetmask info. was included in the
          *  DHCPOFFER message and not included in the DHCPACK message.
          *  In this case, subnetmask value must not been overwritten.
          */
         iface_data->ifd_netmask = getNaturalMask(SocketBase, iface_data->ifd_addr);
      }
   }
   else
   {
      iface_data->ifd_netmask  = *((u_int *)(OptPtr[Onetmask]+1));
      Printf("assigned subnetmask:        %ls\n", Inet_NtoA(iface_data->ifd_netmask));
   }
   if(OptPtr[ObcastInaddr] == NULL)
   {
      /* if the server responds only subnetmask, I should presume
       * the broadcast address from the subnetmask instead of
       * using the 'natural' broadcast address.
       */
      if(iface_data->ifd_dst == 0)
      {
         /* if iface_data->ifd_dst != 0, broadcast info. was included in the
          *  DHCPOFFER message and not included in the DHCPACK message.
          *  In this case, broadcast addr. must not been overwritten.
          */
         iface_data->ifd_dst = (iface_data->ifd_addr & iface_data->ifd_netmask) | ~iface_data->ifd_netmask;
      }
   }
   else
   {
      iface_data->ifd_dst = *((u_int *)(OptPtr[ObcastInaddr]+1));
      Printf("assigned broadcast address: %ls\n", Inet_NtoA(iface_data->ifd_dst));
   }
   if(ifConfig(SocketBase, iface_data))
   {
      saveIfInfo(SocketBase, iface_data);         /* save interface information onto file */
      saveHostInfo(SocketBase, (const u_char **)OptPtr, iface_data);
      success = TRUE;
   }
   return(success);
}

///
/// getNextTimeout
long getNextTimeout(int flag)
{
   static long  prevTimeout;
   long      rv;        /* return value */

   if(flag == INIT_TIMEOUT)
      prevTimeout = 4;     /* 4 seconds */
   else if(prevTimeout > 64)
      prevTimeout = 4;

   rv = (prevTimeout - 1) * 1000000 + rand()/2000000;
   prevTimeout *= 2;
   return(rv);
}

///
