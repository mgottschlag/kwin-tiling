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


#ifndef DBUSAPI_SCREENS_H
#define DBUSAPI_SCREENS_H


#include <QObject>
#include <QPoint>
#include <QSize>
#include <QStringList>

namespace Kephal {
    class Screen;
}

class DBusAPIScreens : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Kephal.Screens")

    public:
        DBusAPIScreens(QObject * parent);

    public Q_SLOTS:
        int numScreens();
        int id(int index);
        QSize size(int id);
        QPoint position(int id);
        int primaryScreen();
        QStringList outputs(int id);

    private Q_SLOTS:
        void screenResized(Kephal::Screen * s, QSize oldSize, QSize newSize);
        void screenMoved(Kephal::Screen * s, QPoint oldPosition, QPoint newPosition);
        void screenAdded(Kephal::Screen * s);
        void screenRemovedSlot(int id);

    Q_SIGNALS:
        void screenResized(int screen);
        void screenMoved(int screen);
        void screenAdded(int screen);
        void screenRemoved(int screen);
};


#endif // DBUSAPI_SCREENS_H

