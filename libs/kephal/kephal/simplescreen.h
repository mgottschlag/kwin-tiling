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


#ifndef KEPHAL_SIMPLESCREEN_H
#define KEPHAL_SIMPLESCREEN_H

#include "screens.h"

namespace Kephal {

    class SimpleScreen : public Screen {
        Q_OBJECT
        public:
            SimpleScreen(int id, const QSize & resolution, const QPoint & position, bool privacy, QObject * parent = 0);
            SimpleScreen(QObject * parent = 0);
            virtual ~SimpleScreen();
            virtual int id();

            virtual QSize size();
            virtual QPoint position();

            virtual bool isPrivacyMode();
            virtual void setPrivacyMode(bool b);

            QList<Output *> outputs();

            void _setId(int id);
            void _setSize(const QSize & size);
            void _setPosition(const QPoint & position);
            void _setGeom(const QRect & geom);
            QList<Output *> & _outputs();

        Q_SIGNALS:
            void privacyModeChangeRequested(SimpleScreen * screen, bool privacy);

        private:
            int m_id;
            QSize m_size;
            QPoint m_position;
            bool m_privacy;
            QList<Output *> m_outputs;
    };

}


#endif // KEPHAL_SIMPLESCREEN_H

