/****************************************************************************
** 
**
** Implementation of QColor class for X11
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
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
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

#include "qcolor.h"
#include "qcolor_p.h"
//#include "string.h"
//#include "qpaintdevice.h"
//#include "qapplication.h"
//#include "qapplication_p.h"
//#include "qt_x11_p.h"
#include "x11_defs.h"
#include "string.h"

int		qt_ncols_option  = 216;		// used in qcolor_x11.cpp

// NOT REVISED

/*****************************************************************************
  The color dictionary speeds up color allocation significantly for X11.
  When there are no more colors, QColor::alloc() will set the colors_avail
  flag to FALSE and try to find the nearest color.
  NOTE: From deep within the event loop, the colors_avail flag is reset to
  TRUE (calls the function qt_reset_color_avail()), because some other
  application might free its colors, thereby making them available for
  this Qt application.
 *****************************************************************************/

//#include "qintdict.h"

struct QColorData {
    uint pix;					// allocated pixel value
    int	 context;				// allocation context
};

#if 0
typedef QIntDict<QColorData> QColorDict;
typedef QIntDictIterator<QColorData> QColorDictIt;
#endif
static int	current_alloc_context = 0;	// current color alloc context
static const uint col_std_dict = 419;
static const uint col_large_dict = 18397;

class QColorScreenData {
public:
    QColorScreenData()
    {
//#	colorDict = 0;
	colors_avail = TRUE;
	g_vis = 0;
	g_carr = 0;
	g_carr_fetch = TRUE;
	g_cells = 0;
	g_our_alloc = 0;
	color_reduce = FALSE;
    }

//#    QColorDict *colorDict;		// dict of allocated colors
    bool colors_avail;			// X colors available
    bool g_truecolor;			// truecolor visual
    Visual *g_vis;			// visual
    XColor *g_carr;			// color array
    bool g_carr_fetch;			// perform XQueryColors?
    int	g_cells;			// number of entries in g_carr
    bool *g_our_alloc;			// our allocated colors
    uint red_mask , green_mask , blue_mask;
    int	red_shift, green_shift, blue_shift;
    bool color_reduce;
    int	col_div_r;
    int	col_div_g;
    int	col_div_b;
};

static int screencount = 0;
static QColorScreenData **screendata = 0; // array of screendata pointers


/*
  This function is called from the event loop. It resets the colors_avail
  flag so that the application can retry to allocate read-only colors
  that other applications may have deallocated lately.

  The g_our_alloc and g_carr are global arrays that optimize color
  approximation when there are no more colors left to allocate.
*/

void qt_reset_color_avail()
{
    int i;
    for ( i = 0; i < screencount; i++ ) {
	screendata[i]->colors_avail = TRUE;
	screendata[i]->g_carr_fetch = TRUE;	// do XQueryColors if !colors_avail
    }
}


/*
  Finds the nearest color.
*/

static int find_nearest_color( int r, int g, int b, int* mindist_out,
			       QColorScreenData *sd )
{
    int mincol = -1;
    int mindist = 200000;
    int rx, gx, bx, dist;
    XColor *xc = &sd->g_carr[0];
    for ( int i=0; i<sd->g_cells; i++ ) {
	rx = r - (xc->red >> 8);
	gx = g - (xc->green >> 8);
	bx = b - (xc->blue>> 8);
	dist = rx*rx + gx*gx + bx*bx;		// calculate distance
	if ( dist < mindist ) {			// minimal?
	    mindist = dist;
	    mincol = i;
	}
	xc++;
    }
    *mindist_out = mindist;
    return mincol;
}


/*****************************************************************************
  QColor misc internal functions
 *****************************************************************************/

static int highest_bit( uint v )
{
    int i;
    uint b = (uint)1 << 31;			// get pos of highest bit in v
    for ( i=31; ((b & v) == 0) && i>=0;	 i-- )
	b >>= 1;
    return i;
}


/*****************************************************************************
  QColor static member functions
 *****************************************************************************/

/*!
    Returns the maximum number of colors supported by the underlying
    window system if the window system uses a palette.

    Otherwise returns -1. Use numBitPlanes() to calculate the available
    colors in that case.
*/

int QColor::maxColors()
{
    Visual *visual = (Visual *) QPaintDevice::x11AppVisual();
    if (visual->c_class & 1)
	return QPaintDevice::x11AppCells();
    return -1;
}

/*!
    Returns the number of color bit planes for the underlying window
    system.

    The returned value is equal to the default pixmap depth.

    \sa QPixmap::defaultDepth()
*/

