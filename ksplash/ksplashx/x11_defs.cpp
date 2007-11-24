/*
 *   Copyright (C) 2007 Luboš Luňák <l.lunak@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2
 *   or later as published by the Free Software Foundation
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

#include <config-workspace.h>
#include <config-X11.h>

#include "qnamespace.h"
#include "qcolor.h"
#include "x11_defs.h"

#include <string.h>

#ifdef HAVE_XINERAMA
extern "C" { // for older XFree86 versions
#include <X11/extensions/Xinerama.h>
}
#endif

Display* spl_dpy;
int spl_screen;
void* spl_visual;
int spl_depth;
Colormap spl_colormap;
int spl_cells;

static Display* appDpy;
static int appScreenCount;

static int sx, sy, sw, sh;

QRect screenGeometry()
    {
    return QRect( sx, sy, sw, sh );
    }

void detectScreenGeometry()
    {
    Display* dpy = x11Display();
#ifdef HAVE_XINERAMA
    // Xinerama code from Qt
    XineramaScreenInfo *xinerama_screeninfo = 0;
    int unused;
    bool use_xinerama = XineramaQueryExtension( dpy, &unused, &unused )
        && XineramaIsActive( dpy );
    if (use_xinerama)
        {
        int screenCount;
	xinerama_screeninfo = XineramaQueryScreens( dpy, &screenCount );
        QRect sg(xinerama_screeninfo[ 0 ].x_org,
                 xinerama_screeninfo[ 0 ].y_org,
                 xinerama_screeninfo[ 0 ].width,
                 xinerama_screeninfo[ 0 ].height);

        for (int s = 1; s < screenCount; ++s)
        {
            QRect cs(xinerama_screeninfo[s].x_org,
                    xinerama_screeninfo[s].y_org,
                    xinerama_screeninfo[s].width,
                    xinerama_screeninfo[s].height);

            if (sg.intersects(cs))
                sg = sg.unite(cs);
        }

        sx = sg.x();
        sy = sg.y();
        sw = sg.width();
        sh = sg.height();
        }
    else
#endif
        {
        sx = sy = 0;
        sw = WidthOfScreen( ScreenOfDisplay( dpy, DefaultScreen( dpy )));
	sh = HeightOfScreen( ScreenOfDisplay( dpy, DefaultScreen( dpy )));
        }
    }

void createColormap()
    {
		    // use default colormap
		    XStandardColormap *stdcmap;
		    VisualID vid =
			XVisualIDFromVisual((Visual *) spl_visual );
		    int i, count;

		    spl_colormap = 0;
                    Display* appDpy = spl_dpy;
                    int screen = spl_screen;

		    if ( true /*|| ! serverVendor.contains( "Hewlett-Packard" )*/ ) {
			// on HPUX 10.20 local displays, the RGB_DEFAULT_MAP colormap
			// doesn't give us correct colors. Why this happens, I have
			// no clue, so we disable this for HPUX
			if (XGetRGBColormaps(appDpy,
					     RootWindow(spl_dpy,spl_screen)/*QPaintDevice::x11AppRootWindow( screen )*/,
					     &stdcmap, &count, XA_RGB_DEFAULT_MAP)) {
			    i = 0;
			    while (i < count &&
				   spl_colormap == 0) {
				if (stdcmap[i].visualid == vid) {
				    spl_colormap =
					stdcmap[i].colormap;
				}
				i++;
			    }

			    XFree( (char *)stdcmap );
			}
		    }

		    if (spl_colormap == 0) {
			spl_colormap =
			    DefaultColormap(appDpy, screen);
		    }
    }

static
int x11ErrorHandler(Display *d, XErrorEvent *e)
    {
    char msg[80], req[80], number[80];
    XGetErrorText(d, e->error_code, msg, sizeof(msg));
    sprintf(number, "%d", e->request_code);
    XGetErrorDatabaseText(d, "XRequest", number, "<unknown>", req, sizeof(req));

    fprintf(stderr, "%s(0x%lx): %s\n", req, e->resourceid, msg);
    return 0;
    }

