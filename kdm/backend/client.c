/* $TOG: verify.c /main/37 1998/02/11 10:00:45 kaleb $ */
/*

Copyright 1988, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

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
 * user verification and session initiation.
 */

#include "dm.h"
#include "dm_auth.h"
#include "dm_error.h"

#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#ifdef SECURE_RPC
# include <rpc/rpc.h>
# include <rpc/key_prot.h>
#endif
#ifdef K5AUTH
# include <krb5/krb5.h>
#endif
#ifdef CSRG_BASED
# ifdef HAS_SETUSERCONTEXT
#  include <login_cap.h>
#  define USE_LOGIN_CAP 1
# endif
#endif
#ifdef USE_PAM
# ifdef __DARWIN__
#  include <pam/pam_appl.h>
# else
#  include <security/pam_appl.h>
# endif
#elif defined(AIXV3) /* USE_PAM */
# include <login.h>
# include <usersec.h>
extern int loginrestrictions (const char *Name, const int Mode, const char *Tty, char **Msg);
extern int loginfailed (const char *User, const char *Host, const char *Tty);
extern int loginsuccess (const char *User, const char *Host, const char *Tty, char **Msg);
#else /* USE_PAM || AIXV3 */
# ifdef USESHADOW
#  include <shadow.h>
# endif
# ifdef KERBEROS
#  include <sys/param.h>
#  include <krb.h>
#  ifndef NO_AFS
#   include <kafs.h>
#  endif
# endif
/* for nologin */
# include <sys/types.h>
# include <unistd.h>
/* for expiration */
# include <time.h>
#endif	/* USE_PAM || AIXV3 */

#if defined(__osf__) || defined(linux) || defined(__QNXNTO__) || defined(__GNU__)
# define setpgrp setpgid
#endif

#ifdef QNX4
extern char *crypt(const char *, const char *);
#endif

/*
 * Session data, mostly what struct verify_info was for
 */
char *curuser;
char *curpass;
char **userEnviron;
char **systemEnviron;
static int curuid;
static int curgid;

char *dmrcuser;
char *curdmrc;
char *newdmrc;

static struct passwd *p;
#ifdef USE_PAM
static pam_handle_t *pamh;
#elif defined(AIXV3)
static char tty[16], hostname[100];
#else
# ifdef USESHADOW
static struct spwd *sp;
# endif
# ifdef KERBEROS
static char krbtkfile[MAXPATHLEN];
# endif
#endif

#define V_RET(e) \
	do { \
		PrepErrorGreet (); \
		GSendInt (e); \
		return 0; \
	} while(0)

#ifdef USE_PAM

# ifdef sun
typedef struct pam_message pam_message_type;
# else
typedef const struct pam_message pam_message_type;
# endif

struct pam_data {
    GConvFunc gconv;
    int usecur;
    int abort;
};

static int
PAM_conv (int num_msg,
	  pam_message_type **msg,
	  struct pam_response **resp,
	  void *appdata_ptr)
{
    int count;
    const char *prompt;
    struct pam_response *reply;
    struct pam_data *pd = (struct pam_data *)appdata_ptr;

    if (!(reply = calloc(num_msg, sizeof(*reply))))
	return PAM_CONV_ERR;

    ReInitErrorLog ();
    Debug( "PAM_conv\n" );
    for (count = 0; count < num_msg; count++) {
	switch (msg[count]->msg_style) {
	case PAM_TEXT_INFO:
	    Debug( " PAM_TEXT_INFO: %s\n", msg[count]->msg );
	    PrepErrorGreet ();
	    GSendInt (V_MSG_INFO);
	    GSendStr (msg[count]->msg);
	    continue;
	case PAM_ERROR_MSG:
	    Debug( " PAM_ERROR_MSG: %s\n", msg[count]->msg );
	    PrepErrorGreet ();
	    GSendInt (V_MSG_ERR);
	    GSendStr (msg[count]->msg);
	    continue;
	default:
	    /* could do better error handling here, but see below ... */
	    if (pd->usecur) {
		switch (msg[count]->msg_style) {
		case PAM_PROMPT_ECHO_ON:
		    Debug( " PAM_PROMPT_ECHO_ON (usecur): %s\n", msg[count]->msg );
		    if (StrCmp (msg[count]->msg, "<user>")) {
			LogError( "Unexpected PAM prompt: %s\n", msg[count]->msg );
			goto conv_err;
		    }
		    StrDup (&reply[count].resp, curuser);
		    break;
		case PAM_PROMPT_ECHO_OFF:
		    Debug( " PAM_PROMPT_ECHO_OFF (usecur): %s\n", msg[count]->msg );
		    if (!curpass)
			pd->gconv (GCONV_PASS, 0);
		    StrDup (&reply[count].resp, curpass);
		    break;
		default:
		    LogError( "Unknown PAM message style <%d>\n", msg[count]->msg_style );
		    goto conv_err;
		}
	    } else {
		switch (msg[count]->msg_style) {
		case PAM_PROMPT_ECHO_ON:
		    prompt = strcmp (msg[count]->msg, "<user>") ? msg[count]->msg : 0;
		    Debug( " PAM_PROMPT_ECHO_ON: %s\n", prompt );
		    reply[count].resp = pd->gconv (GCONV_NORMAL, prompt);
		    break;
		case PAM_PROMPT_ECHO_OFF:
		    prompt = memcmp (msg[count]->msg, "Password:", 9) ? msg[count]->msg : 0;
		    Debug( " PAM_PROMPT_ECHO_OFF: %s\n", prompt );
		    reply[count].resp = pd->gconv (GCONV_HIDDEN, prompt);
		    break;
		case PAM_BINARY_PROMPT:
		    Debug( " PAM_BINARY_PROMPT\n" );
		    reply[count].resp = pd->gconv (GCONV_BINARY, msg[count]->msg);
		    break;
		default:
		    LogError( "Unknown PAM message style <%d>\n", msg[count]->msg_style );
		    goto conv_err;
		}
	    }
	    if (!reply[count].resp) {
		Debug( "  PAM_conv aborted\n" );
		pd->abort = TRUE;
		goto conv_err;
	    }
	}
	reply[count].resp_retcode = PAM_SUCCESS; /* unused in linux-pam */
    }
    Debug( " PAM_conv success\n" );
    *resp = reply;
    return PAM_SUCCESS;

  conv_err:
    for (; count >= 0; count--)
	if (reply[count].resp) {
	    switch (msg[count]->msg_style) {
	    case PAM_PROMPT_ECHO_ON:
	    case PAM_PROMPT_ECHO_OFF: /* could wipe ... */
	    case PAM_BINARY_PROMPT: /* ... that too ... */
		free(reply[count].resp);
		break;
	    }
	}
    free (reply);
    return PAM_CONV_ERR;
}

