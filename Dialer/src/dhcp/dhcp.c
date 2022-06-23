/* $Id: main.c,v 0.10 1997/08/28 14:42:26 yoichi v0_70 $
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "if.h"
#include "dhcp.h"
#include "signal-handler.h"
#include "error-handler.h"
#include "daemon.h"
#include "client.h"
#include "memory.h"

#define DEFAULT_IF	"eth0"

char *CommandFile = NULL;		/* invoked command file name when dhcpcd
								 * succeeds in getting an IP address
								 */
int	  BeRFC1541 = 0;			/* default is InternetDraft mode */
char *Hostname = NULL;			/* hostname in the DHCP msg for xmit */

static char VersionStr[] = "dhcpcd 0.70\n";

void	usage();


void
main(argc, argv)
int argc;
char *argv[];
{
	char  ifname[10];			/* interface name */
	char  pidfile[128];			/* file name in which pid is stored */
	char *clientId = NULL;		/* ptr to client identifier user specified */
	int   killFlag = 0;			/* if 1: kill the running proc and exit */

	DebugFlag = 0;				/* default is NON debug mode */
	srand((u_int)time(NULL));
	signalSetup();
	umask(0);					/* clear umask */
	classIDsetup(NULL);			/* setup default class identifier */
	/* option handling
	 */
	while ( *++argv ) {
		if ( **argv == '-' ) {
			switch ( argv[0][1] ) {
			  case 'c':
				if ( *++argv == NULL ) {
					usage();
				}
				if ( (CommandFile = malloc(strlen(*argv)+1)) == NULL ) {
					usage();
				}
				strcpy(CommandFile, *argv);
				break;
			  case 'd':
				DebugFlag = 1;
				break;
			  case 'h':
				if ( *++argv == NULL ) {
					usage();
				}
				Hostname = smalloc(strlen(*argv)+1);
				strcpy(Hostname, *argv);
				break;
			  case 'i':
				if ( *++argv == NULL ) {
					usage();
				}
				classIDsetup(*argv); /* overwrite class identifier */
				break;
			  case 'I':
				if ( *++argv == NULL ) {
					usage();
				}
				clientId = *argv;
				break;
			  case 'k':
				killFlag = 1;	/* kill running process and exit */
				break;
			  case 'l':
				++argv;
				if ( *argv == NULL || **argv == '-' ) {
					usage();
				}
				SuggestLeaseTime = atol(*argv);
				break;
			  case 'r':
				BeRFC1541 = 1;	/* Be RFC1541 compliant */
				break;
			  case 'v':
				fflush(stdout);
				fputs(VersionStr, stderr);
				fflush(NULL);
				exit(0);
			  default:
				usage();
			}
		} else {
			break;
		}
	}

	if ( getuid() != 0 && geteuid() != 0 ) {
		errQuit("Must be root");
	}

	if ( *argv ) {
		strncpy(ifname, *argv, sizeof(ifname));
	} else {
		strncpy(ifname, DEFAULT_IF, sizeof(ifname));
	}
	if ( killFlag ) {
		sprintf(pidfile, PIDFILE, ifname);
		killCurProc(pidfile);
	}
	if ( !DebugFlag ) {
		sprintf(pidfile, PIDFILE, ifname);
		daemonInit(pidfile);
	}
	logOpen("dhcpcd", LOG_PID, LOG_LOCAL0);
	ifReset(ifname);			/* reset interface, 'Ifbuf' */
	clientIDsetup(clientId, ifname);
	dhcpMsgInit(ifname);
	dhcpClient();
}

void
usage()
{
	fflush(stdout);
	fputs("Usage: dhcpcd [-c filename] [-d] [-i classIdentifier] [-I clientIdentifier] [-k] [-l leasetime] [ifname]\n",
		  stderr);
	fflush(NULL);
	exit(1);
}