bool openDisplay()
    {
    spl_dpy = XOpenDisplay( NULL );
    if( spl_dpy == NULL )
        return false;
    XSetErrorHandler( x11ErrorHandler );
    spl_screen = DefaultScreen( spl_dpy );
    spl_depth = DefaultDepth( spl_dpy, spl_screen );
    spl_visual = DefaultVisual( spl_dpy, spl_screen );
    spl_cells = DisplayCells( spl_dpy, spl_screen );
    appScreenCount = ScreenCount( spl_dpy );
    appDpy = spl_dpy;
    createColormap();
    detectScreenGeometry();
    QColor::initialize();
    return true;
    }

void closeDisplay()
    {
    XCloseDisplay( spl_dpy );
    }

static GC*	app_gc_ro	= 0;		// read-only GC
static GC*	app_gc_tmp	= 0;		// temporary GC
static GC*	app_gc_ro_m	= 0;		// read-only GC (monochrome)
static GC*	app_gc_tmp_m	= 0;		// temporary GC (monochrome)

static GC create_gc( int scrn, bool monochrome )
{
    GC gc;
    if ( monochrome ) {
	Pixmap pm = XCreatePixmap( appDpy, RootWindow( appDpy, scrn ), 8, 8, 1 );
	gc = XCreateGC( appDpy, pm, 0, 0 );
	XFreePixmap( appDpy, pm );
    } else {
	if ( QPaintDevice::x11AppDefaultVisual( scrn ) ) {
	    gc = XCreateGC( appDpy, RootWindow( appDpy, scrn ), 0, 0 );
	} else {
	    Window w;
	    XSetWindowAttributes a;
	    a.background_pixel = Qt::black.pixel( scrn );
	    a.border_pixel = Qt::black.pixel( scrn );
	    a.colormap = QPaintDevice::x11AppColormap( scrn );
	    w = XCreateWindow( appDpy, RootWindow( appDpy, scrn ), 0, 0, 100, 100,
			       0, QPaintDevice::x11AppDepth( scrn ), InputOutput,
			       (Visual*)QPaintDevice::x11AppVisual( scrn ),
			       CWBackPixel|CWBorderPixel|CWColormap, &a );
	    gc = XCreateGC( appDpy, w, 0, 0 );
	    XDestroyWindow( appDpy, w );
	}
    }
    XSetGraphicsExposures( appDpy, gc, False );
    return gc;
}

GC qt_xget_readonly_gc( int scrn, bool monochrome )	// get read-only GC
{
    if ( scrn < 0 || scrn >= appScreenCount ) {
#if 0
	qDebug("invalid screen %d %d", scrn, appScreenCount );
	QWidget* bla = 0;
	bla->setName("hello");
#endif
    }
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_ro_m )			// create GC for bitmap
	    memset( (app_gc_ro_m = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_ro_m[scrn] )
	    app_gc_ro_m[scrn] = create_gc( scrn, true );
	gc = app_gc_ro_m[scrn];
    } else {					// create standard GC
	if ( !app_gc_ro )
	    memset( (app_gc_ro = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_ro[scrn] )
	    app_gc_ro[scrn] = create_gc( scrn, false );
	gc = app_gc_ro[scrn];
    }
    return gc;
}

GC qt_xget_temp_gc( int scrn, bool monochrome )		// get temporary GC
{
    if ( scrn < 0 || scrn >= appScreenCount ) {
#if 0
	qDebug("invalid screen (tmp) %d %d", scrn, appScreenCount );
	QWidget* bla = 0;
	bla->setName("hello");
#endif
    }
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_tmp_m )			// create GC for bitmap
	    memset( (app_gc_tmp_m = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_tmp_m[scrn] )
	    app_gc_tmp_m[scrn] = create_gc( scrn, true );
	gc = app_gc_tmp_m[scrn];
    } else {					// create standard GC
	if ( !app_gc_tmp )
	    memset( (app_gc_tmp = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_tmp[scrn] )
	    app_gc_tmp[scrn] = create_gc( scrn, false );
	gc = app_gc_tmp[scrn];
    }
    return gc;
}
