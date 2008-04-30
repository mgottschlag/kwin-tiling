/***************************************************************************
 *   Copyright 2004 Lubos Lunak <l.lunak@kde.org>                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

#include <config-workspace.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <config-X11.h>
#ifdef HAVE_XINERAMA
extern "C" { // for older XFree86 versions
#include <X11/extensions/Xinerama.h>
}
#endif
#include <stdio.h>
#include <unistd.h>
#include <string.h>

# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# endif
#include <time.h>


//#define DEBUG

int main( int argc, char* argv[])
    {
    bool small = false;
    bool test = false;
    bool print_pid = false;
    for( int i = 1;
         i < argc;
         ++i )
        {
        if( strcmp( argv[ i ], "--test" ) == 0 )
            test = true;
        if( strcmp( argv[ i ], "--pid" ) == 0 )
            print_pid = true;
        if( strcmp( argv[ i ], "SimpleSmall" ) == 0 )
            small = true;
        }
    pid_t pid = fork();
    if( pid < -1 )
        {
        perror( "fork()" );
        return -1;
        }
    if( pid != 0 )
        { // parent
        if( print_pid )
            printf( "%d\n", pid );
        return 0;
        }
    // child
    close( 0 ); // close stdin,stdout,stderr, otherwise startkde will block
    close( 1 );
    close( 2 );
    Display* dpy = XOpenDisplay( NULL );
    if( dpy == NULL )
        return 1;
    int sx, sy, sw, sh;
    Window* wins = NULL;
    int wins_count = 0;
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
        if( !small )
            { // create windows covering other xinerama screens
            wins_count = screenCount;
            wins = new Window[ wins_count ];
            for( int i = 1; // not 0
                 i < wins_count;
                 ++i )
                {
                XSetWindowAttributes attrs;
                attrs.override_redirect = True;
                attrs.background_pixel = BlackPixel( dpy, 0 ); // background
                wins[ i ] = XCreateWindow( dpy, DefaultRootWindow( dpy ),
                    xinerama_screeninfo[ i ].x_org, xinerama_screeninfo[ i ].y_org,
                    xinerama_screeninfo[ i ].width, xinerama_screeninfo[ i ].height,
                    0, CopyFromParent, CopyFromParent, CopyFromParent,
                    CWOverrideRedirect | CWBackPixel, &attrs );
                }
            }
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
    const int pw = segment * states + 2 * frame; // size of progressbar
    const int ph = sh / 20 + frame;
    const int px = small ? 0 : sx + ( sw - pw ) / 2; // position in the pixmap
    const int py = small ? 0 : sy + ( sh - ph ) / 2;
    const int x = small ? sx + ( sw - pw ) / 2 : sx; // position of the window
    const int y = small ? sy + ( sh - ph ) / 2 : sy;
    const int w = small ? pw : sw;
    const int h = small ? ph : sh;
    if( wins == NULL )
        {
        wins = new Window[ 1 ];
        wins_count = 1;
        }
    Window win = XCreateWindow( dpy, DefaultRootWindow( dpy ), x, y, w, h,
        0, CopyFromParent, CopyFromParent, CopyFromParent, CWOverrideRedirect, &attrs );
    wins[ 0 ] = win;
    Pixmap pix = XCreatePixmap( dpy, DefaultRootWindow( dpy ), w, h, DefaultDepth( dpy, 0 ));
    XGCValues values;
    values.foreground = BlackPixel( dpy, 0 ); // background
    GC gc = XCreateGC( dpy, pix, GCForeground, &values );
    XFillRectangle( dpy, pix, gc, 0, 0, w, h );
    values.foreground = WhitePixel( dpy, 0 ); // outline
    XChangeGC( dpy, gc, GCForeground, &values );
    XFillRectangle( dpy, pix, gc, px, py, pw, ph );
    values.foreground = BlackPixel( dpy, 0 ); // progressbar
    XChangeGC( dpy, gc, GCForeground, &values );
    XSetWindowBackgroundPixmap( dpy, win, pix );
    XSelectInput( dpy, DefaultRootWindow( dpy ), SubstructureNotifyMask );
    for( int i = 0;
         i < wins_count;
         ++i )
        {
        XSelectInput( dpy, wins[ i ], ButtonPressMask );
        XClassHint class_hint;
        class_hint.res_name = const_cast< char* >( "ksplashsimple" );
        class_hint.res_class = const_cast< char* >( "ksplashsimple" );
        XSetWMProperties( dpy, wins[ i ], NULL, NULL, NULL, NULL, NULL, NULL, &class_hint );
        XMapWindow( dpy, wins[ i ] );
        }
    int pos = 0;
    int state = 1; // cannot check dcop connection - make this state initial
    const int delay = 200; // ms
    time_t final_time = time( NULL ) + 300;
    time_t test_time = time( NULL ) + 1;
    Atom kde_splash_progress = XInternAtom( dpy, "_KDE_SPLASH_PROGRESS", False );
    for(;;)
        {
        while( XPending( dpy ))
            {
            XEvent ev;
            XNextEvent( dpy, &ev );
            if( ev.type == ButtonPress && ev.xbutton.button == Button1 )
                {
                final_time = time( NULL );
                break;
                }
            if( ev.type == ConfigureNotify && ev.xconfigure.event == DefaultRootWindow( dpy ))
                {
                XRaiseWindow( dpy, wins[ 0 ] );
                if( wins_count > 1 )
                    XRestackWindows( dpy, wins, wins_count );
                }
            if( ev.type == ClientMessage && ev.xclient.window == DefaultRootWindow( dpy )
                && ev.xclient.message_type == kde_splash_progress )
                {
                // these are also in ksplashx and in krunner
                const char* s = ev.xclient.data.b;
#ifdef DEBUG
                fprintf( stderr,"MESSAGE: %s\n", s );
#endif
                if( strcmp( s, "initial" ) == 0 && state < 0 )
                    state = 0; // not actually used
                else if( strcmp( s, "kded" ) == 0 && state < 1 )
                    state = 1;
                else if( strcmp( s, "confupdate" ) == 0 && state < 2 )
                    state = 2;
                else if( strcmp( s, "kcminit" ) == 0 && state < 3 )
                    state = 3;
                else if( strcmp( s, "ksmserver" ) == 0 && state < 4 )
                    state = 4;
                else if( strcmp( s, "wm" ) == 0 && state < 5 )
                    state = 5;
                else if( strcmp( s, "desktop" ) == 0 && state < 6 )
                    state = 6;
// This last one isn't used, make splash go away as soon as desktop is ready.
//                else if( strcmp( s, "ready" ) == 0 && state < 7 )
//                    state = 7;
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
                XFillRectangle( dpy, pix, gc, px + frame + pos * segment, py + frame, segment, ph - 2 * frame );
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
    for( int i = 0;
         i < wins_count;
         ++i )
        XDestroyWindow( dpy, wins[ i ] );
    delete[] wins;
    XFreeGC( dpy, gc );
    XCloseDisplay( dpy );
    }
