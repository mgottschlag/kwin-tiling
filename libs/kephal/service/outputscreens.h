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


#ifndef KEPHAL_OUTPUTSCREENS_H
#define KEPHAL_OUTPUTSCREENS_H

#include "screens.h"
#include "simplescreen.h"
#include "outputs.h"


class QTimer;
namespace Kephal {

    /**
     * Maps outputs to a single Screen
     */
    class OutputScreen : public SimpleScreen {
        Q_OBJECT
        public:
            OutputScreen(QObject * parent);

            void add(Output * output);
            void remove(Output * output);
            void clearOutputs();
            QList<Output *> outputs();

        private:
            QList<Output *> m_outputs;
    };


    /**
     * Screens object containing output signalling and mapping between Outputs and Screens
     */
    class OutputScreens : public Screens {
        Q_OBJECT
        public:
            OutputScreens(QObject * parent);
            virtual QList<Screen *> screens();

        protected:
            //cannot be pure virtual because libkephal.cpp instantates an OutputScreens
            virtual void prepareScreens(QMap<int, OutputScreen *> & screens);
            void rebuildScreens();
            void triggerRebuildScreens(); // Triggers a rebuild aftera short delay; requests are coalesced.

        private Q_SLOTS:
            void outputActivated(Kephal::Output * o);
            void outputDeactivated(Kephal::Output * o);
            void outputResized(Kephal::Output * o, QSize oldSize, QSize newSize);
            void outputMoved(Kephal::Output * o, QPoint oldPosition, QPoint newPosition);
            void rebuildTimeout();

        private:
            void init();
            void buildScreens();
            int findId();

            QMap<int, OutputScreen *> m_screens;

            QTimer * m_rebuildTimer;
    };

}


#endif // KEPHAL_OUTPUTSCREENS_H

