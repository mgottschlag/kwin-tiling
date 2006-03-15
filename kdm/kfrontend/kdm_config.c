/*

Read options from kdmrc

Copyright (C) 2001-2005 Oswald Buddenhagen <ossi@kde.org>


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <config.h>

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <grp.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
# include <sched.h>
#endif

#include <X11/X.h>
#ifdef FamilyInternet6
# define IPv6
#endif

#include <greet.h>
#include <config.ci>

/*
 * Section/Entry definition structs
 */

typedef struct Ent {
	const char *name;
	int id;
	void *ptr;
	const char *def;
} Ent;

typedef struct Sect {
	const char *name;
	Ent *ents;
	int numents;
} Sect;

/*
 * Parsed ini file structs
 */

typedef struct Entry {
	struct Entry *next;
	const char *val;
	Ent *ent;
	int vallen;
	int line;
} Entry;

typedef struct Section {
	struct Section *next;
	Entry *entries;
	Sect *sect;
	const char *name, *dname, *dhost, *dnum, *dclass;
	int nlen, dlen, dhostl, dnuml, dclassl;
} Section;


/*
 * Split up display-name/-class for fast comparison
 */
typedef struct DSpec {
	const char *dhost, *dnum, *dclass;
	int dhostl, dnuml, dclassl;
} DSpec;


/*
 * Config value storage structures
 */

typedef struct Value {
	const char *ptr;
	int len;
} Value;

typedef struct Val {
	Value val;
	int id;
} Val;

typedef struct ValArr {
	Val *ents;
	int nents, esiz, nchars, nptrs;
} ValArr;


static void *Malloc( size_t size );
static void *Realloc( void *ptr, size_t size );

#define PRINT_QUOTES
#define LOG_NAME "kdm_config"
#define LOG_DEBUG_MASK DEBUG_CONFIG
#define LOG_PANIC_EXIT 1
#define STATIC static
#include <printf.c>


static void *
Malloc( size_t size )
{
	void *ret;

	if (!(ret = malloc( size )))
		LogOutOfMem();
	return ret;
}

static void *
Realloc( void *ptr, size_t size )
{
	void *ret;

	if (!(ret = realloc( ptr, size )) && size)
		LogOutOfMem();
	return ret;
}


static void
MkDSpec( DSpec *spec, const char *dname, const char *dclass )
{
	spec->dhost = dname;
	for (spec->dhostl = 0; dname[spec->dhostl] != ':'; spec->dhostl++);
	spec->dnum = dname + spec->dhostl + 1;
	spec->dnuml = strlen( spec->dnum );
	spec->dclass = dclass;
	spec->dclassl = strlen( dclass );
}


static int rfd, wfd;

static int
Reader( void *buf, int count )
{
	int ret, rlen;

	for (rlen = 0; rlen < count; ) {
	  dord:
		ret = read( rfd, (void *)((char *)buf + rlen), count - rlen );
		if (ret < 0) {
			if (errno == EINTR)
				goto dord;
			if (errno == EAGAIN)
				break;
			return -1;
		}
		if (!ret)
			break;
		rlen += ret;
	}
	return rlen;
}

static void
GRead( void *buf, int count )
{
	if (Reader( buf, count ) != count)
		LogPanic( "Can't read from core\n" );
}

static void
GWrite( const void *buf, int count )
{
	if (write( wfd, buf, count ) != count)
		LogPanic( "Can't write to core\n" );
#ifdef _POSIX_PRIORITY_SCHEDULING
	if ((debugLevel & DEBUG_HLPCON))
		sched_yield();
#endif
}

static void
GSendInt( int val )
{
	GWrite( &val, sizeof(val) );
}

static void
GSendStr( const char *buf )
{
	if (buf) {
		int len = strlen( buf ) + 1;
		GWrite( &len, sizeof(len) );
		GWrite( buf, len );
	} else
		GWrite( &buf, sizeof(int));
}

static void
GSendNStr( const char *buf, int len )
{
	int tlen = len + 1;
	GWrite( &tlen, sizeof(tlen) );
	GWrite( buf, len );
	GWrite( "", 1 );
}

#ifdef XDMCP
static void
GSendArr( int len, const char *data )
{
	GWrite( &len, sizeof(len) );
	GWrite( data, len );
}
#endif

static int
GRecvCmd( int *val )
{
	if (Reader( val, sizeof(*val) ) != sizeof(*val))
		return 0;
	return 1;
}

static int
GRecvInt()
{
	int val;

	GRead( &val, sizeof(val) );
	return val;
}

static char *
GRecvStr()
{
	int len;
	char *buf;

	len = GRecvInt();
	if (!len)
		return 0;
	if (!(buf = malloc( len )))
		LogPanic( "No memory for read buffer" );
	GRead( buf, len );
	return buf;
}


/* #define WANT_CLOSE 1 */

typedef struct File {
	char *buf, *eof, *cur;
#if defined(HAVE_MMAP) && defined(WANT_CLOSE)
	int ismapped;
#endif
} File;

