#include <kwin.h>
#include "testwin.h"

class QXEmbed;

TestWin::TestWin()
    : QXEmbed(0, 0, WStyle_Customize | WStyle_NoBorder | WX11BypassWM )
{
    setFocusPolicy(StrongFocus);
    KWin::setState( winId(), NET::StaysOnTop );
}

