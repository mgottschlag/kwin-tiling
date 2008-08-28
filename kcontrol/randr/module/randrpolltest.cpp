#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <stdio.h>
#include <unistd.h>

int main( int argc, char* argv[] )
    {
    Display* dpy = XOpenDisplay( NULL );
    XSetWindowAttributes attrs;
    Window w = XCreateWindow( dpy, DefaultRootWindow( dpy ), 0, 0, 100, 100, 0, CopyFromParent, CopyFromParent,
        CopyFromParent, 0, &attrs );
//    XMapWindow( dpy, w );
    int base, error;
    if( !XRRQueryExtension( dpy, &base, &error ))
        return 1;
    int major = 1;
    int minor = 2;
    if( !XRRQueryVersion( dpy, &major, &minor ) || major < 1 || (major == 1 && minor < 2 ))
        return 2;
    XRRSelectInput( dpy, w,
        RRScreenChangeNotifyMask | RRCrtcChangeNotifyMask | RROutputChangeNotifyMask | RROutputPropertyNotifyMask );
    for(;;)
        {
        XEvent ev;
        int a, b, c, d;
        static int cnt = 0;
        if( ++cnt % 30 == 0 )
            {
//            XRRFreeScreenResources(XRRGetScreenResources( dpy, w ));
            XRRGetScreenSizeRange( dpy, w, &a, &b, &c, &d );
//            XSync( dpy, False );
            printf( "Poll\n" );
            }
        sleep( 1 );
        if( !XPending( dpy ))
            continue;
        XNextEvent( dpy, &ev );
        if( ev.xany.type == base + RRScreenChangeNotify )
            {
            printf( "Screen Change\n" );
            }
        if( ev.xany.type == base + RRNotify )
            {
            XRRNotifyEvent* e = reinterpret_cast< XRRNotifyEvent* >( &ev );
            switch( e->subtype )
                {
                case RRNotify_CrtcChange:
                    printf( "Crtc Change\n" );
                    break;
                case RRNotify_OutputChange:
                    printf( "Output Change\n" );
                    break;
                case RRNotify_OutputProperty:
                    printf( "Output Property Change\n" );
                    break;
                default:
                    printf( "Unknown Notify\n" );
                    break;
                }
            }
        }
    XCloseDisplay( dpy );
    }
