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

# include	"dm.h"
# include	"dm_error.h"

#include	<pwd.h>
#ifdef USE_PAM
# include	<security/pam_appl.h>
# ifdef KDE_PAM_SERVICE
#  define KDE_PAM KDE_PAM_SERVICE
# else  
#  define KDE_PAM "xdm"  /* default PAM service called by kdm */
# endif 
#else
# ifdef USESHADOW
#  include	<shadow.h>
#  include	<errno.h>
#  ifdef X_NOT_STDC_ENV
extern int errno;
#  endif
# endif
# ifdef HAVE_CRYPT_H
#  include <crypt.h>
# endif
#endif

#ifdef KRB4
# include <sys/param.h>
/* Some systems define an des_encrypt() in their crypt.h (where it
   doesn't belong!), which conflicts with the one from KTH Kerberos.  As we
   are not using it anyway, we temporarily redefine it.  */
# define des_encrypt des_encrypt_faked_XXX
/* # include <kerberosIV/krb.h> */
# include <krb.h>
# ifdef AFS
#  include <kafs.h>
/* #  include <kerberosIV/kafs.h> */
# endif
# undef des_encrypt
static char krbtkfile[MAXPATHLEN];
#endif

#if defined(HAVE_LOGIN_CAP_H) && !defined(__NetBSD__)
# define USE_LOGIN_CAP 1
# include <login_cap.h>
#endif

/*
#ifdef _AIX
# include <login.h>
# include <usersec.h>
extern int loginrestrictions (char *Name, int Mode, char *Tty, char **Msg);
extern int loginfailed (char *User, char *Host, char *Tty);
extern int loginsuccess (char *User, char *Host, char *Tty, char **Msg);
#endif / * _AIX */

/* for nologin */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
/* for expiration */
#include <time.h>

#ifdef X_NOT_STDC_ENV
char *getenv();
#endif

#ifdef QNX4
extern char *crypt(const char *, const char *);
#endif

# include	"greet.h"

