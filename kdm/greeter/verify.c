/* $TOG: verify.c /main/37 1998/02/11 10:00:45 kaleb $ */
/* $Id$ */
/*

Copyright 1988, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/xdm/greeter/verify.c,v 3.9 2000/06/14 00:16:16 dawes Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * verify.c
 *
 * typical unix verification routine.
 */

#include "dm.h"
#include "dm_error.h"

#include <pwd.h>
#ifdef USE_PAM
# include <security/pam_appl.h>
# ifdef KDE_PAM_SERVICE
#  define KDE_PAM KDE_PAM_SERVICE
# else  
#  define KDE_PAM "xdm"  /* default PAM service called by kdm */
# endif 
#elif defined(AIXV3) /* USE_PAM */
# include <login.h>
# include <usersec.h>
extern int loginrestrictions (char *Name, int Mode, char *Tty, char **Msg);
extern int loginfailed (char *User, char *Host, char *Tty);
extern int loginsuccess (char *User, char *Host, char *Tty, char **Msg);
#else /* USE_PAM || AIXV3 */
# ifdef USESHADOW
#  include <shadow.h>
#  include <errno.h>
#  ifdef X_NOT_STDC_ENV
extern int errno;
#  endif
# endif
# ifdef HAVE_CRYPT_H
/* Some systems define an des_encrypt() in their crypt.h (where it
 * doesn't belong!), which conflicts with the one from KTH Kerberos.
 */
#  define des_encrypt __des_encrypt_faked
#  include <crypt.h>
#  undef des_encrypt
# endif
# ifdef KRB4
#  include <sys/param.h>
static char krbtkfile[MAXPATHLEN];
# endif
# ifdef SECURE_RPC
#  include <rpc/rpc.h>
#  include <rpc/key_prot.h>
# endif
# ifdef K5AUTH
#  include <krb5/krb5.h>
# endif
# if defined(HAVE_LOGIN_CAP_H) && !defined(__NetBSD__)
#  define USE_LOGIN_CAP 1
#  include <login_cap.h>
# endif
/* for nologin */
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
/* for expiration */
# include <time.h>
#endif	/* USE_PAM || AIXV3 */

#ifdef X_NOT_STDC_ENV
char *getenv();
#endif

#ifdef QNX4
extern char *crypt(const char *, const char *);
#endif

#include "miscfunc.h"
#include "greet.h"

#ifdef USE_PAM

static char *PAM_password;

#ifdef PAM_MESSAGE_NONCONST
typedef struct pam_message pam_message_type;
#else
typedef const struct pam_message pam_message_type;
#endif

static int PAM_conv (int num_msg,
		     pam_message_type **msg,
		     struct pam_response **resp,
		     void *appdata_ptr)
{
    int count;
    struct pam_response *reply;

    if (!(reply = calloc(num_msg, sizeof(struct pam_response))))
	return PAM_CONV_ERR;

#define COPY_STRING(s) (s) ? strdup(s) : NULL
    for (count = 0; count < num_msg; count++) {
	switch (msg[count]->msg_style) {
	case PAM_TEXT_INFO:
	case PAM_ERROR_MSG:
	    Debug("PAM: %s\n", msg[count]->msg);
	    /* ignore the mesage */
	    break;
	case PAM_PROMPT_ECHO_OFF:
	    /* wants password */
	    reply[count].resp_retcode = PAM_SUCCESS;
	    reply[count].resp = COPY_STRING(PAM_password);
	    break;
	case PAM_PROMPT_ECHO_ON:
	    /* user name given to PAM already */
	    /* fall through */
	default:
	    /* unknown */
	    goto conv_fail;
	}
    }
#undef COPY_STRING

    *resp = reply;
    return PAM_SUCCESS;

  conv_fail:
    for (count = 0; count < num_msg; ++count) {
	if (reply[count].resp == NULL)
	    continue;
	switch (msg[count]->msg_style) {
	case PAM_PROMPT_ECHO_ON:
	case PAM_PROMPT_ECHO_OFF:
	    bzero(reply[count].resp, strlen(reply[count].resp));
	    free(reply[count].resp);
	    break;
	case PAM_ERROR_MSG:
	case PAM_TEXT_INFO:
	    /* should not actually be able to get here... */
	    free(reply[count].resp);
	}                                            
	reply[count].resp = NULL;
    }
    /* forget reply too */
    free (reply);
    return PAM_CONV_ERR;
}

