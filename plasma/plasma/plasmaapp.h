/*
 *   Copyright 2006, 2007 Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_APP_H
#define PLASMA_APP_H

#include <QList>

#include <KUniqueApplication>

namespace Plasma
{
    class AppletBrowser;
    class Containment;
    class Corona;
} // namespace Plasma

class RootWidget;
class PanelView;

class PlasmaApp : public KUniqueApplication
{
    Q_OBJECT
public:
    ~PlasmaApp();

    static PlasmaApp* self();
    static bool hasComposite();

    void notifyStartup(bool completed);
    Plasma::Corona* corona();
    void showAppletBrowser(Plasma::Containment *containment);

public Q_SLOTS:
    // DBUS interface. if you change these methods, you MUST run:
    // qdbuscpp2xml plasmaapp.h -o org.kde.plasma.App.xml
    void initializeWallpaper();
    void toggleDashboard();

private Q_SLOTS:
    void setCrashHandler();
    void cleanup();
    void appletBrowserDestroyed();
    void createView(Plasma::Containment *containment);
    void panelRemoved(QObject* panel);

private:
    PlasmaApp(Display* display, Qt::HANDLE visual, Qt::HANDLE colormap);
    static void crashHandler(int signal);

    RootWidget *m_root;
    Plasma::Corona *m_corona;
    QList<PanelView*> m_panels;
    Plasma::AppletBrowser *m_appletBrowser;
};

#endif // multiple inclusion guard
