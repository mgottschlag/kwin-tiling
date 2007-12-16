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
#ifdef HAVE_GETSPNAM
# include <shadow.h>
#endif
#include <signal.h>

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
#ifdef HAVE_SETUSERCONTEXT
# ifdef HAVE_LOGIN_GETCLASS
login_cap_t *lc;
# else
struct login_cap *lc;
# endif
#endif
#ifdef USE_PAM
static pam_handle_t *pamh;
static int inAuth;
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

static void
displayStr( int lv, const char *msg )
{
	prepareErrorGreet();
	gSendInt( lv );
	gSendStr( msg );
	gRecvInt();
}

#if !defined(USE_PAM) && (defined(HAVE_STRUCT_PASSWD_PW_EXPIRE) || defined(USESHADOW))
static void
displayMsg( int lv, const char *msg, ... )
{
	char *ae;
	va_list args;

	va_start( args, msg );
	VASPrintf( &ae, msg, args );
	va_end( args );
	if (ae) {
		displayStr( lv, ae );
		free( ae );
	}
}
#endif

#ifdef _AIX
# define _ENDUSERDB , enduserdb()
#else
# define _ENDUSERDB
#endif
#ifdef HAVE_GETSPNAM /* (sic!) - not USESHADOW */
# define _ENDSPENT , endspent()
#else
# define _ENDSPENT()
#endif
#define END_ENT endpwent() _ENDSPENT _ENDUSERDB

#define V_RET_NP \
		do { \
			END_ENT; \
			return False; \
		} while(0)

#define V_RET \
		do { \
			wipeStr( curpass ); \
			curpass = 0; \
			V_RET_NP; \
		} while(0)

#define V_RET_AUTH \
		do { \
			prepareErrorGreet(); \
			gSendInt( V_AUTH ); \
			V_RET; \
		} while(0)

