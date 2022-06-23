/// includes
#include "/includes.h"

#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include "/Genesis.h"
#include "iface.h"
#include "sana.h"
#include "protos.h"
#include "Strings.h"

///

/// dummy
int dummy(void)
{
  return(0);
}

static ULONG SanaTags[] = {
   S2_CopyToBuff, (ULONG)dummy,
   S2_CopyFromBuff, (ULONG)dummy,
   TAG_DONE, 0
};
///
/// sana2_create
struct sana2 *sana2_create(STRPTR device, LONG unit)
{
   struct sana2 * s2;

   if(s2 = AllocVec(sizeof(*s2), MEMF_ANY | MEMF_CLEAR))
   {
      s2->s2_openerr = 1;
      strncpy(s2->s2_name, device, MAXPATHLEN);
      s2->s2_unit = unit;

      if(s2->s2_port = CreateMsgPort())
      {
         if(s2->s2_req = CreateIORequest(s2->s2_port, sizeof(*s2->s2_req)))
         {
            s2->s2_req->ios2_BufferManagement = SanaTags;
            if(!(s2->s2_openerr = OpenDevice(s2->s2_name, s2->s2_unit, (struct IORequest *)s2->s2_req, 0L)))
               return(s2);
         }
      }
   }

   sana2_delete(s2);
   return(NULL);
}

///
/// sana2_delete
VOID sana2_delete(struct sana2 *s2)
{
   if(s2)
   {
      if(!s2->s2_openerr)
         CloseDevice((struct IORequest*)s2->s2_req);
      if(s2->s2_req)
         DeleteIORequest((struct IORequest *)s2->s2_req);
      if(s2->s2_port)
         DeleteMsgPort(s2->s2_port);

      FreeVec(s2);
   }
}

///

unsigned short in_cksum( unsigned short *addr, int len );

/// sana2_do_icmp
BOOL sana2_do_icmp(struct Library *SocketBase, struct sana2 *s2, struct Interface *iface)
{
/*
   unsigned short myid = ((ULONG)FindTask( NULL ) ) & 0xffff;
   static char outpack[ 256 ];
   struct icmp *icp;
   struct ip *ip;

   bzero(outpack, sizeof(outpack));
   ip = (struct ip *) outpack;
   icp = (struct icmp *)(outpack + sizeof(struct ip));

   ip->ip_v    = 4;
   ip->ip_hl   = sizeof(struct ip) >> 2;
   ip->ip_tos  = 0;
   ip->ip_len  = sizeof(struct ip) + sizeof(struct icmp);
   ip->ip_id   = myid;
   ip->ip_ttl  = 5;
   ip->ip_p    = 1; // ICMP = 1
   ip->ip_src.s_addr = inet_addr(iface->if_addr);
   ip->ip_dst.s_addr = 0;
   ip->ip_sum  = in_cksum((USHORT *)ip, 16);

   icp->icmp_type = ICMP_IREQ;
   icp->icmp_seq  = 0;
   icp->icmp_id   = myid;

   // Compute ICMP checksum here
   icp->icmp_cksum = in_cksum((USHORT *)icp, 16);

Printf("sending icmp req\n");
   // send stuff
   s2->s2_req->ios2_Data            = outpack;
   s2->s2_req->ios2_DataLength      = sizeof(struct ip) + 16;
   s2->s2_req->ios2_Req.io_Command  = CMD_WRITE;
   DoIO((struct IORequest *)s2->s2_req);

   bzero(outpack, sizeof(outpack));

   s2->s2_req->ios2_Data            = outpack;
   s2->s2_req->ios2_DataLength      = sizeof(outpack);
   s2->s2_req->ios2_Req.io_Command  = CMD_READ;
   SendIO((struct IORequest *)s2->s2_req);
Printf("waiting for input\n");
   Delay(100);
Printf("finished waiting\n");
//   if(len > 8)
   {
      struct ip *ip = (struct ip *) outpack;
      int hlen = ip->ip_hl << 2;

      icp = (struct icmp *)(outpack + hlen);

Printf("ip_v: %ld\n", ip->ip_v);
Printf("ip_hl: %ld\n", ip->ip_hl);
Printf("ip_tos: %ld\n", ip->ip_tos);
Printf("ip_len: %ld\n", ip->ip_len);
Printf("ip_id: %ld\n", ip->ip_id);
Printf("ip_off: %ld\n", ip->ip_off);
Printf("ip_ttl: %ld\n", ip->ip_ttl);
Printf("ip_p: %ld\n", ip->ip_p);
Printf("ip_sum: %ld\n", ip->ip_sum);
Printf("ip_src: %ld, %ls\n", ip->ip_src.s_addr, Inet_NtoA(ip->ip_src.s_addr));
Printf("ip_dst: %ld, %ls\n", ip->ip_dst.s_addr, Inet_NtoA(ip->ip_dst.s_addr));

Printf("icmp_type: %ld\n", icp->icmp_type);
Printf("icmp_code: %ld\n", icp->icmp_code);
Printf("icmp_cksum: %ld\n", icp->icmp_cksum);
Printf("icmp_pptr: %ld\n", icp->icmp_pptr);
Printf("icmp_gwaddr: %ld, %ls\n", icp->icmp_gwaddr.s_addr, Inet_NtoA(icp->icmp_gwaddr.s_addr));
Printf("icmp_id: %ld\n", icp->icmp_id);
Printf("icmp_seq: %ld\n", icp->icmp_seq);
Printf("icmp_void: %ld\n", icp->icmp_void);
Printf("icmp_otime: %ld\n", icp->icmp_otime);
Printf("icmp_rtime: %ld\n", icp->icmp_rtime);
Printf("icmp_ttime: %ld\n", icp->icmp_ttime);
Printf("icmp_mask: %ld\n", icp->icmp_mask);
Printf("icmp_data: '%ls' %ld  '%lc'\n", icp->icmp_data, icp->icmp_data, *icp->icmp_data);
   }
   if(!(CheckIO((struct IORequest *)s2->s2_req)))
      AbortIO((struct IORequest *)s2->s2_req);
   WaitIO((struct IORequest *)s2->s2_req);
*/
   return(TRUE);
}

