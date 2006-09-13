/*

Copyright 1988, 1998  The Open Group
Copyright 2000-2004 Oswald Buddenhagen <ossi@kde.org>

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
IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the copyright holder.

*/

/*
 * xdm - display manager daemon
 * Author: Keith Packard, MIT X Consortium
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
extern int key_setnet( struct key_netstarg *arg );
#endif
#ifdef K5AUTH
# include <krb5/krb5.h>
#endif
#ifdef HAVE_SETUSERCONTEXT
# include <login_cap.h>
# define USE_LOGIN_CAP 1
#endif
#ifdef USE_PAM
# ifdef HAVE_PAM_PAM_APPL_H
#  include <pam/pam_appl.h>
# else
#  include <security/pam_appl.h>
# endif
#elif defined(_AIX) /* USE_PAM */
# include <login.h>
# include <usersec.h>
extern int loginrestrictions( const char *Name, const int Mode, const char *Tty, char **Msg );
extern int loginfailed( const char *User, const char *Host, const char *Tty );
extern int loginsuccess( const char *User, const char *Host, const char *Tty, char **Msg );
#else /* USE_PAM || _AIX */
# ifdef USESHADOW
#  include <shadow.h>
# endif
# ifdef KERBEROS
#  include <sys/param.h>
#  include <krb.h>
#  ifdef AFS
#   include <kafs.h>
#  endif
# endif
/* for nologin */
# include <sys/types.h>
# include <unistd.h>
/* for expiration */
# include <time.h>
#endif /* USE_PAM || _AIX */

/*
 * Session data, mostly what struct verify_info was for
 */
char *curuser;
char *curpass;
char *curtype;
char *newpass;
char **userEnviron;
char **systemEnviron;
static int curuid;
static int curgid;
int cursource;

char *dmrcuser;
char *curdmrc;
char *newdmrc;

static struct passwd *p;
#ifdef USE_PAM
static pam_handle_t *pamh;
#elif defined(_AIX)
static char tty[16], hostname[100];
#else
# ifdef USESHADOW
static struct spwd *sp;
# endif
# ifdef KERBEROS
static char krbtkfile[MAXPATHLEN];
# endif
#endif

#define V_RET_AUTH \
		do { \
			PrepErrorGreet (); \
			GSendInt (V_AUTH); \
			return 0; \
		} while(0)

#define V_RET_FAIL(m) \
		do { \
			PrepErrorGreet (); \
			GSendInt (V_MSG_ERR); \
			GSendStr (m); \
			GSendInt (V_FAIL); \
			return 0; \
		} while(0)

#ifdef USE_PAM

# ifndef PAM_MESSAGE_CONST
typedef struct pam_message pam_message_type;
typedef void *pam_gi_type;
# else
typedef const struct pam_message pam_message_type;
typedef const void *pam_gi_type;
# endif

struct pam_data {
	GConvFunc gconv;
	int usecur;
	int abort;
};