static int
readFile( File *file, const char *fn, const char *what )
{
	int fd;
	off_t flen;

	if ((fd = open( fn, O_RDONLY )) < 0) {
		LogInfo( "Cannot open %s file %s\n", what, fn );
		return 0;
	}

	flen = lseek( fd, 0, SEEK_END );
#ifdef HAVE_MMAP
# ifdef WANT_CLOSE
	file->ismapped = 0;
# endif
	file->buf = mmap( 0, flen + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0 );
# ifdef WANT_CLOSE
	if (file->buf)
		file->ismapped = 1;
	else
# else
	if (!file->buf)
# endif
#endif
	{
		if (!(file->buf = Malloc( flen + 1 ))) {
			close( fd );
			return 0;
		}
		lseek( fd, 0, SEEK_SET );
		if (read( fd, file->buf, flen ) != flen) {
			free( file->buf );
			LogError( "Cannot read %s file %s\n", what, fn );
			close( fd );
			return 0;
		}
	}
	file->eof = (file->cur = file->buf) + flen;
	close( fd );
	return 1;
}

#ifdef WANT_CLOSE
static void
freeBuf( File *file )
{
# ifdef HAVE_MMAP
	if (file->ismapped)
		munmap( file->buf, file->eof - file->buf + 1 );
	else
# endif
		free( file->buf );
}
#endif

CONF_READ_VARS

#define C_MTYPE_MASK  0x30000000
# define C_PATH          0x10000000	/* C_TYPE_STR is a path spec */
# define C_BOOL          0x10000000	/* C_TYPE_INT is a boolean */
# define C_ENUM          0x20000000	/* C_TYPE_INT is an enum (option) */
# define C_GRP           0x30000000	/* C_TYPE_INT is a group spec */
#define C_INTERNAL    0x40000000	/* don't expose to core */
#define C_CONFIG      0x80000000	/* process only for finding deps */

#ifdef XDMCP
static int
PrequestPort( Value *retval )
{
	if (!VxdmcpEnable.ptr) {
		retval->ptr = (char *)0;
		return 1;
	}
	return 0;
}
#endif

static Value
	emptyStr = { "", 1 },
	nullValue = { 0, 0 },
	emptyArgv = { (char *)&nullValue, 0 };

static int
PnoPassUsers( Value *retval )
{
	if (!VnoPassEnable.ptr) {
		*retval = emptyArgv;
		return 1;
	}
	return 0;
}

static int
PautoLoginX( Value *retval )
{
	if (!VautoLoginEnable.ptr) {
		*retval = emptyStr;
		return 1;
	}
	return 0;
}

CONF_READ_ENTRIES

static const char *kdmrc = KDMCONF "/kdmrc";

static Section *rootsec;

