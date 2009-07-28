/*
 *   Copyright 2006-2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include <kephal/screens.h>

#include <Plasma/Containment>
#include <Plasma/Theme>
#include <Plasma/WindowEffects>

#include "netcorona.h"
#include "netview.h"

#include "appletbrowser.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

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
      m_controlBar(0),
      m_mainView(0),
      m_isDesktop(false),
      m_autoHideControlBar(true),
      m_unHideTimer(0)
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

    m_mainView = new NetView(0, NetView::mainViewId(), 0);
    connect(m_mainView, SIGNAL(containmentActivated()), this, SLOT(mainContainmentActivated()));
    connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SLOT(positionPanel()));
    m_mainView->installEventFilter(this);

    int width = 400;
    int height = 200;
    if (isDesktop) {
        QRect rect = Kephal::ScreenUtils::screenGeometry(m_mainView->screen());
        width = rect.width();
        height = rect.height();
    } else {
        QAction *action = KStandardAction::quit(qApp, SLOT(quit()), m_mainView);
        m_mainView->addAction(action);

        QString geom = args->getOption("screen");
        int x = geom.indexOf('x');

        if (x > 0)  {
            width = qMax(width, geom.left(x).toInt());
            height = qMax(height, geom.right(geom.length() - x - 1).toInt());
        }
    }

    // this line initializes the corona.
    corona();
    setIsDesktop(isDesktop);
    reserveStruts();

    //FIXME: monstrous hack: force it non fullscreen by making it a pixel too short
    m_mainView->setFixedSize(width, height-1);
    m_mainView->move(0,0);


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

    if (m_controlBar) {
        viewIds.writeEntry(QString::number(m_controlBar->containment()->id()), NetView::controlBarId());
    }

    delete m_mainView;
    m_mainView = 0;

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
    if (!m_controlBar) {
        return;
    }

    QRect screenRect = Kephal::ScreenUtils::screenGeometry(m_controlBar->screen());

    //move
    controlBarMoved(m_controlBar);

    if (m_controlBar->formFactor() == Plasma::Horizontal) {
        m_controlBar->setFixedSize(screenRect.width(), m_controlBar->size().height());
    } else if (m_controlBar->formFactor() == Plasma::Vertical) {
        m_controlBar->setFixedSize(m_controlBar->size().width(), screenRect.height());
    }


    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;

    switch (m_controlBar->location()) {
    case Plasma::LeftEdge:
        left = m_controlBar->width();
        break;
    case Plasma::RightEdge:
        right = m_controlBar->width();
        break;
    case Plasma::TopEdge:
        top = m_controlBar->height();
        break;
    case Plasma::BottomEdge:
        bottom = m_controlBar->height();
        break;
    default:
        break;
    }

    foreach (Plasma::Containment *containment, m_corona->containments()) {
        if (containment->formFactor() == Plasma::Planar) {
            containment->setContentsMargins(left, top, right, bottom);
        }
    }

    if (m_autoHideControlBar) {
        destroyUnHideTrigger();
        createUnhideTrigger();
    }
}

void PlasmaApp::mainContainmentActivated()
{
    if (!m_isDesktop) {
        return;
    }

    const WId id = m_mainView->effectiveWinId();

    QWidget * activeWindow = QApplication::activeWindow();
    KWindowSystem::raiseWindow(id);

    if (activeWindow) {
        KWindowSystem::raiseWindow(activeWindow->effectiveWinId());
        m_mainView->activateWindow();
        activeWindow->setFocus();
    } else {
        m_mainView->activateWindow();
    }
}

void PlasmaApp::setIsDesktop(bool isDesktop)
{
    m_isDesktop = isDesktop;

    if (isDesktop) {
        m_mainView->setWindowFlags(m_mainView->windowFlags() | Qt::FramelessWindowHint);
        KWindowSystem::setOnAllDesktops(m_mainView->winId(), true);
        m_mainView->show();
        KWindowSystem::setType(m_mainView->winId(), NET::Normal);
    } else {
        m_mainView->setWindowFlags(m_mainView->windowFlags() & ~Qt::FramelessWindowHint);
        KWindowSystem::setOnAllDesktops(m_mainView->winId(), false);
        KWindowSystem::setType(m_mainView->winId(), NET::Normal);
    }
}

bool PlasmaApp::isDesktop() const
{
    return m_isDesktop;
}

void PlasmaApp::adjustSize(Kephal::Screen *screen)
{
    Q_UNUSED(screen)

    QRect rect = Kephal::ScreenUtils::screenGeometry(m_mainView->screen());

    int width = rect.width();
    int height = rect.height();
    //FIXME: ugly hack there too
    m_mainView->setFixedSize(width, height-1);
    positionPanel();
    reserveStruts();
}

void PlasmaApp::reserveStruts()
{
    if (!m_controlBar || !isDesktop()) {
        return;
    }

    NETExtendedStrut strut;

    if (!m_autoHideControlBar) {
        switch (m_controlBar->location()) {
        case Plasma::LeftEdge:
            strut.left_width = m_controlBar->width();
            strut.left_start = m_mainView->y();
            strut.left_end = m_mainView->y() + m_mainView->height() - 1;
            break;
        case Plasma::RightEdge:
            strut.right_width = m_controlBar->width();
            strut.right_start = m_mainView->y();
            strut.right_end = m_mainView->y() + m_mainView->height() - 1;
            break;
        case Plasma::TopEdge:
            strut.top_width = m_controlBar->height();
            strut.top_start = m_mainView->x();
            strut.top_end = m_mainView->x() + m_mainView->width() - 1;
            break;
        case Plasma::BottomEdge:
        default:
            strut.bottom_width = m_controlBar->height();
            strut.bottom_start = m_mainView->x();
            strut.bottom_end = m_mainView->x() + m_mainView->width() - 1;
        }
    }

    KWindowSystem::setExtendedStrut(m_mainView->winId(),
                                    strut.left_width, strut.left_start, strut.left_end,
                                    strut.right_width, strut.right_start, strut.right_end,
                                    strut.top_width, strut.top_start, strut.top_end,
                                    strut.bottom_width, strut.bottom_start, strut.bottom_end);
}

Plasma::Corona* PlasmaApp::corona()
{
    if (!m_corona) {
        m_corona = new NetCorona(this, m_mainView);
        connect(m_corona, SIGNAL(containmentAdded(Plasma::Containment*)),
                this, SLOT(createView(Plasma::Containment*)));
        connect(m_corona, SIGNAL(configSynced()), this, SLOT(syncConfig()));


        m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
        m_corona->initializeLayout();

        m_mainView->show();

        connect(m_corona, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                m_mainView, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));

    }

    return m_corona;
}

bool PlasmaApp::hasComposite()
{
    return KWindowSystem::compositingActive();
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
        (!m_controlBar || m_controlBar->containment() == 0) ) {
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
    } else if (id == NetView::controlBarId()) {
        if (!m_controlBar) {
            m_controlBar = new NetView(0, NetView::controlBarId(), 0);

            Kephal::Screens *screens = Kephal::Screens::self();
            connect(screens, SIGNAL(screenResized(Kephal::Screen *, QSize, QSize)),
                    this, SLOT(adjustSize(Kephal::Screen *)));

            m_controlBar->show();
            KWindowSystem::setOnAllDesktops(m_controlBar->effectiveWinId(), true);
            m_controlBar->setWindowFlags(m_mainView->windowFlags() | Qt::FramelessWindowHint);
            m_controlBar->setFrameShape(QFrame::NoFrame);
            unsigned long state = NET::Sticky | NET::StaysOnTop | NET::KeepAbove;
            KWindowSystem::setState(m_controlBar->effectiveWinId(), state);
            KWindowSystem::setType(m_controlBar->effectiveWinId(), NET::Dock);

            m_controlBar->show();

            m_controlBar->setAutoFillBackground(false);
            m_controlBar->viewport()->setAutoFillBackground(false);
            m_controlBar->setAttribute(Qt::WA_TranslucentBackground);

            connect(m_controlBar, SIGNAL(locationChanged(const NetView *)), this, SLOT(controlBarMoved(const NetView *)));
            connect(m_controlBar, SIGNAL(geometryChanged()), this, SLOT(positionPanel()));
        }

        m_controlBar->setContainment(containment);

        m_autoHideControlBar = m_controlBar->config().readEntry("panelAutoHide", true);

        setAutoHideControlBar(m_autoHideControlBar);
    } else {
        containment->setScreen(-1);
    }
}

void PlasmaApp::controlBarMoved(const NetView *controlBar)
{
    if (!m_controlBar || controlBar != m_controlBar) {
        return;
    }

    QRect screenRect = Kephal::ScreenUtils::screenGeometry(m_controlBar->screen());

    switch (controlBar->location()) {
    case Plasma::LeftEdge:
        m_controlBar->move(screenRect.topLeft());
        break;
    case Plasma::RightEdge:
        m_controlBar->move(screenRect.bottomLeft()-QPoint(m_controlBar->size().width(), 0));
        break;
    case Plasma::TopEdge:
        m_controlBar->move(screenRect.topLeft());
        break;
    case Plasma::BottomEdge:
        m_controlBar->move(screenRect.bottomLeft()-QPoint(0,m_controlBar->size().height()));
    default:
        break;
    }

    reserveStruts();
}

void PlasmaApp::setAutoHideControlBar(bool autoHide)
{
    if (!m_controlBar) {
        return;
    }

    if (autoHide) {
        createUnhideTrigger();
        m_controlBar->hide();
        m_controlBar->installEventFilter(this);
        m_unHideTimer = new QTimer(this);
        m_unHideTimer->setSingleShot(true);
        connect(m_unHideTimer, SIGNAL(timeout()), this, SLOT(controlBarVisibilityUpdate()));
    } else {
        destroyUnHideTrigger();
        delete m_unHideTimer;
        m_unHideTimer = 0;
        m_controlBar->show();
        m_controlBar->removeEventFilter(this);
    }

    reserveStruts();
    m_controlBar->config().writeEntry("panelAutoHide", autoHide);
    m_autoHideControlBar = autoHide;
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

bool PlasmaApp::eventFilter(QObject * watched, QEvent *event)
{
    if (watched == m_mainView && event->type() == QEvent::WindowActivate) {
        destroyUnHideTrigger();
        Plasma::WindowEffects::slideWindow(m_controlBar, m_controlBar->location());
        m_controlBar->show();
    } else if ((watched == m_mainView &&
                event->type() == QEvent::WindowDeactivate &&
                !QApplication::activeWindow()) ||
               (watched == m_controlBar &&
                event->type() == QEvent::Leave &&
                !QApplication::activeWindow())) {
        //delayed hide
        m_unHideTimer->start(600);
    }
    return false;
}

bool PlasmaApp::x11EventFilter(XEvent *event)
{
    if (m_controlBar && m_autoHideControlBar && !m_controlBar->isVisible() && event->xcrossing.window == m_unhideTrigger &&
        (event->xany.send_event != True && event->type == EnterNotify)) {
        //delayed show
        m_unHideTimer->start(600);
    }
    return KUniqueApplication::x11EventFilter(event);
}

void PlasmaApp::controlBarVisibilityUpdate()
{
    //FIXME: QCursor::pos() can be avoided somewat? the good news is that is quite rare, one time per trigger
    if (!m_controlBar->isVisible()) {
        if (m_unhideTriggerGeom.adjusted(-1, -1, 1, 1).contains(QCursor::pos())) {
            destroyUnHideTrigger();
            Plasma::WindowEffects::slideWindow(m_controlBar, m_controlBar->location());
            m_controlBar->show();
        }
    } else {
        createUnhideTrigger();
        Plasma::WindowEffects::slideWindow(m_controlBar, m_controlBar->location());
        m_controlBar->hide();
    }
}

void PlasmaApp::createUnhideTrigger()
{
#ifdef Q_WS_X11
    //kDebug() << m_unhideTrigger << None;
    if (!m_autoHideControlBar || m_unhideTrigger != None) {
        return;
    }

    int actualWidth = 1;
    int actualHeight = 1;
    int triggerWidth = 1;
    int triggerHeight = 1;

    QPoint actualTriggerPoint = m_controlBar->pos();
    QPoint triggerPoint = m_controlBar->pos();

    switch (m_controlBar->location()) {
        case Plasma::TopEdge:
            actualWidth = triggerWidth = m_controlBar->width();

            break;
        case Plasma::BottomEdge:
            actualWidth = triggerWidth = m_controlBar->width();
            actualTriggerPoint = triggerPoint = m_controlBar->geometry().bottomLeft();

            break;
        case Plasma::RightEdge:
            actualHeight = triggerHeight = m_controlBar->height();
            actualTriggerPoint = triggerPoint = m_controlBar->geometry().topRight();

            break;
        case Plasma::LeftEdge:
            actualHeight = triggerHeight = m_controlBar->height();

            break;
        default:
            // no hiding unless we're on an edge.
            return;
            break;
    }


    XSetWindowAttributes attributes;
    attributes.override_redirect = True;
    attributes.event_mask = EnterWindowMask;


    attributes.event_mask = EnterWindowMask | LeaveWindowMask | PointerMotionMask |
                            KeyPressMask | KeyPressMask | ButtonPressMask |
                            ButtonReleaseMask | ButtonMotionMask |
                            KeymapStateMask | VisibilityChangeMask |
                            StructureNotifyMask | ResizeRedirectMask |
                            SubstructureNotifyMask |
                            SubstructureRedirectMask | FocusChangeMask |
                            PropertyChangeMask | ColormapChangeMask | OwnerGrabButtonMask;

    unsigned long valuemask = CWOverrideRedirect | CWEventMask;
    m_unhideTrigger = XCreateWindow(QX11Info::display(), QX11Info::appRootWindow(),
                                    triggerPoint.x(), triggerPoint.y(), triggerWidth, triggerHeight,
                                    0, CopyFromParent, InputOnly, CopyFromParent,
                                    valuemask, &attributes);

    XMapWindow(QX11Info::display(), m_unhideTrigger);
    m_unhideTriggerGeom = QRect(triggerPoint, QSize(triggerWidth, triggerHeight));
    m_triggerZone = QRect(actualTriggerPoint, QSize(actualWidth, actualHeight));
#endif
}

void PlasmaApp::destroyUnHideTrigger()
{
#ifdef Q_WS_X11
    if (m_unhideTrigger != None) {
        XDestroyWindow(QX11Info::display(), m_unhideTrigger);
        m_unhideTrigger = None;
        m_triggerZone = m_unhideTriggerGeom = QRect();
    }
#endif
}

#include "plasmaapp.moc"
