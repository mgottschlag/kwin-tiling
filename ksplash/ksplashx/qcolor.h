/****************************************************************************
** 
**
** Definition of QColor class
**
** Created : 940112
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

#ifndef QCOLOR_H
#define QCOLOR_H

#ifndef QT_H
#include "qwindowdefs.h"
//##include "qstringlist.h"
#endif // QT_H

const QRgb  RGB_MASK    = 0x00ffffff;		// masks RGB values

Q_EXPORT inline int qRed( QRgb rgb )		// get red part of RGB
{ return (int)((rgb >> 16) & 0xff); }

Q_EXPORT inline int qGreen( QRgb rgb )		// get green part of RGB
{ return (int)((rgb >> 8) & 0xff); }

Q_EXPORT inline int qBlue( QRgb rgb )		// get blue part of RGB
{ return (int)(rgb & 0xff); }

Q_EXPORT inline int qAlpha( QRgb rgb )		// get alpha part of RGBA
{ return (int)((rgb >> 24) & 0xff); }

Q_EXPORT inline QRgb qRgb( int r, int g, int b )// set RGB value
{ return (0xff << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

Q_EXPORT inline QRgb qRgba( int r, int g, int b, int a )// set RGBA value
{ return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

Q_EXPORT inline int qGray( int r, int g, int b )// convert R,G,B to gray 0..255
{ return (r*11+g*16+b*5)/32; }

Q_EXPORT inline int qGray( QRgb rgb )		// convert RGB to gray 0..255
{ return qGray( qRed(rgb), qGreen(rgb), qBlue(rgb) ); }


class Q_EXPORT QColor
{
public:
    enum Spec { Rgb, Hsv };

    QColor();
    QColor( int r, int g, int b );
    QColor( int x, int y, int z, Spec );
    QColor( QRgb rgb, uint pixel=0xffffffff);
#if 0
    QColor( const QString& name );
#endif
    QColor( const char *name );
    QColor( const QColor & );
    QColor &operator=( const QColor & );

    bool   isValid() const;
    bool   isDirty() const;
#if 0
    QString name() const;
    void   setNamedColor( const QString& name );
#else
    void   setNamedColor( const char* name );
#endif

    QRgb   rgb()    const;
    void   setRgb( int r, int g, int b );
    void   setRgb( QRgb rgb );
    void   getRgb( int *r, int *g, int *b ) const { rgb( r, g, b ); }
    void   rgb( int *r, int *g, int *b ) const; // obsolete

    int	   red()    const;
    int	   green()  const;
    int	   blue()   const;

    void   setHsv( int h, int s, int v );
    void   getHsv( int *h, int *s, int *v ) const { hsv( h, s, v ); }
    void   hsv( int *h, int *s, int *v ) const; // obsolete
    void   getHsv( int &h, int &s, int &v ) const { hsv( &h, &s, &v ); } // obsolete

    QColor light( int f = 150 ) const;
    QColor dark( int f = 200 )	const;

    bool   operator==( const QColor &c ) const;
    bool   operator!=( const QColor &c ) const;

    uint   alloc();
    uint   pixel()  const;

#if defined(Q_WS_X11)
    // ### in 4.0, make this take a default argument of -1 for default screen?
    uint alloc( int screen );
    uint pixel( int screen ) const;
#endif

    static int  maxColors();
    static int  numBitPlanes();

    static int  enterAllocContext();
    static void leaveAllocContext();
    static int  currentAllocContext();
    static void destroyAllocContext( int );

#if defined(Q_WS_WIN)
    static const QRgb* palette( int* numEntries = 0 );
    static int setPaletteEntries( const QRgb* entries, int numEntries,
				  int base = -1 );
    static HPALETTE hPal()  { return hpal; }
    static uint	realizePal( QWidget * );
#endif

    static void initialize();
    static void cleanup();
#ifndef QT_NO_STRINGLIST
    static QStringList colorNames();
#endif
    enum { Dirt = 0x44495254, Invalid = 0x49000000 };

private:
#if 0
    void setSystemNamedColor( const QString& name );
#else
    void setSystemNamedColor( const char* name );
#endif
    void setPixel( uint pixel );
    static void initGlobalColors();
    static uint argbToPix32(QRgb);
    static QColor* globalColors();
    static bool color_init;
    static bool globals_init;
#if defined(Q_WS_WIN)
    static HPALETTE hpal;
#endif
    static enum ColorModel { d8, d32 } colormodel;
    union {
	QRgb argb;
	struct D8 {
	    QRgb argb;
	    uchar pix;
	    uchar invalid;
	    uchar dirty;
	    uchar direct;
	} d8;
	struct D32 {
	    QRgb argb;
	    uint pix;
	    bool invalid() const { return argb == QColor::Invalid && pix == QColor::Dirt; }
	    bool probablyDirty() const { return pix == QColor::Dirt; }
	} d32;
    } d;
};


inline QColor::QColor()
{ d.d32.argb = Invalid; d.d32.pix = Dirt; }

inline QColor::QColor( int r, int g, int b )
{
    d.d32.argb = Invalid;
    d.d32.pix = Dirt;
    setRgb( r, g, b );
}

inline QRgb QColor::rgb() const
{ return d.argb; }

inline int QColor::red() const
{ return qRed(d.argb); }

inline int QColor::green() const
{ return qGreen(d.argb); }

inline int QColor::blue() const
{ return qBlue(d.argb); }

inline bool QColor::isValid() const
{
    if ( colormodel == d8 )
	return !d.d8.invalid;
    else
	return !d.d32.invalid();
}

inline bool QColor::operator==( const QColor &c ) const
{
    return d.argb == c.d.argb && isValid() == c.isValid();
}

inline bool QColor::operator!=( const QColor &c ) const
{
    return !operator==(c);
}


/*****************************************************************************
  QColor stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QColor & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QColor & );
#endif

#endif // QCOLOR_H
