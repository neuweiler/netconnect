RCS_ID_C="$Id: login.c,v 4.12 1996/12/09 23:44:31 too Exp $";
/*
 *      login.c -- log in using usergroup.library
 *
 *      Copyright © 1994 AmiTCP/IP Group, 
 *                       Network Solutions Development Inc.
 *                       All rights reserved.
 */

#include "login_rev.h"

static const char version[] = VERSTAG;
static const char defrelstr[] =
  "AmiTCP/IP release 4.3";
static const char copyright[] =
  "\n"
  "Copyright © 1993 - 1996 AmiTCP/IP Group,\n"
  "            Network Solutions Development Inc., Finland.\n"
  "Copyright © 1980 - 1991 The Regents of the University of California.\n"
  "            All rights reserved.\n";

#include <sys/time.h>
#include <dos/rdargs.h>

#if defined(__SASC)
#include <proto/dos.h>
extern struct ExecBase *SysBase;
#include <pragmas/exec_sysbase_pragmas.h>
#include <clib/exec_protos.h>
#include <proto/usergroup.h>
/* Disable ^C signaling */
void __regargs __chkabort(void) {}
#else
#include <clib/exec_protos.h>
#include <clib/usergroup_protos.h>
#include <clib/dos_protos.h>
#endif

#include <sys/param.h>
#include <sys/syslog.h>

#include <string.h>
#include <stdlib.h>

#include <pwd.h>
#include <grp.h>
#include <utmp.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#include "config.h"
#include "pathnames.h"

#include <bsdsocket.h>

#include <amitcp/socketbasetags.h>

BPTR Stdin, Stdout, Stderr;
APTR WinPtr = NULL;
struct RDArgs *rdargs = NULL;

struct Library *SocketBase = NULL;
static UBYTE SOCKETNAME[] = "bsdsocket.library";
#define SOCKETVERSION 4		/* minimum bsdsocket version to use */
int errno;

static void 
syslog_AmiTCP(ULONG level, const STRPTR format, ...)
{
  va_list va;
  va_start(va, format);
  if (SocketBase) {
    vsyslog(level, format, (LONG *)va);
  }
  va_end(va);
}

static int gethostname_AmiTCP(char *name, int namelen)
{
  if (SocketBase) {
    return (int)gethostname(name, namelen);
  } else {
    char *cp = getenv("HOSTNAME");

    if (cp == NULL) {
      errno = ENOENT;
      return -1;
    }

    strncpy(name, cp, namelen);
    free(cp);
    return 0;
  }
}

#define syslog      syslog_AmiTCP
#define gethostname gethostname_AmiTCP

static void resume_amiga_stdio(void)
{
  struct Process* me = (struct Process *)FindTask(NULL);
  me->pr_WindowPtr = WinPtr;

  if (rdargs)
    FreeArgs(rdargs);
  rdargs = NULL;

  if (SocketBase)
    CloseLibrary(SocketBase);
  SocketBase = NULL;
}

static void startup_amiga_stdio(void)
{
  struct Process* me = (struct Process *)FindTask(NULL);
  Stdin = me -> pr_CIS;
  Stdout = me -> pr_COS;
  Stderr = (me -> pr_CES ? me -> pr_CES : Stdout);

  /* Remove requesters */
  WinPtr = me->pr_WindowPtr;
  me->pr_WindowPtr = (void *)-1L;
  atexit(resume_amiga_stdio);

  /* Open SocketBase (only for logging) */
  if (!SocketBase) {
    if ((SocketBase = OpenLibrary(SOCKETNAME, SOCKETVERSION)) != NULL) {
      SetErrnoPtr(&errno, sizeof(errno));
    }
  }
}

void endexit(int retval)
{
  exit(retval);
}

char *hostname, *username;
int failures;

void badlogin(char *name);
void getloginname(void);

