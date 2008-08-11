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


#include "configurations.h"

#ifdef CONFIGURATIONS_FACTORY
void CONFIGURATIONS_FACTORY();
#endif


namespace kephal {

    Configurations * Configurations::instance() {
#ifdef CONFIGURATIONS_FACTORY
        if (Configurations::m_instance == 0) {
            CONFIGURATIONS_FACTORY();
        }
#endif
        return Configurations::m_instance;
    }
    
    Configurations::Configurations(QObject * parent)
            : QObject(parent)
    {
        Configurations::m_instance = this;
    }
    
    Configurations * Configurations::m_instance = 0;
    
    
    
    Configuration::Configuration(QObject * parent)
            : QObject(parent)
    {
    }

}

