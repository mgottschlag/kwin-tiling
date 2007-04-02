#include "testwin.h"

TestWin::TestWin()
    : KSWidget(0, Qt::X11BypassWindowManagerHint)
{
    setFocusPolicy(Qt::StrongFocus);
}

#include "testwin.moc"