static void
ReadConf()
{
	const char *nstr, *dstr, *cstr, *dhost, *dnum, *dclass;
	char *s, *e, *st, *en, *ek, *sl, *pt;
	Section *cursec;
	Entry *curent;
	Ent *ce;
	int nlen, dlen, clen, dhostl, dnuml, dclassl;
	int i, line, sectmoan, restl;
	File file;
	static int confread;

	if (confread)
		return;
	confread = 1;

	Debug( "reading config %s ...\n", kdmrc );
	if (!readFile( &file, kdmrc, "master configuration" ))
		return;

	for (s = file.buf, line = 0, cursec = 0, sectmoan = 1; s < file.eof; s++) {
		line++;

		while ((s < file.eof) && isspace( *s ) && (*s != '\n'))
			s++;

		if ((s < file.eof) && ((*s == '\n') || (*s == '#'))) {
		  sktoeol:
			while ((s < file.eof) && (*s != '\n'))
				s++;
			continue;
		}
		sl = s;

		if (*s == '[') {
			sectmoan = 0;
			while ((s < file.eof) && (*s != '\n'))
				s++;
			e = s - 1;
			while ((e > sl) && isspace( *e ))
				e--;
			if (*e != ']') {
				cursec = 0;
				LogError( "Invalid section header at %s:%d\n", kdmrc, line );
				continue;
			}
			nstr = sl + 1;
			nlen = e - nstr;
			for (cursec = rootsec; cursec; cursec = cursec->next)
				if (nlen == cursec->nlen &&
				    !memcmp( nstr, cursec->name, nlen ))
				{
					LogInfo( "Multiple occurrences of section [%.*s] in %s. "
					         "Consider merging them.\n", nlen, nstr, kdmrc );
					goto secfnd;
				}
			if (nstr[0] == 'X' && nstr[1] == '-') {
				cstr = nstr + nlen;
				clen = 0;
				while (++clen, *--cstr != '-');
				if (cstr == nstr + 1)
					goto illsec;
				dstr = nstr + 2;
				dlen = nlen - clen - 2;
				dhost = dstr;
				dhostl = 0;
				for (restl = dlen; restl; restl--) {
					if (dhost[dhostl] == ':') {
						dnum = dhost + dhostl + 1;
						dnuml = 0;
						for (restl--; restl; restl--) {
							if (dnum[dnuml] == '_') {
								dclass = dnum + dnuml + 1;
								dclassl = restl;
								goto gotall;
							}
							dnuml++;
						}
						goto gotnum;
					}
					dhostl++;
				}
				dnum = "*";
				dnuml = 1;
			  gotnum:
				dclass = "*";
				dclassl = 1;
			  gotall: ;
			} else {
				if (nstr[0] == '-')
					goto illsec;
				dstr = 0;
				dlen = 0;
				dhost = 0;
				dhostl = 0;
				dnum = 0;
				dnuml = 0;
				dclass = 0;
				dclassl = 0;
				cstr = nstr;
				clen = nlen;
			}
			for (i = 0; i < as(allSects); i++)
				if ((int)strlen( allSects[i]->name ) == clen &&
				    !memcmp( allSects[i]->name, cstr, clen ))
					goto newsec;
		  illsec:
			cursec = 0;
			LogError( "Unrecognized section name [%.*s] at %s:%d\n",
			          nlen, nstr, kdmrc, line );
			continue;
		  newsec:
			if (!(cursec = Malloc( sizeof(*cursec) )))
				return;
			cursec->name = nstr;
			cursec->nlen = nlen;
			cursec->dname = dstr;
			cursec->dlen = dlen;
			cursec->dhost = dhost;
			cursec->dhostl = dhostl;
			cursec->dnum = dnum;
			cursec->dnuml = dnuml;
			cursec->dclass = dclass;
			cursec->dclassl = dclassl;
			cursec->sect = allSects[i];
			cursec->entries = 0;
			cursec->next = rootsec;
			rootsec = cursec;
			/*Debug( "now in section [%.*s], dpy '%.*s', core '%.*s'\n",
			       nlen, nstr, dlen, dstr, clen, cstr );*/
		  secfnd:
			continue;
		}

		if (!cursec) {
			if (sectmoan) {
				sectmoan = 0;
				LogError( "Entry outside any section at %s:%d", kdmrc, line );
			}
			goto sktoeol;
		}

		for (; (s < file.eof) && (*s != '\n'); s++)
			if (*s == '=')
				goto haveeq;
		LogError( "Invalid entry (missing '=') at %s:%d\n", kdmrc, line );
		continue;

	  haveeq:
		for (ek = s - 1; ; ek--) {
			if (ek < sl) {
				LogError( "Invalid entry (empty key) at %s:%d\n", kdmrc, line );
				goto sktoeol;
			}
			if (!isspace( *ek ))
				break;
		}

		s++;
		while ((s < file.eof) && isspace( *s ) && (*s != '\n'))
			s++;
		for (pt = st = en = s; s < file.eof && *s != '\n'; s++) {
			if (*s == '\\') {
				s++;
				if (s >= file.eof || *s == '\n') {
					LogError( "Trailing backslash at %s:%d\n", kdmrc, line );
					break;
				}
				switch (*s) {
				case 's': *pt++ = ' '; break;
				case 't': *pt++ = '\t'; break;
				case 'n': *pt++ = '\n'; break;
				case 'r': *pt++ = '\r'; break;
				case '\\': *pt++ = '\\'; break;
				default: *pt++ = '\\'; *pt++ = *s; break;
				}
				en = pt;
			} else {
				*pt++ = *s;
				if (*s != ' ' && *s != '\t')
					en = pt;
			}
		}

		nstr = sl;
		nlen = ek - sl + 1;
		/*Debug( "read entry '%.*s'='%.*s'\n", nlen, nstr, en - st, st );*/
		for (i = 0; i < cursec->sect->numents; i++) {
			ce = cursec->sect->ents + i;
			if ((int)strlen( ce->name ) == nlen &&
			    !memcmp( ce->name, nstr, nlen ))
				goto keyok;
		}
		LogError( "Unrecognized key '%.*s' in section [%.*s] at %s:%d\n",
		          nlen, nstr, cursec->nlen, cursec->name, kdmrc, line );
		continue;
	  keyok:
		for (curent = cursec->entries; curent; curent = curent->next)
			if (ce == curent->ent) {
				LogError( "Multiple occurrences of key '%s' in section [%.*s]"
				          " of %s\n",
				          ce->name, cursec->nlen, cursec->name, kdmrc );
				goto keyfnd;
			}
		if (!(curent = Malloc( sizeof(*curent) )))
			return;
		curent->ent = ce;
		curent->line = line;
		curent->val = st;
		curent->vallen = en - st;
		curent->next = cursec->entries;
		cursec->entries = curent;
	  keyfnd:
		continue;
	}
}

static Entry *
FindGEnt( int id )
{
	Section *cursec;
	Entry *curent;

	for (cursec = rootsec; cursec; cursec = cursec->next)
		if (!cursec->dname)
			for (curent = cursec->entries; curent; curent = curent->next)
				if (curent->ent->id == id) {
					Debug( "line %d: %s = %'.*s\n",
					       curent->line, curent->ent->name,
					       curent->vallen, curent->val );
					return curent;
				}
	return 0;
}

