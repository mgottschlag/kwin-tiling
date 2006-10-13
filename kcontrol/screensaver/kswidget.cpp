#include <config-screensaver.h>

#include <kwin.h>
#include "testwin.h"
#include <X11/Xlib.h>

#ifdef HAVE_GLXCHOOSEVISUAL
#include <GL/glx.h>
#endif

KSWidget::KSWidget( QWidget* parent )
    : QX11EmbedWidget( parent ), colormap( None )
{
// use visual with support for double-buffering, for opengl
// this code is duplicated in kdebase/kdesktop/lock/
#ifdef HAVE_GLXCHOOSEVISUAL
    Visual* visual = CopyFromParent;
    XSetWindowAttributes attrs;
    int flags = 0;
    if( true /*mOpenGLVisual*/ )
    {
        int attribs[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_BUFFER_SIZE, x11Depth(), None };
        if( XVisualInfo* i = glXChooseVisual( x11Display(), x11Screen(), attribs ))
        {
            visual = i->visual;
            colormap = XCreateColormap( x11Display(), RootWindow( x11Display(), x11Screen()), visual, AllocNone );
            attrs.colormap = colormap;
            flags |= CWColormap;
            XFree( i );
        }
    }
    Window w = XCreateWindow( x11Display(), parentWidget() ? parentWidget()->winId() : RootWindow( x11Display(), x11Screen()),
        x(), y(), width(), height(), 0, x11Depth(), InputOutput, visual, flags, &attrs );
    create( w );
#endif
}

KSWidget::~KSWidget()
{
    if( colormap != None )
        XFreeColormap( x11Display(), colormap );
}

#include "kswidget.moc"
