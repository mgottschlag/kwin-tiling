/*

Copyright 1988, 1998  The Open Group
Copyright 2001-2005 Oswald Buddenhagen <ossi@kde.org>

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
 * display manager
 */

#define NEED_SIGNAL
#include "dm.h"
#include "dm_socket.h"
#include "dm_error.h"

#include <string.h>

static void
acceptSock( CtrlRec *cr )
{
	struct cmdsock *cs;
	int fd;

	if ((fd = accept( cr->fd, 0, 0 )) < 0) {
	  bust:
		LogError( "Error accepting command connection\n" );
		return;
	}
	if (!(cs = Malloc( sizeof(*cs) ))) {
		close( fd );
		goto bust;
	}
	cs->sock.fd = fd;
	cs->sock.buffer = 0;
	cs->sock.buflen = 0;
	cs->next = cr->css;
	cr->css = cs;
	fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK );
	RegisterCloseOnFork( fd );
	RegisterInput( fd );
}

static void
nukeSock( struct cmdsock *cs )
{
	UnregisterInput( cs->sock.fd );
	CloseNClearCloseOnFork( cs->sock.fd );
	if (cs->sock.buffer)
		free( cs->sock.buffer );
	free( cs );
}


static CtrlRec ctrl = { 0, 0, -1, 0, 0, { -1, 0, 0 } };

void
openCtrl( struct display *d )
{
	CtrlRec *cr;
	const char *dname;
	char *sockdir;
	struct sockaddr_un sa;

	if (!*fifoDir)
		return;
	if (d) {
		cr = &d->ctrl, dname = d->name;
		if (!memcmp( dname, "localhost:", 10 ))
			dname += 9;
	} else
		cr = &ctrl, dname = 0;
	if (cr->fifo.fd < 0) {
		if (mkdir( fifoDir, 0755 )) {
			if (errno != EEXIST) {
				LogError( "mkdir %\"s failed; no control FiFos will be available\n",
				          fifoDir );
				return;
			}
		} else
			chmod( fifoDir, 0755 ); /* override umask */
		StrApp( &cr->fpath, fifoDir, dname ? "/xdmctl-" : "/xdmctl",
		        dname, (char *)0 );
		if (cr->fpath) {
			unlink( cr->fpath );
			if (mkfifo( cr->fpath, 0 ) < 0)
				LogError( "Cannot create control FiFo %\"s\n", cr->fpath );
			else {
				cr->gid = fifoGroup;
				if (!d)
					chown( cr->fpath, -1, fifoGroup );
				chmod( cr->fpath, 0620 );
				if ((cr->fifo.fd = open( cr->fpath, O_RDWR | O_NONBLOCK )) >= 0) {
					RegisterCloseOnFork( cr->fifo.fd );
					RegisterInput( cr->fifo.fd );
					goto fifok;
				}
				unlink( cr->fpath );
				LogError( "Cannot open control FiFo %\"s\n", cr->fpath );
			}
			free( cr->fpath );
			cr->fpath = 0;
		}
	}
  fifok:
	if (cr->fd < 0) {
		/* fifoDir is created above already */
		sockdir = 0;
		StrApp( &sockdir, fifoDir, dname ? "/dmctl-" : "/dmctl",
		        dname, (char *)0 );
		if (sockdir) {
			StrApp( &cr->path, sockdir, "/socket", (char *)0 );
			if (cr->path) {
				if (strlen( cr->path ) >= sizeof(sa.sun_path))
					LogError( "path %\"s too long; no control sockets will be available\n",
					          cr->path );
				else if (mkdir( sockdir, 0755 ) && errno != EEXIST)
					LogError( "mkdir %\"s failed; no control sockets will be available\n",
					          sockdir );
				else {
					if (!d)
						chown( sockdir, -1, fifoGroup );
					chmod( sockdir, 0750 );
					if ((cr->fd = socket( PF_UNIX, SOCK_STREAM, 0 )) < 0)
						LogError( "Cannot create control socket\n" );
					else {
						unlink( cr->path );
						sa.sun_family = AF_UNIX;
						strcpy( sa.sun_path, cr->path );
						if (!bind( cr->fd, (struct sockaddr *)&sa, sizeof(sa) )) {
							if (!listen( cr->fd, 5 )) {
								chmod( cr->path, 0666 );
								RegisterCloseOnFork( cr->fd );
								RegisterInput( cr->fd );
								free( sockdir );
								return;
							}
							unlink( cr->path );
							LogError( "Cannot listen on control socket %\"s\n",
							          cr->path );
						} else
							LogError( "Cannot bind control socket %\"s\n",
							          cr->path );
						close( cr->fd );
						cr->fd = -1;
					}
				}
				free( cr->path );
				cr->path = 0;
			}
			free( sockdir );
		}
	}
}