/* Display name match scoring:
 * - class (any/exact) -> 0/1
 * - number (any/exact) -> 0/2
 * - host (any/nonempty/trail/exact) -> 0/4/8/12
 */
static Entry *
FindDEnt( int id, DSpec *dspec )
{
	Section *cursec, *bestsec;
	Entry *curent, *bestent;
	int score, bestscore;

	bestscore = -1, bestent = 0;
	for (cursec = rootsec; cursec; cursec = cursec->next)
		if (cursec->dname) {
			score = 0;
			if (cursec->dclassl != 1 || cursec->dclass[0] != '*') {
				if (cursec->dclassl == dspec->dclassl &&
				    !memcmp( cursec->dclass, dspec->dclass, dspec->dclassl ))
					score = 1;
				else
					continue;
			}
			if (cursec->dnuml != 1 || cursec->dnum[0] != '*') {
				if (cursec->dnuml == dspec->dnuml &&
				    !memcmp( cursec->dnum, dspec->dnum, dspec->dnuml ))
					score += 2;
				else
					continue;
			}
			if (cursec->dhostl != 1 || cursec->dhost[0] != '*') {
				if (cursec->dhostl == 1 && cursec->dhost[0] == '+') {
					if (dspec->dhostl)
						score += 4;
					else
						continue;
				} else if (cursec->dhost[0] == '.') {
					if (cursec->dhostl < dspec->dhostl &&
					    !memcmp( cursec->dhost,
					             dspec->dhost + dspec->dhostl - cursec->dhostl,
					             cursec->dhostl ))
						score += 8;
					else
						continue;
				} else {
					if (cursec->dhostl == dspec->dhostl &&
					    !memcmp( cursec->dhost, dspec->dhost, dspec->dhostl ))
						score += 12;
					else
						continue;
				}
			}
			if (score > bestscore) {
				for (curent = cursec->entries; curent; curent = curent->next)
					if (curent->ent->id == id) {
						bestent = curent;
						bestsec = cursec;
						bestscore = score;
						break;
					}
			}
		}
	if (bestent)
		Debug( "line %d: %.*s:%.*s_%.*s/%s = %'.*s\n", bestent->line,
		       bestsec->dhostl, bestsec->dhost,
		       bestsec->dnuml, bestsec->dnum,
		       bestsec->dclassl, bestsec->dclass,
		       bestent->ent->name, bestent->vallen, bestent->val );
	return bestent;
}

static const char *
CvtValue( Ent *et, Value *retval, int vallen, const char *val, char **eopts )
{
	Value *ents;
	int i, b, e, tlen, nents, esiz;
	char buf[80];

	switch (et->id & C_TYPE_MASK) {
		case C_TYPE_INT:
			for (i = 0; i < vallen && i < (int)sizeof(buf) - 1; i++)
				buf[i] = tolower( val[i] );
			buf[i] = 0;
			if ((et->id & C_MTYPE_MASK) == C_BOOL) {
				if (!strcmp( buf, "true" ) ||
				    !strcmp( buf, "on" ) ||
				    !strcmp( buf, "yes" ) ||
				    !strcmp( buf, "1" ))
						retval->ptr = (char *)1;
				else if (!strcmp( buf, "false" ) ||
				         !strcmp( buf, "off" ) ||
				         !strcmp( buf, "no" ) ||
				         !strcmp( buf, "0" ))
							retval->ptr = (char *)0;
				else
					return "boolean";
				return 0;
			} else if ((et->id & C_MTYPE_MASK) == C_ENUM) {
				for (i = 0; eopts[i]; i++)
					if (!memcmp( eopts[i], val, vallen ) && !eopts[i][vallen]) {
						retval->ptr = (char *)i;
						return 0;
					}
				return "option";
			} else if ((et->id & C_MTYPE_MASK) == C_GRP) {
				struct group *ge;
				if ((ge = getgrnam( buf ))) {
					retval->ptr = (char *)ge->gr_gid;
					return 0;
				}
			}
			retval->ptr = 0;
			if (sscanf( buf, "%li", (long *)&retval->ptr ) != 1)
				return "integer";
			return 0;
		case C_TYPE_STR:
			retval->ptr = val;
			retval->len = vallen + 1;
			if ((et->id & C_MTYPE_MASK) == C_PATH)
				if (vallen && val[vallen-1] == '/')
					retval->len--;
			return 0;
		case C_TYPE_ARGV:
			if (!(ents = Malloc( sizeof(Value) * (esiz = 10) )))
				return 0;
			for (nents = 0, tlen = 0, i = 0; ; i++) {
				for (; i < vallen && isspace( val[i] ); i++);
				for (b = i; i < vallen && val[i] != ','; i++);
				if (b == i)
					break;
				for (e = i; e > b && isspace( val[e - 1] ); e--);
				if (esiz < nents + 2) {
					Value *entsn = Realloc( ents,
					                        sizeof(Value) * (esiz = esiz * 2 + 1) );
					if (!nents)
						break;
					ents = entsn;
				}
				ents[nents].ptr = val + b;
				ents[nents].len = e - b;
				nents++;
				tlen += e - b + 1;
			}
			ents[nents].ptr = 0;
			retval->ptr = (char *)ents;
			retval->len = tlen;
			return 0;
		default:
			LogError( "Internal error: unknown value type in id %#x\n", et->id );
			return 0;
	}
}

