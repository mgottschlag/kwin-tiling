/*
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QTimeLine>

#include <KAuthorized>
#include <KDebug>
#include <KRun>
#include <KStandardDirs>
#include <KWindowSystem>

#include "plasma/appletbrowser.h"
#include "plasma/phase.h"
#include "plasma/svg.h"
#include "kworkspace/kworkspace.h"

#include "krunner_interface.h"
#include "ksmserver_interface.h"
#include "screensaver_interface.h"

using namespace Plasma;

DefaultDesktop::DefaultDesktop(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_appletBrowserAction(0),
      m_runCommandAction(0),
      m_lockAction(0),
      m_logoutAction(0),
      m_background(0),
      m_bitmapBackground(0),
      m_wallpaperPath(0)
{
    //kDebug() << "!!! loading desktop";
}

DefaultDesktop::~DefaultDesktop()
{
}

void DefaultDesktop::init()
{
    KConfigGroup config(KGlobal::config(), "General");
    m_wallpaperPath = config.readEntry("wallpaper", KStandardDirs::locate("wallpaper", "plasma-default.png"));

    //kDebug() << "wallpaperPath is" << m_wallpaperPath << QFile::exists(m_wallpaperPath);
    if (m_wallpaperPath.isEmpty() ||
        !QFile::exists(m_wallpaperPath)) {
        //kDebug() << "SVG wallpaper!";
        m_background = new Plasma::Svg("widgets/wallpaper", this);
    }

    Containment::init();
}

void DefaultDesktop::constraintsUpdated(Plasma::Constraints constraints)
{
    //kDebug() << "DefaultDesktop constraints have changed";
    const QRect geom = QApplication::desktop()->screenGeometry(screen());
    if (m_background) {
        //kDebug() << "Rescaling SVG wallpaper to" << geom.size();
        m_background->resize(geom.size());
    } else if (!m_wallpaperPath.isEmpty()) {
        if (!m_bitmapBackground || !(m_bitmapBackground->size() == geom.size())) {
            //kDebug() << "Loading and scaling bitmap wallpaper to" << geom.size() << "while our geometry is" << geometry();
            delete m_bitmapBackground;
            m_bitmapBackground = new QPixmap(m_wallpaperPath);
            (*m_bitmapBackground) = m_bitmapBackground->scaled(geom.size());
        }
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

    QString interface("org.freedesktop.ScreenSaver");
    org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",
                                              QDBusConnection::sessionBus());
    if (screensaver.isValid()) {
        screensaver.Lock();
    }
}

QList<QAction*> DefaultDesktop::contextActions()
{
    //FIXME: several items here ... probably all junior jobs =)
    //  - pretty up the menu with separators
    //  - should we offer "Switch User" here?

    if (!m_appletBrowserAction) {
        m_appletBrowserAction = new QAction(i18n("Add Widgets..."), this);
        connect(m_appletBrowserAction, SIGNAL(triggered(bool)), this, SIGNAL(showAddWidgets()));

        m_runCommandAction = new QAction(i18n("Run Command..."), this);
        connect(m_runCommandAction, SIGNAL(triggered(bool)), this, SLOT(runCommand()));

        m_lockAction = new QAction(i18n("Lock Screen"), this);
        m_lockAction->setIcon(KIcon("system-lock-screen"));
        connect(m_lockAction, SIGNAL(triggered(bool)), this, SLOT(lockScreen()));

        m_logoutAction = new QAction(i18n("Logout"), this);
        m_logoutAction->setIcon(KIcon("system-log-out"));
        connect(m_logoutAction, SIGNAL(triggered(bool)), this, SLOT(logout()));
    }

    QList<QAction*> actions;

    actions.append(m_appletBrowserAction);

    if (KAuthorized::authorizeKAction("run_command")) {
        actions.append(m_runCommandAction);
    }

    if (KAuthorized::authorizeKAction("lock_screen")) {
        actions.append(m_lockAction);
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

    QString interface("org.kde.ksmserver");
    org::kde::KSMServerInterface smserver(interface, "/KSMServer",
                                          QDBusConnection::sessionBus());
    if (smserver.isValid()) {
        smserver.logout(KWorkSpace::ShutdownConfirmDefault,
                        KWorkSpace::ShutdownTypeDefault,
                        KWorkSpace::ShutdownModeDefault);
    }
}

void DefaultDesktop::paintInterface(QPainter *painter,
                                    const QStyleOptionGraphicsItem *option,
                                    const QRect& contentsRect)
{
    //kDebug() << "paintInterface of background";
    if (!m_background && !m_bitmapBackground) {
        Containment::paintInterface(painter, option, contentsRect);
        return;
    }

    painter->save();

    if (painter->worldMatrix() == QMatrix()) {
        // draw the background untransformed when possible;(saves lots of per-pixel-math)
        painter->resetTransform();
    }

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    if (m_background) {
        // Plasma::Svg doesn't support drawing only part of the image (it only
        // supports drawing the whole image to a rect), so we blit to 0,0-w,h
        m_background->paint(painter, contentsRect);
    //kDebug() << "draw svg of background";
    } else if (m_bitmapBackground) {
        // for pixmaps we draw only the exposed part (untransformed since the
        // bitmapBackground already has the size of the viewport)
        painter->drawPixmap(option->exposedRect, *m_bitmapBackground, option->exposedRect);
        //kDebug() << "draw pixmap of background to" << option->exposedRect;
    }

    // restore transformation and composition mode
    painter->restore();
}

K_EXPORT_PLASMA_APPLET(desktop, DefaultDesktop)

#include "desktop.moc"
