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


#ifndef KEPHAL_OUTPUTS_H
#define KEPHAL_OUTPUTS_H

#include <QObject>
#include <QSize>
#include <QPoint>
#include <QRect>


namespace kephal {

    class Output : public QObject {
        Q_OBJECT
        public:
            Output(QObject * parent);

            virtual QString id() = 0;

            virtual QSize size() = 0;
            virtual void setSize(QSize size) = 0;
            virtual QPoint position() = 0;
            virtual bool isConnected() = 0;
            virtual bool isActivated() = 0;
            //QList<PositionType> getRelativePosition();

            QRect geom();
    };
    

    class Outputs : public QObject {
        Q_OBJECT
        public:
            static Outputs * instance();
            
            Outputs(QObject * parent);
            virtual QList<Output *> outputs() = 0;
            
        Q_SIGNALS:
            void outputConnected(Output * o);
            void outputDisconnected(Output * o);
            void outputActivated(Output * o);
            void outputDeactivated(Output * o);
            void outputResized(Output * o, QSize oldSize, QSize newSize);
            void outputMoved(Output * o, QPoint oldPosition, QPoint newPosition);
            
        protected:
            static Outputs * m_instance;
    };
    
}


#endif // KEPHAL_OUTPUTS_H

