/*
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
*
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
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

#include "desktop.h"

#include <limits>

#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsProxyWidget>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QTimeLine>

#include <KAuthorized>
#include <KComboBox>
#include <KDebug>
#include <KFileDialog>
#include <KImageFilePreview>
#include <KRun>
#include <KStandardDirs>
#include <KWindowSystem>

#include "plasma/corona.h"
#include "plasma/appletbrowser.h"
#include "plasma/animator.h"
#include "plasma/theme.h"
#include "kworkspace/kworkspace.h"
#include "knewstuff2/engine.h"

#include "krunner_interface.h"
#include "screensaver_interface.h"

#ifdef Q_OS_WIN
#define _WIN32_WINNT 0x0500 // require NT 5.0 (win 2k pro)
#include <windows.h>
#endif // Q_OS_WIN

using namespace Plasma;

DefaultDesktop::DefaultDesktop(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_lockDesktopAction(0),
      m_appletBrowserAction(0),
      m_addPanelAction(0),
      m_runCommandAction(0),
      m_lockScreenAction(0),
      m_logoutAction(0),
      restoring(false)
{
    qRegisterMetaType<QImage>("QImage");
    qRegisterMetaType<QPersistentModelIndex>("QPersistentModelIndex");
    connect(this, SIGNAL(appletAdded(Plasma::Applet *, const QPointF &)),
            this, SLOT(onAppletAdded(Plasma::Applet *, const QPointF &)));
    connect(KWindowSystem::self(), SIGNAL(workAreaChanged()),
            this, SLOT(refreshWorkingArea()));

    m_layout = new DesktopLayout;
    m_layout->setAutoWorkingArea(false);
    m_layout->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_layout->setPlacementSpacing(20);
    m_layout->setScreenSpacing(5);
    m_layout->setShiftingSpacing(0);
    m_layout->setTemporaryPlacement(true);
    setLayout(m_layout);

    //kDebug() << "!!! loading desktop";
}

DefaultDesktop::~DefaultDesktop()
{
    disconnect(KWindowSystem::self(), SIGNAL(workAreaChanged()),
               this, SLOT(refreshWorkingArea()));
}

QSize DefaultDesktop::resolution() const
{
    return QApplication::desktop()->screenGeometry(screen()).size();
}

void DefaultDesktop::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::ImmutableConstraint && m_appletBrowserAction) {
        // we need to update the menu items that have already been created
        bool locked = immutability() != Mutable;
        m_addPanelAction->setVisible(!locked);
    }
}

void DefaultDesktop::configure()
{
    emit configureRequested();
}

void DefaultDesktop::addPanel()
{
    if (corona()) {
        // make a panel at the top
        Containment* panel = corona()->addContainment("panel");
        panel->showConfigurationInterface();

        panel->setScreen(screen());
        panel->setLocation(Plasma::TopEdge);

        // trigger an instant layout so we immediately have a proper geometry
        // rather than waiting around for the event loop
        panel->updateConstraints(Plasma::StartupCompletedConstraint);
        panel->flushPendingConstraintsEvents();
    }
}

void DefaultDesktop::runCommand()
{
    if (!KAuthorized::authorizeKAction("run_command")) {
        return;
    }

    QString interface("org.kde.krunner");
    org::kde::krunner::Interface krunner(interface, "/Interface",
                                         QDBusConnection::sessionBus());
    if (krunner.isValid()) {
        krunner.display();
    }
}

void DefaultDesktop::lockScreen()
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

QList<QAction*> DefaultDesktop::contextualActions()
{
    //TODO: should we offer "Switch User" here?

    if (!m_appletBrowserAction) {
        m_appletBrowserAction = action("add widgets");

        m_addPanelAction = new QAction(i18n("Add Panel"), this);
        connect(m_addPanelAction, SIGNAL(triggered(bool)), this, SLOT(addPanel()));
        m_addPanelAction->setIcon(KIcon("list-add"));

        m_runCommandAction = new QAction(i18n("Run Command..."), this);
        connect(m_runCommandAction, SIGNAL(triggered(bool)), this, SLOT(runCommand()));
        m_runCommandAction->setIcon(KIcon("system-run"));

        m_setupDesktopAction = new QAction(i18n("Desktop Settings..."), this);
        m_setupDesktopAction->setIcon(KIcon("configure"));
        connect(m_setupDesktopAction, SIGNAL(triggered()), this, SLOT(configure()));

        m_lockDesktopAction = action("lock widgets");

        m_lockScreenAction = new QAction(i18n("Lock Screen"), this);
        m_lockScreenAction->setIcon(KIcon("system-lock-screen"));
        connect(m_lockScreenAction, SIGNAL(triggered(bool)), this, SLOT(lockScreen()));

        m_logoutAction = new QAction(i18n("Leave..."), this);
        m_logoutAction->setIcon(KIcon("system-shutdown"));
        connect(m_logoutAction, SIGNAL(triggered(bool)), this, SLOT(logout()));
        constraintsEvent(Plasma::ImmutableConstraint);

        m_separator = new QAction(this);
        m_separator->setSeparator(true);

        m_separator2 = new QAction(this);
        m_separator2->setSeparator(true);
    }

    QList<QAction*> actions;

    if (KAuthorized::authorizeKAction("run_command")) {
        actions.append(m_runCommandAction);
    }

    actions.append(m_appletBrowserAction);
    actions.append(m_addPanelAction);
    actions.append(m_setupDesktopAction);
    if (screen() == -1) {
        actions.append(action("remove"));
    }

    actions.append(m_lockDesktopAction);

    actions.append(m_separator);

    if (KAuthorized::authorizeKAction("lock_screen")) {
        actions.append(m_lockScreenAction);
    }

    if (KAuthorized::authorizeKAction("logout")) {
        actions.append(m_logoutAction);
    }

    return actions;
}

void DefaultDesktop::logout()
{
    if (!KAuthorized::authorizeKAction("logout")) {
        return;
    }
#ifndef Q_WS_WIN
    KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmDefault,
                                KWorkSpace::ShutdownTypeDefault,
                                KWorkSpace::ShutdownModeDefault);
#endif
}

void DefaultDesktop::onAppletAdded(Plasma::Applet *applet, const QPointF &pos)
{
    Q_UNUSED(pos)
    if (!restoring) {
        /*
            There seems to be no uniform way to get the applet's preferred size.
            Regular applets properly set their current size when created, but report useless size hints.
            Proxy widget applets don't set their size properly, but report valid size hints. However, they
            will obtain proper size if we just re-set the geometry to the current value.
        */
        applet->setGeometry(applet->geometry());
        m_layout->addItem(applet, true, applet->geometry().size());
    }

    connect(applet, SIGNAL(destroyed(QObject *)), this, SLOT(onAppletDestroyed(QObject *)));
    connect(applet, SIGNAL(geometryChanged()), this, SLOT(onAppletGeometryChanged()));
}

