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


#ifndef KEPHAL_CONFIGURATIONSCREENS_H
#define KEPHAL_CONFIGURATIONSCREENS_H


#include "outputscreens.h"


namespace Kephal {

    class Configuration;

    /**
     * updates the geometries of Screen objects using active Configuration
     * Has a special case for the 'external' configuration. Why?
     */
    class ConfigurationScreens : public OutputScreens {
        Q_OBJECT
        public:
            ConfigurationScreens(QObject * parent);

        protected:
            void prepareScreens(QMap<int, OutputScreen *> & screens);

        private Q_SLOTS:
            void configurationActivated(Kephal::Configuration * configuration);
    };

}

#endif // KEPHAL_CONFIGURATIONSCREENS_H

