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


#include <QDBusInterface>

#include "screens/dbus/dbusscreens.h"
#include "outputs/dbus/dbusoutputs.h"
#include "configurations/dbus/dbusconfigurations.h"
#include "configurations/noconfigurations.h"
#include "screens/output/outputscreens.h"
#include "outputs/desktopwidget/desktopwidgetoutputs.h"

using namespace Kephal;


void libkephal_factory() {
    //qDebug() << "trying to access kephald...";
    QDBusInterface interface("org.kde.kded", "/modules/kded_kephal");
    
    DBusConfigurations * configurations = new DBusConfigurations(qApp);
    DBusOutputs * outputs = new DBusOutputs(qApp);
    DBusScreens * screens = new DBusScreens(qApp);
    if ((! screens->isValid()) || (! outputs->isValid()) || (! configurations->isValid())) {
        qWarning() << "could not access kephald, falling back to QDesktopWidget";
        
        delete screens;
        delete outputs;
        delete configurations;
        
        new NoConfigurations(qApp);
        new DesktopWidgetOutputs(qApp);
        new OutputScreens(qApp);
    }
}

