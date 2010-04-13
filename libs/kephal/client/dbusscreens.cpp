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


#include "dbusscreens.h"

#include <QDebug>

#include "simplescreen.h"

#include "outputs.h"

#include "screens_interface.h"

namespace Kephal {

    DBusScreens::DBusScreens(QObject * parent)
            : Screens(parent)
    {
        m_interface = new org::kde::Kephal::Screens(
            "org.kde.Kephal",
            "/modules/kephal/Screens",
            QDBusConnection::sessionBus(),
            this);

        if (! m_interface->isValid()) {
            m_valid = false;
            return;
        }

        m_valid = true;

        int numScreens = m_interface->numScreens();
        for (int i = 0; i < numScreens; ++i) {
            int id = m_interface->id(i);
            QPoint pos = m_interface->position(id);
            QSize size = m_interface->size(id);
            //qDebug() << "adding a screen" << id << "with geom: " << pos << size;

            SimpleScreen * screen = new SimpleScreen( id, size, pos, false, this);
            m_screens.append(screen);

            const QStringList outputIds = m_interface->outputs(id);
            foreach (const QString& outputId, outputIds) {
                Output * output = Outputs::self()->output(outputId);
                if (output) {
                    screen->_outputs() << output;
                }
            }
        }

        connect(m_interface, SIGNAL(screenResized(int)), this, SLOT(screenResizedSlot(int)));
        connect(m_interface, SIGNAL(screenMoved(int)), this, SLOT(screenMovedSlot(int)));
        connect(m_interface, SIGNAL(screenAdded(int)), this, SLOT(screenAddedSlot(int)));
        connect(m_interface, SIGNAL(screenRemoved(int)), this, SLOT(screenRemovedSlot(int)));
    }

    void DBusScreens::screenResizedSlot(int id) {
        SimpleScreen * s = (SimpleScreen *) screen(id);
        if (s) {
            QSize prev = s->size();
            s->_setSize(m_interface->size(id));
            emit screenResized(s, prev, s->size());
        }
    }

    void DBusScreens::screenMovedSlot(int id) {
        SimpleScreen * s = (SimpleScreen *) screen(id);
        if (s) {
            QPoint prev = s->position();
            s->_setPosition(m_interface->position(id));
            emit screenMoved(s, prev, s->position());
        }
    }

    void DBusScreens::screenAddedSlot(int id) {
        QPoint pos = m_interface->position(id);
        QSize size = m_interface->size(id);
        //qDebug() << "adding a screen" << id << "with geom: " << pos << size;

        SimpleScreen * screen = new SimpleScreen(id, size, pos, false, this);
        m_screens.append(screen);

        const QStringList outputIds = m_interface->outputs(id);
        foreach (const QString& outputId, outputIds) {
            Output * output = Outputs::self()->output(outputId);
            if (output) {
                screen->_outputs() << output;
            }
        }

        emit screenAdded(screen);
    }

    void DBusScreens::screenRemovedSlot(int id) {
        SimpleScreen * s = (SimpleScreen *) screen(id);
        if (s) {
            m_screens.removeAll(s);
            delete s;
            emit screenRemoved(id);
        }
    }


    DBusScreens::~DBusScreens() {
        foreach(Screen * screen, m_screens) {
            delete screen;
        }
        m_screens.clear();
    }

    QList<Screen *> DBusScreens::screens()
    {
        QList<Screen *> result;
        foreach(SimpleScreen * screen, m_screens) {
            result.append(screen);
        }
        return result;
    }

    bool DBusScreens::isValid() {
        return m_valid;
    }

}

