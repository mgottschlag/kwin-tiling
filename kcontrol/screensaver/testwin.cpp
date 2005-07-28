#include <kwin.h>
#include "testwin.h"

class QXEmbed;

TestWin::TestWin()
    : KSWidget(0, 0, Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WX11BypassWM )
{
    setFocusPolicy(Qt::StrongFocus);
    KWin::setState( winId(), NET::StaysOnTop );
}

#include "testwin.moc"
