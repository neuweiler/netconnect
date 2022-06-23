/// includes
#include "/includes.h"

#include "/Genesis.h"
#include "/genesis.lib/libraries/genesis.h"
#include "/genesis.lib/proto/genesis.h"
#include "/genesis.lib/pragmas/genesis_lib.h"
#include <devices/netinfo.h>
#include "Strings.h"
#include "mui.h"
#include "mui_MainWindow.h"
#include "protos.h"
#include "rev.h"

///
/// external variables
extern struct Library *MUIMasterBase, *GenesisBase, *UserGroupBase;
extern struct ExecBase *SysBase;
extern struct Catalog          *cat;
extern struct MUI_CustomClass  *CL_MainWindow;
extern char serial_in[];
extern BOOL ReadQueued, no_carrier, use_old_modem;
extern Object *app, *win;
extern int addr_assign, dst_assign, dns_assign, domainname_assign, gw_assign;
extern const char AmiTCP_PortName[];
extern struct Modem Modem;
extern struct Interface Iface;
extern struct MsgPort *MainPort;

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
      MUIA_Text_Contents,  string,     // don't use GetStr() here !! once and forever !
   End;

   return(obj);
}

///
/// MakeString
Object *MakeString(STRPTR string, LONG len)
{
   return(StringObject,
      StringFrame,
      MUIA_CycleChain      , 1,
      MUIA_String_MaxLen   , len,
      MUIA_String_Contents , string,
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
/// add_server
struct ServerEntry *add_server(struct MinList *list, STRPTR name)
{
   struct ServerEntry *server;

   if(!name || !*name || !strcmp(name, "0.0.0.0"))
      return(NULL);

   if(server = find_server_by_name(list, name))
   {
      Remove((struct Node *)server);
      FreeVec(server);
   }
   if(server = AllocVec(sizeof(struct ServerEntry), MEMF_ANY))
   {
      strncpy(server->se_name, name, sizeof(server->se_name) - 1);
      AddTail((struct List *)list, (struct Node *)server);
   }
   return(server);
}

///

#define ENCODE(c) (c ? (c & 0x3F) + 0x20 : 0x60)
/// is_true
BOOL is_true(struct ParseConfig_Data *pc_data)
{
   if(!*pc_data->pc_contents || !stricmp(pc_data->pc_contents, "yes") ||
      !stricmp(pc_data->pc_contents, "true") || !strcmp(pc_data->pc_contents, "1"))
      return(TRUE);
   return(FALSE);
}

///
/// is_false
BOOL is_false(struct ParseConfig_Data *pc_data)
{
   if(!stricmp(pc_data->pc_contents, "no") || !stricmp(pc_data->pc_contents, "false") ||
      !strcmp(pc_data->pc_contents, "0"))
      return(TRUE);
   return(FALSE);
}

///
/// yes_no
const STRPTR yes_no(BOOL value)
{
   return((value ? "yes" : "no"));
}

///
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

/// clear_config
VOID clear_config(VOID)
{
   if(Modem.mo_serreq)
      serial_delete();
   bzero(&Modem, sizeof(struct Modem));

   Modem.mo_id = ((int)(&Modem) % 100) + 100;
   strcpy(Modem.mo_name   , "Generic");
   strcpy(Modem.mo_comment, "GENESiS Wizard");
   strcpy(Modem.mo_device, "serial.device");
   Modem.mo_baudrate = 38400;
   Modem.mo_serbuflen = 16384;
   Modem.mo_flags = MFL_7Wire | MFL_RadBoogie | MFL_DropDTR;
   strcpy(Modem.mo_init       , "AT&F&D2\\r");
   strcpy(Modem.mo_dialprefix , "ATDT");
   strcpy(Modem.mo_dialsuffix , "\\r");
   strcpy(Modem.mo_answer     , "ATA\\r");
   strcpy(Modem.mo_hangup     , "~~~+++~~~ATH0\\r");
   strcpy(Modem.mo_ring       , "RING");
   strcpy(Modem.mo_connect    , "CONNECT");
   strcpy(Modem.mo_nocarrier  , "NO CARRIER");
   strcpy(Modem.mo_nodialtone , "NO DIALTONE");
   strcpy(Modem.mo_busy       , "BUSY");
   strcpy(Modem.mo_ok         , "OK");
   strcpy(Modem.mo_error      , "ERROR");
   Modem.mo_redialattempts = 10;
   Modem.mo_redialdelay    = 5;
   Modem.mo_commanddelay   = 20;

   // do NOT free sana2configtext or configparams here ! they point to a global variable
   clear_list(&Iface.if_loginscript);
   clear_list(&Iface.if_events);
   clear_list(&Iface.if_nameservers);
   clear_list(&Iface.if_domainnames);
   bzero(&Iface, sizeof(struct Interface));
   Iface.if_modemid = Modem.mo_id;
   strcpy(Iface.if_comment, "GENESiS Wizard");
   strcpy(Iface.if_netmask, "255.255.255.255");
   Iface.if_MTU = 1500;
   NewList((struct List *)&Iface.if_events);
   NewList((struct List *)&Iface.if_loginscript);
   NewList((struct List *)&Iface.if_nameservers);
   NewList((struct List *)&Iface.if_domainnames);
}

///
/// load_config
BOOL load_config(STRPTR file)
{
   struct ParseConfig_Data pc_data;
   BOOL first_modem = FALSE;

   clear_config();

   if(ParseConfig(file, &pc_data))
   {
      if(ParseNext(&pc_data))
      {
         if(!strcmp(pc_data.pc_argument, "##") && !strcmp(pc_data.pc_contents, "This file was generated by GENESiS Prefs"))
         {
            ParseEnd(&pc_data);
            SystemTags("AmiTCP:genesis_converter", TAG_DONE);
            if(!ParseConfig(file , &pc_data))
               return(FALSE);
         }
      }

      while(ParseNext(&pc_data))
      {
         if(!stricmp(pc_data.pc_argument, "MODEM"))
         {
            if(first_modem)   // only read the first modem
               break;
            else
               first_modem = TRUE;
         }
         else if(!stricmp(pc_data.pc_argument, "ModemID"))
            Modem.mo_id = atol(pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "ModemName"))
            strncpy(Modem.mo_name, pc_data.pc_contents, sizeof(Modem.mo_name));
         else if(!stricmp(pc_data.pc_argument, "ModemComment"))
            strncpy(Modem.mo_comment, pc_data.pc_contents, sizeof(Modem.mo_comment));

         if(!stricmp(pc_data.pc_argument, "SerialDevice"))
            strncpy(Modem.mo_device, pc_data.pc_contents, sizeof(Modem.mo_device));
         else if(!stricmp(pc_data.pc_argument, "SerialUnit"))
            Modem.mo_unit = atol(pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "BaudRate"))
            Modem.mo_baudrate = atol(pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "SerBufLen"))
            Modem.mo_serbuflen = atol(pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "IgnoreDSR") && is_true(&pc_data))
            Modem.mo_flags |= MFL_IgnoreDSR;
         else if(!stricmp(pc_data.pc_argument, "7Wire") && is_false(&pc_data))
            Modem.mo_flags &= ~MFL_7Wire;
         else if(!stricmp(pc_data.pc_argument, "RadBoogie") && is_false(&pc_data))
            Modem.mo_flags &= ~MFL_RadBoogie;
         else if(!stricmp(pc_data.pc_argument, "XonXoff") && is_true(&pc_data))
            Modem.mo_flags |= MFL_XonXoff;
         else if(!stricmp(pc_data.pc_argument, "OwnDevUnit") && is_true(&pc_data))
            Modem.mo_flags |= MFL_OwnDevUnit;
         else if(!stricmp(pc_data.pc_argument, "DropDTR") && is_false(&pc_data))
            Modem.mo_flags &= ~MFL_DropDTR;

         else if(!stricmp(pc_data.pc_argument, "InitString"))
            strncpy(Modem.mo_init, pc_data.pc_contents, sizeof(Modem.mo_init));
         else if(!stricmp(pc_data.pc_argument, "DialPrefix"))
            strncpy(Modem.mo_dialprefix, pc_data.pc_contents, sizeof(Modem.mo_dialprefix));
         else if(!stricmp(pc_data.pc_argument, "DialSuffix"))
            strncpy(Modem.mo_dialsuffix, pc_data.pc_contents, sizeof(Modem.mo_dialsuffix));
         else if(!stricmp(pc_data.pc_argument, "Answer"))
            strncpy(Modem.mo_answer, pc_data.pc_contents, sizeof(Modem.mo_answer));
         else if(!stricmp(pc_data.pc_argument, "Hangup"))
            strncpy(Modem.mo_hangup, pc_data.pc_contents, sizeof(Modem.mo_hangup));

         else if(!stricmp(pc_data.pc_argument, "Ring"))
            strncpy(Modem.mo_ring, pc_data.pc_contents, sizeof(Modem.mo_ring));
         else if(!stricmp(pc_data.pc_argument, "Connect"))
            strncpy(Modem.mo_connect, pc_data.pc_contents, sizeof(Modem.mo_connect));
         else if(!stricmp(pc_data.pc_argument, "NoCarrier"))
            strncpy(Modem.mo_nocarrier, pc_data.pc_contents, sizeof(Modem.mo_nocarrier));
         else if(!stricmp(pc_data.pc_argument, "NoDialtone"))
            strncpy(Modem.mo_nodialtone, pc_data.pc_contents, sizeof(Modem.mo_nodialtone));
         else if(!stricmp(pc_data.pc_argument, "Busy"))
            strncpy(Modem.mo_busy, pc_data.pc_contents, sizeof(Modem.mo_busy));
         else if(!stricmp(pc_data.pc_argument, "Ok"))
            strncpy(Modem.mo_ok, pc_data.pc_contents, sizeof(Modem.mo_ok));
         else if(!stricmp(pc_data.pc_argument, "Error"))
            strncpy(Modem.mo_error, pc_data.pc_contents, sizeof(Modem.mo_error));

         else if(!stricmp(pc_data.pc_argument, "RedialAttempts"))
            Modem.mo_redialattempts = atol(pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "RedialDelay"))
            Modem.mo_redialdelay = atol(pc_data.pc_contents);
         else if(!stricmp(pc_data.pc_argument, "CommandDelay"))
            Modem.mo_commanddelay = atol(pc_data.pc_contents);
      }

      if(!strstr(Modem.mo_dialsuffix, "\\r"))
         strcat(Modem.mo_dialsuffix, "\\r");
      if(!strchr(Modem.mo_init, '\\r'))
         strcat(Modem.mo_init, "\\r");

      use_old_modem = first_modem;

      ParseEnd(&pc_data);
      return(TRUE);
   }
   return(FALSE);
}

