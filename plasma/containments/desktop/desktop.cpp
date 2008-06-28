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
#include "plasma/animator.h"
#include "plasma/theme.h"
#include "kworkspace/kworkspace.h"
#include "knewstuff2/engine.h"

#include "krunner_interface.h"
#include "screensaver_interface.h"
#include "ksmserver_interface.h"

#include "backgrounddialog.h"

using namespace Plasma;

DefaultDesktop::DefaultDesktop(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_lockDesktopAction(0),
      m_appletBrowserAction(0),
      m_addPanelAction(0),
      m_runCommandAction(0),
      m_lockScreenAction(0),
      m_logoutAction(0),
      m_configDialog(0),
      m_wallpaperPath(0),
      m_wallpaperPosition(0),
      m_renderer(resolution(), 1.0),
      m_rendererToken(-1)
{
    qRegisterMetaType<QImage>("QImage");
    qRegisterMetaType<QPersistentModelIndex>("QPersistentModelIndex");
    connect(&m_renderer, SIGNAL(done(int, QImage)),
            this, SLOT(updateBackground(int, QImage)));
    connect(&m_slideshowTimer, SIGNAL(timeout()),
            this, SLOT(nextSlide()));
    //kDebug() << "!!! loading desktop";
}

DefaultDesktop::~DefaultDesktop()
{
    delete m_configDialog;
}

void DefaultDesktop::nextSlide(bool skipUpdates)
{
    if (++m_currentSlide >= m_slideFiles.size()) {
        m_currentSlide = 0;
    }

    if (m_slideFiles.size() > 0) {
        // do not change to the same background (same path)
        if (m_wallpaperPath == m_slideFiles[m_currentSlide]) {
            if (m_slideFiles.size() == 1) {
                return;
            }
            // try next one, they can't be the same (at least the same path)
            if (++m_currentSlide >= m_slideFiles.size()) {
                m_currentSlide = 0;
            }
        }

        m_wallpaperPath = m_slideFiles[m_currentSlide];
        if (!skipUpdates) {
            updateBackground();
        }
    }
}

QSize DefaultDesktop::resolution() const
{
    return QApplication::desktop()->screenGeometry(screen()).size();
}

void DefaultDesktop::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::StartupCompletedConstraint) {
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        reloadConfig();
    }

    if (constraints & Plasma::SizeConstraint) {
        m_renderer.setSize(size().toSize()); 

        if (m_rendererToken != -1) {
            // if the renderer token is still -1, then we haven't actually started up yet
            // and there is no point in touching the renderer at this point
            updateBackground();
        }
    }

    if (constraints & Plasma::ImmutableConstraint && m_appletBrowserAction) {
        // we need to update the menu items that have already been created
        bool locked = immutability() != Mutable;
        m_addPanelAction->setVisible(!locked);
    }
}

void DefaultDesktop::configure()
{
    KConfigGroup cg = config();
    KConfigGroup gcg = globalConfig();
    if (m_configDialog == 0) {
        const QSize resolution = 
            QApplication::desktop()->screenGeometry(screen()).size();
        m_configDialog = new BackgroundDialog(resolution, cg, gcg, 0);
        connect(m_configDialog, SIGNAL(okClicked()), 
                this, SLOT(applyConfig()));
        connect(m_configDialog, SIGNAL(applyClicked()), 
                this, SLOT(applyConfig()));
    }
    else {
        m_configDialog->reloadConfig(cg, gcg);
    }

    m_configDialog->show();
    KWindowSystem::setOnDesktop(m_configDialog->winId(), KWindowSystem::currentDesktop());
    KWindowSystem::activateWindow(m_configDialog->winId());
}

void DefaultDesktop::applyConfig()
{
    Q_ASSERT(m_configDialog);
    m_configDialog->saveConfig(config(), globalConfig());
    emit configNeedsSaving();

    reloadConfig();
}

