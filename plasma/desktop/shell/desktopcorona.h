/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef DESKTOPCORONA_H
#define DESKTOPCORONA_H

#include <QtGui/QGraphicsScene>
#include <QHash>

#include <Plasma/Corona>

class QMenu;
class QAction;

class Activity;
class KActivityController;

namespace Plasma
{
    class Applet;
} // namespace Plasma

namespace Kephal {
    class Screen;
} // namespace Kephal

/**
 * @short A Corona with desktop-y considerations
 */
class DesktopCorona : public Plasma::Corona
{
    Q_OBJECT

public:
    explicit DesktopCorona(QObject * parent = 0);
    ~DesktopCorona();

    /**
     * Loads the default (system wide) layout for this user
     **/
    void loadDefaultLayout();

    /**
     * Ensures we have the necessary containments for every screen
     */
    void checkScreens(bool signalWhenExists = false);

    /**
     * Ensures we have the necessary containments for the given screen
     */
    void checkScreen(int screen, bool signalWhenExists = false);

    int numScreens() const;
    QRect screenGeometry(int id) const;
    QRegion availableScreenRegion(int id) const;
    int screenId(const QPoint &pos) const;

    bool loadDefaultLayoutScripts();
    void processUpdateScripts();

    /**
     * Ensures activities exist for the containments
     */
    void checkActivities();

    /**
     * @return the Activity object for the given activity id
     */
    Activity* activity(const QString &id);

public Q_SLOTS:
    QRect availableScreenRect(int id) const;
    void addPanel();
    void addPanel(QAction *action);
    void addPanel(const QString &plugin);
    void populateAddPanelsMenu();
    void activateNextActivity();
    void activatePreviousActivity();
    void evaluateScripts(const QStringList &scripts, bool isStartup = true);

protected Q_SLOTS:
    void screenAdded(Kephal::Screen *s);
    void saveDefaultSetup();
    void printScriptError(const QString &error);
    void printScriptMessage(const QString &error);
    void updateImmutability(Plasma::ImmutabilityType immutability);
    void checkAddPanelAction(const QStringList &sycocaChanges = QStringList());
    void currentActivityChanged(const QString &activity);
    void activityAdded(const QString &id);
    void activityRemoved(const QString &id);

private:
    void init();

    Plasma::Applet *loadDefaultApplet(const QString &pluginName, Plasma::Containment *c);
    void checkDesktop(Activity *activity, bool signalWhenExists, int screen, int desktop = -1);

    QAction *m_addPanelAction;
    QMenu *m_addPanelsMenu;
    KActivityController *m_activityController;
    QHash<QString, Activity*> m_activities;
};

#endif