///
/// save_config
BOOL save_config(STRPTR file)
{
   struct ServerEntry *server;
   struct ScriptLine *script_line;
   BPTR fh;
   STRPTR ptr;
   char buff[110];

   Iface.if_modemid = Modem.mo_id;  // they HAVE to match !

   if(fh = Open(file, MODE_READWRITE))
   {
      Seek(fh, 0, OFFSET_END);
      FPrintf(fh, "## This config was generated by GENESiS Wizard "VERSIONSTRING"\n##\n\n");

      if((Iface.if_modemid > 0) && !use_old_modem)
      {
         FPrintf(fh, "MODEM\n");
         FPrintf(fh, "ModemID             %ld\n", Modem.mo_id);
         FPrintf(fh, "ModemName           %ls\n", Modem.mo_name);
         FPrintf(fh, "ModemComment        %ls\n", Modem.mo_comment);
         FPrintf(fh, "SerialDevice        %ls\n", Modem.mo_device);
         FPrintf(fh, "SerialUnit          %ld\n", Modem.mo_unit);
         FPrintf(fh, "BaudRate            %ld\n", Modem.mo_baudrate);
         FPrintf(fh, "SerBufLen           %ld\n", Modem.mo_serbuflen);
         FPrintf(fh, "IgnoreDSR           %ls\n", yes_no(Modem.mo_flags & MFL_IgnoreDSR));
         FPrintf(fh, "7Wire               %ls\n", yes_no(Modem.mo_flags & MFL_7Wire));
         FPrintf(fh, "RadBoogie           %ls\n", yes_no(Modem.mo_flags & MFL_RadBoogie));
         FPrintf(fh, "XonXoff             %ls\n", yes_no(Modem.mo_flags & MFL_XonXoff));
         FPrintf(fh, "OwnDevUnit          %ls\n", yes_no(Modem.mo_flags & MFL_OwnDevUnit));
         FPrintf(fh, "DropDTR             %ls\n", yes_no(Modem.mo_flags & MFL_DropDTR));

         FPrintf(fh, "InitString          %ls\n", Modem.mo_init);
         FPrintf(fh, "DialPrefix          %ls\n", Modem.mo_dialprefix);
         FPrintf(fh, "DialSuffix          %ls\n", Modem.mo_dialsuffix);
         FPrintf(fh, "Answer              %ls\n", Modem.mo_answer);
         FPrintf(fh, "HangUp              %ls\n", Modem.mo_hangup);
         FPrintf(fh, "RedialAttempts      %ld\n", Modem.mo_redialattempts);
         FPrintf(fh, "RedialDelay         %ld\n", Modem.mo_redialdelay);
         FPrintf(fh, "CommandDelay        %ld\n", Modem.mo_commanddelay);

         FPrintf(fh, "Ring                %ls\n", Modem.mo_ring);
         FPrintf(fh, "Connect             %ls\n", Modem.mo_connect);
         FPrintf(fh, "NoCarrier           %ls\n", Modem.mo_nocarrier);
         FPrintf(fh, "NoDialtone          %ls\n", Modem.mo_nodialtone);
         FPrintf(fh, "Busy                %ls\n", Modem.mo_busy);
         FPrintf(fh, "Ok                  %ls\n", Modem.mo_ok);
         FPrintf(fh, "Error               %ls\n", Modem.mo_error);
      }

      FPrintf(fh, "\nINTERFACE\n");
      FPrintf(fh, "IfName              %ls\n", Iface.if_name);
      FPrintf(fh, "IfComment           %ls\n", Iface.if_comment);
      FPrintf(fh, "Sana2Device         %ls\n", Iface.if_sana2device);
      FPrintf(fh, "Sana2Unit           %ld\n", Iface.if_sana2unit);
      FPrintf(fh, "Sana2Config         %ls\n", Iface.if_sana2config);

      if(Iface.if_flags & IFL_PPP)
      {
         FPrintf(fh, "Sana2ConfigText    \"sername %ls\\nserunit %ld\\nserbaud %ld\\nlocalipaddress %%a\\n",
            Modem.mo_device, Modem.mo_unit, Modem.mo_baudrate);
         FPrintf(fh, "sevenwire      %ld\\n", (Modem.mo_flags & MFL_7Wire ? 1 : 0));
         FPrintf(fh, "xonxoff        %ld\\n", (Modem.mo_flags & MFL_XonXoff ? 1 : 0));
         if(*Iface.if_login)
            FPrintf(fh, "user %%u\\n");
         if(*Iface.if_password)
            FPrintf(fh, "secret %%p\\n");
         FPrintf(fh, "cd             %ls\\n", yes_no(!no_carrier));
         FPrintf(fh, "mppcomp        no\\n");
         FPrintf(fh, "vjcomp         no\\n");
         FPrintf(fh, "bsdcomp        no\\n");
         FPrintf(fh, "deflatecomp    no\\n");
         FPrintf(fh, "eof            no\\n\"");
         FPrintf(fh, "\n");
         FPrintf(fh, "DefaultPPP\n");
         FPrintf(fh, "MPPCompression      no\n");
         FPrintf(fh, "VJCompression       no\n");
         FPrintf(fh, "BSDCompression      no\n");
         FPrintf(fh, "DeflateCompression  no\n");
         FPrintf(fh, "EOF                 no\n");
      }
      else if(Iface.if_flags & IFL_SLIP)
      {
         FPrintf(fh, "Sana2ConfigText     \"%ls %ld %ld Shared %%a MTU=%ld",
            Modem.mo_device, Modem.mo_unit, Modem.mo_baudrate, Iface.if_MTU);
         if(!no_carrier)
            FPrintf(fh, " CD");
         if(Modem.mo_flags & MFL_7Wire)
            FPrintf(fh, " 7Wire");
         FPrintf(fh, "\n");
         FPrintf(fh, "DefaultSLIP\n");
      }
      else if(Iface.if_sana2configtext && *Iface.if_sana2configtext)
      {
         FPrintf(fh, "Sana2ConfigText     \"");

         ptr = Iface.if_sana2configtext;
         while(*ptr)
         {
            if(*ptr == '\n')
               FPrintf(fh, "\\n");
            else
               FPrintf(fh, "%lc", *ptr);
            ptr++;
         }
         FPrintf(fh, "\"\n");
      }
      if(Iface.if_configparams && *Iface.if_configparams)
         FPrintf(fh, "IfConfigParams      %ls\n", Iface.if_configparams);
      if(*Iface.if_addr && addr_assign == ASSIGN_Static)
         FPrintf(fh, "IPAddr              %ls\n", Iface.if_addr);
      if(*Iface.if_dst && dst_assign == ASSIGN_Static)
         FPrintf(fh, "DestIP              %ls\n", Iface.if_dst);
      if(*Iface.if_gateway && gw_assign == ASSIGN_Static)
         FPrintf(fh, "Gateway             %ls\n", Iface.if_gateway);
      if(*Iface.if_netmask)
         FPrintf(fh, "Netmask             %ls\n", Iface.if_netmask);
      FPrintf(fh, "MTU                 %ld\n", Iface.if_MTU);
      FPrintf(fh, "CarrierDetect       %ls\n", yes_no(!no_carrier));
      if(Iface.if_flags & IFL_BOOTP)
         FPrintf(fh, "UseBOOTP\n");
      if(Iface.if_flags & IFL_UseDomainName)
         FPrintf(fh, "UseDomainname\n");
      if(Iface.if_flags & IFL_UseHostName)
         FPrintf(fh, "UseHostname\n");
      if(Iface.if_flags & IFL_UseNameServer)
         FPrintf(fh, "UseNameserver\n");

      if(*Iface.if_hostname && addr_assign == ASSIGN_Static)
         FPrintf(fh, "HostName            %ls\n", Iface.if_hostname);
      if(Iface.if_nameservers.mlh_TailPred != (struct MinNode *)&Iface.if_nameservers)
      {
         server = (struct ServerEntry *)Iface.if_nameservers.mlh_Head;
         while(server->se_node.mln_Succ)
         {
            FPrintf(fh, "NameServer          %ls\n", server->se_name);
            server = (struct ServerEntry *)server->se_node.mln_Succ;
         }
      }
      if(Iface.if_domainnames.mlh_TailPred != (struct MinNode *)&Iface.if_domainnames)
      {
         server = (struct ServerEntry *)Iface.if_domainnames.mlh_Head;
         while(server->se_node.mln_Succ)
         {
            FPrintf(fh, "DomainName          %ls\n", server->se_name);
            server = (struct ServerEntry *)server->se_node.mln_Succ;
         }
      }
      if(Iface.if_modemid > 0)
      {
         FPrintf(fh, "IfModemID           %ld\n", Iface.if_modemid);
         if(*Iface.if_login)
            FPrintf(fh, "Login           %ls\n", Iface.if_login);
         if(*Iface.if_password)
         {
            encrypt(Iface.if_password, buff);
            FPrintf(fh, "Password        \"%ls\"\n", buff);
         }
         if(*Iface.if_phonenumber)
            FPrintf(fh, "Phone           %ls\n", Iface.if_phonenumber);

         if(Iface.if_loginscript.mlh_TailPred != (struct MinNode *)&Iface.if_loginscript)
         {
            FPrintf(fh, "\nLOGINSCRIPT\n");
            script_line = (struct ScriptLine *)Iface.if_loginscript.mlh_Head;
            while(script_line->sl_node.mln_Succ)
            {
               if(*script_line->sl_contents)
                  FPrintf(fh, "%-19ls \"%ls\"\n", script_commands[script_line->sl_command], script_line->sl_contents);
               else
                  FPrintf(fh, "%ls\n", script_commands[script_line->sl_command]);
               script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
            }
         }
      }
      Close(fh);
   }
   else
      return(FALSE);


   if(*Iface.if_login)  // check if user already exists in db/passwd
   {
      struct   MsgPort         *netinfo_port  = NULL;
      struct   NetInfoReq      *netinfo_req   = NULL;

      if(netinfo_port = (struct MsgPort *)CreateMsgPort())
      {
         if(netinfo_req = (struct NetInfoReq *)CreateIORequest(netinfo_port, sizeof(struct NetInfoReq)))
         {
            if(!(OpenDevice(NETINFONAME, NETINFO_PASSWD_UNIT, (struct IORequest *)netinfo_req, 0)))
            {
               struct   NetInfoPasswd   *passwd;
               char     pw_buffer[256];

               bzero(pw_buffer, sizeof(pw_buffer));
               passwd = (struct NetInfoPasswd *)pw_buffer;
               passwd->pw_name = Iface.if_login;
               netinfo_req->io_Data    = passwd;
               netinfo_req->io_Length  = sizeof(pw_buffer);
               netinfo_req->io_Command = NI_GETBYNAME;
               if(DoIO((struct IORequest *)netinfo_req))    // user not found => create it
               {
                  int uid;
                  BPTR fh;
                  char dir_buf[81];

                  // find free uid first
                  uid = 99;
                  do
                  {
                     bzero(pw_buffer, sizeof(pw_buffer));
                     passwd = (struct NetInfoPasswd *)pw_buffer;
                     passwd->pw_uid = ++uid;
                     netinfo_req->io_Data    = passwd;
                     netinfo_req->io_Length  = sizeof(pw_buffer);
                     netinfo_req->io_Command = NI_GETBYID;
                  }  while(!(DoIO((struct IORequest *)netinfo_req)) && uid < 200);

                  if(fh = CreateDir("AmiTCP:home"))
                     UnLock(fh);
                  sprintf(dir_buf, "AmiTCP:home/%ls", Iface.if_login);
                  if(fh = CreateDir(dir_buf))
                     UnLock(fh);

                  passwd = (struct NetInfoPasswd *)pw_buffer;
                  passwd->pw_name   = Iface.if_login;
                  passwd->pw_passwd = "";
                  passwd->pw_uid    = uid;
                  passwd->pw_gid    = 100;
                  passwd->pw_gecos  = "GENESiS Wizard";
                  passwd->pw_dir    = dir_buf;
                  passwd->pw_shell  = "noshell";

                  netinfo_req->io_Data    = passwd;
                  netinfo_req->io_Command = CMD_WRITE;
                  DoIO((struct IORequest *)netinfo_req);
                  netinfo_req->io_Command = CMD_UPDATE;
                  DoIO((struct IORequest *)netinfo_req);

                  // a trick to update amitcp:db/utm.conf
                  DeleteFile("AmiTCP:db/utm.conf");
                  ReloadUserList();
               }

               CloseDevice((struct IORequest *)netinfo_req);
            }
            DeleteIORequest(netinfo_req);
         }
         DeleteMsgPort(netinfo_port);
      }
   }

   return(TRUE);
}