static int
PAM_conv_null (int num_msg,
	  pam_message_type **msg,
	  struct pam_response **resp,
	  void *appdata_ptr ATTR_UNUSED)
{
    int count;
    struct pam_response *reply;

    if (!(reply = calloc(num_msg, sizeof(*reply))))
	return PAM_CONV_ERR;

    ReInitErrorLog ();
    Debug( "PAM_conv_null\n" );
    for (count = 0; count < num_msg; count++) {
	switch (msg[count]->msg_style) {
	case PAM_TEXT_INFO:
	    Debug( " PAM_TEXT_INFO: %s\n", msg[count]->msg );
	    continue;
	case PAM_ERROR_MSG:
	    LogError( "PAM error message: %s\n", msg[count]->msg );
	    continue;
	default:
	    /* unknown */
	    Debug( " PAM_<%d>\n", msg[count]->msg_style );
	    free (reply);
	    return PAM_CONV_ERR;
	}
	reply[count].resp_retcode = PAM_SUCCESS; /* unused in linux-pam */
    }
    Debug( " PAM_conv_null success\n" );
    *resp = reply;
    return PAM_SUCCESS;
}

# ifdef PAM_FAIL_DELAY
static void
fail_delay(int retval ATTR_UNUSED, unsigned usec_delay ATTR_UNUSED, 
	   void *appdata_ptr ATTR_UNUSED)
{}
# endif

static int
doPAMAuth (const char *psrv, struct pam_data *pdata)
{
    const char		*pitem;
    struct pam_conv	pconv;
    int			pretc;

    pdata->abort = FALSE;
    pconv.conv = PAM_conv;
    pconv.appdata_ptr = (void *)pdata;
    Debug (" PAM service %s\n", psrv);
    if (pamh) {
	pam_get_item (pamh, PAM_SERVICE, (const void **)&pitem);
	ReInitErrorLog ();
	if (strcmp (pitem, psrv)) {
	    Debug ("closing old PAM handle\n");
	    pam_end (pamh, PAM_SUCCESS);
	    ReInitErrorLog ();
	    goto opennew;
	}
	Debug ("reusing old PAM handle\n");
/*    this makes linux-pam crash ...
	if ((pretc = pam_set_item (pamh, PAM_SERVICE, psrv)) != PAM_SUCCESS)
	    goto pam_bail;
*/
	if ((pretc = pam_set_item (pamh, PAM_CONV, &pconv)) != PAM_SUCCESS) {
	  pam_bail:
	    pam_end (pamh, pretc);
	    pamh = 0;
	  pam_bail2:
	    ReInitErrorLog ();
	    LogError ("PAM error: %s\n", pam_strerror (pamh, pretc));
	    V_RET (V_ERROR);
	}
	pam_set_item (pamh, PAM_USER, 0);
    } else {
      opennew:
	Debug ("opening new PAM handle\n");
	if (pam_start (psrv, 0, &pconv, &pamh) != PAM_SUCCESS)
	    goto pam_bail2;
	if ((pretc = pam_set_item (pamh, PAM_TTY, td->name)) != PAM_SUCCESS)
	    goto pam_bail;
	if ((pretc = pam_set_item (pamh, PAM_RHOST, "")) != PAM_SUCCESS)
	    goto pam_bail;
	if ((pretc = pam_set_item (pamh, PAM_USER_PROMPT, "<user>")) != PAM_SUCCESS)
	    goto pam_bail;
# ifdef PAM_FAIL_DELAY
	pam_set_item (pamh, PAM_FAIL_DELAY, (void *)fail_delay);
# endif
    }
    ReInitErrorLog ();

    Debug (" pam_authenticate() ...\n");
    pretc = pam_authenticate (pamh,
			td->allowNullPasswd ? 0 : PAM_DISALLOW_NULL_AUTHTOK);
    ReInitErrorLog ();
    Debug (" pam_authenticate() returned: %s\n", pam_strerror (pamh, pretc));
    if (pdata->abort)
	return 0;
    if (!curuser) {
	Debug (" asking PAM for user ...\n");
	pam_get_item (pamh, PAM_USER, (const void **)&pitem);
	ReInitErrorLog ();
	StrDup (&curuser, pitem);
	GSendInt (V_PUT_USER);
	GSendStr (curuser);
    }
    if (pretc != PAM_SUCCESS) {
	switch (pretc) {
	case PAM_USER_UNKNOWN:
	case PAM_AUTH_ERR:
	case PAM_MAXTRIES: /* should handle this better ... */
	case PAM_AUTHINFO_UNAVAIL: /* returned for unknown users ... bogus */
	    V_RET (V_AUTH);
	default:
	    V_RET (V_ERROR);
	}
    }
    return 1;
}

#endif /* USE_PAM */

static int
AccNoPass (const char *un)
{
    char **fp;

    if (!strcmp (un, td->autoUser))
	return 1;

    for (fp = td->noPassUsers; *fp; fp++)
	if (!strcmp (un, *fp))
	    return 1;

    return 0;
}

#if !defined(USE_PAM) && !defined(AIXV3) && defined(USE_LOGIN_CAP)
# define LC_RET0 do { login_close(lc); return 0; } while(0)
#else
# define LC_RET0 return 0
#endif

