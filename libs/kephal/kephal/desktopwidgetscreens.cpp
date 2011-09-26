/*
Copyright 2010 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "desktopwidgetscreens.h"

#include <QDesktopWidget>
#include <QApplication>

#include <KDebug>

#include "simplescreen.h"

Kephal::DesktopWidgetScreens::DesktopWidgetScreens(QObject * parent)
    : Screens(parent)
{
    QDesktopWidget * desktop = QApplication::desktop();
    connect(desktop, SIGNAL(screenCountChanged(int)), this, SLOT(handleScreenCountChanged(int)));
    connect(desktop, SIGNAL(resized(int)), this, SLOT(handleScreenResized(int)));

    for (int i = 0; i < desktop->numScreens(); i++) {
        QRect geometry = desktop->screenGeometry(i);
        Kephal::SimpleScreen * screen =
            new Kephal::SimpleScreen(i, geometry.size(), geometry.topLeft(), false, this);
        m_screens.append(screen);
    }
}

Kephal::DesktopWidgetScreens::~DesktopWidgetScreens()
{
    foreach (Kephal::SimpleScreen * screen, m_screens) {
        int id = screen->id();
        delete screen;
        emit screenRemoved(id);
    }
}

QList<Kephal::Screen*> Kephal::DesktopWidgetScreens::screens()
{
    QList<Kephal::Screen*> list;
    foreach (Kephal::SimpleScreen * screen, m_screens) {
        list.append(screen);
    }
    return list;
}

void Kephal::DesktopWidgetScreens::handleScreenResized(int id)
{
    Kephal::SimpleScreen * screen = m_screens.value(id);
    QSize oldSize = screen->size();
    QPoint oldPosition = screen->position();
    //FIXME: find a better way to restrict changing Screen to Screens - friend modifier class?
    screen->_setGeom(QApplication::desktop()->screenGeometry(id));
    if (oldSize != screen->size()) {
        emit screenResized(screen, oldSize, screen->size());
    }
    if (oldPosition != screen->position()) {
        emit screenMoved(screen, oldPosition, screen->position());
    }
}

void Kephal::DesktopWidgetScreens::handleScreenCountChanged(int newCount)
{
    // add new Screens
    for (int i = m_screens.count(); i < newCount; i++) {
        QRect geometry = QApplication::desktop()->screenGeometry(i);
        Kephal::SimpleScreen * screen =
            new Kephal::SimpleScreen(i, geometry.size(), geometry.topLeft(), false, this);
        m_screens.append(screen);
        emit screenAdded(screen);
    }

    // remove no longer existing Screens
    while (m_screens.count() > newCount) {
        Kephal::SimpleScreen * screen = m_screens.takeLast();
        if (screen) {
            int id = screen->id();
            delete screen;
            emit screenRemoved(id);
        }
    }
}

// vim: sw=4 sts=4 et tw=100
