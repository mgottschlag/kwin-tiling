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


#ifndef KEPHAL_SIMPLEOUTPUT_H
#define KEPHAL_SIMPLEOUTPUT_H

#include "outputs.h"

#include <QSize>
#include <QPoint>
#include <QObject>


namespace kephal {

    class SimpleOutput : public Output {
        Q_OBJECT
        public:
            SimpleOutput(QObject * parent, QString id, QSize resolution, QPoint position, bool connected, bool activated);
            SimpleOutput(QObject * parent);
            
            virtual QString id();

            virtual QSize size();
            virtual void setSize(QSize size);
            virtual QPoint position();
            //QList<PositionType> getRelativePosition();
            
            virtual bool isConnected();
            virtual bool isActivated();

            void _setId(QString id);
            void _setSize(QSize size);
            void _setPosition(QPoint position);
            void _setActivated(bool activated);
            void _setConnected(bool connected);
            
        Q_SIGNALS:
            void sizeChangeRequested(SimpleOutput * screen, QSize oldSize, QSize newSize);
            
        private:
            QString m_id;
            QSize m_size;
            QPoint m_position;
            bool m_activated;
            bool m_connected;
    };
    
}


#endif // KEPHAL_SIMPLESCREEN_H

