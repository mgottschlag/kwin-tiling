/* $TOG: genauth.c /main/24 1998/02/09 13:55:23 kaleb $ */
/* $Id$ */
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
/* $XFree86: xc/programs/xdm/genauth.c,v 3.9 2000/05/31 07:15:11 eich Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 */

#include "dm.h"
#include "dm_auth.h"
#include "dm_error.h"

#include <X11/Xauth.h>
#include <X11/Xos.h>

static unsigned epool[4] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };

#if !defined(ARC4_RANDOM) && !defined(DEV_RANDOM)

/* ####################################################################### */

/*
 * This code implements something close to the MD5 message-digest
 * algorithm. This code is based on code written by Colin Plumb
 * in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 */

/* The four core functions - F1 is optimized somewhat */

#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1 (z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define pmd5_step(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  PMD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void 
add_entropy (unsigned const in[16])
{
    unsigned a, b, c, d;

    a = epool[0];
    b = epool[1];
    c = epool[2];
    d = epool[3];

    pmd5_step(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    pmd5_step(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    pmd5_step(F1, c, d, a, b, in[2] + 0x242070db, 17);
    pmd5_step(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    pmd5_step(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    pmd5_step(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    pmd5_step(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    pmd5_step(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    pmd5_step(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    pmd5_step(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    pmd5_step(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    pmd5_step(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    pmd5_step(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    pmd5_step(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    pmd5_step(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    pmd5_step(F1, b, c, d, a, in[15] + 0x49b40821, 22);
		
    pmd5_step(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    pmd5_step(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    pmd5_step(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    pmd5_step(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    pmd5_step(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    pmd5_step(F2, d, a, b, c, in[10] + 0x02441453, 9);
    pmd5_step(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    pmd5_step(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    pmd5_step(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    pmd5_step(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    pmd5_step(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    pmd5_step(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    pmd5_step(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    pmd5_step(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    pmd5_step(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    pmd5_step(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);
		
    pmd5_step(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    pmd5_step(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    pmd5_step(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    pmd5_step(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    pmd5_step(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    pmd5_step(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    pmd5_step(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    pmd5_step(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    pmd5_step(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    pmd5_step(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    pmd5_step(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    pmd5_step(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    pmd5_step(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    pmd5_step(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    pmd5_step(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    pmd5_step(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);
		
    pmd5_step(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    pmd5_step(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    pmd5_step(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    pmd5_step(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    pmd5_step(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    pmd5_step(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    pmd5_step(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    pmd5_step(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    pmd5_step(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    pmd5_step(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    pmd5_step(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    pmd5_step(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    pmd5_step(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    pmd5_step(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    pmd5_step(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    pmd5_step(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    epool[0] += a;
    epool[1] += b;
    epool[2] += c;
    epool[3] += d;
}

/* ####################################################################### */


static int
sumFile (const char *name, int len, int whence, long offset)
{
    int fd, i, cnt;
    unsigned char buf[1024];

    if ((fd = open (name, O_RDONLY)) < 0) {
	Debug("Cannot open entropy source %\"s: %s\n", name, SysErrorMsg());
	return 0;
    }
    lseek (fd, offset, whence);
    while (len > 0) {
	if (!(cnt = read (fd, buf, sizeof (buf))))
	    break;
	if (cnt < 0) {
	    close (fd);
	    Debug("Cannot read entropy source %\"s: %s\n", name, SysErrorMsg());
	    return 0;
	}
	len -= cnt;
	for (i = 0; i < cnt; i += 64)
	    if (sizeof(unsigned) == 4)
		add_entropy((unsigned*)(buf + i));
	    else {
		int j, o;
		unsigned in[16];
		for (j = 0, o = i; j < 16; j++, o += 4)
		    in[j] = buf[o] | (buf[o+1] << 8) | (buf[o+2] << 16) | (buf[o+3] << 24);
		add_entropy(in);
	    }
    }
    close (fd);
    return 1;
}

#ifndef X_GETTIMEOFDAY
/* WABA: According to the man page gettimeofday takes a second argument */
/* if this breaks on your system, we need to have a configure test.     */
# define X_GETTIMEOFDAY(t) gettimeofday(t, NULL)
#endif

static void
UpdateEntropyPool()
{
    int hit = 0;
    static int epoolinited;
    if (!epoolinited) {
	struct timeval now;
	unsigned longs[16];
	X_GETTIMEOFDAY (&now);
	longs[0] = now.tv_usec;
	longs[1] = now.tv_sec;
	longs[2] = getpid();
	longs[3] = getppid();
	/* rest unitialized ... so what? */
	add_entropy(longs);
	sumFile ("/proc/partitions", 0x10000, SEEK_SET, 0);
    }
    /* XXX -- these will work only on linux and similar, but those already have urandom ... */
    if (sumFile ("/proc/stat", 0x10000, SEEK_SET, 0))
	hit++;
    if (sumFile ("/proc/interrupts", 0x10000, SEEK_SET, 0))
	hit++;
    if (sumFile ("/proc/loadavg", 0x10000, SEEK_SET, 0))
	hit++;
    if (sumFile ("/proc/net/dev", 0x10000, SEEK_SET, 0))
	hit++;
    if (!hit) {
	static long offset = ~0, length;
	if (!length) {
	    struct stat st;
	    stat (randomFile, &st);
	    length = st.st_size;
	}
	if (offset > length)
	    offset = 0x4000;
	if (sumFile (randomFile, 0x10000, SEEK_SET, offset)) {
	    hit++;
	    offset += 0x10000;
#ifdef FRAGILE_DEV_MEM
	    if (offset == 0xa0000)	/* skip rom mappings */
		offset = 0x100000;
	    else if (offset == 0xf00000) /* skip 15-16MB memory hole */
		offset = 0x1000000;
#endif
	}
	if (!hit) {
	    if (sumFile ("/var/log/messages", 0x1000, SEEK_END, -0x1000))
		hit++;
	    if (sumFile ("/var/spool/mail/root", 0x1000, SEEK_END, -0x1000))
		hit++;
	    if (!hit) {
		if (!epoolinited)
		    LogError("No entropy sources found; X cookies may be easily guessable\n");
		else {
		    struct timeval now;
		    unsigned longs[16];
		    X_GETTIMEOFDAY (&now);
		    longs[0] = now.tv_usec;
		    longs[1] = now.tv_sec;
		    add_entropy(longs);
		}
	    }
	}
    }
    epoolinited = 1;
}
#endif

/* ONLY 8 or 16 bytes! */
void
GenerateAuthData (char *auth, int len)
{
#ifdef ARC4_RANDOM
    epool[0] = arc4random();
    epool[1] = arc4random();
    if (len == 16) {
	epool[2] = arc4random();
	epool[3] = arc4random();
    }
#elif defined(DEV_RANDOM)
    int fd;
    if ((fd = open(DEV_RANDOM, O_RDONLY)) >= 0) {
	if (read(fd, auth, len) == len) {
	    close(fd);
	    return;
	}
	close(fd);
	LogError("Cannot read " DEV_RANDOM ": %s. X cookies may be easily guessable\n", SysErrorMsg());
    } else
	LogError("Cannot open " DEV_RANDOM ": %s. X cookies may be easily guessable\n", SysErrorMsg());
    {
	/* we don't try the fancy entropy pool stuff here, as if we cannot
           read DEV_RANDOM, the system is really screwed anyway. */
	/* really weak ... */
	struct timeval now;
	static int epoolinited;
	if (!epoolinited) {
	    epoolinited = 1;
	    epool[0] ^= getpid();
	    epool[1] ^= getppid();
	}
	X_GETTIMEOFDAY (&now);
	epool[0] ^= now.tv_usec;
	epool[1] ^= now.tv_sec;
	if (len == 16) {
	    epool[2] += epool[1];
	    epool[3] -= epool[0];
	}
    }
#else
    UpdateEntropyPool();
    if (len == 8) {
	epool[0] ^= epool[2];
	epool[1] ^= epool[3];
    }
#endif
    if (sizeof(unsigned) == 4)
	memcpy(auth, epool, len);
    else {
	int i, o;
	for (i = o = 0; i < len; i += 4, o++) {
	    auth[i] = epool[o] & 0xff;
	    auth[i+1] = (epool[o] >> 8) & 0xff;
	    auth[i+2] = (epool[o] >> 16) & 0xff;
	    auth[i+3] = (epool[o] >> 24) & 0xff;
	}
    }
}
