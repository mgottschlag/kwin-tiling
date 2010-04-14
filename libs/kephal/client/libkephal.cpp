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

#include <QApplication>
#include <QDebug>
#include <QDBusInterface>

//#include "dbusscreens.h"
//#include "dbusoutputs.h"
//#include "dbusconfigurations.h"
//include "noconfigurations.h"
//#include "outputscreens.h"
//include "desktopwidgetoutputs.h"
#include "desktopwidgetscreens.h"

using namespace Kephal;


/**
 * This factory-method is invoked the first time any of Screens::self(),
 * Outputs::self() or Configurations::self() is called.
 * It will create the singleton-instances and check for their validity.
 * If the dbus-based implementations are not valid a fallback to
 * QDesktopWidget will be used instead.
 *
 * The method is hooked into the ...::self()-methods with these lines
 * in the cmake-file:
 * ADD_DEFINITIONS (-DSCREENS_FACTORY=libkephal_factory)
 * ADD_DEFINITIONS (-DOUTPUTS_FACTORY=libkephal_factory)
 * ADD_DEFINITIONS (-DCONFIGURATIONS_FACTORY=libkephal_factory)
 */
void libkephal_factory() {
    //qDebug() << "trying to access kephald...";
    //QDBusInterface interface("org.kde.kded", "/modules/kephal");

//#DBusConfigurations * configurations = new DBusConfigurations(qApp);
    //DBusOutputs * outputs = new DBusOutputs(qApp);
    DesktopWidgetScreens  * screens = new DesktopWidgetScreens(qApp);
    if (false/*(! screens->isValid()) || (! outputs->isValid()) || (! configurations->isValid())*/) {
        qWarning() << "could not access kephald, falling back to QDesktopWidget";

        delete screens;
        //delete outputs;
        //delete configurations;

        //new NoConfigurations(qApp);
        //new DesktopWidgetOutputs(qApp);
        //new OutputScreens(qApp);
    }
}