static struct pam_conv PAM_conversation = {
	&PAM_conv,
	NULL
};

# ifdef HAVE_PAM_FAIL_DELAY
void fail_delay(int retval, unsigned usec_delay, void *appdata_ptr) {}
# endif

pam_handle_t **pamh;

#else /* USE_PAM */

# ifdef KRB4
int krb4_authed = 0;

int
krb4_auth(struct passwd *p, const char *password)
{
    int ret;
    char realm[REALM_SZ];

    if (krb4_authed)
	return 1;

    if (!p->pw_uid)
	return 0;

    if (krb_get_lrealm(realm, 1)) {
	Debug ("Can't get Kerberos realm.\n");
	return 0;
    }

    sprintf(krbtkfile, "%s.%s", TKT_ROOT, d->name);
    krb_set_tkt_string(krbtkfile);
    unlink(krbtkfile);

    ret = krb_verify_user(p->pw_name, "", realm, password, 1, "rcmd");
    if (ret == KSUCCESS) {
	chown(krbtkfile, p->pw_uid, p->pw_gid);
	Debug("kerberos verify succeeded\n");
	return 1;
    } else if(ret != KDC_PR_UNKNOWN && ret != SKDC_CANT) {
	/* failure */
	Debug("kerberos verify failure %d\n", ret);
	LogError("KRB4 verification failure '%s' for %s\n", 
		 krb_get_err_text(ret), p->pw_name);
	krbtkfile[0] = '\0';
    }
    return 0;
}
# endif /* KRB4 */

#endif /* USE_PAM */

int		pwinited;
struct passwd	*p;
#if !defined(USE_PAM) && defined(USESHADOW)
struct spwd	*sp;
char		*user_pass;
# define arg_shadow , int shadow
# define NSHADOW , 0
# define YSHADOW , 1
#else
# define arg_shadow
# define NSHADOW
# define YSHADOW
#endif

int
init_pwd(const char *name arg_shadow)
{
#if !defined(USE_PAM) && defined(USESHADOW)
    if (pwinited == 2) {
	Debug("p & sp for %s already read\n", name);
	return 1;
    }
#endif

    if (!pwinited) {
	Debug("reading p for %s\n", name);
	p = getpwnam (name);
	endpwent();
	if (!p) {
	    Debug ("getpwnam() failed.\n");
	    return 0;
	}
#ifndef USE_PAM
# ifdef linux	/* only Linux? */
	if (p->pw_passwd[0] == '!' || p->pw_passwd[0] == '*') {
	    Debug ("The account is locked, no login allowed.\n");
	    return 0;
	}
# endif
# ifdef USESHADOW
	user_pass = p->pw_passwd;
# endif
#endif	/* !USE_PAM */
	pwinited = 1;
    }

#if defined(USE_PAM) || !defined(USESHADOW)
# define user_pass p->pw_passwd
#else
    if (shadow) {
	Debug("reading sp for %s\n", name);
	errno = 0;
	sp = getspnam(name);
# ifndef QNX4
	endspent();
# endif  /* QNX4 doesn't need endspent() to end shadow passwd ops */
	if (sp) {
	    user_pass = sp->sp_pwdp;
	    pwinited = 2;
	} else
	    Debug ("getspnam() failed, errno=%d.  Are you root?\n", errno);
    }
#endif

    return 1;
}

extern struct display          *d;
extern struct verify_info      *verify;
extern struct greet_info       *greet;


int
re_str(char **dst, const char *src)
{
    int len = strlen(src) + 1;
    char *bk;

    if (!(bk = malloc (len)))
	return 0;
    memcpy(bk, src, len);
    if (*dst)
	free(*dst);
    *dst = bk;
    return 1;
}


#if !defined(USE_PAM) && defined(AIXV3)
char tty[16], hostname[100];
#endif