static char *envvars[] = {
    "TZ",			/* SYSV and SVR4, but never hurts */
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
#ifdef KRB4
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
	int count = 0;
	struct pam_response *reply = NULL;

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

#endif /* USE_PAM */

#define UFAILV do { bzero(greet->password, strlen(greet->password)); return V_FAIL; } while(0)
#define FAILVV(rv) do { if (greet->password) bzero(greet->password, strlen(greet->password)); return rv; } while(0)
#define FAILV FAILVV(V_FAIL)

VerifyRet
Verify (struct display *d, struct greet_info *greet, struct verify_info *verify, 
	time_t *expire, char **nologin)
{
    struct passwd	*p;
#ifdef USE_PAM
    pam_handle_t **pamh = thepamh();
    int			pretc;
#else
# ifdef USESHADOW
    struct spwd		*sp;
# endif
    struct stat		st;
    char		*nolg;
#endif
#ifdef HAVE_GETUSERSHELL
    char		*s;
#endif
#if defined(HAVE_PW_EXPIRE) || defined(USESHADOW)
    time_t		tim, exp, warntime;
    int			quietlog;
#endif
#ifdef USE_LOGIN_CAP
# ifdef __bsdi__
    /* This only works / is needed on BSDi */
    login_cap_t		*lc;
# else
    struct login_cap	*lc;
# endif
#endif
    char		*user_pass = NULL;
    char		*shell, *home;
    char		**argv;
    int			retv = V_OK;

    Debug ("Verify %s ...\n", greet->name);
    if (!strlen (greet->name)) {
	Debug ("Empty user name provided.\n");
	FAILV;
    }

#ifdef USE_PAM

# define PAM_DO(todo)	\
    if ((pretc = todo) != PAM_SUCCESS) { \
	pam_end(*pamh, pretc); \
	FAILV; \
    }
    PAM_password = greet->password;
    if (pam_start(KDE_PAM, greet->name, &PAM_conversation, pamh) != PAM_SUCCESS)
	FAILV;
    PAM_DO(pam_set_item(*pamh, PAM_TTY, d->name));
    if (greet->password) {
# ifdef HAVE_PAM_FAIL_DELAY
	PAM_DO(pam_set_item(*pamh, PAM_FAIL_DELAY, fail_delay));
# endif
	PAM_DO(pam_authenticate(*pamh, d->allowNullPasswd ? 
				0 : PAM_DISALLOW_NULL_AUTHTOK));
    }
    PAM_DO(pam_acct_mgmt(*pamh, 0));
    /* really should do password changing, but it doesn't fit well */
# undef PAM_DO

/*
#elif defined(_AIX) / * USE_PAM * /

    int		i, loginType;
    char	tty[16], hostname[100], *msg;

    if (d->displayType.location == Foreign) {
	char *tmpch;
	strncpy(hostname, d->name, sizeof(hostname)-1);
	hostname[sizeof(hostname)-1] = '\0';
	if ((tmpch = strchr(hostname, ':')))
	    *tmpch = '\0';
    } else
	hostname[0] = '\0';

    / * tty names should only be 15 characters long * /
    memcpy(tty, "/dev/xdm/", 10);
    for (i = 0; i < 6 && d->name[i]; i++)
	if (d->name[i] == ':' || d->name[i] == '.')
	    tty[9 + i] = '_';
	else
	    tty[9 + i] = d->name[i];
    }
    tty[9 + i] = '\0';

    loginType = d->displayType.location == Foreign ? S_RLOGIN : S_LOGIN;

    if (greet->password) {
	enduserdb();
	msg = NULL;
	rc = authenticate(greet->name, greet->password, &reenter, &msg);
	if (rc || reenter) {
	    Debug("authenticate() - %s\n", msg ? msg : "Error\n");
	    if (msg)
		free((void *)msg);
	    loginfailed(greet->name, hostname, tty);
	    UFAILV;
	}
	if (msg)
	    free((void *)msg);
    }
    msg = NULL;
    if (!loginrestrictions(greet->name, loginType, tty, &msg)) {
	Debug("loginrestrictions() - %s\n", msg ? msg : "Error\n");
	if (msg)
	    free((void *)msg);
	/ * loginfailed(greet->name, hostname, tty);  here too? * /
	FAILV;	/ * XXX: V_AUTHMSG * /
    }
    if (msg)
	free((void *)msg);
*/
#endif	/* USE_PAM && _AIX */

    p = getpwnam (greet->name);
    endpwent();
    if (!p) {
	Debug ("getpwnam() failed.\n");
	FAILV;
    }
    user_pass = p->pw_passwd;

#ifdef USESHADOW
    errno = 0;
    sp = getspnam(greet->name);
    if (!sp)
	Debug ("getspnam() failed, errno=%d.  Are you root?\n", errno);
    else
	user_pass = sp->sp_pwdp;
# ifndef QNX4
    endspent();
# endif  /* QNX4 doesn't need endspent() to end shadow passwd ops */
#endif /* USESHADOW */

#if !defined(USE_PAM) /* && !defined(_AIX) */

# ifdef __linux__	/* only Linux? */
    if (p->pw_passwd[0] == '!' || p->pw_passwd[0] == '*') {
	Debug ("The account is locked, no login allowed.\n");
	FAILV;
    }
# endif

    if (greet->password) {

# ifdef KRB4
	if (strcmp(greet->name, "root") != 0) {
	    char name[ANAME_SZ];
	    char realm[REALM_SZ];
	    char *q;
	    int ret;
	    
	    if (krb_get_lrealm(realm, 1))
		Debug ("Can't get Kerberos realm.\n");
	    else {

		sprintf(krbtkfile, "%s.%s", TKT_ROOT, d->name);
		krb_set_tkt_string(krbtkfile);
		unlink(krbtkfile);

		ret = krb_verify_user(greet->name, "", realm, 
				      greet->password, 1, "rcmd");
           
		if (ret == KSUCCESS) {
		    chown(krbtkfile, p->pw_uid, p->pw_gid);
		    Debug("kerberos verify succeeded\n");
#  ifdef AFS
		    if (k_hasafs()) {
			if (k_setpag() == -1)
			    LogError ("setpag() failed for %s\n", greet->name);

			if ((ret = k_afsklog(NULL, NULL)) != KSUCCESS)
			    LogError("Warning %s\n", krb_get_err_text(ret));
		    }
#  endif
		    goto done;
		} else if(ret != KDC_PR_UNKNOWN && ret != SKDC_CANT) {
		    /* failure */
		    Debug("kerberos verify failure %d\n", ret);
		    LogError("KRB4 verification failure %s for %s'\n", 
			     krb_get_err_text(ret), greet->name);
		    krbtkfile[0] = '\0';
		}
	    }
	}
# endif  /* KRB4 */

# if defined(ultrix) || defined(__ultrix__)
	if (authenticate_user(p, greet->password, NULL) < 0)
# else
	if (strcmp (crypt (greet->password, user_pass), user_pass))
# endif
	{
	    if(!d->allowNullPasswd || p->pw_passwd[0]) {
		Debug ("password verify failed\n");
		UFAILV;
	    } /* else: null passwd okay */
	}
    }	/* greet->password */
done:
#endif /* USE_PAM && _AIX */
    bzero(user_pass, strlen(user_pass)); /* in case shadow password */

    Debug("Restrict ...\n");

    if (!p->pw_uid) {
	if (!d->allowRootLogin)
	    FAILVV(V_NOROOT);
	else
	    goto norestr;	/* don't deny root to log in */
    }

#ifdef HAVE_GETUSERSHELL
    for (;;) {
	s = getusershell();
	if (s == NULL) {
	    /* did not find the shell in /etc/shells  -> failure */
	    Debug("shell not in /etc/shells\n");
	    endusershell();
	    FAILVV(V_BADSHELL);
	}
	if (strcmp(s, p->pw_shell) == 0) {
	    /* found the shell in /etc/shells */
	    endusershell();
	    break;
	}
    } 
#endif

#ifdef USE_LOGIN_CAP
# ifdef __bsdi__
    /* This only works / is needed on BSDi */
    lc = login_getclass(p->pw_class);
# else
    lc = login_getpwclass(p);
# endif
    if (!lc)
	FAILVV(V_ERROR);
#endif


/* restrict_nologin */
#ifndef USE_PAM

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
	    FAILVV(V_NOLOGIN);
	}
    }
