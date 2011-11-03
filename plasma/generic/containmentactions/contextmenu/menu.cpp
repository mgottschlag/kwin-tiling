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
#include <Plasma/Wallpaper>

#include "kworkspace/kworkspace.h"
#include "krunner_interface.h"
#include "screensaver_interface.h"

#ifdef Q_OS_WIN
#define _WIN32_WINNT 0x0500 // require NT 5.0 (win 2k pro)
#include <windows.h>
#endif // Q_OS_WIN

ContextMenu::ContextMenu(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args),
      m_runCommandAction(0),
      m_lockScreenAction(0),
      m_logoutAction(0),
      m_separator1(0),
      m_separator2(0),
      m_separator3(0),
      m_buttons(0)
{
}

ContextMenu::~ContextMenu()
{
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
        m_allActions << "_context" << "_run_command" << "add widgets" << "_add panel" << "add sibling containment" << "manage activities" << "remove" << "lock widgets" << "_sep1" << "_lock_screen" << "_logout" << "_sep2" << "configure" << "configure shortcuts" << "_sep3" << "_wallpaper";
        defaultEnabled << true << true << true << true << false << true << true << true << true << true << true << true << true << false << true << true;
    }

    for (int i = 0; i < m_allActions.count(); ++i) {
        m_enabledActions << config.readEntry(m_allActions.at(i), defaultEnabled.at(i));
    }

    if (isInitialized()) {
        kDebug() << "second init";
        return; //below here is stuff we only want to do once
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
        connect(m_logoutAction, SIGNAL(triggered(bool)), this, SLOT(startLogout()));

        m_separator1 = new QAction(this);
        m_separator1->setSeparator(true);
        m_separator2 = new QAction(this);
        m_separator2->setSeparator(true);
        m_separator3 = new QAction(this);
        m_separator3->setSeparator(true);
    }
}

void ContextMenu::contextEvent(QEvent *event)
{
    QList<QAction *> actions = contextualActions();
    if (actions.isEmpty()) {
        return;
    }

    KMenu desktopMenu;
    desktopMenu.addActions(actions);
    desktopMenu.adjustSize();
    desktopMenu.exec(popupPosition(desktopMenu.size(), event));
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
            } if (name == "_wallpaper") {
                if (c->wallpaper() &&
                      !c->wallpaper()->contextualActions().isEmpty()) {
                    actions << c->wallpaper()->contextualActions();
                }
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
    } else if (name == "_sep3") {
        return m_separator3;
    } else if (name == "_add panel") {
        if (c->corona() && c->corona()->immutability() == Plasma::Mutable) {
            return c->corona()->action("add panel");
        }
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
    } else if (name == "manage activities") {
        if (c->corona()) {
            return c->corona()->action("manage activities");
        }
    } else {
        //FIXME: remove action: make removal of current activity possible
        return c->action(name);
    }
    return 0;
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

void ContextMenu::startLogout()
{
    // this short delay is due to two issues:
    // a) KWorkSpace's DBus alls are all syncronous
    // b) the destrution of the menu that this action is in is delayed
    //
    // (a) leads to the menu hanging out where everyone can see it because
    // the even loop doesn't get returned to allowing it to close.
    //
    // (b) leads to a 0ms timer not working since a 0ms timer just appends to
    // the event queue, and then the menu closing event gets appended to that.
    //
    // ergo a timer with small timeout
    QTimer::singleShot(10, this, SLOT(logout()));
}

void ContextMenu::logout()
{
    if (!KAuthorized::authorizeKAction("logout")) {
        return;
    }

    KWorkSpace::requestShutDown();
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
        } else if (name == "_wallpaper") {
            item = new QCheckBox(widget);
            item->setText(i18n("Wallpaper Actions"));
            item->setIcon(KIcon("user-desktop"));
        } else if (name == "_sep1" || name =="_sep2" || name == "_sep3") {
            item = new QCheckBox(widget);
            item->setText(i18n("[Separator]"));
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
