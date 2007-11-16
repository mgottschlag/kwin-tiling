/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2007 André Duffeck <duffeck@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "dashboardview.h"

#include "plasma/applet.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/svg.h"
#include "plasma/appletbrowser.h"

#include "plasmaapp.h"

#include <KWindowSystem>

DashBoardView::DashBoardView(int screen, QWidget *parent)
    : Plasma::View(screen, PlasmaApp::self()->corona(), parent), 
      m_appletBrowser( 0 )
{
    setContextMenuPolicy(Qt::NoContextMenu);
    setWindowFlags( Qt::FramelessWindowHint );
    setWindowOpacity( 0.9 );
    setWindowState( Qt::WindowFullScreen );
    KWindowSystem::setState(winId(), NET::KeepAbove);

    //FIXME: this OUGHT to be true if we don't have composite, probably
    setDrawWallpaper(false);
    hide();

    connect( scene(), SIGNAL(launchActivated()), SLOT(hide()) );
}

void DashBoardView::showAppletBrowser()
{
    if (!m_appletBrowser) {
        m_appletBrowser = new Plasma::AppletBrowser(qobject_cast<Plasma::Corona *>(scene()), this, Qt::FramelessWindowHint );
        m_appletBrowser->setApplication();
        m_appletBrowser->setAttribute(Qt::WA_DeleteOnClose);
        m_appletBrowser->setWindowTitle(i18n("Add Widgets"));
        connect(m_appletBrowser, SIGNAL(destroyed()), this, SLOT(appletBrowserDestroyed()));
        KWindowSystem::setState(m_appletBrowser->winId(), NET::KeepAbove);
        m_appletBrowser->move( 0, 0 );
    }

    m_appletBrowser->show();
}

void DashBoardView::appletBrowserDestroyed()
{
    m_appletBrowser = 0;
}

DashBoardView::~DashBoardView()
{
}

void DashBoardView::toggleVisibility()
{
    if (isHidden()) {
      show();
      raise();
    
      showAppletBrowser();
    } else {
      hide();
      if (m_appletBrowser) {
          m_appletBrowser->hide();
      }
    }
}

#include "dashboardview.moc"

