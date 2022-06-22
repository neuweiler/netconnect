/*
 *      $Id: iface.h,v 1.6 1996/02/14 21:46:09 jraja Exp $
 *
 *      iface.h - 
 *
 *      Copyright © 1994 AmiTCP/IP Group, 
 *                       Network Solutions Development Inc.
 *                       All rights reserved.
 */

#ifndef NETINET_IN_H
#include <netinet/in.h>
#endif

#ifndef NET_IF_H
#include <net/if.h>
#endif

struct iface {
/* public: */
  union {
    char              iface_u_name[IFNAMSIZ];   /* name is first on ifr */
    struct ifreq      iface_u_ifr;     /* ifreq for ioctl's */
    struct ifaliasreq iface_u_ifra;    /* req for SIOCAIFADDR */
  } iface_u;
#define iface_name  iface_u.iface_u_name
#define iface_ifr   iface_u.iface_u_ifr
#define iface_ifra  iface_u.iface_u_ifra
  u_long iface_sanatype;   /* Sana type of the interface */
  u_char iface_htype;      /* hw type of the interface */
  u_char iface_hlen;    /* length of the hw address (in bytes) */
  u_char iface_haddr[16];  /* the hw address itself */
  short  iface_flags;      /* interface flags */
  u_long iface_addr; /* our configured address */
  u_long iface_dst; /* destination (or broadcast) address */
  u_long iface_netmask; /* destination (or broadcast) address */
  u_long iface_MTU; /* interface maximum transfer unit size */
/* private: */
  int iface_fd;         /* socket descriptor used for ioctl's */
/* optional sana2 info */
  struct sana2 *            iface_s2;
};

struct iface *iface_alloc(void);
void iface_free(struct iface *iface);
BOOL iface_init(struct iface *iface, struct config *conf);
void iface_deinit(struct iface *iface);

void iface_close_sana2(struct iface *iface);

BOOL iface_prepare_bootp(struct iface *iface, struct config *conf);
void  iface_cleanup_bootp(struct iface *iface, struct config *conf);

BOOL iface_online(struct iface * iface);
BOOL    iface_offline(struct iface * iface);

BOOL    iface_runscript(struct iface * iface, struct config *conf);

BOOL    iface_config(struct iface *iface, struct config *conf);

void iface_print(struct iface *iface, const char *caption);

int route_add(int fd, short flags, u_long from, u_long to, BOOL    force);
int route_delete(int fd, u_long from);

int iface_getaddr(struct iface *iface, u_long *addr);
int iface_setaddr(struct iface *iface, u_long addr);
int iface_getdstaddr(struct iface *iface, u_long *addr);
int iface_setdstaddr(struct iface *iface, u_long addr);
int iface_getnetmask(struct iface *iface, u_long *addr);
int iface_setnetmask(struct iface *iface, u_long addr);
int iface_getflags(struct iface *iface, short *flags);
int iface_setflags(struct iface *iface, short flags);
int iface_getmtu(struct iface *iface, int *mtu);
int iface_setmtu(struct iface *iface, int mtu);

int iface_getlinkinfo(struct iface *iface);

char * my_link_ntoa(u_char len, u_char *addr);

int iface_findprimary(struct iface *iface);
