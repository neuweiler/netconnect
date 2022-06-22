#ifndef LIB_PROTOS_H
#define LIB_PROTOS_H

#ifdef __cplusplus
extern "C" {
#endif

LONG   GetFileSize(STRPTR file);
BOOL   ParseConfig(STRPTR file, struct pc_Data *pc_data);
BOOL   ParseNext(struct pc_Data *pc_data);
BOOL   ParseNextLine(struct pc_Data *pc_data);
BOOL   ParseEnd(struct pc_Data *pc_data);
VOID   FreeUserData(struct UserData *ud);
struct UserData *GetUser(LONG user_number);
struct UserData *GetCurrentUser(VOID);
struct UserData *GetUserByName(STRPTR username);
BOOL   SetCurrentUser(long user_number);
BOOL   SetCurrentUserByName(char *name);
struct UserData *AskForUser(struct Library *muilib, Object *app, Object *win, STRPTR name);
VOID   ClearUserList(VOID);
BOOL   LoadUserList(STRPTR file);

#ifdef __cplusplus
};
#endif

#endif
