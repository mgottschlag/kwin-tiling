/*
 *   Copyright 2006-2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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


#include <KUniqueApplication>

#include <plasma/plasma.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

namespace Plasma
{
    class Containment;
    class Corona;
    class Dialog;
    class View;
    class WidgetExplorer;
} // namespace Plasma

class NetView;
class NetCorona;
class QTimer;
class GlowBar;
class ShadowWindow;

namespace Kephal
{
    class Screen;
}

class PlasmaApp : public KUniqueApplication
{
    Q_OBJECT
public:
    ~PlasmaApp();

    static PlasmaApp* self();
    static bool hasComposite();

    void suspendStartup(bool suspend);
    Plasma::Corona* corona();

    /**
     * Sets the view to be a desktop window if @p isDesktop is true
     * or an ordinary window otherwise.
     *
     * Desktop windows are displayed beneath all other windows, have
     * no window decoration and occupy the full size of the screen.
     *
     * The default behaviour is not to register the view as the desktop
     * window.
     */
    void setIsDesktop(bool isDesktop);

    NetView *controlBar() const;

    NetView *mainView() const;

    QWidget *widgetExplorer() const;

    /**
     * Returns true if this widget is currently a desktop window.
     * See setIsDesktop()
     */
    bool isDesktop() const;

    void showWidgetExplorer(Plasma::Containment *containment);

public Q_SLOTS:
    void setAutoHideControlBar(bool autoHide);

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    bool x11EventFilter(XEvent *event);
    //true if the application has an active window
    bool hasForegroundWindows() const;
    //more strict than hasForegroundWindows, the mainview has to be not only active, but also on top of other windows
    bool mainViewOnTop() const;

private:
    PlasmaApp();
    void reserveStruts();
    void createUnhideTrigger();
    void destroyUnHideTrigger();

private Q_SLOTS:
    void cleanup();
    void syncConfig();
    void positionPanel();
    void createView(Plasma::Containment *containment);
    void adjustSize(Kephal::Screen *);
    void controlBarMoved(const NetView *controlBar);
    void showWidgetExplorer();
    void widgetExplorerDestroyed();
    void closeWidgetExplorer();
    void mainContainmentActivated();
    //TODO: probably those functions can go in netview
    void controlBarVisibilityUpdate();
    void showControlBar();
    void hideControlBar();
    void setControlBarVisible(bool visible);
    void toggleControlBarVisibility();
    void lowerMainView();
    void configureContainment(Plasma::Containment *containment);
    void updateToolBoxVisibility(bool visible);
    void unhideHintMousePoll();
    void wallpaperCheckedIn();
    void wallpaperCheckInTimeout();
    void checkShadow();

Q_SIGNALS:
    void controlBarChanged();

private:
    NetCorona *m_corona;
    Plasma::Dialog *m_widgetExplorerView;
    Plasma::WidgetExplorer *m_widgetExplorer;
#ifdef Q_WS_X11
    Window m_unhideTrigger;
    QRect m_triggerZone;
    QRect m_unhideTriggerGeom;
#endif
    GlowBar *m_glowBar;
    QTimer *m_mousePollTimer;
    NetView *m_controlBar;
    NetView *m_mainView;
    bool m_isDesktop;
    bool m_autoHideControlBar;
    QTimer *m_unHideTimer;
    ShadowWindow *m_shadowWindow;
    int m_startupSuspendWaitCount;
};

#endif // multiple inclusion guard

