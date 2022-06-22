#ifndef BOOTPC_H
#define BOOTPC_H

#ifndef BOOTP_H
#include "bootp.h"
#endif

#define BPC_BUFLEN   1024

/* The wait time is increased exponentially up to BPC_MAXWAIT */
#define BPC_WAITSECS 2  /* Initial waiting time */
#define BPC_MAXWAIT  64 /* maximum waiting time */
#define BPC_MAXRETRIES  10 /* maximum number of BOOTP retries */

struct bootpc {

  u_short bpc_s_port;      /* server port number */
  u_short bpc_c_port;      /* client port number */

  u_long bpc_s_addr;    /* server address */
  
  struct bootp * bpc_snd_packet; /* packet to send (BPC_BUFLEN bytes) */
  u_long bpc_snd_len;      /* actual length of the send packet */
  struct bootp * bpc_rcv_packet; /* received packet (BPC_BUFLEN bytes) */
  u_long bpc_rcv_len;      /* actual length of the received packet */
};

struct bootpc *bootpc_create(void);
void bootpc_delete(struct bootpc *);
int bootpc_do(struct bootpc *bpc, struct iface *ifc, struct config *conf);
int bootpc_send(struct bootpc *bpc, int fd, struct sockaddr_in * to);
int bootpc_examine(struct bootpc *bpc, struct iface *ifc, struct config *conf);

#endif /* !BOOTPC_H */

