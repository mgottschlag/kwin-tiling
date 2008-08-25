/*
 *   Copyright 2006-2008 Aaron Seigo <aseigo@kde.org>
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
    class Containment;
    class Corona;
    class View;
} // namespace Plasma

class MidView;

class PlasmaApp : public KUniqueApplication
{
    Q_OBJECT
public:
    ~PlasmaApp();

    static PlasmaApp* self();
    static bool hasComposite();

    void notifyStartup(bool completed);
    Plasma::Corona* corona();

    /**
     * Sets the view to be a desktop window if @p isDesktop is true
     * or an ordinary window otherwise.
     *
     * Desktop windows are displayed beneath all other windows, have
     * no window decoration and occupy the full size of the screen.
     *
     * The default behaviour is not to regsiter the view as the desktop
     * window.
     */
    void setIsDesktop(bool isDesktop);

    /**
     * Returns true if this widget is currently a desktop window.
     * See setIsDesktop()
     */
    bool isDesktop() const;

private:
    PlasmaApp(Display* display, Qt::HANDLE visual, Qt::HANDLE colormap);
    static void crashHandler(int signal);
    void reserveStruts();

private Q_SLOTS:
    void setCrashHandler();
    void cleanup();
    void syncConfig();
    void createView(Plasma::Containment *containment);
    void adjustSize(int screen);

private:
    Plasma::Corona *m_corona;
    QWidget *m_window;
    Plasma::View *m_controlBar;
    MidView *m_mainView;
    bool m_isDesktop;
};

#endif // multiple inclusion guard

