
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <pragma/exec_lib.h>
#include <pragma/dos_lib.h>
#include <pragma/intuition_lib.h>
#include <pragma/muimaster_lib.h>
#include <libraries/mui.h>
#include <stdio.h>

#include "libraries/genesis.h"
#include "proto/genesis.h"
#include "genesis_lib.h"

#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))

struct GenesisBase *GenesisBase;
struct Library *MUIMasterBase;

void main()
{
   if(GenesisBase = (struct GenesisBase *) OpenLibrary(GENESISNAME, 1))
   {
      LONG pos=0;
      struct UserData *ud;

      while(TRUE)
      {
         if(!(ud = GetUser(pos++)))
            break;
         Printf("User: %ls\nReal: %ls\nEMail: %ls\nMailLogin: %ls\nMailPassword: %ls\nMailServer: %ls\n\n", ud->ud_Name, ud->ud_RealName, ud->ud_EMail, ud->ud_MailLogin, ud->ud_MailPassword, ud->ud_MailServer);
      }

      if(MUIMasterBase = (struct Library *)OpenLibrary("muimaster.library", 0))
      {
         Object *app, *win;

         if(app = ApplicationObject,
            MUIA_Application_Author       , "Michael Neuweiler",
            MUIA_Application_Base         , "genesis.library test",
            MUIA_Application_Title        , "genesis.library test",
            MUIA_Application_Window       , win = WindowObject,
               MUIA_Window_Title    , "Genesis login",
               MUIA_Window_ID       , MAKE_ID('m','a','i','n'),
               WindowContents       , VGroup,
                  Child, HVSpace,
                  Child, CLabel("Just testin that funny AskForUser()"),
                  Child, HVSpace,
               End,
            End,
         End)
         {
            set(win, MUIA_Window_Open, TRUE);
            if(ud = AskForUser(MUIMasterBase, app, win, NULL))
               Printf("AskForUser() successful:\nUser: %ls\nReal: %ls\nEMail: %ls\nMailLogin: %ls\nMailPassword: %ls\nMailServer: %ls\n\n", ud->ud_Name, ud->ud_RealName, ud->ud_EMail, ud->ud_MailLogin, ud->ud_MailPassword, ud->ud_MailServer);
            else
               Printf("AskForUser() not successful\n");

            MUI_DisposeObject(app);
         }
         CloseLibrary(MUIMasterBase);
      }

      CloseLibrary((struct Library *) GenesisBase);
   }
   else
   {
      printf("unable to open \"genesis.library\".");
   };
}
