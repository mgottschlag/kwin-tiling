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
#ifdef NGROUPS_MAX
# include	<grp.h>
#endif
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

# include	"greet.h"

#if defined(HAVE_LOGIN_CAP_H) && !defined(__NetBSD__)
# define USE_LOGIN_CAP 1
# include <login_cap.h>
#endif

#ifdef _AIX
# include <login.h>
# include <usersec.h>
extern int loginrestrictions (char *Name, int Mode, char *Tty, char **Msg);
extern int loginfailed (char *User, char *Host, char *Tty);
extern int loginsuccess (char *User, char *Host, char *Tty, char **Msg);
#endif /* _AIX */

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

#ifdef KRB4
# include <sys/param.h>
/* # include <kerberosIV/krb.h> */
# include <krb.h>
# ifdef AFS
#  include <kafs.h>
/* #  include <kerberosIV/kafs.h> */
# endif
static char krbtkfile[MAXPATHLEN];
#endif

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

#ifdef NGROUPS_MAX
static int
groupMember (name, members)
    char *name;
    char **members;
{
	while (*members) {
		if (!strcmp (name, *members))
			return 1;
		++members;
	}
	return 0;
}

static void
getGroups (name, verify, gid)
    char		*name;
    struct verify_info	*verify;
    int			gid;
{
	int		ngroups;
	struct group	*g;
	int		i;

	ngroups = 0;
	verify->groups[ngroups++] = gid;
	setgrent ();
	/* SUPPRESS 560 */
	while ( (g = getgrent())) {
		/*
		 * make the list unique
		 */
		for (i = 0; i < ngroups; i++)
			if (verify->groups[i] == g->gr_gid)
				break;
		if (i != ngroups)
			continue;
		if (groupMember (name, g->gr_mem)) {
			if (ngroups >= NGROUPS_MAX)
				LogError ("%s belongs to more than %d groups, %s ignored\n",
					name, NGROUPS_MAX, g->gr_name);
			else
				verify->groups[ngroups++] = g->gr_gid;
		}
	}
	verify->ngroups = ngroups;
	endgrent ();
}
#endif /* NGROUPS_MAX */

#ifdef USE_PAM
static char *PAM_password;
static int pam_error;

#ifdef PAM_MESSAGE_NONCONST
typedef struct pam_message pam_message_type;
#else
typedef const struct pam_message pam_message_type;
#endif

static int PAM_conv (int num_msg,
		     pam_message_type **msg,
		     struct pam_response **resp,
		     void *appdata_ptr) {
	int replies = 0;
	struct pam_response *reply = NULL;

	reply = malloc(sizeof(struct pam_response));
	if (!reply) return PAM_CONV_ERR;
#define COPY_STRING(s) (s) ? strdup(s) : NULL

	for (replies = 0; replies < num_msg; replies++) {
		switch (msg[replies]->msg_style) {
		case PAM_PROMPT_ECHO_OFF:
			/* wants password */
			reply[replies].resp_retcode = PAM_SUCCESS;
			reply[replies].resp = COPY_STRING(PAM_password);
			break;
		case PAM_TEXT_INFO:
			/* ignore the informational mesage */
			break;
		case PAM_PROMPT_ECHO_ON:
			/* user name given to PAM already */
			/* fall through */
		default:
			/* unknown or PAM_ERROR_MSG */
			free (reply);
			return PAM_CONV_ERR;
		}
	}

#undef COPY_STRING
	*resp = reply;
	return PAM_SUCCESS;
}

static struct pam_conv PAM_conversation = {
	&PAM_conv,
	NULL
};
#endif /* USE_PAM */

#define UFAILV do { bzero(greet->password, strlen(greet->password)); return V_FAIL; } while(0)
#define FAILV do { if (greet->password) bzero(greet->password, strlen(greet->password)); return V_FAIL; } while(0)
#define FAILVV(rv) do { if (greet->password) bzero(greet->password, strlen(greet->password)); return rv; } while(0)

