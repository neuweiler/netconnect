/// includes
#include "/includes.h"
#pragma header

#include "/Genesis.h"
#include "Strings.h"
#include "mui.h"
#include "mui_MainWindow.h"
#include "protos.h"

///
/// external variables
extern struct Catalog          *cat;
extern struct MUI_CustomClass  *CL_MainWindow;
extern struct MsgPort          *WritePortSER;
extern struct MsgPort          *ReadPortSER;
extern struct MsgPort          *MainPort;
extern struct IOExtSer         *WriteSER;
extern struct IOExtSer         *ReadSER;
extern char serial_in[];
extern BOOL ReadQueued;
extern Object *win;
extern struct Config Config;
extern int addr_assign, dst_assign, dns_assign, domainname_assign;
extern const char AmiTCP_PortName[];

///

/// xget
LONG xget(Object *obj,ULONG attribute)
{
   LONG x;
   get(obj,attribute,&x);
   return(x);
}

///
/// DoSuperNew
ULONG DoSuperNew(struct IClass *cl,Object *obj,ULONG tag1,...)
{
   return(DoSuperMethod(cl,obj,OM_NEW,&tag1,NULL));
}

///
/// MakeButton
Object *MakeButton(STRPTR string)
{
   Object *obj = MUI_MakeObject(MUIO_Button, GetStr(string));
   if(obj)
      set(obj,MUIA_CycleChain,1);
   return(obj);
}

///
/// MakeText
Object *MakeText(STRPTR string)
{
   Object *obj = TextObject,
//      TextFrame,
//      MUIA_Background, MUII_TextBack,
      MUIA_Text_Contents,  string,
   End;

   return(obj);
}

///
/// MakeString
Object *MakeString(STRPTR string, LONG len)
{
/*   return(TextinputObject,
      StringFrame,
      MUIA_CycleChain         , 1,
      MUIA_Textinput_Multiline, FALSE,
      MUIA_Textinput_Contents , string,
      MUIA_Textinput_MaxLen   , len,
      End);
*/
   return(StringObject,\
      MUIA_CycleChain, 1,
      StringFrame,
      MUIA_String_MaxLen  , len,
      MUIA_String_Contents, string,
      End);
}

///
/// MakePopAsl
Object *MakePopAsl(Object *string, STRPTR title, BOOL drawers_only)
{
   return(PopaslObject,
      MUIA_Popstring_String, string,
      MUIA_Popstring_Button, PopButton((drawers_only ? MUII_PopDrawer : MUII_PopFile)),
      MUIA_Popasl_Type     , ASL_FileRequest,
      ASLFR_TitleText      , GetStr(title),
      ASLFR_DrawersOnly    , drawers_only,
   End);
}

///
/// GetStr
STRPTR GetStr(STRPTR idstr)
{
   STRPTR local;

   local = idstr + 2;

   if(cat)
      return((STRPTR)GetCatalogStr(cat, *(UWORD *)idstr, local));

   return(local);
}

///
/// GetEnvDOS
LONG GetEnvDOS(STRPTR name, STRPTR buffer, LONG max_len)
{
   char  file[MAXPATHLEN];
   LONG  size;
   BPTR  fh;

   *buffer = NULL;
   strcpy(file, "ENV:");
   AddPart(file, name, MAXPATHLEN);
   if(fh = Open(file, MODE_OLDFILE))
   {
      size = Read(fh, buffer, max_len);
      Close(fh);
      if(size >= 0)
         buffer[size] = NULL;
   }
   return((LONG)strlen(buffer));
}

