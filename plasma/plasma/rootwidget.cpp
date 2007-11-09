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

#include "plasma/corona.h"
#include "plasma/plasma.h"
#include "plasma/svg.h"

#include "desktopview.h"
#include "plasmaapp.h"

RootWidget::RootWidget()
    : QWidget(0)
{
    setFocusPolicy(Qt::NoFocus);

    QDesktopWidget *desktop = QApplication::desktop();
    int numScreens = desktop->numScreens();
    // create a containment for each screen
    //FIXME: we need to respond to randr changes
    for (int i = 0; i < numScreens; ++i) {
        DesktopView *view = new DesktopView(this, i);
        view->setGeometry(desktop->screenGeometry(i));
        m_desktops.append(view);
    }

    PlasmaApp::self()->corona();
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

        connect(QApplication::desktop(), SIGNAL(resized(int)), SLOT(adjustSize()));
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

void RootWidget::adjustSize()
{
    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->geometry());
    foreach (DesktopView *view, m_desktops) {
        view->setGeometry(desktop->screenGeometry(view->screen()));
    }
}

#include "rootwidget.moc"

