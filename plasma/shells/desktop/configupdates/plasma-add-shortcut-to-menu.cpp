/*
 *   Copyright 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 
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

#include <KComponentData>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>

int main()
{
    KComponentData cd("plasma-add-shortcut-to-menu-update");
    QString file = KStandardDirs::locateLocal("config", "plasma-appletsrc");

    if (file.isEmpty()) {
        return 0;
    }

    KConfig config(file);
    KConfigGroup containments(&config, "Containments");
    foreach (const QString &group, containments.groupList()) {
        KConfigGroup applets(&containments, group);
        applets = KConfigGroup(&applets, "Applets");
        foreach (const QString &appletGroup, applets.groupList()) {
            KConfigGroup applet(&applets, appletGroup);
            QString plugin = applet.readEntry("plugin", QString());
            if (plugin == "launcher" || plugin == "simplelauncher") {
                KConfigGroup shortcuts(&applet, "Shortcuts");

                if (!shortcuts.hasKey("global")) {
                    shortcuts.writeEntry("global", "Alt+F1");
                }

                return 0;
            }
        }
    }

    return 0;
}

