#ifndef GENESIS_LIB_PROTOS_H
#define GENESIS_LIB_PROTOS_H

LONG   GetFileSize(STRPTR file);
BOOL   ParseConfig(STRPTR file, struct ParseConfig_Data *pc_data);
BOOL   ParseNext(struct ParseConfig_Data *pc_data);
BOOL   ParseNextLine(struct ParseConfig_Data *pc_data);
VOID   ParseEnd(struct ParseConfig_Data *pc_data);
VOID   FreeUser(struct User *user);
STRPTR ReallocCopy(STRPTR *old, STRPTR src);
LONG   ReadFile(STRPTR file, STRPTR buffer, LONG len);
BOOL   WriteFile(STRPTR file, STRPTR buffer, LONG len);

BOOL   GetUserName(LONG user_number, char *buffer, LONG len);
struct User *GetUser(STRPTR name, STRPTR password, STRPTR title, LONG flags);
struct User *GetGlobalUser(VOID);
VOID   SetGlobalUser(struct User *user);
VOID   ClearUserList(VOID);
BOOL   ReloadUserList(VOID);
BOOL   IsOnline(LONG flags);
#endif
