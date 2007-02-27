/****************************************************************************
** 
**
** Implementation of QSize class
**
** Created : 931028
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsize.h"
#include "qdatastream.h"


/*!
  \class QSize
  \brief The QSize class defines the size of a two-dimensional object.

  \ingroup images
  \ingroup graphics

  A size is specified by a width and a height.

  The coordinate type is QCOORD (defined in \c <qwindowdefs.h> as \c int).
  The minimum value of QCOORD is QCOORD_MIN (-2147483648) and the maximum
  value is QCOORD_MAX (2147483647).

  The size can be set in the constructor and changed with setWidth()
  and setHeight(), or using operator+=(), operator-=(), operator*=()
  and operator/=(), etc. You can swap the width and height with
  transpose(). You can get a size which holds the maximum height and
  width of two sizes using expandedTo(), and the minimum height and
  width of two sizes using boundedTo().


  \sa QPoint, QRect
*/


/*****************************************************************************
  QSize member functions
 *****************************************************************************/

/*!
  \fn QSize::QSize()
  Constructs a size with invalid (negative) width and height.
*/

/*!
  \fn QSize::QSize( int w, int h )
  Constructs a size with width \a w and height \a h.
*/

/*!
  \fn bool QSize::isNull() const
  Returns TRUE if the width is 0 and the height is 0; otherwise
  returns FALSE.
*/

/*!
  \fn bool QSize::isEmpty() const
  Returns TRUE if the width is less than or equal to 0, or the height is
  less than or equal to 0; otherwise returns FALSE.
*/

/*!
  \fn bool QSize::isValid() const
  Returns TRUE if the width is equal to or greater than 0 and the height is
  equal to or greater than 0; otherwise returns FALSE.
*/

/*!
  \fn int QSize::width() const
  Returns the width.
  \sa height()
*/

/*!
  \fn int QSize::height() const
  Returns the height.
  \sa width()
*/

/*!
  \fn void QSize::setWidth( int w )
  Sets the width to \a w.
  \sa width(), setHeight()
*/

/*!
  \fn void QSize::setHeight( int h )
  Sets the height to \a h.
  \sa height(), setWidth()
*/

/*!
  Swaps the values of width and height.
*/

void QSize::transpose()
{
    QCOORD tmp = wd;
    wd = ht;
    ht = tmp;
}

/*! \enum QSize::ScaleMode

    This enum type defines the different ways of scaling a size.

    \img scaling.png

    \value ScaleFree  The size is scaled freely. The ratio is not preserved.
    \value ScaleMin  The size is scaled to a rectangle as large as possible
                     inside a given rectangle, preserving the aspect ratio.
    \value ScaleMax  The size is scaled to a rectangle as small as possible
                     outside a given rectangle, preserving the aspect ratio.

    \sa QSize::scale(), QImage::scale(), QImage::smoothScale()
*/

/*!
    Scales the size to a rectangle of width \a w and height \a h according
    to the ScaleMode \a mode.

    \list
    \i If \a mode is \c ScaleFree, the size is set to (\a w, \a h).
    \i If \a mode is \c ScaleMin, the current size is scaled to a rectangle
       as large as possible inside (\a w, \a h), preserving the aspect ratio.
    \i If \a mode is \c ScaleMax, the current size is scaled to a rectangle
       as small as possible outside (\a w, \a h), preserving the aspect ratio.
    \endlist

    Example:
    \code
    QSize t1( 10, 12 );
    t1.scale( 60, 60, QSize::ScaleFree );
    // t1 is (60, 60)

    QSize t2( 10, 12 );
    t2.scale( 60, 60, QSize::ScaleMin );
    // t2 is (50, 60)

    QSize t3( 10, 12 );
    t3.scale( 60, 60, QSize::ScaleMax );
    // t3 is (60, 72)
    \endcode
*/
void QSize::scale( int w, int h, ScaleMode mode )
{
    if ( mode == ScaleFree ) {
	wd = (QCOORD)w;
	ht = (QCOORD)h;
    } else {
	bool useHeight = TRUE;
	int w0 = width();
	int h0 = height();
	int rw = h * w0 / h0;

	if ( mode == ScaleMin ) {
	    useHeight = ( rw <= w );
	} else { // mode == ScaleMax
	    useHeight = ( rw >= w );
	}

	if ( useHeight ) {
	    wd = (QCOORD)rw;
	    ht = (QCOORD)h;
	} else {
	    wd = (QCOORD)w;
	    ht = (QCOORD)( w * h0 / w0 );
	}
    }
}

/*!
    \overload

    Equivalent to scale(\a{s}.width(), \a{s}.height(), \a mode).
*/
void QSize::scale( const QSize &s, ScaleMode mode )
{
    scale( s.width(), s.height(), mode );
}