int QColor::numBitPlanes()
{
    return QPaintDevice::x11AppDepth();
}


/*!
    Internal initialization required for QColor.
    This function is called from the QApplication constructor.

    \sa cleanup()
*/

void QColor::initialize()
{
    static const int blackIdx = 2;
    static const int whiteIdx = 3;

    if ( color_init )				// already initialized
	return;
    color_init = TRUE;

    Display *dpy  = QPaintDevice::x11AppDisplay();
    int	     spec = QApplication::colorSpec();

    screencount = ScreenCount( dpy );
    screendata = new QColorScreenData*[ screencount ];

    int scr = QPaintDevice::x11AppScreen();

    for ( scr = 0; scr < screencount; ++scr ) {
	screendata[scr] = new QColorScreenData;
        screendata[scr]->g_vis = (Visual *) QPaintDevice::x11AppVisual( scr );
	screendata[scr]->g_truecolor = screendata[scr]->g_vis->c_class == TrueColor;

	int	     ncols= QPaintDevice::x11AppCells( scr );

	if ( screendata[scr]->g_truecolor ) {
	    colormodel = d32;
	} else {
	    colormodel = d8;
	    // Create the g_our_alloc array, which remembers which color pixels
	    // we allocated.
	    screendata[scr]->g_cells = QMIN(ncols,256);
	    screendata[scr]->g_carr  = new XColor[screendata[scr]->g_cells];
	    Q_CHECK_PTR( screendata[scr]->g_carr );
	    memset( screendata[scr]->g_carr, 0,
		    screendata[scr]->g_cells*sizeof(XColor) );
	    screendata[scr]->g_carr_fetch = TRUE;	// run XQueryColors on demand
	    screendata[scr]->g_our_alloc = new bool[screendata[scr]->g_cells];
	    Q_CHECK_PTR( screendata[scr]->g_our_alloc );
	    memset( screendata[scr]->g_our_alloc, FALSE,
		    screendata[scr]->g_cells*sizeof(bool) );
	    XColor *xc = &screendata[scr]->g_carr[0];
	    for ( int i=0; i<screendata[scr]->g_cells; i++ ) {
		xc->pixel = i;		// g_carr[i] = color i
		xc++;
	    }
	}

	int dictsize;
	if ( screendata[scr]->g_truecolor ) {			// truecolor
	    dictsize    = 1;			// will not need color dict
	    screendata[scr]->red_mask    = (uint)screendata[scr]->g_vis->red_mask;
	    screendata[scr]->green_mask  = (uint)screendata[scr]->g_vis->green_mask;
	    screendata[scr]->blue_mask   = (uint)screendata[scr]->g_vis->blue_mask;
	    screendata[scr]->red_shift =
		highest_bit( screendata[scr]->red_mask ) - 7;
	    screendata[scr]->green_shift =
		highest_bit( screendata[scr]->green_mask ) - 7;
	    screendata[scr]->blue_shift =
		highest_bit( screendata[scr]->blue_mask ) - 7;
	} else {
	    dictsize = col_std_dict;
	}
#if 0
	screendata[scr]->colorDict = new QColorDict(dictsize);	// create dictionary
	Q_CHECK_PTR( screendata[scr]->colorDict );
#endif

	if ( spec == (int)QApplication::ManyColor ) {
	    screendata[scr]->color_reduce = TRUE;

	    switch ( qt_ncols_option ) {
	    case 216:
		// 6:6:6
		screendata[scr]->col_div_r = screendata[scr]->col_div_g =
		screendata[scr]->col_div_b = (255/(6-1));
		break;
	    default: {
		// 2:3:1 proportions, solved numerically
		if ( qt_ncols_option > 255 ) qt_ncols_option = 255;
		if ( qt_ncols_option < 1 ) qt_ncols_option = 1;
		int nr = 2;
		int ng = 2;
		int nb = 2;
		for (;;) {
		    if ( nb*2 < nr && (nb+1)*nr*ng < qt_ncols_option )
			nb++;
		    else if ( nr*3 < ng*2 && nb*(nr+1)*ng < qt_ncols_option )
			nr++;
		    else if ( nb*nr*(ng+1) < qt_ncols_option )
			ng++;
		    else break;
		}
		qt_ncols_option = nr*ng*nb;
		screendata[scr]->col_div_r = (255/(nr-1));
		screendata[scr]->col_div_g = (255/(ng-1));
		screendata[scr]->col_div_b = (255/(nb-1));
	    }
	    }
	}
    }

    scr = QPaintDevice::x11AppScreen();

    // Initialize global color objects
    if ( QPaintDevice::x11AppDefaultVisual(scr) &&
	 QPaintDevice::x11AppDefaultColormap(scr) ) {
	globalColors()[blackIdx].setPixel((uint) BlackPixel(dpy, scr));
	globalColors()[whiteIdx].setPixel((uint) WhitePixel(dpy, scr));
    } else {
	globalColors()[blackIdx].alloc(scr);
	globalColors()[whiteIdx].alloc(scr);
    }

#if 0 /* 0 == allocate colors on demand */
    setLazyAlloc( FALSE );			// allocate global colors
    ((QColor*)(&darkGray))->	alloc();
    ((QColor*)(&gray))->	alloc();
    ((QColor*)(&lightGray))->	alloc();
    ((QColor*)(&::red))->	alloc();
    ((QColor*)(&::green))->	alloc();
    ((QColor*)(&::blue))->	alloc();
    ((QColor*)(&cyan))->	alloc();
    ((QColor*)(&magenta))->	alloc();
    ((QColor*)(&yellow))->	alloc();
    ((QColor*)(&darkRed))->	alloc();
    ((QColor*)(&darkGreen))->	alloc();
    ((QColor*)(&darkBlue))->	alloc();
    ((QColor*)(&darkCyan))->	alloc();
    ((QColor*)(&darkMagenta))-> alloc();
    ((QColor*)(&darkYellow))->	alloc();
    setLazyAlloc( TRUE );
#endif
}

