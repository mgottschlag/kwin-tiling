/*****************************************************************

Copyright (c) 1996-2001,2002 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

// kicker.h needs to be up here due to compilation errors that result
// it's placed further on. >=(
#include "kicker.h"
#include "kickerSettings.h"

#include <kwin.h>
#include <kwinmodule.h>
#include <netwm.h>
#include <QX11Info>

#include <QList>

#include "showdesktop.h"
#include "showdesktop.moc"

ShowDesktop* ShowDesktop::self()
{
    static ShowDesktop showDesktop;
    return &showDesktop;
}

ShowDesktop::ShowDesktop()
  : QObject(),
    m_showingDesktop(false)
{
    // This feature is implemented in KWin. Keep old code in Kicker for the case
    // KDE is running with another WM without the feature.
    NETRootInfo i( QX11Info::display(), NET::Supported );
    m_wmSupport = i.isSupported( NET::WM2ShowingDesktop );
    if( m_wmSupport )
    {
        connect( Kicker::self()->kwinModule(), SIGNAL( showingDesktopChanged( bool )),
            SLOT( showingDesktopChanged( bool )));
        showingDesktopChanged( m_showingDesktop = Kicker::self()->kwinModule()->showingDesktop());
    }
}

void ShowDesktop::slotCurrentDesktopChanged(int)
{
    showDesktop( false );
}

void ShowDesktop::slotWindowAdded(WId w)
{
    if (!m_showingDesktop)
    {
        return;
    }

    NETWinInfo inf(QX11Info::display(), w, QX11Info::appRootWindow(),
                   NET::XAWMState | NET::WMWindowType);
    NET::WindowType windowType = inf.windowType(NET::AllTypesMask);

    if ((windowType == NET::Normal || windowType == NET::Unknown) &&
        inf.mappingState() == NET::Visible)
    {
        KConfig _kwincfg( "kwinrc" );
        KConfigGroup kwincfg(&_kwincfg, "Windows" );
        if( kwincfg.readEntry( "ShowDesktopIsMinimizeAll", false ))
        {
            m_iconifiedList.clear();
            m_showingDesktop = false;
            emit desktopShown(false);
        }
        else
        {
            m_activeWindow = w;
            showDesktop(false);
        }
    }
}

void ShowDesktop::slotWindowChanged(WId w, unsigned int dirty)
{
    if (!m_showingDesktop)
    {
        return;
    }

    if (dirty & NET::XAWMState)
    {
        NETWinInfo inf(QX11Info::display(), w, QX11Info::appRootWindow(),
                       NET::XAWMState | NET::WMWindowType);
        NET::WindowType windowType = inf.windowType(NET::AllTypesMask);

        if ((windowType == NET::Normal || windowType == NET::Unknown) &&
            inf.mappingState() == NET::Visible)
        {
            // a window was deiconified, abort the show desktop mode.
            m_iconifiedList.clear();
            m_showingDesktop = false;
            emit desktopShown(false);
        }
    }
}

void ShowDesktop::showDesktop( bool b )
{
    if (b == m_showingDesktop)
    {
        return;
    }

    if( m_wmSupport )
    {
        NETRootInfo i( QX11Info::display(), 0 );
        i.setShowingDesktop( b );
        return;
    }

    if (b)
    {
        m_activeWindow = Kicker::self()->kwinModule()->activeWindow();
        m_iconifiedList.clear();

        const QList<WId> windows = Kicker::self()->kwinModule()->windows();
        QList<WId>::const_iterator itEnd = windows.constEnd();
        for (QList<WId>::const_iterator it = windows.constBegin();
             it != itEnd;
             ++it)
        {
            WId w = *it;

            NETWinInfo info( QX11Info::display(), w, QX11Info::appRootWindow(),
                             NET::XAWMState | NET::WMDesktop );

            if (info.mappingState() == NET::Visible &&
                (info.desktop() == NETWinInfo::OnAllDesktops ||
                 info.desktop() == (int)Kicker::self()->kwinModule()->currentDesktop()))
            {
                m_iconifiedList.append( w );
            }
        }

        // find first, hide later, otherwise transients may get minimized
        // with the window they're transient for
        QVector<WId>::iterator itEnd2 = m_iconifiedList.end();
        for (QVector<WId>::iterator it = m_iconifiedList.begin();
             it != itEnd2;
             ++it)
        {
            KWin::iconifyWindow( *it, false );
        }

        // on desktop changes or when a window is deiconified, we abort the show desktop mode
        connect(Kicker::self()->kwinModule(), SIGNAL(currentDesktopChanged(int)),
                SLOT(slotCurrentDesktopChanged(int)));
        connect(Kicker::self()->kwinModule(), SIGNAL(windowChanged(WId,unsigned int)),
                SLOT(slotWindowChanged(WId,unsigned int)));
        connect(Kicker::self()->kwinModule(), SIGNAL(windowAdded(WId)),
                SLOT(slotWindowAdded(WId)));
    }
    else
    {
        disconnect(Kicker::self()->kwinModule(), SIGNAL(currentDesktopChanged(int)),
                   this, SLOT(slotCurrentDesktopChanged(int)));
        disconnect(Kicker::self()->kwinModule(), SIGNAL(windowChanged(WId,unsigned int)),
                   this, SLOT(slotWindowChanged(WId,unsigned int)));
        disconnect(Kicker::self()->kwinModule(), SIGNAL(windowAdded(WId)),
                   this, SLOT(slotWindowAdded(WId)));

        QVector<WId>::const_iterator itEnd = m_iconifiedList.constEnd();
        for (QVector<WId>::const_iterator it = m_iconifiedList.constBegin();
             it != itEnd;
             ++it)
        {
            KWin::deIconifyWindow(*it, false);
        }

        KWin::forceActiveWindow(m_activeWindow);
    }

    m_showingDesktop = b;
    emit desktopShown(m_showingDesktop);
}

void ShowDesktop::showingDesktopChanged( bool showing )
{
    m_showingDesktop = showing;
    emit desktopShown(m_showingDesktop);
}
