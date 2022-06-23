/// includes
#include <sys/types.h>
#include <string.h>
#include "dhcp.h"
#include "dhcp-options.h"
#include "error-handler.h"

///
/// variables

/* length of each option. -1 means 'n'. */
static char OptLen[256] = {
    0,                     /* 0: pad */
    4,
    4,
   -1,
   -1,
   -1,                     /* 5: Name Server */
   -1,
   -1,
   -1,
   -1,
   -1,                     /* 10: Impress Server */
   -1,
   -1,
    2,
   -1,
   -1,                     /* 15: Domain Name */
   -1,
   -1,
   -1,
    1,
    1,                  /* 20: Non-Local Source Routing Enable/Disable */
   -1,
    2,
    1,
    4,
   -1,                     /* 25: Path MTU Plateau Table */
    2,
    1,
    4,
    1,
    1,                     /* 30: Mask Supplier */
    1,
    4,
   -1,
    1,
    4,                     /* 35: ARP Cache Timeout */
    1,
    1,
    4,
    1,
   -1,                     /* 40: NIS Domain */
   -1,
   -1,
   -1,
   -1,
   -1,                     /* 45: NetBIOS over TCP/IP... */
    1,
   -1,
   -1,
   -1,
    4,                     /* 50: Requested IP address (DHCP) */
    4,
    1,
    1,
    4,
   -1,                     /* 55: Parameter Request List (DHCP) */
   -1,
    2,
    4,
    4,
   -1,                     /* 60: Class Identifier (DHCP) */
   -1,
    0,
};

///

/// getOptions
void getOptions(u_char *optp[], dhcpMessage *msg)
{
   u_char *p;
   u_char *end;
   int      len;
   int      i;

   /* opt must have 312 elements. */

   p  = msg->options + 4;     /* skip magic cookie */
   end = msg-> options + sizeof(msg->options);  /* last element + 1 */

   i = 0;
   bzero((u_char *)optp, sizeof(msg->options) * sizeof(u_char *));
   while(p < end)
   {
      if(*p == endOption) /* end */
         return;
      if(*p == padOption) /* pad */
      {
         ++p;
         continue;
      }
      /* get the length of this tag code */
      len = (OptLen[*p] == -1) ? p[1] : OptLen[*p];
      optp[i++] = p;       /* store the pointer to the tag */
      p += len + 2;        /* set the pointer to the next tag */
   }
   return;
}

///
/// parseDhcpMsg

/* optp must contain pointers to msg->option
 * return 1 if msg is good
 * return 0 if msg is not good
 * optp[Oxxx] has the pointer to the option field if msg is good
 * see dhcp.h for more details on Oxxx
 * CAUTION: This function uses DhcpMsgSend directly!!
 */
int parseDhcpMsg(u_char *optp[], dhcpMessage *msg)
{
   char    buf[512];
   u_char *opt[N_SUPPORT_OPTIONS];
   u_char **p = optp;

   if(msg->op != BOOTREPLY)
      return(0);            /* NG */
   if(msg->xid != DhcpMsgSend.xid)
      return(0);            /* NG */
   if(bcmp(msg->chaddr, DhcpMsgSend.chaddr, DhcpMsgSend.hlen))
      return(0);            /* NG */

   bzero((char *)opt, sizeof(opt));
   getOptions(p, msg);        /* get pointers to each option */
   for(; *p != NULL; ++p)
   {
      switch(**p)
      {
         case dhcpMessageType:
            opt[OmsgType] = *p + 1;
            break;
         case dhcpServerIdentifier:
            opt[OserverInaddr] = *p + 1;
            break;
         case dhcpIPaddrLeaseTime:
            opt[OleaseTime] = *p + 1;
            break;
         case dhcpT1value:
            opt[OrenewalTime] = *p + 1;
            break;
         case dhcpT2value:
            opt[OrebindTime] = *p + 1;
            break;
         case subnetMask:
            opt[Onetmask] = *p + 1;
            break;
         case broadcastAddr:
            opt[ObcastInaddr] = *p + 1;
            break;
         case timeServer:
            opt[OtimeServer] = ( (*p)[1]%4 ) ? NULL : *p + 1;
            break;
         case dns:
            opt[Odns] = ( (*p)[1]%4 ) ? NULL : *p + 1;
            break;
         case lprServer:
            opt[OlprServer] = ( (*p)[1]%4 ) ? NULL : *p + 1;
            break;
         case hostName:
            opt[OhostName] = *p + 1;
            break;
         case domainName:
            opt[OdomainName] = *p + 1;
            break;
         case nisDomainName:
            opt[OnisDomName] = *p + 1;
            break;
         case ntpServers:
            opt[OntpServer] = ( (*p)[1]%4 ) ? NULL : *p + 1;
            break;
         case dhcpMsg:
            strcpy(buf, "msg from the DHCP server: ");
            strncat(buf, *p + 2, (*p)[1]);
            Printf("%ls\n", buf);
            opt[OdhcpMessage] = *p + 1;
            break;
         case dhcpClassIdentifier:
            opt[OdhcpClassId] = ( (*p)[1]%4 ) ? NULL : *p + 1;
            break;
         case routersOnSubnet:
            opt[Orouter] = ( (*p)[1]%4 ) ? NULL : *p + 1;
            break;
         default:
            break;
      }
   }
   bcopy(opt, optp, sizeof(opt));

   /* check option field */
   if(optp[OmsgType] == NULL)  /* illegal DHCP msg, or BOOTP msg */
      return(0);

   /* DHCPOFFER & DHCPACK msgs must have server IP addres and lease time */
   switch(*optp[OmsgType])
   {
      case DHCP_OFFER:
         if(optp[OleaseTime] == NULL)
         {
            Printf("No leasetime (msgType: %ld) (parseDhcpMsg)\n", *optp[OmsgType]);
            return(0);
         }
      case DHCP_ACK:
         if(optp[OserverInaddr] == NULL)
         {
            Printf("No server IP address (msgType: %ld) (parseDhcpMsg)\n", *optp[OmsgType]);
            return(0);
         }
         break;
      default:
         break;
   }
   return(1);
}

///
