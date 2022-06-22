/// includes
#include "/includes.h"
#pragma header

#include "rev.h"
#include "Strings.h"
#include "/Genesis.h"
#include "mui.h"
#include "protos.h"

///
/// external variables
extern struct Library *LocaleBase;
extern struct Catalog *cat;
extern const char AmiTCP_PortName[];
extern struct MinList dialscript;
extern struct MinList user_startnet;
extern struct MinList user_stopnet;
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
/// des_func
VOID SAVEDS des_func(register __a2 APTR pool, register __a1 APTR ptr)
{
   if(ptr)
      FreeVec(ptr);
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
      MUIA_Text_Contents,  (string ? string : (STRPTR)""),
   End;

   return(obj);
}

///
/// desfunc
VOID SAVEDS desfunc(register __a2 APTR pool, register __a1 APTR *entry)
{
   if(entry)
      FreeVec(entry);
}

///
/// GetStr
STRPTR GetStr(STRPTR idstr)
{
   STRPTR local;

   local = idstr + 2;

   if(LocaleBase)
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
   strcpy(file, "Env:");
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
/// SetEnvDOS
BOOL SetEnvDOS(STRPTR name, STRPTR string, LONG len, BOOL save)
{
   char  file[MAXPATHLEN];
   BPTR  fh;
   BOOL  success = FALSE;

   strcpy(file, (save ? "EnvArc:": "Env:"));
   AddPart(file, name, MAXPATHLEN);
   if(fh = Open(file, MODE_NEWFILE))
   {
      if(len == -1)
         len = strlen(string);
      if(Write(fh, string, len) == len)
         success = TRUE;
      Close(fh);
   }
   return(success);
}

///
/// get_file_size
LONG get_file_size(STRPTR file)
{
   struct FileInfoBlock *fib;
   BPTR lock;
   LONG size = -1;

   if(lock = Lock(file, ACCESS_READ))
   {
      if(fib = AllocDosObject(DOS_FIB, NULL))
      {
         if(Examine(lock, fib))
            size = (fib->fib_DirEntryType > 0 ? -2 : fib->fib_Size);

         FreeDosObject(DOS_FIB, fib);
      }
      UnLock(lock);
   }
   return(size);
}

///
/// ParseConfig
BOOL ParseConfig(STRPTR file, struct pc_Data *pc_data)
{
   LONG size;
   STRPTR buf = NULL;
   BPTR fh;
   BOOL success = FALSE;

   if((size = get_file_size(file)) > -1)
   {
      if(buf = AllocVec(size, MEMF_ANY))
      {
         if(fh = Open(file, MODE_OLDFILE))
         {
            if(Read(fh, buf, size) == size)
            {
               success = TRUE;

               pc_data->Buffer   = buf;
               pc_data->Size     = size;
               pc_data->Current  = buf;

               pc_data->Argument = NULL;
               pc_data->Contents = NULL;
            }

            Close(fh);
         }
      }
   }

   return(success);
}

///
/// ParseNext
BOOL ParseNext(struct pc_Data *pc_data)
{
   BOOL success = FALSE;
   STRPTR ptr_eol, ptr_tmp;

   if(pc_data->Current && pc_data->Current < pc_data->Buffer + pc_data->Size)
   {
      if(ptr_eol = strchr(pc_data->Current, '\n'))
      {
         *ptr_eol = NULL;

         if(pc_data->Contents = strchr(pc_data->Current, 34))              /* is the content between ""'s ? */
         {
            pc_data->Contents++;
            if(ptr_tmp = strchr(pc_data->Contents, 34))  /* find the ending '"' */
               *ptr_tmp = NULL;

            ptr_tmp = pc_data->Contents - 2;
            while(((*ptr_tmp == ' ') || (*ptr_tmp == 9)) && ptr_tmp >= pc_data->Current)
               ptr_tmp--;

            ptr_tmp++;
            *ptr_tmp = NULL;
         }
         else
         {
            pc_data->Contents = strchr(pc_data->Current, ' ');                   /* a space  */
            ptr_tmp           = strchr(pc_data->Current, 9);                     /* or a TAB */

            if((ptr_tmp < pc_data->Contents && ptr_tmp) || !pc_data->Contents)   /* which one comes first ? */
               pc_data->Contents = ptr_tmp;
            if(pc_data->Contents)
            {
               *pc_data->Contents++ = NULL;
               while((*pc_data->Contents == ' ') || (*pc_data->Contents == 9))
                  pc_data->Contents++;

               if(ptr_tmp = strchr(pc_data->Contents, ';')) /* cut out the comment */
                  *ptr_tmp = NULL;
            }
            else
               pc_data->Contents = "";
         }

         pc_data->Argument = pc_data->Current;
         pc_data->Current  = ptr_eol + 1;
         success = TRUE;
      }
      else
         pc_data->Current = NULL;
   }
   return(success);
}

///
/// ParseNextLine
BOOL ParseNextLine(struct pc_Data *pc_data)
{
   BOOL success = FALSE;
   STRPTR ptr_eol;

   if(pc_data->Current && pc_data->Current < pc_data->Buffer + pc_data->Size)
   {
      if(ptr_eol = strchr(pc_data->Current, '\n'))
      {
         *ptr_eol = NULL;

         pc_data->Argument = "";
         pc_data->Contents = pc_data->Current;
         pc_data->Current  = ptr_eol + 1;
         success = TRUE;
      }
      else
         pc_data->Current = NULL;
   }

   return(success);
}

///
/// ParseEnd
VOID ParseEnd(struct pc_Data *pc_data)
{
   if(pc_data->Buffer)
      FreeVec(pc_data->Buffer);

   pc_data->Buffer   = NULL;
   pc_data->Size     = NULL;
   pc_data->Current  = NULL;

   pc_data->Argument = NULL;
   pc_data->Contents = NULL;
}

///
/// launch_amitcp
BOOL launch_amitcp(VOID)
{
   int i;
Printf("PORT:'%ls'\n", AmiTCP_PortName);

   if(!FindPort((STRPTR)AmiTCP_PortName))
      SystemTags("run <nil: >nil: AmiTCP:bin/AmiTCP.kernel", TAG_DONE);
      
   /* wait until AmiTCP is running */
   i = 0;
   while(!FindPort((STRPTR)AmiTCP_PortName) && i++ < 2)
      Delay(25);

   if(FindPort((STRPTR)AmiTCP_PortName))
      return(TRUE);

   return(FALSE);
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
Printf("buf:%ls\n", buf);

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
         if(*help == 'b' || *help == 'B')
            buffer[offs++] = '\b';
         else if(*help == 't' || *help == 'T')
            buffer[offs++] = '\t';
         else if(*help == 'n' || *help == 'N')
            buffer[offs++] = '\n';
         else if(*help == 'v' || *help == 'V')
            buffer[offs++] = '\v';
         else if(*help == 'f' || *help == 'F')
            buffer[offs++] = '\f';
         else if(*help == 'r' || *help == 'R')
            buffer[offs++] = '\r';
         else if(*help == 'e' || *help == 'E')
            buffer[offs++] = 27;
         else if(*help == 'x' || *help == 'X')
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
         else if(*help == '\\')
            buffer[offs++] = '\\';

         help++;
      }
      len--;
   }
   buffer[offs] = '\0';
}

