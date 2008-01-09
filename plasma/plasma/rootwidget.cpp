/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "rootwidget.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>

#include <KWindowSystem>
#include <KAction>
#include <KShortcut>

#include "plasma/corona.h"
#include "plasma/plasma.h"
#include "plasma/svg.h"
#include "plasma/theme.h"

#include "desktopview.h"
#include "dashboardview.h"
#include "plasmaapp.h"


RootWidget::RootWidget()
    : QWidget(0)
{
    setFocusPolicy(Qt::NoFocus);

    // this line also initializes the corona.
    KConfigGroup cg(KGlobal::config(), "General");
    Plasma::Theme::self()->setFont(cg.readEntry("desktopFont", font()));

    Plasma::Corona *corona = PlasmaApp::self()->corona();

    // create a containment for each screen
    QDesktopWidget *desktop = QApplication::desktop();
    int numScreens = desktop->numScreens();
    for (int i = 0; i < numScreens; ++i) {
        createDesktopView(i);
    }

    connect(corona, SIGNAL(newScreen(int)), this, SLOT(createDesktopView(int)));

    //TODO: Make the shortcut configurable
    KAction *showAction = new KAction( this );
    showAction->setText( i18n( "Show Dashboard" ) );
    showAction->setGlobalShortcut( KShortcut( Qt::CTRL + Qt::Key_F12 ) );
    connect( showAction, SIGNAL( triggered() ), this, SLOT( toggleDashboard() ) );
}

void RootWidget::toggleDashboard()
{
    int currentScreen = 0;
    if (QApplication::desktop()->numScreens() > 1) {
        currentScreen = QApplication::desktop()->screenNumber(QCursor::pos());
    }

    if (currentScreen > m_desktops.count() - 1) {
        kWarning() << "we don't have a DesktopView for the current screen!";
        return;
    }

    m_desktops[currentScreen]->toggleDashboard();
}

void RootWidget::setAsDesktop(bool setAsDesktop)
{
    if (setAsDesktop) {
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

        KWindowSystem::setOnAllDesktops(winId(), true);
        KWindowSystem::setType(winId(), NET::Desktop);
        lower();

        QRect desktopGeometry = QApplication::desktop()->geometry();

        if (geometry() != desktopGeometry) {
            setGeometry(desktopGeometry);
        }

        connect(QApplication::desktop(), SIGNAL(resized(int)), SLOT(adjustSize(int)));
    } else {
        setWindowFlags(windowFlags() & ~Qt::FramelessWindowHint);

        KWindowSystem::setOnAllDesktops(winId(), false);
        KWindowSystem::setType(winId(), NET::Normal); 

        disconnect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(adjustSize()));
    }
}

bool RootWidget::isDesktop() const
{
    return KWindowInfo(winId(), NET::WMWindowType).windowType(NET::Desktop);
}

RootWidget::~RootWidget()
{
}

void RootWidget::adjustSize(int screen)
{
    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->geometry());

    foreach (DesktopView *view, m_desktops) {
        if (view->screen() == screen) {
            view->adjustSize();
        }
    }
}

void RootWidget::createDesktopView(int screen)
{
    // we have a new screen. neat.
    DesktopView *view = new DesktopView(screen, this);
    view->setGeometry(QApplication::desktop()->screenGeometry(screen));
    m_desktops.append(view);
}

#include "rootwidget.moc"