/*!
    Internal clean up required for QColor.
    This function is called from the QApplication destructor.

    \sa initialize()
*/

void QColor::cleanup()
{
    if ( !color_init )
	return;
    color_init = FALSE;
    int scr;
    for ( scr = 0; scr < screencount; scr++ ) {
	if ( screendata[scr]->g_carr ) {
	    delete [] screendata[scr]->g_carr;
	    screendata[scr]->g_carr = 0;
	}
	if ( screendata[scr]->g_our_alloc ) {
	    delete [] screendata[scr]->g_our_alloc;
	    screendata[scr]->g_our_alloc = 0;
	}
#if 0
	if ( screendata[scr]->colorDict ) {
	    screendata[scr]->colorDict->setAutoDelete( TRUE );
	    screendata[scr]->colorDict->clear();
	    delete screendata[scr]->colorDict;
	    screendata[scr]->colorDict = 0;
	}
#endif
	delete screendata[scr];
	screendata[scr] = 0;
    }
    delete [] screendata;
    screendata = 0;
    screencount = 0;
}


/*****************************************************************************
  QColor member functions
 *****************************************************************************/

/*!
  \internal
  Allocates the color on screen \a screen.  Only used in X11.

  \sa alloc(), pixel()
*/
uint QColor::alloc( int screen )
{
    Display *dpy = QPaintDevice::x11AppDisplay();
    if ( screen < 0 )
	screen = QPaintDevice::x11AppScreen();
    if ( !color_init )
	return dpy ? (uint)BlackPixel(dpy, screen) : 0;
    int r = qRed(d.argb);
    int g = qGreen(d.argb);
    int b = qBlue(d.argb);
    uint pix = 0;
    QColorScreenData *sd = screendata[screen];
    if ( sd->g_truecolor ) {			// truecolor: map to pixel
	r = sd->red_shift   > 0 ? r << sd->red_shift   : r >> -sd->red_shift;
	g = sd->green_shift > 0 ? g << sd->green_shift : g >> -sd->green_shift;
	b = sd->blue_shift  > 0 ? b << sd->blue_shift  : b >> -sd->blue_shift;
	pix = (b & sd->blue_mask) | (g & sd->green_mask) | (r & sd->red_mask)
	      | ~(sd->blue_mask | sd->green_mask | sd->red_mask);
	if ( screen == QPaintDevice::x11AppScreen() )
	    d.d32.pix = pix;
	return pix;
    }
#if 0
    QColorData *c = sd->colorDict->find( (long)(d.argb) );
    if ( c ) {					// found color in dictionary
	pix = c->pix;
	if ( screen == QPaintDevice::x11AppScreen() ) {
	    d.d8.invalid = FALSE;		// color ok
	    d.d8.dirty = FALSE;
	    d.d8.pix = pix;			// use same pixel value
	    if ( c->context != current_alloc_context ) {
		c->context = 0;			// convert to default context
		sd->g_our_alloc[pix] = TRUE;	// reuse without XAllocColor
	    }
	}
	return pix;
    }
#endif

    XColor col;
    col.red   = r << 8;
    col.green = g << 8;
    col.blue  = b << 8;

    bool try_again = FALSE;
    bool try_alloc = !sd->color_reduce;
    int  try_count = 0;

    do {
	// This loop is run until we manage to either allocate or
	// find an approximate color, it stops after a few iterations.

	try_again = FALSE;

	if ( try_alloc && sd->colors_avail &&
	     XAllocColor(dpy, QPaintDevice::x11AppColormap( screen ),&col) ) {
	    // We could allocate the color
	    pix = (uint) col.pixel;
	    if ( screen == QPaintDevice::x11AppScreen() ) {
		d.d8.pix = pix;
		d.d8.invalid = FALSE;
		d.d8.dirty = FALSE;
		sd->g_carr[d.d8.pix] = col;		// update color array
		if ( current_alloc_context == 0 )
		    sd->g_our_alloc[d.d8.pix] = TRUE;	// reuse without XAllocColor
	    }
	} else {
	    // No available colors, or we did not want to allocate one
	    int i;
	    sd->colors_avail = FALSE;		// no more available colors
	    if ( sd->g_carr_fetch ) {		// refetch color array
		sd->g_carr_fetch = FALSE;
		XQueryColors( dpy, QPaintDevice::x11AppColormap( screen ), sd->g_carr,
			      sd->g_cells );
	    }
	    int mindist;
	    i = find_nearest_color( r, g, b, &mindist, sd );

	    if ( mindist != 0 && !try_alloc ) {
		// Not an exact match with an existing color
		int rr = ((r+sd->col_div_r/2)/sd->col_div_r)*sd->col_div_r;
		int rg = ((g+sd->col_div_g/2)/sd->col_div_g)*sd->col_div_g;
		int rb = ((b+sd->col_div_b/2)/sd->col_div_b)*sd->col_div_b;
		int rx = rr - r;
		int gx = rg - g;
		int bx = rb - b;
		int dist = rx*rx + gx*gx + bx*bx; // calculate distance
		if ( dist < mindist ) {
		    // reduced color is closer - try to alloc it
		    r = rr;
		    g = rg;
		    b = rb;
		    col.red   = r << 8;
		    col.green = g << 8;
		    col.blue  = b << 8;
		    try_alloc = TRUE;
		    try_again = TRUE;
		    sd->colors_avail = TRUE;
		    continue; // Try alloc reduced color
		}
	    }

	    if ( i == -1 ) {			// no nearest color?!
		int unused, value;
		hsv(&unused, &unused, &value);
		if (value < 128) { // dark, use black
		    d.argb = qRgb(0,0,0);
		    pix = (uint)BlackPixel( dpy, screen );
		    if ( screen == QPaintDevice::x11AppScreen() ) {
			d.d8.invalid = FALSE;
			d.d8.dirty = FALSE;
			d.d8.pix = pix;
		    }
		} else { // light, use white
		    d.argb = qRgb(0xff,0xff,0xff);
		    pix = (uint)WhitePixel( dpy, screen );
		    if ( screen == QPaintDevice::x11AppScreen() ) {
			d.d8.invalid = FALSE;
			d.d8.dirty = FALSE;
			d.d8.pix = pix;
		    }
		}
		return pix;
	    }
	    if ( sd->g_our_alloc[i] ) {		// we've already allocated it
		; // i == g_carr[i].pixel
	    } else {
		// Try to allocate existing color
		col = sd->g_carr[i];
		if ( XAllocColor(dpy, QPaintDevice::x11AppColormap( screen ), &col) ) {
		    i = (uint)col.pixel;
		    sd->g_carr[i] = col;		// update color array
		    if ( screen == QPaintDevice::x11AppScreen() ) {
			if ( current_alloc_context == 0 )
			    sd->g_our_alloc[i] = TRUE;	// only in the default context
		    }
		} else {
		    // Oops, it's gone again
		    try_count++;
		    try_again    = TRUE;
		    sd->colors_avail = TRUE;
		    sd->g_carr_fetch = TRUE;
		}
	    }
	    if ( !try_again ) {				// got it
		pix = (uint)sd->g_carr[i].pixel;
		if ( screen == QPaintDevice::x11AppScreen() ) {
		    d.d8.invalid = FALSE;
		    d.d8.dirty = FALSE;
		    d.d8.pix = pix;			// allocated X11 color
		}
	    }
	}

    } while ( try_again && try_count < 2 );

    if ( try_again ) {				// no hope of allocating color
	int unused, value;
	hsv(&unused, &unused, &value);
	if (value < 128) { // dark, use black
	    d.argb = qRgb(0,0,0);
	    pix = (uint)BlackPixel( dpy, screen );
	    if ( screen == QPaintDevice::x11AppScreen() ) {
		d.d8.invalid = FALSE;
		d.d8.dirty = FALSE;
		d.d8.pix = pix;
	    }
	} else { // light, use white
	    d.argb = qRgb(0xff,0xff,0xff);
	    pix = (uint)WhitePixel( dpy, screen );
	    if ( screen == QPaintDevice::x11AppScreen() ) {
		d.d8.invalid = FALSE;
		d.d8.dirty = FALSE;
		d.d8.pix = pix;
	    }
       	}
	return pix;
    }
#if 0
    // All colors outside context 0 must go into the dictionary
    bool many = sd->colorDict->count() >= sd->colorDict->size() * 8;
    if ( many && sd->colorDict->size() == col_std_dict ) {
	sd->colorDict->resize( col_large_dict );
    }
    if ( !many || current_alloc_context != 0 ) {
	c = new QColorData;			// insert into color dict
	Q_CHECK_PTR( c );
	c->pix	   = pix;
	c->context = current_alloc_context;
	sd->colorDict->insert( (long)d.argb, c );	// store color in dict
    }
#endif
    return pix;
}

