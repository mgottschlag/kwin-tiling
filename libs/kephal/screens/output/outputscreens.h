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


#ifndef KEPHAL_XRANDRSCREENS_H
#define KEPHAL_XRANDRSCREENS_H

#include <QPoint>
#include "../screens.h"
#include "../simplescreen.h"
#include "outputs/outputs.h"


namespace kephal {

    class OutputScreen : public SimpleScreen {
        Q_OBJECT
        public:
            OutputScreen(QObject * parent);
            
            void add(Output * output);
            QList<Output *> outputs();
            
        private:
            QList<Output *> m_outputs;
    };
    

    class OutputScreens : public Screens {
        Q_OBJECT
        public:
            OutputScreens(QObject * parent);
            virtual QList<Screen *> screens();
            
        private:
            void init();
            
            QList<OutputScreen *> m_screens;
    };
    
}


#endif // KEPHAL_XRANDRSCREENS_H

