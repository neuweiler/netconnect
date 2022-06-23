#ifndef NETINET_UDP_H
#define NETINET_UDP_H \
       "$Id: udp.h,v 1.1.1.1 1999/08/08 14:20:33 zapek Exp $"
/*
 *      UDP protocol header, per RFC 768, September, 1981.
 *
 *      Copyright © 1994 AmiTCP/IP Group,
 *                       Network Solutions Development, Inc.
 *                       All rights reserved.
 */

struct udphdr {
	u_short	uh_sport;		/* source port */
	u_short	uh_dport;		/* destination port */
	short	uh_ulen;		/* udp length */
	u_short	uh_sum;			/* udp checksum */
};

#endif /* !NETINET_UDP_H */