void DefaultDesktop::reloadConfig()
{
    KConfigGroup cg = config();

    // store the state of the existing wallpaper config so we can determine later
    // if we need to trigger updates
    QString oldWallpaperPath(m_wallpaperPath);
    QColor old_wallpaperColor = m_wallpaperColor;
    int old_wallPaperPosition = m_wallpaperPosition;

    // If no wallpaper is set, a default will be set in updateBackground()
    // which is called as soon as constraints are updated.
    m_wallpaperPath = cg.readEntry("wallpaper", QString());
    m_backgroundMode = cg.readEntry("backgroundmode", int(BackgroundDialog::kStaticBackground));

    if (m_backgroundMode != BackgroundDialog::kNoBackground &&
        (m_wallpaperPath.isEmpty() || !KStandardDirs::exists(m_wallpaperPath)))  {
        m_wallpaperPath = Plasma::Theme::defaultTheme()->wallpaperPath(size().toSize());
        cg.writeEntry("wallpaper", m_wallpaperPath);
    }

    if (!m_wallpaperPath.isEmpty()) {
        kDebug() << "Using configured wallpaper" << m_wallpaperPath;
    }

    // used in both modes, so read it no matter which mode we are in
    m_wallpaperPosition = cg.readEntry("wallpaperposition", int(Background::ScaleCrop));
    m_wallpaperColor = cg.readEntry("wallpapercolor", QColor(Qt::black));

    if (m_backgroundMode == BackgroundDialog::kStaticBackground ||
        m_backgroundMode == BackgroundDialog::kNoBackground) {
        m_slideshowTimer.stop();
        // Only set the wallpaper if constraints have been loaded
        // and background image has changed
        if (oldWallpaperPath != m_wallpaperPath ||
            m_wallpaperPosition != old_wallPaperPosition ||
            m_wallpaperColor != old_wallpaperColor) {
            updateBackground();
        }
    } else {
        QStringList dirs = cg.readEntry("slidepaths", QStringList());
        QStringList filters;
        filters << "*.png" << "*.jpeg" << "*.jpg" << "*.svg" << "*.svgz";

        m_slideFiles.clear();

        for (int i = 0; i < dirs.size(); ++i) {
            QString path = dirs[i];
            // TODO load packages, too
            QDir dir(path);
            dir.setNameFilters(filters);
            dir.setFilter(QDir::Files | QDir::Hidden);

            QFileInfoList files = dir.entryInfoList();
            foreach (const QFileInfo &wp, files) {
                int position = m_slideFiles.size() == 0 ? 0 : qrand() % m_slideFiles.size();
                m_slideFiles.insert(position, wp.filePath());
            }

            // now make it look in sub-dirs
            dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
            QFileInfoList subdirs = dir.entryInfoList();
            foreach (const QFileInfo &wp, subdirs) {
                dirs.append(wp.filePath());
            }
        }

        int delay = cg.readEntry("slideTimer", 60);
        m_slideshowTimer.setInterval(delay * 1000);
        if (!m_slideshowTimer.isActive()) {
            m_slideshowTimer.start();
        }
        m_currentSlide = -1;
        nextSlide(false);
    }
}

void DefaultDesktop::updateBackground()
{
    if (m_wallpaperPath.isEmpty() && m_backgroundMode != BackgroundDialog::kNoBackground) {
        m_wallpaperPath = Plasma::Theme::defaultTheme()->wallpaperPath(size().toSize());
        kDebug() << "Setting wallpaper to default" << m_wallpaperPath;
        emit configNeedsSaving();
    }

    m_rendererToken = 
        m_renderer.render(m_wallpaperPath,
                          m_wallpaperColor,
                          (Background::ResizeMethod)m_wallpaperPosition,
                          Qt::SmoothTransformation);                       
    suspendStartup( true ); // during KDE startup, make ksmserver until the wallpaper is ready
}

void DefaultDesktop::updateBackground(int token, const QImage &img)
{
    if (m_rendererToken == token) {
        m_bitmapBackground = QPixmap::fromImage(img);
        update();
        suspendStartup( false );
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
    org::kde::krunner::Interface krunner(interface, "/Interface",
                                         QDBusConnection::sessionBus());
    if (krunner.isValid()) {
        krunner.display();
    }
}

void DefaultDesktop::suspendStartup(bool suspend)
{
    org::kde::KSMServerInterface ksmserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());
    const QString startupID("desktop wallaper");
    if (suspend) {
        ksmserver.suspendStartup(startupID);
    } else {
        ksmserver.resumeStartup(startupID);
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
    KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmDefault,
                                KWorkSpace::ShutdownTypeDefault,
                                KWorkSpace::ShutdownModeDefault);
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

    // for pixmaps we draw only the exposed part (untransformed since the
    // bitmapBackground already has the size of the viewport)
    painter->drawPixmap(option->exposedRect, m_bitmapBackground, option->exposedRect);
    //kDebug() << "draw pixmap of background to" << option->exposedRect;

    // restore transformation and composition mode
    painter->restore();
}

K_EXPORT_PLASMA_APPLET(desktop, DefaultDesktop)

#include "desktop.moc"