# ifdef USE_LOGIN_CAP
nolog_succ:
# endif
#endif /* !USE_PAM */


/* restrict_nohome */
#ifdef USE_LOGIN_CAP

    if (login_getcapbool(lc, "requirehome", 0)) {
	seteuid(p->pw_uid);
	if (!*p->pw_dir || chdir(p->pw_dir) < 0) {
	    login_close(lc);
	    FAILVV(V_NOHOME);
	}
	seteuid(0);
    }

#endif


/* restrict_expired */
#if defined(HAVE_PW_EXPIRE) || defined(USESHADOW) /* && !defined(USE_PAM) ? */

# define DEFAULT_WARN  (2L * 7L * 86400L)  /* Two weeks */

    tim = time(NULL);

# ifdef USE_LOGIN_CAP
    quietlog = login_getcapbool(lc, "hushlogin", 0);
    warntime = login_getcaptime(lc, "warnexpire",
				DEFAULT_WARN, DEFAULT_WARN);
# else
    quietlog = 0;
#  ifdef USESHADOW
    warntime = sp->sp_warn*86400;
#  else
    warntime = DEFAULT_WARN;
#  endif
# endif

# ifdef HAVE_PW_EXPIRE
    exp = p->pw_expire;
    if (exp) {
# else
    if (sp->sp_expire != -1) {
	exp = sp->sp_expire*86400;
# endif
	if (exp <= tim) {
# ifdef USE_LOGIN_CAP
	    login_close(lc);
# endif
	    FAILVV(V_AEXPIRED);
	} else if (exp - tim < warntime && !quietlog) {
	    if (expire)
		*expire = exp;
	    retv = V_AWEXPIRE;
	}
    }

# ifdef HAVE_PW_EXPIRE
    exp = p->pw_change;
    if (exp) {
# else
    if (sp->sp_max != -1) {
	exp = (sp->sp_lstchg+sp->sp_max)*86400;
# endif
	if (exp <= tim) {
# ifdef USE_LOGIN_CAP
	    login_close(lc);
# endif
	    FAILVV(V_PEXPIRED);
	} else if (exp - tim < warntime && !quietlog) {
	    if (expire && (retv == V_OK || *expire > exp))
		*expire = exp;
	    retv = V_PWEXPIRE;
	}
    }

#endif /* HAVE_PW_EXPIRE || USESHADOW */


/* restrict_time */
#ifdef USE_LOGIN_CAP
    if (!auth_timeok(lc, time(NULL))) {
	login_close(lc);
	FAILVV(V_BADTIME);
    }


    login_close(lc);
#endif

norestr:
    Debug ("verify succeeded\n");
/*
#ifdef _AIX
    msg = NULL;
    loginsuccess(greet->name, hostname, tty, &msg);
    if (msg) {
	Debug("loginsuccess() - %s\n", msg);
	free((void *)msg);
    }
#endif
*/
    /* The password is passed to StartClient() for use by user-based
       authorization schemes.  It is zeroed there. */
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
    return retv;
}


VerifyRet
VerifyRoot( const char *pw)
{
#ifndef USE_PAM
    struct passwd *pws = getpwnam( "root");
    if (!pws) {
	printf("can't verify root password, getpwnam() failed\n");
	return V_FAIL;
    }
    endpwent();
#ifdef USESHADOW
    /* If USESHADOW is defined, kdm will work for both shadow and non
     * shadow systems
     */
    {
	struct spwd *spws = getspnam( "root");
	if( spws != NULL)
	    pws->pw_passwd = spws->sp_pwdp;
	endspent();
    }
#endif /* USESHADOW */
#ifdef HAVE_CRYPT
    if( strcmp( crypt( pw, pws->pw_passwd), pws->pw_passwd)) {
	printf("Root passwd verification failed\n");
	return V_FAIL;
    }
#else
    printf("can't verify root password, lacking crypt() support\n");
    return V_FAIL;
#endif
#else /* USE_PAM */
    pam_handle_t *pamh;
    int pretc;
    PAM_password = (char *)pw;
    if (pam_start(KDE_PAM, "root", &PAM_conversation, &pamh) != PAM_SUCCESS)
	return V_FAIL;
    if ((pretc = pam_authenticate( pamh, 0)) != PAM_SUCCESS) {
	pam_end(pamh, pretc);
	return V_FAIL;
    }
    /* OK, if we get here, the user _should_ be root */
    pam_end( pamh, PAM_SUCCESS);
#endif /* USE_PAM */
    return V_OK;
}

