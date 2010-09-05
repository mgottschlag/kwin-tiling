/*
 *   Copyright 2006, 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 Chani Armitage <chanika@gmail.com>
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
#include <QPointer>
#include <QTimer>

#include <KUniqueApplication>

#include <plasma/plasma.h>

namespace Plasma
{
    class Containment;
    class Corona;
} // namespace Plasma

class SaverView;
class BackgroundDialog;

class PlasmaApp : public KUniqueApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.plasmaoverlay.App")
public:
    ~PlasmaApp();

    static PlasmaApp* self();
    static bool hasComposite();

    Plasma::Corona* corona();

    void setActiveOpacity(qreal opacity);
    void setIdleOpacity(qreal opacity);
    qreal activeOpacity() const;
    qreal idleOpacity() const;

Q_SIGNALS:
    // DBUS interface.
    //if you change stuff, remember to regenerate with:
    //qdbuscpp2xml -S -M plasmaapp.h > org.kde.plasma-overlay.App.xml

    //XXX can this be deleted? probably. if lockprocess really cares it can use the unmapnotify
    void hidden();

public Q_SLOTS:
    // DBUS interface.
    //if you change stuff, remember to regenerate ^^^
    /**
     * tell plasma to go into active mode, ready for interaction
     */
    void setActive(bool activate);

    /**
     * lock widgets
     */
    void lock();

    //not really slots, but we want them in dbus:

    /**
     * get plasma all set up and ready
     * this makes sure things like opacity, visibility and locked-ness are set right
     * normally this is called only by plasmaapp itself when it finishes initialization, but it's
     * possible that it might need to be run again by lockprocess
     *
     * @param setupMode whether we're starting in setup mode
     */
    void setup(bool setupMode);

    /**
     * quit the application
     * this is a duplicate so we can have everything we need in one dbus interface
     */
    void quit();

private Q_SLOTS:
    void cleanup();
    //void adjustSize(int screen);
    void dialogDestroyed(QObject *obj);
    void configureContainment(Plasma::Containment*);
    void syncConfig();
    void immutabilityChanged(Plasma::ImmutabilityType immutability);
    void createWaitingViews();
    void containmentScreenOwnerChanged(int, int, Plasma::Containment*);

Q_SIGNALS:
    void showViews();
    void hideViews();
    void setViewOpacity(qreal opacity);
    void showDialogs();
    void hideDialogs();
    void hideWidgetExplorer();
    void enableSetupMode();
    void disableSetupMode();
    void openToolBox();
    void closeToolBox();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    PlasmaApp(Display* display, Qt::HANDLE visual, Qt::HANDLE colormap);
    SaverView *viewForScreen(int screen);

    Plasma::Corona *m_corona;
    QList<SaverView*> m_views;
    QList<QWidget*> m_dialogs;
    QPointer<BackgroundDialog> m_configDialog;
    
    QList<QWeakPointer<Plasma::Containment> > m_viewsWaiting;
    QTimer m_viewCreationTimer;

    qreal m_activeOpacity;
    qreal m_idleOpacity;
    bool m_active;
};

#endif // multiple inclusion guard