static int
PAM_conv( int num_msg,
          pam_message_type **msg,
          struct pam_response **resp,
          void *appdata_ptr )
{
	int count;
	struct pam_response *reply;
	struct pam_data *pd = (struct pam_data *)appdata_ptr;

	if (!(reply = Calloc( num_msg, sizeof(*reply) )))
		return PAM_CONV_ERR;

	ReInitErrorLog();
	Debug( "PAM_conv\n" );
	for (count = 0; count < num_msg; count++)
		switch (msg[count]->msg_style) {
		case PAM_TEXT_INFO:
			Debug( " PAM_TEXT_INFO: %s\n", msg[count]->msg );
			PrepErrorGreet();
			GSendInt( V_MSG_INFO );
			GSendStr( msg[count]->msg );
			continue;
		case PAM_ERROR_MSG:
			Debug( " PAM_ERROR_MSG: %s\n", msg[count]->msg );
			PrepErrorGreet();
			GSendInt( V_MSG_ERR );
			GSendStr( msg[count]->msg );
			continue;
		default:
			/* could do better error handling here, but see below ... */
			if (pd->usecur) {
				switch (msg[count]->msg_style) {
				/* case PAM_PROMPT_ECHO_ON: cannot happen */
				case PAM_PROMPT_ECHO_OFF:
					Debug( " PAM_PROMPT_ECHO_OFF (usecur): %s\n", msg[count]->msg );
					if (!curpass)
						pd->gconv( GCONV_PASS, 0 );
					StrDup( &reply[count].resp, curpass );
					break;
				default:
					LogError( "Unknown PAM message style <%d>\n", msg[count]->msg_style );
					goto conv_err;
				}
			} else {
				switch (msg[count]->msg_style) {
				case PAM_PROMPT_ECHO_ON:
					Debug( " PAM_PROMPT_ECHO_ON: %s\n", msg[count]->msg );
					reply[count].resp = pd->gconv( GCONV_NORMAL, msg[count]->msg );
					break;
				case PAM_PROMPT_ECHO_OFF:
					Debug( " PAM_PROMPT_ECHO_OFF: %s\n", msg[count]->msg );
					reply[count].resp = pd->gconv( GCONV_HIDDEN, msg[count]->msg );
					break;
#ifdef PAM_BINARY_PROMPT
				case PAM_BINARY_PROMPT:
					Debug( " PAM_BINARY_PROMPT\n" );
					reply[count].resp = pd->gconv( GCONV_BINARY, msg[count]->msg );
					break;
#endif
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
			reply[count].resp_retcode = PAM_SUCCESS; /* unused in linux-pam */
		}
	Debug( " PAM_conv success\n" );
	*resp = reply;
	return PAM_SUCCESS;

  conv_err:
	for (; count >= 0; count--)
		if (reply[count].resp)
			switch (msg[count]->msg_style) {
			case PAM_PROMPT_ECHO_ON:
			case PAM_PROMPT_ECHO_OFF: /* could wipe ... */
#ifdef PAM_BINARY_PROMPT
			case PAM_BINARY_PROMPT: /* ... that too ... */
#endif
				free( reply[count].resp );
				break;
			}
	free( reply );
	return PAM_CONV_ERR;
}

static int
PAM_conv_null( int num_msg,
               pam_message_type **msg,
               struct pam_response **resp,
               void *appdata_ptr ATTR_UNUSED )
{
	int count;
	struct pam_response *reply;

	if (!(reply = Calloc( num_msg, sizeof(*reply) )))
		return PAM_CONV_ERR;

	ReInitErrorLog();
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
			free( reply );
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
fail_delay( int retval ATTR_UNUSED, unsigned usec_delay ATTR_UNUSED,
            void *appdata_ptr ATTR_UNUSED )
{}
# endif

static int
doPAMAuth( const char *psrv, struct pam_data *pdata )
{
	pam_gi_type pitem;
	struct pam_conv pconv;
	int pretc;

	pdata->abort = FALSE;
	pconv.conv = PAM_conv;
	pconv.appdata_ptr = (void *)pdata;
	Debug( " PAM service %s\n", psrv );
	if ((pretc = pam_start( psrv, curuser, &pconv, &pamh )) != PAM_SUCCESS)
		goto pam_bail2;
	if ((pretc = pam_set_item( pamh, PAM_TTY, td->name )) != PAM_SUCCESS) {
	  pam_bail:
		pam_end( pamh, pretc );
		pamh = 0;
	  pam_bail2:
		ReInitErrorLog();
		LogError( "PAM error: %s\n", pam_strerror( 0, pretc ) );
		V_RET_FAIL( 0 );
	}
	if ((td->displayType & d_location) == dForeign) {
		char *cp = strchr( td->name, ':' );
		*cp = 0;
		pretc = pam_set_item( pamh, PAM_RHOST, td->name );
		*cp = ':';
		if (pretc != PAM_SUCCESS)
			goto pam_bail;
	}
# ifdef PAM_FAIL_DELAY
	pam_set_item( pamh, PAM_FAIL_DELAY, (void *)fail_delay );
# endif
	ReInitErrorLog();

	Debug( " pam_authenticate() ...\n" );
	pretc = pam_authenticate( pamh,
	                          td->allowNullPasswd ? 0 : PAM_DISALLOW_NULL_AUTHTOK );
	ReInitErrorLog();
	Debug( " pam_authenticate() returned: %s\n", pam_strerror( pamh, pretc ) );
	if (pdata->abort) {
		pam_end( pamh, PAM_SUCCESS );
		pamh = 0;
		return 0;
	}
	if (!curuser) {
		Debug( " asking PAM for user ...\n" );
		pam_get_item( pamh, PAM_USER, &pitem );
		ReInitErrorLog();
		StrDup( &curuser, (const char *)pitem );
		GSendInt( V_PUT_USER );
		GSendStr( curuser );
	}
	if (pretc != PAM_SUCCESS) {
		switch (pretc) {
		case PAM_USER_UNKNOWN:
		case PAM_AUTH_ERR:
		case PAM_MAXTRIES: /* should handle this better ... */
		case PAM_AUTHINFO_UNAVAIL: /* returned for unknown users ... bogus */
			pam_end( pamh, pretc );
			pamh = 0;
			V_RET_AUTH;
		default:
			pam_end( pamh, pretc );
			pamh = 0;
			V_RET_FAIL( 0 );
		}
	}
	return 1;
}

#endif /* USE_PAM */

static int
#if defined(USE_PAM) || defined(_AIX)
AccNoPass( const char *un )
{
	struct passwd *pw = 0;
#else
AccNoPass( const char *un, struct passwd *pw )
{
#endif
	struct group *gr;
	char **fp;
	int hg;

	if (!*un)
		return 0;

	if (cursource != PWSRC_MANUAL)
		return 1;

	for (hg = 0, fp = td->noPassUsers; *fp; fp++)
		if (**fp == '@')
			hg = 1;
		else if (!strcmp( un, *fp ))
			return 1;
		else if (!strcmp( "*", *fp )) {
#if defined(USE_PAM) || defined(_AIX)
			if (!(pw = getpwnam( un )))
				return 0;
#endif
			if (pw->pw_uid)
				return 1;
		}

#if defined(USE_PAM) || defined(_AIX)
	if (hg && (pw || (pw = getpwnam( un )))) {
#else
	if (hg) {
#endif
		for (setgrent(); (gr = getgrent()); )
			for (fp = td->noPassUsers; *fp; fp++)
				if (**fp == '@' && !strcmp( gr->gr_name, *fp + 1 )) {
					if (pw->pw_gid == gr->gr_gid) {
						endgrent();
						return 1;
					}
					for (; *gr->gr_mem; gr->gr_mem++)
						if (!strcmp( un, *gr->gr_mem )) {
							endgrent();
							return 1;
						}
				}
		endgrent();
	}

	return 0;
}

#if !defined(USE_PAM) && !defined(_AIX) && defined(USE_LOGIN_CAP)
# define LC_RET0 do { login_close(lc); return 0; } while(0)
#else
# define LC_RET0 return 0
#endif

int
Verify( GConvFunc gconv, int rootok )
{
#ifdef USE_PAM
	const char *psrv;
	struct pam_data pdata;
	int pretc, pnopass;
	char psrvb[64];
#elif defined(_AIX)
	char *msg, *curret;
	int i, reenter;
#else
	struct stat st;
	const char *nolg;
	char *buf;
	int fd;
# ifdef HAVE_GETUSERSHELL
	char *s;
# endif
# if defined(HAVE_STRUCT_PASSWD_PW_EXPIRE) || defined(USESHADOW)
	int tim, expir, warntime, quietlog;
# endif
# ifdef USE_LOGIN_CAP
#  ifdef HAVE_LOGIN_GETCLASS
	login_cap_t *lc;
#  else
	struct login_cap *lc;
#  endif
# endif
#endif

	Debug( "Verify ...\n" );

#ifdef USE_PAM

	pnopass = FALSE;
	if (!strcmp( curtype, "classic" )) {
		if (!gconv( GCONV_USER, 0 ))
			return 0;
		if (AccNoPass( curuser )) {
			gconv( GCONV_PASS_ND, 0 );
			if (!*curpass) {
				pnopass = TRUE;
				sprintf( psrvb, "%.31s-np", PAMService );
				psrv = psrvb;
			} else
				psrv = PAMService;
		} else
			psrv = PAMService;
		pdata.usecur = TRUE;
	} else {
		sprintf( psrvb, "%.31s-%.31s", PAMService, curtype );
		psrv = psrvb;
		pdata.usecur = FALSE;
	}
	pdata.gconv = gconv;
	if (!doPAMAuth( psrv, &pdata ))
		return 0;

#elif defined(_AIX)

	if ((td->displayType & d_location) == dForeign) {
		char *tmpch;
		strncpy( hostname, td->name, sizeof(hostname) - 1 );
		hostname[sizeof(hostname)-1] = '\0';
		if ((tmpch = strchr( hostname, ':' )))
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
	memcpy( tty, "/dev/xdm/", 9 );
	for (i = 0; i < 6 && td->name[i]; i++) {
		if (td->name[i] == ':' || td->name[i] == '.')
			tty[9 + i] = '_';
		else
			tty[9 + i] = td->name[i];
	}
	tty[9 + i] = '\0';
# endif

	if (!strcmp( curtype, "classic" )) {
		if (!gconv( GCONV_USER, 0 ))
			return 0;
		if (AccNoPass( curuser )) {
			gconv( GCONV_PASS_ND, 0 );
			if (!*curpass) {
				Debug( "accepting despite empty password\n" );
				goto done;
			}
		} else
			if (!gconv( GCONV_PASS, 0 ))
				return 0;
		enduserdb();
		msg = NULL;
		if ((i = authenticate( curuser, curpass, &reenter, &msg ))) {
			Debug( "authenticate() failed: %s\n", msg );
			if (msg)
				free( msg );
			loginfailed( curuser, hostname, tty );
			if (i == ENOENT || i == ESAD)
				V_RET_AUTH;
			else
				V_RET_FAIL( 0 );
		}
		if (reenter) {
			LogError( "authenticate() requests more data: %s\n", msg );
			free( msg );
			V_RET_FAIL( 0 );
		}
	} else if (!strcmp( curtype, "generic" )) {
		if (!gconv( GCONV_USER, 0 ))
			return 0;
		for (curret = 0;;) {
			msg = NULL;
			if ((i = authenticate( curuser, curret, &reenter, &msg ))) {
				Debug( "authenticate() failed: %s\n", msg );
				if (msg)
					free( msg );
				loginfailed( curuser, hostname, tty );
				if (i == ENOENT || i == ESAD)
					V_RET_AUTH;
				else
					V_RET_FAIL( 0 );
			}
			if (curret)
				free( curret );
			if (!reenter)
				break;
			if (!(curret = gconv( GCONV_HIDDEN, msg )))
				return 0;
			free( msg );
		}
	} else {
		LogError( "Unsupported authentication type %\"s requested\n", curtype );
		V_RET_FAIL( 0 );
	}
	if (msg) {
		PrepErrorGreet();
		GSendInt( V_MSG_INFO );
		GSendStr( msg );
		free( msg );
	}

  done:

#else

	if (strcmp( curtype, "classic" )) {
		LogError( "Unsupported authentication type %\"s requested\n", curtype );
		V_RET_FAIL( 0 );
	}

	if (!gconv( GCONV_USER, 0 ))
		return 0;

	if (!(p = getpwnam( curuser ))) {
		Debug( "getpwnam() failed.\n" );
		V_RET_AUTH;
	}
# ifdef __linux__ /* only Linux? */
	if (p->pw_passwd[0] == '!' || p->pw_passwd[0] == '*') {
		Debug( "account is locked\n" );
		V_RET_AUTH;
	}
# endif

# ifdef USESHADOW
	if ((sp = getspnam( curuser )))
		p->pw_passwd = sp->sp_pwdp;
	else
		Debug( "getspnam() failed: %m. Are you root?\n" );
# endif

	if (!*p->pw_passwd) {
		if (!td->allowNullPasswd) {
			Debug( "denying user with empty password\n" );
			V_RET_AUTH;
		}
		goto nplogin;
	}

	if (AccNoPass( curuser, p )) {
	  nplogin:
		gconv( GCONV_PASS_ND, 0 );
		if (!*curpass) {
			Debug( "accepting password-less login\n" );
			goto done;
		}
	} else
		if (!gconv( GCONV_PASS, 0 ))
			return 0;

# ifdef KERBEROS
	if (p->pw_uid) {
		int ret;
		char realm[REALM_SZ];

		if (krb_get_lrealm( realm, 1 )) {
			LogError( "Can't get KerberosIV realm.\n" );
			V_RET_FAIL( 0 );
		}

		sprintf( krbtkfile, "%s.%.*s", TKT_ROOT, MAXPATHLEN - strlen( TKT_ROOT ) - 2, td->name );
		krb_set_tkt_string( krbtkfile );
		unlink( krbtkfile );

		ret = krb_verify_user( curuser, "", realm, curpass, 1, "rcmd" );
		if (ret == KSUCCESS) {
			chown( krbtkfile, p->pw_uid, p->pw_gid );
			Debug( "KerberosIV verify succeeded\n" );
			goto done;
		} else if (ret != KDC_PR_UNKNOWN && ret != SKDC_CANT) {
			LogError( "KerberosIV verification failure %\"s for %s\n",
			          krb_get_err_text( ret ), curuser );
			krbtkfile[0] = '\0';
			V_RET_FAIL( 0 );
		}
		Debug( "KerberosIV verify failed: %s\n", krb_get_err_text( ret ) );
	}
	krbtkfile[0] = '\0';
# endif	 /* KERBEROS */

# if defined(ultrix) || defined(__ultrix__)
	if (authenticate_user( p, curpass, NULL ) < 0)
# elif defined(HAVE_CRYPT)
	if (strcmp( crypt( curpass, p->pw_passwd ), p->pw_passwd ))
# else
	if (strcmp( curpass, p->pw_passwd ))
# endif
	{
		Debug( "password verify failed\n" );
		V_RET_AUTH;
	}

  done:

#endif /* !defined(USE_PAM) && !defined(_AIX) */

	Debug( "restrict %s ...\n", curuser );

#if defined(USE_PAM) || defined(_AIX)
	if (!(p = getpwnam( curuser ))) {
		LogError( "getpwnam(%s) failed.\n", curuser );
		V_RET_FAIL( 0 );
	}
#endif
	if (!p->pw_uid) {
		if (!rootok && !td->allowRootLogin)
			V_RET_FAIL( "Root logins are not allowed" );
		return 1; /* don't deny root to log in */
	}

#ifdef USE_PAM

	Debug( " pam_acct_mgmt() ...\n" );
	pretc = pam_acct_mgmt( pamh, 0 );
	ReInitErrorLog();
	Debug( " pam_acct_mgmt() returned: %s\n", pam_strerror( pamh, pretc ) );
	if (pretc == PAM_NEW_AUTHTOK_REQD) {
		pdata.usecur = FALSE;
		pdata.gconv = conv_interact;
		/* pam will have output a message already, so no PrepErrorGreet () */
		if (gconv != conv_interact || pnopass) {
			pam_end( pamh, PAM_SUCCESS );
			pamh = 0;
			GSendInt( V_CHTOK_AUTH );
			/* this cannot auth the wrong user, as only classic auths get here */
			while (!doPAMAuth( PAMService, &pdata ))
				if (pdata.abort)
					return 0;
			GSendInt( V_PRE_OK );
		} else
			GSendInt( V_CHTOK );
		for (;;) {
			Debug( " pam_chauthtok() ...\n" );
			pretc = pam_chauthtok( pamh, PAM_CHANGE_EXPIRED_AUTHTOK );
			ReInitErrorLog();
			Debug( " pam_chauthtok() returned: %s\n", pam_strerror( pamh, pretc ) );
			if (pdata.abort) {
				pam_end( pamh, PAM_SUCCESS );
				pamh = 0;
				return 0;
			}
			if (pretc == PAM_SUCCESS)
				break;
			/* effectively there is only PAM_AUTHTOK_ERR */
			GSendInt( V_FAIL );
		}
		if (curpass)
			free( curpass );
		curpass = newpass;
		newpass = 0;
	} else if (pretc != PAM_SUCCESS) {
		pam_end( pamh, pretc );
		pamh = 0;
		V_RET_AUTH;
	}

#elif defined(_AIX) /* USE_PAM */

	msg = NULL;
	if (loginrestrictions( curuser,
	                       ((td->displayType & d_location) == dForeign) ? S_RLOGIN : S_LOGIN,
	                       tty, &msg ) == -1)
	{
		Debug( "loginrestrictions() - %s\n", msg ? msg : "error" );
		loginfailed( curuser, hostname, tty );
		PrepErrorGreet();
		if (msg) {
			GSendInt( V_MSG_ERR );
			GSendStr( msg );
		}
		GSendInt( V_AUTH );
		return 0;
	}
	if (msg)
		free( (void *)msg );

#else /* USE_PAM || _AIX */

# ifdef HAVE_GETUSERSHELL
	for (;;) {
		if (!(s = getusershell())) {
			Debug( "shell not in /etc/shells\n" );
			endusershell();
			V_RET_FAIL( "Your login shell is not listed in /etc/shells" );
		}
		if (!strcmp( s, p->pw_shell )) {
			endusershell();
			break;
		}
	}
# endif

# ifdef USE_LOGIN_CAP
#  ifdef HAVE_LOGIN_GETCLASS
	lc = login_getclass( p->pw_class );
#  else
	lc = login_getpwclass( p );
#  endif
	if (!lc)
		V_RET_FAIL( 0 );
# endif


/* restrict_nologin */
# ifndef _PATH_NOLOGIN
#  define _PATH_NOLOGIN "/etc/nologin"
# endif

	if ((
# ifdef USE_LOGIN_CAP
	     /* Do we ignore a nologin file? */
	     !login_getcapbool( lc, "ignorenologin", 0 )) &&
	    (!stat( (nolg = login_getcapstr( lc, "nologin", "", NULL )), &st ) ||
# endif
		 !stat( (nolg = _PATH_NOLOGIN), &st )))
	{
		PrepErrorGreet();
		GSendInt( V_MSG_ERR );
		if (st.st_size && (fd = open( nolg, O_RDONLY )) >= 0) {
			if ((buf = Malloc( st.st_size + 1 ))) {
				if (read( fd, buf, st.st_size ) == st.st_size) {
					buf[st.st_size] = 0;
					GSendStr( buf );
					free( buf );
					close( fd );
					GSendInt( V_FAIL );
					LC_RET0;
				}
				free( buf );
			}
			close( fd );
		}
		GSendStr( "Logins are not allowed at the moment.\nTry again later" );
		GSendInt( V_FAIL );
		LC_RET0;
	}


/* restrict_nohome */
# ifdef USE_LOGIN_CAP
	if (login_getcapbool( lc, "requirehome", 0 )) {
		struct stat st;
		if (!*p->pw_dir || stat( p->pw_dir, &st ) || st.st_uid != p->pw_uid) {
			PrepErrorGreet();
			GSendInt( V_MSG_ERR );
			GSendStr( "Home folder not available" );
			GSendInt( V_FAIL );
			LC_RET0;
		}
	}
# endif


/* restrict_time */
# ifdef USE_LOGIN_CAP
#  ifdef HAVE_AUTH_TIMEOK
	if (!auth_timeok( lc, time( NULL ) )) {
		PrepErrorGreet();
		GSendInt( V_MSG_ERR );
		GSendStr( "You are not allowed to login at the moment" );
		GSendInt( V_FAIL );
		LC_RET0;
	}
#  endif
# endif


/* restrict_expired; this MUST be the last one */
# if defined(HAVE_STRUCT_PASSWD_PW_EXPIRE) || defined(USESHADOW)

#  if !defined(HAVE_STRUCT_PASSWD_PW_EXPIRE) || (!defined(USE_LOGIN_CAP) && defined(USESHADOW))
	if (sp)
#  endif
	{

#  define DEFAULT_WARN	(2L * 7L)  /* Two weeks */

		tim = time( NULL ) / 86400L;

#  ifdef USE_LOGIN_CAP
		quietlog = login_getcapbool( lc, "hushlogin", 0 );
		warntime = login_getcaptime( lc, "warnexpire",
		                             DEFAULT_WARN * 86400L,
		                             DEFAULT_WARN * 86400L ) / 86400L;
#  else
		quietlog = 0;
#	ifdef USESHADOW
		warntime = sp->sp_warn != -1 ? sp->sp_warn : DEFAULT_WARN;
#	else
		warntime = DEFAULT_WARN;
#	endif
#  endif

#  ifdef HAVE_STRUCT_PASSWD_PW_EXPIRE
		if (p->pw_expire) {
			expir = p->pw_expire / 86400L;
#  else
		if (sp->sp_expire != -1) {
			expir = sp->sp_expire;
#  endif
			if (tim > expir) {
				PrepErrorGreet();
				GSendInt( V_MSG_ERR );
				GSendStr( "Your account has expired;"
				          " please contact your system administrator" );
				GSendInt( V_FAIL );
				LC_RET0;
			} else if (tim > (expir - warntime) && !quietlog) {
				ASPrintf( &buf,
				          "Warning: your account will expire in %d day(s)",
				          expir - tim );
				if (buf) {
					PrepErrorGreet();
					GSendInt( V_MSG_INFO );
					GSendStr( buf );
					free( buf );
				}
			}
		}

#  ifdef HAVE_STRUCT_PASSWD_PW_EXPIRE
		if (p->pw_change) {
			expir = p->pw_change / 86400L;
#  else
		if (!sp->sp_lstchg) {
			PrepErrorGreet();
			GSendInt( V_MSG_ERR );
			GSendStr( "You are required to change your password immediately"
			          " (root enforced)" );
			/* XXX todo password change */
			GSendInt( V_FAIL );
			LC_RET0;
		} else if (sp->sp_max != -1) {
			expir = sp->sp_lstchg + sp->sp_max;
			if (sp->sp_inact != -1 && tim > expir + sp->sp_inact) {
				PrepErrorGreet();
				GSendInt( V_MSG_ERR );
				GSendStr( "Your account has expired;"
				          " please contact your system administrator" );
				GSendInt( V_FAIL );
				LC_RET0;
			}
#  endif
			if (tim > expir) {
				PrepErrorGreet();
				GSendInt( V_MSG_ERR );
				GSendStr( "You are required to change your password immediately"
				          " (password aged)" );
				/* XXX todo password change */
				GSendInt( V_FAIL );
				LC_RET0;
			} else if (tim > (expir - warntime) && !quietlog) {
				ASPrintf( &buf,
				          "Warning: your password will expire in %d day(s)",
				          expir - tim );
				if (buf) {
					PrepErrorGreet();
					GSendInt( V_MSG_INFO );
					GSendStr( buf );
					free( buf );
				}
			}
		}

	}

# endif /* HAVE_STRUCT_PASSWD_PW_EXPIRE || USESHADOW */

# ifdef USE_LOGIN_CAP
	login_close( lc );
# endif

#endif /* USE_PAM || _AIX */

	return 1;

}


static const char *envvars[] = {
	"TZ", /* SYSV and SVR4, but never hurts */
#ifdef _AIX
	"AUTHSTATE", /* for kerberos */
#endif
	NULL
};


#if defined(USE_PAM) && defined(HAVE_INITGROUPS)
int num_saved_gids;
gid_t saved_gids[NGROUPS];

static int
saveGids( void )
{
	if ((num_saved_gids = getgroups( as(saved_gids), saved_gids )) < 0) {
		LogError( "saving groups failed: %m\n" );
		return 0;
	}
	return 1;
}

static int
restoreGids( void )
{
	if (setgroups( num_saved_gids, saved_gids ) < 0) {
		LogError( "restoring groups failed: %m\n" );
		return 0;
	}
	if (setgid( p->pw_gid ) < 0) {
		LogError( "restoring gid failed: %m\n" );
		return 0;
	}
	return 1;
}
#endif /* USE_PAM && HAVE_INITGROUPS */

static int
resetGids( void )
{
#ifdef HAVE_INITGROUPS
	if (setgroups( 0, &p->pw_gid /* anything */ ) < 0) {
		LogError( "restoring groups failed: %m\n" );
		return 0;
	}
#endif
	if (setgid( 0 ) < 0) {
		LogError( "restoring gid failed: %m\n" );
		return 0;
	}
	return 1;
}

static int
SetGid( const char *name, int gid )
{
	if (setgid( gid ) < 0) {
		LogError( "setgid(%d) (user %s) failed: %m\n", gid, name );
		return 0;
	}
#ifdef HAVE_INITGROUPS
	if (initgroups( name, gid ) < 0) {
		LogError( "initgroups for %s failed: %m\n", name );
		setgid( 0 );
		return 0;
	}
#endif	 /* QNX4 doesn't support multi-groups, no initgroups() */
	return 1;
}

static int
SetUid( const char *name, int uid )
{
	if (setuid( uid ) < 0) {
		LogError( "setuid(%d) (user %s) failed: %m\n", uid, name );
		return 0;
	}
	return 1;
}

static int
SetUser( const char *name, int uid, int gid )
{
	if (SetGid( name, gid )) {
		if (SetUid( name, uid ))
			return 1;
		resetGids();
	}
	return 0;
}

#if defined(SECURE_RPC) || defined(K5AUTH)
static void
NukeAuth( int len, const char *name )
{
	int i;

	for (i = 0; i < td->authNum; i++)
		if (td->authorizations[i]->name_length == len &&
		    !memcmp( td->authorizations[i]->name, name, len ))
		{
			memcpy( &td->authorizations[i], &td->authorizations[i+1],
			        sizeof(td->authorizations[i]) * (--td->authNum - i) );
			break;
		}
}
#endif

static void
mergeSessionArgs( int cansave )
{
	char *mfname;
	const char *fname;
	int i, needsave;

	mfname = 0;
	fname = ".dmrc";
	if ((!curdmrc || newdmrc) && *dmrcDir)
		if (StrApp( &mfname, dmrcDir, "/", curuser, fname, (char *)0 ))
			fname = mfname;
	needsave = 0;
	if (!curdmrc) {
		curdmrc = iniLoad( fname );
		if (!curdmrc) {
			StrDup( &curdmrc, "[Desktop]\nSession=default\n" );
			needsave = 1;
		}
	}
	if (newdmrc) {
		curdmrc = iniMerge( curdmrc, newdmrc );
		needsave = 1;
	}
	if (needsave && cansave)
		if (!iniSave( curdmrc, fname ) && errno == ENOENT && mfname) {
			for (i = 0; mfname[i]; i++)
				if (mfname[i] == '/') {
					mfname[i] = 0;
					mkdir( mfname, 0755 );
					mfname[i] = '/';
				}
			iniSave( curdmrc, mfname );
		}
	if (mfname)
		free( mfname );
}

static int removeAuth;
#ifdef USE_PAM
static int removeSession;
static int removeCreds;
#endif

int
StartClient()
{
	const char *home, *sessargs, *desksess;
	char **env, *xma;
	char **argv, *fname, *str;
#ifdef USE_PAM
	char ** volatile pam_env;
# ifndef HAVE_PAM_GETENVLIST
	char **saved_env;
# endif
	struct pam_conv pconv;
	int pretc;
#else
# ifdef _AIX
	char *msg;
	char **theenv;
	extern char **newenv; /* from libs.a, this is set up by setpenv */
# endif
#endif
#ifdef HAVE_SETUSERCONTEXT
	extern char **environ;
#endif
	char *failsafeArgv[2], *lname;
	int i, pid, lfd;

	if (StrCmp( dmrcuser, curuser )) {
		if (curdmrc) { free( curdmrc ); curdmrc = 0; }
		if (dmrcuser) { free( dmrcuser ); dmrcuser = 0; }
	}

#if defined(USE_PAM) || defined(_AIX)
	if (!(p = getpwnam( curuser ))) {
		LogError( "getpwnam(%s) failed.\n", curuser );
		return 0;
	}
#endif

#ifndef USE_PAM
# ifdef _AIX
	msg = NULL;
	loginsuccess( curuser, hostname, tty, &msg );
	if (msg) {
		Debug( "loginsuccess() - %s\n", msg );
		free( (void *)msg );
	}
# else /* _AIX */
#  if defined(KERBEROS) && defined(AFS)
	if (krbtkfile[0] != '\0') {
		if (k_hasafs()) {
			if (k_setpag() == -1)
				LogError( "setpag() for %s failed\n", curuser );
			if ((ret = k_afsklog( NULL, NULL )) != KSUCCESS)
				LogError( "AFS Warning: %s\n", krb_get_err_text( ret ) );
		}
	}
#  endif /* KERBEROS && AFS */
# endif /* _AIX */
#endif	/* !PAM */

	curuid = p->pw_uid;
	curgid = p->pw_gid;

	env = baseEnv( curuser );
	xma = 0;
	if (td->ctrl.fpath && StrDup( &xma, td->ctrl.fpath )) {
		if ((td->allowShutdown == SHUT_ALL ||
		     (td->allowShutdown == SHUT_ROOT && !curuser)) &&
		    StrApp( &xma, ",maysd", (char *)0 ))
		{
			if (td->allowNuke == SHUT_ALL ||
			    (td->allowNuke == SHUT_ROOT && !curuser))
				StrApp( &xma, ",mayfn", (char *)0 );
			StrApp( &xma, td->defSdMode == SHUT_FORCENOW ? ",fn" :
			        td->defSdMode == SHUT_TRYNOW ? ",tn" : ",sched",
			        (char *)0 );
		}
		if ((td->displayType & d_location) == dLocal && AnyReserveDisplays())
			StrApp( &xma, ",rsvd", (char *)0 );
	} else
		StrDup( &xma, "true" );
	StrApp( &xma, ",method=", curtype, (char *)0 );
	if (td_setup)
		StrApp( &xma, ",auto", (char *)0 );
	if (xma) {
		env = setEnv( env, "XDM_MANAGED", xma );
		free( xma );
	}
	if (td->autoLock && cursource == PWSRC_AUTOLOGIN)
		env = setEnv( env, "DESKTOP_LOCKED", "true" );
	env = setEnv( env, "PATH", curuid ? td->userPath : td->systemPath );
	env = setEnv( env, "SHELL", p->pw_shell );
	env = setEnv( env, "HOME", p->pw_dir );
#if !defined(USE_PAM) && !defined(_AIX) && defined(KERBEROS)
	if (krbtkfile[0] != '\0')
		env = setEnv( env, "KRBTKFILE", krbtkfile );
#endif
	userEnviron = inheritEnv( env, envvars );
	env = systemEnv( curuser );
	systemEnviron = setEnv( env, "HOME", p->pw_dir );
	Debug( "user environment:\n%[|''>'\n's"
	       "system environment:\n%[|''>'\n's"
	       "end of environments\n",
	       userEnviron,
	       systemEnviron );

	/*
	 * for user-based authorization schemes,
	 * add the user to the server's allowed "hosts" list.
	 */
	for (i = 0; i < td->authNum; i++) {
#ifdef SECURE_RPC
		if (td->authorizations[i]->name_length == 9 &&
		    !memcmp( td->authorizations[i]->name, "SUN-DES-1", 9 ))
		{
			XHostAddress addr;
			char netname[MAXNETNAMELEN+1];
			char domainname[MAXNETNAMELEN+1];

			getdomainname( domainname, sizeof(domainname) );
			user2netname( netname, curuid, domainname );
			addr.family = FamilyNetname;
			addr.length = strlen( netname );
			addr.address = netname;
			XAddHost( dpy, &addr );
		}
#endif
#ifdef K5AUTH
		if (td->authorizations[i]->name_length == 14 &&
		    !memcmp( td->authorizations[i]->name, "MIT-KERBEROS-5", 14 ))
		{
			/* Update server's auth file with user-specific info.
			 * Don't need to AddHost because X server will do that
			 * automatically when it reads the cache we are about
			 * to point it at.
			 */
			XauDisposeAuth( td->authorizations[i] );
			td->authorizations[i] =
				Krb5GetAuthFor( 14, "MIT-KERBEROS-5", td->name );
			SaveServerAuthorizations( td, td->authorizations, td->authNum );
		}
#endif
	}

	if (*dmrcDir)
		mergeSessionArgs( TRUE );

	Debug( "now starting the session\n" );

#ifdef USE_PAM
	/* the greeter is gone by now ... */
	pconv.conv = PAM_conv_null;
	pconv.appdata_ptr = 0;
	if ((pretc = pam_set_item( pamh, PAM_CONV, &pconv )) != PAM_SUCCESS) {
		ReInitErrorLog();
		LogError( "pam_set_item() for %s failed: %s\n",
		          curuser, pam_strerror( pamh, pretc ) );
		return 0;
	}
	ReInitErrorLog();
#endif

#ifdef USE_PAM

# ifdef HAVE_SETUSERCONTEXT
	if (setusercontext( lc, p, p->pw_uid, LOGIN_SETGROUP )) {
		LogError( "setusercontext(groups) for %s failed: %m\n",
		          curuser );
		return 0;
	}
# else
	if (!SetGid( curuser, curgid ))
		return 0;
# endif

# ifndef HAVE_PAM_GETENVLIST
	if (!(pam_env = initStrArr( 0 ))) {
		resetGids();
		return 0;
	}
	saved_env = environ;
	environ = pam_env;
# endif
	removeCreds = 1; /* set it first - i don't trust PAM's rollback */
	pretc = pam_setcred( pamh, 0 );
	ReInitErrorLog();
# ifndef HAVE_PAM_GETENVLIST
	pam_env = environ;
	environ = saved_env;
# endif
# ifdef HAVE_INITGROUPS
	/* This seems to be a strange place for it, but do it:
	   - after the initial groups are set
	   - after pam_setcred might have set something, even in the error case
	   - before pam_setcred(DELETE_CRED) might need it
	 */
	if (!saveGids())
		return 0;
# endif
	if (pretc != PAM_SUCCESS) {
		LogError( "pam_setcred() for %s failed: %s\n",
		          curuser, pam_strerror( pamh, pretc ) );
		resetGids();
		return 0;
	}

	removeSession = 1; /* set it first - same as above */
	pretc = pam_open_session( pamh, 0 );
	ReInitErrorLog();
	if (pretc != PAM_SUCCESS) {
		LogError( "pam_open_session() for %s failed: %s\n",
		          curuser, pam_strerror( pamh, pretc ) );
		resetGids();
		return 0;
	}

	/* we don't want sessreg and the startup/reset scripts run with user
	   credentials. unfortunately, we can reset only the gids. */
	resetGids();

# define D_LOGIN_SETGROUPS LOGIN_SETGROUPS
#else /* USE_PAM */
# define D_LOGIN_SETGROUPS 0
#endif /* USE_PAM */

	removeAuth = 1;
	chownCtrl( &td->ctrl, curuid );
	endpwent();
#if !defined(USE_PAM) && defined(USESHADOW) && !defined(_AIX)
	endspent();
#endif
	ClearCloseOnFork( mstrtalk.pipe->wfd );
	switch (pid = Fork()) {
	case 0:

		sessreg( td, getpid(), curuser, curuid );

		if (source( systemEnviron, td->startup, td_setup )) {
			LogError( "Cannot execute startup script %\"s\n", td->startup );
			exit( 1 );
		}

		if (Setjmp( mstrtalk.errjmp ))
			exit( 1 );
		GSet( &mstrtalk );

		setsid();

	/* Memory leaks are ok here as we exec() soon. */

#if defined(USE_PAM) || !defined(_AIX)

# ifdef USE_PAM
		/* pass in environment variables set by libpam and modules it called */
#  ifdef HAVE_PAM_GETENVLIST
		pam_env = pam_getenvlist( pamh );
		ReInitErrorLog();
#  endif
		if (pam_env)
			for (; *pam_env; pam_env++)
				userEnviron = putEnv( *pam_env, userEnviron );
# endif

# ifdef HAVE_SETLOGIN
		if (setlogin( curuser ) < 0) {
			LogError( "setlogin for %s failed: %m\n", curuser );
			exit( 1 );
		}
#  define D_LOGIN_SETLOGIN LOGIN_SETLOGIN
# else
#  define D_LOGIN_SETLOGIN 0
# endif

# if defined(USE_PAM) && defined(HAVE_INITGROUPS)
		if (!restoreGids())
			exit( 1 );
# endif

# ifndef HAVE_SETUSERCONTEXT

#  ifdef USE_PAM
		if (!SetUid( curuser, curuid ))
			exit( 1 );
#  else
		if (!SetUser( curuser, curuid, curgid ))
			exit( 1 );
#  endif

# else /* !HAVE_SETUSERCONTEXT */

		/*
		 * Destroy environment.
		 * We need to do this before setusercontext() because that may
		 * set or reset some environment variables.
		 */
		if (!(environ = initStrArr( 0 )))
			exit( 1 );

		/*
		 * Set the user's credentials: uid, gid, groups,
		 * environment variables, resource limits, and umask.
		 */
		if (setusercontext( lc, p, p->pw_uid,
		        LOGIN_SETALL & ~(D_LOGIN_SETGROUPS|D_LOGIN_SETLOGIN) ) < 0)
		{
			LogError( "setusercontext for %s failed: %m\n", curuser );
			exit( 1 );
		}

		for (i = 0; environ[i]; i++)
			userEnviron = putEnv( environ[i], userEnviron );

# endif /* !HAVE_SETUSERCONTEXT */

#else /* PAM || !_AIX */
		/*
		 * Set the user's credentials: uid, gid, groups,
		 * audit classes, user limits, and umask.
		 */
		if (setpcred( curuser, NULL ) == -1) {
			LogError( "setpcred for %s failed: %m\n", curuser );
			exit( 1 );
		}

		/*
		 * Set the users process environment. Store protected variables and
		 * obtain updated user environment list. This call will initialize
		 * global 'newenv'.
		 */
		if (setpenv( curuser, PENV_INIT | PENV_ARGV | PENV_NOEXEC,
		             userEnviron, NULL ) != 0)
		{
			LogError( "Can't set %s's process environment\n", curuser );
			exit( 1 );
		}
		userEnviron = newenv;

#endif /* _AIX */

		/*
		 * for user-based authorization schemes,
		 * use the password to get the user's credentials.
		 */
#ifdef SECURE_RPC
		/* do like "keylogin" program */
		if (!curpass[0])
			LogInfo( "No password for NIS provided.\n" );
		else {
			char netname[MAXNETNAMELEN+1], secretkey[HEXKEYBYTES+1];
			int nameret, keyret;
			int len;
			int key_set_ok = 0;
			struct key_netstarg netst;

			nameret = getnetname( netname );
			Debug( "user netname: %s\n", netname );
			len = strlen( curpass );
			if (len > 8)
				bzero( curpass + 8, len - 8 );
			keyret = getsecretkey( netname, secretkey, curpass );
			Debug( "getsecretkey returns %d, key length %d\n",
			       keyret, strlen( secretkey ) );
			netst.st_netname = netname;
			memcpy( netst.st_priv_key, secretkey, HEXKEYBYTES );
			memset( netst.st_pub_key, 0, HEXKEYBYTES );
			if (key_setnet( &netst ) < 0)
				Debug( "Could not set secret key.\n" );
			/* is there a key, and do we have the right password? */
			if (keyret == 1) {
				if (*secretkey) {
					keyret = key_setsecret( secretkey );
					Debug( "key_setsecret returns %d\n", keyret );
					if (keyret == -1)
						LogError( "Failed to set NIS secret key\n" );
					else
						key_set_ok = 1;
				} else {
					/* found a key, but couldn't interpret it */
					LogError( "Password incorrect for NIS principal %s\n",
					          nameret ? netname : curuser );
				}
			}
			if (!key_set_ok)
				NukeAuth( 9, "SUN-DES-1" );
			bzero( secretkey, strlen( secretkey ) );
		}
#endif
#ifdef K5AUTH
		/* do like "kinit" program */
		if (!curpass[0])
			LogInfo( "No password for Kerberos5 provided.\n" );
		else
			if ((str = Krb5Init( curuser, curpass, td->name )))
				userEnviron = setEnv( userEnviron, "KRB5CCNAME", str );
			else
				NukeAuth( 14, "MIT-KERBEROS-5" );
#endif /* K5AUTH */
		if (td->autoReLogin) {
			GSendInt( D_ReLogin );
			GSendStr( curuser );
			GSendStr( curpass );
			GSendStr( newdmrc );
		}
		if (curpass)
			bzero( curpass, strlen( curpass ) );
		SetUserAuthorization( td );
		home = getEnv( userEnviron, "HOME" );
		if (home) {
			if (chdir( home ) < 0) {
				LogError( "Cannot chdir to %s's home %s: %m, using /\n",
				          curuser, home );
				home = 0;
				userEnviron = setEnv( userEnviron, "HOME", "/" );
				goto cdroot;
			}
			ASPrintf( &lname, td->clientLogFile, td->name );
			if ((lfd = creat( lname, 0600 )) < 0) {
				LogWarn( "Cannot create session log file %s: %m\n", lname );
				free( lname );
				goto tmperr;
			}
		} else {
		  cdroot:
			chdir( "/" );
		  tmperr:
			ASPrintf( &lname, "/tmp/xerr-%s-%s", curuser, td->name );
			unlink( lname );
			if ((lfd = open( lname, O_WRONLY|O_CREAT|O_EXCL, 0600 )) < 0) {
				LogError( "Cannot create fallback session log file %s: %m\n",
				          lname );
				goto logerr;
			}
		}
		dup2( lfd, 1 );
		dup2( lfd, 2 );
		close( lfd );
	  logerr:
		free( lname );
		if (!*dmrcDir)
			mergeSessionArgs( home != 0 );
		if (!(desksess = iniEntry( curdmrc, "Desktop", "Session", 0 )))
			desksess = "failsafe"; /* only due to OOM */
		GSendInt( D_User );
		GSendInt( curuid );
		GSendStr( curuser );
		GSendStr( desksess );
		close( mstrtalk.pipe->wfd );
		userEnviron = setEnv( userEnviron, "DESKTOP_SESSION", desksess );
		for (i = 0; td->sessionsDirs[i]; i++) {
			fname = 0;
			if (StrApp( &fname, td->sessionsDirs[i], "/", desksess, ".desktop", (char *)0 )) {
				if ((str = iniLoad( fname ))) {
					if (!StrCmp( iniEntry( str, "Desktop Entry", "Hidden", 0 ), "true" ) ||
					    !(sessargs = iniEntry( str, "Desktop Entry", "Exec", 0 )))
						sessargs = "";
					free( str );
					free( fname );
					goto gotit;
				}
				free( fname );
			}
		}
		if (!strcmp( desksess, "failsafe" ) ||
		    !strcmp( desksess, "default" ) ||
		    !strcmp( desksess, "custom" ))
			sessargs = desksess;
		else
			sessargs = "";
	  gotit:
		if (!(argv = parseArgs( (char **)0, td->session )) ||
		    !(argv = addStrArr( argv, sessargs, -1 )))
			exit( 1 );
		if (argv[0] && *argv[0]) {
			Debug( "executing session %\"[s\n", argv );
			execute( argv, userEnviron );
			LogError( "Session %\"s execution failed: %m\n", argv[0] );
		} else
			LogError( "Session has no command/arguments\n" );
		failsafeArgv[0] = td->failsafeClient;
		failsafeArgv[1] = 0;
		execute( failsafeArgv, userEnviron );
		LogError( "Failsafe client %\"s execution failed: %m\n",
		          failsafeArgv[0] );
		exit( 1 );
	case -1:
		RegisterCloseOnFork( mstrtalk.pipe->wfd );
		LogError( "Forking session on %s failed: %m\n", td->name );
		return 0;
	default:
		RegisterCloseOnFork( mstrtalk.pipe->wfd );
		Debug( "StartSession, fork succeeded %d\n", pid );
		return pid;
	}
}

void
SessionExit( int status )
{
	int pid;
#ifdef USE_PAM
	int pretc;
#endif

	if (removeAuth) {
		if (source( systemEnviron, td->reset, td_setup ))
			LogError( "Cannot execute reset script %\"s\n", td->reset );
		sessreg( td, 0, 0, 0 );

		switch ((pid = Fork())) {
		case 0:
#if defined(USE_PAM) && defined(HAVE_INITGROUPS)
			if (restoreGids() && SetUid( curuser, curuid ))
#else
			if (SetUser( curuser, curuid, curgid ))
#endif

			{
				RemoveUserAuthorization( td );
#ifdef K5AUTH
				Krb5Destroy( td->name );
#endif /* K5AUTH */
#if !defined(USE_PAM) && !defined(_AIX)
# ifdef KERBEROS
				if (krbtkfile[0]) {
					(void)dest_tkt();
#  ifdef AFS
					if (k_hasafs())
						(void)k_unlog();
#  endif
				}
# endif
#endif /* !USE_PAM && !_AIX*/
			}
			exit( 0 );
		case -1:
			LogError( "Cannot clean up session: fork() failed: %m" );
			break;
		default:
			Wait4( pid );
			break;
		}
	}

#ifdef USE_PAM
# ifdef HAVE_INITGROUPS
	restoreGids();
# endif
	if (removeSession)
		if ((pretc = pam_close_session( pamh, 0 )) != PAM_SUCCESS)
			LogError( "pam_close_session() failed: %s\n",
			          pam_strerror( pamh, pretc ) );
	if (removeCreds)
		if ((pretc = pam_setcred( pamh, PAM_DELETE_CRED )) != PAM_SUCCESS)
			LogError( "pam_setcred(DELETE_CRED) failed: %s\n",
			          pam_strerror( pamh, pretc ) );
	resetGids();
	if (pamh) {
		pam_end( pamh, PAM_SUCCESS );
		ReInitErrorLog();
	}
#endif

	/* make sure the server gets reset after the session is over */
	if (td->serverPid >= 2) {
		if (!td->terminateServer && td->resetSignal)
			TerminateProcess( td->serverPid, td->resetSignal );
	} else
		ResetServer( td );
	Debug( "display %s exiting with status %d\n", td->name, status );
	exit( status );
}

int
ReadDmrc()
{
	char *data, *fname = 0;
	int len, pid, pfd[2], err;

	if (!dmrcuser || !dmrcuser[0] || !(p = getpwnam( dmrcuser )))
		return GE_NoUser;

	if (*dmrcDir) {
		if (!StrApp( &fname, dmrcDir, "/", dmrcuser, ".dmrc", (char *)0 ))
			return GE_Error;
		if (!(curdmrc = iniLoad( fname ))) {
			free( fname );
			return GE_Ok;
		}
		free( fname );
		return GE_NoFile;
	}

	if (!StrApp( &fname, p->pw_dir, "/.dmrc", (char *)0 ))
		return GE_Error;
	if (pipe( pfd ))
		return GE_Error;
	if ((pid = Fork()) < 0) {
		close( pfd[0] );
		close( pfd[1] );
		return GE_Error;
	}
	if (!pid) {
		if (!SetUser( p->pw_name, p->pw_uid, p->pw_gid ))
			exit( 0 );
		if (!(data = iniLoad( fname ))) {
			static const int m1 = -1;
			write( pfd[1], &m1, sizeof(int) );
			exit( 0 );
		}
		len = strlen( data );
		write( pfd[1], &len, sizeof(int) );
		write( pfd[1], data, len + 1 );
		exit( 0 );
	}
	close( pfd[1] );
	free( fname );
	err = GE_Error;
	if (Reader( pfd[0], &len, sizeof(int) ) == sizeof(int)) {
		if (len == -1)
			err = GE_Denied;
		else if ((curdmrc = Malloc( len + 1 ))) {
			if (Reader( pfd[0], curdmrc, len + 1 ) == len + 1)
				err = GE_Ok;
			else {
				free( curdmrc );
				curdmrc = 0;
			}
		}
	}
	close( pfd[0] );
	(void)Wait4( pid );
	return err;
}
