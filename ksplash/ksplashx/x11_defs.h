/****************************************************************************
**
** This file is based on sources of the Qt GUI Toolkit, used under the terms
** of the GNU General Public License version 2 (see the original copyright
** notice below).
** All further contributions to this file are (and are required to be)
** licensed under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** The original Qt license header follows:
**
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

int screenCount();
QRect screenGeometry(int screen);
QRect totalScreenGeometry();

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
