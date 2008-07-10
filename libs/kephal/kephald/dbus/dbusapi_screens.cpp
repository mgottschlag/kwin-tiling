/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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


#include "dbusapi_screens.h"
#include "../../screens/screens.h"
#include "../../screens/screen.h"
#include "screensadaptor.h"

#include <QDebug>



using namespace kephal;

DBusAPIScreens::DBusAPIScreens(QObject * parent)
        : QObject(parent)
{
    new ScreensAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    
    bool result;
    result = dbus.registerObject("/Screens", this);
    qDebug() << "registered on the bus:" << result;
}

QSize DBusAPIScreens::size(int screen)
{
    QList<Screen *> screens = Screens::instance()->screens();
    if (screen < screens.size()) {
        return screens.at(screen)->size();
    }
    return QSize(0,0);
}

QPoint DBusAPIScreens::position(int screen)
{
    QList<Screen *> screens = Screens::instance()->screens();
    if (screen < screens.size()) {
        return screens.at(screen)->position();
    }
    return QPoint(0,0);
}

int DBusAPIScreens::numScreens()
{
    QList<Screen *> screens = Screens::instance()->screens();
    return screens.size();
}

int DBusAPIScreens::primaryScreen()
{
    QList<Screen *> screens = Screens::instance()->screens();
    for (int i = 0; i < screens.size(); ++i) {
        if (screens[i]->isPrimary()) {
            return i;
        }
    }
    return 0;
}

