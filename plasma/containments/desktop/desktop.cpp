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

#include "plasma/appletbrowser.h"
#include "plasma/phase.h"
#include "plasma/svg.h"
#include "kworkspace/kworkspace.h"

#include "krunner_interface.h"
#include "ksmserver_interface.h"
#include "screensaver_interface.h"

#include "ui_config.h"

using namespace Plasma;

DefaultDesktop::DefaultDesktop(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_appletBrowserAction(0),
      m_runCommandAction(0),
      m_lockAction(0),
      m_logoutAction(0),
      m_configDialog(0),
      m_background(0),
      m_bitmapBackground(0),
      m_wallpaperPath(0)
{
    //kDebug() << "!!! loading desktop";
}

DefaultDesktop::~DefaultDesktop()
{
    delete m_configDialog;
}

void DefaultDesktop::init()
{
    KConfigGroup cg = config();
    m_backgroundMode = cg.readEntry("backgroundmode", int(kStaticBackground));

    m_slideShowTimer = new QTimer(this);
    connect(m_slideShowTimer, SIGNAL(timeout()), this, SLOT(nextSlide()));
    m_slideShowTimer->setInterval(cg.readEntry("slideTimer", 60) * 1000);

    m_slidePath = cg.readEntry("slidepath", KStandardDirs::kde_default("wallpaper"));

    if (m_backgroundMode == kStaticBackground) {
        m_wallpaperPath = cg.readEntry("wallpaper", KStandardDirs::locate("wallpaper", "plasma-default.png"));

        //kDebug() << "wallpaperPath is" << m_wallpaperPath << QFile::exists(m_wallpaperPath);
        if (m_wallpaperPath.isEmpty() ||
            !QFile::exists(m_wallpaperPath)) {
            //kDebug() << "SVG wallpaper!";
            m_background = new Plasma::Svg("widgets/wallpaper", this);
        }
    }
    else if (m_backgroundMode == kSlideshowBackground) {
        updateSlideList();
        m_currentSlide = 0;
        nextSlide(); // to show the first image
        m_slideShowTimer->start();
    }

    Containment::init();
}

void DefaultDesktop::updateSlideList()
{
    QDir dir(m_slidePath);
    QStringList filters;
    filters << "*.png" << "*.jpeg" << "*.jpg" << "*.svg" << "*.svgz";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden);
    QFileInfoList files = dir.entryInfoList();
    m_slideFiles.clear();

    for (int i = 0; i < files.size(); ++i) {
        m_slideFiles << files[i].absoluteFilePath();
    }

    kDebug() << "updated slide list from contents of folder: " << m_slidePath;
    kDebug() << m_slideFiles.size() << " files found.";
    if (m_currentSlide > m_slideFiles.size()) {
        m_currentSlide = 0;
    }
}

void DefaultDesktop::nextSlide()
{
    if (++m_currentSlide >= m_slideFiles.size()) {
        m_currentSlide = 0;
    }

    if (m_slideFiles.size() > 0)
    {
        m_wallpaperPath = m_slideFiles[m_currentSlide];
        kDebug() << "switching slides to: " << m_wallpaperPath;

        getBitmapBackground();
        update();
    }
}