void main()
{
  char *domain, localhost[MAXHOSTNAMELEN];
  int cnt, ask, rootlogin = 0, quietlog;
  uid_t uid = getuid();
  struct passwd *pwd;
  char linebuf[128];
  
  const char *args_template = 
    "-f=FORCE/S,-p=PRESERVE/S,-a=ALL/S,-h=HOST/K,USERNAME";
  struct {
    LONG   a_force;
    LONG   a_preserve;
    LONG   a_all;
    STRPTR a_host;
    STRPTR a_name;
  } args[1] = { 0 };
  BYTE oldpriority;

  startup_amiga_stdio();

#ifdef HAVE_OPENLOG
  openlog("login", LOG_ODELAY, LOG_AUTH);
#endif

  domain = NULL;

  if (gethostname(localhost, sizeof(localhost)) < 0)
    syslog(LOG_ERR, "couldn't get local hostname: %m");
  else
    domain = index(localhost, '.');

  rdargs = ReadArgs((STRPTR)args_template, (LONG *)args, NULL);
  if (!rdargs) {
    if (uid == 0)
      syslog(LOG_ERR, "invalid arguments \"%s\"", GetArgStr());
    PrintFault(IoErr(), "login");
    endexit(RETURN_ERROR);
  }

  if (args->a_host) {
#ifdef notyet
    if (uid != 0) {
      FPrintf(Stderr, "login: host option: %s\n", strerror(EPERM));
      endexit(RETURN_ERROR);
    }
#endif
    hostname = args->a_host;
  }

  if (args->a_name) {
    ask = 0;
    username = args->a_name;
  } else {
    if (args->a_force) {
      /*
       * If -f is given check environment variable LOGNAME for default user name
       */
      username = getenv("LOGNAME");
      /* ask if didn't get the name */
      ask = username == NULL || username[0] == '\0';	
    }
    else
      ask = 1;
  }

  for (cnt = 0;; ask = 1) {
    char *p, *salt;
    int match;

    if (ask) {
      args->a_force = 0;
      getloginname();
    }
    if (strlen(username) > UT_NAMESIZE)
      username[UT_NAMESIZE] = '\0';

    if (pwd = getpwnam(username))
      salt = pwd->pw_passwd;
    else
      salt = "xx";

    /*
     * if we have a valid account name, and it doesn't have a
     * password, or the FORCE option was specified and the caller
     * is root or the caller isn't changing their uid, don't
     * authenticate.
     */
    if (pwd && args->a_force && (uid == 0 || uid == pwd->pw_uid))
      break;

    args->a_force = 0;

    /* Don't log rootlogin if there is no root password */
    if (pwd && pwd->pw_passwd[0] == '\0')
      break;

    if (pwd && pwd->pw_uid == 0)
      rootlogin = 1;
    else
      rootlogin = 0;

    /* oldpriority = SetTaskPri(FindTask(NULL), -98); */

    p = getpass("Password:");

    if (pwd) {
      match = !strcmp(crypt(p, salt), pwd->pw_passwd);
    }
     
    bzero(p, strlen(p));

    /* SetTaskPri(FindTask(NULL), oldpriority); */

    if (pwd != NULL && match)
      break;

    PutStr("Login incorrect\n");

    failures++;

    /* we allow 10 tries, but after 3 we start backing off */
    if (++cnt > 3) {
      if (cnt >= 10) {
	badlogin(username);
	endexit(RETURN_ERROR);
      }
      Delay((u_int)((cnt - 3) * 5 * 50));
    }
  }    

  /*
   * check nologin if user has got no force (member of wheel)
   */
  if (pwd->pw_gid != 0)  {
    BPTR nologin = Open(_PATH_NOLOGIN, MODE_OLDFILE);
 
    if (nologin) {
      LONG n;

      FPuts(Stdout, "No logins are allowed.\n");

      do {
	n = Read(nologin, linebuf, sizeof(linebuf));
	Write(Stdout, linebuf, n);
      } while (n == sizeof(linebuf));

      Close(nologin);
      exit(RETURN_ERROR);
    }
  }

#if 0
  {
    BPTR homedir, oldcwd, hushlock;
    
    homedir = Lock(pwd->pw_dir, SHARED_LOCK);

    if (homedir == NULL || !SetCurrentDirName(pwd->pw_dir)) {
      FPrintf(Stderr, "Cannot change to home directory %s\n"
	      "Logging in with home = \"SYS:\".\n", pwd->pw_dir);
      if (homedir) 
	UnLock(homedir);
      homedir = Lock("SYS:", SHARED_LOCK);
      SetCurrentDirName(pwd->pw_dir = "SYS:");
    } 
    oldcwd = CurrentDir(homedir);

    if (hushlock = Lock(_PATH_HUSHLOGIN, SHARED_LOCK))
      UnLock(hushlock);

    quietlog = hushlock != 0;

    /* Make assign to home dir */
    if (homedir) {
      if (homedir = DupLock(homedir)) {
	if (!AssignLock("HOME", homedir)) {
	  PrintFault(IoErr(), "AssignLock"); 
	}
      } else {
	PrintFault(IoErr(), "DupLock"); 
      }
    }

    UnLock(oldcwd);
  }
#else
  {
    BPTR homedir, oldcwd, hushlock;
    
    homedir = Lock(pwd->pw_dir, SHARED_LOCK);

    if (homedir == NULL) {
      PrintFault(IoErr(), pwd->pw_dir); 
      homedir = Lock("SYS:", SHARED_LOCK);
    }

    oldcwd = CurrentDir(homedir);

    if (hushlock = Lock(_PATH_HUSHLOGIN, SHARED_LOCK))
      UnLock(hushlock);

    quietlog = hushlock != 0;

    /* Make assign to home dir */
    if (homedir = DupLock(homedir)) {
      if (!AssignLock("HOME", homedir)) {
	PrintFault(IoErr(), "AssignLock"); 
      }
    } else {
      PrintFault(IoErr(), "DupLock"); 
    }

    homedir = CurrentDir(oldcwd);
    
    UnLock(homedir);
  }
#endif

  if (!quietlog) {
    BPTR motd;
    /* Print the last login time */
    struct lastlog *ll = getlastlog(pwd->pw_uid);

    if (!quietlog && ll != NULL && ll->ll_time != 0) {
      char *lfp, *tp = ctime(&ll->ll_time);
      if (lfp = index(tp, '\n'))
	*lfp = '\0';
      Printf("Last login: %19s ", tp);
      if (*ll->ll_host != '\0')
	Printf("from %s\n", ll->ll_host);
      else 
	Printf("\n");
    }

    /* Print out the release information */
    {
      STRPTR releasestring = (STRPTR)defrelstr;	/* set default string */
      
      if (SocketBase != NULL) {
	SocketBaseTags(SBTM_GETREF(SBTC_RELEASESTRPTR), &releasestring,
		       TAG_END);
      }

      Write(Stdout, (UBYTE *)releasestring, strlen(releasestring));
    }

    /* Print copyright message */
    Write(Stdout, (UBYTE *)copyright, sizeof(copyright) - 1);

    /* Print message of the day */
    if (motd = Open(_PATH_MOTDFILE, MODE_OLDFILE)) {
      LONG n;
      for (;;) {
	n = Read(motd, linebuf, sizeof(linebuf));
	Write(Stdout, linebuf, n);
	if (n != sizeof(linebuf))
	  break;
      }
      Close(motd);
    }
  }

#ifndef HAVE_SUID
  /* Emulate suid bit effect if no force was used */
  if (!args->a_force)
    (void)setreuid(-1, 0);
#endif

  (void)setgid(pwd->pw_gid);

  initgroups(username, pwd->pw_gid);

  SetVar("HOME", pwd->pw_dir, -1, GVF_GLOBAL_ONLY | LV_VAR);
#if 0
  SetVar("SHELL", pwd->pw_shell, -1, GVF_GLOBAL_ONLY | LV_VAR);
#endif
  SetVar("LOGNAME", pwd->pw_name, -1, GVF_GLOBAL_ONLY | LV_VAR);
  SetVar("USER", pwd->pw_name, -1, GVF_GLOBAL_ONLY | LV_VAR);

  if (setlogin(pwd->pw_name) < 0 && getuid() == 0) 
    syslog(LOG_ERR, "setlogin() failure: %s", ug_StrError(errno));

  if ( setlastlog(pwd->pw_uid, username, 
		  hostname ? hostname : "Console") == -1)
    syslog(LOG_ERR, "setlastlog() failure: %s", ug_StrError(errno));

  if (rootlogin)
    (void) setuid(0);
  else
    (void) setuid(pwd->pw_uid);

  exit(0);
}

