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

#include "plasma/containment.h"
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

    //TODO: Make the shortcut configurable
    KAction *showAction = new KAction( this );
    showAction->setText( i18n( "Show Dashboard" ) );
    showAction->setObjectName( "Show Dashboard" ); // NO I18N
    showAction->setGlobalShortcut( KShortcut( Qt::CTRL + Qt::Key_F12 ) );
    connect( showAction, SIGNAL( triggered() ), this, SLOT( toggleDashboard() ) );
}

void RootWidget::toggleDashboard()
{
    int currentScreen = 0;
    if (QApplication::desktop()->numScreens() > 1) {
        currentScreen = QApplication::desktop()->screenNumber(QCursor::pos());
    }

    DesktopView *view = viewForScreen(currentScreen);
    if (!view) {
        kWarning() << "we don't have a DesktopView for the current screen!";
        return;
    }

    view->toggleDashboard();
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
    DesktopView *view = viewForScreen(screen);
    
    if (view) {
        if (screen < desktop->numScreens()) {
            view->adjustSize();
        } else {
            // the screen was removed, so we'll destroy the
            // corresponding view
            kDebug() << "removing the view for screen" << screen;
            view->setContainment(0);
            m_desktops.removeAll(view);
            delete view;
        }
    }
}

DesktopView* RootWidget::viewForScreen(int screen) const
{
    foreach (DesktopView *view, m_desktops) {
        if (view->screen() == screen) {
            return view;
        }
    }

    return 0;
}

void RootWidget::createDesktopView(Plasma::Containment *containment)
{
    if (viewForScreen(containment->screen())) {
        // we already have a view for this screen
        return;
    }

    kDebug() << "creating a view for" << containment->screen() << "and we have"
        << QApplication::desktop()->numScreens() << "screens";

    // we have a new screen. neat.
    DesktopView *view = new DesktopView(containment, this);
    view->setGeometry(QApplication::desktop()->screenGeometry(containment->screen()));
    m_desktops.append(view);
    view->show();
}

void RootWidget::screenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment* containment)
{
    kDebug() << "was, is, containment:" << wasScreen << isScreen << (QObject*)containment;
    if (containment->containmentType() == Plasma::Containment::PanelContainment) {
        // we don't care about panel containments changing screens on us
        return;
    }

    if (wasScreen > -1) {
        DesktopView *view = viewForScreen(isScreen);
        if (view) {
            view->setContainment(0);
        }
    }

    if (isScreen > -1 && isScreen < QApplication::desktop()->numScreens()) {
        DesktopView *view = viewForScreen(isScreen);
        if (view) {
            view->setContainment(containment);
        } else {
            createDesktopView(containment);
        }
    }
}

#include "rootwidget.moc"

