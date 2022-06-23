/*
 *      $Id: sana.h,v 1.3 1996/02/04 15:53:03 jraja Exp $
 *
 *      sana.h - 
 *
 *      Copyright © 1994 AmiTCP/IP Group, 
 *                       Network Solutions Development Inc.
 *                       All rights reserved.
 */
#ifndef SANA_H
#define SANA_H

#ifndef DEVICES_SANA2_H
#include <devices/sana2.h>
#endif

#ifndef NET_SANA2ERRNO_H
#include <net/sana2errno.h>
#endif

struct sana2 {
/* Sana2 stuff */
  struct MsgPort    *s2_port; /* message port for Sana2 requests */
  struct IOSana2Req *s2_req;  /* A Sana2 request to use */
  long               s2_openerr; /* Error from OpenDevice() */
  char               s2_name[MAXPATHLEN]; /* name of the Sana2 device */
  unsigned long      s2_unit; /* Sana2 device unit number */
/* Sana2 device information */
  ULONG              s2_hwtype;  /* Sana2 hardware type */
  UWORD              s2_addrbits;/* size of the hw address in bits */
};

struct sana2 * sana2_create(STRPTR device, LONG unit);
void sana2_delete(struct sana2 *s2);
void sana2_print(struct sana2 *s2);
BOOL    sana2_getaddresses(struct Library *SocketBase, struct sana2 *s2, struct Interface *iface);
BOOL    sana2_online(struct Library *SocketBase, struct sana2 *s2);
BOOL    sana2_offline(struct Library *SocketBase, struct sana2 *s2);
BOOL    sana2_devicequery(struct sana2 *s2);

#endif /* !SANA_H */