///
/// EscapeString
VOID EscapeString(STRPTR buffer, STRPTR str)
{
/* with backlash '\' :
   B   - backspace
   T   - horizontal tab
   N   - newline
   V   - vertical tab
   F   - formfeed
   R   - return
   E   - escape (ASCII 27 decimal)
   Xnn - character represented by hex value nn.
*/
   ULONG len;
   UWORD offs;
   STRPTR help;

   offs  = 0;
   len   = 80;
   help  = str;

   while(*help && len)
   {
      if(*help != '\\')
      {
         buffer[offs++] = *help++;
      }
      else
      {
         help++;     /* auf cmd-char stellen.. */
         switch((ULONG)*help++)
         {
            case 'b':
            case 'B':
               buffer[offs++] = '\b';
               break;
            case 't':
            case 'T':
               buffer[offs++] = '\t';
               break;
            case 'n':
            case 'N':
               buffer[offs++] = '\n';
               break;
            case 'v':
            case 'V':
               buffer[offs++] = '\v';
               break;
            case 'f':
            case 'F':
               buffer[offs++] = '\f';
               break;
            case 'r':
            case 'R':
               buffer[offs++] = '\r';
               break;
            case 'e':
            case 'E':
               buffer[offs++] = 27;
               break;
            case 'x':
            case 'X':
            {
               UWORD i;
               UBYTE cnt = 0;
               UBYTE c;

               for(i=0; i<2; i++)
               {
                  c = *help;
                  cnt <<= 4;     /* mit 16 multiplizieren */

                  if(c >= '0' && c <= '9')
                     cnt += c - '0';
                  else
                  {
                     if(c >= 'A' && c <= 'F')
                        cnt += 9 + (c - ('A'-1));
                     else
                     {
                        if(c >= 'a' && c <= 'f')
                           cnt += 9 + (c - ('a'-1));
                     }
                  }
                  help++;
               }
               buffer[offs++] = cnt;
            }
               break;
            case '\\':
               buffer[offs++] = '\\';
               break;
         }
      }
      len--;
   }
   buffer[offs] = '\0';
}

///
/// DoMainMethod
ULONG DoMainMethod(Object *obj, LONG MethodID, APTR data1, APTR data2, APTR data3)
{
   struct MainMessage *message, *reply_message;
   struct MsgPort *reply_port;
   ULONG ret = NULL;
   BOOL finished = FALSE;
   ULONG sig;

   if(reply_port = CreateMsgPort())
   {
      if(message = AllocVec(sizeof(struct MainMessage), MEMF_ANY | MEMF_CLEAR))
      {
         message->msg.mn_ReplyPort  = reply_port;
         message->msg.mn_Length     = sizeof(struct MainMessage);

         message->obj      = obj;
         message->MethodID = MethodID;
         message->data1    = data1;
         message->data2    = data2;
         message->data3    = data3;

         PutMsg(MainPort, (struct Message *)message);

         while(!finished)
         {
            sig = Wait(1L << reply_port->mp_SigBit);  // must not be interuptable by ctrl_c because HandleMainMethod() needs the reply_port and the message to be still alive when it does a ReplyMsg()

            if(sig & (1L << reply_port->mp_SigBit))
            {
               while(reply_message = (struct MainMessage *)GetMsg(reply_port))
               {
                  if(reply_message->MethodID == MUIM_Genesis_Handshake)
                  {
                     ret = reply_message->result;
                     finished = TRUE;
                  }
               }
            }
         }
         FreeVec(message);
      }
      DeleteMsgPort(reply_port);
   }
   return(ret);
}

///

/// clear_list
VOID clear_list(struct MinList *list)
{
   if((struct MinList *)list->mlh_TailPred != list)
   {
      struct MinNode *entry1, *entry2;

      entry1 = (struct MinNode *)list->mlh_Head;
      while(entry2 = entry1->mln_Succ)
      {
         Remove((struct Node *)entry1);
         FreeVec(entry1);
         entry1 = entry2;
      }
   }
}

///
/// add_server
struct ServerEntry *add_server(struct MinList *list, STRPTR name)
{
   struct ServerEntry *server;

   if(!name || !*name || !strcmp(name, "0.0.0.0"))
      return(NULL);

   if(server = AllocVec(sizeof(struct ServerEntry), MEMF_ANY))
   {
      strncpy(server->se_name, name, sizeof(server->se_name) - 1);
      AddTail((struct List *)list, (struct Node *)server);
   }
   return(server);
}

