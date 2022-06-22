/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "bootpc.h"
#include "iface.h"
#include "protos.h"

///

/* RFC1048 vendor magic cookie */
static unsigned char vm_rfc1048[4] = VM_RFC1048;

extern int errno;

/// bootpc_create
struct bootpc *bootpc_create(void)
{
   struct bootpc *bpc;
   struct servent *sep;

   if(bpc = AllocVec(sizeof(*bpc) + 2 * BPC_BUFLEN, MEMF_ANY | MEMF_CLEAR))
   {
      bpc->bpc_snd_packet = (struct bootp *)(bpc + 1);
      bpc->bpc_rcv_packet = (struct bootp *)((char *)bpc->bpc_snd_packet + BPC_BUFLEN);

      if(sep = getservbyname("bootps", "udp"))
         bpc->bpc_s_port = ntohs((u_short) sep->s_port);
      else
         bpc->bpc_s_port = (u_short)IPPORT_BOOTPS;

      if(sep = getservbyname("bootpc", "udp"))
         bpc->bpc_c_port = ntohs(sep->s_port);
      else
         bpc->bpc_c_port = (u_short)IPPORT_BOOTPC;

      bpc->bpc_s_addr = INADDR_BROADCAST;
   }
   return(bpc);
}

///
/// bootpc_delete
void bootpc_delete(struct bootpc *bpc)
{
  FreeVec(bpc);
}

///
/// bootpc_examine
static char * opt_names[] =
{
  "PAD",
  "SUBNET_MASK",
  "TIME_OFFSET",
  "GATEWAY",
  "TIME_SERVER",
  "NAME_SERVER",
  "DOMAIN_SERVER",
  "LOG_SERVER",
  "COOKIE_SERVER",
  "LPR_SERVER",
  "IMPRESS_SERVER",
  "RLP_SERVER",
  "HOST_NAME",
  "BOOT_SIZE",
  "DUMP_FILE",
  "DOMAIN_NAME",
  "SWAP_SERVER",
  "ROOT_PATH",
  "EXTEN_FILE",
};

int bootpc_examine(struct bootpc *bpc, struct Interface_Data *iface_data, struct Interface *iface, struct ISP *isp)
{
   struct bootp *bp = bpc->bpc_rcv_packet;

   // Check our own IP
   if(iface_data->ifd_addr == INADDR_ANY && bp->bp_yiaddr.s_addr != INADDR_ANY)
   {
      iface_data->ifd_addr = bp->bp_yiaddr.s_addr;
      syslog_AmiTCP(LOG_DEBUG, "bootpc_examine: got IP address %ls from BOOTP reply.", Inet_NtoA(iface_data->ifd_addr));
   }

   // Check RFC options
   if(memcmp(bp->bp_vend, vm_rfc1048, sizeof(vm_rfc1048)) == 0)
   {
      u_char option;
      u_char opt_len;
      u_char *opt_ptr = bp->bp_vend + sizeof(vm_rfc1048);
      u_char *opt_end = (char *)bp + bpc->bpc_rcv_len;
      int    optcount = 0;
      int    tmplen;

      syslog_AmiTCP(LOG_DEBUG, "bootpc_examine: received valid BOOTP reply.");

      while(opt_ptr < opt_end - 1)  // have to have at least two bytes
      {
         if((option = *opt_ptr++) == BPTAG_END)
            break;
         if(option == TAG_PAD)
            continue;

         opt_len = *opt_ptr++;
         if(opt_ptr + opt_len > opt_end) // have space for the option?
            break;         // no

         if(option <= TAG_EXTEN_FILE)
            syslog_AmiTCP(LOG_DEBUG, "bootpc_examine: option: %ls, value: %ld", opt_names[option], (ULONG)*opt_ptr);
         else
            syslog_AmiTCP(LOG_DEBUG, "bootpc_examine: option: %ld, value: %ld", option, (ULONG)*opt_ptr);

         optcount++;

         if(option == TAG_SUBNET_MASK)
         {
            if(iface_data->ifd_netmask == INADDR_ANY && opt_len == 4)
               memcpy(&iface_data->ifd_netmask, opt_ptr, 4);
         }
         else if(option == TAG_GATEWAY)    // using only first gateway
         {
            if(iface_data->ifd_gateway == INADDR_ANY && opt_len >= 4)
               memcpy(&iface_data->ifd_gateway, opt_ptr, 4);
         }
         else if(option == TAG_DOMAIN_SERVER)
         {
            ULONG tmp_addr;

            for(tmplen = 0; tmplen + 4 <= opt_len; tmplen += 4)
            {
               memcpy(&tmp_addr, &opt_ptr[tmplen], 4);

               if(!find_server_by_name(&isp->isp_nameservers, Inet_NtoA(tmp_addr)))
                  add_server(&isp->isp_nameservers, Inet_NtoA(tmp_addr));
            }
         }
         else if(option == TAG_HOST_NAME)
         {
            tmplen = MIN(opt_len, sizeof(isp->isp_hostname)-1);
            memcpy(isp->isp_hostname, opt_ptr, tmplen);
            isp->isp_hostname[tmplen] = '\0';
         }
         else if(option == TAG_DOMAIN_NAME)
         {
            char buf[41];

            tmplen = MIN(opt_len, sizeof(buf) - 1);
            memcpy(buf, opt_ptr, tmplen);
            buf[tmplen] = '\0';

            if(!find_server_by_name(&isp->isp_nameservers, buf))
               add_server(&isp->isp_domainnames, buf);
         }

         opt_ptr += opt_len;
      }
   }

   // Additional test to get some working destination address
   if(iface_data->ifd_flags & IFF_POINTOPOINT && iface_data->ifd_dst == INADDR_ANY && iface_data->ifd_gateway == INADDR_ANY && bp->bp_siaddr.s_addr != INADDR_ANY)
   {
      iface_data->ifd_dst = bp->bp_siaddr.s_addr;
      syslog_AmiTCP(LOG_DEBUG, "bootpc_examine: using BOOTP server address %ls as destination addr.", Inet_NtoA(iface_data->ifd_dst));
   }

   return(0);
}

