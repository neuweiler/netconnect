/// includes & defines
#include <intuition/intuitionbase.h>
#include <devices/netinfo.h>
#include "libraries/genesis.h"
#include "/Genesis.h"
#include "rev.h"

#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define ID_OKAY   42
#define ID_CANCEL 43
#define ID_OBJSTR 44
#define ID_STROBJ 45
///
/// variables
const char version_string[] = "$VER:genesis.library " VERTAG;

struct   DosLibrary      *DOSBase       = NULL;
struct   ExecBase        *SysBase       = NULL;
struct   IntuitionBase   *IntuitionBase = NULL;
struct   Library         *UserGroupBase = NULL;
struct   Library         *MUIMasterBase = NULL;
struct   Library         *RexxSysBase   = NULL;

struct   User            *global_user   = NULL;
struct   SignalSemaphore LibSemaphore;
struct   MinList         UserList;

struct UserNode
{
   struct MinNode un_node;
   struct User *un_user;
};

///

// internal stuff.. we don't need to export everything

/// clear_list
VOID clear_list(struct MinList *list)
{
   if(list->mlh_TailPred != (struct MinNode *)list)
   {
      struct MinNode *e1, *e2;

      e1 = list->mlh_Head;
      while(e2 = e1->mln_Succ)
      {
         Remove((struct Node *)e1);
         FreeVec(e1);
         e1 = e2;
      }
   }
}
///
/// my_rand
static LONG my_rand( ULONG *seed )
{
   LONG    ret, a;

   a     = (LONG)*seed;    // MOVE.L  0040(A4),D0
   ret   = a;              // MOVE.L  D0,D1
   ret <<= 3;              // ASL.L   #3,D1
   ret  -= a;              // SUB.L   D0,D1
   ret <<= 3;              // ASL.L   #3,D1
   ret  += a;              // ADD.L   D0,D1
   ret  += ret;            // ADD.L   D1,D1
   ret  += a;              // ADD.L   D0,D1
   ret <<= 4;              // ASL.L   #4,D1
   ret  -= a;              // SUB.L   D0,D1
   ret  += ret;            // ADD.L   D1,D1
   ret  -= a;              // SUB.L   D0,D1
   ret  += 0x00000E60;     // ADDI.L  #00000e60,D1
   ret  &= 0x7FFFFFFF;     // ANDI.L  #7fffffff,D1
   a     = ret;            // MOVE.L  D1,D0
   a    -= 1;              // SUBQ.L  #1,D0
   *seed = (ULONG)a;       // MOVE.L  D0,0040(A4)

   return(ret);            // MOVE.L  D1,D0
}

///
/// my_crypt
VOID my_crypt(UBYTE *From, UBYTE *To, ULONG len)
{
   ULONG i, seed;

   seed = len;
   for(i = 0; i < len; i++)
      *To++ = *From++ ^ my_rand(&seed);
}

///
/// AllocFGetString
STRPTR AllocFGetString(BPTR fh, STRPTR *str)
{
   UWORD   len;

   if(*str)
      FreeVec(*str);
   *str = NULL;

   if(FRead(fh, &len, sizeof(UWORD), 1))
   {
      if(*str = AllocVec(len + 1, MEMF_ANY))
      {
         if(FRead(fh, *str, len, 1))
         {
            (*str)[len] = '\0';
            my_crypt(*str, *str, len);
         }
         else
         {
            FreeVec(*str);
            *str = NULL;
         }
      }
   }
   return(*str);
}

