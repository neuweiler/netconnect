/// includes
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "//Genesis.h"
#include "/iface.h"
#include "if.h"
#include "dhcp.h"
#include "error-handler.h"
#include "/protos.h"

///
/// defines
#define ioctl IoctlSocket

///

/// ifReset
void ifReset(struct Library *SocketBase, struct Interface *iface, struct Interface_Data *iface_data)
{
   strncpy(iface_data->ifd_name, iface->if_name, sizeof(iface_data->ifd_name));
   iface_data->ifd_addr     = inet_addr(iface->if_addr);
   iface_data->ifd_netmask  = inet_addr(iface->if_netmask);
   iface_data->ifd_dst      = inet_addr("255.255.255.255");
   iface_data->ifd_gateway  = inet_addr(iface->if_gateway);
   ifConfig(SocketBase, iface_data);
}

///
/// ifConfig
BOOL ifConfig(struct Library *SocketBase, struct Interface_Data *iface_data)
{
   BOOL success = FALSE;

   strncpy(iface_data->ifd_ifr.ifr_name, iface_data->ifd_name, sizeof(iface_data->ifd_ifr.ifr_name));

   if((iface_data->ifd_fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
   {
      if(!iface_setaddr(SocketBase, iface_data, iface_data->ifd_addr))
      {
         if(!iface_setdstaddr(SocketBase, iface_data, iface_data->ifd_dst))
         {
            if(!iface_setnetmask(SocketBase, iface_data, iface_data->ifd_netmask))
            {
               /* set route to the interface */
               if(!route_add(SocketBase, iface_data->ifd_fd, RTF_UP, iface_data->ifd_addr & iface_data->ifd_netmask, iface_data->ifd_addr, TRUE))
               {
                  success = TRUE;
               }
            }
         }
      }
      CloseSocket(iface_data->ifd_fd);
      iface_data->ifd_fd = -1;
   }
   else
      syslog_AmiTCP(SocketBase, LOG_ERR, "socket (ifConfig)");

   return(success);
}

///
/// ifDown
void ifDown(struct Library *SocketBase, struct Interface_Data *iface_data)
{
   strncpy(iface_data->ifd_ifr.ifr_name, iface_data->ifd_name, sizeof(iface_data->ifd_ifr.ifr_name));

   if(iface_data->ifd_fd < 0)
      iface_data->ifd_fd = socket(AF_INET, SOCK_DGRAM, 0);
   if(iface_data->ifd_fd < 0)
      syslog_AmiTCP(SocketBase, LOG_ERR, "socket (ifDown)");
   else
   {
      /* down interface */
      iface_data->ifd_ifr.ifr_flags = ~IFF_UP;
      if(ioctl(iface_data->ifd_fd, SIOCSIFFLAGS, (char *)&iface_data->ifd_ifr) < 0)
         syslog_AmiTCP(SocketBase, LOG_ERR, "ioctl SIOCSIFFLAGS (ifDown)");
   }
   CloseSocket(iface_data->ifd_fd);
   iface_data->ifd_fd = -1;
}

///
/// getNaturalMask
u_long getNaturalMask(struct Library *SocketBase, u_long inaddr)
{
   if(isClassA(inaddr))
      return(inet_addr("255.0.0.0"));
   if(isClassB(inaddr))
      return(inet_addr("255.255.0.0"));
   if(isClassC(inaddr))
      return(inet_addr("255.255.255.0"));

   return(htonl(0));
}

///
/// getNaturalBcast
u_long getNaturalBcast(u_long inaddr)
{
   if(isClassA(inaddr))
      return((inaddr & htonl(0xff000000)) | htonl(0x00ffffff));
   if(isClassB(inaddr))
      return((inaddr & htonl(0xffff0000)) | htonl(0x0000ffff));
   if(isClassC(inaddr))
      return((inaddr & htonl(0xffffff00)) | htonl(0x000000ff));

   return(htonl(0));
}

///
/// saveIfInfo
void saveIfInfo(struct Library *SocketBase, struct Interface_Data *iface_data)
{
   int       fd;
   int       isfailed;
   u_long    leaseLeft;
   char   filename[IFNAMSIZ + 128];

   isfailed = 0;
   strcpy(filename, DHCP_CACHE_FILE);
   strcat(filename, iface_data->ifd_name);
   if((fd = creat(filename, 0644)) >= 0)
   {
      if(write(fd, (const char *)&iface_data->ifd_addr, sizeof(iface_data->ifd_addr)) < 0)
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, "write (saveIfinfo)");
         isfailed = 1;
      }
      if(LeaseTime == INFINITE_LEASE_TIME)
         leaseLeft = INFINITE_LEASE_TIME;
      else
         leaseLeft = ntohl(LeaseTime) - (time(NULL) - ReqSentTime);

      if(write(fd, (const char *)&leaseLeft, sizeof(leaseLeft)) < 0 )
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, "write (saveIfinfo)");
         isfailed = 1;
      }
      if(isfailed)
      {
         if(unlink((const char*)filename) < 0)
            syslog_AmiTCP(SocketBase, LOG_ERR, "unlink (saveIfinfo)");
      }
      close(fd);
   }
   else
      syslog_AmiTCP(SocketBase, LOG_ERR, "creat (saveIfinfo)");
}

///
/// setDefRoute
void setDefRoute(struct Library *SocketBase, const char *routers, struct Interface_Data *iface_data)
{
   int s;
   int i;
   u_long gwAddr;          /* router's IP address (network byte order) */
   struct ortentry    rtent;
   struct sockaddr_in   *p;

   if((s = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
   {
      bzero((char *)&rtent, sizeof(rtent));
      p = (struct sockaddr_in *)&rtent.rt_dst;
      p->sin_family     = AF_INET;
      p->sin_addr.s_addr   = htonl(0); /* dest. net address (default route) */
      p = (struct sockaddr_in *)&rtent.rt_gateway;
      p->sin_family     = AF_INET;

      /* verify rouer addresses are correct */
      for(i = 0; i < *routers/4; ++i)
      {
         gwAddr = *((u_long *)(routers+1) + i);
         if((gwAddr & iface_data->ifd_netmask) == (iface_data->ifd_addr & iface_data->ifd_netmask))
         {
            p->sin_addr.s_addr   = gwAddr;
            rtent.rt_flags    = RTF_GATEWAY;    /* dest. is a gateway */
            if(ioctl(s, SIOCADDRT, (char *)&rtent) < 0)
               syslog_AmiTCP(SocketBase, LOG_ERR, "ioctl SIOCADDRT (setDefRoute)");
            break;
         }
         /* TODO: also verify if the router is alive or not by using ping */
      }
      CloseSocket(s);
   }
   else
      syslog_AmiTCP(SocketBase, LOG_ERR, "socket (setDefRoute)");
}

///
