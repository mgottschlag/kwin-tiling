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
#include <KSvgRenderer>
#include <KWindowSystem>

#include "plasma/corona.h"
#include "plasma/appletbrowser.h"
#include "plasma/phase.h"
#include "plasma/theme.h"
#include "kworkspace/kworkspace.h"
#include "knewstuff2/engine.h"

#include "krunner_interface.h"
#include "ksmserver_interface.h"
#include "screensaver_interface.h"

#include "backgrounddialog.h"

using namespace Plasma;

DefaultDesktop::DefaultDesktop(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_lockDesktopAction(0),
      m_appletBrowserAction(0),
      m_runCommandAction(0),
      m_lockScreenAction(0),
      m_logoutAction(0),
      m_configDialog(0),
      m_wallpaperPath(0),
      m_renderer(resolution(), 1.0)
{
    //kDebug() << "!!! loading desktop";
}

DefaultDesktop::~DefaultDesktop()
{
    delete m_configDialog;
}

void DefaultDesktop::init()
{
    connect(&m_renderer, SIGNAL(done(int, QImage)),
            this, SLOT(updateBackground(int, QImage)));
    connect(&m_slideshowTimer, SIGNAL(timeout()),
            this, SLOT(nextSlide()));
    reloadConfig();
    Containment::init();
}

void DefaultDesktop::nextSlide()
{
    if (++m_currentSlide >= m_slideFiles.size()) {
        m_currentSlide = 0;
    }

    if (m_slideFiles.size() > 0) {
        m_wallpaperPath = m_slideFiles[m_currentSlide];
        updateBackground();
    }
}

QSize DefaultDesktop::resolution() const
{
    return QApplication::desktop()->screenGeometry(screen()).size();
}

void DefaultDesktop::constraintsUpdated(Plasma::Constraints constraints)
{
    Q_UNUSED(constraints);
    
    if (constraints & Plasma::SizeConstraint) {
        m_renderer.setSize(resolution()); 
        updateBackground();
    }

    if (constraints & Plasma::ImmutableConstraint && m_appletBrowserAction) {
        // we need to update the menu items that have already been created
        bool locked = isImmutable();
        m_appletBrowserAction->setVisible(!locked);
        m_setupDesktopAction->setVisible(!locked);
        if (locked) {
            m_lockDesktopAction->setIcon(KIcon("object-unlocked"));
            m_lockDesktopAction->setText(i18n("Unlock Widgets"));
        } else {
            m_lockDesktopAction->setIcon(KIcon("object-locked"));
            m_lockDesktopAction->setText(i18n("Lock Widgets"));
        }
    }
}

void DefaultDesktop::configure()
{
    KConfigGroup cg = config();
    if (m_configDialog == 0) {
        const QSize resolution = 
            QApplication::desktop()->screenGeometry(screen()).size();
        m_configDialog = new BackgroundDialog(resolution, cg, 0);
        connect(m_configDialog, SIGNAL(okClicked()), 
                this, SLOT(applyConfig()));
        connect(m_configDialog, SIGNAL(applyClicked()), 
                this, SLOT(applyConfig()));
    }
    else {
        m_configDialog->reloadConfig(cg);
    }

    m_configDialog->show();
}

void DefaultDesktop::applyConfig()
{
    Q_ASSERT(m_configDialog);
    m_configDialog->saveConfig(config());
    config().config()->sync();
    
    reloadConfig();
}

void DefaultDesktop::reloadConfig()
{
    KConfigGroup cg = config();
    
    m_backgroundMode = cg.readEntry("backgroundmode", 
        (int) BackgroundDialog::kStaticBackground);
    if (m_backgroundMode == BackgroundDialog::kStaticBackground) {
        m_wallpaperPath = cg.readEntry("wallpaper",
            KStandardDirs::locate("wallpaper", "plasma-default.png"));
        m_wallpaperPosition = cg.readEntry("wallpaperposition", 0);
        m_wallpaperColor = cg.readEntry("wallpapercolor", QColor(Qt::black));
        m_slideshowTimer.stop();
        updateBackground();
    }
    else {
        QStringList dirs = cg.readEntry("slidepaths", QStringList());
        QStringList filters;
        filters << "*.png" << "*.jpeg" << "*.jpg" << "*.svg" << "*.svgz";
        
        m_slideFiles.clear();
        foreach (QString path, dirs) {
            // TODO load packages, too
            QDir dir(path);
            dir.setNameFilters(filters);
            dir.setFilter(QDir::Files | QDir::Hidden);
            QFileInfoList files = dir.entryInfoList();
            foreach (QFileInfo wp, files)
            {
                m_slideFiles.append(wp.filePath());
                kDebug() << "add background to slideshow" << wp.filePath();
            }
        }
        int delay = cg.readEntry("slideTimer", 60);
		kDebug() << "reading slideTimer as : " << delay;
        m_slideshowTimer.setInterval(delay * 1000);
        if (!m_slideshowTimer.isActive()) {
            m_slideshowTimer.start();
        }
        m_currentSlide = -1;
        nextSlide();
    }
}

void DefaultDesktop::updateBackground()
{
    m_current_renderer_token = 
        m_renderer.render(m_wallpaperPath,
                          m_wallpaperColor,
                          (Background::ResizeMethod)m_wallpaperPosition,
                          Qt::SmoothTransformation);                       
}

void DefaultDesktop::updateBackground(int token, const QImage &img)
{
    if (m_current_renderer_token == token) {
        m_bitmapBackground = QPixmap::fromImage(img);
        update();
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
        m_appletBrowserAction->setIcon(KIcon("edit-add"));

        m_runCommandAction = new QAction(i18n("Run Command..."), this);
        connect(m_runCommandAction, SIGNAL(triggered(bool)), this, SLOT(runCommand()));
        m_runCommandAction->setIcon(KIcon("system-run"));

        m_setupDesktopAction = new QAction(i18n("Configure Desktop..."), this);
        m_setupDesktopAction->setIcon(KIcon("configure"));
        connect(m_setupDesktopAction, SIGNAL(triggered()), this, SLOT(configure()));

        m_lockDesktopAction = new QAction(i18n("Lock Widgets"), this);
        connect(m_lockDesktopAction, SIGNAL(triggered(bool)), this, SLOT(toggleDesktopImmutability()));

        m_lockScreenAction = new QAction(i18n("Lock Screen"), this);
        m_lockScreenAction->setIcon(KIcon("system-lock-screen"));
        connect(m_lockScreenAction, SIGNAL(triggered(bool)), this, SLOT(lockScreen()));

        m_logoutAction = new QAction(i18n("Logout"), this);
        m_logoutAction->setIcon(KIcon("system-log-out"));
        connect(m_logoutAction, SIGNAL(triggered(bool)), this, SLOT(logout()));
        constraintsUpdated(Plasma::ImmutableConstraint);

        m_separator = new QAction(this);
        m_separator->setSeparator(true);
    }

    QList<QAction*> actions;

    if (KAuthorized::authorizeKAction("run_command")) {
        actions.append(m_runCommandAction);
    }

    actions.append(m_appletBrowserAction);
    actions.append(m_setupDesktopAction);

    actions.append(m_separator);
    actions.append(m_lockDesktopAction);

    if (KAuthorized::authorizeKAction("lock_screen")) {
        actions.append(m_lockScreenAction);
    }

    if (KAuthorized::authorizeKAction("logout")) {
        actions.append(m_logoutAction);
    }

    return actions;
}

void DefaultDesktop::toggleDesktopImmutability() {
    if (corona()) {
        corona()->setImmutable(!corona()->isImmutable());
    } else {
        setImmutable(!isImmutable());
    }
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
    if (m_bitmapBackground.isNull()) {
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

    if (!m_bitmapBackground.isNull()) {
        // for pixmaps we draw only the exposed part (untransformed since the
        // bitmapBackground already has the size of the viewport)
        painter->drawPixmap(option->exposedRect, m_bitmapBackground, option->exposedRect);
        //kDebug() << "draw pixmap of background to" << option->exposedRect;
    }

    // restore transformation and composition mode
    painter->restore();
}

K_EXPORT_PLASMA_APPLET(desktop, DefaultDesktop)

#include "desktop.moc"
