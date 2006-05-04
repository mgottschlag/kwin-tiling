/***************************************************************************
 *   Copyright 2004 Lubos Lunak <l.lunak@kde.org>                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

#include <config.h>

#include <X11/Xlib.h>
#ifdef HAVE_XINERAMA
extern "C" { // for older XFree86 versions
#include <X11/extensions/Xinerama.h>
}
#endif
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif


//#define DEBUG

int main( int argc, char* argv[])
    {
    if( fork() != 0 )
        return 0;
    Display* dpy = XOpenDisplay( NULL );
    if( dpy == NULL )
        return 1;
    bool test = false;
    if( argc == 2 && strcmp( argv[ 1 ], "--test" ) == 0 )
        test = true;
    int sx, sy, sw, sh;
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
        sx = xinerama_screeninfo[ 0 ].x_org;
        sy = xinerama_screeninfo[ 0 ].y_org;
        sw = xinerama_screeninfo[ 0 ].width;
        sh = xinerama_screeninfo[ 0 ].height;
        }
    else
#endif
        {
        sx = sy = 0;
        sw = WidthOfScreen( ScreenOfDisplay( dpy, DefaultScreen( dpy )));
	sh = HeightOfScreen( ScreenOfDisplay( dpy, DefaultScreen( dpy )));
        }
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    const int states = 6;
    const int frame = 3;
    const int segment = sw / 2 / states;
    const int w = segment * states + 2 * frame;
    const int h = sh / 20 + frame;
    Window win = XCreateWindow( dpy, DefaultRootWindow( dpy ), sx + ( sw - w ) / 2, sy + ( sh - h ) /2, w, h,
        0, CopyFromParent, CopyFromParent, CopyFromParent, CWOverrideRedirect, &attrs );
    Pixmap pix = XCreatePixmap( dpy, DefaultRootWindow( dpy ), w, h, DefaultDepth( dpy, 0 ));
    XGCValues values;
    values.foreground = WhitePixel( dpy, 0 );
    GC gc = XCreateGC( dpy, pix, GCForeground, &values );
    XFillRectangle( dpy, pix, gc, 0, 0, w, h );
    values.foreground = BlackPixel( dpy, 0 );
    XChangeGC( dpy, gc, GCForeground, &values );
//    XFillRectangle( dpy, pix, gc, 0, 0, w, frame );
//    XFillRectangle( dpy, pix, gc, 0, h - frame, w, frame );
//    XFillRectangle( dpy, pix, gc, 0, 0, frame, h );
//    XFillRectangle( dpy, pix, gc, w - frame, 0, frame, h );
    XSetWindowBackgroundPixmap( dpy, win, pix );
    XSelectInput( dpy, win, ButtonPressMask );
    XSelectInput( dpy, DefaultRootWindow( dpy ), SubstructureNotifyMask );
    XMapWindow( dpy, win );
    int pos = 0;
    int state = 1; // cannot check dcop connection - make this state initial
    const int delay = 10; // ms
    time_t final_time = time( NULL ) + 60;
    time_t test_time = time( NULL ) + 1;
    Atom kde_splash_progress = XInternAtom( dpy, "_KDE_SPLASH_PROGRESS", False );
    for(;;)
        {
        while( XPending( dpy ))
            {
            XEvent ev;
            XNextEvent( dpy, &ev );
            if( ev.type == ButtonPress && ev.xbutton.window == win && ev.xbutton.button == Button1 )
                {
                final_time = time( NULL );
                break;
                }
            if( ev.type == ConfigureNotify && ev.xconfigure.event == DefaultRootWindow( dpy ))
                XRaiseWindow( dpy, win );
            if( ev.type == ClientMessage && ev.xclient.window == DefaultRootWindow( dpy )
                && ev.xclient.message_type == kde_splash_progress )
                {
                // based on ksplash
                const char* s = ev.xclient.data.b;
#ifdef DEBUG
                fprintf( stderr,"MESSAGE: %s\n", s );
#endif
                if( strcmp( s, "dcop" ) == 0 && state < 1 )
                    state = 1; // not actually used, state starts from 1, because dcop cannot be checked
                else if( strcmp( s, "kded" ) == 0 && state < 2 )
                    state = 2;
                else if( strcmp( s, "kcminit" ) == 0 )
                    ; // unused
                else if( strcmp( s, "ksmserver" ) == 0 && state < 3 )
                    state = 3;
                else if( strcmp( s, "wm started" ) == 0 && state < 4 )
                    state = 4;
                else if( strcmp( s, "kdesktop" ) == 0 && state < 5 )
                    state = 5;
                else if(( strcmp( s, "kicker" ) == 0 || strcmp( s, "session ready" ) == 0 ) && state < 6 )
                    state = 6;
                }
            }
        if( test && time( NULL ) >= test_time )
            {
            ++state;
            test_time = time( NULL ) + 1;
            }
        if( pos != state )
            {
            while( pos < state && pos < states )
                {
#ifdef DEBUG
                fprintf( stderr, "POS: %d\n", pos );
#endif
                final_time = time( NULL ) + 60;
                XFillRectangle( dpy, pix, gc, frame + pos * segment, frame, segment, h - 2 * frame );
                XSetWindowBackgroundPixmap( dpy, win, pix );
                XClearWindow( dpy, win );
                ++pos;
                if( pos >= states )
                    {
#ifdef DEBUG
                    fprintf( stderr, "CLOSING DOWN\n" );
#endif
                    final_time = time( NULL ) + 2;
                    }
                }
            }
        fd_set set;
        FD_ZERO( &set );
        FD_SET( XConnectionNumber( dpy ), &set );
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = delay * 1000;
        select( XConnectionNumber( dpy ) + 1, &set, NULL, NULL, &tv );
        if( time( NULL ) >= final_time )
            {
#ifdef DEBUG
            fprintf( stderr, "EXITING\n" );
#endif
            break; // --->
            }
        }
    XFreePixmap( dpy, pix );
    XDestroyWindow( dpy, win );
    XFreeGC( dpy, gc );
    XCloseDisplay( dpy );
    }