///
/// find_server_by_name
struct ServerEntry *find_server_by_name(struct MinList *list, STRPTR name)
{
   struct ServerEntry *server;

   if(list->mlh_TailPred != (struct MinNode *)list)
   {
      server = (struct ServerEntry *)list->mlh_Head;
      while(server->se_node.mln_Succ)
      {
         if(!strcmp(server->se_name, name))
            return(server);
         server = (struct ServerEntry *)server->se_node.mln_Succ;
      }
   }
   return(NULL);
}

///

#define ENCODE(c) (c ? (c & 0x3F) + 0x20 : 0x60)
/// encrypt
VOID encrypt(STRPTR in, STRPTR out)
{
   LONG n, i;
   UBYTE c;
   STRPTR s, t;

   n = strlen(in);
   if (n > 0)
   {
      s = out;
      *s++ = ENCODE(n);

      for (i = 0; i < n; i += 3)
      {
         t = &in[i];

         c = t[0] >> 2;
         *s++ = ENCODE(c);
         c = (t[0] << 4) & 0x30 | (t[1] >> 4) & 0x0F;
         *s++ = ENCODE(c);
         c = (t[1] << 2) & 0x3C | (t[2] >> 6) & 0x03;
         *s++ = ENCODE(c);
         c = t[2] & 0x3F;
         *s++ = ENCODE(c);
      }
      *s = NULL;
   }
}

