/// serial.c
VOID serial_stopread(VOID);
VOID serial_startread(STRPTR data, LONG len);
VOID serial_send(STRPTR cmd, LONG len);
BOOL serial_waitfor(STRPTR string, int secs);
BOOL serial_carrier(VOID);
BOOL serial_dsr(VOID);
VOID serial_clear(VOID);
VOID serial_hangup(VOID);
BOOL serial_create(STRPTR device, LONG unit);
VOID serial_delete(VOID);

///

