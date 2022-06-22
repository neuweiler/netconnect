/// includes
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/memory.h>
#include <libraries/mui.h>
#include <pragma/exec_lib.h>
#include <pragma/dos_lib.h>
#include <pragma/intuition_lib.h>
#include <pragma/muimaster_lib.h>
#include <string.h>
#include <stormamigainline.h>

#include "libraries/genesis.h"
#include "/Genesis.h"

#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
///
/// variables
#pragma libbase GenesisBase
static const char version_string[] = "$VER:Genesis "VERTAG;

struct   DosLibrary    *DOSBase       = NULL;
struct   Library       *IntuitionBase = NULL;
struct   Library       *MUIMasterBase = NULL;

///

// internal functions, for your eyes only

/// create_usercopy
struct UserData *create_usercopy(struct UserData_intern *udi)
{
   struct UserData *ud;

   if(!udi)
      return(NULL);

   if(ud = AllocVec(sizeof(struct UserData), MEMF_ANY | MEMF_CLEAR))
   {
      strcpy(ud->ud_Name         , udi->udi_Name);
      strcpy(ud->ud_RealName     , udi->udi_RealName);
      strcpy(ud->ud_EMail        , udi->udi_EMail);
      strcpy(ud->ud_MailLogin    , udi->udi_MailLogin);
      strcpy(ud->ud_MailPassword , udi->udi_MailPassword);
      strcpy(ud->ud_MailServer   , udi->udi_MailServer);
   }
   return(ud);
}

///

// public functions that don't need blocking