/*!
  Allocates the RGB color and returns the pixel value.

  Allocating a color means to obtain a pixel value from the RGB
  specification.  The pixel value is an index into the global color
  table, but should be considered an arbitrary platform-dependent value.

  The pixel() function calls alloc() if necessary, so in general you
  don't need to call this function.

  \sa enterAllocContext()
*/
// ### 4.0 - remove me?
uint QColor::alloc()
{
    return alloc( -1 );
}

/*!
    \overload

  Returns the pixel value for screen \a screen.

  This value is used by the underlying window system to refer to a color.
  It can be thought of as an index into the display hardware's color table,
  but the value is an arbitrary 32-bit value.

  \sa alloc()
*/
uint QColor::pixel( int screen ) const
{
    if (screen != QPaintDevice::x11AppScreen() &&
	// don't allocate color0 or color1, they have fixed pixel
	// values for all screens
	d.argb != qRgba(255, 255, 255, 1) && d.argb != qRgba(0, 0, 0, 1))
	return ((QColor*)this)->alloc( screen );
    return pixel();
}

#if 0
void QColor::setSystemNamedColor( const QString& name )
#else
void QColor::setSystemNamedColor( const char* name )
#endif
{
    // setSystemNamedColor should look up rgb values from the built in
    // color tables first (see qcolor_p.cpp), and failing that, use
    // the window system's interface for translating names to rgb values...
    // we do this so that things like uic can load an XPM file with named colors
    // and convert it to a png without having to use window system functions...
#if 0
    d.argb = qt_get_rgb_val( name.latin1() );
#else
    d.argb = qt_get_rgb_val( name );
#endif
    QRgb rgb;
#if 0
    if ( qt_get_named_rgb( name.latin1(), &rgb ) ) {
#else
    if ( qt_get_named_rgb( name, &rgb ) ) {
#endif
	setRgb( qRed(rgb), qGreen(rgb), qBlue(rgb) );
	if ( colormodel == d8 ) {
	    d.d8.invalid = FALSE;
	    d.d8.dirty = TRUE;
	    d.d8.pix = 0;
	} else {
	    alloc();
	}
    } else if ( !color_init ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QColor::setSystemNamedColor: Cannot perform this operation "
		  "because QApplication does not exist" );
#endif
	// set color to invalid
	*this = QColor();
    } else {
	XColor col, hw_col;
	if ( XLookupColor(QPaintDevice::x11AppDisplay(),
#if 0
			  QPaintDevice::x11AppColormap(), name.latin1(),
#else
			  QPaintDevice::x11AppColormap(), name,
#endif
			  &col, &hw_col) ) {
	    setRgb( col.red>>8, col.green>>8, col.blue>>8 );
	} else {
	    // set color to invalid
	    *this = QColor();
	}
    }
}

