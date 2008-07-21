#include "dbusscreens.h"
#include "../simplescreen.h"
#include "screens_interface.h"
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


#include <QApplication>
#include <QDebug>


namespace kephal {

    DBusScreens::DBusScreens(QObject * parent)
            : Screens(parent)
    {
        m_interface = new org::kde::Kephal::Screens(
            "org.kde.Kephal",
            "/Screens",
            QDBusConnection::sessionBus(),
            this);
            
        if (! m_interface->isValid()) {
            m_valid = false;
            return;
        }
            
        m_valid = true;
        
        int numScreens = m_interface->numScreens();
        int primary = m_interface->primaryScreen();
        for (int i = 0; i < numScreens; ++i) {
            QPoint pos = m_interface->position(i);
            QSize size = m_interface->size(i);
            qDebug() << "adding a screen" << i << "with geom: " << pos << size;
            
            SimpleScreen * screen = new SimpleScreen(this,
                    i,
                    size,
                    pos,
                    false,
                    i == primary);
            m_screens.append(screen);
        }
        m_primaryScreen = m_screens.at(primary);
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