void DefaultDesktop::getBitmapBackground()
{
    if (!m_wallpaperPath.isEmpty()) {
        const QRect geom = QApplication::desktop()->screenGeometry(screen());
        delete m_bitmapBackground;
        if (m_wallpaperPath.endsWith("svg") || m_wallpaperPath.endsWith("svgz"))
        {
            KSvgRenderer renderer(m_wallpaperPath);
            m_bitmapBackground = new QPixmap(geom.size());
            QPainter p(m_bitmapBackground);
            renderer.render(&p);
        }
        else
        {
            m_bitmapBackground = new QPixmap(m_wallpaperPath);
            // NOTE: this could change to allow Full & clipped modes, etc.
            (*m_bitmapBackground) = m_bitmapBackground->scaled(geom.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
    }
}

void DefaultDesktop::constraintsUpdated(Plasma::Constraints constraints)
{
    Q_UNUSED(constraints);
    //kDebug() << "DefaultDesktop constraints have changed";
    const QRect geom = QApplication::desktop()->screenGeometry(screen());
    if (m_background) {
        //kDebug() << "Rescaling SVG wallpaper to" << geom.size();
        m_background->resize(geom.size());
    } else if (!m_wallpaperPath.isEmpty()) {
        if (!m_bitmapBackground || !(m_bitmapBackground->size() == geom.size())) {
            getBitmapBackground();
        }
    }
}

void DefaultDesktop::configure()
{
    if (m_configDialog == 0) {
        m_configDialog = new KDialog;
        m_configDialog->setWindowIcon(KIcon("preferences-desktop-wallpaper"));
        m_configDialog->setCaption( i18n("Configure Desktop") );
        m_ui = new Ui::config;
        m_ui->setupUi(m_configDialog->mainWidget());
        m_configDialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect( m_configDialog, SIGNAL(applyClicked()), this, SLOT(applyConfig()) );
        connect( m_configDialog, SIGNAL(okClicked()), this, SLOT(applyConfig()) );
        m_ui->picRequester->comboBox()->insertItem(0, "http://tools.wikimedia.de/~daniel/potd/potd.php/commons/400x300");
        m_ui->slideShowRequester->setMode(KFile::Directory);
        m_ui->slideShowRequester->setGeometry(m_ui->picRequester->frameGeometry());
        m_ui->slideShowTime->setMinimumTime(QTime(0,0,1)); // minimum to 1 seconds

        // hide these since we don't use them yet
        m_ui->colorFrame->hide();
    }

    m_ui->pictureComboBox->setCurrentIndex(m_backgroundMode);
    m_ui->picRequester->fileDialog()->setCaption(i18n("Configure Desktop")); // TODO: change caption after string freeze; e.g. "Select Wallpaper"
    m_ui->picRequester->fileDialog()->setPreviewWidget(new KImageFilePreview(m_ui->picRequester));
    m_ui->picRequester->setUrl(m_wallpaperPath);
    m_ui->slideShowRequester->setUrl(KUrl(m_slidePath));
    int mseconds = m_slideShowTimer->interval() / 1000;
    m_ui->slideShowTime->setTime(QTime(mseconds / 3600, (mseconds / 60) % 60, mseconds % 60));
    m_configDialog->show();
}

void DefaultDesktop::applyConfig()
{
    KConfigGroup cg = config();
    m_wallpaperPath = m_ui->picRequester->url().path();
    cg.writeEntry("wallpaper", m_wallpaperPath);

    m_backgroundMode = m_ui->pictureComboBox->currentIndex();
    cg.writeEntry("backgroundmode", m_backgroundMode);

    m_slidePath = m_ui->slideShowRequester->url().path();
    cg.writeEntry("slidepath", m_slidePath);

    QTime timerTime = m_ui->slideShowTime->time();
    unsigned int mseconds = timerTime.second() + timerTime.minute() * 60 + timerTime.hour() * 3600;
    m_slideShowTimer->setInterval(mseconds * 1000);
    cg.writeEntry("slideTimer", mseconds);

    if (m_backgroundMode == kStaticBackground) {
        m_slideShowTimer->stop();
    }
    else if (m_backgroundMode == kSlideshowBackground) {
        updateSlideList();
        nextSlide();
        m_slideShowTimer->start();
    }

    getBitmapBackground();
    update();
    cg.config()->sync();
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

        m_setupDesktopAction = new QAction(i18n("Configure Desktop..."), this);
        m_setupDesktopAction->setIcon(KIcon("configure"));
        connect(m_setupDesktopAction, SIGNAL(triggered()), this, SLOT(configure()));

        m_lockAction = new QAction(i18n("Lock Screen"), this);
        m_lockAction->setIcon(KIcon("system-lock-screen"));
        connect(m_lockAction, SIGNAL(triggered(bool)), this, SLOT(lockScreen()));

        m_logoutAction = new QAction(i18n("Logout"), this);
        m_logoutAction->setIcon(KIcon("system-log-out"));
        connect(m_logoutAction, SIGNAL(triggered(bool)), this, SLOT(logout()));
    }

    QList<QAction*> actions;

    actions.append(m_appletBrowserAction);

    actions.append(m_setupDesktopAction);

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
