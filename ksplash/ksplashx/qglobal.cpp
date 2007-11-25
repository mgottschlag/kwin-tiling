/*
 *   Copyright (C) 2007 Luboš Luňák <l.lunak@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "qglobal.h"

static bool si_alreadyDone = false;
static int  si_wordSize;
static bool si_bigEndian;

/*!
    \relates QApplication

    Obtains information about the system.

    The system's word size in bits (typically 32) is returned in \a
    *wordSize. The \a *bigEndian is set to TRUE if this is a big-endian
    machine, or to FALSE if this is a little-endian machine.

    In debug mode, this function calls qFatal() with a message if the
    computer is truly weird (i.e. different endianness for 16 bit and
    32 bit integers); in release mode it returns FALSE.
*/

bool qSysInfo( int *wordSize, bool *bigEndian )
{
#if defined(QT_CHECK_NULL)
    Q_ASSERT( wordSize != 0 );
    Q_ASSERT( bigEndian != 0 );
#endif

    if ( si_alreadyDone ) {			// run it only once
	*wordSize  = si_wordSize;
	*bigEndian = si_bigEndian;
	return true;
    }

    si_wordSize = 0;
    Q_ULONG n = (Q_ULONG)(~0);
    while ( n ) {				// detect word size
	si_wordSize++;
	n /= 2;
    }
    *wordSize = si_wordSize;

    if ( *wordSize != 64 &&
	 *wordSize != 32 &&
	 *wordSize != 16 ) {			// word size: 16, 32 or 64
#if defined(QT_CHECK_RANGE)
	qFatal( "qSysInfo: Unsupported system word size %d", *wordSize );
#endif
	return false;
    }
    if ( sizeof(Q_INT8) != 1 || sizeof(Q_INT16) != 2 || sizeof(Q_INT32) != 4 ||
	 sizeof(Q_ULONG)*8 != si_wordSize || sizeof(float) != 4 || sizeof(double) != 8 ) {
#if defined(QT_CHECK_RANGE)
	qFatal( "qSysInfo: Unsupported system data type size" );
#endif
	return false;
    }

    bool  be16, be32;				// determine byte ordering
    short ns = 0x1234;
    int	  nl = 0x12345678;

    unsigned char *p = (unsigned char *)(&ns);	// 16-bit integer
    be16 = *p == 0x12;

    p = (unsigned char *)(&nl);			// 32-bit integer
    if ( p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78 )
	be32 = true;
    else
    if ( p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12 )
	be32 = false;
    else
	be32 = !be16;

    if ( be16 != be32 ) {			// strange machine!
#if defined(QT_CHECK_RANGE)
	qFatal( "qSysInfo: Inconsistent system byte order" );
#endif
	return false;
    }

    *bigEndian = si_bigEndian = be32;
    si_alreadyDone = true;
    return true;
}
