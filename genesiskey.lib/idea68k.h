#ifndef IDEA68K_H
#define IDEA68K_H

#define IDEA_MODE_ENCRYPT 1
#define IDEA_MODE_DECRYPT 0

#define IDEA_SIZE_PASSWORD  16
#define IDEA_SIZE_SCHEDULE 216
#define IDEA_SIZE_IVEC       8

VOID __stdargs idea_key_schedule( APTR password,APTR schedule);
VOID __stdargs idea_cbc_encrypt(APTR src,APTR dst,int len,APTR keyschedule,APTR ivec,int mode);

#endif
