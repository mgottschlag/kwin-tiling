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


#include "configurationscreens.h"

#include "configurations.h"
#include "backendconfigurations.h"

#include "KDebug"

namespace Kephal {

    ConfigurationScreens::ConfigurationScreens(QObject * parent)
        : OutputScreens(parent)
    {
        connect(Configurations::self(), SIGNAL(configurationActivated(Kephal::Configuration*)), this, SLOT(configurationActivated(Kephal::Configuration*)));
    }

    void ConfigurationScreens::configurationActivated(Configuration * configuration) {
        Q_UNUSED(configuration)
        kDebug();
        triggerRebuildScreens();
    }

    void ConfigurationScreens::prepareScreens(QMap<int, OutputScreen *> & screens) {
        BackendConfiguration * config = BackendConfigurations::self()->activeBackendConfiguration();
        if (! config) {
            return;
        }

        if (config->name() == "external") {
            screens.clear();
            return;
        }

        QMap<int, QRect> layout = config->realLayout();
        for (QMap<int, QRect>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            if (screens.contains(i.key())) {
                screens[i.key()]->_setGeom(i.value());
            } else {
                OutputScreen * screen = new OutputScreen(this);
                screen->_setId(i.key());
                screen->_setGeom(i.value());
                screens.insert(screen->id(), screen);
            }
        }

        for (QMap<int, OutputScreen *>::iterator i = screens.begin(); i != screens.end();) {
            if (! layout.contains(i.key())) {
                i = screens.erase(i);
            } else {
                ++i;
            }
        }
    }

}

