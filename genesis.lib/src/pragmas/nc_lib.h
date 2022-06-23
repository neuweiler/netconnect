#pragma libcall NetConnectBase NCL_OpenSocket 36 0
#pragma libcall NetConnectBase NCL_GetSerial 42 0
#pragma libcall NetConnectBase NCL_GetOwner 4e 0
#pragma libcall NetConnectBase NCL_CallMeSometimes 54 0
#pragma libcall NetConnectBase NCL_CallMeFrequently 5a 0

struct Library *NCL_OpenSocket( void );
char *NCL_GetSerial( void );
char *NCL_GetOwner( void );
void NCL_CallMeSometimes( void );
void NCL_CallMeFrequently( void );