static void
GetValue( Ent *et, DSpec *dspec, Value *retval, char **eopts )
{
	Entry *ent;
	const char *errs;

/*	Debug( "Getting value %#x\n", et->id );*/
	if (dspec)
		ent = FindDEnt( et->id, dspec );
	else
		ent = FindGEnt( et->id );
	if (ent) {
		if (!(errs = CvtValue( et, retval, ent->vallen, ent->val, eopts )))
			return;
		LogError( "Invalid %s value '%.*s' at %s:%d\n",
		          errs, ent->vallen, ent->val, kdmrc, ent->line );
	}
	Debug( "default: %s = %'s\n", et->name, et->def );
	if ((errs = CvtValue( et, retval, strlen( et->def ), et->def, eopts )))
		LogError( "Internal error: invalid default %s value '%s' for key %s\n",
		          errs, et->def, et->name );
}

static int
AddValue( ValArr *va, int id, Value *val )
{
	int nu;

/*	Debug( "Addig value %#x\n", id );*/
	if (va->nents == va->esiz) {
		va->ents = Realloc( va->ents, sizeof(Val) * (va->esiz += 50) );
		if (!va->ents)
			return 0;
	}
	va->ents[va->nents].id = id;
	va->ents[va->nents].val = *val;
	va->nents++;
	switch (id & C_TYPE_MASK) {
		case C_TYPE_INT:
			break;
		case C_TYPE_STR:
			va->nchars += val->len;
			break;
		case C_TYPE_ARGV:
			va->nchars += val->len;
			for (nu = 0; ((Value *)val->ptr)[nu++].ptr; );
			va->nptrs += nu;
			break;
	}
	return 1;
}

static void
CopyValues( ValArr *va, Sect *sec, DSpec *dspec, int isconfig )
{
	Value val;
	int i;

	Debug( "getting values for section class [%s]\n", sec->name );
	for (i = 0; i < sec->numents; i++) {
/*Debug ("value %#x\n", sec->ents[i].id);*/
		if ((sec->ents[i].id & (int)C_CONFIG) != isconfig)
			;
		else if (sec->ents[i].id & C_INTERNAL) {
			GetValue( sec->ents + i, dspec, ((Value *)sec->ents[i].ptr), 0 );
		} else {
			if (((sec->ents[i].id & C_MTYPE_MASK) == C_ENUM) ||
			    !sec->ents[i].ptr ||
			    !((int (*)( Value * ))sec->ents[i].ptr)(&val)) {
				GetValue( sec->ents + i, dspec, &val,
				          (char **)sec->ents[i].ptr );
			}
			if (!AddValue( va, sec->ents[i].id, &val ))
				break;
		}
	}
	return;
}

static void
SendValues( ValArr *va )
{
	Value *cst;
	int i, nu;

	GSendInt( va->nents );
	GSendInt( va->nptrs );
	GSendInt( 0/*va->nints*/ );
	GSendInt( va->nchars );
	for (i = 0; i < va->nents; i++) {
		GSendInt( va->ents[i].id & ~C_PRIVATE );
		switch (va->ents[i].id & C_TYPE_MASK) {
		case C_TYPE_INT:
			GSendInt( (int)va->ents[i].val.ptr );
			break;
		case C_TYPE_STR:
			GSendNStr( va->ents[i].val.ptr, va->ents[i].val.len - 1 );
			break;
		case C_TYPE_ARGV:
			cst = (Value *)va->ents[i].val.ptr;
			for (nu = 0; cst[nu].ptr; nu++);
			GSendInt( nu );
			for (; cst->ptr; cst++)
				GSendNStr( cst->ptr, cst->len );
			break;
		}
	}
}


#ifdef XDMCP
static char *
ReadWord( File *file, int *len, int EOFatEOL )
{
	char *wordp, *wordBuffer;
	int quoted;
	char c;

  rest:
	wordp = wordBuffer = file->cur;
  mloop:
	quoted = 0;
  qloop:
	if (file->cur == file->eof) {
	  doeow:
		if (wordp == wordBuffer)
			return 0;
	  retw:
		*wordp = '\0';
		*len = wordp - wordBuffer;
		return wordBuffer;
	}
	c = *file->cur++;
	switch (c) {
	case '#':
		if (quoted)
			break;
		do {
			if (file->cur == file->eof)
				goto doeow;
			c = *file->cur++;
		} while (c != '\n');
	case '\0':
	case '\n':
		if (EOFatEOL && !quoted) {
			file->cur--;
			goto doeow;
		}
		if (wordp != wordBuffer) {
			file->cur--;
			goto retw;
		}
		goto rest;
	case ' ':
	case '\t':
		if (wordp != wordBuffer)
			goto retw;
		goto rest;
	case '\\':
		if (!quoted) {
			quoted = 1;
			goto qloop;
		}
		break;
	}
	*wordp++ = c;
	goto mloop;
}