///
/// print_config
VOID print_config(BPTR fh)
{
   struct ServerEntry *server;
   struct ScriptLine *script_line;

   FPrintf(fh, GetStr(MSG_TX_InfoTitle));

   if(Iface.if_modemid > 0)
   {
      FPrintf(fh, "%ls, unit %ld (%ld baud)\n\n", Modem.mo_device, Modem.mo_unit, Modem.mo_baudrate);

      FPrintf(fh, "%-15ls %ls\n", GetStr(MSG_LA_Modem), Modem.mo_name);
      FPrintf(fh, "%-15ls %ls\n", GetStr(MSG_LA_InitString), Modem.mo_init);
      FPrintf(fh, "%-15ls %ls\n", GetStr(MSG_LA_DialPrefix), Modem.mo_dialprefix);
      if(*Iface.if_phonenumber)
         FPrintf(fh, "%-15ls %ls\n\n", GetStr(MSG_LA_Phone), Iface.if_phonenumber);
      if(*Iface.if_login)
         FPrintf(fh, "%-15ls %ls\n\n", GetStr(MSG_LA_LoginName), Iface.if_login);
   }

   FPrintf(fh, "%-15ls %ls\n", GetStr(MSG_LA_IfaceName), Iface.if_name);
   FPrintf(fh, "%-15ls %ls, unit %ld\n", GetStr(MSG_LA_Sana2Device), Iface.if_sana2device, Iface.if_sana2unit);

   if(addr_assign == ASSIGN_Static)
      FPrintf(fh, "%-15ls %ls\n", GetStr(MSG_LA_IPAddr), Iface.if_addr);
   else
      FPrintf(fh, "%-15ls %ls (%ls %ls)\n", GetStr(MSG_LA_IPAddr), GetStr(MSG_TX_Dynamic), GetStr(MSG_TX_ObtainedBy), (addr_assign == ASSIGN_BOOTP ? "BOOTP" : "ICPC"));
   if(dst_assign == ASSIGN_Static)
      FPrintf(fh, "%-15ls %ls\n", GetStr(MSG_LA_Destination), Iface.if_dst);
   else
      FPrintf(fh, "15%ls %ls (%ls %ls)\n", GetStr(MSG_LA_Destination), GetStr(MSG_TX_Dynamic), GetStr(MSG_TX_ObtainedBy), (dst_assign == ASSIGN_BOOTP ? "BOOTP" : "ICPC"));
   if(gw_assign == ASSIGN_Static && *Iface.if_gateway)
      FPrintf(fh, "%-15ls %ls\n", GetStr(MSG_LA_Gateway), Iface.if_gateway);
   FPrintf(fh, "%-15ls %ls\n\n", GetStr(MSG_LA_Netmask), Iface.if_netmask);

   if(Iface.if_domainnames.mlh_TailPred != (struct MinNode *)&Iface.if_domainnames)
   {
      server = (struct ServerEntry *)Iface.if_domainnames.mlh_Head;
      while(server->se_node.mln_Succ)
      {
         FPrintf(fh, "%-15ls %ls (%ls %ls)\n\n", GetStr(MSG_LA_DomainName), server->se_name, GetStr(MSG_TX_ObtainedBy), (domainname_assign == ASSIGN_BOOTP ? "BOOTP" : "DNSQuery"));
         server = (struct ServerEntry *)server->se_node.mln_Succ;
      }
   }
   if(Iface.if_nameservers.mlh_TailPred != (struct MinNode *)&Iface.if_nameservers)
   {
      server = (struct ServerEntry *)Iface.if_nameservers.mlh_Head;
      while(server->se_node.mln_Succ)
      {
         if(dns_assign == ASSIGN_Root)
            FPrintf(fh, "%-15ls %ls (Root DNS)\n", GetStr(MSG_LA_DNSIPAddr), server->se_name);
         else
            FPrintf(fh, "%-15ls %ls (%ls %ls)\n", GetStr(MSG_LA_DNSIPAddr), server->se_name, GetStr(MSG_TX_ObtainedBy), (dns_assign == ASSIGN_BOOTP ? "BOOTP" : "MSDNS"));
         server = (struct ServerEntry *)server->se_node.mln_Succ;
      }
   }
   if(Iface.if_loginscript.mlh_TailPred != (struct MinNode *)&Iface.if_loginscript)
   {
      FPrintf(fh, "\n\n%ls\n\n", GetStr(MSG_LA_LoginScript));
      script_line = (struct ScriptLine *)Iface.if_loginscript.mlh_Head;
      while(script_line->sl_node.mln_Succ)
      {
         if(*script_line->sl_contents)
            FPrintf(fh, "%-10ls \"%ls\"\n", script_commands[script_line->sl_command], script_line->sl_contents);
         else
            FPrintf(fh, "%ls\n", script_commands[script_line->sl_command]);
         script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
      }
   }
}