void
closeCtrl( struct display *d )
{
	CtrlRec *cr = d ? &d->ctrl : &ctrl;

	if (cr->fd >= 0) {
		UnregisterInput( cr->fd );
		CloseNClearCloseOnFork( cr->fd );
		cr->fd = -1;
		unlink( cr->path );
		*strrchr( cr->path, '/' ) = 0;
		rmdir( cr->path );
		*strrchr( cr->path, '/' ) = 0;
		rmdir( cr->path );
		free( cr->path );
		cr->path = 0;
		while (cr->css) {
			struct cmdsock *cs = cr->css;
			cr->css = cs->next;
			nukeSock( cs );
		}
	}
	if (cr->fifo.fd >= 0) {
		UnregisterInput( cr->fifo.fd );
		CloseNClearCloseOnFork( cr->fifo.fd );
		cr->fifo.fd = -1;
		unlink( cr->fpath );
		*strrchr( cr->fpath, '/' ) = 0;
		rmdir( cr->fpath );
		free( cr->fpath );
		cr->fpath = 0;
		if (cr->fifo.buffer)
			free( cr->fifo.buffer );
		cr->fifo.buffer = 0;
		cr->fifo.buflen = 0;
	}
}

void
chownCtrl( CtrlRec *cr, int uid, int gid )
{
	if (cr->fpath)
		chown( cr->fpath, uid, gid );
	if (cr->path) {
		char *ptr = strrchr( cr->path, '/' );
		*ptr = 0;
		chown( cr->path, uid, gid );
		*ptr = '/';
	}
}

void
updateCtrl( void )
{
	unsigned ffl, slc;

	ffl = 0;
	if (ctrl.path)
		for (ffl = strlen( ctrl.path ), slc = 2; ;)
			if (ctrl.path[--ffl] == '/')
				if (!--slc)
					break;
	if (ffl != strlen( fifoDir ) || memcmp( fifoDir, ctrl.path, ffl ) ||
	    ctrl.gid != fifoGroup)
	{
		closeCtrl( 0 );
		openCtrl( 0 );
	}
}


static void
fLog( struct display *d, int fd, const char *sts, const char *msg, ... )
{
	char *fmsg, *otxt;
	const char *what;
	int olen;
	va_list va;

	va_start( va, msg );
	VASPrintf( &fmsg, msg, va );
	va_end( va );
	if (!fmsg)
		return;
	if (fd >= 0) {
		olen = ASPrintf( &otxt, "%s\t%\\s\n", sts, fmsg );
		if (otxt) {
			Writer( fd, otxt, olen );
			free( otxt );
		}
		what = "socket";
	} else
		what = "FiFo";
	if (d)
		Debug( "control %s for %s: %s - %s", what, d->name, sts, fmsg );
	else
		Debug( "global control %s: %s - %s", what, sts, fmsg );
	free( fmsg );
}


