/// includes & defines
#include <devices/netinfo.h>

#include "libraries/genesis.h"
#include "/Genesis.h"

#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#define ID_OKAY   42
#define ID_CANCEL 43
#define ID_OBJSTR 44
#define ID_STROBJ 45
///
/// variables
struct   DosLibrary      *DOSBase       = NULL;
struct   ExecBase        *SysBase       = NULL;
struct   IntuitionBase   *IntuitionBase = NULL;
struct   Library         *UserGroupBase = NULL;
struct   Library         *MUIMasterBase = NULL;

struct   MsgPort         *netinfo_port  = NULL;
struct   NetInfoReq      *netinfo_req   = NULL;
struct   NetInfoPasswd   *passwd;
struct   User            *global_user   = NULL;
struct   SignalSemaphore LibSemaphore;
struct   MinList         UserList;
char     pw_buffer[1024];

struct UserNode
{
   struct MinNode un_Node;

   char un_Name[41];
};

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
            if(ptr_tmp = strchr(pc_data->pc_contents, 34))  // find the ending '"'
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

// internal functions, for your eyes only

/// netinfo_delete
VOID netinfo_delete(VOID)
{
   if(netinfo_req)
   {
      CloseDevice((struct IORequest *)netinfo_req);
      DeleteIORequest(netinfo_req);
      netinfo_req = NULL;
   }
   if(netinfo_port)
      DeleteMsgPort(netinfo_port);
   netinfo_port = NULL;
}

///
/// netinfo_create
BOOL netinfo_create(VOID)
{
   if(netinfo_port = (struct MsgPort *)CreateMsgPort())
   {
      if(netinfo_req = (struct NetInfoReq *)CreateIORequest(netinfo_port, sizeof(struct NetInfoReq)))
      {
         if(!(OpenDevice(NETINFONAME, NETINFO_PASSWD_UNIT, (struct IORequest *)netinfo_req, 0)))
         {
            return(TRUE);
         }
      }
   }
   netinfo_delete();
   return(FALSE);
}

///
/// create_usercopy
struct User *create_usercopy(struct NetInfoPasswd *passwd)
{
   struct User *user;

   if(user = AllocVec(sizeof(struct User), MEMF_ANY | MEMF_CLEAR))
   {
      ReallocCopy(&user->us_name    , passwd->pw_name);
      ReallocCopy(&user->us_passwd  , passwd->pw_passwd);
      ReallocCopy(&user->us_gecos   , passwd->pw_gecos);
      ReallocCopy(&user->us_dir     , passwd->pw_dir);
      ReallocCopy(&user->us_shell   , passwd->pw_shell);
      user->us_uid = passwd->pw_uid;
      user->us_gid = passwd->pw_gid;
   }
   return(user);
}

///
/// passwd_by_name
struct NetInfoPasswd *passwd_by_name(char *name)
{
   BOOL ok = FALSE;

   if(netinfo_create())
   {
      passwd = (struct NetInfoPasswd *)pw_buffer;
      passwd->pw_name = name;
      netinfo_req->io_Data    = passwd;
      netinfo_req->io_Length  = sizeof(pw_buffer);
      netinfo_req->io_Command = NI_GETBYNAME;
      if(!DoIO((struct IORequest *)netinfo_req))
         ok = TRUE;

      netinfo_delete();

      if(ok)
         return(passwd);
   }
   return(NULL);
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
      while(user_node->un_Node.mln_Succ)
      {
         if(i == pos)
            return(user_node->un_Name);
         user_node = (struct UserNode *)user_node->un_Node.mln_Succ;
         i++;
      }
   }

   return(NULL);
}

///
/// get_user
struct User *get_user(char *name, char *title, LONG flags)
{
   Object *app, *win, *PO_Username = NULL, *STR_Username = NULL, *LV_Usernames = NULL,
          *STR_Password, *BT_Okay, *BT_Cancel;
   ULONG sigs = NULL, id;
   STRPTR password = NULL;
   struct User *user = NULL;
   int tries;

   if(name)
      if(!passwd_by_name(name))
         name = NULL;

   if(name && !*passwd->pw_passwd)
      return(create_usercopy(passwd));

