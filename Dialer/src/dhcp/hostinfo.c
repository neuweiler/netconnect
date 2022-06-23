/// includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <net/if.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "/iface.h"
#include "dhcp.h"
#include "dhcp-options.h"
#include "error-handler.h"
#include "signal-handler.h"
#include "hostinfo.h"
#include "if.h"
#include "memory.h"
#include "/protos.h"
#include "//Genesis.h"
#include "//genesis.lib/libraries/genesis.h"
#include "//genesis.lib/pragmas/genesis_lib.h"
#include "//genesis.lib/proto/genesis.h"
#include "/mui_MainWindow.h"

///
/// defines
#define MAXLEN 256

///
/// variables
extern char *CommandFile;
extern Object *win;
extern struct MUI_CustomClass *CL_MainWindow;
extern struct Library *GenesisBase;

///

/// setupOptInfo
void setupOptInfo(u_char *dest[], const u_char *src[])
{
   int i;

   for(i = 0; i < MAXNOPT; ++i)
   {
      if(src[i] != NULL)
      {
         if(dest[i] != NULL)
         {
            free(dest[i]);
         }
         if(dest[i] = malloc(*src[i]+1))
            bcopy(src[i], dest[i], *src[i]+1);
      }
   }
}

///
/// freeOptInfo
void freeOptInfo(u_char *optp[])
{
   int i;

   for(i = 0; i < MAXNOPT; ++i)
   {
      if(optp[i] != NULL)
      {
         free(optp[i]);
         optp[i] = NULL;
      }
   }
}

///
/// saveHostInfo
void saveHostInfo(struct Library *SocketBase, const u_char *optp[], struct Interface_Data *iface_data)
{
   struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);
   int fd;
   char path[MAXLEN];
   char buf[MAXLEN];
   
   /* hostname */
   if(optp[OhostName] != NULL)
      strncpy(data->isp.isp_hostname, optp[OhostName]+1, *optp[OhostName]);

   /* NIS domain name */
   if ( optp[OnisDomName] != NULL )
   {
      strncpy(path, optp[OnisDomName]+1, *optp[OnisDomName]);
      if(!find_server_by_name(&data->isp.isp_nameservers, path))
         add_server(&data->isp.isp_domainnames, path);
   }

   /* default route (routers) */
   if ( optp[Orouter] != NULL ) {
      setDefRoute(SocketBase, optp[Orouter], iface_data);
   }

   /* make directory for resolv.conf and  hostinfo file */
   if(mkdir(HOST_INFO_DIR))
      return;

   /* ntp.conf */
   if(optp[OntpServer] != NULL)
      mkNTPconf(SocketBase, optp[OntpServer]);

   /* resolv.conf */
   if(optp[Odns] != NULL && optp[OdomainName] != NULL)
      mkResolvConf(SocketBase, optp[Odns], optp[OdomainName]);

   /* hostinfo file */
   strncpy(path, HOST_INFO_DIR, MAXLEN);
   strncat(path, "/", MAXLEN);
   strncat(path, HOST_INFO_FILE, MAXLEN);
   strncat(path, "-", MAXLEN);
   strncat(path, iface_data->ifd_name, MAXLEN);
   if((fd = creat(path,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "creat (setupHostInfo)");
      return;
   }
   sprintf(buf, "LEASETIME=%ld\n", ntohl(LeaseTime));
   sprintf(buf+strlen(buf), "RENEWALTIME=%ld\n", ntohl(RenewTime));
   sprintf(buf+strlen(buf), "REBINDTIME=%ld\n", ntohl(RebindTime));
   sprintf(buf+strlen(buf), "IPADDR=%s\n", Inet_NtoA(iface_data->ifd_addr));
   sprintf(buf+strlen(buf), "NETMASK=%s\n", Inet_NtoA(iface_data->ifd_netmask));
   sprintf(buf+strlen(buf), "BROADCAST=%s\n", Inet_NtoA(iface_data->ifd_dst));
   if(write(fd, buf, strlen(buf)) < 0)
      syslog_AmiTCP(SocketBase, LOG_ERR, "write (setupHostInfo)");

   if (optp[OhostName] != NULL)
      addHostInfo(SocketBase, fd, OT_STRING, "HOSTNAME", optp[OhostName]);

   if(optp[OnisDomName] != NULL)
      addHostInfo(SocketBase, fd, OT_STRING, "NISDOMAINNAME", optp[OnisDomName]);

   if(optp[OlprServer] != NULL)
      addHostInfo(SocketBase, fd, OT_ADDR, "LPRSERVER", optp[OlprServer]);

   if(optp[OntpServer] != NULL)
      addHostInfo(SocketBase, fd, OT_ADDR, "NTPSERVER", optp[OntpServer]);

   if(optp[OtimeServer] != NULL)
      addHostInfo(SocketBase, fd, OT_ADDR, "TIMESERVR", optp[OtimeServer]);

   if(optp[Orouter] != NULL)
      addHostInfo(SocketBase, fd, OT_ADDR, "ROUTER", optp[Orouter]);

   close(fd);
}