#define MAX_CONTEXTS 16
static int  context_stack[MAX_CONTEXTS];
static int  context_ptr = 0;

static void init_context_stack()
{
    static bool did_init = FALSE;
    if ( !did_init ) {
	did_init = TRUE;
	context_stack[0] = current_alloc_context = 0;
    }
}


/*!
    Enters a color allocation context and returns a non-zero unique
    identifier.

    Color allocation contexts are useful for programs that need to
    allocate many colors and throw them away later, like image
    viewers. The allocation context functions work for true color
    displays as well as for colormap displays, except that
    QColor::destroyAllocContext() does nothing for true color.

    Example:
    \code
    QPixmap loadPixmap( QString fileName )
    {
        static int alloc_context = 0;
	if ( alloc_context )
	    QColor::destroyAllocContext( alloc_context );
	alloc_context = QColor::enterAllocContext();
	QPixmap pm( fileName );
	QColor::leaveAllocContext();
	return pm;
    }
    \endcode

    The example code loads a pixmap from file. It frees up all colors
    that were allocated the last time loadPixmap() was called.

    The initial/default context is 0. Qt keeps a list of colors
    associated with their allocation contexts. You can call
    destroyAllocContext() to get rid of all colors that were allocated
    in a specific context.

    Calling enterAllocContext() enters an allocation context. The
    allocation context lasts until you call leaveAllocContext().
    QColor has an internal stack of allocation contexts. Each call to
    enterAllocContex() must have a corresponding leaveAllocContext().

    \code
	// context 0 active
    int c1 = QColor::enterAllocContext();    // enter context c1
	// context c1 active
    int c2 = QColor::enterAllocContext();    // enter context c2
	// context c2 active
    QColor::leaveAllocContext();             // leave context c2
	// context c1 active
    QColor::leaveAllocContext();             // leave context c1
	// context 0 active
	// Now, free all colors that were allocated in context c2
    QColor::destroyAllocContext( c2 );
    \endcode

    You may also want to set the application's color specification.
    See QApplication::setColorSpec() for more information.

    \sa leaveAllocContext(), currentAllocContext(), destroyAllocContext(),
    QApplication::setColorSpec()
*/

