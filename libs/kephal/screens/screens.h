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


#ifndef KEPHAL_SCREENS_H
#define KEPHAL_SCREENS_H

#include <QPoint>
#include <QSize>
#include <QObject>


namespace kephal {

    class Screen;

    
    class Screens : public QObject {
        Q_OBJECT
        public:
            static Screens * instance();
            
            Screens(QObject * parent = 0);
            virtual QList<Screen *> screens() = 0;
            
        Q_SIGNALS:
            void screenAdded(Screen * s);
            void screenRemoved(Screen * s);
            void screenResized(Screen * s, QSize oldSize, QSize newSize);
            void screenMoved(Screen * s, QPoint oldPosition, QPoint newPosition);
            
        protected:
            static Screens * _instance;
    };
    
}


#endif // KEPHAL_SCREENS_H