///
/// addHostInfo
void addHostInfo(struct Library *SocketBase, int fd, const int flag, const char *name, const u_char *optp)
{
   char  buf[MAXLEN];
   char  env[MAXLEN];
   int      i;
   u_long  *p;

   switch(flag)
   {
      case OT_STRING:
         strncpy(env, optp+1, *optp);
         sprintf(buf, "ENV:%ls", name);
         if(!WriteFile(buf, env, -1))
            Printf("setenv (addHostInfo): insufficient space\n");
         strcpy(buf, name);
         strcat(buf, "=");
         strncat(buf, optp+1, *optp);
         strcat(buf, "\n");
         break;
      case OT_ADDR:
         p = (u_long *)(optp + 1);
         sprintf(buf, "ENV:%ls", name);
         if(!WriteFile(buf, Inet_NtoA(*p), -1))
            Printf("setenv (addHostInfo): insufficient space\n");
         strcpy(buf, name);
         strcat(buf, "=");
         strcat(buf, Inet_NtoA(*p));
         ++p;
         strcat(buf, "\n");
         for(i = 1; i < *optp/4; ++i)
         {
            sprintf(env, "ENV:%ls%ld", name, i+1);
            if(!WriteFile(env, Inet_NtoA(*p), -1))
               Printf("setenv (addHostInfo): insufficient space\n");
            sprintf(buf+strlen(buf), "%ls%ld=%ls\n", name, i+1, Inet_NtoA(*p));
            ++p;
         }
         break;
      default:
         return;
   }
   if(write(fd, buf, strlen(buf)) < 0)
      syslog_AmiTCP(SocketBase, LOG_ERR, "write (addHostInfo)");
}

///
/// mkNTPconf
void mkNTPconf(struct Library *SocketBase, const u_char *addr)
{
   char buf[MAXLEN];
   int  fd;
   int  i;
   u_long *p;

   strcpy(buf, HOST_INFO_DIR);
   strcat(buf, "/");
   strcat(buf, "ntp.conf");
   if((fd = creat(buf,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0 )
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "creat (mkNTPconf)");
      return;
   }
   /* TODO: check whether NTP service is working on those addresses */
   p = (u_long *)(addr + 1);
   *buf = '\0';
   for(i = 0; i < *addr/4; ++i)
   {
      sprintf(buf+strlen(buf), "server %s\n", Inet_NtoA(*p));
      ++p;
   }
   if(write(fd, buf, strlen(buf)) < 0)
      syslog_AmiTCP(SocketBase, LOG_ERR, "write (mkNTPconf)");

   close(fd);
}

///
/// mkResolvConf
void mkResolvConf(struct Library *SocketBase, const u_char *addr, const u_char *domName)
{
   char buf[MAXLEN];
   int  fd;
   int  i;
   u_long *p;

   strcpy(buf, HOST_INFO_DIR);
   strcat(buf, "/");
   strcat(buf, "resolv.conf");
   if((fd = creat(buf,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
   {
      syslog_AmiTCP(SocketBase, LOG_ERR, "creat (mkResolvConf)");
      return;
   }

   /* TODO: check whether name server is working on those addresses */
   strcpy(buf, "domain ");
   strncat(buf, domName+1, *domName);
   strcat(buf, "\n");
   p = (u_long *)(addr + 1);
   for(i = 0; i < *addr/4; ++i)
   {
      sprintf(buf+strlen(buf), "nameserver %s\n", Inet_NtoA(*p));
      ++p;
   }
   if(write(fd, buf, strlen(buf)) < 0)
      syslog_AmiTCP(SocketBase, LOG_ERR, "write (mkResolvConf)");

   close(fd);
}

///
/// execCommandFile
void execCommandFile(VOID)
{
//   pid_t pid;
//
//   if ( CommandFile == NULL ) {
//      return;
//   }
//   if ( (pid = fork()) < 0 ) {
//      logSysRet("first fork (execCommandFile)");
//   } else if ( pid == 0 ) {
//      if ( (pid = fork()) < 0 ) {
//         logSysRet("second fork (execCommandFile)");
//      } else if ( pid > 0 ) {
//         /* we're the 1st child. let's just exit
//          */
//         exit(0);
//      }
//      /* we are the second child. let's exec the command file
//       */
//      if ( execlp(CommandFile, CommandFile, NULL) < 0 ) {
//         logSysRet("execlp (execCommandFile)");
//      }
//   }
//   /* we're the parent (original process).
//    * let's wait for the first child
//    */
//   if ( waitpid(pid, NULL, 0) != pid ) {
//      logSysRet("waitpid (execCommandFile)");
//   }
}

///