///
/// save_config
BOOL save_config(STRPTR file, struct ISP *isp, struct Interface *iface, struct Config *config)
{
   struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);
   BPTR fh;
   STRPTR ptr;
   char buff[110];

   if(!isp)
      return(FALSE);

   if(fh = Open(file, MODE_READWRITE))
   {
      Seek(fh, 0, OFFSET_END);
      FPrintf(fh, "## This config was generated by Genesis Wizard\n##\n\n");

      FPrintf(fh, "SerialDevice    %ls\n", config->cnf_serialdevice);
      FPrintf(fh, "SerialUnit      %ls\n", config->cnf_serialunit);
      FPrintf(fh, "BaudRate        38400\n");
      FPrintf(fh, "SerBufLen       16384\n\n");

      FPrintf(fh, "Modem           %ls\n", config->cnf_modemname);
      FPrintf(fh, "InitString      %ls\n", config->cnf_initstring);
      FPrintf(fh, "DialPrefix      %ls\n", config->cnf_dialprefix);
      FPrintf(fh, "RedialAttempts  10\n");
      FPrintf(fh, "RedialDelay     5\n\n");
      FPrintf(fh, "IgnoreDSR       %ls\n", (config->cnf_flags & CFL_IgnoreDSR ? "yes" : "no"));
      FPrintf(fh, "7Wire           yes\n");
      FPrintf(fh, "RadBoogie       no\n");

      FPrintf(fh, "## User, isp and interface configuration\n##\n\n");

      FPrintf(fh, "\nISP\n");
      FPrintf(fh, "Name            %ls\n", isp->isp_name);
      if(*isp->isp_comment)
         FPrintf(fh, "Comment         %ls\n", isp->isp_comment);
      if(*isp->isp_login)
         FPrintf(fh, "Login           %ls\n", isp->isp_login);
      if(*isp->isp_password)
      {
         encrypt(isp->isp_password, buff);
         FPrintf(fh, "Password        \"%ls\"\n", buff);
      }
      if(*isp->isp_phonenumber)
         FPrintf(fh, "Phone           %ls\n", isp->isp_phonenumber);
      if(*isp->isp_organisation)
         FPrintf(fh, "Organisation    %ls\n", isp->isp_organisation);

      if(*isp->isp_bootp)
         FPrintf(fh, "BOOTPServer     %ls\n", isp->isp_bootp);
      if(isp->isp_flags & ISF_UseBootp)
         FPrintf(fh, "UseBOOTP           yes\n");
      if(isp->isp_flags & ISF_DontQueryHostname)
         FPrintf(fh, "DontQueryHostname  yes\n");

if(*isp->isp_hostname)
   FPrintf(fh, "HostName        %ls\n", isp->isp_hostname);

      if(isp->isp_nameservers.mlh_TailPred != (struct MinNode *)&isp->isp_nameservers)
      {
         struct ServerEntry *server;

         server = (struct ServerEntry *)isp->isp_nameservers.mlh_Head;
         while(server->se_node.mln_Succ)
         {
            FPrintf(fh, "NameServer         %ls\n", server->se_name);
            server = (struct ServerEntry *)server->se_node.mln_Succ;
         }
      }
      if(isp->isp_domainnames.mlh_TailPred != (struct MinNode *)&isp->isp_domainnames)
      {
         struct ServerEntry *server;

         server = (struct ServerEntry *)isp->isp_domainnames.mlh_Head;
         while(server->se_node.mln_Succ)
         {
            FPrintf(fh, "DomainName         %ls\n", server->se_name);
            server = (struct ServerEntry *)server->se_node.mln_Succ;
         }
      }

      // interfaces (only one => don't use ISP's list)

      FPrintf(fh, "\nINTERFACE\n");
      FPrintf(fh, "IfName          %ls\n", iface->if_name);
      FPrintf(fh, "Sana2Device     %ls\n", iface->if_sana2device);
      FPrintf(fh, "Sana2Config     %ls\n", iface->if_sana2config);

      if(iface->if_flags & IFL_PPP)
      {
         FPrintf(fh, "Sana2ConfigText    sername %ls\\nserunit %ld\\nserbaud 38400\\nlocalipaddress %%a\\ncd yes\\n", config->cnf_serialdevice, config->cnf_serialunit);
         if(*isp->isp_login)
            FPrintf(fh, "user %%u\\n");
         if(*isp->isp_password)
            FPrintf(fh, "secret %%p\\n");
         FPrintf(fh, "\n");
         FPrintf(fh, "DefaultPPP\n");
      }
      else if(iface->if_flags & IFL_SLIP)
      {
         FPrintf(fh, "Sana2ConfigText    %ls %ld %ld Shared %%a MTU=%ld CD",
            config->cnf_serialdevice, config->cnf_serialunit, iface->if_MTU);
         if(config->cnf_flags & CFL_7Wire)
            FPrintf(fh, " 7Wire");
         FPrintf(fh, "\n");
         FPrintf(fh, "DefaultSLIP\n");
      }
      else if(iface->if_sana2configtext && *iface->if_sana2configtext)
      {
         FPrintf(fh, "Sana2ConfigText    ");

         ptr = iface->if_sana2configtext;
         while(*ptr)
         {
            if(*ptr == '\n')
               FPrintf(fh, "\\n");
            else
               FPrintf(fh, "%lc", *ptr);
            ptr++;
         }
         FPrintf(fh, "\n");
      }
      if(iface->if_configparams && *iface->if_configparams)
         FPrintf(fh, "IfConfigParams  %ls\n", iface->if_configparams);
      FPrintf(fh, "MTU             %ld\n", iface->if_MTU);
      FPrintf(fh, "CarrierDetect   yes\n");
if(*iface->if_addr)
   FPrintf(fh, "IPAddr          %ls\n", iface->if_addr);
if(*iface->if_dst)
   FPrintf(fh, "DestIP          %ls\n", iface->if_dst);
if(*iface->if_gateway)
   FPrintf(fh, "Gateway         %ls\n", iface->if_gateway);
if(*iface->if_netmask)
   FPrintf(fh, "Netmask         %ls\n", iface->if_netmask);

      if(isp->isp_loginscript.mlh_TailPred != (struct MinNode *)&isp->isp_loginscript)
      {
         struct ScriptLine *script_line;

         FPrintf(fh, "\nLOGINSCRIPT\n");
         script_line = (struct ScriptLine *)isp->isp_loginscript.mlh_Head;
         while(script_line->sl_node.mln_Succ)
         {
            if(*script_line->sl_contents)
               FPrintf(fh, "%ls \"%ls\"\n", script_commands[script_line->sl_command], script_line->sl_contents);
            else
               FPrintf(fh, "%ls\n", script_commands[script_line->sl_command]);
            script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
         }
      }

      FPrintf(fh, "\nUSER\n");
      if(*isp->isp_login)
         FPrintf(fh, "Name         %ls\n", isp->isp_login);
      FPrintf(fh, "RealName     Genesis Wizard\n");
      Close(fh);
   }
   else
      return(FALSE);

   return(TRUE);
}

