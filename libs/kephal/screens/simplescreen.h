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

#include "screen.h"

#include <QSize>
#include <QPoint>
#include <QObject>


namespace kephal {

    class SimpleScreen : public Screen {
        Q_OBJECT
        public:
            SimpleScreen(int id, QSize resolution, QPoint position, bool privacy, bool primary);
            
            virtual int id();

            virtual QSize size();
            virtual void setSize(QSize size);
            virtual QPoint position();
            //QList<PositionType> getRelativePosition();

            virtual bool isPrivacyMode();
            virtual void setPrivacyMode(bool b);
            virtual bool isPrimary();
            virtual void setAsPrimary();
            
            void _setSize(QSize size);
            void _setPosition(QPoint position);
            void _setPrimary(bool primary);
            
        Q_SIGNALS:
            void selectedAsPrimary(SimpleScreen * screen);
            void privacyModeChanged(SimpleScreen * screen, bool privacy);
            void sizeChanged(SimpleScreen * screen, QSize oldSize, QSize newSize);
            
        private:
            int _id;
            QSize _size;
            QPoint _position;
            bool _privacy;
            bool _primary;
    };
    
}


#endif // KEPHAL_SIMPLESCREEN_H