   if(MUIMasterBase = OpenLibrary("muimaster.library", 11))
   {
      if(app = ApplicationObject,
         MUIA_Application_Title      , "Genesis User Request",
         MUIA_Application_Version    , "$VER: GenesisUserReq 1.0 (07.06.98)",
         MUIA_Application_Copyright  , "©1998, Michael Neuweiler",
         MUIA_Application_Author     , "Michael Neuweiler",
         MUIA_Application_Description, "Requester to login a user",
         MUIA_Application_Base       , "GENESISUSERREQ",
         SubWindow, win = WindowObject,
            MUIA_Window_ID       , MAKE_ID('U','R','E','Q'),
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
            while(user_node->un_Node.mln_Succ)
            {
               DoMethod(LV_Usernames, MUIM_List_InsertSingle, user_node->un_Name, MUIV_List_Insert_Bottom);
               user_node = (struct UserNode *)user_node->un_Node.mln_Succ;
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
               STRPTR salt;
               if(!name)
                  get(STR_Username, MUIA_String_Contents, &name);

               if(passwd_by_name(name))
               {
                  get(STR_Password, MUIA_String_Contents, &password);

                  if(*passwd->pw_passwd)
                     salt = passwd->pw_passwd;
                  else
                  {
                     user = create_usercopy(passwd);
                     break;
                  }

                  if(!UserGroupBase)
                     UserGroupBase = OpenLibrary(USERGROUPNAME, 0);

                  if(UserGroupBase)
                  {
                     if(!strcmp(crypt(password, salt), passwd->pw_passwd))
                     {
                        user = create_usercopy(passwd);
                        break;
                     }
                     else
                     {
                        DisplayBeep(NULL);
                        nnset(STR_Password, MUIA_String_Contents, NULL);
                        set(win, MUIA_Window_ActiveObject, STR_Password);
                        tries++;
                     }
                     CloseLibrary(UserGroupBase);
                     UserGroupBase = NULL;
                  }
               }
               else
               {
                  DisplayBeep(NULL);
                  nnset(STR_Username, MUIA_String_Contents, NULL);
                  nnset(STR_Password, MUIA_String_Contents, NULL);
                  set(win, MUIA_Window_ActiveObject, STR_Username);
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

   return(user);
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

      if(user)
      {
         global_user = create_usercopy((struct NetInfoPasswd *)user);

         if(homedir = Lock(user->us_dir, SHARED_LOCK))
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
   if(global_user)
      return(create_usercopy((struct NetInfoPasswd *)global_user));

   return(NULL);
}

///
/// clear_userlist
VOID clear_userlist(VOID)
{
   if(UserList.mlh_TailPred != (struct MinNode *)&UserList)
   {
      struct UserNode *un1, *un2;

      un1 = (struct UserNode *)UserList.mlh_Head;
      while(un2 = (struct UserNode *)un1->un_Node.mln_Succ)
      {
         Remove((struct Node *)un1);
         FreeVec(un1);
         un1 = un2;
      }
   }
}

///
/// load_userlist
BOOL load_userlist(VOID)
{
   struct UserNode *un = NULL;
   BOOL success = FALSE;

   clear_userlist();

   if(netinfo_create())
   {
      netinfo_req->io_Command = CMD_RESET;
      if(!DoIO((struct IORequest *)netinfo_req))
      {
         passwd = (struct NetInfoPasswd *)pw_buffer;
         netinfo_req->io_Data    = passwd;
         netinfo_req->io_Length  = sizeof(pw_buffer);
         netinfo_req->io_Command = CMD_READ;
         while(!DoIO((struct IORequest *)netinfo_req))
         {
            if(!(passwd->pw_passwd[0] == '*' && passwd->pw_passwd[1] == NULL))
            if(un = AllocVec(sizeof(struct UserNode), MEMF_ANY | MEMF_CLEAR))
            {
               strncpy(un->un_Name, passwd->pw_name, sizeof(un->un_Name));
               AddTail((struct List *)&UserList, (struct Node *)un);
            }

            netinfo_req->io_Data    = passwd;
            netinfo_req->io_Length  = sizeof(pw_buffer);
            netinfo_req->io_Command = CMD_READ;
         }

         success = TRUE;
      }
      netinfo_delete();
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
SAVEDS ASM struct User *GetUser(register __a0 STRPTR name, register __a1 STRPTR title, register __d0 LONG flags)
{
   struct User *user;

   ObtainSemaphore(&LibSemaphore);
   user = get_user(name, title, flags);
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

