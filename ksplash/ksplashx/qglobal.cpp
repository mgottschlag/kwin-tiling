#include "qglobal.h"

static bool si_alreadyDone = FALSE;
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
	return TRUE;
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
	return FALSE;
    }
    if ( sizeof(Q_INT8) != 1 || sizeof(Q_INT16) != 2 || sizeof(Q_INT32) != 4 ||
	 sizeof(Q_ULONG)*8 != si_wordSize || sizeof(float) != 4 || sizeof(double) != 8 ) {
#if defined(QT_CHECK_RANGE)
	qFatal( "qSysInfo: Unsupported system data type size" );
#endif
	return FALSE;
    }

    bool  be16, be32;				// determine byte ordering
    short ns = 0x1234;
    int	  nl = 0x12345678;

    unsigned char *p = (unsigned char *)(&ns);	// 16-bit integer
    be16 = *p == 0x12;

    p = (unsigned char *)(&nl);			// 32-bit integer
    if ( p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78 )
	be32 = TRUE;
    else
    if ( p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12 )
	be32 = FALSE;
    else
	be32 = !be16;

    if ( be16 != be32 ) {			// strange machine!
#if defined(QT_CHECK_RANGE)
	qFatal( "qSysInfo: Inconsistent system byte order" );
#endif
	return FALSE;
    }

    *bigEndian = si_bigEndian = be32;
    si_alreadyDone = TRUE;
    return TRUE;
}