///
/// bootpc_send
int bootpc_send(struct bootpc *bpc, int fd, struct sockaddr_in *to)
{
   /* Send the request packet. */
   if(sendto(fd, (char *)bpc->bpc_snd_packet, bpc->bpc_snd_len, 0, (struct sockaddr *)to, sizeof(*to)) < 0)
   {
      if(errno == ENOBUFS)
      {
         /*
          * AmiTCP does not have any more free buffers. This is probably due to
          * not being able to send the datagrams at all, but they are not
          * dropped either.
          */
         syslog_AmiTCP(LOG_WARNING, "bootpc_send: network is unable to deliver packets (check connection).");
      }
      else
         syslog_AmiTCP(LOG_ERR, "bootpc_send: sendto");
      return(errno);
   }
   return(0);
}

///
/// bootpc_do
int bootpc_do(struct bootpc *bpc, struct Interface_Data *iface_data, struct Interface *iface, struct ISP *isp)
{
   int fd = iface_data->ifd_fd;
   short secs;
   int   retries;
   int32 xid;
   struct bootp *bp;
   int error, n;
   long fromlen;
   struct sockaddr_in sin_server; // where to send requests
   struct sockaddr_in sin_client; // for bind and listen
   struct sockaddr_in sin_from;      // Packet source

   // Set up server socket address (for send)
   bzero(&sin_server, sizeof(sin_server));
   sin_server.sin_len = sizeof(sin_server);;
   sin_server.sin_family = AF_INET;
   sin_server.sin_port = htons(bpc->bpc_s_port);
   sin_server.sin_addr.s_addr = bpc->bpc_s_addr;

   // Set up client socket address (for listen)
   bzero(&sin_client, sizeof(sin_client));
   sin_client.sin_len = sizeof(sin_client);;
   sin_client.sin_family = AF_INET;
   sin_client.sin_port = htons(bpc->bpc_c_port);
   sin_client.sin_addr.s_addr = INADDR_ANY;

   // Bind client socket to BOOTPC port.
   if(bind(fd, (struct sockaddr *)&sin_client, sizeof(sin_client)) < 0)
   {
      syslog_AmiTCP(LOG_ERR, "bind BOOTPC port: %m");
      if(errno == EACCES)
         syslog_AmiTCP(LOG_DEBUG, "bootpc_do: you need to run this as root");
      return(errno);
   }

   // Build a request.
   bp = bpc->bpc_snd_packet;
   bzero(bp, sizeof(*bp));
   bp->bp_op = BOOTREQUEST;
   xid = (int32)FindTask(NULL);
   bp->bp_xid = (u_int32)htonl(xid);

   // Fill in the hardware address, if appropriate (not for ppp, slip, etc)
   if(iface_data->ifd_use_hwtype)
   {
      syslog_AmiTCP(LOG_DEBUG, "bootpc_do: using hardware address in BOOTP request.");
      // Copy hardware address into request packet.
      bp->bp_htype   = iface_data->ifd_sanatype;
      bp->bp_hlen    = iface_data->ifd_hlen;
      memmove(bp->bp_chaddr, iface_data->ifd_haddr, bp->bp_hlen);
   }

  // Fill in the client IP address.
   if(iface_data->ifd_addr != INADDR_ANY)
   {
      syslog_AmiTCP(LOG_DEBUG, "bootpc_do: using IP addr %ls in BOOTP request.", Inet_NtoA(iface_data->ifd_addr));
      bp->bp_ciaddr.s_addr = iface_data->ifd_addr;
   }
  
   memcpy(bp->bp_vend, vm_rfc1048, sizeof(vm_rfc1048)); // copy default verdor data
   bp->bp_vend[4] = BPTAG_END;

   bpc->bpc_snd_len = sizeof(*bp); // total length of the request

   secs = 2;

   for(retries = 0; retries < 3; retries++)
   {
      struct timeval tv;
      int readfds;

      syslog_AmiTCP(LOG_DEBUG, "bootpc_do: sending BOOTP request to %ls.", Inet_NtoA(bpc->bpc_s_addr));
      bp->bp_secs = htons(secs);
      if(error = bootpc_send(bpc, fd, &sin_server))
         return(error);

      tv.tv_sec = 2;
      tv.tv_usec = 0L;
      secs += 2;

      readfds = (1 << fd);
      n = WaitSelect(fd + 1, (fd_set *)&readfds, NULL, NULL, &tv, NULL);
      if(n < 0)
      {
         if(errno == EINTR)
            syslog_AmiTCP(LOG_NOTICE, "bootpc_do: BOOTP request cancelled.");
         else
            syslog_AmiTCP(LOG_ERR, "bootpc_do: BOOTP select");
         break;
      }
      if(n == 0)  // We have not received a response since the last send.
         continue;

      fromlen = sizeof(sin_from);
      n = recvfrom(fd, (char *)bpc->bpc_rcv_packet, BPC_BUFLEN, 0, (struct sockaddr *)&sin_from, &fromlen);
      if(n <= 0)
         continue;         // send again if any error
      if(n < sizeof(struct bootp))
      {
         syslog_AmiTCP(LOG_DEBUG, "bootpc_do: received BOOTP packet was too short.");
         continue;
      }

      // check the packet opcode & transaction ID
      if(bpc->bpc_rcv_packet->bp_op != BOOTREPLY || bpc->bpc_rcv_packet->bp_xid != (u_int32) htonl(xid))
      {
         syslog_AmiTCP(LOG_DEBUG, "bootpc_do: discarding a BOOTP reply because of wrong op (%ld) or wrong transaction id (%lx != %lx).", bpc->bpc_rcv_packet->bp_op, bpc->bpc_rcv_packet->bp_xid, (u_int32) htonl(xid));
            continue;
      }

      bpc->bpc_rcv_len = n;  // store the actual length of the reply

      syslog_AmiTCP(LOG_DEBUG, "bootpc_do: received BOOTP reply from %ls:%ld", Inet_NtoA(sin_from.sin_addr.s_addr), sin_from.sin_port);
      return(bootpc_examine(bpc, iface_data, iface, isp));
   }

   syslog_AmiTCP(LOG_WARNING, "bootpc_do: no response to BOOTP request...");

   return(ENOENT);
}
///

