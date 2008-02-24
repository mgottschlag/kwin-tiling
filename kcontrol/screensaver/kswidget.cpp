#include <config-screensaver.h>

#include "testwin.h"

#include <QX11Info>

#ifdef HAVE_GLXCHOOSEVISUAL
#include <GL/glx.h>
#endif

KSWidget::KSWidget( QWidget* parent, Qt::WindowFlags wf )
    : QWidget( parent, wf ), colormap( None )
{
// use visual with support for double-buffering, for opengl
// this code is (partially) duplicated in kdebase/workspace/krunner/lock/
#ifdef HAVE_GLXCHOOSEVISUAL
    Visual* visual = CopyFromParent;
    int depth = CopyFromParent;
    XSetWindowAttributes attrs;
    int flags = parentWidget() ? 0 : CWOverrideRedirect;
    if( true /*mOpenGLVisual*/ )
    {
        static int attribs[][ 15 ] =
        {
        #define R GLX_RED_SIZE
        #define G GLX_GREEN_SIZE
        #define B GLX_BLUE_SIZE
            { GLX_RGBA, R, 8, G, 8, B, 8, GLX_DEPTH_SIZE, 8, GLX_DOUBLEBUFFER, GLX_STENCIL_SIZE, 1, None },
            { GLX_RGBA, R, 4, G, 4, B, 4, GLX_DEPTH_SIZE, 4, GLX_DOUBLEBUFFER, GLX_STENCIL_SIZE, 1, None },
            { GLX_RGBA, R, 8, G, 8, B, 8, GLX_DEPTH_SIZE, 8, GLX_DOUBLEBUFFER, None },
            { GLX_RGBA, R, 4, G, 4, B, 4, GLX_DEPTH_SIZE, 4, GLX_DOUBLEBUFFER, None },
            { GLX_RGBA, R, 8, G, 8, B, 8, GLX_DEPTH_SIZE, 8, GLX_STENCIL_SIZE, 1, None },
            { GLX_RGBA, R, 4, G, 4, B, 4, GLX_DEPTH_SIZE, 4, GLX_STENCIL_SIZE, 1, None },
            { GLX_RGBA, R, 8, G, 8, B, 8, GLX_DEPTH_SIZE, 8, None },
            { GLX_RGBA, R, 4, G, 4, B, 4, GLX_DEPTH_SIZE, 4, None },
            { GLX_RGBA, GLX_DEPTH_SIZE, 8, GLX_DOUBLEBUFFER, GLX_STENCIL_SIZE, 1, None },
            { GLX_RGBA, GLX_DEPTH_SIZE, 8, GLX_DOUBLEBUFFER, None },
            { GLX_RGBA, GLX_DEPTH_SIZE, 8, GLX_STENCIL_SIZE, 1, None },
            { GLX_RGBA, GLX_DEPTH_SIZE, 8, None }
        #undef R
        #undef G
        #undef B
        };
        for( unsigned int i = 0;
             i < sizeof( attribs ) / sizeof( attribs[ 0 ] );
             ++i )
        {
            if( XVisualInfo* info = glXChooseVisual( x11Info().display(), x11Info().screen(), attribs[ i ] ))
            {
                visual = info->visual;
                depth = info->depth;
                colormap = XCreateColormap( x11Info().display(), RootWindow( x11Info().display(), x11Info().screen()), visual, AllocNone );
                attrs.colormap = colormap;
                flags |= CWColormap;
                XFree( info );
                break;
            }
        }
    }
    attrs.override_redirect = 1;
    Window w = XCreateWindow( x11Info().display(), RootWindow( x11Info().display(), x11Info().screen()),
        x(), y(), width(), height(), 0, depth, InputOutput, visual, flags, &attrs );
    if( parentWidget())
        XReparentWindow( x11Info().display(), w, parentWidget()->winId(), 0, 0 );
    create( w, false, true );
#endif
}

KSWidget::~KSWidget()
{
#ifdef HAVE_GLXCHOOSEVISUAL
    if( colormap != None )
        XFreeColormap( x11Info().display(), colormap );
#endif
}

#include "kswidget.moc"