static char *
unQuote( const char *str )
{
	char *ret, *adp;

	if (!(ret = Malloc( strlen( str ) + 1 )))
		return 0;
	for (adp = ret; *str; str++, adp++)
		if (*str == '\\')
			switch (*++str) {
			case 0: str--; /* fallthrough */
			case '\\': *adp = '\\'; break;
			case 'n': *adp = '\n'; break;
			case 't': *adp = '\t'; break;
			default: *adp++ = '\\'; *adp = *str; break;
			}
		else
			*adp = *str;
	*adp = 0;
	return ret;
}

static void
str_cat_l( char **bp, const char *str, int max )
{
	int dnl = strlen( str );
	if (dnl > max)
		dnl = max;
	memcpy( *bp, str, dnl );
	*bp += dnl;
}

static void
str_cat( char **bp, const char *str )
{
	int dnl = strlen( str );
	memcpy( *bp, str, dnl );
	*bp += dnl;
}

static void
sd_cat( char **bp, SdRec *sdr )
{
	if (sdr->how == SHUT_HALT)
		str_cat( bp, "halt," );
	else
		str_cat( bp, "reboot," );
	if (sdr->start == TO_INF)
		str_cat( bp, "0," );
	else
		*bp += sprintf( *bp, "%d,", sdr->start );
	if (sdr->timeout == TO_INF)
		str_cat( bp, "-1," );
	else
		*bp += sprintf( *bp, "%d,", sdr->timeout );
	if (sdr->force == SHUT_ASK)
		str_cat( bp, "ask" );
	else if (sdr->force == SHUT_FORCE)
		str_cat( bp, "force" );
	else if (sdr->force == SHUT_FORCEMY)
		str_cat( bp, "forcemy" );
	else
		str_cat( bp, "cancel" );
	*bp += sprintf( *bp, ",%d,%s", sdr->uid, sdr->osname ? sdr->osname : "-" );
}