VerifyRet
Verify (struct display *d, struct greet_info *greet, struct verify_info *verify, 
	time_t *expire, char **nologin)
{
    struct passwd	*p;
#ifdef USE_PAM
    pam_handle_t **pamh = thepamh();
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
#ifdef HAVE_LOGIN_CAP_H
# ifdef __bsdi__
    /* On BSD/OS the login_cap_t typedef has the 'struct' builtin */
    login_cap_t	*lc;
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

#define PAM_BAIL	\
    if (pam_error != PAM_SUCCESS) { \
	pam_end(*pamh, 0); \
	FAILV; \
    }
    PAM_password = greet->password;
    pam_error = pam_start(KDE_PAM, greet->name, &PAM_conversation, pamh);
    PAM_BAIL;
    pam_error = pam_set_item(*pamh, PAM_TTY, d->name);
    PAM_BAIL;
    if (greet->password) {
	pam_error = pam_authenticate(*pamh, 0);
	PAM_BAIL;
    }
    pam_error = pam_acct_mgmt(*pamh, 0);
    /* really should do password changing, but it doesn't fit well */
    PAM_BAIL;
    pam_error = pam_setcred(*pamh, 0);
    PAM_BAIL;
#undef PAM_BAIL

/*
#elif defined(_AIX) / * USE_PAM * /

	int		loginType;
	struct stat	statBuf;
	char		v_work[512], tty[512];
	char		*msg1;
	char		hostname[512];
	char		*tmpch;

	if (d->displayType.location == Foreign) {
	    strncpy(hostname, d->name, 511);
	    hostname[511] = '\0';
	    if ((tmpch = strchr(hostname, ':')))
	        *tmpch = '\0';
	} else {
	    hostname[0] = '\0';
	}
	CleanUpName(d->name,v_work,512);
        sprintf(tty,"/dev/xdm/%s",v_work);
	tty[15] = '\0';      / ** tty names should only be 15 characters long ** /

	loginType = d->displayType.location == Foreign ? S_RLOGIN : S_LOGIN;
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

#if !defined(USE_PAM) /*&& !defined(_AIX)*/

#ifdef __linux__	/* only Linux? */
    if (p->pw_passwd[0] == '!' || p->pw_passwd[0] == '*') {
	Debug ("The account is locked, no login allowed.\n");
	FAILV;
    }
#endif

    if (greet->password) {

#ifdef KRB4
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
# ifdef AFS
		    if (k_hasafs()) {
			if (k_setpag() == -1)
			    LogError ("setpag() failed for %s\n", greet->name);

			if ((ret = k_afsklog(NULL, NULL)) != KSUCCESS)
			    LogError("Warning %s\n", krb_get_err_text(ret));
		    }
# endif
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
#endif  /* KRB4 */

#if defined(ultrix) || defined(__ultrix__)
	if (authenticate_user(p, greet->password, NULL) < 0)
#else
	if (strcmp (crypt (greet->password, user_pass), user_pass))
#endif
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

#ifndef _PATH_NOLOGIN
# define _PATH_NOLOGIN "/etc/nologin"
#endif

#ifdef USE_LOGIN_CAP
    /* Do we ignore a nologin file? */
    if (login_getcapbool(lc, "ignorenologin", 0))
	goto nolog_succ;

    nolg = login_getcapstr(lc, "nologin", "", NULL);
    if (!stat(nolg, &st))
	goto nolog;
#endif

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
#ifdef USE_LOGIN_CAP
nolog_succ:
#endif
#endif /* !USE_PAM */
/* restrict_nologin */


/* restrict_nohome */
#if defined(HAVE_LOGIN_CAP_H) && !defined(__NetBSD__)

    if (login_getcapbool(lc, "requirehome", 0)) {
	seteuid(p->pw_uid);
	if (!*p->pw_dir || chdir(p->pw_dir) < 0) {
	    login_close(lc);
	    FAILVV(V_NOHOME);
	}
	seteuid(0);
    }

#endif
/* restrict_nohome */


/* restrict_expired */
#if defined(HAVE_PW_EXPIRE) || defined(USESHADOW) /* && !defined(USE_PAM) ? */

#define DEFAULT_WARN  (2L * 7L * 86400L)  /* Two weeks */

    tim = time(NULL);

#if defined(HAVE_LOGIN_CAP_H) && !defined(__NetBSD__)
    quietlog = login_getcapbool(lc, "hushlogin", 0);
    warntime = login_getcaptime(lc, "warnexpire",
				DEFAULT_WARN, DEFAULT_WARN);
#else
    quietlog = 0;
# ifdef USESHADOW
    warntime = sp->sp_warn*86400;
# else
    warntime = DEFAULT_WARN;
# endif
#endif

#ifdef HAVE_PW_EXPIRE
    exp = p->pw_expire;
    if (exp) {
#else
    if (sp->sp_expire != -1) {
	exp = sp->sp_expire*86400;
#endif
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

#ifdef HAVE_PW_EXPIRE
    exp = p->pw_change;
    if (exp) {
#else
    if (sp->sp_max != -1) {
	exp = (sp->sp_lstchg+sp->sp_max)*86400;
#endif
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
/* restrict_expired */


/* restrict_time */
#ifdef USE_LOGIN_CAP
    if (!auth_timeok(lc, time(NULL))) {
	login_close(lc);
	FAILVV(V_BADTIME);
    }
/* restrict_time */


    login_close(lc);
#endif

norestr:
    Debug ("verify succeeded\n");
    /* The password is passed to StartClient() for use by user-based
       authorization schemes.  It is zeroed there. */
    verify->uid = p->pw_uid;
#ifdef NGROUPS_MAX
    getGroups (greet->name, verify, p->pw_gid);
#else
    verify->gid = p->pw_gid;
#endif
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
#define superuser "root"
#ifndef USE_PAM
    struct passwd *pws = getpwnam( superuser);
    if (!pws) {
	printf("can't verify " superuser " passwd, getpwnam() failed\n");
	return V_FAIL;
    }
    endpwent();
#ifdef USESHADOW
    /* If USESHADOW is defined, kdm will work for both shadow and non
     * shadow systems
     */
    {
	struct spwd *spws = getspnam( superuser);
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
    printf("can't verify " superuser " passwd, lacking crypt() support\n");
    return V_FAIL;
#endif
#else /* USE_PAM */
    {
	pam_handle_t *pamh;
#   	define PAM_BAIL \
	    if (pam_error != PAM_SUCCESS) { \
		pam_end(pamh, 0); \
		return V_FAIL; \
	    }
	PAM_password = (char *)pw;
	pam_error = pam_start(KDE_PAM, superuser, &PAM_conversation, &pamh);
	PAM_BAIL;
	pam_error = pam_authenticate( pamh, 0);
	PAM_BAIL;
	/* OK, if we get here, the user _should_ be root */
	pam_end( pamh, PAM_SUCCESS);
    }
#endif /* USE_PAM */
    return V_OK;
}

