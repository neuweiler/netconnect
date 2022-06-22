#include "globals.c"
#include "bootpc.h"
#include "iface.h"

/* RFC1048 vendor magic cookie */
static unsigned char vm_rfc1048[4] = VM_RFC1048;

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

int bootpc_examine(struct bootpc *bpc, struct iface *ifc, struct config *conf)
{
	struct bootp *bp = bpc->bpc_rcv_packet;

	/* Check our own IP */
	if(conf->cnf_addr == INADDR_ANY && bp->bp_yiaddr.s_addr != INADDR_ANY)
	{
		conf->cnf_addr = bp->bp_yiaddr.s_addr;
Printf("Got IP address %ls from the BOOTP reply.\n", Inet_NtoA(conf->cnf_addr));
	}

  /* Check RFC options */
	if(memcmp(bp->bp_vend, vm_rfc1048, sizeof(vm_rfc1048)) == 0)
	{
		u_char option;
		u_char opt_len;
		u_char *opt_ptr = bp->bp_vend + sizeof(vm_rfc1048);
		u_char *opt_end = (char *)bp + bpc->bpc_rcv_len;
		BOOL using;
		int    optcount = 0;
		int    tmplen;

Printf("It is a valid BOOTP reply.\nserver_hostname: '%ls'   boot_file: '%ls'\n", bp->bp_sname, bp->bp_file);

Printf("Checking if RFC options are present on the reply:\n");

		while(opt_ptr < opt_end - 1)  /* have to have at least two bytes */
		{
			if((option = *opt_ptr++) == BPTAG_END)
				break;
			if(option == TAG_PAD)
				continue;

			opt_len = *opt_ptr++;
			if(opt_ptr + opt_len > opt_end) /* have space for the option? */
				break;         /* no */

			if(option <= TAG_EXTEN_FILE)
Printf("Option: %ls, value: %ls, ", opt_names[option], my_link_ntoa(opt_len, opt_ptr));
			else
Printf("Option: %ld, value: %ls, ", option, my_link_ntoa(opt_len, opt_ptr));

			using = TRUE;
			optcount++;

			switch (option)
			{
				case TAG_SUBNET_MASK:
					if(conf->cnf_netmask == INADDR_ANY && opt_len == 4)
						memcpy(&conf->cnf_netmask, opt_ptr, 4);
Printf("%ls, ", Inet_NtoA(conf->cnf_netmask));
					break;
				case TAG_GATEWAY:    /* using only first gateway */
					if(conf->cnf_gateway == INADDR_ANY && opt_len >= 4)
						memcpy(&conf->cnf_gateway, opt_ptr, 4);
Printf("%ls, ", Inet_NtoA(conf->cnf_gateway));
					break;
				case TAG_DOMAIN_SERVER:
					for(tmplen = 0; tmplen + 4 <= opt_len; tmplen += 4)
					{
						if(conf->cnf_dns1 == INADDR_ANY)
							memcpy(&conf->cnf_dns1, &opt_ptr[tmplen], 4);
						else if(conf->cnf_dns2 == INADDR_ANY)
							memcpy(&conf->cnf_dns2, &opt_ptr[tmplen], 4);
					}
Printf("%ls, ", Inet_NtoA(conf->cnf_dns1));
					break;
				case TAG_HOST_NAME:
					tmplen = MIN(opt_len, sizeof(conf->cnf_hostname)-1);
					memcpy(conf->cnf_hostname, opt_ptr, tmplen);
					conf->cnf_hostname[tmplen] = '\0';
					break;
				case TAG_DOMAIN_NAME:
					tmplen = MIN(opt_len, sizeof(conf->cnf_domainname)-1);
					memcpy(conf->cnf_domainname, opt_ptr, tmplen);
					conf->cnf_domainname[tmplen] = '\0';
					break;
				default:
					using = FALSE;
					break;
			}

Printf((using) ? "using it.\n" : "not using it.\n");

			opt_ptr += opt_len;
		}
	}

	/* Additional test to get some working destination address */
	if(ifc->iface_flags & IFF_POINTOPOINT && conf->cnf_dst == INADDR_ANY && conf->cnf_gateway == INADDR_ANY && bp->bp_siaddr.s_addr != INADDR_ANY)
	{
		conf->cnf_dst = bp->bp_siaddr.s_addr;
Printf("Using BOOTP server address %s as the destination IP address.\n", Inet_NtoA(conf->cnf_dst));
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
			Printf("\nNetwork doesn't seem to be able to deliver packets.\nPlease check your network connection.\n");
		}
		else
			Printf("ERROR: bootpc_send: sendto\n");
		return(errno);
	}
	return(0);
}

