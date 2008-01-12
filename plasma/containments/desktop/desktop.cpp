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

IconLoader * DefaultDesktop::s_icons = 0;

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
    if (s_icons && s_icons->parent() == this) {
        // reset the static var; the s_icons objects itself is parented to us,
        // so it'll get deleted just fine
        s_icons = 0;
    }
}

void DefaultDesktop::init()
{
    reloadConfig(true);
    Containment::init();
}

void DefaultDesktop::nextSlide(bool skipUpdates)
{
    if (++m_currentSlide >= m_slideFiles.size()) {
        m_currentSlide = 0;
    }

    if (m_slideFiles.size() > 0) {
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

void DefaultDesktop::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & StartupCompletedConstraint) {
        if (screen() == 0 && !s_icons) {
            s_icons = new IconLoader(this);
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        m_renderer.setSize(resolution()); 
        updateBackground();
    }

    if (constraints & Plasma::ImmutableConstraint && m_appletBrowserAction) {
        // we need to update the menu items that have already been created
        bool locked = isImmutable();
        m_appletBrowserAction->setVisible(!locked);
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
    s_icons->reloadConfig();
}

void DefaultDesktop::reloadConfig(bool skipUpdates)
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    KConfigGroup cg = config();

    // If no wallpaper is set, a default will be set in updateBackground()
    // which is called as soon as constraints are updated.
    m_wallpaperPath = cg.readEntry("wallpaper", QString());
    if (!m_wallpaperPath.isEmpty()) {
        kDebug() << "Using configured wallpaper" << m_wallpaperPath;
    }

    m_backgroundMode = cg.readEntry("backgroundmode", 
        (int) BackgroundDialog::kStaticBackground);

    // used in both modes, so read it no matter which mode we are in
    m_wallpaperPosition = cg.readEntry("wallpaperposition", 0);
    m_wallpaperColor = cg.readEntry("wallpapercolor", QColor(Qt::black));

    if (m_backgroundMode == BackgroundDialog::kStaticBackground ||
        m_backgroundMode == BackgroundDialog::kNoBackground) {
        m_slideshowTimer.stop();
        // Only set the wallpaper if constraints have been loaded
        if (!skipUpdates) {
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
            foreach (QFileInfo wp, files) {
                int position = m_slideFiles.size() == 0 ? 0 : qrand() % m_slideFiles.size();
                m_slideFiles.insert(position, wp.filePath());
            }

            // now make it look in sub-dirs
            dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
            QFileInfoList subdirs = dir.entryInfoList();
            foreach (QFileInfo wp, subdirs) {
                dirs.append(wp.filePath());
            }
        }

        int delay = cg.readEntry("slideTimer", 60);
        m_slideshowTimer.setInterval(delay * 1000);
        if (!m_slideshowTimer.isActive()) {
            m_slideshowTimer.start();
        }
        m_currentSlide = -1;
        nextSlide(true);
    }
}

void DefaultDesktop::updateBackground()
{
    if (m_wallpaperPath.isEmpty() && m_backgroundMode != BackgroundDialog::kNoBackground) {
        QString defaultPath = QString("EOS/contents/images/%1x%2.jpg");

        QString testPath = defaultPath.arg(geometry().width()).arg(geometry().height());
        m_wallpaperPath = KStandardDirs::locate("wallpaper", testPath);

        if (m_wallpaperPath.isEmpty()) {
            kDebug() << "Trying" << defaultPath.arg(1920).arg(1200);
            m_wallpaperPath = KStandardDirs::locate("wallpaper", defaultPath.arg(1920).arg(1200));
        }

        kDebug() << "Setting wallpaper to default" << m_wallpaperPath;
        emit configNeedsSaving();
    }

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
        m_appletBrowserAction->setIcon(KIcon("list-add"));

        m_runCommandAction = new QAction(i18n("Run Command..."), this);
        connect(m_runCommandAction, SIGNAL(triggered(bool)), this, SLOT(runCommand()));
        m_runCommandAction->setIcon(KIcon("system-run"));

        m_setupDesktopAction = new QAction(i18n("Configure Desktop..."), this);
        m_setupDesktopAction->setIcon(KIcon("configure"));
        connect(m_setupDesktopAction, SIGNAL(triggered()), this, SLOT(configure()));

        m_lockDesktopAction = new QAction(i18n("Lock Widgets"), this);
        m_lockDesktopAction->setIcon(KIcon("object-locked"));
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

        m_separator2 = new QAction(this);
        m_separator2->setSeparator(true);
    }

    QList<QAction*> actions;

    if (KAuthorized::authorizeKAction("run_command")) {
        actions.append(m_runCommandAction);
    }

    actions.append(m_appletBrowserAction);
    actions.append(m_setupDesktopAction);

    actions.append(m_separator);

    if (!isImmutable() && s_icons && s_icons->showIcons()) {
        //icon actions
        actions << s_icons->contextActions();
        actions.append(m_separator2);
    }

    actions.append(m_lockDesktopAction);

    if (KAuthorized::authorizeKAction("lock_screen")) {
        actions.append(m_lockScreenAction);
    }

    if (KAuthorized::authorizeKAction("logout")) {
        actions.append(m_logoutAction);
    }

    return actions;
}

void DefaultDesktop::toggleDesktopImmutability()
{
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