///

/// run_async
BOOL run_async(STRPTR file, STRPTR name)
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
            NP_StackSize   , 8192,
            (name ? NP_Name : TAG_IGNORE), name,
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
   {
      if((SysBase->AttnFlags & AFF_68020) && (GetFileSize("AmiTCP:kernel/AmiTCP.020") > 0))
         run_async("AmiTCP:kernel/AmiTCP.020", "AmiTCP");
      else
         run_async("AmiTCP:kernel/AmiTCP", "AmiTCP");
   }

   /* wait until AmiTCP is running */
   i = 0;
   while(!FindPort((STRPTR)AmiTCP_PortName) && i++ < 30)
      Delay(25);

   if(FindPort((STRPTR)AmiTCP_PortName))
   {
      return(TRUE);
   }

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

   vsprintf(buf, fmt, (STRPTR)(&fmt + 1));
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
SAVEDS ASM LONG strobjfunc(register __a2 Object *list, register __a1 Object *str)
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
SAVEDS ASM LONG txtobjfunc(register __a2 Object *list, register __a1 Object *txt)
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
SAVEDS ASM VOID objstrfunc(register __a2 Object *list,register __a1 Object *str)
{
   char *x;

   DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &x);
   if(x)
      set(str, MUIA_String_Contents, x);
}

///
/// objtxtfunc
SAVEDS ASM VOID objtxtfunc(register __a2 Object *list,register __a1 Object *txt)
{
   char *x;

   DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &x);
   if(x)
      set(txt, MUIA_Text_Contents, x);
}

///
/// desfunc
SAVEDS ASM VOID desfunc(register __a2 APTR pool, register __a1 APTR *entry)
{
   if(entry)
      FreeVec(entry);
}

///
/// sortfunc
SAVEDS ASM LONG sortfunc(register __a1 STRPTR str1, register __a2 STRPTR str2)
{
   return(stricmp(str1, str2));
}

///

