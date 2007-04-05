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
// this code is (apparently) duplicated in kdebase/workspace/krunner/lock/
#ifdef HAVE_GLXCHOOSEVISUAL
    Visual* visual = CopyFromParent;
    XSetWindowAttributes attrs;
    int flags = 0;
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
                colormap = XCreateColormap( x11Info().display(), RootWindow( x11Info().display(), x11Info().screen()), visual, AllocNone );
                attrs.colormap = colormap;
                flags |= CWColormap;
                XFree( info );
                break;
            }
        }
    }
    Window w = XCreateWindow( x11Info().display(), parentWidget() ? parentWidget()->winId() : RootWindow( x11Info().display(), x11Info().screen()),
        x(), y(), width(), height(), 0, x11Info().depth(), InputOutput, visual, flags, &attrs );
    create( w );
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