VerifyRet
init_vrf(const char *name, const char *password)
{
#if !defined(USE_PAM) && defined(AIXV3)
    int		i;
    char	*tmpch;
#endif
    int		pretc;
    static char	*pname;

    if (!strlen (name)) {
	Debug ("Empty user name provided.\n");
	return V_FAIL;
    }

    if (pname && strcmp(pname, name)) {
	Debug("Re-initializing for new user %s\n", name);
#if !defined(USE_PAM) && !defined(AIXV3) && defined(KRB4)
	krb4_authed = 0;
	krbtkfile[0] = '\0';
#endif
	pwinited = 0;
    }
    re_str (&pname, name);

    greet->name = (char *)name;
    PAM_password = greet->password = (char *)password;

#ifdef USE_PAM

    pamh = thepamh();
    if (!*pamh) {
	Debug("opening new PAM handle\n");
	if (pam_start(KDE_PAM, name, &PAM_conversation, pamh) != PAM_SUCCESS) {
	    Debug("pam_start() failed\n");
	    return V_FAIL;
	}
	if ((pretc = pam_set_item(*pamh, PAM_TTY, d->name)) != PAM_SUCCESS) {
	    pam_end(*pamh, pretc);
	    *pamh = NULL;
	    return V_FAIL;
	}
# ifdef HAVE_PAM_FAIL_DELAY
	pam_set_item(*pamh, PAM_FAIL_DELAY, fail_delay);
# endif
    } else
	if (pam_set_item(*pamh, PAM_USER, name) != PAM_SUCCESS)
	    return V_FAIL;

#elif defined(AIXV3)

    if (d->displayType.location == Foreign) {
	strncpy(hostname, d->name, sizeof(hostname) - 1);
	hostname[sizeof(hostname)-1] = '\0';
	if ((tmpch = strchr(hostname, ':')))
	    *tmpch = '\0';
    } else
	hostname[0] = '\0';

    /* tty names should only be 15 characters long */
# if 1
    for (i = 0; i < 15 && d->name[i]; i++) {
	if (d->name[i] == ':' || d->name[i] == '.')
	    tty[i] = '_';
	else
	    tty[i] = d->name[i];
    }
    tty[i] = '\0';
# else
    memcpy(tty, "/dev/xdm/", 9);
    for (i = 0; i < 6 && d->name[i]; i++) {
	if (d->name[i] == ':' || d->name[i] == '.')
	    tty[9 + i] = '_';
	else
	    tty[9 + i] = d->name[i];
    }
    tty[9 + i] = '\0';
# endif

#endif

    return V_OK;
}

VerifyRet
Verify (const char *name, const char *password)
{
    int		pretc;
#if !defined(USE_PAM) && defined(AIXV3)
    int		reenter;
    char	*msg;
#endif

    Debug ("Verify %s ...\n", name);

    if ((pretc = init_vrf(name, password)) != V_OK)
	return pretc;

#ifdef USE_PAM

    if (pam_authenticate(*pamh, d->allowNullPasswd ? 
				0 : PAM_DISALLOW_NULL_AUTHTOK) != PAM_SUCCESS)
	return V_FAIL;

#elif defined(AIXV3) /* USE_PAM */

    enduserdb();
    msg = NULL;
    if (authenticate(name, password, &reenter, &msg) || reenter) {
	Debug("authenticate() - %s\n", msg ? msg : "Error\n");
	if (msg)
	    free((void *)msg);
	loginfailed(name, hostname, tty);
	return V_FAIL;
    }
    if (msg)
	free((void *)msg);

#else	/* USE_PAM && AIXV3 */

    if (!init_pwd(name YSHADOW))
	return V_FAIL;

# ifdef KRB4
    if (!krb4_auth(p, password)) {
# endif  /* KRB4 */

# if defined(ultrix) || defined(__ultrix__)
	if (authenticate_user(p, password, NULL) < 0)
# else
	if (strcmp (crypt (password, user_pass), user_pass))
# endif
	{
	    if(!d->allowNullPasswd || p->pw_passwd[0]) {
		Debug ("password verify failed\n");
# ifdef USESHADOW
		bzero(user_pass, strlen(user_pass));
# endif
		return V_FAIL;
	    } /* else: null passwd okay */
	}
# ifdef KRB4
    }
# endif  /* KRB4 */

# ifdef USESHADOW
    bzero(user_pass, strlen(user_pass));
# endif

#endif /* USE_PAM && AIXV3 */

    Debug ("verify succeeded\n");

    return V_OK;
}