///
/// print_config
VOID print_config(BPTR fh, struct ISP *isp, struct Interface *iface, struct Config *config)
{
   struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);

   FPrintf(fh, GetStr(MSG_TX_InfoTitle));

   FPrintf(fh, "%ls %ls, unit %ld (38'400 baud)\n\n", GetStr(MSG_LA_Device), config->cnf_serialdevice, config->cnf_serialunit);

   FPrintf(fh, "%ls %ls\n", GetStr(MSG_LA_Modem), config->cnf_modemname);
   FPrintf(fh, "%ls %ls\n", GetStr(MSG_LA_InitString), config->cnf_initstring);
   FPrintf(fh, "%ls %ls\n", GetStr(MSG_LA_DialPrefix), config->cnf_dialprefix);

   FPrintf(fh, "%ls %ls\n\n", GetStr(MSG_LA_Phone), isp->isp_phonenumber);

   FPrintf(fh, "%ls %ls\n", GetStr(MSG_LA_LoginName), isp->isp_login);

   FPrintf(fh, "%ls %ls\n\n\n", GetStr(MSG_LA_Protocol), iface->if_name);

   if(addr_assign = CNF_Assign_Static)
      FPrintf(fh, "%ls %ls\n", GetStr(MSG_LA_IPAddress), iface->if_addr);
   else
      FPrintf(fh, "%ls %ls (%ls %ls)\n", GetStr(MSG_LA_IPAddress), GetStr(MSG_TX_Dynamic), GetStr(MSG_TX_ObtainedBy), (addr_assign == CNF_Assign_BootP ? "BOOTP" : "ICPC"));
   if(dst_assign = CNF_Assign_Static)
      FPrintf(fh, "%ls %ls\n", GetStr(MSG_LA_RemoteIPAddress), iface->if_dst);
   else
      FPrintf(fh, "%ls %ls (%ls %ls)\n", GetStr(MSG_LA_RemoteIPAddress), GetStr(MSG_TX_Dynamic), GetStr(MSG_TX_ObtainedBy), (dst_assign == CNF_Assign_BootP ? "BOOTP" : "ICPC"));
   FPrintf(fh, "%ls %ls\n\n", GetStr(MSG_LA_Netmask), iface->if_netmask);

/*
   FPrintf(fh, "%ls %ls (%ls %ls)\n\n", GetStr(MSG_LA_DomainName), isp->isp_domainname, GetStr(MSG_TX_ObtainedBy), (domainname_assign == CNF_Assign_BootP ? "BOOTP" : "DNSQuery"));

   if(dns_assign == CNF_Assign_Root)
   {
      FPrintf(fh, "%ls %ls (Root DNS)\n", GetStr(MSG_LA_DNS1IPAddr), isp->isp_dns1);
      FPrintf(fh, "%ls %ls (Root DNS)\n", GetStr(MSG_LA_DNS2IPAddr), isp->isp_dns2);
   }
   else
   {
      FPrintf(fh, "%ls %ls (%ls %ls)\n", GetStr(MSG_LA_DNS1IPAddr), isp->isp_dns1, GetStr(MSG_TX_ObtainedBy), (dns_assign == CNF_Assign_BootP ? "BOOTP" : "MSDNS"));
      FPrintf(fh, "%ls %ls (%ls %ls)\n", GetStr(MSG_LA_DNS2IPAddr), isp->isp_dns2, GetStr(MSG_TX_ObtainedBy), (dns_assign == CNF_Assign_BootP ? "BOOTP" : "MSDNS"));
   }
*/

   if(isp->isp_loginscript.mlh_TailPred != (struct MinNode *)&isp->isp_loginscript)
   {
      struct ScriptLine *script_line;

      FPrintf(fh, "\n\n%ls\n\n", GetStr(MSG_LA_LoginScript));
      script_line = (struct ScriptLine *)isp->isp_loginscript.mlh_Head;
      while(script_line->sl_node.mln_Succ)
      {
         if(*script_line->sl_contents)
            FPrintf(fh, "%ls \"%ls\"\n", script_commands[script_line->sl_command], script_line->sl_contents);
         else
            FPrintf(fh, "%ls\n", script_commands[script_line->sl_command]);
         script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
      }
   }
}