#define ALIAS_CHARACTER     '%'
#define NEGATE_CHARACTER    '!'
#define CHOOSER_STRING      "CHOOSER"
#define BROADCAST_STRING    "BROADCAST"
#define NOBROADCAST_STRING  "NOBROADCAST"
#define LISTEN_STRING       "LISTEN"
#define WILDCARD_STRING     "*"

typedef struct hostEntry {
	struct hostEntry *next;
	int type;
	union _hostOrAlias {
		char *aliasPattern;
		char *hostPattern;
		struct _display {
			int connectionType;
			int hostAddrLen;
			char *hostAddress;
		} displayAddress;
	} entry;
} HostEntry;

typedef struct listenEntry {
	struct listenEntry *next;
	int iface;
	int mcasts;
	int nmcasts;
} ListenEntry;

typedef struct aliasEntry {
	struct aliasEntry *next;
	char *name;
	int hosts;
	int nhosts;
} AliasEntry;

typedef struct aclEntry {
	struct aclEntry *next;
	int entries;
	int nentries;
	int hosts;
	int nhosts;
	int flags;
} AclEntry;


static int
HasGlobCharacters( char *s )
{
	for (;;)
		switch (*s++) {
		case '?':
		case '*':
			return 1;
		case '\0':
			return 0;
		}
}

#define PARSE_ALL       0
#define PARSE_NO_BCAST  1
#define PARSE_NO_PAT    2
#define PARSE_NO_ALIAS  4

