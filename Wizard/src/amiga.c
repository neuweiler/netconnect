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
/// StopSerialRead
VOID StopSerialRead(VOID)
{
   if(ReadSER && ReadQueued)
   {
      if(!(CheckIO(ReadSER)))
         AbortIO(ReadSER);

      WaitIO(ReadSER);
      ReadQueued = FALSE;
   }
}

///
/// StartSerialRead
VOID __regargs StartSerialRead(register APTR Data, register ULONG Length)
{
   if(ReadSER)
   {
      if(ReadQueued)
         StopSerialRead();

      ReadSER->IOSer.io_Command  = CMD_READ;
      ReadSER->IOSer.io_Length   = Length;
      ReadSER->IOSer.io_Data     = Data;

      SetSignal(0, 1L << ReadPortSER->mp_SigBit);

      SendIO(ReadSER);

      ReadQueued = TRUE;
   }
}

///
/// close_serial
VOID close_serial(VOID)
{
   if(ReadSER && ReadSER->IOSer.io_Device)
   {
      if(!(CheckIO(ReadSER)))
      {
         AbortIO(ReadSER);
         WaitIO(ReadSER);
      }
      CloseDevice(ReadSER);
      ReadSER->IOSer.io_Device = NULL;
   }
   if(WriteSER)         DeleteIORequest(WriteSER);
   if(ReadSER)          DeleteIORequest(ReadSER);
   if(WritePortSER)     DeleteMsgPort(WritePortSER);
   if(ReadPortSER)      DeleteMsgPort(ReadPortSER);

   WriteSER = ReadSER  = NULL;
   WritePortSER = ReadPortSER = NULL;
   ReadQueued = FALSE;
}

///
/// open_serial
BOOL open_serial(STRPTR device_name, LONG unit)
{
   WritePortSER   = CreateMsgPort();
   ReadPortSER    = CreateMsgPort();

   if(WritePortSER && ReadPortSER)
   {
      WriteSER = CreateIORequest(WritePortSER,  sizeof(struct IOExtSer));
      ReadSER  = CreateIORequest(ReadPortSER,   sizeof(struct IOExtSer));

      if(WriteSER && ReadSER)
      {
         ReadSER->io_SerFlags = SERF_7WIRE | SERF_RAD_BOOGIE | SERF_XDISABLED | SERF_SHARED;
         if(!(OpenDevice(device_name, unit, ReadSER, NULL)))
         {
            ReadSER->io_SerFlags = SERF_7WIRE | SERF_RAD_BOOGIE | SERF_XDISABLED | SERF_SHARED;
            ReadSER->io_Baud     = 38400;
            ReadSER->io_RBufLen  = 1024;
            ReadSER->io_WriteLen = 8;
            ReadSER->io_ReadLen  = 8;
            ReadSER->io_ExtFlags = NULL;
            ReadSER->IOSer.io_Command  = SDCMD_SETPARAMS;
            DoIO(ReadSER);

            memcpy(WriteSER, ReadSER, sizeof(struct IOExtSer));
            WriteSER->IOSer.io_Message.mn_ReplyPort = WritePortSER;
            StartSerialRead(serial_in, 1);
            return(TRUE);
         }
      }
   }
   close_serial();
   return(FALSE);
}

///
/// send_serial
VOID send_serial(STRPTR cmd, LONG len)
{
   if(WriteSER && *cmd)
   {
      if(len < 0)
         len = strlen(cmd);

      WriteSER->IOSer.io_Length  = len;
      WriteSER->IOSer.io_Command = CMD_WRITE;
      WriteSER->IOSer.io_Data    = cmd;
      DoIO(WriteSER);
   }
}

///
/// serial_carrier
BOOL serial_carrier(VOID)
{
   ULONG CD = 1<<5;

   if(!WriteSER)
      return(FALSE);
   WriteSER->IOSer.io_Command = SDCMD_QUERY;
   DoIO(WriteSER);
   return((BOOL)(CD & WriteSER->io_Status ? FALSE : TRUE));
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

      switch(chr)
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
/// save_config
BOOL save_config(STRPTR file)
{
   struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);
   BPTR fh;
   LONG pos;
   STRPTR ptr;

   if(fh = Open(file, MODE_NEWFILE))
   {
      FPrintf(fh, "# AmiTCP configuration file\n# Created by Setup AmiTCP, � 1997 Michael Neuweiler\n\n");

      FPrintf(fh, "LoginName        \"%ls\"\n", xgetstr(data->STR_UserName));
      FPrintf(fh, "Password         \"%ls\"\n", xgetstr(data->STR_Password));
      FPrintf(fh, "RealName         \"%ls\"\n", xgetstr(data->STR_FullName));
      FPrintf(fh, "Phone            \"%ls\"\n\n", xgetstr(data->STR_PhoneNumber));

      FPrintf(fh, "SerialDevice     \"%ls\"\n", Config.cnf_serialdevice);
      FPrintf(fh, "SerialUnit       %ld\n", Config.cnf_serialunit);
      FPrintf(fh, "BaudRate         38400\n");
      FPrintf(fh, "CarrierDetect    1\n");
      FPrintf(fh, "7Wire            1\n");
      FPrintf(fh, "OwnDevUnit       0\n\n");

      FPrintf(fh, "Modem            \"%ls\"\n", Config.cnf_modemname);
      FPrintf(fh, "InitString       \"%ls\"\n", Config.cnf_initstring);
      FPrintf(fh, "DialPrefix       \"%ls\"\n", Config.cnf_dialprefix);
      FPrintf(fh, "DialSuffix       \"%ls\"\n", Config.cnf_dialsuffix);

      FPrintf(fh, "Interface        %ls\n", (xget(data->CY_Protocol, MUIA_Cycle_Active) ? "slip" : "ppp"));
      FPrintf(fh, "IPDynamic        %ld\n", (xget(data->CY_IPAddress, MUIA_Cycle_Active) ? 0 : 1));
      FPrintf(fh, "IPAddr           %ls\n", (xget(data->CY_IPAddress, MUIA_Cycle_Active) ? xgetstr(data->STR_IPAddress) : "0.0.0.0"));
      FPrintf(fh, "UseBootP         %ld\n", ((addr_assign == CNF_Assign_BootP || dns_assign == CNF_Assign_BootP || dst_assign == CNF_Assign_BootP) ? 1 : 0));
      FPrintf(fh, "MTU              %ld\n", Config.cnf_MTU);
      if(*Config.cnf_domainname)
         FPrintf(fh, "DomainName       %ls\n", Config.cnf_domainname);
      if(Config.cnf_dns1 != INADDR_ANY)
         FPrintf(fh, "NameServer       %ls\n", dns1);
      if(Config.cnf_dns2 != INADDR_ANY)
         FPrintf(fh, "NameServer       %ls\n\n", dns2);

      FPrintf(fh, "# LoginScript\n");
      pos = 0;
      FOREVER
      {
         DoMethod(li_script, MUIM_List_GetEntry, pos++, &ptr);
         if(!ptr)
            break;
         FPrintf(fh, "%ls\n", ptr);
      }
      FPrintf(fh, "# EOS\n");

      Close(fh);

      return(TRUE);
   }
   return(FALSE);
}

