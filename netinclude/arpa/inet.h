#ifndef ARPA_INET_H
#define ARPA_INET_H \
       "$Id: inet.h,v 1.1.1.1 1999/08/08 14:20:36 zapek Exp $"
/*
 *      Inet Library Functions 
 *
 *      Copyright © 1994 AmiTCP/IP Group,
 *                       Network Solutions Development, Inc.
 *                       All rights reserved.
 */

#ifndef KERNEL
#ifndef NETINET_IN_H
#include <netinet/in.h>
#endif

/*
 * Include socket protos/inlines/pragmas
 */
#if !defined(BSDSOCKET_H) && !defined(CLIB_SOCKET_PROTOS_H)
#include <bsdsocket.h>
#endif

#endif /* !KERNEL */

#endif /* ARPA_INET_H */
