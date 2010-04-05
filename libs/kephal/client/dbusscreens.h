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


#ifndef KEPHAL_DBUSSCREENS_H
#define KEPHAL_DBUSSCREENS_H

#include "screens.h"

class OrgKdeKephalScreensInterface;

namespace Kephal {

    class SimpleScreen;
    /**
     * Client stub to Screens as provided by the Kephal service
     */
    class DBusScreens : public Screens {
        Q_OBJECT
        public:
            DBusScreens(QObject * parent);
            ~DBusScreens();
            virtual QList<Screen *> screens();
            bool isValid();

        private Q_SLOTS:
            void screenResizedSlot(int id);
            void screenMovedSlot(int id);
            void screenAddedSlot(int id);
            void screenRemovedSlot(int id);

        private:
            QList<SimpleScreen *> m_screens;
            OrgKdeKephalScreensInterface * m_interface;
            bool m_valid;
    };

}


#endif // KEPHAL_DBUSSCREENS_H