///
/// print_config
VOID print_config(BPTR fh)
{
   struct MainWindow_Data *data = INST_DATA(CL_MainWindow->mcc_Class, win);
   STRPTR ptr;
   LONG pos;

   FPrintf(fh, GetStr(MSG_TX_InfoTitle));

   FPrintf(fh, "%ls %ls, unit %ld (38'400 baud)\n\n", GetStr(MSG_LA_Device), Config.cnf_serialdevice, Config.cnf_serialunit);

   FPrintf(fh, "%ls %ls\n", GetStr(MSG_LA_Modem), Config.cnf_modemname);
   FPrintf(fh, "%ls %ls\n", GetStr(MSG_LA_InitString), Config.cnf_initstring);
   FPrintf(fh, "%ls %ls\n", GetStr(MSG_LA_DialPrefix), Config.cnf_dialprefix);
   FPrintf(fh, "%ls %ls\n\n", GetStr(MSG_LA_DialSuffix), Config.cnf_dialsuffix);

   FPrintf(fh, "%ls %ls\n\n", GetStr(MSG_LA_Phone), xgetstr(data->STR_PhoneNumber));

   FPrintf(fh, "%ls %ls\n", GetStr(MSG_LA_FullName), xgetstr(data->STR_FullName));
   FPrintf(fh, "%ls %ls\n", GetStr(MSG_LA_UserName), xgetstr(data->STR_UserName));
   FPrintf(fh, "%ls - hidden -\n\n", GetStr(MSG_LA_Password));

   FPrintf(fh, "%ls %ls\n\n\n", GetStr(MSG_LA_Protocol), (xget(data->CY_Protocol, MUIA_Cycle_Active) ? "slip" : "ppp"));

   if(addr_assign = CNF_Assign_Static)
      FPrintf(fh, "%ls %ls\n", GetStr(MSG_LA_IPAddress), ip);
   else
      FPrintf(fh, "%ls %ls (%ls %ls)\n", GetStr(MSG_LA_IPAddress), ip, GetStr(MSG_TX_ObtainedBy), (addr_assign == CNF_Assign_BootP ? "BOOTP" : "ICPC"));
   FPrintf(fh, "%ls %ls (%ls %ls)\n", GetStr(MSG_LA_RemoteIPAddress), dest, GetStr(MSG_TX_ObtainedBy), (dst_assign == CNF_Assign_BootP ? "BOOTP" : "ICPC"));
   FPrintf(fh, "%ls %ls\n\n", GetStr(MSG_LA_Netmask), mask);

   FPrintf(fh, "%ls %ls (%ls %ls)\n\n", GetStr(MSG_LA_DomainName), Config.cnf_domainname, GetStr(MSG_TX_ObtainedBy), (domainname_assign == CNF_Assign_BootP ? "BOOTP" : "DNSQuery"));

   if(dns_assign == CNF_Assign_Root)
   {
      FPrintf(fh, "%ls %ls (Root DNS)\n", GetStr(MSG_LA_DNS1IPAddr), dns1);
      FPrintf(fh, "%ls %ls (Root DNS)\n", GetStr(MSG_LA_DNS2IPAddr), dns2);
   }
   else
   {
      FPrintf(fh, "%ls %ls (%ls %ls)\n", GetStr(MSG_LA_DNS1IPAddr), dns1, GetStr(MSG_TX_ObtainedBy), (dns_assign == CNF_Assign_BootP ? "BOOTP" : "MSDNS"));
      FPrintf(fh, "%ls %ls (%ls %ls)\n", GetStr(MSG_LA_DNS2IPAddr), dns2, GetStr(MSG_TX_ObtainedBy), (dns_assign == CNF_Assign_BootP ? "BOOTP" : "MSDNS"));
   }

   FPrintf(fh, "\n\n%ls\n\n", GetStr(MSG_LA_LoginScript));
   pos = 0;
   FOREVER
   {
      DoMethod(li_script, MUIM_List_GetEntry, pos++, &ptr);
      if(!ptr)
         break;
      FPrintf(fh, "%ls\n", ptr);
   }
}

///