///
/// run_async
BOOL run_async(STRPTR file)
{
   BPTR ofh = NULL, ifh = NULL;
   BOOL success = FALSE;

   if(ofh = Open("NIL:", MODE_NEWFILE))
   {
      if(ifh = Open("NIL:", MODE_OLDFILE))
      {
         if(SystemTags(file,
            SYS_Output     , ofh,
            SYS_Input      , ifh,
            SYS_Asynch     , TRUE,
            SYS_UserShell  , TRUE,
            NP_StackSize   , 8192,
            TAG_DONE) != -1)
               success = TRUE;

         if(!success)
            Close(ifh);
      }
      if(!success)
         Close(ofh);
   }
   return(success);
}

///
/// launch_amitcp
BOOL launch_amitcp(VOID)
{
   int i;

   if(!FindPort((STRPTR)AmiTCP_PortName))
      run_async("AmiTCP:kernel/AmiTCP");

   /* wait until AmiTCP is running */
   i = 0;
   while(!FindPort((STRPTR)AmiTCP_PortName) && i++ < 30)
      Delay(25);

   if(FindPort((STRPTR)AmiTCP_PortName))
      return(TRUE);

   return(FALSE);
}

///
/// syslog_AmiTCP
VOID syslog_AmiTCP(ULONG level, const STRPTR format, ...)
{
   va_list va;

   va_start(va, format);
   if(SocketBase)
      vsyslog(level, format, (LONG *)va);
   va_end(va);
}