/// GetFileSize
LONG GetFileSize(register __a0 STRPTR file)
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
BOOL ParseConfig(register __a0 STRPTR file, register __a1 struct pc_Data *pc_data)
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

               pc_data->Buffer   = buf;
               pc_data->Size     = size;
               pc_data->Current  = buf;

               pc_data->Argument = NULL;
               pc_data->Contents = NULL;
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
BOOL ParseNext(register __a0 struct pc_Data *pc_data)
{
   BOOL success = FALSE;
   STRPTR ptr_eol, ptr_tmp;

   if(pc_data->Current && pc_data->Current < pc_data->Buffer + pc_data->Size)
   {
      if(ptr_eol = strchr(pc_data->Current, '\n'))
      {
         *ptr_eol = NULL;

         if(pc_data->Contents = strchr(pc_data->Current, 34))              // is the content between ""'s ?
         {
            pc_data->Contents++;
            if(ptr_tmp = strchr(pc_data->Contents, 34))  // find the ending '"'
               *ptr_tmp = NULL;

            ptr_tmp = pc_data->Contents - 2;
            while(((*ptr_tmp == ' ') || (*ptr_tmp == 9)) && ptr_tmp >= pc_data->Current)
               ptr_tmp--;

            ptr_tmp++;
            *ptr_tmp = NULL;
         }
         else
         {
            pc_data->Contents = strchr(pc_data->Current, ' ');                   // a space
            ptr_tmp           = strchr(pc_data->Current, 9);                     // or a TAB

            if((ptr_tmp < pc_data->Contents && ptr_tmp) || !pc_data->Contents)   // which one comes first ?
               pc_data->Contents = ptr_tmp;
            if(pc_data->Contents)
            {
               *pc_data->Contents++ = NULL;
               while((*pc_data->Contents == ' ') || (*pc_data->Contents == 9))
                  pc_data->Contents++;

               if(ptr_tmp = strchr(pc_data->Contents, ';')) // cut out the comment
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
BOOL ParseNextLine(register __a0 struct pc_Data *pc_data)
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
VOID ParseEnd(register __a0 struct pc_Data *pc_data)
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
/// FreeUserData
VOID FreeUserData(register __a0 struct UserData *ud)
{
   if(ud)
      FreeVec(ud);
}

///

// internal functions, don't use semaphore here !

/// get_user
struct UserData_intern *get_user(long user_number, GenesisBase *base)
{
   if(base->gb_UserList.mlh_TailPred != (struct MinNode *)&base->gb_UserList)
   {
      struct UserData_intern *udi;
      LONG pos;

      pos = 0;
      udi = (struct UserData_intern *)base->gb_UserList.mlh_Head;
      while(udi->udi_Node.mln_Succ)
      {
         if(pos == user_number)
            return(udi);
         udi = (struct UserData_intern *)udi->udi_Node.mln_Succ;
         pos++;
      }
   }

   return(NULL)
}

///
/// get_user_by_name
struct UserData_intern *get_user_by_name(STRPTR user_name, GenesisBase *base)
{
   if(base->gb_UserList.mlh_TailPred != (struct MinNode *)&base->gb_UserList)
   {
      struct UserData_intern *udi;
      LONG pos;

      pos = 0;
      udi = (struct UserData_intern *)base->gb_UserList.mlh_Head;
      while(udi->udi_Node.mln_Succ)
      {
         if(!stricmp(udi->udi_Name, user_name))
            return(udi);
         udi = (struct UserData_intern *)udi->udi_Node.mln_Succ;
         pos++;
      }
   }
   return(NULL);
}

///
/// set_current_user_by_name
BOOL set_current_user_by_name(STRPTR user_name, GenesisBase *base)
{
   if(base->gb_UserList.mlh_TailPred != (struct MinNode *)&base->gb_UserList)
   {
      struct UserData_intern *udi;
      LONG pos;

      pos = 0;
      udi = (struct UserData_intern *)base->gb_UserList.mlh_Head;
      while(udi->udi_Node.mln_Succ)
      {
         if(!stricmp(udi->udi_Name, user_name))
         {
            base->gb_CurrentUser = pos;
            return(TRUE);
         }
         udi = (struct UserData_intern *)udi->udi_Node.mln_Succ;
         pos++;
      }
   }
   return(FALSE);
}

///

/// ask_for_user
#define ID_OKAY   42
#define ID_CANCEL 43

struct UserData_intern *ask_for_user(struct Libary *muibase, Object *app, Object *ref_win, char *name, GenesisBase *base)
{
   struct UserData_intern *udi = NULL;
   Object *win, *STR_Username, *STR_Password, *BT_Okay, *BT_Cancel;
   ULONG sigs, id;
   STRPTR username = NULL, password = NULL;

   if(!muibase || !app)
      return(NULL);

   MUIMasterBase = (struct Library *)muibase;
   if(name)
      udi = get_user_by_name(name, base);

   if(udi)
      if(!*udi->udi_Password)
         return(udi);

   set(app, MUIA_Application_Sleep, TRUE);
   if(win = WindowObject,
      MUIA_Window_Title    , "Genesis login",
      MUIA_Window_RefWindow   , ref_win,
      MUIA_Window_LeftEdge    , MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge     , MUIV_Window_TopEdge_Centered,
//      MUIA_Window_Height      , MUIV_Window_Height_MinMax(0),
//      MUIA_Window_Width       , MUIV_Window_Width_MinMax(0),
      WindowContents       , VGroup,
         Child, ColGroup(2),
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, KeyLabel("User:", 'u'),
            Child, STR_Username = StringObject,
               MUIA_ControlChar     , 'u',
               MUIA_CycleChain      , 1,
               MUIA_Frame           , MUIV_Frame_String,
               MUIA_String_MaxLen   , 40,
               MUIA_String_Contents , (udi ? udi->udi_Name : NULL),
            End,
            Child, KeyLabel("Password:", 'p'),
            Child, STR_Password = StringObject,
               MUIA_ControlChar     , 'p',
               MUIA_CycleChain      , 1,
               MUIA_Frame           , MUIV_Frame_String,
               MUIA_String_Secret   , TRUE,
               MUIA_String_MaxLen   , 40,
            End,
         End,
         Child, HGroup,
            Child, BT_Okay   = MUI_MakeObject(MUIO_Button, "_Okay"),
            Child, BT_Cancel = MUI_MakeObject(MUIO_Button, "_Cancel"),
         End,
      End,
   End)
   {
      DoMethod(app, OM_ADDMEMBER, win);
      DoMethod(STR_Username, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, STR_Password);
      DoMethod(STR_Password, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, win, 3, MUIM_Set, MUIA_Window_ActiveObject, BT_Okay);
      DoMethod(BT_Okay     , MUIM_Notify, MUIA_Pressed           , FALSE, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, app, 2, MUIM_Application_ReturnID, ID_OKAY);
      DoMethod(BT_Cancel   , MUIM_Notify, MUIA_Pressed           , FALSE, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, app, 2, MUIM_Application_ReturnID, ID_CANCEL);
      set(BT_Okay  , MUIA_CycleChain, 1);
      set(BT_Cancel, MUIA_CycleChain, 1);

      set(win, MUIA_Window_Open, TRUE);
      set(win, MUIA_Window_ActiveObject, STR_Username);

      while((id = DoMethod(app, MUIM_Application_NewInput, &sigs)) != MUIV_Application_ReturnID_Quit)
      {
         if(id == ID_CANCEL)
            break;
         if(id == ID_OKAY)
         {
            get(STR_Username, MUIA_String_Contents, &username);
            get(STR_Password, MUIA_String_Contents, &password);
            break;
         }
         if(sigs)
         {
            sigs = Wait(sigs | SIGBREAKF_CTRL_C);

            if(sigs & SIGBREAKF_CTRL_C)
               break;
         }
      }

      if(username)
      {
         if(udi = get_user_by_name(username, base))
         {
            if(*udi->udi_Password)
            {
               if(password)
               {
                  if(strcmp(password, udi->udi_Password))
                     udi = NULL;
               }
               else
                  udi = NULL;
            }
         }
      }
      else
         udi = NULL;

      DoMethod(app, OM_REMMEMBER, win);
      MUI_DisposeObject(win);
   }
   else
      udi = NULL;

   set(app, MUIA_Application_Sleep, FALSE);

   return(udi);
}

///
/// clear_userlist
VOID clear_userlist(GenesisBase *base)
{
   if(base->gb_UserList.mlh_TailPred != (struct MinNode *)&base->gb_UserList)
   {
      struct UserData_intern *udi1, *udi2;

      udi1 = (struct UserData_intern *)base->gb_UserList.mlh_Head;
      while(udi2 = (struct UserData_intern *)udi1->udi_Node.mln_Succ)
      {
         Remove((struct Node *)udi1);
         FreeVec(udi1);
         udi1 = udi2;
      }
   }
}

///
/// load_userlist
BOOL load_userlist(STRPTR file, GenesisBase *base)
{
   struct pc_Data pc_data;
   struct UserData_intern *udi = NULL;
   BOOL success = FALSE;

   if(!file)
      file = DEFAULT_CONFIGFILE;

   clear_userlist(base);

   if(ParseConfig(file, &pc_data))
   {
      while(ParseNext(&pc_data))
      {
         if(!stricmp(pc_data.Argument, "ISP") || !stricmp(pc_data.Argument, "INTERFACE") ||
            !stricmp(pc_data.Argument, "LOGINSCRIPT"))
            udi = NULL;
         else if(!stricmp(pc_data.Argument, "USER"))
         {
            if(udi = AllocVec(sizeof(struct UserData_intern), MEMF_ANY | MEMF_CLEAR))
               AddTail((struct List *)&base->gb_UserList, (struct Node *)udi);
         }
         else if(udi)
         {
            if(!stricmp(pc_data.Argument, "Name"))
               strcpy(udi->udi_Name, pc_data.Contents);
            if(!stricmp(pc_data.Argument, "Password"))
               decrypt(pc_data.Contents, udi->udi_Password);
            else if(!stricmp(pc_data.Argument, "RealName"))
               strcpy(udi->udi_RealName, pc_data.Contents);
            else if(!stricmp(pc_data.Argument, "EMail"))
               strcpy(udi->udi_EMail, pc_data.Contents);
            else if(!stricmp(pc_data.Argument, "MailLogin"))
               strcpy(udi->udi_MailLogin, pc_data.Contents);
            else if(!stricmp(pc_data.Argument, "MailPassword"))
               decrypt(pc_data.Contents, udi->udi_MailPassword);
            else if(!stricmp(pc_data.Argument, "MailServer"))
               strcpy(udi->udi_MailServer, pc_data.Contents);
         }
      }
      ParseEnd(&pc_data);
      success = TRUE;
   }

   return(success);
}
///

// public functions, everything needs to use semaphore !

/// GetUser
struct UserData *GetUser(register __d0 long user_number, register __a6 GenesisBase *base)
{
   struct UserData *ud = NULL;

   ObtainSemaphore(&base->gb_UserListSemaphore);
   ud = create_usercopy(get_user(user_number, base));
   ReleaseSemaphore(&base->gb_UserListSemaphore);

   return(ud);
}

///
/// GetCurrentUser
struct UserData *GetCurrentUser(register __a6 GenesisBase *base)
{
   // we don't have to use the semaphore here because the called function already does.
   return(GetUser(base->gb_CurrentUser, base));
}

///
/// GetUserByName
struct UserData *GetUserByName(register __a0 STRPTR user_name, register __a6 GenesisBase *base)
{
   struct UserData *ud = NULL;

   ObtainSemaphore(&base->gb_UserListSemaphore);
   ud = create_usercopy(get_user_by_name(user_name, base));
   ReleaseSemaphore(&base->gb_UserListSemaphore);

   return(ud);
}

///
/// SetCurrentUser
BOOL SetCurrentUser(register __d0 long user_number, register __a6 GenesisBase *base)
{
   if(user_number > -2)
   {
      base->gb_CurrentUser = user_number;
      return(TRUE);
   }
   return(FALSE);
}

///
/// SetCurrentUserByName
BOOL SetCurrentUserByName(register __a0 char *name, register __a6 GenesisBase *base)
{
   BOOL result;

   ObtainSemaphore(&base->gb_UserListSemaphore);
   result = set_current_user_by_name(name, base);
   ReleaseSemaphore(&base->gb_UserListSemaphore);

   return(result);
}

///
/// AskForUser
struct UserData *AskForUser(register __a0 struct Libary *muibase, register __a1 Object *app, register __a2 Object *ref_win, register __a3 char *name, register __a6 GenesisBase *base)
{
   struct UserData *ud = NULL;

   ObtainSemaphore(&base->gb_UserListSemaphore);
   ud = create_usercopy(ask_for_user(muibase, app, ref_win, name, base));
   ReleaseSemaphore(&base->gb_UserListSemaphore);

   return(ud);
}

///
/// ClearUserList
VOID ClearUserList(register __a6 GenesisBase *base)
{
   ObtainSemaphore(&base->gb_UserListSemaphore);
   clear_userlist(base);
   ReleaseSemaphore(&base->gb_UserListSemaphore);
}

///
/// LoadUserList
BOOL LoadUserList(register __a0 STRPTR file, register __a6 GenesisBase *base)
{
   BOOL success = FALSE;

   ObtainSemaphore(&base->gb_UserListSemaphore);
   success = load_userlist(file, base);
   ReleaseSemaphore(&base->gb_UserListSemaphore);

   return(success);
}
///

// init & cleanup

/// INIT_9_InitLib
void INIT_9_InitLib(register __a6 GenesisBase *base)
{
   DOSBase        = (struct DosLibrary *)OpenLibrary("dos.library", 0);
   IntuitionBase  = OpenLibrary("intuition.library"   , 0);

   InitSemaphore(&base->gb_UserListSemaphore);
   ObtainSemaphore(&base->gb_UserListSemaphore);
   base->gb_CurrentUser = -1;
   NewList((struct List *)&base->gb_UserList);
   load_userlist(NULL, base);
   ReleaseSemaphore(&base->gb_UserListSemaphore);
}

///
/// EXIT_9_ExitLib
void EXIT_9_ExitLib(register __a6 GenesisBase *base)
{
   ObtainSemaphore(&base->gb_UserListSemaphore);
   clear_userlist(base);
   ReleaseSemaphore(&base->gb_UserListSemaphore);

   if(IntuitionBase)    CloseLibrary(IntuitionBase);
   if(DOSBase)          CloseLibrary((struct Library *)DOSBase);
}

///