static void
processCtrl( const char *string, int len, int fd, struct display *d )
{
#define Reply(t) Writer (fd, t, strlen (t))

	struct display *di;
	const char *word;
	char **ar, **ap, *args, *bp;
	SdRec sdr;
	char cbuf[1024];

	if (!(ar = initStrArr( 0 )))
		return;
	for (word = string; ; string++, len--)
		if (!len || *string == '\t') {
			if (!(ar = addStrArr( ar, word, string - word )))
				return;
			if (!len)
				break;
			word = string + 1;
		}
	word = fd >= 0 ? "socket" : "FiFo";
	if (d)
		Debug( "control %s for %s received %'[s\n", word, d->name, ar );
	else
		Debug( "global control %s received %'[s\n", word, ar );
	if (ar[0]) {
		if (fd >= 0 && !strcmp( ar[0], "caps" )) {
			if (ar[1])
				goto exce;
			Reply( "ok\tkdm\tlist\t" );
			if (bootManager != BO_NONE)
				Reply( "bootoptions\t" );
			if (d) {
				if ((d->displayType & d_location) == dLocal)
#ifdef HAVE_VTS
					Reply( "local\tactivate\t" );
#else
					Reply( "local\t" );
#endif
				if (d->allowShutdown != SHUT_NONE) {
					if (d->allowShutdown == SHUT_ROOT && d->userSess)
						Reply( "shutdown root\t" );
					else
						Reply( "shutdown\t" );
					if (d->allowNuke != SHUT_NONE) {
						if (d->allowNuke == SHUT_ROOT && d->userSess)
							Reply( "nuke root\t" );
						else
							Reply( "nuke\t" );
					}
				}
				if ((d->displayType & d_location) == dLocal &&
				    AnyReserveDisplays())
					Writer( fd, cbuf, sprintf( cbuf, "reserve %d\t",
					                           idleReserveDisplays() ) );
				Reply( "lock\tsuicide\n" );
			} else {
				if (fifoAllowShutdown) {
					Reply( "shutdown\t" );
					if (fifoAllowNuke)
						Reply( "nuke\t" );
				}
				if (AnyReserveDisplays())
					Writer( fd, cbuf, sprintf( cbuf, "reserve %d\t",
					                           idleReserveDisplays() ) );
#ifdef HAVE_VTS
				Reply( "login\tactivate\n" );
#else
				Reply( "login\n" );
#endif
			}
			goto bust;
		} else if (fd >= 0 && !strcmp( ar[0], "list" )) {
			int all = 0;
			if (ar[1]) {
				if (!strcmp( ar[1], "all" ))
					all = 1;
				else if (!strcmp( ar[1], "alllocal" ))
					all = 3;
				else {
					fLog( d, fd, "bad", "invalid list scope %\"s", ar[1] );
					goto bust;
				}
				if (ar[2])
					goto exce;
			}
			Reply( "ok" );
			for (di = displays; di; di = di->next) {
				if (((all & 2) && (di->displayType & d_location) != dLocal) ||
				    (di->status != remoteLogin &&
				     ((all & 1) ? di->status != running : di->userSess < 0)))
					continue;
				bp = cbuf;
				*bp++ = '\t';
				args = di->name;
				if (!memcmp( args, "localhost:", 10 ))
					args += 9;
				str_cat_l( &bp, args, sizeof(cbuf)/2 );
				*bp++ = ',';
#ifdef HAVE_VTS
				if (di->serverVT)
					bp += sprintf( bp, "vt%d", di->serverVT );
#endif
				*bp++ = ',';
				if (di->userName)
					str_cat_l( &bp, di->userName, sizeof(cbuf)/5 );
				*bp++ = ',';
				if (di->status == remoteLogin)
					str_cat( &bp, "<remote>" );
				else if (di->sessName)
					str_cat_l( &bp, di->sessName, sizeof(cbuf)/5 );
				*bp++ = ',';
				if (di == d)
					*bp++ = '*';
				if (di->userSess >= 0 &&
				    (d ? (d->userSess != di->userSess &&
				          (d->allowNuke == SHUT_NONE ||
				           (d->allowNuke == SHUT_ROOT && d->userSess))) :
				     !fifoAllowNuke))
					*bp++ = '!';
				Writer( fd, cbuf, bp - cbuf );
			}
			Reply( "\n" );
			goto bust;
		} else if (!strcmp( ar[0], "reserve" )) {
			int lt = 60; /* XXX make default timeout configurable? */
			if (ar[1]) {
				lt = strtol( ar[1], &bp, 10 );
				if (lt < 15 || *bp) {
					fLog( d, fd, "bad", "invalid timeout %\"s", ar[1] );
					goto bust;
				}
				if (ar[2])
					goto exce;
			}
			if (d && (d->displayType & d_location) != dLocal) {
				fLog( d, fd, "perm", "display is not local" );
				goto bust;
			}
			if (!StartReserveDisplay( lt )) {
				fLog( d, fd, "noent", "no reserve display available" );
				goto bust;
			}
#ifdef HAVE_VTS
		} else if (!strcmp( ar[0], "activate" )) {
			int vt;
			if (!ar[1])
				goto miss;
			if (ar[2])
				goto exce;
			if (d && (d->displayType & d_location) != dLocal) {
				fLog( d, fd, "perm", "display is not local" );
				goto bust;
			}
			if (ar[1][0] != 'v' || ar[1][1] != 't' ||
			    (vt = atoi( ar[1] + 2 )) <= 0)
			{
				if (!(di = FindDisplayByName( ar[1] ))) {
					fLog( d, fd, "noent", "display not found" );
					goto bust;
				}
				if ((di->displayType & d_location) != dLocal) {
					fLog( d, fd, "inval", "target display is not local" );
					goto bust;
				}
				if (!di->serverVT) {
					fLog( d, fd, "noent", "target display has no VT assigned" );
					goto bust;
				}
				vt = di->serverVT;
			}
			if (!activateVT( vt )) {
				fLog( d, fd, "inval", "VT switch failed" );
				goto bust;
			}
#endif
		} else if (!strcmp( ar[0], "shutdown" )) {
			ap = ar;
			if (!*++ap)
				goto miss;
			sdr.force = SHUT_CANCEL;
			sdr.osname = 0;
			if (!strcmp( *ap, "status" )) {
				if (fd < 0)
					goto bust;
				if (*++ap)
					goto exce;
				bp = cbuf;
				*bp++ = 'o';
				*bp++ = 'k';
				if (sdRec.how) {
					str_cat( &bp, "\tglobal," );
					sd_cat( &bp, &sdRec );
				}
				if (d && d->hstent->sdRec.how) {
					str_cat( &bp, "\tlocal," );
					sd_cat( &bp, &d->hstent->sdRec );
				}
				*bp++ = '\n';
				Writer( fd, cbuf, bp - cbuf );
				goto bust;
			} else if (!strcmp( *ap, "cancel" )) {
				sdr.how = 0;
				sdr.start = 0;
				if (ap[1]) {
					if (!d)
						goto exce;
					if (!strcmp( *++ap, "global" ))
						sdr.start = TO_INF;
					else if (strcmp( *ap, "local" )) {
						fLog( d, fd, "bad", "invalid cancel scope %\"s", *ap );
						goto bust;
					}
				}
			} else {
				if (!strcmp( *ap, "reboot" ))
					sdr.how = SHUT_REBOOT;
				else if (!strcmp( *ap, "halt" ))
					sdr.how = SHUT_HALT;
				else {
					fLog( d, fd, "bad", "invalid type %\"s", *ap );
					goto bust;
				}
				sdr.uid = -1;
				if (!*++ap)
					goto miss;
				if (**ap == '=') {
					switch (setBootOption( *ap + 1, &sdr )) {
					case BO_NOMAN:
						fLog( d, fd, "notsup", "boot options unavailable" );
						goto bust;
					case BO_NOENT:
						fLog( d, fd, "noent", "no such boot option" );
						goto bust;
					case BO_IO:
						fLog( d, fd, "io", "io error" );
						goto bust;
					}
					if (!*++ap)
						goto miss;
				}
				sdr.start = strtol( *ap, &bp, 10 );
				if (bp != *ap && !*bp) {
					if (**ap == '+')
						sdr.start += now;
					if (!*++ap)
						goto miss;
					sdr.timeout = strtol( *ap, &bp, 10 );
					if (bp == *ap || *bp) {
						fLog( d, fd, "bad", "invalid timeout %\"s", ar[3] );
						goto bust;
					}
					if (**ap == '+')
						sdr.timeout += sdr.start ? sdr.start : now;
					if (sdr.timeout < 0)
						sdr.timeout = TO_INF;
					else {
						if (!*++ap)
							goto miss;
						if (!strcmp( *ap, "force" ))
							sdr.force = SHUT_FORCE;
						else if (d && !strcmp( *ap, "forcemy" ))
							sdr.force = SHUT_FORCEMY;
						else if (strcmp( *ap, "cancel" )) {
							fLog( d, fd, "bad", "invalid timeout action %\"s",
							      *ap );
							goto bust;
						}
					}
				} else {
					sdr.timeout = 0;
					if (d && !strcmp( *ap, "ask" ))
						sdr.force = SHUT_ASK;
					else if (!strcmp( *ap, "forcenow" ))
						sdr.force = SHUT_FORCE;
					else if (!strcmp( *ap, "schedule" ))
						sdr.timeout = TO_INF;
					else if (strcmp( *ap, "trynow" )) {
						fLog( d, fd, "bad", "invalid mode %\"s", *ap );
						goto bust;
					}
				}
			}
			if (*++ap)
				goto exce;
			if (d) {
				sdr.uid = d->userSess >= 0 ? d->userSess : 0;
				if (d->allowShutdown == SHUT_NONE ||
				    (d->allowShutdown == SHUT_ROOT && sdr.uid &&
				     sdr.force != SHUT_ASK))
				{
					fLog( d, fd, "perm", "shutdown forbidden" );
					goto bust;
				}
				if (!sdr.how && !sdr.start) {
					if (d->hstent->sdRec.osname)
						free( d->hstent->sdRec.osname );
					d->hstent->sdRec = sdr;
				} else {
					if (sdRec.how && sdRec.force == SHUT_FORCE &&
					    ((d->allowNuke == SHUT_NONE && sdRec.uid != sdr.uid) ||
					     (d->allowNuke == SHUT_ROOT && sdr.uid)))
					{
						fLog( d, fd, "perm", "overriding forced shutdown forbidden" );
						goto bust;
					}
					if (sdr.force == SHUT_FORCE &&
					    (d->allowNuke == SHUT_NONE ||
					     (d->allowNuke == SHUT_ROOT && sdr.uid)))
					{
						fLog( d, fd, "perm", "forced shutdown forbidden" );
						goto bust;
					}
					if (!sdr.start) {
						if (d->hstent->sdRec.osname)
							free( d->hstent->sdRec.osname );
						d->hstent->sdRec = sdr;
					} else {
						if (!sdr.how)
							cancelShutdown();
						else {
							if (sdRec.osname)
								free( sdRec.osname );
							sdRec = sdr;
						}
					}
				}
			} else {
				if (!fifoAllowShutdown) {
					fLog( d, fd, "perm", "shutdown forbidden" );
					goto bust;
				}
				if (sdRec.how && sdRec.force == SHUT_FORCE &&
				    sdRec.uid != -1 && !fifoAllowNuke)
				{
					fLog( d, fd, "perm", "overriding forced shutdown forbidden" );
					goto bust;
				}
				if (!sdr.how)
					cancelShutdown();
				else {
					if (sdr.force != SHUT_CANCEL) {
						if (!fifoAllowNuke) {
							fLog( d, fd, "perm", "forced shutdown forbidden" );
							goto bust;
						}
					} else {
						if (!sdr.start && !sdr.timeout && AnyActiveDisplays()) {
							fLog( d, fd, "busy", "user sessions running" );
							goto bust;
						}
					}
					sdr.uid = -1;
					if (sdRec.osname)
						free( sdRec.osname );
					sdRec = sdr;
				}
			}
		} else if (fd >= 0 && !strcmp( ar[0], "listbootoptions" )) {
			char **opts;
			int def, cur, i, j;

			if (ar[1])
				goto exce;
			switch (getBootOptions( &opts, &def, &cur )) {
			case BO_NOMAN:
				fLog( d, fd, "notsup", "boot options unavailable" );
				goto bust;
			}
			Reply( "ok\t" );
			for (i = 0; opts[i]; i++) {
				bp = cbuf;
				if (i)
					*bp++ = ' ';
				for (j = 0; opts[i][j]; j++)
					if (opts[i][j] == ' ') {
						*bp++ = '\\';
						*bp++ = 's';
					} else
						*bp++ = opts[i][j];
				Writer( fd, cbuf, bp - cbuf );
			}
			freeStrArr( opts );
			Writer( fd, cbuf, sprintf( cbuf, "\t%d\t%d\n", def, cur ) );
			goto bust;
		} else if (d) {
			if (!strcmp( ar[0], "lock" )) {
				if (ar[1])
					goto exce;
				d->hstent->lock = 1;
			} else if (!strcmp( ar[0], "unlock" )) {
				if (ar[1])
					goto exce;
				d->hstent->lock = 0;
			} else if (!strcmp( ar[0], "suicide" )) {
				if (ar[1])
					goto exce;
				if (d->status == running && d->pid != -1) {
					TerminateProcess( d->pid, SIGTERM );
					d->status = raiser;
				}
			} else {
				fLog( d, fd, "nosys", "unknown command" );
				goto bust;
			}
		} else {
			if (!strcmp( ar[0], "login" )) {
				int nuke;
				if (arrLen( ar ) < 5) {
				  miss:
					fLog( d, fd, "bad", "missing argument(s)" );
					goto bust;
				}
				if (!(di = FindDisplayByName( ar[1] ))) {
					fLog( d, fd, "noent", "display %s not found", ar[1] );
					goto bust;
				}
				if (ar[5]) {
					if (!(args = unQuote( ar[5] ))) {
						fLog( d, fd, "nomem", "out of memory" );
						goto bust;
					}
					if (ar[6]) {
					  exce:
						fLog( d, fd, "bad", "excess argument(s)" );
						goto bust;
					}
					setNLogin( di, ar[3], ar[4], args, 2 );
					free( args );
				} else
					setNLogin( di, ar[3], ar[4], 0, 2 );
				nuke = !strcmp( ar[2], "now" );
				switch (di->status) {
				case running:
					if (di->pid != -1 && (di->userSess < 0 || nuke)) {
						TerminateProcess( di->pid, SIGTERM );
						di->status = raiser;
					}
					break;
				case remoteLogin:
					if (di->serverPid != -1 && nuke)
						TerminateProcess( di->serverPid, d->termSignal );
					break;
				case reserve:
					di->status = notRunning;
					break;
				case textMode:
#ifndef HAVE_VTS
					SwitchToX( di );
#endif
					break;
				default:
					break;
				}
			} else {
				fLog( d, fd, "nosys", "unknown command" );
				goto bust;
			}
		}
		if (fd >= 0)
			Reply( "ok\n" );
	}
  bust:
	freeStrArr( ar );
}

