/*
 *   Copyright 2009 by Chani Armitage <chani@kde.org>
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

#include "menu.h"

#include <QAction>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QSignalMapper>

#include <KAuthorized>
#include <KDebug>
#include <KIcon>
#include <KMenu>

#include <Plasma/Containment>
#include <Plasma/Corona>

#include "kworkspace/kworkspace.h"
#include "krunner_interface.h"
#include "screensaver_interface.h"

#ifdef Q_OS_WIN
#define _WIN32_WINNT 0x0500 // require NT 5.0 (win 2k pro)
#include <windows.h>
#endif // Q_OS_WIN

ContextMenu::ContextMenu(QObject *parent, const QVariantList &args)
    : Plasma::ContextAction(parent, args),
      m_addPanelsMenu(0),
      m_addPanelAction(0),
      m_runCommandAction(0),
      m_lockScreenAction(0),
      m_logoutAction(0)
{
}

ContextMenu::~ContextMenu()
{
    delete m_addPanelsMenu;
}

void ContextMenu::init(const KConfigGroup &config)
{
    if (isInitialized()) {
        kDebug() << "second init";
        return; //atm we have no config to reload
    }

    Plasma::Containment *c = containment();
    Q_ASSERT(c);
    connect(c, SIGNAL(immutabilityChanged(const Plasma::ImmutabilityType)), this, SLOT(updateImmutability(const Plasma::ImmutabilityType)));

    //FIXME what if it's a panel? what if it's a customcontainment?
    KPluginInfo::List panelPlugins = c->listContainmentsOfType("panel");

    if (panelPlugins.size() == 1) {
        m_addPanelAction = new QAction(i18n("Add Panel"), this);
        connect(m_addPanelAction, SIGNAL(triggered(bool)), this, SLOT(addPanel()));
    } else if (!panelPlugins.isEmpty()) {
        m_addPanelsMenu = new QMenu();
        m_addPanelAction = m_addPanelsMenu->menuAction();
        m_addPanelAction->setText(i18n("Add Panel"));

        QSignalMapper *mapper = new QSignalMapper(this);
        connect(mapper, SIGNAL(mapped(QString)), this, SLOT(addPanel(QString)));

        foreach (const KPluginInfo &plugin, panelPlugins) {
            QAction *action = new QAction(plugin.name(), this);
            if (!plugin.icon().isEmpty()) {
                action->setIcon(KIcon(plugin.icon()));
            }

            mapper->setMapping(action, plugin.pluginName());
            connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
            m_addPanelsMenu->addAction(action);
        }
    }

    if (m_addPanelAction) {
        m_addPanelAction->setIcon(KIcon("list-add"));
    }

    if (c->containmentType() == Plasma::Containment::PanelContainment ||
            c->containmentType() == Plasma::Containment::CustomPanelContainment) {
        //panel does its own config action atm... FIXME how do I fit it in properly?
        //can I do something with configureRequested? damn privateslot...
    } else {
        m_runCommandAction = new QAction(i18n("Run Command..."), this);
        m_runCommandAction->setIcon(KIcon("system-run"));
        connect(m_runCommandAction, SIGNAL(triggered(bool)), this, SLOT(runCommand()));

        m_lockScreenAction = new QAction(i18n("Lock Screen"), this);
        m_lockScreenAction->setIcon(KIcon("system-lock-screen"));
        connect(m_lockScreenAction, SIGNAL(triggered(bool)), this, SLOT(lockScreen()));

        m_logoutAction = new QAction(i18n("Leave..."), this);
        m_logoutAction->setIcon(KIcon("system-shutdown"));
        connect(m_logoutAction, SIGNAL(triggered(bool)), this, SLOT(logout()));
    }

    updateImmutability(c->immutability());
}

void ContextMenu::updateImmutability(const Plasma::ImmutabilityType immutable)
{
    bool locked = immutable != Plasma::Mutable;
    m_addPanelAction->setVisible(!locked);
}

void ContextMenu::contextEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::GraphicsSceneMouseRelease:
            contextEvent(dynamic_cast<QGraphicsSceneMouseEvent*>(event));
            break;
        case QEvent::GraphicsSceneWheel:
            wheelEvent(dynamic_cast<QGraphicsSceneWheelEvent*>(event));
            break;
        default:
            break;
    }
}

void ContextMenu::contextEvent(QGraphicsSceneMouseEvent *event)
{
    QList<QAction*> actions = contextualActions();

    KMenu desktopMenu;
    desktopMenu.addActions(actions);
    desktopMenu.exec(event->screenPos());
}

void ContextMenu::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    kDebug() << "test!!!!!!!!!!!!!11111111!!";
    kDebug() << event->orientation() << event->delta();
    kDebug() << event->buttons() << event->modifiers();
}

QList<QAction*> ContextMenu::contextualActions()
{
    Plasma::Containment *c = containment();
    Q_ASSERT(c);
    QList<QAction*> actions = c->contextualActions();

    //standard actions
    //FIXME what if it's a customcontainment?
    //FIXME does anyone care that the panel/desktopaction orders are different?
    if (c->containmentType() == Plasma::Containment::PanelContainment ||
            c->containmentType() == Plasma::Containment::CustomPanelContainment) {
        QAction *appletBrowserAction = c->action("add widgets");
        if (appletBrowserAction) {
            actions.append(appletBrowserAction);
        }

        if (m_addPanelAction) {
            actions.append(m_addPanelAction);
        }

        QAction *lockDesktopAction = c->action("lock widgets");
        if (lockDesktopAction) {
            actions.append(lockDesktopAction);
        }

        //FIXME config action used to go here

        QAction *removeAction = c->action("remove");
        if (removeAction) {
            actions.append(removeAction);
        }

    } else {
        if (KAuthorized::authorizeKAction("run_command")) {
            actions.append(m_runCommandAction);
        }

        QAction *appletBrowserAction = c->action("add widgets");
        if (appletBrowserAction) {
            actions.append(appletBrowserAction);
        }

        if (m_addPanelAction) {
            actions.append(m_addPanelAction);
        }

        QAction *removeAction = c->action("remove");
        //FIXME make removal of current activity possible
        if (c->screen() == -1 && removeAction) {
            actions.append(removeAction);
        }

        QAction *lockDesktopAction = c->action("lock widgets");
        if (lockDesktopAction) {
            actions.append(lockDesktopAction);
        }

        QAction *sep = new QAction(this);
        sep->setSeparator(true);
        actions.append(sep);

        if (KAuthorized::authorizeKAction("lock_screen")) {
            actions.append(m_lockScreenAction);
        }

        if (KAuthorized::authorizeKAction("logout")) {
            actions.append(m_logoutAction);
        }

        if (c->containmentType() == Plasma::Containment::DesktopContainment) {
            actions << sep;
            actions << c->action("configure");
        }

    }
    return actions;
}

void ContextMenu::addPanel()
{
    KPluginInfo::List panelPlugins = Plasma::Containment::listContainmentsOfType("panel");

    if (!panelPlugins.isEmpty()) {
        addPanel(panelPlugins.first().pluginName());
    }
}

//FIXME maybe this function belongs somewhere else. corona? plasmaapp?
void ContextMenu::addPanel(const QString &plugin)
{
    Plasma::Containment *c = containment();
    if (c->corona()) {
        Plasma::Containment* panel = c->corona()->addContainment(plugin);
        panel->showConfigurationInterface();

        panel->setScreen(c->screen());

        QList<Plasma::Location> freeEdges = c->corona()->freeEdges(c->screen());
        //kDebug() << freeEdges;
        Plasma::Location destination;
        if (freeEdges.contains(Plasma::TopEdge)) {
            destination = Plasma::TopEdge;
        } else if (freeEdges.contains(Plasma::BottomEdge)) {
            destination = Plasma::BottomEdge;
        } else if (freeEdges.contains(Plasma::LeftEdge)) {
            destination = Plasma::LeftEdge;
        } else if (freeEdges.contains(Plasma::RightEdge)) {
            destination = Plasma::RightEdge;
        } else destination = Plasma::TopEdge;

        panel->setLocation(destination);

        // trigger an instant layout so we immediately have a proper geometry
        // rather than waiting around for the event loop
        panel->updateConstraints(Plasma::StartupCompletedConstraint);
        panel->flushPendingConstraintsEvents();

        const QRect screenGeom = c->corona()->screenGeometry(c->screen());
        const QRegion availGeom = c->corona()->availableScreenRegion(c->screen());
        int minH = 10;
        int minW = 10;
        int w = 35;
        int h = 35;

        if (destination == Plasma::LeftEdge) {
            QRect r = availGeom.intersected(QRect(0, 0, w, screenGeom.height())).boundingRect();
            h = r.height();
            minW = 35;
        } else if (destination == Plasma::RightEdge) {
            QRect r = availGeom.intersected(QRect(screenGeom.width() - w, 0, w, screenGeom.height())).boundingRect();
            h = r.height();
            minW = 35;
        } else if (destination == Plasma::TopEdge) {
            QRect r = availGeom.intersected(QRect(0, 0, screenGeom.width(), h)).boundingRect();
            w = r.width();
            minH = 35;
        } else if (destination == Plasma::BottomEdge) {
            QRect r = availGeom.intersected(QRect(0, screenGeom.height() - h, screenGeom.width(), h)).boundingRect();
            w = r.width();
            minH = 35;
        }

        panel->setMinimumSize(minW, minH);
        panel->setMaximumSize(w, h);
        panel->resize(w, h);
    }
}

void ContextMenu::runCommand()
{
    if (!KAuthorized::authorizeKAction("run_command")) {
        return;
    }

    QString interface("org.kde.krunner");
    org::kde::krunner::App krunner(interface, "/App", QDBusConnection::sessionBus());
    krunner.display();
}

void ContextMenu::lockScreen()
{
    if (!KAuthorized::authorizeKAction("lock_screen")) {
        return;
    }

#ifndef Q_OS_WIN
    QString interface("org.freedesktop.ScreenSaver");
    org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",
                                              QDBusConnection::sessionBus());
    if (screensaver.isValid()) {
        screensaver.Lock();
    }
#else
    LockWorkStation();
#endif // !Q_OS_WIN
}

void ContextMenu::logout()
{
    if (!KAuthorized::authorizeKAction("logout")) {
        return;
    }
#ifndef Q_WS_WIN
    KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmYes,
                                KWorkSpace::ShutdownTypeDefault,
                                KWorkSpace::ShutdownModeDefault);
#endif
}


#include "menu.moc"
