/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef PLASMA_APP_H
#define PLASMA_APP_H

#include <KUniqueApplication>

class RootWidget;

class PlasmaApp : public KUniqueApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.plasma.App")

public:
    PlasmaApp();
    ~PlasmaApp();

    static PlasmaApp* self();
    void notifyStartup(bool completed);

public Q_SLOTS:
    // DBUS interface. if you change these methods, you MUST run:
    // qdbuscpp2xml plasmaapp.h -o org.kde.plasma.App.xml
    void initializeWallpaper();

private slots:
    void setCrashHandler();

private:
    static void crashHandler(int signal);

    RootWidget *m_root;
};

#endif // multiple inclusion guard