static int
handleChan( struct display *d, struct bsock *cs, int fd, FD_TYPE *reads )
{
	char *bufp, *nbuf, *obuf, *eol;
	int len, bl, llen;
	char buf[1024];

	bl = cs->buflen;
	obuf = cs->buffer;
	if (bl <= 0 && FD_ISSET( cs->fd, reads )) {
		FD_CLR( cs->fd, reads );
		bl = -bl;
		memcpy( buf, obuf, bl );
		if ((len = Reader( cs->fd, buf + bl, sizeof(buf) - bl )) <= 0)
			return -1;
		bl += len;
		bufp = buf;
	} else {
		len = 0;
		bufp = obuf;
	}
	if (bl > 0) {
		if ((eol = memchr( bufp, '\n', bl ))) {
			llen = eol - bufp + 1;
			bl -= llen;
			if (bl) {
				if (!(nbuf = Malloc( bl )))
					return -1;
				memcpy( nbuf, bufp + llen, bl );
			} else
				nbuf = 0;
			cs->buffer = nbuf;
			cs->buflen = bl;
			processCtrl( bufp, llen - 1, fd, d );
			if (obuf)
				free( obuf );
			return 1;
		} else if (!len) {
			if (fd >= 0)
				cs->buflen = -bl;
			else
				fLog( d, -1, "bad", "unterminated command" );
		}
	}
	return 0;
}

int
handleCtrl( FD_TYPE *reads, struct display *d )
{
	CtrlRec *cr = d ? &d->ctrl : &ctrl;
	struct cmdsock *cs, **csp;

	if (cr->fifo.fd >= 0) {
		switch (handleChan( d, &cr->fifo, -1, reads )) {
		case -1:
			if (cr->fifo.buffer)
				free( cr->fifo.buffer );
			cr->fifo.buflen = 0;
			break;
		case 1:
			return 1;
		default:
			break;
		}
	}
	if (cr->fd >= 0 && FD_ISSET( cr->fd, reads ))
		acceptSock( cr );
	else {
		for (csp = &cr->css; (cs = *csp); ) {
			switch (handleChan( d, &cs->sock, cs->sock.fd, reads )) {
			case -1:
				*csp = cs->next;
				nukeSock( cs );
				continue;
			case 1:
				return 1;
			default:
				break;
			}
			csp = &cs->next;
		}
	}
	return 0;
}
