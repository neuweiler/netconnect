/*
 *      $Id: iface_data.h,v 1.6 1996/02/14 21:46:09 jraja Exp $
 *
 *      iface_data.h -
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

struct Interface_Data {
/* public: */
  union {
    char              ifd_u_name[IFNAMSIZ];   /* name is first on ifr */
    struct ifreq      ifd_u_ifr;     /* ifreq for ioctl's */
    struct ifaliasreq ifd_u_ifra;    /* req for SIOCAIFADDR */
  } ifd_u;
#define ifd_name  ifd_u.ifd_u_name
#define ifd_ifr   ifd_u.ifd_u_ifr
#define ifd_ifra  ifd_u.ifd_u_ifra
  u_long ifd_sanatype;   /* Sana type of the interface */
  u_char ifd_htype;      /* hw type of the interface */
  u_char ifd_hlen;    /* length of the hw address (in bytes) */
  u_char ifd_haddr[16];  /* the hw address itself */
  short  ifd_flags;      /* interface flags */
  u_long ifd_addr; /* our configured address */
  u_long ifd_dst; /* destination (or broadcast) address */
  u_long ifd_netmask; /* destination (or broadcast) address */
  u_long ifd_gateway;
  u_long ifd_MTU; /* interface maximum transfer unit size */
/* private: */
  int ifd_fd;         /* socket descriptor used for ioctl's */
/* optional sana2 info */
  BOOL   ifd_use_hwtype;   /* use hardware type on BOOTP? */
  struct sana2 *            ifd_s2;
};

struct Interface_Data *iface_alloc(void);
void iface_free(struct Interface_Data *iface_data);
BOOL iface_init(struct Interface_Data *iface_data, struct Interface *iface, struct ISP *isp, struct Config *conf);
void iface_deinit(struct Interface_Data *iface_data);

void iface_close_sana2(struct Interface_Data *iface_data);

BOOL iface_prepare_bootp(struct Interface_Data *iface_data, struct Interface *iface, struct Config *conf);
void  iface_cleanup_bootp(struct Interface_Data *iface_data, struct Config *conf);

BOOL iface_online(struct Interface_Data *iface_data);
BOOL    iface_offline(struct Interface_Data * iface_data);

BOOL    iface_runscript(struct Interface_Data * iface_data, struct Interface *iface, struct ISP *isp, struct Config *conf);
BOOL    iface_config(struct Interface_Data *iface_data, struct Interface *iface, struct Config *conf);

void iface_print(struct Interface_Data *iface_data, const char *caption);

int route_add(int fd, short flags, u_long from, u_long to, BOOL    force);
int route_delete(int fd, u_long from);

int iface_getaddr(struct Interface_Data *iface_data, u_long *addr);
int iface_setaddr(struct Interface_Data *iface_data, u_long addr);
int iface_getdstaddr(struct Interface_Data *iface_data, u_long *addr);
int iface_setdstaddr(struct Interface_Data *iface_data, u_long addr);
int iface_getnetmask(struct Interface_Data *iface_data, u_long *addr);
int iface_setnetmask(struct Interface_Data *iface_data, u_long addr);
int iface_getflags(struct Interface_Data *iface_data, short *flags);
int iface_setflags(struct Interface_Data *iface_data, short flags);
int iface_getmtu(struct Interface_Data *iface_data, int *mtu);
int iface_setmtu(struct Interface_Data *iface_data, int mtu);

int iface_getlinkinfo(struct Interface_Data *iface_data);

char * my_link_ntoa(u_char len, u_char *addr);

int iface_findprimary(struct Interface_Data *iface_data);