VerifyRet
Restrict(const char *name, int *expire, char **nologin)
{
    int			pretc;
#ifndef USE_PAM
# ifdef AIXV3
    char		*msg
# else /* AIXV3 */
    struct stat		st;
    char		*nolg;
#  ifdef HAVE_GETUSERSHELL
    char		*s;
#  endif
#  if defined(HAVE_PW_EXPIRE) || defined(USESHADOW)
    int			tim, exp, warntime;
    int			quietlog;
#  endif
#  ifdef USE_LOGIN_CAP
#   ifdef HAVE_LOGIN_GETCLASS
    login_cap_t		*lc;
#   else
    struct login_cap	*lc;
#   endif
#  endif
# endif /* AIXV3 */
#endif

    Debug("Restrict %s ...\n", name);

    if ((pretc = init_vrf(name, 0)) != V_OK)
	return pretc;

    if (!init_pwd(name YSHADOW))
	return V_FAIL;

    if (!p->pw_uid) {
	if (!d->allowRootLogin)
	    return V_NOROOT;
	else
	    return V_OK;	/* don't deny root to log in */
    }

#ifdef USE_PAM

    if (pam_acct_mgmt(*pamh, 0) != PAM_SUCCESS)
	return V_FAIL;	/* XXX: V_AUTHMSG */
    /* really should do password changing, but it doesn't fit well */

#elif defined(AIXV3)	/* USE_PAM */

    msg = NULL;
    if (loginrestrictions(name, 
		d->displayType.location == Foreign ? S_RLOGIN : S_LOGIN, 
		tty, &msg) == -1)
    {
	Debug("loginrestrictions() - %s\n", msg ? msg : "Error\n");
	if (msg)
	    free((void *)msg);
	loginfailed(name, hostname, tty);
	return V_FAIL;	/* XXX: V_AUTHMSG */
    }
    if (msg)
	free((void *)msg);

#else	/* USE_PAM || AIXV3 */

# ifdef HAVE_GETUSERSHELL
    for (;;) {
	s = getusershell();
	if (s == NULL) {
	    /* did not find the shell in /etc/shells  -> failure */
	    Debug("shell not in /etc/shells\n");
	    endusershell();
	    return V_BADSHELL;
	}
	if (strcmp(s, p->pw_shell) == 0) {
	    /* found the shell in /etc/shells */
	    endusershell();
	    break;
	}
    } 
# endif

# ifdef USE_LOGIN_CAP
#  ifdef HAVE_LOGIN_GETCLASS
    lc = login_getclass(p->pw_class);
#  else
    lc = login_getpwclass(p);
#  endif
    if (!lc)
	return V_ERROR;
# endif


/* restrict_nologin */
# ifndef _PATH_NOLOGIN
#  define _PATH_NOLOGIN "/etc/nologin"
# endif

# ifdef USE_LOGIN_CAP
    /* Do we ignore a nologin file? */
    if (login_getcapbool(lc, "ignorenologin", 0))
	goto nolog_succ;

    nolg = login_getcapstr(lc, "nologin", "", NULL);
    if (!stat(nolg, &st))
	goto nolog;
# endif

    nolg = _PATH_NOLOGIN;
    if (!stat(nolg, &st)) {
      nolog:
	if (nologin) {
	    *nologin = nolg;
# ifdef USE_LOGIN_CAP
	    login_close(lc);
# endif
	    return V_NOLOGIN;
	}
    }
# ifdef USE_LOGIN_CAP
nolog_succ:
# endif


/* restrict_nohome */
# ifdef USE_LOGIN_CAP

    if (login_getcapbool(lc, "requirehome", 0)) {
	seteuid(p->pw_uid);
	if (!*p->pw_dir || chdir(p->pw_dir) < 0) {
	    login_close(lc);
	    return V_NOHOME;
	}
	seteuid(0);
    }

# endif


/* restrict_expired */
# if defined(HAVE_PW_EXPIRE) || defined(USESHADOW) /* && !defined(USE_PAM) ? */

#  if !defined(HAVE_PW_EXPIRE) || (!defined(USE_LOGIN_CAP) && defined(USESHADOW))
    if (!sp)
	goto spbad;
#  endif

#  define DEFAULT_WARN  (2L * 7L)  /* Two weeks */

    tim = time(NULL) / 86400L;

#  ifdef USE_LOGIN_CAP
    quietlog = login_getcapbool(lc, "hushlogin", 0);
    warntime = login_getcaptime(lc, "warnexpire",
				DEFAULT_WARN * 86400L, DEFAULT_WARN * 86400L
			       ) / 86400L;
#  else
    quietlog = 0;
#   ifdef USESHADOW
    warntime = sp->sp_warn != -1 ? sp->sp_warn : DEFAULT_WARN;
#   else
    warntime = DEFAULT_WARN;
#   endif
#  endif

#  ifdef HAVE_PW_EXPIRE
    exp = p->pw_expire / 86400L;
    if (exp) {
#  else
    if (sp->sp_expire != -1) {
	exp = sp->sp_expire;
#  endif
	if (tim > exp) {
#  ifdef USE_LOGIN_CAP
	    login_close(lc);
#  endif
	    return V_AEXPIRED;
	} else if (tim > (exp - warntime) && !quietlog) {
	    if (expire)
		*expire = exp - tim;
	    retv = V_AWEXPIRE;
	}
    }

#  ifdef HAVE_PW_EXPIRE
    exp = p->pw_change / 86400L;
    if (exp) {
#  else
    if (sp->sp_max != -1) {
	exp = sp->sp_lstchg + sp->sp_max;
#  endif
	if (tim > exp) {
#  ifdef USE_LOGIN_CAP
	    login_close(lc);
#  endif
	    return V_PEXPIRED;
	} else if (tim > (exp - warntime) && !quietlog) {
	    if (expire && (retv == V_OK || *expire > exp))
		*expire = exp - tim;
	    retv = V_PWEXPIRE;
	}
    }

#  if !defined(HAVE_PW_EXPIRE) || (!defined(USE_LOGIN_CAP) && defined(USESHADOW))
spbad:
#  endif

# endif /* HAVE_PW_EXPIRE || USESHADOW */


/* restrict_time */
# ifdef USE_LOGIN_CAP
#  ifdef HAVE_AUTH_TIMEOK
    if (!auth_timeok(lc, time(NULL))) {
	login_close(lc);
	return V_BADTIME;
    }
#  endif


    login_close(lc);
# endif

#endif /* USE_PAM || AIXV3 */

    return V_OK;
}


static char *envvars[] = {
    "TZ",			/* SYSV and SVR4, but never hurts */
#ifdef AIXV3
    "AUTHSTATE",		/* for kerberos */
#endif
#if defined(sony) && !defined(SYSTYPE_SYSV) && !defined(_SYSTYPE_SYSV)
    "bootdev",
    "boothowto",
    "cputype",
    "ioptype",
    "machine",
    "model",
    "CONSDEVTYPE",
    "SYS_LANGUAGE",
    "SYS_CODE",
#endif
#if (defined(SVR4) || defined(SYSV)) && defined(i386) && !defined(sun)
    "XLOCAL",
#endif
    NULL
};

static char **
userEnv (struct display *d, int useSystemPath, char *user, char *home, char *shell)
{
    char	**env;
    char	**envvar;
    char	*str;

    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    env = setEnv (env, "HOME", home);
    env = setEnv (env, "LOGNAME", user); /* POSIX, System V */
    env = setEnv (env, "USER", user);    /* BSD */
    env = setEnv (env, "PATH", useSystemPath ? d->systemPath : d->userPath);
    env = setEnv (env, "SHELL", shell);
#if !defined(USE_PAM) && !defined(AIXV3) && defined(KRB4)
    if (krbtkfile[0] != '\0')
        env = setEnv (env, "KRBTKFILE", krbtkfile);
#endif
    for (envvar = envvars; *envvar; envvar++)
    {
	str = getenv(*envvar);
	if (str)
	    env = setEnv (env, *envvar, str);
    }
    return env;
}

/* XXX merge with StartClient! */
VerifyRet
PrepSession(const char *name, const char *password)
{
    int		pretc;
    char	*shell, *home;
    char	**argv;
#ifndef USE_PAM
# ifdef AIXV3
    char	*msg;
# else
    int 	i;
# endif
#endif

    if ((pretc = init_vrf(name, password)) != V_OK)
	return pretc;

    if (!init_pwd(name NSHADOW))
	return V_FAIL;

#ifndef USE_PAM
# ifdef AIXV3
    msg = NULL;
    loginsuccess(greet->name, hostname, tty, &msg);
    if (msg) {
	Debug("loginsuccess() - %s\n", msg);
	free((void *)msg);
    }
# else /* AIXV3 */
#  if defined(KRB4) && defined(AFS)
    if (password && krb4_auth(p, password)) {
	if (k_hasafs()) {
	    if (k_setpag() == -1)
		LogError ("setpag() failed for %s\n", name);
	    /*  XXX maybe, use k_afsklog_uid(NULL, NULL, p->pw_uid)? */
	    if ((ret = k_afsklog(NULL, NULL)) != KSUCCESS)
		LogError("Warning %s\n", krb_get_err_text(ret));
	}
    }
#  endif /* KRB4 && AFS */
# endif /* AIXV3 */
#endif	/* !PAM */

    verify->uid = p->pw_uid;
    verify->gid = p->pw_gid;
    home = p->pw_dir;
    shell = p->pw_shell;
    argv = 0;
    if (d->session)
	argv = parseArgs (argv, d->session);
    if (greet->string)
	argv = parseArgs (argv, greet->string);
    if (!argv)
	argv = parseArgs (argv, "xsession");
    verify->argv = argv;
    verify->userEnviron = userEnv (d, p->pw_uid == 0,
				   greet->name, home, shell);
    verify->systemEnviron = systemEnv (d, greet->name, home);
    Debug ("user environment:\n");
    printEnv (verify->userEnviron);
    Debug ("system environment:\n");
    printEnv (verify->systemEnviron);
    Debug ("end of environments\n");

    /*
     * for user-based authorization schemes,
     * add the user to the server's allowed "hosts" list.
     */
#if !defined(USE_PAM) && !defined(AIXV3)
    for (i = 0; i < d->authNum; i++)
    {
# ifdef SECURE_RPC
	if (d->authorizations[i]->name_length == 9 &&
	    memcmp(d->authorizations[i]->name, "SUN-DES-1", 9) == 0)
	{
	    XHostAddress	addr;
	    char		netname[MAXNETNAMELEN+1];
	    char		domainname[MAXNETNAMELEN+1];
    
	    getdomainname(domainname, sizeof domainname);
	    user2netname (netname, verify->uid, domainname);
	    addr.family = FamilyNetname;
	    addr.length = strlen (netname);
	    addr.address = netname;
	    XAddHost (*dpy, &addr);
	}
# endif
# ifdef K5AUTH
	if (d->authorizations[i]->name_length == 14 &&
	    memcmp(d->authorizations[i]->name, "MIT-KERBEROS-5", 14) == 0)
	{
	    /* Update server's auth file with user-specific info.
	     * Don't need to AddHost because X server will do that
	     * automatically when it reads the cache we are about
	     * to point it at.
	     */
	    extern Xauth *Krb5GetAuthFor();

	    XauDisposeAuth (d->authorizations[i]);
	    d->authorizations[i] =
		Krb5GetAuthFor(14, "MIT-KERBEROS-5", d->name);
	    SaveServerAuthorizations (d, d->authorizations, d->authNum);
	}
# endif
    }
#endif /* USE_PAM && AIXV3 */

    return V_OK;
}

