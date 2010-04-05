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


#ifndef KEPHAL_DBUSOUTPUTS_H
#define KEPHAL_DBUSOUTPUTS_H

#include "outputs.h"
#include "outputs_interface.h"

namespace Kephal {

class SimpleOutput;
    /**
     * Client side interface to outputs.  Setup by libkephal.cpp
     * Deals in SimpleOutput
     */
    class DBusOutputs : public Outputs {
        Q_OBJECT
        public:
            DBusOutputs(QObject * parent);

            QList<Output *> outputs();
            void activateLayout(const QMap<Output *, QRect> & layout);

            bool isValid();

        private Q_SLOTS:
            void outputConnectedSlot(QString id);
            void outputDisconnectedSlot(QString id);
            void outputActivatedSlot(QString id);
            void outputDeactivatedSlot(QString id);
            void outputResizedSlot(QString id);
            void outputMovedSlot(QString id);
            void outputRotatedSlot(QString id);
            void outputRateChangedSlot(QString id);
            void outputReflectedSlot(QString id);

        private:
            QList<SimpleOutput *> m_outputs;
            org::kde::Kephal::Outputs * m_interface;
            bool m_valid;
    };

}


#endif // KEPHAL_DBUSSCREENS_H

