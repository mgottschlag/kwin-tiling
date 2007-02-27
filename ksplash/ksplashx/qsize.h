/****************************************************************************
** 
**
** Definition of QSize class
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

#ifndef QSIZE_H
#define QSIZE_H

#ifndef QT_H
#include "qpoint.h" // ### change to qwindowdefs.h?
#endif // QT_H

class Q_EXPORT QSize
// ### Make QSize inherit Qt in Qt 4.0
{
public:
    // ### Move this enum to qnamespace.h in Qt 4.0
    enum ScaleMode {
	ScaleFree,
	ScaleMin,
	ScaleMax
    };

    QSize();
    QSize( int w, int h );

    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;

    int width() const;
    int height() const;
    void setWidth( int w );
    void setHeight( int h );
    void transpose();

    void scale( int w, int h, ScaleMode mode );
    void scale( const QSize &s, ScaleMode mode );

    QSize expandedTo( const QSize & ) const;
    QSize boundedTo( const QSize & ) const;

    QCOORD &rwidth();
    QCOORD &rheight();

    QSize &operator+=( const QSize & );
    QSize &operator-=( const QSize & );
    QSize &operator*=( int c );
    QSize &operator*=( double c );
    QSize &operator/=( int c );
    QSize &operator/=( double c );

    friend inline bool operator==( const QSize &, const QSize & );
    friend inline bool operator!=( const QSize &, const QSize & );
    friend inline const QSize operator+( const QSize &, const QSize & );
    friend inline const QSize operator-( const QSize &, const QSize & );
    friend inline const QSize operator*( const QSize &, int );
    friend inline const QSize operator*( int, const QSize & );
    friend inline const QSize operator*( const QSize &, double );
    friend inline const QSize operator*( double, const QSize & );
    friend inline const QSize operator/( const QSize &, int );
    friend inline const QSize operator/( const QSize &, double );

private:
    static void warningDivByZero();

    QCOORD wd;
    QCOORD ht;
};


/*****************************************************************************
  QSize stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QSize & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QSize & );
#endif

/*****************************************************************************
  QSize inline functions
 *****************************************************************************/

inline QSize::QSize()
{ wd = ht = -1; }

inline QSize::QSize( int w, int h )
{ wd=(QCOORD)w; ht=(QCOORD)h; }

inline bool QSize::isNull() const
{ return wd==0 && ht==0; }

inline bool QSize::isEmpty() const
{ return wd<1 || ht<1; }

inline bool QSize::isValid() const
{ return wd>=0 && ht>=0; }

inline int QSize::width() const
{ return wd; }

inline int QSize::height() const
{ return ht; }

inline void QSize::setWidth( int w )
{ wd=(QCOORD)w; }

inline void QSize::setHeight( int h )
{ ht=(QCOORD)h; }

inline QCOORD &QSize::rwidth()
{ return wd; }

inline QCOORD &QSize::rheight()
{ return ht; }

inline QSize &QSize::operator+=( const QSize &s )
{ wd+=s.wd; ht+=s.ht; return *this; }

inline QSize &QSize::operator-=( const QSize &s )
{ wd-=s.wd; ht-=s.ht; return *this; }

inline QSize &QSize::operator*=( int c )
{ wd*=(QCOORD)c; ht*=(QCOORD)c; return *this; }

inline QSize &QSize::operator*=( double c )
{ wd=(QCOORD)(wd*c); ht=(QCOORD)(ht*c); return *this; }

inline bool operator==( const QSize &s1, const QSize &s2 )
{ return s1.wd == s2.wd && s1.ht == s2.ht; }

inline bool operator!=( const QSize &s1, const QSize &s2 )
{ return s1.wd != s2.wd || s1.ht != s2.ht; }

inline const QSize operator+( const QSize & s1, const QSize & s2 )
{ return QSize(s1.wd+s2.wd, s1.ht+s2.ht); }

inline const QSize operator-( const QSize &s1, const QSize &s2 )
{ return QSize(s1.wd-s2.wd, s1.ht-s2.ht); }

inline const QSize operator*( const QSize &s, int c )
{ return QSize(s.wd*c, s.ht*c); }

inline const QSize operator*( int c, const QSize &s )
{  return QSize(s.wd*c, s.ht*c); }

inline const QSize operator*( const QSize &s, double c )
{ return QSize((QCOORD)(s.wd*c), (QCOORD)(s.ht*c)); }

inline const QSize operator*( double c, const QSize &s )
{ return QSize((QCOORD)(s.wd*c), (QCOORD)(s.ht*c)); }

inline QSize &QSize::operator/=( int c )
{
#if defined(QT_CHECK_MATH)
    if ( c == 0 )
	warningDivByZero();
#endif
    wd/=(QCOORD)c; ht/=(QCOORD)c;
    return *this;
}

inline QSize &QSize::operator/=( double c )
{
#if defined(QT_CHECK_MATH)
    if ( c == 0.0 )
	warningDivByZero();
#endif
    wd=(QCOORD)(wd/c); ht=(QCOORD)(ht/c);
    return *this;
}

inline const QSize operator/( const QSize &s, int c )
{
#if defined(QT_CHECK_MATH)
    if ( c == 0 )
	QSize::warningDivByZero();
#endif
    return QSize(s.wd/c, s.ht/c);
}

inline const QSize operator/( const QSize &s, double c )
{
#if defined(QT_CHECK_MATH)
    if ( c == 0.0 )
	QSize::warningDivByZero();
#endif
    return QSize((QCOORD)(s.wd/c), (QCOORD)(s.ht/c));
}

inline QSize QSize::expandedTo( const QSize & otherSize ) const
{
    return QSize( QMAX(wd,otherSize.wd), QMAX(ht,otherSize.ht) );
}

inline QSize QSize::boundedTo( const QSize & otherSize ) const
{
    return QSize( QMIN(wd,otherSize.wd), QMIN(ht,otherSize.ht) );
}


#endif // QSIZE_H