///
/// sana2_getaddresses
BOOL sana2_getaddresses(struct Library *SocketBase, struct sana2 *s2, struct Interface *iface)
{
   ULONG tmp_addr;

   if(!(s2->s2_hwtype == S2WireType_PPP
      || s2->s2_hwtype == S2WireType_SLIP
      || s2->s2_hwtype == S2WireType_CSLIP))
      return(FALSE);

   s2->s2_req->ios2_Req.io_Command = S2_GETSTATIONADDRESS;
   DoIO((struct IORequest *)s2->s2_req);
   if(s2->s2_req->ios2_Req.io_Error)
   {
      SetIoErr(s2->s2_req->ios2_Req.io_Error); // Set secondary error also
      return(FALSE);
   }

   if(!*iface->if_addr && (s2->s2_req->ios2_SrcAddr != INADDR_ANY))
   {
      memcpy(&tmp_addr, s2->s2_req->ios2_SrcAddr, sizeof(tmp_addr));
      strcpy(iface->if_addr, Inet_NtoA(tmp_addr));
      syslog_AmiTCP(SocketBase, LOG_DEBUG, GetStr(MSG_SYSLOG_UsingSana2IPAddress), iface->if_addr);
   }
   if(s2->s2_hwtype == S2WireType_PPP)
   {
      if(!*iface->if_dst && (s2->s2_req->ios2_DstAddr != INADDR_ANY))
      {
         // check the destination address
         memcpy(&tmp_addr, s2->s2_req->ios2_DstAddr, sizeof(tmp_addr));
         strcpy(iface->if_dst, Inet_NtoA(tmp_addr));
         syslog_AmiTCP(SocketBase, LOG_DEBUG, GetStr(MSG_SYSLOG_UsingSana2DestIPAddress), iface->if_dst);
      }
   }

   return(TRUE);
}

///
/// sana2_online
BOOL sana2_online(struct Library *SocketBase, struct sana2 *s2)
{
   s2->s2_req->ios2_Req.io_Command = S2_ONLINE;
   DoIO((struct IORequest *)s2->s2_req);
   if(s2->s2_req->ios2_Req.io_Error)
   {
      if(s2->s2_req->ios2_Req.io_Error != S2ERR_BAD_STATE)
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_ErrorSana2Online), s2->s2_name, s2->s2_unit);
         SetIoErr(s2->s2_req->ios2_Req.io_Error); // Set secondary error also
         return(FALSE);
      }
   }
   return(TRUE);
}

///
/// sana2_offline
BOOL sana2_offline(struct Library *SocketBase, struct sana2 *s2)
{
   s2->s2_req->ios2_Req.io_Command = S2_OFFLINE;
   DoIO((struct IORequest *)s2->s2_req);
   if(s2->s2_req->ios2_Req.io_Error)
   {
      if(s2->s2_req->ios2_Req.io_Error != S2ERR_BAD_STATE)
      {
         syslog_AmiTCP(SocketBase, LOG_ERR, GetStr(MSG_SYSLOG_ErrorSana2Offline), s2->s2_name, s2->s2_unit);
         SetIoErr(s2->s2_req->ios2_Req.io_Error); // Set secondary error also
         return(FALSE);
      }
   }
   return(TRUE);
}

///
/// sana2_devicequery
BOOL sana2_devicequery(struct sana2 *s2)
{
   struct Sana2DeviceQuery devicequery;

   s2->s2_req->ios2_Req.io_Command   = S2_DEVICEQUERY;
   s2->s2_req->ios2_StatData         = &devicequery;
   devicequery.SizeAvailable     = sizeof(devicequery);
   devicequery.DevQueryFormat    = NULL;
   devicequery.DeviceLevel       = NULL;

   DoIO((struct IORequest *)s2->s2_req);
   if(s2->s2_req->ios2_Req.io_Error)
   {
      SetIoErr(s2->s2_req->ios2_Req.io_Error); // Set secondary error also
      return(FALSE);
   }
   s2->s2_hwtype     = devicequery.HardwareType;
   s2->s2_addrbits   = devicequery.AddrFieldSize;

   return(TRUE);
}
///

