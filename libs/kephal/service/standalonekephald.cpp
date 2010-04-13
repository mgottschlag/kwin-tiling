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

#if 0
#include <QDebug>
#include <QDBusConnection>
#include <QAbstractEventDispatcher>
#include <QThread>


#include "outputs/desktopwidget/desktopwidgetoutputs.h"
#include "screens/configuration/configurationscreens.h"
#ifdef Q_WS_X11
#include "outputs/xrandr/xrandroutputs.h"
#endif
#include "dbus/dbusapi_screens.h"
#include "dbus/dbusapi_outputs.h"
#include "dbus/dbusapi_configurations.h"
#include "configurations/xml/xmlconfigurations.h"
#ifdef Q_WS_X11
#include "xrandr12/randrdisplay.h"
#include "xrandr12/randrscreen.h"
#endif
#endif

#include "kephalservice.h"



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    new KephalService(&app);
    return app.exec();
}

