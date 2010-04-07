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


#ifndef KEPHAL_DESKTOPWIDGETOUTPUTS_H
#define KEPHAL_DESKTOPWIDGETOUTPUTS_H

#include "simpleoutput.h"

#include "kephal_export.h"

namespace Kephal {

    /**
     * Fallback Outputs for when there is no XRandR
     * Although QDesktopWidget does not report outputs, this class
     * fakes them using the screens that QDesktopWidget reports
     */
    class KEPHAL_EXPORT DesktopWidgetOutputs : public Outputs {
        Q_OBJECT
        public:
            DesktopWidgetOutputs(QObject * parent);
            ~DesktopWidgetOutputs();

            QList<Output *> outputs();
            void activateLayout(const QMap<Output *, QRect> & layout);

        private Q_SLOTS:
            void screenChanged(int screen);

        private:
            QList<SimpleOutput *> m_outputs;
    };

}


#endif // KEPHAL_DESKTOPWIDGETOUTPUTS_H

