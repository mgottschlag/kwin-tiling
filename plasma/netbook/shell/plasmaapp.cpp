/*
 *   Copyright 2006-2008 Aaron Seigo <aseigo@kde.org>
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

#include "plasmaapp.h"

#include <unistd.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmapCache>
#include <QTimer>
#include <QVBoxLayout>
#include <QtDBus/QtDBus>

#include <KAction>
#include <KCrash>
#include <KDebug>
#include <KCmdLineArgs>
#include <KStandardAction>
#include <KWindowSystem>

#include <ksmserver_interface.h>

#include <Plasma/Containment>
#include <Plasma/Theme>

#include "netcorona.h"
#include "netview.h"

#include "appletbrowser.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

Display* dpy = 0;
Colormap colormap = 0;
Visual *visual = 0;


PlasmaApp* PlasmaApp::self()
{
    if (!kapp) {
        return new PlasmaApp();
    }

    return qobject_cast<PlasmaApp*>(kapp);
}

PlasmaApp::PlasmaApp()
    : KUniqueApplication(),
      m_corona(0),
      m_appletBrowser(0),
      m_window(0),
      m_controlBar(0),
      m_mainView(0),
      m_isDesktop(false)
{
    KGlobal::locale()->insertCatalog("libplasma");
    KCrash::setFlags(KCrash::AutoRestart);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool isDesktop = args->isSet("desktop");
    if (isDesktop) {
        notifyStartup(false);
    }

    //TODO: decide how to handle the cache size; possibilities:
    //      * % of ram, as in desktop
    //      * fixed size, hardcoded (uck)
    //      * optional size, specified on command line
    //      * optional size, in a config file
    //      * don't do anything special at all
    //QPixmapCache::setCacheLimit(cacheSize);

    KConfigGroup cg(KGlobal::config(), "General");
    Plasma::Theme::defaultTheme()->setFont(cg.readEntry("desktopFont", font()));

    //m_window = new QWidget;
    m_window = m_mainView = new NetView(0, NetView::mainViewId(), 0);
    connect(m_mainView, SIGNAL(containmentActivated()), this, SLOT(mainContainmentActivated()));
    m_window->installEventFilter(this);

    //FIXME: if argb visuals enabled Qt will always set WM_CLASS as "qt-subapplication" no matter what
    //the application name is we set the proper XClassHint here, hopefully won't be necessary anymore when
    //qapplication will manage apps with argb visuals in a better way
    XClassHint classHint;
    classHint.res_name = const_cast<char*>("Plasma");
    classHint.res_class = const_cast<char*>("Plasma");
    XSetClassHint(QX11Info::display(), m_window->winId(), &classHint);


    m_controlBar = new NetView(0, NetView::controlBarId(), 0);
    m_controlBar->show();
    KWindowSystem::setOnAllDesktops(m_controlBar->effectiveWinId(), true);
    unsigned long state = NET::Sticky | NET::StaysOnTop | NET::KeepAbove;
    KWindowSystem::setState(m_controlBar->effectiveWinId(), state);
    KWindowSystem::setType(m_controlBar->effectiveWinId(), NET::Dock);
    //m_controlBar->setWindowFlags(m_window->windowFlags() | Qt::FramelessWindowHint);


    //m_controlBar->setFixedHeight(CONTROL_BAR_HEIGHT);
    m_controlBar->setAttribute(Qt::WA_TranslucentBackground);
    m_controlBar->setAutoFillBackground(false);
    m_controlBar->viewport()->setAutoFillBackground(false);
    m_controlBar->setAttribute(Qt::WA_TranslucentBackground);
    connect(m_controlBar, SIGNAL(locationChanged(const NetView *)), this, SLOT(controlBarMoved(const NetView *)));
    connect(m_controlBar, SIGNAL(geometryChanged()), this, SLOT(positionPanel()));

    //m_layout->addWidget(m_mainView);

    int width = 400;
    int height = 200;
    if (isDesktop) {
        QRect rect = desktop()->screenGeometry(0);
        width = rect.width();
        height = rect.height();
    } else {
        QAction *action = KStandardAction::quit(qApp, SLOT(quit()), m_window);
        m_window->addAction(action);

        QString geom = args->getOption("screen");
        int x = geom.indexOf('x');

        if (x > 0)  {
            width = qMax(width, geom.left(x).toInt());
            height = qMax(height, geom.right(geom.length() - x - 1).toInt());
        }
    }

    m_window->setFixedSize(width, height);

    // this line initializes the corona.
    corona();
    setIsDesktop(isDesktop);
    reserveStruts();

    if (isDesktop) {
        notifyStartup(true);
    }

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
}

PlasmaApp::~PlasmaApp()
{
}

void PlasmaApp::cleanup()
{
    if (m_corona) {
        m_corona->saveLayout();
    }

    if (!m_mainView->containment()) {
        return;
    }

    // save the mapping of Views to Containments at the moment
    // of application exit so we can restore that when we start again.
    KConfigGroup viewIds(KGlobal::config(), "ViewIds");
    viewIds.deleteGroup();
    viewIds.writeEntry(QString::number(m_mainView->containment()->id()), NetView::mainViewId());

    viewIds.writeEntry(QString::number(m_controlBar->containment()->id()), NetView::controlBarId());

    delete m_window;
    m_window = 0;

    delete m_corona;
    m_corona = 0;

    //TODO: This manual sync() should not be necessary?
    syncConfig();
}

void PlasmaApp::syncConfig()
{
    KGlobal::config()->sync();
}

void PlasmaApp::positionPanel()
{
    //move
    //TODO: support locations
    controlBarMoved(m_mainView);
    m_controlBar->resize(m_mainView->size().width(), m_controlBar->size().height());
    //sync margins
    const QRect availableScreen = m_corona->availableScreenRegion(0).boundingRect();
    const QRect screen = m_corona->screenGeometry(0);

    int left, top, right, bottom;
    left = availableScreen.left() - screen.left();
    right = screen.right() - availableScreen.right();
    top = availableScreen.top() - screen.top();
    bottom = screen.bottom() - availableScreen.bottom();

    foreach (Plasma::Containment *containment, m_corona->containments()) {
        if (containment->formFactor() == Plasma::Planar) {
            containment->setContentsMargins(left, top, right, bottom);
        }
    }
}

void PlasmaApp::mainContainmentActivated()
{
    if (!m_isDesktop) {
        return;
    }

    const WId id = m_window->effectiveWinId();

    QWidget * activeWindow = QApplication::activeWindow();
    KWindowSystem::clearState(id, NET::KeepBelow);
    KWindowSystem::raiseWindow(id);
    if (activeWindow) {
        KWindowSystem::raiseWindow(activeWindow->effectiveWinId());
        activeWindow->setFocus();
    }
}

bool PlasmaApp::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_isDesktop) {
        return false;
    }

    if (watched == m_window && event->type() == QEvent::WindowDeactivate) {
        const WId id = m_window->effectiveWinId();
        QWidget * activeWindow = QApplication::activeWindow();

        if (!activeWindow) {
            KWindowSystem::setState(id, NET::KeepBelow);
        }
    } else if (watched == m_window && event->type() == QEvent::WindowActivate) {
        QTimer::singleShot(0, this, SLOT(maybeRaise()));
    }
    return false;
}

void PlasmaApp::maybeRaise()
{
    const WId id = m_window->effectiveWinId();

    if (!m_controlBar->canRaise()) {
        KWindowSystem::clearState(id, NET::KeepBelow);
        KWindowSystem::raiseWindow(id);
    }
}

void PlasmaApp::setIsDesktop(bool isDesktop)
{
    m_isDesktop = isDesktop;

    if (isDesktop) {
        m_window->setWindowFlags(m_window->windowFlags() | Qt::FramelessWindowHint);
        KWindowSystem::setOnAllDesktops(m_window->winId(), true);
        m_window->show();
        KWindowSystem::setState(m_window->winId(), NET::SkipTaskbar | NET::SkipPager);
        //KWindowSystem::setType(m_window->winId(), NET::Desktop);
        KWindowSystem::setState(m_window->winId(), NET::KeepBelow);
        m_window->lower();
        connect(QApplication::desktop(), SIGNAL(resized(int)), SLOT(adjustSize(int)));
    } else {
        m_window->setWindowFlags(m_window->windowFlags() & ~Qt::FramelessWindowHint);
        KWindowSystem::setOnAllDesktops(m_window->winId(), false);
        KWindowSystem::setType(m_window->winId(), NET::Normal);
        disconnect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(adjustSize(int)));
    }
}

bool PlasmaApp::isDesktop() const
{
    return m_isDesktop;
}

void PlasmaApp::adjustSize(int screen)
{
    Q_UNUSED(screen)

    QRect rect = desktop()->screenGeometry(0);

    int width = rect.width();
    int height = rect.height();
    m_window->setFixedSize(width, height);
    reserveStruts();
}

void PlasmaApp::reserveStruts()
{
    if (!isDesktop()) {
        return;
    }

    NETExtendedStrut strut;
    switch (m_controlBar->location()) {
    case Plasma::LeftEdge:
        strut.left_width = m_controlBar->width();
        strut.left_start = m_window->y();
        strut.left_end = m_window->y() + m_window->height() - 1;
        break;
    case Plasma::RightEdge:
        strut.right_width = m_controlBar->width();
        strut.right_start = m_window->y();
        strut.right_end = m_window->y() + m_window->height() - 1;
        break;
    case Plasma::TopEdge:
        strut.top_width = m_controlBar->height();
        strut.top_start = m_window->x();
        strut.top_end = m_window->x() + m_window->width() - 1;
        break;
    case Plasma::BottomEdge:
    default:
        strut.bottom_width = m_controlBar->height();
        strut.bottom_start = m_window->x();
        strut.bottom_end = m_window->x() + m_window->width() - 1;
    }

    KWindowSystem::setExtendedStrut(m_window->winId(),
                                    strut.left_width, strut.left_start, strut.left_end,
                                    strut.right_width, strut.right_start, strut.right_end,
                                    strut.top_width, strut.top_start, strut.top_end,
                                    strut.bottom_width, strut.bottom_start, strut.bottom_end);
}

Plasma::Corona* PlasmaApp::corona()
{
    if (!m_corona) {
        m_corona = new NetCorona(this, m_window);
        connect(m_corona, SIGNAL(containmentAdded(Plasma::Containment*)),
                this, SLOT(createView(Plasma::Containment*)));
        connect(m_corona, SIGNAL(configSynced()), this, SLOT(syncConfig()));


        m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
        m_corona->initializeLayout();

        m_window->show();

        connect(m_corona, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                m_mainView, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));

    }

    return m_corona;
}

bool PlasmaApp::hasComposite()
{
    return colormap && KWindowSystem::compositingActive();
}

void PlasmaApp::notifyStartup(bool completed)
{
    org::kde::KSMServerInterface ksmserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());

    const QString startupID("workspace desktop");
    if (completed) {
        ksmserver.resumeStartup(startupID);
    } else {
        ksmserver.suspendStartup(startupID);
    }
}

void PlasmaApp::createView(Plasma::Containment *containment)
{
    connect(containment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));

    KConfigGroup viewIds(KGlobal::config(), "ViewIds");
    int defaultId = 0;
    if (containment->containmentType() == Plasma::Containment::PanelContainment && 
        m_controlBar->containment() == 0 ) {
        defaultId = NetView::controlBarId();
    } else if (containment->containmentType() == Plasma::Containment::PanelContainment && 
        m_mainView->containment() == 0 ) {
        defaultId = NetView::mainViewId();
    }

    int id = viewIds.readEntry(QString::number(containment->id()), defaultId);

    kDebug() << "new containment" << (QObject*)containment << containment->id()<<"view id"<<id;

    if ((m_mainView && id == NetView::mainViewId()) ||
        (containment->containmentType() != Plasma::Containment::PanelContainment &&
         containment->containmentType() != Plasma::Containment::CustomPanelContainment &&
         !viewIds.exists() && m_mainView->containment() == 0)) {
        m_mainView->setContainment(containment);
        containment->setScreen(0);
    } else if (m_controlBar && id == NetView::controlBarId()) {
        m_controlBar->setContainment(containment);
    } else {
        containment->setScreen(-1);
    }
}

void PlasmaApp::controlBarMoved(const NetView *controlBar)
{
    if (controlBar != m_controlBar) {
        return;
    }

    //TODO: manage layouts in the new way
    switch (controlBar->location()) {
    case Plasma::LeftEdge:
        m_controlBar->move(m_mainView->geometry().topLeft());
        break;
    case Plasma::RightEdge:
        m_controlBar->move(m_mainView->geometry().bottomLeft()-QPoint(m_controlBar->size().width(), 0));
        break;
    case Plasma::TopEdge:
        m_controlBar->move(m_mainView->geometry().topLeft());
        break;
    case Plasma::BottomEdge:
        m_controlBar->move(m_mainView->geometry().bottomLeft()-QPoint(0,m_controlBar->size().height()));
    default:
        break;
    }

    reserveStruts();
}


void PlasmaApp::showAppletBrowser()
{
    Plasma::Containment *containment = dynamic_cast<Plasma::Containment *>(sender());

    if (!containment) {
        return;
    }

    showAppletBrowser(containment);
}

void PlasmaApp::showAppletBrowser(Plasma::Containment *containment)
{
    if (!containment) {
        return;
    }

    if (!m_appletBrowser) {
        m_appletBrowser = new Plasma::AppletBrowser();
        m_appletBrowser->setContainment(containment);
        m_appletBrowser->setApplication();
        m_appletBrowser->setAttribute(Qt::WA_DeleteOnClose);
        m_appletBrowser->setWindowTitle(i18n("Add Widgets"));
        m_appletBrowser->setWindowIcon(KIcon("plasmagik"));
        connect(m_appletBrowser, SIGNAL(destroyed()), this, SLOT(appletBrowserDestroyed()));
    } else {
        m_appletBrowser->setContainment(containment);
    }

    KWindowSystem::setOnDesktop(m_appletBrowser->winId(), KWindowSystem::currentDesktop());
    m_appletBrowser->show();
    KWindowSystem::activateWindow(m_appletBrowser->winId());
}

void PlasmaApp::appletBrowserDestroyed()
{
    m_appletBrowser = 0;
}

#include "plasmaapp.moc"
