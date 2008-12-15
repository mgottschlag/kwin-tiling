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

#include <KUniqueApplication>

#include <Plasma/Plasma>

class KSelectionWatcher;

namespace Plasma
{
    class AppletBrowser;
    class Containment;
    class Corona;
} // namespace Plasma

namespace Kephal {
    class Screen;
} // namespace Kephal

class DesktopView;
class BackgroundDialog;
class RootWidget;
class PanelView;
class DesktopCorona;

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

    /**
     * Creates a view for the given containment
     */
    void createDesktopView(Plasma::Containment *containment, int id = 0);

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

public Q_SLOTS:
    // DBUS interface. if you change these methods, you MUST run:
    // qdbuscpp2xml plasmaapp.h -o dbus/org.kde.plasma.App.xml
    void toggleDashboard();

    /**
     * Request a zoom based on the containment
     */
    void zoom(Plasma::Containment*, Plasma::ZoomDirection);

protected:
#ifdef Q_WS_X11
    PanelView *findPanelForTrigger(WId trigger) const;
    bool x11EventFilter(XEvent *event);
#endif

private:
    PlasmaApp(Display* display, Qt::HANDLE visual, Qt::HANDLE colormap);
    DesktopView* viewForScreen(int screen, int desktop) const;
    void zoomIn(Plasma::Containment *containment);
    void zoomOut(Plasma::Containment *containment);

private Q_SLOTS:
    void setupDesktop();
    void cleanup();
    void containmentAdded(Plasma::Containment *containment);
    void syncConfig();
    void appletBrowserDestroyed();
    void createView(Plasma::Containment *containment);
    void panelRemoved(QObject* panel);
    void configDialogRemoved(QObject* configDialogRemoved);
    void screenRemoved(int id);
    void compositingChanged();
    void showAppletBrowser();
    void addContainment(Plasma::Containment *fromContainment = 0);
    void configureContainment(Plasma::Containment*);

private:
    DesktopCorona *m_corona;
    QList<PanelView*> m_panels;
    Plasma::AppletBrowser *m_appletBrowser;
    QList<DesktopView*> m_desktops;
    QHash<Plasma::Containment *, BackgroundDialog *> m_configDialogs;
    Plasma::ZoomLevel m_zoomLevel;
    int m_panelHidden;

#ifdef Q_WS_X11
    KSelectionWatcher *m_compositeWatch;
#endif
};

#endif // multiple inclusion guard