#define V_RET_FAIL(m) \
		do { \
			displayStr( V_MSG_ERR, m ); \
			gSendInt( V_FAIL ); \
			V_RET; \
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

	reInitErrorLog();
	debug( "PAM_conv\n" );
	for (count = 0; count < num_msg; count++)
		switch (msg[count]->msg_style) {
		case PAM_TEXT_INFO:
			debug( " PAM_TEXT_INFO: %s\n", msg[count]->msg );
			displayStr( inAuth ? V_MSG_INFO_AUTH : V_MSG_INFO, msg[count]->msg );
			continue;
		case PAM_ERROR_MSG:
			debug( " PAM_ERROR_MSG: %s\n", msg[count]->msg );
			displayStr( V_MSG_ERR, msg[count]->msg );
			continue;
		default:
			/* could do better error handling here, but see below ... */
			if (pd->usecur) {
				switch (msg[count]->msg_style) {
				/* case PAM_PROMPT_ECHO_ON: cannot happen */
				case PAM_PROMPT_ECHO_OFF:
					debug( " PAM_PROMPT_ECHO_OFF (usecur): %s\n", msg[count]->msg );
					if (!curpass)
						pd->gconv( GCONV_PASS, 0 );
					strDup( &reply[count].resp, curpass );
					break;
				default:
					logError( "Unknown PAM message style <%d>\n", msg[count]->msg_style );
					goto conv_err;
				}
			} else {
				switch (msg[count]->msg_style) {
				case PAM_PROMPT_ECHO_ON:
					debug( " PAM_PROMPT_ECHO_ON: %s\n", msg[count]->msg );
					reply[count].resp = pd->gconv( GCONV_NORMAL, msg[count]->msg );
					break;
				case PAM_PROMPT_ECHO_OFF:
					debug( " PAM_PROMPT_ECHO_OFF: %s\n", msg[count]->msg );
					reply[count].resp = pd->gconv( GCONV_HIDDEN, msg[count]->msg );
					break;
#ifdef PAM_BINARY_PROMPT
				case PAM_BINARY_PROMPT:
					debug( " PAM_BINARY_PROMPT\n" );
					reply[count].resp = pd->gconv( GCONV_BINARY, msg[count]->msg );
					break;
#endif
				default:
					logError( "Unknown PAM message style <%d>\n", msg[count]->msg_style );
					goto conv_err;
				}
			}
			if (!reply[count].resp) {
				debug( "  PAM_conv aborted\n" );
				pd->abort = True;
				goto conv_err;
			}
			reply[count].resp_retcode = PAM_SUCCESS; /* unused in linux-pam */
		}
	debug( " PAM_conv success\n" );
	*resp = reply;
	return PAM_SUCCESS;

  conv_err:
	for (; count >= 0; count--)
		if (reply[count].resp)
			switch (msg[count]->msg_style) {
			case PAM_PROMPT_ECHO_OFF:
				wipeStr( reply[count].resp );
				break;
#ifdef PAM_BINARY_PROMPT
			case PAM_BINARY_PROMPT: /* Don't know length, so how wipe? */
#endif
			case PAM_PROMPT_ECHO_ON:
				free( reply[count].resp );
				break;
			}
	free( reply );
	return PAM_CONV_ERR;
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

	pdata->abort = False;
	pconv.conv = PAM_conv;
	pconv.appdata_ptr = (void *)pdata;
	debug( " PAM service %s\n", psrv );
	if ((pretc = pam_start( psrv, curuser, &pconv, &pamh )) != PAM_SUCCESS)
		goto pam_bail2;
	if ((pretc = pam_set_item( pamh, PAM_TTY, td->name )) != PAM_SUCCESS) {
	  pam_bail:
		pam_end( pamh, pretc );
		pamh = 0;
	  pam_bail2:
		reInitErrorLog();
		logError( "PAM error: %s\n", pam_strerror( 0, pretc ) );
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
# ifdef __sun__ /* Only Solaris <= 9, but checking it does not seem worth it. */
	else if (pam_set_item( pamh, PAM_RHOST, 0 ) != PAM_SUCCESS)
		goto pam_bail;
# endif
# ifdef PAM_FAIL_DELAY
	pam_set_item( pamh, PAM_FAIL_DELAY, (void *)fail_delay );
# endif
	reInitErrorLog();

	inAuth = True;
	debug( " pam_authenticate() ...\n" );
	pretc = pam_authenticate( pamh,
	                          td->allowNullPasswd ? 0 : PAM_DISALLOW_NULL_AUTHTOK );
	reInitErrorLog();
	debug( " pam_authenticate() returned: %s\n", pam_strerror( pamh, pretc ) );
	inAuth = False;
	if (pdata->abort) {
		pam_end( pamh, PAM_SUCCESS );
		pamh = 0;
		V_RET;
	}
	if (!curuser) {
		debug( " asking PAM for user ...\n" );
		pam_get_item( pamh, PAM_USER, &pitem );
		reInitErrorLog();
		strDup( &curuser, (const char *)pitem );
		gSendInt( V_PUT_USER );
		gSendStr( curuser );
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
	return True;
}

#endif /* USE_PAM */

static int
#if defined(USE_PAM) || defined(_AIX)
isNoPassAllowed( const char *un )
{
	struct passwd *pw;
# ifdef HAVE_GETSPNAM /* (sic!) - not USESHADOW */
	struct spwd *spw;
# endif
#else
isNoPassAllowed( struct passwd *pw )
{
#endif
	struct group *gr;
	char **fp;
	int hg;

#if defined(USE_PAM) || defined(_AIX)
	if (!*un)
		return False;
#endif

	if (cursource != PWSRC_MANUAL)
		return True;

#if defined(USE_PAM) || defined(_AIX)
	/* Give nss_ldap, etc. a chance to normalize (uppercase) the name. */
	if (!(pw = getpwnam( un )) ||
	    pw->pw_passwd[0] == '!' || pw->pw_passwd[0] == '*')
		return False;
# ifdef HAVE_GETSPNAM /* (sic!) - not USESHADOW */
	if ((spw = getspnam( un )) &&
	    (spw->sp_pwdp[0] == '!' || spw->sp_pwdp[0] == '*'))
		return False;
# endif
#endif

	for (hg = False, fp = td->noPassUsers; *fp; fp++)
		if (**fp == '@')
			hg = True;
		else if (!strcmp( pw->pw_name, *fp ))
			return True;
		else if (!strcmp( "*", *fp ) && pw->pw_uid)
			return True;

	if (hg) {
		for (setgrent(); (gr = getgrent()); )
			for (fp = td->noPassUsers; *fp; fp++)
				if (**fp == '@' && !strcmp( gr->gr_name, *fp + 1 )) {
					if (pw->pw_gid == gr->gr_gid) {
						endgrent();
						return True;
					}
					for (; *gr->gr_mem; gr->gr_mem++)
						if (!strcmp( pw->pw_name, *gr->gr_mem )) {
							endgrent();
							return True;
						}
				}
		endgrent();
	}

	return False;
}

#if !defined(USE_PAM) && !defined(_AIX) && defined(HAVE_SETUSERCONTEXT)
# define LC_RET0 do { login_close(lc); V_RET; } while(0)
#else
# define LC_RET0 V_RET
#endif

int
verify( GConvFunc gconv, int rootok )
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
#endif

	debug( "verify ...\n" );

#ifdef USE_PAM

	pnopass = False;
	if (!strcmp( curtype, "classic" )) {
		if (!gconv( GCONV_USER, 0 ))
			return False;
		if (isNoPassAllowed( curuser )) {
			gconv( GCONV_PASS_ND, 0 );
			if (!*curpass) {
				pnopass = True;
				sprintf( psrvb, "%.31s-np", PAMService );
				psrv = psrvb;
			} else
				psrv = PAMService;
		} else
			psrv = PAMService;
		pdata.usecur = True;
	} else {
		sprintf( psrvb, "%.31s-%.31s", PAMService, curtype );
		psrv = psrvb;
		pdata.usecur = False;
	}
	pdata.gconv = gconv;
	if (!doPAMAuth( psrv, &pdata ))
		return False;

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
			return False;
		if (isNoPassAllowed( curuser )) {
			gconv( GCONV_PASS_ND, 0 );
			if (!*curpass) {
				debug( "accepting despite empty password\n" );
				goto done;
			}
		} else
			if (!gconv( GCONV_PASS, 0 ))
				V_RET_NP;
		msg = NULL;
		if ((i = authenticate( curuser, curpass, &reenter, &msg ))) {
			debug( "authenticate() failed: %s\n", msg );
			if (msg)
				free( msg );
			loginfailed( curuser, hostname, tty );
			if (i == ENOENT || i == ESAD)
				V_RET_AUTH;
			else
				V_RET_FAIL( 0 );
		}
		if (reenter) {
			logError( "authenticate() requests more data: %s\n", msg );
			free( msg );
			V_RET_FAIL( 0 );
		}
	} else if (!strcmp( curtype, "generic" )) {
		if (!gconv( GCONV_USER, 0 ))
			return False;
		for (curret = 0;;) {
			msg = NULL;
			if ((i = authenticate( curuser, curret, &reenter, &msg ))) {
				debug( "authenticate() failed: %s\n", msg );
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
				return False;
			free( msg );
		}
	} else {
		logError( "Unsupported authentication type %\"s requested\n", curtype );
		V_RET_FAIL( 0 );
	}
	if (msg) {
		displayStr( V_MSG_INFO, msg );
		free( msg );
	}

  done:

#else

	if (strcmp( curtype, "classic" )) {
		logError( "Unsupported authentication type %\"s requested\n", curtype );
		V_RET_FAIL( 0 );
	}

	if (!gconv( GCONV_USER, 0 ))
		return False;

	if (!(p = getpwnam( curuser ))) {
		debug( "getpwnam() failed.\n" );
		gconv( GCONV_PASS, 0 );
		V_RET_AUTH;
	}
	if (p->pw_passwd[0] == '!' || p->pw_passwd[0] == '*') {
		debug( "account is locked\n" );
		gconv( GCONV_PASS, 0 );
		V_RET_AUTH;
	}

# ifdef USESHADOW
	if ((sp = getspnam( curuser ))) {
		p->pw_passwd = sp->sp_pwdp;
		if (p->pw_passwd[0] == '!' || p->pw_passwd[0] == '*') {
			debug( "account is locked\n" );
			gconv( GCONV_PASS, 0 );
			V_RET_AUTH;
		}
	} else
		debug( "getspnam() failed: %m. Are you root?\n" );
# endif

	if (!*p->pw_passwd) {
		if (!td->allowNullPasswd) {
			debug( "denying user with empty password\n" );
			gconv( GCONV_PASS, 0 );
			V_RET_AUTH;
		}
		goto nplogin;
	}

	if (isNoPassAllowed( p )) {
	  nplogin:
		gconv( GCONV_PASS_ND, 0 );
		if (!*curpass) {
			debug( "accepting password-less login\n" );
			goto done;
		}
	} else
		if (!gconv( GCONV_PASS, 0 ))
			V_RET_NP;

# ifdef KERBEROS
	if (p->pw_uid) {
		int ret;
		char realm[REALM_SZ];

		if (krb_get_lrealm( realm, 1 )) {
			logError( "Cannot get KerberosIV realm.\n" );
			V_RET_FAIL( 0 );
		}

		sprintf( krbtkfile, "%s.%.*s", TKT_ROOT, MAXPATHLEN - strlen( TKT_ROOT ) - 2, td->name );
		krb_set_tkt_string( krbtkfile );
		unlink( krbtkfile );

		ret = krb_verify_user( curuser, "", realm, curpass, 1, "rcmd" );
		if (ret == KSUCCESS) {
			chown( krbtkfile, p->pw_uid, p->pw_gid );
			debug( "KerberosIV verify succeeded\n" );
			goto done;
		} else if (ret != KDC_PR_UNKNOWN && ret != SKDC_CANT) {
			logError( "KerberosIV verification failure %\"s for %s\n",
			          krb_get_err_text( ret ), curuser );
			krbtkfile[0] = '\0';
			V_RET_FAIL( 0 );
		}
		debug( "KerberosIV verify failed: %s\n", krb_get_err_text( ret ) );
	}
	krbtkfile[0] = '\0';
# endif	 /* KERBEROS */

# if defined(ultrix) || defined(__ultrix__)
	if (authenticate_user( p, curpass, NULL ) < 0)
# elif defined(HAVE_PW_ENCRYPT)
	if (strcmp( pw_encrypt( curpass, p->pw_passwd ), p->pw_passwd ))
# elif defined(HAVE_CRYPT)
	if (strcmp( crypt( curpass, p->pw_passwd ), p->pw_passwd ))
# else
	if (strcmp( curpass, p->pw_passwd ))
# endif
	{
		debug( "password verify failed\n" );
		V_RET_AUTH;
	}

  done:

#endif /* !defined(USE_PAM) && !defined(_AIX) */

	debug( "restrict %s ...\n", curuser );

#if defined(USE_PAM) || defined(_AIX)
	if (!(p = getpwnam( curuser ))) {
		logError( "getpwnam(%s) failed.\n", curuser );
		V_RET_FAIL( 0 );
	}
#endif
	if (!p->pw_uid) {
		if (!rootok && !td->allowRootLogin)
			V_RET_FAIL( "Root logins are not allowed" );
		wipeStr( curpass );
		curpass = 0;
		return True; /* don't deny root to log in */
	}

#ifdef USE_PAM

	debug( " pam_acct_mgmt() ...\n" );
	pretc = pam_acct_mgmt( pamh, 0 );
	reInitErrorLog();
	debug( " pam_acct_mgmt() returned: %s\n", pam_strerror( pamh, pretc ) );
	if (pretc == PAM_NEW_AUTHTOK_REQD) {
		pdata.usecur = False;
		pdata.gconv = conv_interact;
		/* pam will have output a message already, so no prepareErrorGreet() */
		if (gconv != conv_interact || pnopass) {
			pam_end( pamh, PAM_SUCCESS );
			pamh = 0;
			gSendInt( V_CHTOK_AUTH );
			/* this cannot auth the wrong user, as only classic auths get here */
			while (!doPAMAuth( PAMService, &pdata ))
				if (pdata.abort)
					return False;
			gSendInt( V_PRE_OK );
		} else
			gSendInt( V_CHTOK );
		for (;;) {
			inAuth = True;
			debug( " pam_chauthtok() ...\n" );
			pretc = pam_chauthtok( pamh, PAM_CHANGE_EXPIRED_AUTHTOK );
			reInitErrorLog();
			debug( " pam_chauthtok() returned: %s\n", pam_strerror( pamh, pretc ) );
			inAuth = False;
			if (pdata.abort) {
				pam_end( pamh, PAM_SUCCESS );
				pamh = 0;
				V_RET;
			}
			if (pretc == PAM_SUCCESS)
				break;
			/* effectively there is only PAM_AUTHTOK_ERR */
			gSendInt( V_FAIL );
		}
		wipeStr( curpass );
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
		debug( "loginrestrictions() - %s\n", msg ? msg : "error" );
		loginfailed( curuser, hostname, tty );
		prepareErrorGreet();
		if (msg) {
			displayStr( V_MSG_ERR, msg );
			free( msg );
		}
		gSendInt( V_AUTH );
		V_RET;
	}
	if (msg)
		free( (void *)msg );

#endif /* USE_PAM || _AIX */

#ifndef _AIX

# ifdef HAVE_SETUSERCONTEXT
#  ifdef HAVE_LOGIN_GETCLASS
	lc = login_getclass( p->pw_class );
#  else
	lc = login_getpwclass( p );
#  endif
	if (!lc)
		V_RET_FAIL( 0 );

	p->pw_shell = login_getcapstr( lc, "shell", p->pw_shell, p->pw_shell );
# endif

# ifndef USE_PAM

/* restrict_expired */
#  if defined(HAVE_STRUCT_PASSWD_PW_EXPIRE) || defined(USESHADOW)

#   if !defined(HAVE_STRUCT_PASSWD_PW_EXPIRE) || (!defined(HAVE_SETUSERCONTEXT) && defined(USESHADOW))
	if (sp)
#   endif
	{

#   define DEFAULT_WARN	(2L * 7L)  /* Two weeks */

		tim = time( NULL ) / 86400L;

#   ifdef HAVE_SETUSERCONTEXT
		quietlog = login_getcapbool( lc, "hushlogin", False );
		warntime = login_getcaptime( lc, "warnexpire",
		                             DEFAULT_WARN * 86400L,
		                             DEFAULT_WARN * 86400L ) / 86400L;
#   else
		quietlog = False;
#    ifdef USESHADOW
		warntime = sp->sp_warn != -1 ? sp->sp_warn : DEFAULT_WARN;
#    else
		warntime = DEFAULT_WARN;
#    endif
#   endif

#   ifdef HAVE_STRUCT_PASSWD_PW_EXPIRE
		if (p->pw_expire) {
			expir = p->pw_expire / 86400L;
#   else
		if (sp->sp_expire != -1) {
			expir = sp->sp_expire;
#   endif
			if (tim > expir) {
				displayStr( V_MSG_ERR,
				            "Your account has expired;"
				            " please contact your system administrator" );
				gSendInt( V_FAIL );
				LC_RET0;
			} else if (tim > (expir - warntime) && !quietlog) {
				displayMsg( V_MSG_INFO,
				            "Warning: your account will expire in %d day(s)",
				            expir - tim );
			}
		}

#   ifdef HAVE_STRUCT_PASSWD_PW_EXPIRE
		if (p->pw_change) {
			expir = p->pw_change / 86400L;
#   else
		if (!sp->sp_lstchg) {
			displayStr( V_MSG_ERR,
			            "You are required to change your password immediately"
			            " (root enforced)" );
			/* XXX todo password change */
			gSendInt( V_FAIL );
			LC_RET0;
		} else if (sp->sp_max != -1) {
			expir = sp->sp_lstchg + sp->sp_max;
			if (sp->sp_inact != -1 && tim > expir + sp->sp_inact) {
				displayStr( V_MSG_ERR,
				            "Your account has expired;"
				            " please contact your system administrator" );
				gSendInt( V_FAIL );
				LC_RET0;
			}
#   endif
			if (tim > expir) {
				displayStr( V_MSG_ERR,
				            "You are required to change your password immediately"
				            " (password aged)" );
				/* XXX todo password change */
				gSendInt( V_FAIL );
				LC_RET0;
			} else if (tim > (expir - warntime) && !quietlog) {
				displayMsg( V_MSG_INFO,
				            "Warning: your password will expire in %d day(s)",
				            expir - tim );
			}
		}

	}

#  endif /* HAVE_STRUCT_PASSWD_PW_EXPIRE || USESHADOW */

/* restrict_nologin */
#  ifndef _PATH_NOLOGIN
#   define _PATH_NOLOGIN "/etc/nologin"
#  endif

	if ((
#  ifdef HAVE_SETUSERCONTEXT
	     /* Do we ignore a nologin file? */
	     !login_getcapbool( lc, "ignorenologin", False )) &&
	    (!stat( (nolg = login_getcapstr( lc, "nologin", "", NULL )), &st ) ||
#  endif
		 !stat( (nolg = _PATH_NOLOGIN), &st )))
	{
		if (st.st_size && (fd = open( nolg, O_RDONLY )) >= 0) {
			if ((buf = Malloc( st.st_size + 1 ))) {
				if (read( fd, buf, st.st_size ) == st.st_size) {
					close( fd );
					buf[st.st_size] = 0;
					displayStr( V_MSG_ERR, buf );
					free( buf );
					gSendInt( V_FAIL );
					LC_RET0;
				}
				free( buf );
			}
			close( fd );
		}
		displayStr( V_MSG_ERR,
		            "Logins are not allowed at the moment.\nTry again later" );
		gSendInt( V_FAIL );
		LC_RET0;
	}

/* restrict_time */
#  if defined(HAVE_SETUSERCONTEXT) && defined(HAVE_AUTH_TIMEOK)
	if (!auth_timeok( lc, time( NULL ) )) {
		displayStr( V_MSG_ERR,
		            "You are not allowed to login at the moment" );
		gSendInt( V_FAIL );
		LC_RET0;
	}
#  endif

#  ifdef HAVE_GETUSERSHELL
	for (;;) {
		if (!(s = getusershell())) {
			debug( "shell not in /etc/shells\n" );
			endusershell();
			V_RET_FAIL( "Your login shell is not listed in /etc/shells" );
		}
		if (!strcmp( s, p->pw_shell )) {
			endusershell();
			break;
		}
	}
#  endif

# endif /* !USE_PAM */

/* restrict_nohome */
# ifdef HAVE_SETUSERCONTEXT
	if (login_getcapbool( lc, "requirehome", False )) {
		struct stat st;
		if (!*p->pw_dir || stat( p->pw_dir, &st ) || st.st_uid != p->pw_uid) {
			displayStr( V_MSG_ERR, "Home folder not available" );
			gSendInt( V_FAIL );
			LC_RET0;
		}
	}
# endif

#endif /* !_AIX */

	return True;

}


static const char *envvars[] = {
	"TZ", /* SYSV and SVR4, but never hurts */
#ifdef _AIX
	"AUTHSTATE", /* for kerberos */
#endif
	NULL
};


#if defined(USE_PAM) && defined(HAVE_INITGROUPS)
static int num_saved_gids;
static gid_t *saved_gids;

static int
saveGids( void )
{
	num_saved_gids = getgroups( 0, 0 );
	if (!(saved_gids = Malloc( sizeof(gid_t) * num_saved_gids )))
		return False;
	if (getgroups( num_saved_gids, saved_gids ) < 0) {
		logError( "saving groups failed: %m\n" );
		return False;
	}
	return True;
}

static int
restoreGids( void )
{
	if (setgroups( num_saved_gids, saved_gids ) < 0) {
		logError( "restoring groups failed: %m\n" );
		return False;
	}
	if (setgid( p->pw_gid ) < 0) {
		logError( "restoring gid failed: %m\n" );
		return False;
	}
	return True;
}
#endif /* USE_PAM && HAVE_INITGROUPS */

static int
resetGids( void )
{
#ifdef HAVE_INITGROUPS
	if (setgroups( 0, &p->pw_gid /* anything */ ) < 0) {
		logError( "restoring groups failed: %m\n" );
		return False;
	}
#endif
	if (setgid( 0 ) < 0) {
		logError( "restoring gid failed: %m\n" );
		return False;
	}
	return True;
}

static int
setGid( const char *name, int gid )
{
	if (setgid( gid ) < 0) {
		logError( "setgid(%d) (user %s) failed: %m\n", gid, name );
		return False;
	}
#ifdef HAVE_INITGROUPS
	if (initgroups( name, gid ) < 0) {
		logError( "initgroups for %s failed: %m\n", name );
		setgid( 0 );
		return False;
	}
#endif	 /* QNX4 doesn't support multi-groups, no initgroups() */
	return True;
}

static int
setUid( const char *name, int uid )
{
	if (setuid( uid ) < 0) {
		logError( "setuid(%d) (user %s) failed: %m\n", uid, name );
		return False;
	}
	return True;
}

static int
setUser( const char *name, int uid, int gid )
{
	if (setGid( name, gid )) {
		if (setUid( name, uid ))
			return True;
		resetGids();
	}
	return False;
}

#if defined(SECURE_RPC) || defined(K5AUTH)
static void
nukeAuth( int len, const char *name )
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
		if (strApp( &mfname, dmrcDir, "/", curuser, fname, (char *)0 ))
			fname = mfname;
	needsave = False;
	if (!curdmrc) {
		curdmrc = iniLoad( fname );
		if (!curdmrc) {
			strDup( &curdmrc, "[Desktop]\nSession=default\n" );
			needsave = True;
		}
	}
	if (newdmrc) {
		curdmrc = iniMerge( curdmrc, newdmrc );
		needsave = True;
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

static int
createClientLog( const char *log )
{
	char randstr[32], *randstrp = 0, *lname;
	int lfd;

	for (;;) {
		struct expando macros[] = {
			{ 'd', 0, td->name },
			{ 'u', 0, curuser },
			{ 'r', 0, randstrp },
			{ 0, 0, 0 }
		};
		if (!(lname = expandMacros( log, macros )))
			exit( 1 );
		unlink( lname );
		if ((lfd = open( lname, O_WRONLY|O_CREAT|O_EXCL, 0600 )) >= 0) {
			dup2( lfd, 1 );
			dup2( lfd, 2 );
			close( lfd );
			free( lname );
			return True;
		}
		if (errno != EEXIST || !macros[2].uses) {
			free( lname );
			return False;
		}
		logInfo( "Session log file %s not usable, trying another one.\n",
		         lname );
		free( lname );
		sprintf( randstr, "%d", secureRandom() );
		randstrp = randstr;
	}
}

static int removeAuth;
#ifdef USE_PAM
static int removeSession;
static int removeCreds;
#endif

static GPipe ctlpipe;
static GTalk ctltalk;

static void
sendStr( int lv, const char *msg )
{
	gSet( &ctltalk );
	gSendInt( lv );
	gSendStr( msg );
	gRecvInt();
}

/*
static void
sendMsg( int lv, const char *msg, ... )
{
	char *ae;
	va_list args;

	va_start( args, msg );
	VASPrintf( &ae, msg, args );
	va_end( args );
	if (ae) {
		sendStr( lv, ae );
		free( ae );
	}
}
*/

int
startClient( volatile int *pid )
{
	const char *home, *sessargs, *desksess;
	char **env, *xma;
	char **argv, *fname, *str;
#ifdef USE_PAM
	char ** volatile pam_env;
# ifndef HAVE_PAM_GETENVLIST
	char **saved_env;
# endif
	int pretc;
#else
# ifdef _AIX
	char *msg;
	char **theenv;
	extern char **newenv; /* from libs.a, this is set up by setpenv */
# endif
#endif
	char *failsafeArgv[2];
	char *buf, *buf2;
	int i;

	if (strCmp( dmrcuser, curuser )) {
		if (curdmrc) { free( curdmrc ); curdmrc = 0; }
		if (dmrcuser) { free( dmrcuser ); dmrcuser = 0; }
	}

#if defined(USE_PAM) || defined(_AIX)
	if (!(p = getpwnam( curuser ))) {
		logError( "getpwnam(%s) failed.\n", curuser );
	  pError:
		displayStr( V_MSG_ERR, 0 );
		V_RET;
	}
#endif

	strcpy( curuser, p->pw_name ); /* Use normalized login name. */

#ifndef USE_PAM
# ifdef _AIX
	msg = NULL;
	loginsuccess( curuser, hostname, tty, &msg );
	if (msg) {
		debug( "loginsuccess() - %s\n", msg );
		free( (void *)msg );
	}
# else /* _AIX */
#  if defined(KERBEROS) && defined(AFS)
	if (krbtkfile[0] != '\0') {
		if (k_hasafs()) {
			int fail = False;
			if (k_setpag() == -1) {
				logError( "setpag() for %s failed\n", curuser );
				fail = True;
			}
			if ((ret = k_afsklog( NULL, NULL )) != KSUCCESS) {
				logError( "AFS Warning: %s\n", krb_get_err_text( ret ) );
				fail = True;
			}
			if (fail)
				displayMsg( V_MSG_ERR,
				            "Warning: Problems during Kerberos4/AFS setup." );
		}
	}
#  endif /* KERBEROS && AFS */
# endif /* _AIX */
#endif	/* !PAM */

	curuid = p->pw_uid;
	curgid = p->pw_gid;

	env = baseEnv( 0, curuser );
	xma = 0;
	strApp( &xma, "method=", curtype, (char *)0 );
	if (td_setup)
		strApp( &xma, ",auto", (char *)0 );
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
	env = systemEnv( 0, curuser );
	systemEnviron = setEnv( env, "HOME", p->pw_dir );
	debug( "user environment:\n%[|''>'\n's"
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
				krb5GetAuthFor( 14, "MIT-KERBEROS-5", td->name );
			saveServerAuthorizations( td, td->authorizations, td->authNum );
		}
#endif
	}

	if (*dmrcDir)
		mergeSessionArgs( True );

	debug( "now starting the session\n" );

#ifdef USE_PAM

# ifdef HAVE_SETUSERCONTEXT
	if (setusercontext( lc, p, p->pw_uid, LOGIN_SETGROUP )) {
		logError( "setusercontext(groups) for %s failed: %m\n",
		          curuser );
		goto pError;
	}
# else
	if (!setGid( curuser, curgid ))
		goto pError;
# endif

# ifndef HAVE_PAM_GETENVLIST
	if (!(pam_env = initStrArr( 0 ))) {
		resetGids();
		goto pError;
	}
	saved_env = environ;
	environ = pam_env;
# endif
	removeCreds = True; /* set it first - i don't trust PAM's rollback */
	pretc = pam_setcred( pamh, 0 );
	reInitErrorLog();
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
		goto pError;
# endif
	if (pretc != PAM_SUCCESS) {
		logError( "pam_setcred() for %s failed: %s\n",
		          curuser, pam_strerror( pamh, pretc ) );
		resetGids();
		V_RET;
	}

	removeSession = True; /* set it first - same as above */
	pretc = pam_open_session( pamh, 0 );
	reInitErrorLog();
	if (pretc != PAM_SUCCESS) {
		logError( "pam_open_session() for %s failed: %s\n",
		          curuser, pam_strerror( pamh, pretc ) );
		resetGids();
		V_RET;
	}

	/* we don't want sessreg and the startup/reset scripts run with user
	   credentials. unfortunately, we can reset only the gids. */
	resetGids();

# define D_LOGIN_SETGROUP LOGIN_SETGROUP
#else /* USE_PAM */
# define D_LOGIN_SETGROUP 0
#endif /* USE_PAM */

	removeAuth = True;
	chownCtrl( &td->ctrl, curuid );
	ctltalk.pipe = &ctlpipe;
	ASPrintf( &buf, "sub-daemon for display %s", td->name );
	ASPrintf( &buf2, "client for display %s", td->name );
	switch (gFork( &ctlpipe, buf, buf2, 0, 0, mstrtalk.pipe, pid )) {
	case 0:

		gCloseOnExec( ctltalk.pipe );
		if (Setjmp( ctltalk.errjmp ))
			exit( 1 );

		gCloseOnExec( mstrtalk.pipe );
		if (Setjmp( mstrtalk.errjmp ))
			goto cError;

#ifndef NOXDMTITLE
		setproctitle( "%s'", td->name );
#endif
		strApp( &prog, " '", (char *)0 );
		reInitErrorLog();

		setsid();

		sessreg( td, getpid(), curuser, curuid );

		/* We do this here, as we want to have the session as parent. */
		switch (source( systemEnviron, td->startup, td_setup )) {
		case 0:
			break;
		case wcCompose( 0, 0, 127 ):
			goto cError;
		default: /* Explicit failure => message already displayed. */
			logError( "Startup script returned non-zero exit code\n" );
			exit( 1 );
		}

	/* Memory leaks are ok here as we exec() soon. */

#if defined(USE_PAM) || !defined(_AIX)

# ifdef USE_PAM
		/* pass in environment variables set by libpam and modules it called */
#  ifdef HAVE_PAM_GETENVLIST
		pam_env = pam_getenvlist( pamh );
		reInitErrorLog();
#  endif
		if (pam_env)
			for (; *pam_env; pam_env++)
				userEnviron = putEnv( *pam_env, userEnviron );
# endif

# ifdef HAVE_SETLOGIN
		if (setlogin( curuser ) < 0) {
			logError( "setlogin for %s failed: %m\n", curuser );
			goto cError;
		}
#  define D_LOGIN_SETLOGIN LOGIN_SETLOGIN
# else
#  define D_LOGIN_SETLOGIN 0
# endif

# if defined(USE_PAM) && defined(HAVE_INITGROUPS)
		if (!restoreGids())
			goto cError;
# endif

# ifndef HAVE_SETUSERCONTEXT

#  ifdef USE_PAM
		if (!setUid( curuser, curuid ))
			goto cError;
#  else
		if (!setUser( curuser, curuid, curgid ))
			goto cError;
#  endif

# else /* !HAVE_SETUSERCONTEXT */

		/*
		 * Destroy environment.
		 * We need to do this before setusercontext() because that may
		 * set or reset some environment variables.
		 */
		if (!(environ = initStrArr( 0 )))
			goto cError;

		/*
		 * Set the user's credentials: uid, gid, groups,
		 * environment variables, resource limits, and umask.
		 */
		if (setusercontext( lc, p, p->pw_uid,
		        LOGIN_SETALL & ~(D_LOGIN_SETGROUP|D_LOGIN_SETLOGIN) ) < 0)
		{
			logError( "setusercontext for %s failed: %m\n", curuser );
			goto cError;
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
			logError( "setpcred for %s failed: %m\n", curuser );
			goto cError;
		}

		/*
		 * Set the users process environment. Store protected variables and
		 * obtain updated user environment list. This call will initialize
		 * global 'newenv'.
		 */
		userEnviron = xCopyStrArr( 1, userEnviron );
		userEnviron[0] = (char *)"USRENVIRON:";
		if (setpenv( curuser, PENV_INIT | PENV_ARGV | PENV_NOEXEC,
		             userEnviron, NULL ) != 0)
		{
			logError( "Cannot set %s's process environment\n", curuser );
			goto cError;
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
			logInfo( "No password for NIS provided.\n" );
		else {
			char netname[MAXNETNAMELEN+1], secretkey[HEXKEYBYTES+1];
			int nameret, keyret;
			int len;
			int key_set_ok = False;
			struct key_netstarg netst;

			nameret = getnetname( netname );
			debug( "user netname: %s\n", netname );
			len = strlen( curpass );
			if (len > 8)
				bzero( curpass + 8, len - 8 );
			keyret = getsecretkey( netname, secretkey, curpass );
			debug( "getsecretkey returns %d, key length %d\n",
			       keyret, strlen( secretkey ) );
			netst.st_netname = netname;
			memcpy( netst.st_priv_key, secretkey, HEXKEYBYTES );
			memset( netst.st_pub_key, 0, HEXKEYBYTES );
			if (key_setnet( &netst ) < 0)
				debug( "Could not set secret key.\n" );
			/* is there a key, and do we have the right password? */
			if (keyret == 1) {
				if (*secretkey) {
					keyret = key_setsecret( secretkey );
					debug( "key_setsecret returns %d\n", keyret );
					if (keyret == -1)
						logError( "Failed to set NIS secret key\n" );
					else
						key_set_ok = True;
				} else {
					/* found a key, but couldn't interpret it */
					logError( "Password incorrect for NIS principal %s\n",
					          nameret ? netname : curuser );
				}
			}
			if (!key_set_ok)
				nukeAuth( 9, "SUN-DES-1" );
			bzero( secretkey, strlen( secretkey ) );
		}
#endif
#ifdef K5AUTH
		/* do like "kinit" program */
		if (!curpass[0])
			logInfo( "No password for Kerberos5 provided.\n" );
		else
			if ((str = krb5Init( curuser, curpass, td->name )))
				userEnviron = setEnv( userEnviron, "KRB5CCNAME", str );
			else
				nukeAuth( 14, "MIT-KERBEROS-5" );
#endif /* K5AUTH */
		if (td->autoReLogin) {
			gSet( &mstrtalk );
			gSendInt( D_ReLogin );
			gSendStr( curuser );
			gSendStr( curpass );
			gSendStr( newdmrc );
		}
		if (curpass)
			bzero( curpass, strlen( curpass ) );
		setUserAuthorization( td );
		home = getEnv( userEnviron, "HOME" );
		if (home && chdir( home ) < 0) {
			logError( "Cannot chdir to %s's home %s: %m\n", curuser, home );
			sendStr( V_MSG_ERR, "Cannot enter home directory. Using /.\n" );
			chdir( "/" );
			userEnviron = setEnv( userEnviron, "HOME", "/" );
			home = 0;
		}
		if (home || td->clientLogFile[0] == '/') {
			if (!createClientLog( td->clientLogFile )) {
				logWarn( "Session log file according to %s cannot be created: %m\n",
				         td->clientLogFile );
				goto tmperr;
			}
		} else {
		  tmperr:
			if (!createClientLog( td->clientLogFallback ))
				logError( "Fallback session log file according to %s cannot be created: %m\n",
				          td->clientLogFallback );
			/* Could inform the user, but I guess this is only confusing. */
		}
		if (!*dmrcDir)
			mergeSessionArgs( home != 0 );
		if (!(desksess = iniEntry( curdmrc, "Desktop", "Session", 0 )))
			desksess = "failsafe"; /* only due to OOM */
		gSet( &mstrtalk );
		gSendInt( D_User );
		gSendInt( curuid );
		gSendStr( curuser );
		gSendStr( desksess );
		close( mstrtalk.pipe->fd.w );
		userEnviron = setEnv( userEnviron, "DESKTOP_SESSION", desksess );
		for (i = 0; td->sessionsDirs[i]; i++) {
			fname = 0;
			if (strApp( &fname, td->sessionsDirs[i], "/", desksess, ".desktop", (char *)0 )) {
				if ((str = iniLoad( fname ))) {
					if (!strCmp( iniEntry( str, "Desktop Entry", "Hidden", 0 ), "true" ) ||
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
			debug( "executing session %\"[s\n", argv );
			execute( argv, userEnviron );
			logError( "Session %\"s execution failed: %m\n", argv[0] );
		} else
			logError( "Session has no command/arguments\n" );
		failsafeArgv[0] = td->failsafeClient;
		failsafeArgv[1] = 0;
		execute( failsafeArgv, userEnviron );
		logError( "Failsafe client %\"s execution failed: %m\n",
		          failsafeArgv[0] );
	  cError:
		sendStr( V_MSG_ERR, 0 );
		exit( 1 );
	case -1:
		free( buf );
		V_RET;
	}
	debug( "StartSession, fork succeeded %d\n", *pid );
	free( buf );
	wipeStr( curpass );
	curpass = 0;
	END_ENT;

	gSet( &ctltalk );
	if (!Setjmp( ctltalk.errjmp ))
		while (gRecvCmd( &i )) {
			buf = gRecvStr();
			displayStr( i, buf );
			free( buf );
			gSet( &ctltalk );
			gSendInt( 0 );
		}
	gClosen( ctltalk.pipe );
	finishGreet();

	return True;
}

void
clientExited( void )
{
	int pid;
#ifdef USE_PAM
	int pretc;
#endif

	if (removeAuth) {
		switch (source( systemEnviron, td->reset, td_setup )) {
		case 0:
		case wcCompose( 0, 0, 127 ):
			break;
		default:
			logError( "Reset script returned non-zero exit code\n" );
			break;
		}
		sessreg( td, 0, 0, 0 );

		switch (Fork( &pid )) {
		case 0:
#if defined(USE_PAM) && defined(HAVE_INITGROUPS)
			if (restoreGids() && setUid( curuser, curuid ))
#else
			if (setUser( curuser, curuid, curgid ))
#endif

			{
				removeUserAuthorization( td );
#ifdef K5AUTH
				krb5Destroy( td->name );
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
			logError( "Cannot clean up session: fork() failed: %m" );
			break;
		default:
			Wait4( &pid );
			break;
		}
		removeAuth = False;
	}

#ifdef USE_PAM
	if (removeCreds) {
# ifdef HAVE_INITGROUPS
		restoreGids();
# endif
		if (removeSession) {
			pretc = pam_close_session( pamh, 0 );
			reInitErrorLog();
			if (pretc != PAM_SUCCESS)
				logError( "pam_close_session() failed: %s\n",
				          pam_strerror( pamh, pretc ) );
		}
		pretc = pam_setcred( pamh, PAM_DELETE_CRED );
		reInitErrorLog();
		if (pretc != PAM_SUCCESS)
			logError( "pam_setcred(DELETE_CRED) failed: %s\n",
			          pam_strerror( pamh, pretc ) );
		resetGids();
		removeCreds = False;
	}
	if (pamh) {
		pam_end( pamh, PAM_SUCCESS );
		reInitErrorLog();
		pamh = 0;
	}
#endif
}

void
sessionExit( int status )
{
	Signal( SIGTERM, SIG_IGN );

	clientExited();

	finishGreet();

	/* make sure the server gets reset after the session is over */
	if (td->serverPid >= 2) {
		if (!td->terminateServer && td->resetSignal)
			terminateProcess( td->serverPid, td->resetSignal );
	} else
		resetServer( td );
	debug( "display %s exiting with status %d\n", td->name, status );
	exit( status );
}

int
readDmrc()
{
	char *data, *fname = 0;
	int len, pid, pfd[2], err;

	endpwent();
	if (!dmrcuser || !dmrcuser[0] || !(p = getpwnam( dmrcuser )))
		return GE_NoUser;

	if (*dmrcDir) {
		if (!strApp( &fname, dmrcDir, "/", p->pw_name, ".dmrc", (char *)0 ))
			return GE_Error;
		if (!(curdmrc = iniLoad( fname ))) {
			free( fname );
			return GE_Ok;
		}
		free( fname );
		return GE_NoFile;
	}

	if (!strApp( &fname, p->pw_dir, "/.dmrc", (char *)0 ))
		return GE_Error;
	if (pipe( pfd ))
		return GE_Error;
	switch (Fork( &pid )) {
	case -1:
		close( pfd[0] );
		close( pfd[1] );
		return GE_Error;
	case 0:
		if (!setUser( p->pw_name, p->pw_uid, p->pw_gid ))
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
	if (reader( pfd[0], &len, sizeof(int) ) == sizeof(int)) {
		if (len == -1)
			err = GE_Denied;
		else if ((curdmrc = Malloc( len + 1 ))) {
			if (reader( pfd[0], curdmrc, len + 1 ) == len + 1)
				err = GE_Ok;
			else {
				free( curdmrc );
				curdmrc = 0;
			}
		}
	}
	close( pfd[0] );
	Wait4( &pid );
	return err;
}
