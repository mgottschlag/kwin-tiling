#include <kwin.h>
#include "testwin.h"

TestWin::TestWin()
    : KSWidget(0)
{
#ifdef __GNUC__
#warning overrideWindowFlags(Qt::X11BypassWindowManagerHint) crash kcmshell screensaver -> slot test
#endif
    //overrideWindowFlags(Qt::X11BypassWindowManagerHint);
    setFocusPolicy(Qt::StrongFocus);
    KWin::setState( winId(), NET::StaysOnTop );
}

#include "testwin.moc"
