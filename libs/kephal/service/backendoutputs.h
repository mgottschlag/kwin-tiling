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


#ifndef KEPHAL_BACKENDOUTPUTS_H
#define KEPHAL_BACKENDOUTPUTS_H

#include "outputs.h"


namespace Kephal {

    class BackendOutput : public Output {
        Q_OBJECT
        public:
            BackendOutput(QObject * parent);

        public Q_SLOTS:
            virtual bool applyGeom(const QRect & geom, float rate) = 0;
            virtual bool applyOrientation(Rotation rotation, bool reflectX, bool reflectY) = 0;
            virtual void deactivate() = 0;

            virtual void revert();
            virtual void mark();

        private:
            bool m_markedActive;
            QRect m_markedGeom;
            double m_markedRate;
            Rotation m_markedRotation;
            bool m_markedReflectX;
            bool m_markedReflectY;
    };

    class BackendOutputs : public Outputs {
        Q_OBJECT
        public:
            static BackendOutputs * self();

            BackendOutputs(QObject * parent);
            virtual ~BackendOutputs();

            /**
             * Deactivates any Outputs not in the layout and applies the geometry in the layout to
             * those that are in
             */
            virtual bool activateLayout(const QMap<Output *, QRect> & layout);
            virtual QList<BackendOutput *> backendOutputs();
            virtual BackendOutput * backendOutput(const QString & id);

        private:
            static BackendOutputs * s_instance;
    };

}

#endif // KEPHAL_BACKENDOUTPUTS_H

