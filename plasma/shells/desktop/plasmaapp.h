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

#include <QHash>
#include <QList>
#include <QSize>
#include <QPoint>
#include <QPointer>

#include <KUniqueApplication>

#include <Plasma/Plasma>
#include <plasma/packagemetadata.h>

#include "ui_globaloptions.h"

class QSignalMapper;
class QTimer;

namespace Plasma
{
    class AccessAppletJob;
    class AppletBrowser;
    class Containment;
    class Corona;
    class Dialog;
} // namespace Plasma

namespace Kephal {
    class Screen;
} // namespace Kephal

class DesktopView;
class PanelView;
class DesktopCorona;
class InteractiveConsole;

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
     * Should be called when a panel hides or unhides itself
     */
    void panelHidden(bool hidden);

    /**
     * Current desktop zoom level
     */
     Plasma::ZoomLevel desktopZoomLevel() const;

    /**
     * Returns the PanelViews
     */
    QList<PanelView*> panelViews() const;

    static bool isPanelContainment(Plasma::Containment *containment);

#ifdef Q_WS_X11
    Atom m_XdndAwareAtom;
    Atom m_XdndEnterAtom;
    Atom m_XdndFinishedAtom;
    Atom m_XdndPositionAtom;
    Atom m_XdndStatusAtom;
    Atom m_XdndVersionAtom;
#endif

public Q_SLOTS:
    // DBUS interface. if you change these methods, you MUST run:
    // qdbuscpp2xml plasmaapp.h -o dbus/org.kde.plasma.App.xml
    void toggleDashboard();
    void showDashboard(bool show);
    void showInteractiveConsole();
    void loadScriptInInteractiveConsole(const QString &script);
    Q_SCRIPTABLE void quit();

    /**
     * Request a zoom based on the containment
     */
    void zoom(Plasma::Containment*, Plasma::ZoomDirection);

protected:
#ifdef Q_WS_X11
    PanelView *findPanelForTrigger(WId trigger) const;
    bool x11EventFilter(XEvent *event);
#endif
    void setControllerVisible(bool show);

private:
    PlasmaApp();
    DesktopView* viewForScreen(int screen, int desktop) const;
    void zoomIn(Plasma::Containment *containment);
    void zoomOut(Plasma::Containment *containment);

private Q_SLOTS:
    void zoomOut();
    void setupDesktop();
    void createConfigurationInterface();
    void configAccepted();
    void cleanup();
    void containmentAdded(Plasma::Containment *containment);
    void syncConfig();
    void createView(Plasma::Containment *containment);
    void panelRemoved(QObject* panel);
    void waitingPanelRemoved(QObject* panel);
    void createWaitingPanels();
    void screenRemoved(int id);
    void compositingChanged();
    void addContainment();
    void configureContainment(Plasma::Containment*);
    void updateActions(Plasma::ImmutabilityType immutability);
    void setPerVirtualDesktopViews(bool perDesktopViews);
    void checkVirtualDesktopViews(int numDesktops);
    void setFixedDashboard(bool fixedDashboard);
    void setWmClass(WId id);
    void slotRemotePlasmoidAdded(Plasma::PackageMetadata metadata);
    void slotAddRemotePlasmoid(const QString &location);
    void slotPlasmoidAccessFinished(Plasma::AccessAppletJob *job);

private:
    DesktopCorona *m_corona;
    QList<PanelView*> m_panels;
    QList<Plasma::Containment*> m_panelsWaiting;
    Plasma::Dialog *m_controllerDialog;
    QList<DesktopView*> m_desktops;
    QTimer *m_panelViewCreationTimer;
    Plasma::ZoomLevel m_zoomLevel;
    QPointer<InteractiveConsole> m_console;
    int m_panelHidden;
    QSignalMapper *m_mapper;

    Ui::GlobalOptions m_configUi;
};

#endif // multiple inclusion guard
