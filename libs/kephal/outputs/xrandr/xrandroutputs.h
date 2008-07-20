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


#ifndef KEPHAL_XRANDROUTPUTS_H
#define KEPHAL_XRANDROUTPUTS_H

#include "../outputs.h"
#include "xrandr12/randrdisplay.h"


namespace kephal {

    class XRandROutputs;

    class XRandROutput : public Output {
        Q_OBJECT
        public:
            XRandROutput(XRandROutputs * parent, RROutput rrId);
            
            virtual QString id();

            virtual QSize size();
            virtual void setSize(QSize size);
            virtual QPoint position();
            virtual bool isConnected();
            virtual bool isActivated();
            //QList<PositionType> getRelativePosition();

        private:
            RandROutput * output();
            
            XRandROutputs * m_outputs;
            RROutput m_rrId;
    };
    

    class XRandROutputs : public Outputs {
        Q_OBJECT
        public:
            XRandROutputs(QObject * parent, RandRDisplay * display);
            virtual QList<Output *> outputs();
            RandROutput * output(RROutput rrId);
            
        private:
            void init();
            
            RandRDisplay * m_display;
            QList<XRandROutput *> m_outputs;
    };
    
}


#endif // KEPHAL_XRANDROUTPUTS_H