int QColor::enterAllocContext()
{
    static int context_seq_no = 0;
    init_context_stack();
    if ( context_ptr+1 == MAX_CONTEXTS ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QColor::enterAllocContext: Context stack overflow" );
#endif
	return 0;
    }
    current_alloc_context = context_stack[++context_ptr] = ++context_seq_no;
    return current_alloc_context;
}


/*!
    Leaves a color allocation context.

    See enterAllocContext() for a detailed explanation.

    \sa enterAllocContext(), currentAllocContext()
*/

void QColor::leaveAllocContext()
{
    init_context_stack();
    if ( context_ptr == 0 ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QColor::leaveAllocContext: Context stack underflow" );
#endif
	return;
    }
    current_alloc_context = context_stack[--context_ptr];
}


/*!
    Returns the current color allocation context.

    The default context is 0.

    \sa enterAllocContext(), leaveAllocContext()
*/

int QColor::currentAllocContext()
{
    return current_alloc_context;
}


/*!
    Destroys a color allocation context, \e context.

    This function deallocates all colors that were allocated in the
    specified \a context. If \a context == -1, it frees up all colors
    that the application has allocated. If \a context == -2, it frees
    up all colors that the application has allocated, except those in
    the default context.

    The function does nothing for true color displays.

    \sa enterAllocContext(), alloc()
*/

void QColor::destroyAllocContext( int context )
{
    init_context_stack();
    if ( !color_init )
	return;

    int screen;
    for ( screen = 0; screen < screencount; ++screen ) {
	if ( screendata[screen]->g_truecolor )
	    continue;

#if 1
        Q_UNUSED(context);
#else
	ulong pixels[256];
	bool freeing[256];
	memset( freeing, FALSE, screendata[screen]->g_cells*sizeof(bool) );
	QColorData   *d;
	QColorDictIt it( *screendata[screen]->colorDict );
	int i = 0;
	uint rgbv;
	while ( (d=it.current()) ) {
	    rgbv = (uint)it.currentKey();
	    if ( (d->context || context==-1) &&
		 (d->context == context || context < 0) ) {
		if ( !screendata[screen]->g_our_alloc[d->pix] && !freeing[d->pix] ) {
		    // will free this color
		    pixels[i++] = d->pix;
		    freeing[d->pix] = TRUE;
		}
		// remove from dict
		screendata[screen]->colorDict->remove( (long)rgbv );
	    }
	    ++it;
	}
	if ( i )
	    XFreeColors( QPaintDevice::x11AppDisplay(),
			 QPaintDevice::x11AppColormap( screen ),
			 pixels, i, 0 );
#endif
    }
}