///
/// free_list
VOID free_list(struct MinList *list)
{
   struct MinNode *n1, *n2;

   n1 = list->mlh_Head;
   while(n2 = n1->mln_Succ)
   {
      Remove((struct Node *)n1);
      FreeVec(n1);
      n1 = n2;
   }
}

///
/// clear_config
VOID clear_config(struct config *conf)
{
   free_list(&dialscript);
   free_list(&user_startnet);
   free_list(&user_stopnet);

   if(conf->cnf_sana2configtext)
      FreeVec(conf->cnf_sana2configtext);
   if(conf->cnf_ifconfigparams)
      FreeVec(conf->cnf_ifconfigparams);
   if(conf->cnf_online)
      FreeVec(conf->cnf_online);
   if(conf->cnf_onlinefail)
      FreeVec(conf->cnf_onlinefail);
   if(conf->cnf_offlineactive)
      FreeVec(conf->cnf_offlineactive);
   if(conf->cnf_offlinepassive)
      FreeVec(conf->cnf_offlinepassive);
   if(conf->cnf_startup)
      FreeVec(conf->cnf_startup);
   if(conf->cnf_shutdown)
      FreeVec(conf->cnf_shutdown);

   bzero(conf, sizeof(struct config));

   conf->cnf_netmask         = 0xffffff00;
   conf->cnf_MTU             = 1500;
   conf->cnf_baudrate        = 38400;
   conf->cnf_carrierdetect   = TRUE;
   conf->cnf_7wire           = TRUE;

   conf->cnf_autologin       = 1;
   conf->cnf_winstartup      = 1;
   conf->cnf_showstatus      = 1;
   conf->cnf_showspeed       = 1;
   conf->cnf_showonlinetime  = 1;
   conf->cnf_showbuttons     = 1;
}

///
/// exec_script
VOID exec_script(struct MinList *list)
{
   struct ScriptLine *script_line;

   if(list->mlh_TailPred == (struct MinNode *)list)
      return;

   script_line = (struct ScriptLine *)list->mlh_Head;
   while(script_line->sl_node.mln_Succ)
   {
      SystemTags(script_line->sl_contents,
         SYS_UserShell  , TRUE,
         TAG_DONE);

      script_line = (struct ScriptLine *)script_line->sl_node.mln_Succ;
   }
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

