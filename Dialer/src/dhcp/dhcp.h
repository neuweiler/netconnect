/* $Id: dhcp.h,v 1.1.1.1 1999/08/08 14:20:12 zapek Exp $
 *
 * dhcpcd - DHCP client daemon -
 * Copyright (C) 1996 - 1997 Yoichi Hariguchi <yoichi@fore.com>
 *
 * dhcpcd is an RFC2131 and RFC1541 compliant DHCP client daemon.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _SYS_TYPES_H
#  include <sys/types.h>
#endif

#define HTYPE_ETHER  1        /* Hardware type (htype) value of Ethernet */
#define DHCP_CACHE_FILE "AmiTCP:db/dhcp/dhcpcd-cache."   /* DHCP cache file name */
//#define PIDFILE         "AmiTCP:db/dhcpcd-%s.pid"
#define HOST_INFO_DIR   "AmiTCP:db/dhcp"
#define HOST_INFO_FILE  "hostinfo"
#define MAGIC_COOKIE 0x63825363  /* magic cookie in the option field */
#define ARP_REPLY_TIMEOUT  15    /* timeout for arp reply msg in sec */
#define F_BROADCAST        0x8000   /* broadcast flag in 'flags' field */
#undef NEED_MACBCAST_RESPONSE    /* define this if MAC broadcast response
                            * is necessary in receiving DHCPOFFER,
                            * DHCPACK, and DHCPNAK message
                            */
enum {
   MAXNOPT =    312,    /* max number of DHCP options */
   MAXIDCHARS = 128,    /* max # of chars for class/client identifiers */
};

#define DEFAULT_LEASE_TIME   (180*60)    /* default lease time in second */
#define INFINITE_LEASE_TIME  0xffffffff  /* infinite lease time */

#define N_REXMIT_DISCOVER     4     /* # of rexmit of DHCPDISCOVER msg */
#define N_REXMIT_REQUEST      10
#define N_REXMIT_REQ_REBOOT   4



/* UDP port numbers for DHCP
 */
enum {
   DHCP_SERVER_PORT = 67,  /* from client to server */
   DHCP_CLIENT_PORT = 68   /* from server to client */
};

/* DHCP message OP code
 */
enum {
   BOOTREQUEST = 1,
   BOOTREPLY   = 2
};

/* DHCP message type
*/
enum {
   DHCP_DISCOVER =  1,
   DHCP_OFFER    =  2,
   DHCP_REQUEST  =  3,
   DHCP_DECLINE  =  4,
   DHCP_ACK      =  5,
   DHCP_NAK      =  6,
   DHCP_RELEASE  =  7
};

/* DHCP client states
 */
enum {
   MAX_STATES  = 8,        /* number of states */
   INIT_REBOOT = 0,
   INIT     = 1,
   REBOOTING   = 2,
   SELECTING   = 3,
   REQUESTING  = 4,
   BOUND    = 5,
   RENEWING = 6,
   REBINDING   = 7,
   EXCEPTION   = -1,
   EXIT        = -2
};

/* Timeout control
 */
enum {
   INIT_TIMEOUT = 0,
   NEXT_TIMEOUT = 1,
};


typedef struct dhcpMessage {
   u_char  op;             /* message type */
   u_char  htype;          /* hardware address type */
   u_char  hlen;           /* hardware address length */
   u_char  hops;           /* should be zero in client's message */
   u_int   xid;            /* transaction id */
   u_short secs;           /* elapssed time in sec. from trying to boot */
   u_short flags;
   u_long ciaddr;           /* (previously allocated) client IP address */
   u_long yiaddr;           /* 'your' client IP address */
   u_long siaddr;           /* should be zero in client's messages */
   u_long giaddr;           /* should be zero in client's messages */
   u_char   chaddr[16];       /* client's hardware address */
   u_char   sname[64];        /* server host name, null terminated string */
   u_char   file[128];        /* boot file name, null terminated string */
   u_char   options[MAXNOPT]; /* message options */
} dhcpMessage;

/* Option index in the DHCP message option field
 */
enum {
   OmsgType   =  0,        /* DHCP message type */
   OserverInaddr =  1,        /* DHCP server IP address */
   OleaseTime    =  2,        /* lease time */
   OrenewalTime  =  3,        /* renewal time */
   OrebindTime   =  4,        /* rebind time */
   Onetmask   =  5,        /* netmask */
   ObcastInaddr  =  6,        /* broadcast address */
   OdhcpMessage  =  7,        /* DHCP message */
   OdhcpClassId  =  8,        /* DHCP class identifier */
   OntpServer    =  9,        /* NTP server's IP address */
   OtimeServer   = 10,        /* time server's IP address */
   Odns       = 11,        /* Domain Name Server's IP address */
   OlprServer    = 12,        /* lpr server's IP address */
   OhostName     = 13,        /* hostname */
   OdomainName   = 14,        /* domainname */
   OnisDomName   = 15,        /* NIS domainname */
   Orouter       = 16,        /* routers on the client's subnet */

   N_SUPPORT_OPTIONS = 17     /* number of supported options */
};

/* global variables in client.c
 */
extern int (*Fsm[MAX_STATES])();/* finite state machine */
extern int CurrState;         /* current state */
extern int PrevState;         /* previous state */

extern dhcpMessage DhcpMsgSend;  /* DHCP message to send */
extern dhcpMessage DhcpMsgRecv;  /* DHCP message received */

extern int Ssend;          /* socket fd for send */
extern int Srecv;          /* socket fd for receive */

extern time_t ReqSentTime;    /* time when DHCPREQUEST message is sent */
extern u_long SuggestLeaseTime;  /* suggested lease time by user */
extern u_long LeaseTime;      /* lease time (network byte order) */
extern u_long RenewTime;      /* T1 time (network byte order) */
extern u_long RebindTime;     /* T2 time (network byte order) */

SAVEDS ASM VOID DHCPHandler(register __a0 STRPTR args, register __d0 LONG arg_len);
struct Interface *send_dhcp_msg(BOOL abort, BOOL success, struct Interface_Data **iface_data);