/*!
  \fn QCOORD &QSize::rwidth()
  Returns a reference to the width.

  Using a reference makes it possible to directly manipulate the width.

  Example:
  \code
    QSize s( 100, 10 );
    s.rwidth() += 20;		// s becomes (120,10)
  \endcode

  \sa rheight()
*/

/*!
  \fn QCOORD &QSize::rheight()
  Returns a reference to the height.

  Using a reference makes it possible to directly manipulate the height.

  Example:
  \code
    QSize s( 100, 10 );
    s.rheight() += 5;		// s becomes (100,15)
  \endcode

  \sa rwidth()
*/

/*!
  \fn QSize &QSize::operator+=( const QSize &s )

  Adds \a s to the size and returns a reference to this size.

  Example:
  \code
    QSize s(  3, 7 );
    QSize r( -1, 4 );
    s += r;			// s becomes (2,11)
\endcode
*/

/*!
  \fn QSize &QSize::operator-=( const QSize &s )

  Subtracts \a s from the size and returns a reference to this size.

  Example:
  \code
    QSize s(  3, 7 );
    QSize r( -1, 4 );
    s -= r;			// s becomes (4,3)
  \endcode
*/

/*!
  \fn QSize &QSize::operator*=( int c )
  Multiplies both the width and height by \a c and returns a reference to
  the size.
*/

/*!
  \overload QSize &QSize::operator*=( double c )

  Multiplies both the width and height by \a c and returns a reference to
  the size.

  Note that the result is truncated.
*/

/*!
  \fn bool operator==( const QSize &s1, const QSize &s2 )
  \relates QSize
  Returns TRUE if \a s1 and \a s2 are equal; otherwise returns FALSE.
*/

/*!
  \fn bool operator!=( const QSize &s1, const QSize &s2 )
  \relates QSize
  Returns TRUE if \a s1 and \a s2 are different; otherwise returns FALSE.
*/

/*!
  \fn const QSize operator+( const QSize &s1, const QSize &s2 )
  \relates QSize
  Returns the sum of \a s1 and \a s2; each component is added separately.
*/

/*!
  \fn const QSize operator-( const QSize &s1, const QSize &s2 )
  \relates QSize
  Returns \a s2 subtracted from \a s1; each component is
  subtracted separately.
*/

/*!
  \fn const QSize operator*( const QSize &s, int c )
  \relates QSize
  Multiplies \a s by \a c and returns the result.
*/

/*!
  \overload const QSize operator*( int c, const QSize &s )
  \relates QSize
  Multiplies \a s by \a c and returns the result.
*/

/*!
  \overload const QSize operator*( const QSize &s, double c )
  \relates QSize
  Multiplies \a s by \a c and returns the result.
*/

/*!
  \overload const QSize operator*( double c, const QSize &s )
  \relates QSize
  Multiplies \a s by \a c and returns the result.
*/

/*!
  \fn QSize &QSize::operator/=( int c )
  Divides both the width and height by \a c and returns a reference to the
  size.
*/

/*!
  \fn QSize &QSize::operator/=( double c )
  \overload
  Divides both the width and height by \a c and returns a reference to the
  size.

  Note that the result is truncated.
*/

/*!
  \fn const QSize operator/( const QSize &s, int c )
  \relates QSize
  Divides \a s by \a c and returns the result.
*/

/*!
  \fn const QSize operator/( const QSize &s, double c )
  \relates QSize
  \overload
  Divides \a s by \a c and returns the result.

  Note that the result is truncated.
*/

/*!
  \fn QSize QSize::expandedTo( const QSize & otherSize ) const

  Returns a size with the maximum width and height of this size and
  \a otherSize.
*/

/*!
  \fn QSize QSize::boundedTo( const QSize & otherSize ) const

  Returns a size with the minimum width and height of this size and
  \a otherSize.
*/


void QSize::warningDivByZero()
{
#if defined(QT_CHECK_MATH)
    qWarning( "QSize: Division by zero error" );
#endif
}


/*****************************************************************************
  QSize stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
  \relates QSize
  Writes the size \a sz to the stream \a s and returns a reference to
  the stream.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QSize &sz )
{
    if ( s.version() == 1 )
	s << (Q_INT16)sz.width() << (Q_INT16)sz.height();
    else
	s << (Q_INT32)sz.width() << (Q_INT32)sz.height();
    return s;
}

/*!
  \relates QSize
  Reads the size from the stream \a s into size \a sz and returns a
  reference to the stream.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QSize &sz )
{
    if ( s.version() == 1 ) {
	Q_INT16 w, h;
	s >> w;  sz.rwidth() = w;
	s >> h;  sz.rheight() = h;
    }
    else {
	Q_INT32 w, h;
	s >> w;  sz.rwidth() = w;
	s >> h;  sz.rheight() = h;
    }
    return s;
}
#endif // QT_NO_DATASTREAM
