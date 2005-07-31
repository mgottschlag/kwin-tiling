#include <kwin.h>
#include "testwin.h"

TestWin::TestWin()
    : KSWidget(0)
{
    overrideWindowFlags(Qt::X11BypassWindowManagerHint);
    setFocusPolicy(Qt::StrongFocus);
    KWin::setState( winId(), NET::StaysOnTop );
}

#include "testwin.moc"
