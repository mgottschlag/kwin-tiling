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
#include "screens.h"
#include "outputs.h"
#include "screensadaptor.h"

#include <KDebug>



using namespace Kephal;

DBusAPIScreens::DBusAPIScreens(QObject * parent)
        : QObject(parent)
{
    new ScreensAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();

    const bool result = dbus.registerObject("/modules/kephal/Screens", this);
    kDebug() << "screens registered on the bus:" << result;

    connect(Screens::self(), SIGNAL(screenResized(Kephal::Screen*,QSize,QSize)), this, SLOT(screenResized(Kephal::Screen*,QSize,QSize)));
    connect(Screens::self(), SIGNAL(screenMoved(Kephal::Screen*,QPoint,QPoint)), this, SLOT(screenMoved(Kephal::Screen*,QPoint,QPoint)));
    connect(Screens::self(), SIGNAL(screenAdded(Kephal::Screen*)), this, SLOT(screenAdded(Kephal::Screen*)));
    connect(Screens::self(), SIGNAL(screenRemoved(int)), this, SLOT(screenRemovedSlot(int)));
}

void DBusAPIScreens::screenResized(Kephal::Screen * s, QSize oldSize, QSize newSize) {
    Q_UNUSED(oldSize)
    Q_UNUSED(newSize)
    emit screenResized(s->id());
}

void DBusAPIScreens::screenMoved(Kephal::Screen * s, QPoint oldPosition, QPoint newPosition) {
    Q_UNUSED(oldPosition)
    Q_UNUSED(newPosition)
    emit screenMoved(s->id());
}

void DBusAPIScreens::screenAdded(Kephal::Screen * s) {
    emit screenAdded(s->id());
}

void DBusAPIScreens::screenRemovedSlot(int id) {
    emit screenRemoved(id);
}

QSize DBusAPIScreens::size(int id)
{
    Screen * s = Screens::self()->screen(id);
    return s ? s->size() : QSize(0,0);
}

QPoint DBusAPIScreens::position(int id)
{
    Screen * s = Screens::self()->screen(id);
    return s ? s->position() : QPoint(0,0);
}

int DBusAPIScreens::numScreens()
{
    QList<Screen *> screens = Screens::self()->screens();
    return screens.size();
}

int DBusAPIScreens::id(int index)
{
    QList<Screen *> screens = Screens::self()->screens();
    if (index < screens.size()) {
        return screens[index]->id();
    }
    return -1;
}

int DBusAPIScreens::primaryScreen()
{
    Screen * s = Screens::self()->primaryScreen();
    return s ? s->id() : 0;
}

QStringList DBusAPIScreens::outputs(int id) {
    Screen * s = Screens::self()->screen(id);
    QStringList result;
    if (s) {
        foreach (Output * output, s->outputs()) {
            result << output->id();
        }
    }
    return result;
}

#ifndef NO_KDE
#include "dbusapi_screens.moc"
#endif