///
/// FPutString
static void FPutString(BPTR file, STRPTR str)
{
   UWORD len;
   STRPTR buffer;

   len = strlen(str);

   if(buffer = AllocVec(len + 10, MEMF_ANY | MEMF_CLEAR))
   {
      FWrite(file, &len, sizeof(UWORD), 1);
      my_crypt(str, buffer, len);
      FWrite(file, buffer, len, 1);
      FreeVec(buffer);
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
/// rexx_cmd
LONG rexx_cmd(STRPTR port_name, const char *fmt, ...)
{
   STRPTR buf;
   struct MsgPort *port, *dest_Port;
   struct RexxMsg *rmsg;
   LONG rc = -1;

   if(buf = AllocVec(256, MEMF_ANY | MEMF_CLEAR))
   {
      vsprintf(buf, fmt, (STRPTR)(&fmt + 1));

      if(RexxSysBase = OpenLibrary("rexxsyslib.library", 0))
      {
         if(port = CreateMsgPort())
         {
            port->mp_Node.ln_Name = "GENESIS.LIB";
            if(rmsg = CreateRexxMsg(port, NULL, port_name))
            {
               rmsg->rm_Action = RXCOMM;
               rmsg->rm_Args[0] = (STRPTR)buf;
               if(FillRexxMsg(rmsg, 1, 0))
               {
                  Forbid();
                  if(dest_Port = FindPort(port_name))
                  {
                     PutMsg(dest_Port, (struct Message *)rmsg);
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
         CloseLibrary(RexxSysBase);
      }
      FreeVec(buf);
   }
   return(rc);
}

///

// public functions that don't need blocking

/// GetFileSize
SAVEDS ASM LONG GetFileSize(register __a0 STRPTR file)
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
SAVEDS ASM BOOL ParseConfig(register __a0 STRPTR file, register __a1 struct ParseConfig_Data *pc_data)
{
   LONG size;
   STRPTR buf = NULL;
   BPTR fh;
   BOOL success = FALSE;

   if((size = GetFileSize(file)) > -1)
   {
      if(buf = AllocVec(size + 1, MEMF_ANY))
      {
         if(fh = Open(file, MODE_OLDFILE))
         {
            if(Read(fh, buf, size) == size)
            {
               success = TRUE;

               pc_data->pc_buffer   = buf;
               pc_data->pc_size     = size;
               pc_data->pc_current  = buf;

               pc_data->pc_argument = NULL;
               pc_data->pc_contents = NULL;
            }
            else
               FreeVec(buf);

            Close(fh);
         }
         else
            FreeVec(buf);
      }
   }

   return(success);
}

///
/// ParseNext
SAVEDS ASM BOOL ParseNext(register __a0 struct ParseConfig_Data *pc_data)
{
   BOOL success = FALSE;
   STRPTR ptr_eol, ptr_tmp;

   if(pc_data->pc_current && pc_data->pc_current < pc_data->pc_buffer + pc_data->pc_size)
   {
      if(ptr_eol = strchr(pc_data->pc_current, '\n'))
      {
         *ptr_eol = NULL;

         if(pc_data->pc_contents = strchr(pc_data->pc_current, 34))              // is the content between ""'s ?
         {
            pc_data->pc_contents++;
            if(ptr_tmp = strrchr(pc_data->pc_contents, 34))  // find the ending '"'
               *ptr_tmp = NULL;

            ptr_tmp = pc_data->pc_contents - 2;
            while(((*ptr_tmp == ' ') || (*ptr_tmp == 9)) && ptr_tmp >= pc_data->pc_current)
               ptr_tmp--;

            ptr_tmp++;
            *ptr_tmp = NULL;
         }
         else
         {
            pc_data->pc_contents = strchr(pc_data->pc_current, ' ');                   // a space
            ptr_tmp           = strchr(pc_data->pc_current, 9);                     // or a TAB

            if((ptr_tmp < pc_data->pc_contents && ptr_tmp) || !pc_data->pc_contents)   // which one comes first ?
               pc_data->pc_contents = ptr_tmp;
            if(pc_data->pc_contents)
            {
               *pc_data->pc_contents++ = NULL;
               while((*pc_data->pc_contents == ' ') || (*pc_data->pc_contents == 9))
                  pc_data->pc_contents++;

               if(ptr_tmp = strchr(pc_data->pc_contents, ';')) // cut out the comment
                  *ptr_tmp = NULL;
            }
            else
               pc_data->pc_contents = "";
         }

         pc_data->pc_argument = pc_data->pc_current;
         pc_data->pc_current  = ptr_eol + 1;
         success = TRUE;
      }
      else
         pc_data->pc_current = NULL;
   }
   return(success);
}

///
/// ParseNextLine
SAVEDS ASM BOOL ParseNextLine(register __a0 struct ParseConfig_Data *pc_data)
{
   BOOL success = FALSE;
   STRPTR ptr_eol;

   if(pc_data->pc_current && pc_data->pc_current < pc_data->pc_buffer + pc_data->pc_size)
   {
      if(ptr_eol = strchr(pc_data->pc_current, '\n'))
      {
         *ptr_eol = NULL;

         pc_data->pc_argument = "";
         pc_data->pc_contents = pc_data->pc_current;
         pc_data->pc_current  = ptr_eol + 1;
         success = TRUE;
      }
      else
         pc_data->pc_current = NULL;
   }
   return(success);
}

///
/// ParseEnd
SAVEDS ASM VOID ParseEnd(register __a0 struct ParseConfig_Data *pc_data)
{
   if(pc_data->pc_buffer)
      FreeVec(pc_data->pc_buffer);

   pc_data->pc_buffer   = NULL;
   pc_data->pc_size     = NULL;
   pc_data->pc_current  = NULL;

   pc_data->pc_argument = NULL;
   pc_data->pc_contents = NULL;
}

///
/// ReallocCopy
SAVEDS ASM STRPTR ReallocCopy(register __a0 STRPTR *old, register __a1 STRPTR src)
{
   if(*old)
      FreeVec(*old);
   *old = NULL;

   if(src && *src)
   {
      if(*old = AllocVec(strlen(src) + 1, MEMF_ANY))
         strcpy(*old, src);
   }

   return(*old);
}

///
/// FreeUser
SAVEDS ASM VOID FreeUser(register __a0 struct User *user)
{
   if(user)
   {
      if(user->us_name)
         FreeVec(user->us_name);
      if(user->us_passwd)
         FreeVec(user->us_passwd);
      if(user->us_gecos)
         FreeVec(user->us_gecos);
      if(user->us_dir)
         FreeVec(user->us_dir);
      if(user->us_shell)
         FreeVec(user->us_shell);

      clear_list(&user->us_restricted_times);

      FreeVec(user);
   }
}

///
/// ReadFile
SAVEDS ASM LONG ReadFile(register __a0 STRPTR file, register __a1 STRPTR buffer, register __d0 LONG max_len)
{
   LONG  size = NULL;
   BPTR  fh;

   if(file && buffer)
   {
      *buffer = NULL;
      if(fh = Open(file, MODE_OLDFILE))
      {
         size = Read(fh, buffer, max_len - 1);
         Close(fh);
         if(size >= 0)
            buffer[size] = NULL;
      }
      else
         size = -1;
   }
   return(size);
}

///
/// WriteFile
SAVEDS ASM BOOL WriteFile(register __a0 STRPTR file, register __a1 STRPTR buffer, register __d0 LONG len)
{
   BPTR  fh;
   BOOL  success = FALSE;

   if(file && buffer)
   {
      if(fh = Open(file, MODE_NEWFILE))
      {
         if(len == -1)
            len = strlen(buffer);
         if(Write(fh, buffer, len) == len)
            success = TRUE;
         Close(fh);
      }
   }
   return(success);
}

///
/// IsOnline
SAVEDS ASM BOOL IsOnline(register __d0 LONG cmd)
{
   STRPTR genesis_name = "GENESIS", miami_name = "MIAMI.1";
   BOOL is_online = FALSE, is_miami = FALSE;
   LONG rc;

   if(FindPort(genesis_name))
   {
      rc = rexx_cmd(genesis_name, "isonline any");
      is_online = (rc > 0 ? TRUE : NULL);
   }
   else if(FindPort(miami_name)) // what a shame !!
   {
      is_miami = TRUE;
      rc = rexx_cmd(miami_name, "ISONLINE");
      is_online = (rc > 0 ? TRUE : NULL);
   }
   else  // if using another stack => set is_online = TRUE;
   {
      struct Library *tmp_lib;

      if(tmp_lib = OpenLibrary("bsdsocket.library", NULL))
      {
         is_online = TRUE;
         CloseLibrary(tmp_lib);
      }
   }

   switch(cmd)
   {
      case IOC_AskUser:
         if(!is_online)
         {
            if(MUIMasterBase = OpenLibrary("muimaster.library", 11))
            {
               if(!MUI_Request(NULL, NULL, NULL, "Network Request", "Go Online|Stay Offline", "An application wishes to exchange data over the\nnetwork but currently no connection is established.\nWould you like to establish a connection now ?"))
                  return(FALSE);
               CloseLibrary(MUIMasterBase);
               MUIMasterBase = NULL;
            }
         }
      case IOC_Force:
         if(!is_online)
         {
            int i;

            if(!is_miami)
            {
               if(!FindPort(genesis_name)) // if genesis is not running, launch it
               {
                  if(run_async("AmiTCP:GENESiS"))
                  {
                     i = 0;
                     while(i++ < 20)
                     {
                        if(FindPort(genesis_name))
                           break;
                        Delay(25);
                     }
                  }
               }
            }
            if(FindPort((is_miami ? miami_name : genesis_name)))
            {
               rexx_cmd((is_miami ? miami_name : genesis_name), (is_miami ? "online" : "connect"));

               i = 0;
               while(!is_online && i++ < 80)  // wait 80 sec until iface comes online
               {
                  Delay(50);
                  rc = rexx_cmd((is_miami ? miami_name : genesis_name), (is_miami ? "ISONLINE" : "isonline any"));
                  is_online = (rc > 0 ? TRUE : NULL);
                  if(rc < 0)  // genesis is no longer running => shorten wait time
                     break;
               }
            }
         }
         break;
   }
   return(is_online);
}

///

// internal functions, for your eyes only

/// find_user
struct User *find_user(char *name)
{
   struct UserNode *user_node;

   if(UserList.mlh_TailPred != (struct MinNode *)&UserList)
   {
      user_node = (struct UserNode *)UserList.mlh_Head;
      while(user_node->un_node.mln_Succ)
      {
         if(!strcmp(name, user_node->un_user->us_name))
            return(user_node->un_user);
         user_node = (struct UserNode *)user_node->un_node.mln_Succ;
      }
   }
   return(NULL);
}

///
/// find_user_verify_password
struct User *find_user_verify_password(STRPTR name, STRPTR password)
{
   struct User *user = NULL;

   if(name)
   {
      if(user = find_user(name))
      {
         if(!user->us_passwd || !(*user->us_passwd))
         {
            if(password && *password)
               user = NULL;
         }
         else
         {
            if(password)
            {
               STRPTR salt;

               salt = user->us_passwd;

               if(!UserGroupBase)
                  UserGroupBase = OpenLibrary(USERGROUPNAME, 0);

               if(UserGroupBase)
               {
                  if(strcmp(crypt(password, salt), user->us_passwd))
                     user = NULL;

                  CloseLibrary(UserGroupBase);
                  UserGroupBase = NULL;
               }
               else
                  user = NULL;
            }
            else
               user = NULL;
         }
      }
   }

   return(user);
}

///
/// create_usercopy
struct User *create_usercopy(struct User *src)
{
   struct User *new = NULL;

   if(src)
   {
      if(new = AllocVec(sizeof(struct User), MEMF_ANY | MEMF_CLEAR))
      {
         NewList((struct List *)&new->us_restricted_times);
         ReallocCopy(&new->us_name  , src->us_name);
         ReallocCopy(&new->us_passwd, src->us_passwd);
         new->us_uid = src->us_uid;
         new->us_gid = src->us_gid;
         ReallocCopy(&new->us_gecos , src->us_gecos);
         ReallocCopy(&new->us_dir   , src->us_dir);
         ReallocCopy(&new->us_shell , src->us_shell);

         new->us_flags     = src->us_flags;
         new->us_max_time  = src->us_max_time;
         ReallocCopy(&new->us_timeserver, src->us_timeserver);

         if(src->us_restricted_times.mlh_TailPred != (struct MinNode *)&src->us_restricted_times)
         {
            struct RestrictedTime *restrict_src, *restrict_new;

            restrict_src = (struct RestrictedTime *)src->us_restricted_times.mlh_Head;
            while(restrict_src->rt_node.mln_Succ)
            {
               if(restrict_new = AllocVec(sizeof(struct RestrictedTime), MEMF_ANY | MEMF_CLEAR))
               {
                  memcpy(restrict_new, restrict_src, sizeof(struct RestrictedTime));
                  AddTail((struct List *)&new->us_restricted_times, (struct Node *)restrict_new);
               }
               restrict_src = (struct RestrictedTime *)restrict_src->rt_node.mln_Succ;
            }
         }
      }
   }

   return(new);
}

///
/// write_passwd
VOID write_passwd(BPTR fh, struct NetInfoPasswd *passwd)
{
   ULONG zero = NULL;
   char empty[2];

   *empty = NULL;

   FPutString(fh, passwd->pw_name);                // login
   FPutString(fh, passwd->pw_passwd);              // pwd
   FWrite(fh, &passwd->pw_uid, sizeof(LONG), 1);   // uid
   FWrite(fh, &passwd->pw_gid, sizeof(LONG), 1);   // gid
   FPutString(fh, passwd->pw_gecos);               // real name
   FPutString(fh, passwd->pw_dir);                 // home dir
   FPutString(fh, passwd->pw_shell);               // shell
   FWrite(fh, &zero, sizeof(ULONG), 1);            // # time restrictions
   FWrite(fh, &zero, sizeof(ULONG), 1);            // user flags
   FWrite(fh, &zero, sizeof(ULONG), 1);            // max time online
   FPutString(fh, empty);                          // timeserver
}

///
/// copy_passwd_file
#define PASSWD_SIZE 1024
VOID copy_passwd_file(VOID)
{
   BOOL found_admin = FALSE, found_root = FALSE;
   struct MsgPort *port;

   if(port = CreateMsgPort())
   {
      struct NetInfoReq  *req;

      if(req = (struct NetInfoReq *)CreateIORequest(port, sizeof(struct NetInfoReq)))
      {
         if(!(OpenDevice(NETINFONAME, NETINFO_PASSWD_UNIT, (struct IORequest *)req, 0)))
         {
            req->io_Command = CMD_RESET;

            if(!(DoIO((struct IORequest *)req)))
            {
               struct NetInfoPasswd   *passwd;
               ULONG zero = 0;
               BPTR fh;

               if(passwd = (struct NetInfoPasswd *)AllocVec(PASSWD_SIZE, MEMF_ANY | MEMF_CLEAR))
               {
                  req->io_Data    = passwd;
                  req->io_Length  = PASSWD_SIZE;
                  req->io_Command = CMD_READ;

                  if(fh = Open("AmiTCP:db/utm.conf", MODE_NEWFILE))
                  {
                     FWrite(fh, &zero, sizeof(ULONG), 1);                 // utm_version

                     while(!(DoIO((struct IORequest *)req)))
                     {
                        if((passwd->pw_uid == 0) && (passwd->pw_gid == 0))
                           found_admin = TRUE;
                        if(!strcmp(passwd->pw_name, "root"))
                           found_root = TRUE;


                        write_passwd(fh, passwd);

                        req->io_Data    = passwd;
                        req->io_Length  = PASSWD_SIZE;
                        req->io_Command = CMD_READ;
                     }
                     if(!found_admin)   // add root user if not existent
                     {
                        passwd->pw_name = (found_root ? "new_root" : "root");
                        passwd->pw_passwd = "";
                        passwd->pw_uid = 0;
                        passwd->pw_gid = 0;
                        passwd->pw_gecos = "System Administrator";
                        passwd->pw_dir = "SYS:";
                        passwd->pw_shell = "noshell";

                        write_passwd(fh, passwd);

                        req->io_Data    = passwd;
                        req->io_Length  = PASSWD_SIZE;
                        req->io_Command = CMD_WRITE;
                        DoIO((struct IORequest *)req);
                        req->io_Command = CMD_UPDATE;
                        DoIO((struct IORequest *)req);
                     }
                     Close(fh);
                  }
                  FreeVec(passwd);
               }
            }
            CloseDevice((struct IORequest * )req);
         }
         DeleteIORequest(req);
      }
      DeleteMsgPort(port);
   }
}

///

// internal functions, don't use semaphore here !

/// get_username
STRPTR get_username(LONG pos)
{
   if(UserList.mlh_TailPred != (struct MinNode *)&UserList)
   {
      struct UserNode *user_node;
      LONG i;

      i = 0;
      user_node = (struct UserNode *)UserList.mlh_Head;
      while(user_node->un_node.mln_Succ)
      {
         if(i == pos)
            return(user_node->un_user->us_name);
         user_node = (struct UserNode *)user_node->un_node.mln_Succ;
         i++;
      }
   }

   return(NULL);
}

///
/// get_user
struct User *get_user(char *name, char *password, char *title, LONG flags)
{
   Object *app, *win, *PO_Username = NULL, *STR_Username = NULL, *LV_Usernames = NULL,
          *STR_Password, *BT_Okay, *BT_Cancel;
   ULONG sigs = NULL, id;
   STRPTR ptr = NULL;
   struct User *user = NULL;
   int tries;

   if(user = find_user_verify_password(name, password))
      return(create_usercopy(user));

   if(MUIMasterBase = OpenLibrary("muimaster.library", 11))
   {
      if(app = ApplicationObject,
         MUIA_Application_Title      , "Genesis User Request",
         MUIA_Application_Copyright  , "©1998, Michael Neuweiler",
         MUIA_Application_Author     , "Michael Neuweiler",
         MUIA_Application_Description, "Requester to login a user",
         MUIA_Application_Base       , "GENESISUSERREQ",
         SubWindow, win = WindowObject,
            MUIA_Window_ID       , MAKE_ID('U','R','E','Q'),
            MUIA_Window_Screen   , IntuitionBase->FirstScreen,
            MUIA_Window_Title    , "login",
            WindowContents       , VGroup,
               Child, VGroup,
                  (title ? Child : TAG_IGNORE), (title ? (VGroup,
                        GroupFrame,
                        MUIA_Background, MUII_GroupBack,
                        Child, TextObject,
                           MUIA_Text_Contents   , title,
                        End,
                     End)
                     : NULL),
                  Child, ColGroup(2),
                     GroupFrame,
                     MUIA_Background, MUII_GroupBack,
                     Child, KeyLabel("User:", 'u'),
                     Child, ((name && (flags & GUF_TextObject)) ?
                        (TextObject,
                           MUIA_Frame           , MUIV_Frame_Text,
                           MUIA_Text_Contents   , name,
                        End)
                        :
                        (PO_Username = PopobjectObject,
                           MUIA_Popstring_String, STR_Username = StringObject,
                              MUIA_ControlChar     , 'u',
                              MUIA_CycleChain      , 1,
                              MUIA_Frame           , MUIV_Frame_String,
                              MUIA_String_Contents , name,
                              MUIA_String_MaxLen   , 40,
                           End,
                           MUIA_Popstring_Button      , PopButton(MUII_PopUp),
                           MUIA_Popobject_Object      , LV_Usernames = ListviewObject,
                              MUIA_Listview_DoubleClick  , TRUE,
                              MUIA_Listview_List         , ListObject,
                                 MUIA_Frame              , MUIV_Frame_InputList,
                                 MUIA_List_ConstructHook , MUIV_List_ConstructHook_String,
                                 MUIA_List_DestructHook  , MUIV_List_DestructHook_String,
                              End,
                           End,
                        End)
                     ),
                     Child, KeyLabel("Password:", 'p'),
                     Child, STR_Password = StringObject,
                        MUIA_ControlChar     , 'p',
                        MUIA_CycleChain      , 1,
                        MUIA_Frame           , MUIV_Frame_String,
                        MUIA_String_Secret   , TRUE,
                        MUIA_String_MaxLen   , 40,
                     End,
                  End,
               End,
               Child, HGroup,
                  Child, BT_Okay   = MUI_MakeObject(MUIO_Button, "_Okay"),
                  Child, BT_Cancel = MUI_MakeObject(MUIO_Button, "_Cancel"),
               End,
            End,
         End,
      End)
      {
         if(STR_Username)
            DoMethod(STR_Username, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, STR_Password);
         if(PO_Username && LV_Usernames)
            DoMethod(LV_Usernames , MUIM_Notify, MUIA_Listview_DoubleClick  , MUIV_EveryTime, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, app, 2, MUIM_Application_ReturnID, ID_OBJSTR);
         DoMethod(STR_Password, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, BT_Okay);
         DoMethod(BT_Okay     , MUIM_Notify, MUIA_Pressed           , FALSE, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, app, 2, MUIM_Application_ReturnID, ID_OKAY);
         DoMethod(BT_Cancel   , MUIM_Notify, MUIA_Pressed           , FALSE, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, app, 2, MUIM_Application_ReturnID, ID_CANCEL);
         set(BT_Okay  , MUIA_CycleChain, 1);
         set(BT_Cancel, MUIA_CycleChain, 1);

         if(LV_Usernames && (UserList.mlh_TailPred != (struct MinNode *)&UserList))
         {
            struct UserNode *user_node;

            user_node = (struct UserNode *)UserList.mlh_Head;
            while(user_node->un_node.mln_Succ)
            {
               DoMethod(LV_Usernames, MUIM_List_InsertSingle, user_node->un_user->us_name, MUIV_List_Insert_Bottom);
               user_node = (struct UserNode *)user_node->un_node.mln_Succ;
            }
         }

         set(win, MUIA_Window_Open, TRUE);
         if(name)
            set(win, MUIA_Window_ActiveObject, STR_Password);
         else
            set(win, MUIA_Window_ActiveObject, STR_Username);

         tries = 0;
         while((id = DoMethod(app, MUIM_Application_NewInput, &sigs)) != MUIV_Application_ReturnID_Quit)
         {
            if(id == ID_CANCEL)
               break;
            if(id == ID_OKAY)
            {
               get(STR_Username, MUIA_String_Contents, &name);
               get(STR_Password, MUIA_String_Contents, &ptr);

               if(user = find_user_verify_password(name, ptr))
                  break;
               else
               {
                  DisplayBeep(NULL);
                  nnset(STR_Password, MUIA_String_Contents, NULL);
                  set(win, MUIA_Window_ActiveObject, STR_Password);
                  tries++;
               }

               if(tries > 4)
                  break;
            }
            if(id == ID_OBJSTR)
            {
               char *x;

               DoMethod(LV_Usernames, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &x);
               if(x)
                  set(STR_Username, MUIA_String_Contents, x);
               DoMethod(PO_Username, MUIM_Popstring_Close, TRUE);
               set(win, MUIA_Window_ActiveObject, STR_Password);
            }
            if(sigs)
            {
               sigs = Wait(sigs | SIGBREAKF_CTRL_C);

               if(sigs & SIGBREAKF_CTRL_C)
                 break;
            }
         }
         MUI_DisposeObject(app);
      }
      CloseLibrary(MUIMasterBase);
      MUIMasterBase = NULL;
   }

   return(create_usercopy(user));
}

///
/// set_globaluser
VOID set_globaluser(struct User *user)
{
   if(global_user)
      FreeUser(global_user);
   global_user = NULL;

   if(!UserGroupBase)
      UserGroupBase = OpenLibrary(USERGROUPNAME, 0);

   if(UserGroupBase)
   {
      BPTR homedir;

      setreuid(-1, 0);  // clear usergroup user

      if(user && user->us_name)
      {
         global_user = create_usercopy(user);

         if(user->us_dir && (homedir = Lock(user->us_dir, SHARED_LOCK)))
            WriteFile("ENV:HOME", user->us_dir, -1);
         else
         {
            homedir = Lock("RAM:", SHARED_LOCK);
            WriteFile("ENV:HOME", "RAM:", -1);
         }
         if(homedir)
         {
            if(!AssignLock("HOME", homedir))
               UnLock(homedir);
         }
         setgid(user->us_gid);
         initgroups(user->us_name, user->us_gid);

         WriteFile("ENV:LOGNAME", user->us_name, -1);
         WriteFile("ENV:USER", user->us_name, -1);
         setlogin(user->us_name);
         setlastlog(user->us_uid, user->us_name, "Console");
         setuid(user->us_uid);
      }
      CloseLibrary(UserGroupBase);
      UserGroupBase = NULL;
   }
}

///
/// get_globaluser
struct User *get_globaluser(VOID)
{
   return(create_usercopy(global_user));
}

///
/// clear_userlist
VOID clear_userlist(VOID)
{
   if(UserList.mlh_TailPred != (struct MinNode *)&UserList)
   {
      struct UserNode *un1, *un2;

      un1 = (struct UserNode *)UserList.mlh_Head;
      while(un2 = (struct UserNode *)un1->un_node.mln_Succ)
      {
         Remove((struct Node *)un1);
         FreeUser(un1->un_user);
         FreeVec(un1);
         un1 = un2;
      }
   }
}

///
/// load_userlist
BOOL load_userlist(VOID)
{
   struct UserNode *user_node = NULL;
   struct User *user;
   struct RestrictedTime *restrict;
   BOOL success = FALSE;
   BPTR                    fh;
   ULONG                   utm_version;
   ULONG   i, num;

   clear_userlist();

   if(!(fh = Open("AmiTCP:db/utm.conf", MODE_OLDFILE)))
   {
      copy_passwd_file();
      fh = Open("AmiTCP:db/utm.conf", MODE_OLDFILE);
   }

   if(fh)
   {
      FRead(fh, &utm_version, sizeof(ULONG), 1);
      success = TRUE;

      while(user = AllocVec(sizeof(struct User), MEMF_ANY | MEMF_CLEAR))
      {
         NewList((struct List *)&user->us_restricted_times);

         if(user_node = AllocVec(sizeof(struct UserNode), MEMF_ANY))
         {
            user_node->un_user = user;
            AddTail((struct List *)&UserList, (struct Node *)user_node);
         }
         else
         {
            FreeVec(user);
            break;
         }

         if(!AllocFGetString(fh, &user->us_name))     // login
            break;
         if(!AllocFGetString(fh, &user->us_passwd))   // pwd
            break;
         FRead(fh, &user->us_uid, sizeof(LONG), 1);   // uid
         FRead(fh, &user->us_gid, sizeof(LONG), 1);   // gid
         if(!AllocFGetString(fh, &user->us_gecos))    // real name
            break;
         if(!AllocFGetString(fh, &user->us_dir))      // home dir
            break;
         if(!AllocFGetString(fh, &user->us_shell))    // shell
            break;
         FRead(fh, &num, sizeof(num), 1);             // # time restrictions
         for(i = 0; i < num; i++)
         {
            if(restrict = AllocVec(sizeof(struct RestrictedTime), MEMF_ANY | MEMF_CLEAR))
               AddTail((struct List *)&user->us_restricted_times, (struct Node *)restrict);

            restrict->rt_day = FGetC(fh);             // daynumber 0=sunday 1=monday, ...
            FGetC(fh);                                // dummy
            FRead(fh, &restrict->rt_start, sizeof(ULONG), 1);  // start time
            FRead(fh, &restrict->rt_end, sizeof(ULONG), 1);    // end time
            if(restrict->rt_end == 0)   // midnight is not 0 !
               restrict->rt_end = 1439;
            if(restrict->rt_end < restrict->rt_start) // make sure start is before end
               restrict->rt_end = restrict->rt_start + 1;
         }
         FRead(fh, &user->us_flags, sizeof(ULONG), 1);         // user flags
         FRead(fh, &user->us_max_time, sizeof(ULONG), 1);      // max time online
         if(!AllocFGetString(fh, &user->us_timeserver))        // timeserver
            break;
      }
      Close(fh);
   }

   return(success);
}
///

// public functions, everything needs to use semaphore !

/// GetUserName
SAVEDS ASM BOOL GetUserName(register __d0 LONG user_number, register __a0 STRPTR buffer, register __d1 LONG len)
{
   STRPTR name = NULL;

   if(buffer && len)
   {
      ObtainSemaphore(&LibSemaphore);
      if(name = get_username(user_number))
         strncpy(buffer, name, len);
      else
         *buffer = NULL;
      ReleaseSemaphore(&LibSemaphore);
   }
   return((BOOL)(name ? TRUE : FALSE));
}

///
/// GetUser
SAVEDS ASM struct User *GetUser(register __a0 STRPTR name, register __a1 STRPTR password, register __a2 STRPTR title, register __d0 LONG flags)
{
   struct User *user;

   ObtainSemaphore(&LibSemaphore);
   user = get_user(name, password, title, flags);
   ReleaseSemaphore(&LibSemaphore);

   return(user);
}

///
/// SetGlobalUser
SAVEDS ASM VOID SetGlobalUser(register __a0 struct User *user)
{
   ObtainSemaphore(&LibSemaphore);
   set_globaluser(user);
   ReleaseSemaphore(&LibSemaphore);
}

///
/// GetGlobalUser
SAVEDS ASM struct User *GetGlobalUser(VOID)
{
   struct User *user;

   ObtainSemaphore(&LibSemaphore);
   user = get_globaluser();
   ReleaseSemaphore(&LibSemaphore);

   return(user);
}

///
/// ClearUserList
SAVEDS ASM VOID ClearUserList(VOID)
{
   ObtainSemaphore(&LibSemaphore);
   clear_userlist();
   ReleaseSemaphore(&LibSemaphore);
}

///
/// ReloadUserList
SAVEDS ASM BOOL ReloadUserList(VOID)
{
   BOOL success = FALSE;

   ObtainSemaphore(&LibSemaphore);
   success = load_userlist();
   ReleaseSemaphore(&LibSemaphore);

   return(success);
}
///

// init & cleanup

/// __UserLibInit
SAVEDS ASM int __UserLibInit(register __a6 struct Library *base)
{
   int failed = TRUE;

   SysBase = (*((struct ExecBase **) 4));

   if(DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 0))
   {
      if(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library"   , 0))
      {
         InitSemaphore(&LibSemaphore);
         ObtainSemaphore(&LibSemaphore);
         global_user = NULL;
         NewList((struct List *)&UserList);
         load_userlist();
         ReleaseSemaphore(&LibSemaphore);

         failed = FALSE;
      }
   }
   return(failed);
}

///
/// __UserLibCleanup
SAVEDS ASM VOID __UserLibCleanup(register __a6 struct Library *base)
{
   ObtainSemaphore(&LibSemaphore);
   clear_userlist();
   ReleaseSemaphore(&LibSemaphore);

   if(global_user)
      FreeUser(global_user);
   global_user = NULL;

   if(IntuitionBase)    CloseLibrary((struct Library *)IntuitionBase);
   if(DOSBase)          CloseLibrary((struct Library *)DOSBase);
}

///