///
/// have_ppp_frame
static UWORD fcstab[256] =
{
   0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
   0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
   0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
   0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
   0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
   0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
   0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
   0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
   0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
   0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
   0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
   0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
   0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
   0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
   0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
   0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
   0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
   0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
   0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
   0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
   0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
   0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
   0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
   0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
   0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
   0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
   0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
   0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
   0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
   0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
   0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
   0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

#define PPP_HARD_HDR_LEN      4

#define PPP_INITFCS  0xffff
#define PPP_GOODFCS  0xf0b8
#define PPP_FCS(fcs, c) (((fcs) >> 8) ^ fcstab[((fcs) ^ (c)) & 0xff])
#define  PPP_FLAG 0x7e
#define  PPP_ESCAPE  0x7d
#define  PPP_TRANS   0x20
#define IN_RMAP(c)   ((((ULONG) (UBYTE) (c)) < 0x20) && recv_async_map & (1 << (c)))

#define buf_size 1500

BOOL have_ppp_frame(UBYTE *data, ULONG count)
{
   UBYTE chr;
   BOOL have_frame = FALSE;
   UWORD buf_fcs = PPP_INITFCS;
   UWORD buf_count = 0;
   UBYTE escape = 0, toss = 0xe0;
   ULONG recv_async_map = 0;

   while (count-- > 0)
   {
      chr = *data++;

      switch((ULONG)chr)
      {
      case PPP_FLAG: /* end of frame */
         if(escape)
            toss |= 0x80;

         if((toss == 0) && (buf_count >= PPP_HARD_HDR_LEN) && (buf_fcs == PPP_GOODFCS))
            have_frame = TRUE;

         buf_count   = 0;
         buf_fcs     = PPP_INITFCS;
         escape      = 0;
         toss     = 0;
         break;

      default:
         if(toss != 0)
            break;

         /* ignore char? */
         if(IN_RMAP(chr))
            break;

         /* check esc seq */
         if(escape)
         {
            chr ^= escape;
            escape = 0;
         }
         else
         if(chr == PPP_ESCAPE)
         {
            escape = PPP_TRANS;
            break;
         }

         if(buf_count < buf_size)
         {
            buf_count++;
            buf_fcs = PPP_FCS (buf_fcs, chr);
            break;
         }

         /* too much data */
         toss |= 0xC0;
         break;
      }
   }

   return(have_frame);
}
///
/// amirexx_do_command
LONG amirexx_do_command(const char *fmt, ...)
{
   char buf[256];
   struct MsgPort *port; /* our reply port */
   struct MsgPort *AmiTCP_Port;
   struct RexxMsg *rmsg;
   LONG rc = 20;         /* fail */

   vsprintf(buf, fmt, (va_list)(&fmt + (va_list)1));
   if(port = CreateMsgPort())
   {
      port->mp_Node.ln_Name = "BOOTPCONFIG";
      if(rmsg = CreateRexxMsg(port, NULL, (STRPTR)AmiTCP_PortName))
      {
         rmsg->rm_Action = RXCOMM;
         rmsg->rm_Args[0] = (STRPTR)buf;
         if(FillRexxMsg(rmsg, 1, 0))
         {
            Forbid();
            if(AmiTCP_Port = FindPort((STRPTR)AmiTCP_PortName))
            {
               PutMsg(AmiTCP_Port, (struct Message *)rmsg);
               Permit();
               do
               {
                  WaitPort(port);
               } while(GetMsg(port) != (struct Message *)rmsg);
               rc = rmsg->rm_Result1;
            }
            else
               Permit();

            ClearRexxMsg(rmsg, 1);
         }
         DeleteRexxMsg(rmsg);
      }
      DeleteMsgPort(port);
   }
   return(rc);
}

///

/// strobjfunc
SAVEDS LONG strobjfunc(register __a2 Object *list, register __a1 Object *str)
{
   char *x, *s;
   int i;

   get(str, MUIA_String_Contents, &s);

   i = 0;
   FOREVER
   {
      DoMethod(list, MUIM_List_GetEntry, i, &x);
      if(!x)
      {
         nnset(list, MUIA_List_Active, MUIV_List_Active_Off);
         break;
      }
      else
      {
         if(!stricmp(x, s))
         {
            nnset(list, MUIA_List_Active, i);
            break;
         }
      }

      i++;
   }
   return(TRUE);
}

///
/// txtobjfunc
SAVEDS LONG txtobjfunc(register __a2 Object *list, register __a1 Object *txt)
{
   char *x, *s;
   int i;

   get(txt, MUIA_Text_Contents, &s);
   i = 0;
   FOREVER
   {
      DoMethod(list, MUIM_List_GetEntry, i, &x);
      if(!x)
      {
         nnset(list, MUIA_List_Active, MUIV_List_Active_Off);
         break;
      }
      else
      {
         if(!stricmp(x, s))
         {
            nnset(list, MUIA_List_Active, i);
            break;
         }
      }

      i++;
   }
   return(TRUE);
}

///
/// objstrfunc
VOID SAVEDS objstrfunc(register __a2 Object *list,register __a1 Object *str)
{
   char *x;

   DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &x);
   if(x)
      set(str, MUIA_String_Contents, x);
}

///
/// objtxtfunc
VOID SAVEDS objtxtfunc(register __a2 Object *list,register __a1 Object *txt)
{
   char *x;

   DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &x);
   if(x)
      set(txt, MUIA_Text_Contents, x);
}

///
/// desfunc
VOID SAVEDS desfunc(register __a2 APTR pool, register __a1 APTR *entry)
{
   if(entry)
      FreeVec(entry);
}

///
/// sortfunc
SAVEDS LONG sortfunc(register __a1 STRPTR str1, register __a2 STRPTR str2)
{
   return(stricmp(str1, str2));
}

///