int
Verify (const char *type, GConvFunc gconv)
{
#ifdef USE_PAM
    const char		*psrv;
    struct pam_data	pdata;
    int			pretc;
    char		psrvb[32];
#elif defined(AIXV3)
    char		*msg, *curret;
    int			i, reenter;
#else
    struct stat		st;
    const char		*nolg;
# ifdef HAVE_GETUSERSHELL
    char		*s;
# endif
# if defined(HAVE_PW_EXPIRE) || defined(USESHADOW)
    int			tim, expir, warntime, quietlog;
# endif
# ifdef USE_LOGIN_CAP
#  ifdef HAVE_LOGIN_GETCLASS
    login_cap_t		*lc;
#  else
    struct login_cap	*lc;
#  endif
# endif
#endif

    Debug ("Verify ...\n");

#ifdef USE_PAM

    if (!strcmp (type, "classic")) {
	if (!gconv (GCONV_USER, 0))
	    return 0;
	if (AccNoPass (curuser)) {
	    gconv (GCONV_PASS_ND, 0);
	    if (!*curpass) {
		sprintf (psrvb, "%.28s-np", PAMService);
		psrv = psrvb;
	    } else
		psrv = PAMService;
	} else
	    psrv = PAMService;
	pdata.usecur = TRUE;
    } else {
	psrv = type;
	pdata.usecur = FALSE;
    }
    pdata.gconv = gconv;
    if (!doPAMAuth (psrv, &pdata))
	return 0;

#elif defined(AIXV3)

    if ((td->displayType & d_location) == dForeign) {
	char *tmpch;
	strncpy (hostname, td->name, sizeof(hostname) - 1);
	hostname[sizeof(hostname)-1] = '\0';
	if ((tmpch = strchr (hostname, ':')))
	    *tmpch = '\0';
    } else
	hostname[0] = '\0';

    /* tty names should only be 15 characters long */
# if 0
    for (i = 0; i < 15 && td->name[i]; i++) {
	if (td->name[i] == ':' || td->name[i] == '.')
	    tty[i] = '_';
	else
	    tty[i] = td->name[i];
    }
    tty[i] = '\0';
# else
    memcpy (tty, "/dev/xdm/", 9);
    for (i = 0; i < 6 && td->name[i]; i++) {
	if (td->name[i] == ':' || td->name[i] == '.')
	    tty[9 + i] = '_';
	else
	    tty[9 + i] = td->name[i];
    }
    tty[9 + i] = '\0';
# endif

    if (!strcmp (type, "classic")) {
	if (!gconv (GCONV_USER, 0))
	    return 0;
	if (AccNoPass (curuser)) {
	    gconv (GCONV_PASS_ND, 0);
	    if (!*curpass) {
		Debug ("accepting despite empty password\n");
		goto done;
	    }
	} else
	    if (!gconv (GCONV_PASS, 0))
		return 0;
	enduserdb();
	msg = NULL;
	if ((i = authenticate (curuser, curpass, &reenter, &msg))) {
	    Debug ("authenticate() failed: %s\n", msg);
	    if (msg)
		free (msg);
	    loginfailed (curuser, hostname, tty);
	    V_RET (i == ENOENT || i == ESAD ? V_AUTH : V_ERROR);
	}
	if (reenter) {
	    LogError ("authenticate() requests more data: %s\n", msg);
	    free (msg);
	    V_RET (V_ERROR);
	}
    } else if (!strcmp (type, "generic")) {
	if (!gconv (GCONV_USER, 0))
	    return 0;
	for (curret = 0;;) {
	    msg = NULL;
	    if ((i = authenticate (curuser, curret, &reenter, &msg))) {
		Debug ("authenticate() failed: %s\n", msg);
		if (msg)
		    free (msg);
		loginfailed (curuser, hostname, tty);
		V_RET (i == ENOENT || i == ESAD ? V_AUTH : V_ERROR);
	    }
	    if (!reenter)
		break;
	    if (!(curret = gconv (GCONV_HIDDEN, msg)))
		return 0;
	    free (msg);
	}
    } else {
	LogError ("Unsupported authentication type %\"s requested\n", type);
	V_RET (V_ERROR);
    }
    if (msg) {
	PrepErrorGreet ();
	GSendInt (V_MSG_INFO);
	GSendStr (msg);
	free (msg);
    }

  done:

#else

    if (strcmp (type, "classic")) {
	LogError ("Unsupported authentication type %\"s requested\n", type);
	V_RET (V_ERROR);
    }

    if (!gconv (GCONV_USER, 0))
	return 0;

    if (!(p = getpwnam (curuser))) {
	Debug ("getpwnam() failed.\n");
	V_RET (V_AUTH);
    }
# ifdef linux	/* only Linux? */
    if (p->pw_passwd[0] == '!' || p->pw_passwd[0] == '*') {
	Debug ("account is locked\n");
	V_RET (V_AUTH);
    }
# endif

# ifdef USESHADOW
    if ((sp = getspnam (curuser)))
	p->pw_passwd = sp->sp_pwdp;
    else
	Debug ("getspnam() failed: %s.  Are you root?\n", SysErrorMsg());
# endif

    if (!*p->pw_passwd) {
	if (!td->allowNullPasswd) {
	    Debug ("denying user with empty password\n");
	    V_RET (V_AUTH);
	}
	goto nplogin;
    }

    if (AccNoPass (curuser)) {
      nplogin:
	gconv (GCONV_PASS_ND, 0);
	if (!*curpass) {
	    Debug ("accepting password-less login\n");
	    goto done;
	}
    } else
	if (!gconv (GCONV_PASS, 0))
	    return 0;

# ifdef KERBEROS
    if (p->pw_uid)
    {
	int ret;
	char realm[REALM_SZ];

	if (krb_get_lrealm (realm, 1)) {
	    LogError ("Can't get KerberosIV realm.\n");
	    V_RET (V_ERROR);
	}

	sprintf (krbtkfile, "%s.%.*s", TKT_ROOT, MAXPATHLEN - strlen(TKT_ROOT) - 2, td->name);
	krb_set_tkt_string (krbtkfile);
	unlink (krbtkfile);

	ret = krb_verify_user (curuser, "", realm, curpass, 1, "rcmd");
	if (ret == KSUCCESS) {
	    chown (krbtkfile, p->pw_uid, p->pw_gid);
	    Debug ("KerberosIV verify succeeded\n");
	    goto done;
	} else if (ret != KDC_PR_UNKNOWN && ret != SKDC_CANT) {
	    LogError ("KerberosIV verification failure %\"s for %s\n",
		      krb_get_err_text (ret), curuser);
	    krbtkfile[0] = '\0';
	    V_RET (V_ERROR);
	}
	Debug ("KerberosIV verify failed: %s\n", krb_get_err_text (ret));
    }
    krbtkfile[0] = '\0';
# endif  /* KERBEROS */

# if defined(ultrix) || defined(__ultrix__)
    if (authenticate_user(p, curpass, NULL) < 0)
# else
    if (strcmp (crypt (curpass, p->pw_passwd), p->pw_passwd))
# endif
    {
	Debug ("password verify failed\n");
	V_RET (V_AUTH);
    }

  done:

#endif /* !defined(USE_PAM) && !defined(AIXV3) */

    Debug ("restrict %s ...\n", curuser);

#if defined(USE_PAM) || defined(AIXV3)
    if (!(p = getpwnam (curuser))) {
	LogError ("getpwnam(%s) failed.\n", curuser);
	V_RET (V_ERROR);
    }
#endif
    if (!p->pw_uid) {
	if (!td->allowRootLogin) /* handle non-sessions differently? */
	    V_RET (V_NOROOT);
	return 1;	/* don't deny root to log in */
    }

#ifdef USE_PAM

    Debug (" pam_acct_mgmt() ...\n");
    pretc = pam_acct_mgmt (pamh, 0);
    ReInitErrorLog ();
    Debug (" pam_acct_mgmt() returned: %s\n", pam_strerror (pamh, pretc));
    if (pretc == PAM_NEW_AUTHTOK_REQD) {
	pdata.usecur = FALSE;
	pdata.gconv = conv_interact;
	/* pam will have output a message already, so no PrepErrorGreet () */
	if (gconv != conv_interact || psrv == psrvb) {
	    GSendInt (V_CHTOK_AUTH);
	    /* this cannot auth the wrong user, as only classic auths get here */
	    while (!doPAMAuth (psrv, &pdata))
		if (pdata.abort)
		    return 0;
	    GSendInt (V_PRE_OK);
	} else
	    GSendInt (V_CHTOK);
	for (;;) {
	    /* don't use PAM_CHANGE_EXPIRED_AUTHTOK:
	       - makes pam_unix ask for a password (why?!)
	       - the frontend cannot depend on the requested item sequence
	    */
	    Debug (" pam_chauthtok() ...\n");
	    pretc = pam_chauthtok (pamh, 0);
	    ReInitErrorLog ();
	    Debug (" pam_chauthtok() returned: %s\n", pam_strerror (pamh, pretc));
	    if (pdata.abort)
		return 0;
	    if (pretc == PAM_SUCCESS)
		break;
	    /* effectively there is only PAM_AUTHTOK_ERR */
	    GSendInt (V_RETRY);
	}
    } else if (pretc != PAM_SUCCESS)
	V_RET (V_AUTH);

#elif defined(AIXV3)	/* USE_PAM */

    msg = NULL;
    if (loginrestrictions (curuser,
	((td->displayType & d_location) == dForeign) ? S_RLOGIN : S_LOGIN,
	tty, &msg) == -1)
    {
	Debug ("loginrestrictions() - %s\n", msg ? msg : "error");
	loginfailed (curuser, hostname, tty);
	PrepErrorGreet ();
	if (msg) {
	    GSendInt (V_MSG_ERR);
	    GSendStr (msg);
	}
	GSendInt (V_AUTH);
	return 0;
    }
    if (msg)
	free ((void *)msg);

#else	/* USE_PAM || AIXV3 */

# ifdef HAVE_GETUSERSHELL
    for (;;) {
	if (!(s = getusershell ())) {
	    Debug ("shell not in /etc/shells\n");
	    endusershell ();
	    V_RET (V_BADSHELL);
	}
	if (!strcmp (s, p->pw_shell)) {
	    endusershell ();
	    break;
	}
    }
# endif

# ifdef USE_LOGIN_CAP
#  ifdef HAVE_LOGIN_GETCLASS
    lc = login_getclass (p->pw_class);
#  else
    lc = login_getpwclass (p);
#  endif
    if (!lc)
	V_RET (V_ERROR);
# endif


/* restrict_nologin */
# ifndef _PATH_NOLOGIN
#  define _PATH_NOLOGIN "/etc/nologin"
# endif

    if ((
# ifdef USE_LOGIN_CAP
    /* Do we ignore a nologin file? */
	!login_getcapbool (lc, "ignorenologin", 0)) &&
	(!stat ((nolg = login_getcapstr (lc, "nologin", "", NULL)), &st) ||
# endif
	 !stat ((nolg = _PATH_NOLOGIN), &st)))
    {
	PrepErrorGreet ();
	GSendInt (V_NOLOGIN);
	GSendStr (nolg);
	LC_RET0;
    }


/* restrict_nohome */
# ifdef USE_LOGIN_CAP
    if (login_getcapbool (lc, "requirehome", 0)) {
	struct stat st;
	if (!*p->pw_dir || stat (p->pw_dir, &st) || st.st_uid != p->pw_uid) {
	    PrepErrorGreet ();
	    GSendInt (V_NOHOME);
	    LC_RET0;
	}
    }
# endif


/* restrict_time */
# ifdef USE_LOGIN_CAP
#  ifdef HAVE_AUTH_TIMEOK
    if (!auth_timeok (lc, time (NULL))) {
	PrepErrorGreet ();
	GSendInt (V_BADTIME);
	LC_RET0;
    }
#  endif
# endif


/* restrict_expired; this MUST be the last one */
# if defined(HAVE_PW_EXPIRE) || defined(USESHADOW)

#  if !defined(HAVE_PW_EXPIRE) || (!defined(USE_LOGIN_CAP) && defined(USESHADOW))
    if (sp)
#  endif
    {

#  define DEFAULT_WARN  (2L * 7L)  /* Two weeks */

	tim = time (NULL) / 86400L;

#  ifdef USE_LOGIN_CAP
	quietlog = login_getcapbool (lc, "hushlogin", 0);
	warntime = login_getcaptime (lc, "warnexpire",
				     DEFAULT_WARN * 86400L, 
				     DEFAULT_WARN * 86400L) / 86400L;
#  else
	quietlog = 0;
#   ifdef USESHADOW
	warntime = sp->sp_warn != -1 ? sp->sp_warn : DEFAULT_WARN;
#   else
	warntime = DEFAULT_WARN;
#   endif
#  endif

#  ifdef HAVE_PW_EXPIRE
	if (p->pw_expire) {
	    expir = p->pw_expire / 86400L;
#  else
	if (sp->sp_expire != -1) {
	    expir = sp->sp_expire;
#  endif
	    if (tim > expir) {
		PrepErrorGreet ();
		GSendInt (V_AEXPIRED);
		LC_RET0;
	    } else if (tim > (expir - warntime) && !quietlog) {
		PrepErrorGreet ();
		GSendInt (V_AWEXPIRE);
		GSendInt (expir - tim);
	    }
	}

#  ifdef HAVE_PW_EXPIRE
	if (p->pw_change) {
	    expir = p->pw_change / 86400L;
#  else
	if (!sp->sp_lstchg) {
	    PrepErrorGreet ();
	    GSendInt (V_PFEXPIRED);
	    /* XXX todo password change */
	    GSendInt (V_AUTH);
	    LC_RET0;
	} else if (sp->sp_max != -1) {
	    expir = sp->sp_lstchg + sp->sp_max;
	    if (sp->sp_inacct != -1 && tim > expir + sp->sp_inacct) {
		PrepErrorGreet ();
		GSendInt (V_APEXPIRED);
		LC_RET0;
	    }
#  endif
	    if (tim > expir) {
		PrepErrorGreet ();
		GSendInt (V_PEXPIRED);
		/* XXX todo password change */
		GSendInt (V_AUTH);
		LC_RET0;
	    } else if (tim > (expir - warntime) && !quietlog) {
		PrepErrorGreet ();
		GSendInt (V_PWEXPIRE);
		GSendInt (expir - tim);
	    }
	}

    }

# endif /* HAVE_PW_EXPIRE || USESHADOW */

# ifdef USE_LOGIN_CAP
    login_close (lc);
# endif

#endif /* USE_PAM || AIXV3 */

    return 1;

}


static const char *envvars[] = {
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
userEnv (int isRoot, const char *user, const char *home, const char *shell)
{
    char	**env, *xma;

    env = defaultEnv (user);
    xma = 0;
    if (td->fifoPath && StrDup (&xma, td->fifoPath))
	if ((td->allowShutdown == SHUT_ALL ||
	     (td->allowShutdown == SHUT_ROOT && isRoot)) &&
	    StrApp (&xma, ",maysd", (char *)0))
	{
	    if (td->allowNuke == SHUT_ALL ||
		(td->allowNuke == SHUT_ROOT && isRoot))
		StrApp (&xma, ",mayfn", (char *)0);
	    StrApp (&xma, td->defSdMode == SHUT_FORCENOW ? ",fn" :
			  td->defSdMode == SHUT_TRYNOW ? ",tn" : ",sched", 
		    (char *)0);
	}
	if ((td->displayType & d_location) == dLocal && AnyReserveDisplays ())
	    StrApp (&xma, ",rsvd", (char *)0);
    if (xma)
    {
	env = setEnv (env, "XDM_MANAGED", xma);
	free (xma);
    }
    else
	env = setEnv (env, "XDM_MANAGED", "true");
    env = setEnv (env, "DISPLAY", td->name);
    env = setEnv (env, "HOME", home);
    env = setEnv (env, "PATH", isRoot ? td->systemPath : td->userPath);
    env = setEnv (env, "SHELL", shell);
#if !defined(USE_PAM) && !defined(AIXV3) && defined(KERBEROS)
    if (krbtkfile[0] != '\0')
	env = setEnv (env, "KRBTKFILE", krbtkfile);
#endif
    env = inheritEnv (env, envvars);
    return env;
}


static int
SetGid (const char *name, int gid)
{
    if (setgid(gid) < 0)
    {
	LogError("setgid(%d) (user %s) failed: %s\n",
		 gid, name, SysErrorMsg());
	return 0;
    }
#ifndef QNX4
    if (initgroups(name, gid) < 0)
    {
	LogError("initgroups for %s failed: %s\n", name, SysErrorMsg());
	return 0;
    }
#endif   /* QNX4 doesn't support multi-groups, no initgroups() */
    return 1;
}

static int
SetUid (const char *name, int uid)
{
    if (setuid(uid) < 0)
    {
	LogError("setuid(%d) (user %s) failed: %s\n",
		 uid, name, SysErrorMsg());
	return 0;
    }
    return 1;
}

static int
SetUser (const char *name, int uid, int gid)
{
    return SetGid (name, gid) && SetUid (name, uid);
}

static void
mergeSessionArgs (int cansave)
{
    char *mfname;
    const char *fname;
    int i, needsave;

    mfname = 0;
    fname = ".dmrc";
    if ((!curdmrc || newdmrc) && *dmrcDir)
	if (StrApp (&mfname, dmrcDir, "/", curuser, fname, 0))
	    fname = mfname;
    needsave = 0;
    if (!curdmrc) {
	curdmrc = iniLoad (fname);
	if (!curdmrc) {
	    StrDup (&curdmrc, "[Desktop]\nSession=default\n");
	    needsave = 1;
	}
    }
    if (newdmrc) {
	curdmrc = iniMerge (curdmrc, newdmrc);
	needsave = 1;
    }
    if (needsave && cansave)
	if (!iniSave (curdmrc, fname) && errno == ENOENT && mfname) {
	    for (i = 0; mfname[i]; i++)
		if (mfname[i] == '/') {
		    mfname[i] = 0;
		    mkdir (mfname, 0755);
		    mfname[i] = '/';
		}
	    iniSave (curdmrc, mfname);
	}
    if (mfname)
	free (mfname);
}

static int removeAuth;
static int sourceReset;

int
StartClient ()
{
    const char	*shell, *home, *sessargs, *desksess;
    char	**argv, *fname, *str;
#ifdef USE_PAM
    char	**pam_env;
    struct pam_conv pconv;
    int		pretc;
#else
# ifdef AIXV3
    char	*msg;
    char	**theenv;
    extern char	**newenv; /* from libs.a, this is set up by setpenv */
# endif
#endif
#ifdef HAS_SETUSERCONTEXT
    extern char	**environ;
#endif
    char	*failsafeArgv[2];
    int		i, pid;

    if (StrCmp (dmrcuser, curuser)) {
	if (curdmrc) { free (curdmrc); curdmrc = 0; }
	if (dmrcuser) { free (dmrcuser); dmrcuser = 0; }
    }

#if defined(USE_PAM) || defined(AIXV3)
    if (!(p = getpwnam (curuser))) {
	LogError ("getpwnam(%s) failed.\n", curuser);
	return 0;
    }
#endif

#ifndef USE_PAM
# ifdef AIXV3
    msg = NULL;
    loginsuccess (curuser, hostname, tty, &msg);
    if (msg) {
	Debug ("loginsuccess() - %s\n", msg);
	free ((void *)msg);
    }
# else /* AIXV3 */
#  if defined(KERBEROS) && !defined(NO_AFS)
    if (krbtkfile[0] != '\0') {
	if (k_hasafs ()) {
	    if (k_setpag () == -1)
		LogError ("setpag() for %s failed\n", curuser);
	    if ((ret = k_afsklog (NULL, NULL)) != KSUCCESS)
		LogError ("AFS Warning: %s\n", krb_get_err_text (ret));
	}
    }
#  endif /* KERBEROS && AFS */
# endif /* AIXV3 */
#endif	/* !PAM */

    curuid = p->pw_uid;
    curgid = p->pw_gid;
    home = p->pw_dir;
    shell = p->pw_shell;
    userEnviron = userEnv (!curuid, curuser, home, shell);
    systemEnviron = systemEnv (curuser, home);
    Debug ("user environment:\n%[|''>'\n's"
	   "system environment:\n%[|''>'\n's"
	   "end of environments\n", 
	   userEnviron,
	   systemEnviron);

    /*
     * for user-based authorization schemes,
     * add the user to the server's allowed "hosts" list.
     */
    for (i = 0; i < td->authNum; i++)
    {
#ifdef SECURE_RPC
	if (td->authorizations[i]->name_length == 9 &&
	    !memcmp (td->authorizations[i]->name, "SUN-DES-1", 9))
	{
	    XHostAddress	addr;
	    char		netname[MAXNETNAMELEN+1];
	    char		domainname[MAXNETNAMELEN+1];
    
	    getdomainname (domainname, sizeof domainname);
	    user2netname (netname, curuid, domainname);
	    addr.family = FamilyNetname;
	    addr.length = strlen (netname);
	    addr.address = netname;
	    XAddHost (dpy, &addr);
	}
#endif
#ifdef K5AUTH
	if (td->authorizations[i]->name_length == 14 &&
	    !memcmp (td->authorizations[i]->name, "MIT-KERBEROS-5", 14))
	{
	    /* Update server's auth file with user-specific info.
	     * Don't need to AddHost because X server will do that
	     * automatically when it reads the cache we are about
	     * to point it at.
	     */
	    extern Xauth *Krb5GetAuthFor ();

	    XauDisposeAuth (td->authorizations[i]);
	    td->authorizations[i] =
		Krb5GetAuthFor (14, "MIT-KERBEROS-5", td->name);
	    SaveServerAuthorizations (td, td->authorizations, td->authNum);
	}
#endif
    }

    /*
     * Run system-wide initialization file
     */
    sourceReset = 1;
    if (source (systemEnviron, td->startup) != 0) {
	LogError ("Cannot execute startup script %\"s\n", td->startup);
	SessionExit (EX_NORMAL);
    }

    if (*dmrcDir)
	mergeSessionArgs (TRUE);

    Debug ("now starting the session\n");
#ifdef USE_PAM
    pconv.conv = PAM_conv_null;
    pconv.appdata_ptr = 0;
    pam_set_item (pamh, PAM_CONV, &pconv); /* XXX this can fail */
    pam_open_session (pamh, 0); /* XXX this can fail, too */
    ReInitErrorLog ();
#endif    
    removeAuth = 1;
    if (td->fifoPath)
	chown (td->fifoPath, curuid, -1);
    endpwent ();
#if !defined(USE_PAM) && !defined(AIXV3)
# ifndef QNX4  /* QNX4 doesn't need endspent() to end shadow passwd ops */
    endspent ();
# endif
#endif
    switch (pid = Fork ()) {
    case 0:
	/* Do system-dependent login setup here */
#ifdef CSRG_BASED
	setsid();
#else
# if defined(SYSV) || defined(SVR4) || defined(__CYGWIN__)
#  if !(defined(SVR4) && defined(i386)) || defined(SCO325) || defined(__GNU__)
	setpgrp ();
#  endif
# else
	setpgrp (0, getpid ());
# endif
#endif

#if defined(USE_PAM) || !defined(AIXV3)

# ifndef HAS_SETUSERCONTEXT
	if (!SetGid (curuser, curgid))
	    exit (1);
# endif
# ifdef USE_PAM
	if ((pretc = pam_setcred (pamh, 0)) != PAM_SUCCESS) {
	    ReInitErrorLog ();
	    LogError ("pam_setcred() for %s failed: %s\n",
		      curuser, pam_strerror (pamh, pretc));
	    exit (1);
	}
	/* pass in environment variables set by libpam and modules it called */
#ifndef _AIX
	pam_env = pam_getenvlist(pamh);
#endif
	ReInitErrorLog ();
	if (pam_env)
	    for(; *pam_env; pam_env++)
		userEnviron = putEnv(*pam_env, userEnviron);
# endif
# ifndef HAS_SETUSERCONTEXT
#  if defined(BSD) && (BSD >= 199103)
	if (setlogin(curuser) < 0)
	{
	    LogError("setlogin for %s failed: %s\n", curuser, SysErrorMsg());
	    exit (1);
	}
#  endif
	if (!SetUid (curuser, curuid))
	    exit (1);
# else /* HAS_SETUSERCONTEXT */

	/*
	 * Destroy environment unless user has requested its preservation.
	 * We need to do this before setusercontext() because that may
	 * set or reset some environment variables.
	 */
	if (!(environ = initStrArr (0))) {
	    LogOutOfMem("StartSession");
	    exit (1);
	}

	/*
	 * Set the user's credentials: uid, gid, groups,
	 * environment variables, resource limits, and umask.
	 */
	if (setusercontext(NULL, p, p->pw_uid, LOGIN_SETALL) < 0)
	{
	    LogError("setusercontext for %s failed: %s\n",
		     curuser, SysErrorMsg());
	    exit (1);
	}

	for (i = 0; environ[i]; i++)
	    userEnviron = putEnv(environ[i], userEnviron);

# endif /* HAS_SETUSERCONTEXT */
#else /* AIXV3 */
	/*
	 * Set the user's credentials: uid, gid, groups,
	 * audit classes, user limits, and umask.
	 */
	if (setpcred(curuser, NULL) == -1)
	{
	    LogError("setpcred for %s failed: %s\n", curuser, SysErrorMsg());
	    exit (1);
	}

	/*
	 * Make a copy of the environment, because setpenv will trash it.
	 */
	if (!(theenv = xCopyStrArr (0, userEnviron)))
	{
	    LogOutOfMem("StartSession");
	    exit (1);
	}

	/*
	 * Set the users process environment. Store protected variables and
	 * obtain updated user environment list. This call will initialize
	 * global 'newenv'. 
	 */
	if (setpenv(curuser, PENV_INIT | PENV_ARGV | PENV_NOEXEC,
		    theenv, NULL) != 0)
	{
	    LogError("Can't set %s's process environment\n", curuser);
	    exit (1);
	}

	/*
	 * Free old userEnviron and replace with newenv from setpenv().
	 */
	free(theenv);
	freeStrArr(userEnviron);
	userEnviron = newenv;

#endif /* AIXV3 */

	/*
	 * for user-based authorization schemes,
	 * use the password to get the user's credentials.
	 */
#ifdef SECURE_RPC
	/* do like "keylogin" program */
	if (!curpass[0])
	    LogInfo("No password for NIS provided.\n");
	else
	{
	    char    netname[MAXNETNAMELEN+1], secretkey[HEXKEYBYTES+1];
	    int	    nameret, keyret;
	    int	    len;
	    int     key_set_ok = 0;

	    nameret = getnetname (netname);
	    Debug ("user netname: %s\n", netname);
	    len = strlen (curpass);
	    if (len > 8)
		bzero (curpass + 8, len - 8);
	    keyret = getsecretkey(netname, secretkey, curpass);
	    Debug ("getsecretkey returns %d, key length %d\n",
		   keyret, strlen (secretkey));
	    /* is there a key, and do we have the right password? */
	    if (keyret == 1)
	    {
		if (*secretkey)
		{
		    keyret = key_setsecret(secretkey);
		    Debug ("key_setsecret returns %d\n", keyret);
		    if (keyret == -1)
			LogError ("Failed to set NIS secret key\n");
		    else
			key_set_ok = 1;
		}
		else
		{
		    /* found a key, but couldn't interpret it */
		    LogError ("Password incorrect for NIS principal %s\n",
			      nameret ? netname : curuser);
		}
	    }
	    if (!key_set_ok)
	    {
		/* remove SUN-DES-1 from authorizations list */
		int i, j;
		for (i = 0; i < td->authNum; i++)
		{
		    if (td->authorizations[i]->name_length == 9 &&
			memcmp (td->authorizations[i]->name, "SUN-DES-1", 9) == 0)
		    {
			for (j = i+1; j < td->authNum; j++)
			    td->authorizations[j-1] = td->authorizations[j];
			td->authNum--;
			break;
		    }
		}
	    }
	    bzero (secretkey, strlen (secretkey));
	}
#endif
#ifdef K5AUTH
	/* do like "kinit" program */
	if (!curpass[0])
	    LogInfo ("No password for Kerberos5 provided.\n");
	else
	{
	    int i, j;
	    int result;
	    extern char *Krb5CCacheName ();

	    result = Krb5Init (curuser, curpass, td);
	    if (result == 0) {
		/* point session clients at the Kerberos credentials cache */
		userEnviron = setEnv (userEnviron,
				      "KRB5CCNAME", Krb5CCacheName(td->name));
	    } else {
		for (i = 0; i < td->authNum; i++)
		{
		    if (td->authorizations[i]->name_length == 14 &&
			!memcmp (td->authorizations[i]->name, "MIT-KERBEROS-5", 14))
		    {
			/* remove Kerberos from authorizations list */
			for (j = i+1; j < td->authNum; j++)
			    td->authorizations[j-1] = td->authorizations[j];
			td->authNum--;
			break;
		    }
		}
	    }
	}
#endif /* K5AUTH */
	if (curpass)
	    bzero (curpass, strlen (curpass));
	SetUserAuthorization (td);
	home = getEnv (userEnviron, "HOME");
	if (home) {
	    if (chdir (home) < 0) {
		LogError ("Cannot chdir to %s's home %s: %s, using /\n",
			  curuser, home, SysErrorMsg());
		home = 0;
		userEnviron = setEnv (userEnviron, "HOME", "/");
		chdir ("/");
	    }
	} else
	    chdir ("/");
	if (!*dmrcDir)
	    mergeSessionArgs (home != 0);
	if (!(desksess = iniEntry (curdmrc, "Desktop", "Session", 0)))
	    desksess = "failsafe"; /* only due to OOM */
	userEnviron = setEnv (userEnviron, "DESKTOP_SESSION", desksess);
	for (i = 0; td->sessionsDirs[i]; i++) {
	    fname = 0;
	    if (StrApp (&fname, td->sessionsDirs[i], "/", desksess, ".desktop", 0)) {
		if ((str = iniLoad (fname))) {
		    if (!StrCmp (iniEntry (str, "Desktop Entry", "Hidden", 0), "true") ||
			!(sessargs = iniEntry (str, "Desktop Entry", "Exec", 0)))
			sessargs = "";
		    free (str);
		    free (fname);
		    goto gotit;
		}
		free (fname);
	    }
	}
	if (!strcmp (desksess, "failsafe") ||
	    !strcmp (desksess, "default") ||
	    !strcmp (desksess, "custom"))
	    sessargs = desksess;
	else
	    sessargs = "";
      gotit:
	argv = parseArgs ((char **)0, td->session);
	if (argv && argv[0] && *argv[0]) {
		argv = addStrArr (argv, sessargs, -1);
		Debug ("executing session %\"[s\n", argv);
		execute (argv, userEnviron);
		LogError ("Session %\"s execution failed: %s\n",
			  argv[0], SysErrorMsg());
	} else {
		LogError ("Session has no command/arguments\n");
	}
	failsafeArgv[0] = td->failsafeClient;
	failsafeArgv[1] = 0;
	execute (failsafeArgv, userEnviron);
	LogError ("Failsafe client %\"s execution failed: %s\n",
		  failsafeArgv[0], SysErrorMsg());
	exit (1);
    case -1:
	LogError ("Forking session on %s failed: %s\n",
		  td->name, SysErrorMsg());
	return 0;
    default:
	Debug ("StartSession, fork succeeded %d\n", pid);
/* ### right after forking dpy	mstrtalk.pipe = &td->pipe; */
#ifdef nofork_session
	if (!nofork_session)
#endif
	if (!Setjmp (mstrtalk.errjmp)) {
	    GSet (&mstrtalk);
	    GSendInt (D_User);
	    GSendInt (curuid);
	    if (td->autoReLogin) {
		GSendInt (D_ReLogin);
		GSendStr (curuser);
		GSendStr (curpass);
		GSendStr (newdmrc);
	    }
	}
	return pid;
    }
}

void
SessionExit (int status)
{
    /* make sure the server gets reset after the session is over */
    if (td->serverPid >= 2) {
	if (!td->terminateServer && td->resetSignal)
	    TerminateProcess (td->serverPid, td->resetSignal);
    } else
	ResetServer (td);
    if (sourceReset) {
	/*
	 * run system-wide reset file
	 */
	Debug ("source reset program %s\n", td->reset);
	source (systemEnviron, td->reset);
    }
    if (removeAuth)
    {
	if (td->fifoPath)
	    chown (td->fifoPath, 0, -1);
#ifdef USE_PAM
	if (pamh) {
	    int pretc;
	    /* shutdown PAM session */
	    if ((pretc = pam_setcred (pamh, PAM_DELETE_CRED)) != PAM_SUCCESS)
		LogError ("pam_setcred(DELETE_CRED) for %s failed: %s\n",
			  curuser, pam_strerror (pamh, pretc));
	    pam_close_session (pamh, 0);
	    pam_end (pamh, PAM_SUCCESS);
	    pamh = NULL;
	    ReInitErrorLog ();
	}
#endif
	SetUser (curuser, curuid, curgid);
	RemoveUserAuthorization (td);
#ifdef K5AUTH
	/* do like "kdestroy" program */
	{
	    krb5_error_code code;
	    krb5_ccache ccache;

	    code = Krb5DisplayCCache (td->name, &ccache);
	    if (code)
		LogError ("%s while getting Krb5 ccache to destroy\n",
			 error_message (code));
	    else {
		code = krb5_cc_destroy (ccache);
		if (code) {
		    if (code == KRB5_FCC_NOFILE)
			Debug ("no Kerberos ccache file found to destroy\n");
		    else
			LogError ("%s while destroying Krb5 credentials cache\n",
				  error_message (code));
		} else
		    Debug ("kerberos ccache destroyed\n");
		krb5_cc_close (ccache);
	    }
	}
#endif /* K5AUTH */
#if !defined(USE_PAM) && !defined(AIXV3)
# ifdef KERBEROS
	if (krbtkfile[0]) {
	    (void) dest_tkt ();
#  ifndef NO_AFS
	    if (k_hasafs ())
		(void) k_unlog ();
#  endif
	}
# endif
#endif /* !USE_PAM && !AIXV3*/
#ifdef USE_PAM
    } else {
	if (pamh) {
	    pam_end (pamh, PAM_SUCCESS);
	    pamh = NULL;
	    ReInitErrorLog ();
	}
#endif
    }
    Debug ("display %s exiting with status %d\n", td->name, status);
    exit (status);
}

int
ReadDmrc ()
{
    char *data, *fname = 0;
    int len, pid, pfd[2], err;

    if (!dmrcuser || !dmrcuser[0] || !(p = getpwnam (dmrcuser)))
	return GE_NoUser;

    if (*dmrcDir) {
	if (!StrApp (&fname, dmrcDir, "/", dmrcuser, ".dmrc", 0))
	    return GE_Error;
	if (!(curdmrc = iniLoad (fname))) {
	    free (fname);
	    return GE_Ok;
	}
	free (fname);
	return GE_NoFile;
    }

    if (!StrApp (&fname, p->pw_dir, "/.dmrc", 0))
	return GE_Error;
    if ((curdmrc = iniLoad (fname))) {
	free (fname);
	return GE_Ok;
    }
    if (errno != EPERM) {
	free (fname);
	return GE_NoFile;
    }

    if (pipe (pfd))
	return GE_Error;
    if ((pid = Fork()) < 0) {
	close (pfd[0]);
	close (pfd[1]);
	return GE_Error;
    }
    if (!pid) {
	if (!SetUser (p->pw_name, p->pw_uid, p->pw_gid))
	    exit (0);
	if (!(data = iniLoad (fname))) {
	    static const int m1 = -1;
	    write (pfd[1], &m1, sizeof(int));
	    exit (0);
	}
	len = strlen (data);
	write (pfd[1], &len, sizeof(int));
	write (pfd[1], data, len + 1);
	exit (0);
    }
    close (pfd[1]);
    free (fname);
    err = GE_Error;
    if (Reader (pfd[0], &len, sizeof(int)) == sizeof(int)) {
	if (len == -1)
	    err = GE_Denied;
	else if ((curdmrc = malloc(len + 1))) {
	    if (Reader (pfd[0], curdmrc, len + 1) == len + 1)
		err = GE_Ok;
	    else {
		free (curdmrc);
		curdmrc = 0;
	    }
	}
    }
    close (pfd[0]);
    (void) Wait4 (pid);
    return err;
}


#if (defined(Lynx) && !defined(HAS_CRYPT)) || (defined(SCO) && !defined(SCO_USA) && !defined(_SCO_DS))
char *crypt(const char *s1, const char *s2)
{
    return(s2);
}
#endif
