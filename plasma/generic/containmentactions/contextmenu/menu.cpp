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
#include <QCheckBox>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QVBoxLayout>
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
    : Plasma::ContainmentActions(parent, args),
      m_addPanelsMenu(0),
      m_addPanelAction(0),
      m_runCommandAction(0),
      m_lockScreenAction(0),
      m_logoutAction(0),
      m_separator1(0),
      m_separator2(0),
      m_buttons(0)
{
}

ContextMenu::~ContextMenu()
{
    delete m_addPanelsMenu;
}

void ContextMenu::init(const KConfigGroup &config)
{
    Plasma::Containment *c = containment();
    Q_ASSERT(c);

    m_allActions.clear();
    m_enabledActions.clear();
    QList<bool> defaultEnabled;

    //FIXME what if it's a customcontainment?
    //FIXME does anyone care that the panel/desktopaction orders are different?
    if (c->containmentType() == Plasma::Containment::PanelContainment ||
            c->containmentType() == Plasma::Containment::CustomPanelContainment) {
        m_allActions << "add widgets" << "_add panel" << "lock widgets" << "_context" << "remove";
        defaultEnabled << true << true << true << true << true;
    } else {
        //FIXME ugly code!
        m_allActions << "_context" << "_run_command" << "add widgets" << "_add panel" << "remove" << "lock widgets" << "zoom in" << "zoom out" << "_sep1" << "_lock_screen" << "_logout" << "_sep2" << "configure" << "configure shortcuts";
        defaultEnabled << true << true << true << true << true << true << false << false << true << true << true << true << true << false;
    }

    for (int i = 0; i < m_allActions.count(); ++i) {
        m_enabledActions << config.readEntry(m_allActions.at(i), defaultEnabled.at(i));
    }

    if (isInitialized()) {
        kDebug() << "second init";
        return; //below here is stuff we only want to do once
    }

    connect(c->corona(), SIGNAL(immutabilityChanged(const Plasma::ImmutabilityType)), this, SLOT(updateImmutability(const Plasma::ImmutabilityType)));

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

        m_separator1 = new QAction(this);
        m_separator1->setSeparator(true);
        m_separator2 = new QAction(this);
        m_separator2->setSeparator(true);
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
    QPoint screenPos;
    switch (event->type()) {
        case QEvent::GraphicsSceneMousePress: {
            QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent*>(event);
            screenPos = e->screenPos();
            break;
        }
        case QEvent::GraphicsSceneWheel: {
            QGraphicsSceneWheelEvent *e = static_cast<QGraphicsSceneWheelEvent*>(event);
            screenPos = e->screenPos();
            break;
        }
        default:
            return;
    }

    KMenu desktopMenu;
    desktopMenu.addActions(contextualActions());
    desktopMenu.exec(screenPos);
}

QList<QAction*> ContextMenu::contextualActions()
{
    Plasma::Containment *c = containment();
    Q_ASSERT(c);
    QList<QAction*> actions;
    for (int i = 0; i < m_allActions.count(); ++i) {
        if (m_enabledActions.at(i)) {
            QString name = m_allActions.at(i);
            if (name == "_context") {
                actions << c->contextualActions();
            } else {
                QAction *a = action(name);
                if (a) {
                    actions << a;
                }
            }
        }
    }
    return actions;
}

QAction *ContextMenu::action(const QString &name)
{
    Plasma::Containment *c = containment();
    Q_ASSERT(c);
    if (name == "_sep1") {
        return m_separator1;
    } else if (name == "_sep2") {
        return m_separator2;
    } else if (name == "_add panel") {
        return m_addPanelAction;
    } else if (name == "_run_command") {
        if (KAuthorized::authorizeKAction("run_command")) {
            return m_runCommandAction;
        }
    } else if (name == "_lock_screen") {
        if (KAuthorized::authorizeKAction("lock_screen")) {
            return m_lockScreenAction;
        }
    } else if (name == "_logout") {
        if (KAuthorized::authorizeKAction("logout")) {
            return m_logoutAction;
        }
    } else {
        //FIXME: remove action: make removal of current activity possible
        return c->action(name);
    }
    return 0;
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

QWidget* ContextMenu::createConfigurationInterface(QWidget* parent)
{
    QWidget *widget = new QWidget(parent);
    QVBoxLayout *lay = new QVBoxLayout();
    widget->setLayout(lay);
    widget->setWindowTitle(i18n("Configure Contextual Menu Plugin"));
    m_buttons = new QButtonGroup(widget);
    m_buttons->setExclusive(false);
    for (int i = 0; i < m_allActions.count(); ++i) {
        QString name = m_allActions.at(i);
        QCheckBox *item = 0;

        if (name == "_context") {
            item = new QCheckBox(widget);
            //FIXME better text
            item->setText(i18n("[Other Actions]"));
        } else {
            QAction *a = action(name);
            if (a) {
                item = new QCheckBox(widget);
                item->setText(a->text());
                item->setIcon(a->icon());
            }
        }

        if (item) {
            item->setChecked(m_enabledActions.at(i));
            lay->addWidget(item);
            m_buttons->addButton(item, i);
        }
    }

    return widget;
}

void ContextMenu::configurationAccepted()
{
    for (int i = 0; i < m_allActions.count(); ++i) {
        QAbstractButton *b = m_buttons->button(i);
        if (b) {
            m_enabledActions.replace(i, b->isChecked());
        }
    }
}

void ContextMenu::save(KConfigGroup &config)
{
    //TODO should I only write the changed ones?
    for (int i = 0; i < m_allActions.count(); ++i) {
        config.writeEntry(m_allActions.at(i), m_enabledActions.at(i));
    }
}


#include "menu.moc"