void DefaultDesktop::onAppletDestroyed(QObject *applet)
{
    for (int i=0; i < m_layout->count(); i++) {
        if ((Applet *)applet == m_layout->itemAt(i)) {
            m_layout->removeAt(i);
            return;
        }
    }
}

void DefaultDesktop::onAppletGeometryChanged()
{
    m_layout->itemGeometryChanged((Applet *)sender());
}

void DefaultDesktop::refreshWorkingArea()
{
    QRectF workingGeom = geometry();
    if (screen() != -1) {
        // we are associated with a screen, make sure not to overlap panels
        QDesktopWidget *desktop = qApp->desktop();
        workingGeom = desktop->availableGeometry(screen());
    }
    m_layout->setWorkingArea(workingGeom);
}

void DefaultDesktop::saveContents(KConfigGroup &group) const
{
    KConfigGroup applets(&group, "Applets");
    for (int i=0; i < m_layout->count(); i++) {
        Applet *applet = (Applet *)m_layout->itemAt(i);
        KConfigGroup appletConfig(&applets, QString::number(applet->id()));
        applet->save(appletConfig);
        appletConfig.writeEntry("desktoplayout_preferredGeometry", m_layout->getPreferredGeometry(i));
        appletConfig.writeEntry("desktoplayout_lastGeometry", m_layout->getLastGeometry(i));
    }
}

void DefaultDesktop::restoreContents(KConfigGroup &group)
{
    // prevent onAppletAdded from adding applets to the layout
    restoring = true;
    Containment::restoreContents(group);
    restoring = false;

    KConfigGroup appletsConfig(&group, "Applets");

    // add restored applets to the layout
    foreach (const QString &appletGroup, appletsConfig.groupList()) {
        KConfigGroup appletConfig(&appletsConfig, appletGroup);
        uint appId = appletConfig.name().toUInt();
        foreach (Applet *applet, applets()) {
            if (applet->id() == appId) {
                QRectF preferredGeom = appletConfig.readEntry("desktoplayout_preferredGeometry", QRectF());
                QRectF lastGeom = appletConfig.readEntry("desktoplayout_lastGeometry", QRectF());
                if (preferredGeom.isValid() && lastGeom.isValid()) {
                    m_layout->addItem(applet, true, preferredGeom, lastGeom);
                } else {
                    m_layout->addItem(applet, true, applet->geometry());
                }
                break;
            }
        }
    }
}

K_EXPORT_PLASMA_APPLET(desktop, DefaultDesktop)

#include "desktop.moc"
