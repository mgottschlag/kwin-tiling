/*

Copyright 2005 Stephan Kulow <coolo@kde.org>
Copyright 2005 Oswald Buddenhagen <ossi@kde.org>

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
 * Boot options
 */

#include "dm.h"
#include "dm_error.h"

#include <X11/Xosdefs.h>
#ifndef X_NOT_STDC_ENV
# include <string.h>
# include <unistd.h>
#endif

#include <stdio.h>
#include <ctype.h>

extern char **environ;

static int
getNull( char ***opts ATTR_UNUSED, int *def ATTR_UNUSED, int *cur ATTR_UNUSED )
{
	return BO_NOMAN;
}

static int
setNull( const char *opt ATTR_UNUSED, BoRec *bo ATTR_UNUSED )
{
	return BO_NOMAN;
}

static char *
match( char *obuf, int *blen, const char *key, int klen )
{
	char *buf = obuf;
	if (memcmp( buf, key, klen ) || !isspace( buf[klen] ))
		return 0;
	buf += klen + 1;
	for (; isspace( *buf ); buf++);
	if (!*buf)
		return 0;
	*blen -= buf - obuf;
	return buf;
}

#define GRUB_MENU "/boot/grub/menu.lst"

static char *grub;

static int
getGrub( char ***opts, int *def, int *cur )
{
	FILE *f;
	char *ptr;
	int len;
	char line[1000];

	if (!grub && !(grub = locate( "grub" )))
		return BO_NOMAN;

	*def = 0;
	*cur = -1;
	*opts = initStrArr( 0 );

	if (!(f = fopen( GRUB_MENU, "r" )))
		return errno == ENOENT ? BO_NOMAN : BO_IO;
	while ((len = fGets( line, sizeof(line), f )) != -1)
		if ((ptr = match( line, &len, "default", 7 )))
			*def = atoi( ptr );
		else if ((ptr = match( line, &len, "title", 5 ))) {
			for (; isspace( ptr[len - 1] ); len--);
			*opts = addStrArr( *opts, ptr, len );
		}
	fclose( f );

	return BO_OK;
}

static int
setGrub( const char *opt, BoRec *bo )
{
	FILE *f;
	char *ptr;
	int len, i;
	char line[1000];

	if (!(f = fopen( GRUB_MENU, "r" )))
		return errno == ENOENT ? BO_NOMAN : BO_IO;
	for (i = 0; (len = fGets( line, sizeof(line), f )) != -1; )
		if ((ptr = match( line, &len, "title", 5 ))) {
			if (!strcmp( ptr, opt )) {
				fclose( f );
				bo->index = i;
				bo->stamp = mTime( GRUB_MENU );
				return BO_OK;
			}
			i++;
		}
	fclose( f );
	return BO_NOENT;
}

static void
commitGrub( void )
{
	FILE *f;
	int pid;
	static const char *args[] = { 0, "--batch", 0 };

	if (boRec.stamp != mTime( GRUB_MENU ) &&
	    setGrub( boRec.name, &boRec ) != BO_OK)
		return;

	args[0] = grub;
	if ((f = pOpen( (char **)args, 'w', &pid ))) {
		fprintf( f, "savedefault --default=%d --once\n", boRec.index );
		pClose( f, pid );
	}
}

static char *lilo;

static int
getLilo( char ***opts, int *def, int *cur )
{
	FILE *f;
	int cdef, pid, len, ret = BO_OK;
	static const char *args[5] = { 0, "-w", "-v", "-q", 0 };
	char buf[256], next[256];

	if (!lilo && !(lilo = locate( "lilo" )))
		return BO_NOMAN;

	args[0] = lilo;
	if (!(f = pOpen( (char **)args, 'r', &pid )))
		return BO_IO;
	*opts = 0;
	next[0] = 0;
	for (;;) {
		if ((len = fGets( buf, sizeof(buf), f)) == -1) {
			ret = BO_NOMAN;
			goto out;
		}
		if (!memcmp( buf, "Images:", 7 ))
			break;
#define Ldeflin "  Default boot command line:"
		if (!memcmp( buf, Ldeflin, strlen(Ldeflin) )) {
			memcpy( next, buf + strlen(Ldeflin) + 2, len - strlen(Ldeflin) - 3 );
			next[len - strlen(Ldeflin) - 3] = 0;
		}
	}
	cdef = *def = 0;
	*cur = -1;
	*opts = initStrArr( 0 );
	while ((len = fGets( buf, sizeof(buf), f)) != -1)
		if (buf[0] == ' ' && buf[1] == ' ' && buf[2] != ' ') {
			if (buf[len - 1] == '*') {
				*def = cdef;
				len--;
			}
			for (; buf[len - 1] == ' '; len--);
			*opts = addStrArr( *opts, buf + 2, len - 2 );
			if (!strcmp( (*opts)[cdef], next ))
				*cur = cdef;
			cdef++;
		}
  out:
	if (pClose( f, pid )) {
		if (*opts)
			freeStrArr( *opts );
		return BO_IO;
	}
	return ret;
}

static int
setLilo( const char *opt, BoRec *bo ATTR_UNUSED )
{
	char **opts;
	int def, cur, ret, i;

	if ((ret = getLilo( &opts, &def, &cur )) != BO_OK)
		return ret;
	if (!*opt)
		opt = 0;
	else {
		for (i = 0; opts[i]; i++)
			if (!strcmp( opts[i], opt ))
				goto oke;
		freeStrArr( opts );
		return BO_NOENT;
	}
  oke:
	freeStrArr( opts );
	return BO_OK;
}

static void
commitLilo( void )
{
	static const char *args[5] = { 0, "-w", "-R", 0, 0 };

	args[0] = lilo;
	args[3] = boRec.name;
	runAndWait( (char **)args, environ );
}

static struct {
	int (*get)( char ***, int *, int * );
	int (*set)( const char *, BoRec * );
	void (*commit)( void );
} bootOpts[] = {
  { getNull, setNull, 0 },
  { getGrub, setGrub, commitGrub },
  { getLilo, setLilo, commitLilo },
};

int
getBootOptions( char ***opts, int *def, int *cur )
{
	return bootOpts[bootManager].get( opts, def, cur );
}

int
setBootOption( const char *opt, BoRec *bo )
{
	int ret;

	if (bo->name) {
		free( bo->name );
		bo->name = 0;
	}
	if (opt) {
		if ((ret = bootOpts[bootManager].set( opt, bo )) != BO_OK)
			return ret;
		if (!StrDup( &bo->name, opt ))
			return BO_IO; /* BO_NOMEM */
	}
	return BO_OK;
}

void
commitBootOption( void )
{
	if (boRec.name) {
		bootOpts[bootManager].commit();
/*
		free( boRec->name );
		boRec->name = 0;
*/
	}
}