#define	NBUFSIZ		(UT_NAMESIZE + 1)

/*
 * Ask for login name
 */
void getloginname(void)
{
  int ch;
  register char *p;
  static char nbuf[NBUFSIZ + 1];
  
  for (;;) {
    PutStr("login: "); Flush(Stdout);
    for (p = nbuf; (ch = FGetC(Stdin)) != '\n'; ) {
      if (ch == -1) {
	/* EOF */
	Write(Stdout, "\n", 1);
	badlogin(username);
	endexit(RETURN_ERROR);
      }
      if (p < nbuf + (NBUFSIZ - 1))
	*p++ = ch;
    }
    if (p > nbuf)
      if (nbuf[0] == '-') {
	PutStr("login names may not start with '-'.\n");
      } else {
	*p = '\0';
	username = nbuf;
	break;
      }
  }
}

/*
 * log a bad login
 */
void badlogin(char *name)
{
  if (failures == 0)
    return;

  if (hostname) {
    syslog(LOG_NOTICE, "%ld LOGIN FAILURE%s FROM %s",
	   failures, failures > 1 ? "S" : "", hostname);
    syslog(LOG_AUTHPRIV|LOG_NOTICE,
	   "%ld LOGIN FAILURE%s FROM %s, %s",
	   failures, failures > 1 ? "S" : "", hostname, name);
  } else 
    {
    syslog(LOG_NOTICE, "%ld LOGIN FAILURE%s",
	   failures, failures > 1 ? "S" : "");
    syslog(LOG_AUTHPRIV|LOG_NOTICE,
	   "%ld LOGIN FAILURE%s %s",
	   failures, failures > 1 ? "S" : "", name);
  }
}
