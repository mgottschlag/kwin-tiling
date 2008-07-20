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


#include "outputscreens.h"


namespace kephal {

    OutputScreens::OutputScreens(QObject * parent)
        : Screens(parent)
    {
        init();
    }
    
    QList<Screen *> OutputScreens::screens() {
        QList<Screen *> result;
        foreach(OutputScreen * screen, m_screens) {
            result.append(screen);
        }
        return result;
    }
    
    void OutputScreens::init() {
        foreach (Output * output, Outputs::instance()->outputs()) {
            if (! output->isConnected() || ! output->isActivated()) {
                continue;
            }
            
            bool found = false;
            foreach (OutputScreen * screen, m_screens) {
                if (screen->geom().intersects(output->geom())) {
                    screen->add(output);
                    found = true;
                    break;
                }
            }
            if (! found) {
                OutputScreen * screen = new OutputScreen(this);
                screen->add(output);
                m_screens.append(screen);
            }
        }
        
        bool changed = false;
        do {
            for (int i = 0; i + 1 < m_screens.size(); ++i) {
                if (m_screens[i]->geom().intersects(m_screens[i + 1]->geom())) {
                    OutputScreen * to = m_screens[i];
                    OutputScreen * from = m_screens.takeAt(i + 1);
                    
                    foreach (Output * output, from->outputs()) {
                        to->add(output);
                    }
                    
                    changed = true;
                    break;
                }
            }
        } while (changed);
        
        m_screens[0]->_setPrimary(true);
        for (int i = 0; i < m_screens.size(); ++i) {
            m_screens[i]->_setId(i);
        }
    }
    
    
    
    OutputScreen::OutputScreen(QObject * parent)
        : SimpleScreen(parent)
    {
    }
    
    void OutputScreen::add(Output * output) {
        m_outputs.append(output);
        
        QRect geom = this->geom().unite(output->geom());
        _setSize(geom.size());
        _setPosition(geom.topLeft());
    }
    
    QList<Output *> OutputScreen::outputs() {
        return m_outputs;
    }
    
}

