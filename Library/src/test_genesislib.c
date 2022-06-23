#include "libraries/genesis.h"
#include "proto/genesis.h"
#include "pragmas/genesis_lib.h"

struct GenesisBase *GenesisBase;

void main()
{
   if(GenesisBase = (struct GenesisBase *) OpenLibrary(GENESISNAME, 1))
   {
      LONG pos=0;
      struct User *user;
      char buffer[81];

      Printf("\nbrowsing username list:\n");
      while(GetUserName(pos++, buffer, 80))
         Printf("user: %ls\n", buffer);

      Printf("\ntesting GetGlobalUser()\n");
      if(user = GetGlobalUser())
      {
         Printf("GetGlobalUser was successful.\nUser: %ls\nPassword: %ls\nUID: %ld\nGID: %ld\nGecos: %ls\nHomeDir: %ls\nShell: %ls\n",
            user->us_name, user->us_passwd, user->us_uid, user->us_gid, user->us_gecos,
            user->us_dir, user->us_shell);
         FreeUser(user);
      }
      else
         Printf("GetGlobalUser was NOT successful.\n");

      Printf("\ntesting normal GetUser()\n");
      if(user = GetUser(NULL, NULL, "please identify yourself", NULL))
      {
         Printf("GetUser was successful.\nUser: %ls\nPassword: %ls\nUID: %ld\nGID: %ld\nGecos: %ls\nHomeDir: %ls\nShell: %ls\n",
            user->us_name, user->us_passwd, user->us_uid, user->us_gid, user->us_gecos,
            user->us_dir, user->us_shell);

         Printf("setting global user to this user\n.");
         SetGlobalUser(user);

         FreeUser(user);
      }
      else
         Printf("GetUser was NOT successful.\n");

      Printf("\nnow testing GetGlobalUser() again. This should return the user we've set before with SetGlobalUser()\n");
      if(user = GetGlobalUser())
      {
         Printf("GetGlobalUser was successful.\nUser: %ls\nPassword: %ls\nUID: %ld\nGID: %ld\nGecos: %ls\nHomeDir: %ls\nShell: %ls\n",
            user->us_name, user->us_passwd, user->us_uid, user->us_gid, user->us_gecos,
            user->us_dir, user->us_shell);

         FreeUser(user);
      }
      else
         Printf("GetGlobalUser was NOT successful.\n");

      Printf("\nclearing global user.\n");
      SetGlobalUser(NULL);
      Printf("calling GetGlobalUser() again. (after clearing it)\n");
      if(user = GetGlobalUser())
      {
         Printf("GetGlobalUser was successful.\nUser: %ls\nPassword: %ls\nUID: %ld\nGID: %ld\nGecos: %ls\nHomeDir: %ls\nShell: %ls\n",
            user->us_name, user->us_passwd, user->us_uid, user->us_gid, user->us_gecos,
            user->us_dir, user->us_shell);

         FreeUser(user);
      }
      else
         Printf("GetGlobalUser was NOT successful.\n");

      Printf("\ntesting GetUser() with given username \"root\"\n");
      if(user = GetUser("root", NULL, NULL, GUF_TextObject))
      {
         Printf("GetUser(\"root\") was successful.\nUser: %ls\nPassword: %ls\nUID: %ld\nGID: %ld\nGecos: %ls\nHomeDir: %ls\nShell: %ls\n",
            user->us_name, user->us_passwd, user->us_uid, user->us_gid, user->us_gecos,
            user->us_dir, user->us_shell);

         FreeUser(user);
      }
      else
         Printf("GetUser(\"root\" was NOT successful.\n");

      Printf("is any interface online ?  %ls\n", (IsOnline(NULL) ? "yes" : "no"));

      CloseLibrary((struct Library *) GenesisBase);
   }
   else
      printf("unable to open \"genesis.library\".");
}
