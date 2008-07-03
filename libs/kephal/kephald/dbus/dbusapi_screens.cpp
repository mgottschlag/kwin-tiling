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

QSize DBusAPIScreens::getResolution(int screen)
{
    QList<Screen *> screens = Screens::instance()->getScreens();
    if (screen < screens.size()) {
        return screens.at(screen)->getResolution();
    }
    return QSize(0,0);
}

QPoint DBusAPIScreens::getPosition(int screen)
{
    QList<Screen *> screens = Screens::instance()->getScreens();
    if (screen < screens.size()) {
        return screens.at(screen)->getPosition();
    }
    return QPoint(0,0);
}

int DBusAPIScreens::numScreens()
{
    QList<Screen *> screens = Screens::instance()->getScreens();
    return screens.size();
}

int DBusAPIScreens::getPrimaryScreen()
{
    QList<Screen *> screens = Screens::instance()->getScreens();
    for (int i = 0; i < screens.size(); ++i) {
        if (screens[i]->isPrimary()) {
            return i;
        }
    }
    return 0;
}

