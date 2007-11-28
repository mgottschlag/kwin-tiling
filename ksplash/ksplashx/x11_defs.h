#ifndef _X11_DEFS_H
#define _X11_DEFS_H

#include "qrect.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

extern Display* spl_dpy;
extern int spl_screen;
extern void* spl_visual;
extern int spl_depth;
extern Colormap spl_colormap;
extern int spl_cells;

inline int x11Depth() { return spl_depth; }
inline Display *x11Display() { return spl_dpy; }
inline Display * qt_xdisplay() { return spl_dpy; }
inline void *x11Visual() { return spl_visual; }
inline int x11Screen() { return spl_screen; }
inline int x11Cells() { return spl_cells; }
GC qt_xget_readonly_gc( int scrn, bool monochrome );	// get read-only GC
GC qt_xget_temp_gc( int scrn, bool monochrome );		// get temporary GC
inline Colormap x11AppColormap() { return spl_colormap; }

QRect screenGeometry();

bool openDisplay();
void closeDisplay();

class QPaintDevice
    {
    public:
        static Display* x11AppDisplay() { return ::x11Display(); }
        static int x11AppScreen() { return ::x11Screen(); }
        static void *x11AppVisual( int = -1 ) { return ::x11Visual(); }
        static void *x11AppDefaultVisual( int = -1 ) { return ::x11Visual(); }
        static int x11AppDepth( int = -1 ) { return ::x11Depth(); }
        static int x11AppCells( int = -1 ) { return ::x11Cells(); }
        static Colormap x11AppColormap( int = -1 ) { return ::x11AppColormap(); }
        static Colormap x11AppDefaultColormap( int = -1 ) { return ::x11AppColormap(); }
    };

class QApplication
    {
    public:
        enum ColorSpec { NormalColor=0, CustomColor=1, ManyColor=2 };
        static int colorSpec() { return NormalColor; }
    };

#endif
