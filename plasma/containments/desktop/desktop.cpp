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

#include <Plasma/Corona>
#include <Plasma/Animator>
#include <Plasma/Theme>
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
      dropping(false)
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
    m_layout->setVisibilityTolerance(0.5);
    setLayout(m_layout);
    setHasConfigurationInterface(true);

    resize(800, 600);
    //kDebug() << "!!! loading desktop";
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
    org::kde::krunner::App krunner(interface, "/App", QDBusConnection::sessionBus());
    krunner.display();
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

        m_setupDesktopAction = action("configure");
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
    if (dropping || pos != QPointF(-1,-1) || applet->geometry().topLeft() != QPointF(0,0)) {
        // add item to the layout using the current position
        m_layout->addItem(applet, true, applet->geometry());
    } else {
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
    QRectF workingGeom;
    if (screen() != -1) {
        // we are associated with a screen, make sure not to overlap panels
        QDesktopWidget *desktop = qApp->desktop();
        workingGeom = desktop->availableGeometry(screen());
        // From screen coordinates to containment coordinates
        workingGeom.translate(-desktop->screenGeometry(screen()).topLeft());
    } else {
        workingGeom = geometry();
        workingGeom = mapFromScene(workingGeom).boundingRect();
    }
    m_layout->setWorkingArea(workingGeom);
}

void DefaultDesktop::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    dropping = true;
    Containment::dropEvent(event);
    dropping = false;
}

K_EXPORT_PLASMA_APPLET(desktop, DefaultDesktop)

#include "desktop.moc"