static int
ParseHost( int *nHosts, HostEntry ***hostPtr, int *nChars,
           char *hostOrAlias, int len, int parse )
{
#if defined(IPv6) && defined(AF_INET6)
	struct addrinfo *ai;
#else
	struct hostent *hostent;
#endif
	void *addr;
	int addr_type, addr_len;

	if (!(**hostPtr = (HostEntry *)Malloc( sizeof(HostEntry))))
		return 0;
	if (!(parse & PARSE_NO_BCAST) && !strcmp( hostOrAlias, BROADCAST_STRING ))
	{
		(**hostPtr)->type = HOST_BROADCAST;
	}
	else if (!(parse & PARSE_NO_ALIAS) && *hostOrAlias == ALIAS_CHARACTER)
	{
		(**hostPtr)->type = HOST_ALIAS;
		(**hostPtr)->entry.aliasPattern = hostOrAlias + 1;
		*nChars += len;
	}
	else if (!(parse & PARSE_NO_PAT) && HasGlobCharacters( hostOrAlias ))
	{
		(**hostPtr)->type = HOST_PATTERN;
		(**hostPtr)->entry.hostPattern = hostOrAlias;
		*nChars += len + 1;
	}
	else
	{
		(**hostPtr)->type = HOST_ADDRESS;
#if defined(IPv6) && defined(AF_INET6)
		if (getaddrinfo( hostOrAlias, NULL, NULL, &ai ))
#else
		if (!(hostent = gethostbyname( hostOrAlias )))
#endif
		{
			LogWarn( "Host \"%s\" not found\n", hostOrAlias );
			free( (char *)(**hostPtr) );
			return 0;
		}
#if defined(IPv6) && defined(AF_INET6)
		addr_type = ai->ai_addr->sa_family;
		if (ai->ai_family == AF_INET) {
			addr = &((struct sockaddr_in *)ai->ai_addr)->sin_addr;
			addr_len = sizeof(struct in_addr);
		} else /*if (ai->ai_addr->sa_family == AF_INET6)*/ {
			addr = &((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr;
			addr_len = sizeof(struct in6_addr);
		}
#else
		addr_type = hostent->h_addrtype;
		addr = hostent->h_addr;
		addr_len = hostent->h_length;
#endif
		if (!((**hostPtr)->entry.displayAddress.hostAddress =
		      Malloc( addr_len )))
		{
			free( (char *)(**hostPtr) );
			return 0;
		}
		memcpy( (**hostPtr)->entry.displayAddress.hostAddress, addr, addr_len );
		*nChars += addr_len;
		(**hostPtr)->entry.displayAddress.hostAddrLen = addr_len;
		(**hostPtr)->entry.displayAddress.connectionType = addr_type;
#if defined(IPv6) && defined(AF_INET6)
		freeaddrinfo( ai );
#endif
	}
	*hostPtr = &(**hostPtr)->next;
	(*nHosts)++;
	return 1;
}

static void
ReadAccessFile( const char *fname )
{
	HostEntry *hostList, **hostPtr = &hostList;
	AliasEntry *aliasList, **aliasPtr = &aliasList;
	AclEntry *acList, **acPtr = &acList;
	ListenEntry *listenList, **listenPtr = &listenList;
	char *displayOrAlias, *hostOrAlias;
	File file;
	int nHosts, nAliases, nAcls, nListens, nChars, error;
	int i, len;

	nHosts = nAliases = nAcls = nListens = nChars = error = 0;
	if (!readFile( &file, fname, "XDMCP access control" ))
		goto sendacl;
	while ((displayOrAlias = ReadWord( &file, &len, FALSE ))) {
		if (*displayOrAlias == ALIAS_CHARACTER)
		{
			if (!(*aliasPtr = (AliasEntry *)Malloc( sizeof(AliasEntry)))) {
				error = 1;
				break;
			}
			(*aliasPtr)->name = displayOrAlias + 1;
			nChars += len;
			(*aliasPtr)->hosts = nHosts;
			(*aliasPtr)->nhosts = 0;
			while ((hostOrAlias = ReadWord( &file, &len, TRUE ))) {
				if (!ParseHost( &nHosts, &hostPtr, &nChars, hostOrAlias, len,
				                PARSE_NO_BCAST|PARSE_NO_PAT ))
					goto sktoeol;
				(*aliasPtr)->nhosts++;
			}
			aliasPtr = &(*aliasPtr)->next;
			nAliases++;
		}
		else if (!strcmp( displayOrAlias, LISTEN_STRING ))
		{
			if (!(*listenPtr = (ListenEntry *)Malloc( sizeof(ListenEntry)))) {
				error = 1;
				break;
			}
			(*listenPtr)->iface = nHosts;
			if (!(hostOrAlias = ReadWord( &file, &len, TRUE )) ||
			    !strcmp( hostOrAlias, WILDCARD_STRING ) ||
			    !ParseHost( &nHosts, &hostPtr, &nChars, hostOrAlias, len,
			                PARSE_NO_BCAST|PARSE_NO_PAT|PARSE_NO_ALIAS ))
			{
				(*listenPtr)->iface = -1;
			}
			(*listenPtr)->mcasts = nHosts;
			(*listenPtr)->nmcasts = 0;
			while ((hostOrAlias = ReadWord( &file, &len, TRUE ))) {
				if (ParseHost( &nHosts, &hostPtr, &nChars, hostOrAlias, len,
				               PARSE_NO_BCAST|PARSE_NO_PAT|PARSE_NO_ALIAS ))
					(*listenPtr)->nmcasts++;
			}
			listenPtr = &(*listenPtr)->next;
			nListens++;
		}
		else
		{
			if (!(*acPtr = (AclEntry *)Malloc( sizeof(AclEntry)))) {
				error = 1;
				break;
			}
			(*acPtr)->flags = 0;
			if (*displayOrAlias == NEGATE_CHARACTER) {
				(*acPtr)->flags |= a_notAllowed;
				displayOrAlias++;
			}
			(*acPtr)->entries = nHosts;
			(*acPtr)->nentries = 1;
			if (!ParseHost( &nHosts, &hostPtr, &nChars, displayOrAlias, len,
			                PARSE_NO_BCAST ))
			{
				if ((*acPtr)->flags & a_notAllowed) {
				  sktoeol:
					error = 1;
				}
				while (ReadWord( &file, &len, TRUE ));
				continue;
			}
			(*acPtr)->hosts = nHosts;
			(*acPtr)->nhosts = 0;
			while ((hostOrAlias = ReadWord( &file, &len, TRUE ))) {
				if (!strcmp( hostOrAlias, CHOOSER_STRING ))
					(*acPtr)->flags |= a_useChooser;
				else if (!strcmp( hostOrAlias, NOBROADCAST_STRING ))
					(*acPtr)->flags |= a_notBroadcast;
				else {
					if (ParseHost( &nHosts, &hostPtr, &nChars,
					               hostOrAlias, len, PARSE_NO_PAT ))
						(*acPtr)->nhosts++;
				}
			}
			acPtr = &(*acPtr)->next;
			nAcls++;
		}
	}

	if (!nListens) {
		if (!(*listenPtr = (ListenEntry *)Malloc( sizeof(ListenEntry))))
			error = 1;
		else {
			(*listenPtr)->iface = -1;
			(*listenPtr)->mcasts = nHosts;
			(*listenPtr)->nmcasts = 0;
#if defined(IPv6) && defined(AF_INET6) && defined(XDM_DEFAULT_MCAST_ADDR6)
			if (ParseHost( &nHosts, &hostPtr, &nChars,
			               XDM_DEFAULT_MCAST_ADDR6,
			               sizeof(XDM_DEFAULT_MCAST_ADDR6)-1,
			               PARSE_ALL ))
				(*listenPtr)->nmcasts++;
#endif
			nListens++;
		}
	}

	if (error) {
		nHosts = nAliases = nAcls = nListens = nChars = 0;
	  sendacl:
		LogError( "No XDMCP requests will be granted\n" );
	}
	GSendInt( nHosts );
	GSendInt( nListens );
	GSendInt( nAliases );
	GSendInt( nAcls );
	GSendInt( nChars );
	for (i = 0; i < nHosts; i++, hostList = hostList->next) {
		GSendInt( hostList->type );
		switch (hostList->type) {
		case HOST_ALIAS:
			GSendStr( hostList->entry.aliasPattern );
			break;
		case HOST_PATTERN:
			GSendStr( hostList->entry.hostPattern );
			break;
		case HOST_ADDRESS:
			GSendArr( hostList->entry.displayAddress.hostAddrLen,
			          hostList->entry.displayAddress.hostAddress );
			GSendInt( hostList->entry.displayAddress.connectionType );
			break;
		}
	}
	for (i = 0; i < nListens; i++, listenList = listenList->next) {
		GSendInt( listenList->iface );
		GSendInt( listenList->mcasts );
		GSendInt( listenList->nmcasts );
	}
	for (i = 0; i < nAliases; i++, aliasList = aliasList->next) {
		GSendStr( aliasList->name );
		GSendInt( aliasList->hosts );
		GSendInt( aliasList->nhosts );
	}
	for (i = 0; i < nAcls; i++, acList = acList->next) {
		GSendInt( acList->entries );
		GSendInt( acList->nentries );
		GSendInt( acList->hosts );
		GSendInt( acList->nhosts );
		GSendInt( acList->flags );
	}
}
#endif


int main( int argc ATTR_UNUSED, char **argv )
{
	DSpec dspec;
	ValArr va;
	char *ci, *disp, *dcls, *cfgfile;
	int what;

	if (!(ci = getenv( "CONINFO" ))) {
		fprintf( stderr, "This program is part of kdm and should not be run manually.\n" );
		return 1;
	}
	if (sscanf( ci, "%d %d", &rfd, &wfd ) != 2)
		return 1;

	InitLog();

	if ((debugLevel = GRecvInt()) & DEBUG_WCONFIG)
		sleep( 100 );

/*	Debug ("parsing command line\n");*/
	if (**++argv)
		kdmrc = *argv;
/*
	while (*++argv) {
	}
*/

	for (;;) {
/*		Debug ("Awaiting command ...\n");*/
		if (!GRecvCmd( &what ))
			break;
		switch (what) {
		case GC_Files:
/*			Debug ("GC_Files\n");*/
			ReadConf();
			CopyValues( 0, &secGeneral, 0, C_CONFIG );
#ifdef XDMCP
			CopyValues( 0, &secXdmcp, 0, C_CONFIG );
			GSendInt( 2 );
#else
			GSendInt( 1 );
#endif
			GSendStr( kdmrc );
				GSendInt( -1 );
#ifdef XDMCP
			GSendNStr( VXaccess.ptr, VXaccess.len - 1 );
				GSendInt( 0 );
#endif
			for (; (what = GRecvInt()) != -1; )
				switch (what) {
				case GC_gGlobal:
				case GC_gDisplay:
					GSendInt( 0 );
					break;
#ifdef XDMCP
				case GC_gXaccess:
					GSendInt( 1 );
					break;
#endif
				default:
					GSendInt( -1 );
					break;
				}
			break;
		case GC_GetConf:
/*		Debug( "GC_GetConf\n" );*/
			memset( &va, 0, sizeof(va) );
			what = GRecvInt();
			cfgfile = GRecvStr();
			switch (what) {
			case GC_gGlobal:
/*		Debug( "GC_gGlobal\n" );*/
				Debug( "getting global config\n" );
				ReadConf();
				CopyValues( &va, &secGeneral, 0, 0 );
#ifdef XDMCP
				CopyValues( &va, &secXdmcp, 0, 0 );
#endif
				CopyValues( &va, &secShutdown, 0, 0 );
				SendValues( &va );
				break;
			case GC_gDisplay:
/*		Debug( "GC_gDisplay\n" );*/
				disp = GRecvStr();
/*		Debug( " Display %s\n", disp );*/
				dcls = GRecvStr();
/*		Debug( " Class %s\n", dcls );*/
				Debug( "getting config for display %s, class %s\n", disp, dcls );
				MkDSpec( &dspec, disp, dcls ? dcls : "" );
				ReadConf();
				CopyValues( &va, &sec_Core, &dspec, 0 );
				CopyValues( &va, &sec_Greeter, &dspec, 0 );
				free( disp );
				if (dcls)
					free( dcls );
				SendValues( &va );
				break;
#ifdef XDMCP
			case GC_gXaccess:
				ReadAccessFile( cfgfile );
				break;
#endif
			default:
				Debug( "Unsupported config cathegory %#x\n", what );
			}
			free( cfgfile );
			break;
		default:
			Debug( "Unknown config command %#x\n", what );
		}
	}

/*	Debug( "Config reader exiting ..." );*/
	return EX_NORMAL;
}
