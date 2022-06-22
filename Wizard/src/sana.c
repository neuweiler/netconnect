#include "/AmiTCP.h"
#include "sana.h"

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
struct sana2 *sana2_create(struct ifconfig *ifc)
{
   struct sana2 * s2;

   if(s2 = AllocVec(sizeof(*s2), MEMF_ANY | MEMF_CLEAR))
   {
      s2->s2_openerr = 1;
      s2->s2_name = (STRPTR)GetTagData(IF_Device, (ULONG)"", ifc->ifc_taglist);
      s2->s2_unit = GetTagData(IF_Unit, 0, ifc->ifc_taglist);

      if(s2->s2_port = CreateMsgPort())
      {
         if(s2->s2_req = CreateIORequest(s2->s2_port, sizeof(*s2->s2_req)))
         {
            s2->s2_req->ios2_BufferManagement = SanaTags;
            if(s2->s2_openerr = OpenDevice(s2->s2_name, s2->s2_unit, (struct IORequest *)s2->s2_req, 0L))
               Printf("Could not open sana2 device: %ls, unit %ld", s2->s2_name, s2->s2_unit);
            else
               return(s2);
         }
         else
            PrintFault(ERROR_NO_FREE_STORE, "CreateIORequest");
      }
      else
         PrintFault(ERROR_NO_FREE_STORE, "CreateMsgPort");

   }
   else
      PrintFault(ERROR_NO_FREE_STORE, "AllocVec");

   sana2_delete(s2);
   return(NULL);
}

///
/// sana2_delete
VOID sana2_delete(struct sana2 *s2)
{
   if(!s2->s2_openerr)
      CloseDevice((struct IORequest*)s2->s2_req);
   if(s2->s2_req)
      DeleteIORequest((struct IORequest *)s2->s2_req);
   if(s2->s2_port)
      DeleteMsgPort(s2->s2_port);

   FreeVec(s2);
}

///
/// sana2_getaddresses
BOOL sana2_getaddresses(struct sana2 *s2, struct config *conf)
{
   if(!(s2->s2_hwtype == S2WireType_PPP
      || s2->s2_hwtype == S2WireType_SLIP
      || s2->s2_hwtype == S2WireType_CSLIP))
      return(FALSE);

   s2->s2_req->ios2_Req.io_Command = S2_GETSTATIONADDRESS;
   DoIO((struct IORequest *)s2->s2_req);
   if(s2->s2_req->ios2_Req.io_Error)
   {
      Printf("Could not get sana2 station address from device %ls, unit %ld.\n", s2->s2_name, s2->s2_unit);
      SetIoErr(s2->s2_req->ios2_Req.io_Error); /* Set secondary error also */
      return(FALSE);
   }

   if(conf->cnf_addr == INADDR_ANY)
   {
      memcpy(&conf->cnf_addr, s2->s2_req->ios2_SrcAddr, sizeof(conf->cnf_addr));
      if(conf->cnf_addr != INADDR_ANY)
Printf("Using IP address %s from Sana2 device configuration.\n", Inet_NtoA(conf->cnf_addr));
   }
   if(s2->s2_hwtype == S2WireType_PPP)
   {
      if(conf->cnf_dst == INADDR_ANY)
      {
         /* check the destination address */
         memcpy(&conf->cnf_dst, s2->s2_req->ios2_DstAddr, sizeof(conf->cnf_addr));
         if(conf->cnf_dst != INADDR_ANY)
Printf("Using destination IP address %s from Sana2 device configuration.", Inet_NtoA(conf->cnf_dst));
      }
   }
   return(TRUE);
}

///
/// sana2_online
BOOL sana2_online(struct sana2 *s2)
{
   s2->s2_req->ios2_Req.io_Command = S2_ONLINE;
   DoIO((struct IORequest *)s2->s2_req);
   if(s2->s2_req->ios2_Req.io_Error)
   {
      if(s2->s2_req->ios2_Req.io_Error != S2ERR_BAD_STATE)
      {
         Printf("Could not put %ls, unit %ld online.\n", s2->s2_name, s2->s2_unit);
         SetIoErr(s2->s2_req->ios2_Req.io_Error); /* Set secondary error also */
         return(FALSE);
      }
   }
   return(TRUE);
}

///
/// sana2_offline
BOOL sana2_offline(struct sana2 *s2)
{
   s2->s2_req->ios2_Req.io_Command = S2_OFFLINE;
   DoIO((struct IORequest *)s2->s2_req);
   if(s2->s2_req->ios2_Req.io_Error)
   {
      if(s2->s2_req->ios2_Req.io_Error != S2ERR_BAD_STATE)
      {
         Printf("Could not put %ls, unit %ld offline.\n", s2->s2_name, s2->s2_unit);
         SetIoErr(s2->s2_req->ios2_Req.io_Error); /* Set secondary error also */
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
      Printf("Could not do a device-query on %ls, unit %ld.\n", s2->s2_name, s2->s2_unit);
      SetIoErr(s2->s2_req->ios2_Req.io_Error); /* Set secondary error also */
      return(FALSE);
   }
   s2->s2_hwtype     = devicequery.HardwareType;
   s2->s2_addrbits   = devicequery.AddrFieldSize;

   return(TRUE);
}
///

