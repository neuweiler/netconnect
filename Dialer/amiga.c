#include "globals.c"

/// xget
LONG xget(Object *obj,ULONG attribute)
{
   LONG x;
   get(obj,attribute,&x);
   return(x);
}

///
/// xgetstr
char *xgetstr(Object *obj)
{
   return((char *)xget(obj,MUIA_String_Contents));
}

///
/// DoSuperNew
ULONG __stdargs DoSuperNew(struct IClass *cl,Object *obj,ULONG tag1,...)
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
/// MakeKeyString
Object *MakeKeyString(STRPTR string, LONG len, STRPTR c)
{
   return(TextinputObject,
      StringFrame,
      MUIA_CycleChain         , 1,
      MUIA_Textinput_Multiline, FALSE,
      MUIA_Textinput_Contents , string,
      MUIA_Textinput_MaxLen   , len,
      MUIA_ControlChar        , *GetStr(c),
      End);
}

///
/// MakeKeyLabel
Object *MakeKeyLabel(STRPTR label, STRPTR control_char)
{
   return(KeyLabel(GetStr(label), *GetStr(control_char)));
}

///
/// MakeKeyCycle
Object *MakeKeyCycle(STRPTR *array, STRPTR control_char)
{
   Object *obj = KeyCycle(array, *GetStr(control_char));

   if(obj)
      set(obj,MUIA_CycleChain,1);
   return(obj);
}

///
/// MakeCheckMark
Object *MakeCheckMark(BOOL selected)
{
   Object *obj = CheckMark(selected);

   if(obj)
      set(obj,MUIA_CycleChain,1);
   return(obj);
}

///
/// MakePopAsl
Object *MakePopAsl(Object *string, STRPTR title, BOOL drawers_only)
{
   Object *obj = PopaslObject,
      MUIA_Popstring_String, string,
      MUIA_Popstring_Button, PopButton((drawers_only ? MUII_PopDrawer : MUII_PopFile)),
      MUIA_Popasl_Type     , ASL_FileRequest,
      ASLFR_TitleText      , GetStr(title),
      ASLFR_DrawersOnly    , drawers_only,
   End;

   if(obj)
      set(obj,MUIA_CycleChain,1);
   return(obj);
}

///
/// desfunc
SAVEDS ASM VOID desfunc(REG(a2) APTR pool, REG(a1) APTR *entry)
{
   if(entry)
      FreeVec(entry);
}

///
/// sortfunc
SAVEDS ASM LONG sortfunc(REG(a1) STRPTR str1, REG(a2) STRPTR str2)
{
   return(stricmp(str1, str2));
}

///
/// strobjfunc
SAVEDS ASM LONG strobjfunc(REG(a2) Object *list, REG(a1) Object *str)
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
SAVEDS ASM LONG txtobjfunc(REG(a2) Object *list, REG(a1) Object *txt)
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
/// extract_arg
STRPTR extract_arg(STRPTR string, STRPTR buffer, LONG len, char sep)
{
   STRPTR ptr1, ptr2;

   strncpy(buffer, string, len);

   ptr1 = strchr(buffer, (sep ? sep : ' '));
   ptr2 = strchr(buffer, 9);

   if(ptr2 && ((ptr2 < ptr1) || !ptr1))
      ptr1 = ptr2;
   if(ptr1)
      *ptr1 = NULL;

   string += strlen(buffer);

   while(*string == ' ' || *string == 9 || (sep ? *string == sep : NULL))
      string++;

   return((*string ? string : NULL));
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
/// launch_async
BOOL launch_async(STRPTR file)
{
   BPTR ofh, ifh;

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
            return(TRUE);

         Close(ifh);
      }
      Close(ofh);
   }
   return(FALSE);
}

///
/// launch_amitcp
BOOL launch_amitcp(VOID)
{
   int i;

   if(!FindPort((STRPTR)AmiTCP_PortName))
      launch_async("AmiTCP:AmiTCP");

   /* wait until amitcp is running */
   i = 0;
   while(!FindPort((STRPTR)AmiTCP_PortName) && i++ < 20)
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
               } while(GetMsg(port) != rmsg);
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
         switch(*help++)
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

   if(list->mlh_TailPred == (struct Node *)list)
      return;

   script_line = (struct ScriptLine *)list->mlh_Head;
   while(script_line->sl_node.mln_Succ)
   {
      SystemTags(script_line->sl_contents,
         SYS_Asynch     , FALSE,
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

         PutMsg(MainPort, message);

         while(!finished)
         {
            sig = Wait(1L << reply_port->mp_SigBit);  // must not be interuptable by ctrl_c because HandleMainMethod() needs the reply_port and the message to be still alive when it does a ReplyMsg()

            if(sig & (1L << reply_port->mp_SigBit))
            {
               while(reply_message = (struct MainMessage *)GetMsg(reply_port))
               {
                  if(reply_message->MethodID == MUIM_AmiTCP_Handshake)
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