///
/// bootpc_do
int bootpc_do(struct bootpc *bpc, struct iface *ifc, struct config *conf)
{
	int fd = ifc->iface_fd;
	short secs;
	int   retries;
	int32 xid;
	struct bootp *bp;
//   ULONG sigs;
	int error, n;
	long fromlen;
	struct sockaddr_in sin_server; /* where to send requests */
	struct sockaddr_in sin_client; /* for bind and listen */
	struct sockaddr_in sin_from;      /* Packet source */

	/* Set up server socket address (for send) */
	bzero(&sin_server, sizeof(sin_server));
	sin_server.sin_len = sizeof(sin_server);;
	sin_server.sin_family = AF_INET;
	sin_server.sin_port = htons(bpc->bpc_s_port);
	sin_server.sin_addr.s_addr = bpc->bpc_s_addr;

	/* Set up client socket address (for listen) */
	bzero(&sin_client, sizeof(sin_client));
	sin_client.sin_len = sizeof(sin_client);;
	sin_client.sin_family = AF_INET;
	sin_client.sin_port = htons(bpc->bpc_c_port);
	sin_client.sin_addr.s_addr = INADDR_ANY;

	/* Bind client socket to BOOTPC port. */
	if(bind(fd, (struct sockaddr *)&sin_client, sizeof(sin_client)) < 0)
	{
		syslog(LOG_ERR, "bind BOOTPC port: %m");
		if(errno == EACCES)
Printf("You need to run this as root\n");
Printf("bind errno=%ld\n", errno);
		return(errno);
	}

	/* Build a request. */
	bp = bpc->bpc_snd_packet;
	bzero(bp, sizeof(*bp));
	bp->bp_op = BOOTREQUEST;
	xid = (int32)FindTask(NULL);
	bp->bp_xid = (u_int32)htonl(xid);

	/* Fill in the hardware address, if appropriate (not for ppp, slip, etc) */
	if(conf->cnf_use_hwtype)
	{
Printf("Using the hardware address in BOOTP request.\n");
		/* Copy hardware address into request packet. */
		bp->bp_htype   = ifc->iface_sanatype;
		bp->bp_hlen    = ifc->iface_hlen;
		bcopy(ifc->iface_haddr, bp->bp_chaddr, bp->bp_hlen);
	}

  /* Fill in the client IP address. */
	if(conf->cnf_addr != INADDR_ANY)
	{
Printf("Using the IP address %ls on BOOTP request.\n", Inet_NtoA(conf->cnf_addr));
		bp->bp_ciaddr.s_addr = conf->cnf_addr;
	}
  
	memcpy(bp->bp_vend, vm_rfc1048, sizeof(vm_rfc1048)); /* copy default verdor data */
	bp->bp_vend[4] = BPTAG_END;

	bpc->bpc_snd_len = sizeof(*bp); /* total length of the request */

	secs = 2;

	for(retries = 0; retries < 3; retries++)
	{
		struct timeval tv;
		int readfds;

Printf("Sending BOOTP request to %ls.\n", Inet_NtoA(bpc->bpc_s_addr));
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
				Printf("\nBOOTP cancelled.\n");
			else
				Printf("Error: BOOTP select\n");
			break;
		}
		if(n == 0)  /* We have not received a response since the last send. */
		{
// handle mui stuff here
			continue;
		}

		fromlen = sizeof(sin_from);
		n = recvfrom(fd, (char *)bpc->bpc_rcv_packet, BPC_BUFLEN, 0, (struct sockaddr *)&sin_from, &fromlen);
		if(n <= 0)
			continue;         /* send again if any error */
		if(n < sizeof(struct bootp))
		{
			Printf("Received BOOTP packet was too short.\n");
			continue;
		}

		/* check the packet opcode & transaction ID */
		if(bpc->bpc_rcv_packet->bp_op != BOOTREPLY || bpc->bpc_rcv_packet->bp_xid != (u_int32) htonl(xid))
		{
			Printf("Discarding a BOOTP reply because of wrong op (%ld) or wrong transaction id (%lx != %lx).\n", bpc->bpc_rcv_packet->bp_op, bpc->bpc_rcv_packet->bp_xid, (u_int32) htonl(xid));
				continue;
		}

		bpc->bpc_rcv_len = n;  /* store the actual length of the reply */

Printf("Received BOOTP reply from %ls:%ld\n", Inet_NtoA(sin_from.sin_addr.s_addr), sin_from.sin_port);
		return(bootpc_examine(bpc, ifc, conf));
	}

	Printf("No response to the BOOTP request...\n");
	return(ENOENT);
}
///

