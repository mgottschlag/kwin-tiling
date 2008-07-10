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


#ifndef KEPHAL_SCREEN_H
#define KEPHAL_SCREEN_H

#include <QPoint>
#include <QSize>
#include <QObject>
#include <QRect>


namespace kephal {

    class Screen : public QObject {
        Q_OBJECT
        public:
            virtual int id() = 0;

            virtual QSize size() = 0;
            virtual void setSize(QSize size) = 0;
            virtual QPoint position() = 0;
            //QList<PositionType> getRelativePosition();

            virtual bool isPrivacyMode() = 0;
            virtual void setPrivacyMode(bool b) = 0;
            virtual bool isPrimary() = 0;
            virtual void setAsPrimary() = 0;
            
            QRect geom();
    };
    
}


#endif // KEPHAL_SCREEN_H

