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
#include <QtDBus/QtDBus>

#include <KCrash>
#include <KDebug>
#include <KCmdLineArgs>
#include <KStandardAction>
#include <KWindowSystem>
#include <KAction>

#include <ksmserver_interface.h>

#include <kephal/screens.h>

#include <Plasma/Containment>
#include <Plasma/Dialog>
#include <Plasma/Theme>
#include <Plasma/Wallpaper>
#include <Plasma/WindowEffects>

#include "netcorona.h"
#include "netview.h"

#include "widgetsexplorer/widgetexplorer.h"
#include "plasmagenericshell/backgrounddialog.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/shape.h>
#endif

PlasmaApp* PlasmaApp::self()
{
    if (!kapp) {
        return new PlasmaApp();
    }

    return qobject_cast<PlasmaApp*>(kapp);
}


class GlowBar : public QWidget
{
public:
    GlowBar(Plasma::Direction direction, const QRect &triggerZone)
        : QWidget(0),
          m_strength(0.3),
          m_svg(new Plasma::Svg(this)),
          m_direction(direction)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        KWindowSystem::setOnAllDesktops(winId(), true);
        unsigned long state = NET::Sticky | NET::StaysOnTop | NET::KeepAbove;
        KWindowSystem::setState(winId(), state);
        KWindowSystem::setType(winId(), NET::Dock);
        m_svg->setImagePath("widgets/glowbar");

#ifdef Q_WS_X11
        QRegion region(QRect(0,0,1,1));
        XShapeCombineRegion(QX11Info::display(), winId(), ShapeInput, 0, 0,
                            region.handle(), ShapeSet);
#endif

        QPalette pal = palette();
        pal.setColor(backgroundRole(), Qt::transparent);
        setPalette(pal);

        QRect glowGeom = triggerZone;
        QSize s = sizeHint();
        switch (m_direction) {
            case Plasma::Up:
                glowGeom.setY(glowGeom.y() - s.height() + 1);
                // fallthrough
            case Plasma::Down:
                glowGeom.setHeight(s.height());
                break;
            case Plasma::Left:
                glowGeom.setX(glowGeom.x() - s.width() + 1);
                // fallthrough
            case Plasma::Right:
                glowGeom.setWidth(s.width());
                break;
        }

        //kDebug() << "glow geom is" << glowGeom << "from" << triggerZone;
        setGeometry(glowGeom);
        m_buffer = QPixmap(size());
    }

    void paintEvent(QPaintEvent* e)
    {
        Q_UNUSED(e)
        QPixmap pixmap;
        const QSize glowRadius = m_svg->elementSize("hint-glow-radius");
        QPoint pixmapPosition(0, 0);

        m_buffer.fill(QColor(0, 0, 0, int(qreal(255)*m_strength)));
        QPainter p(&m_buffer);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);

        switch (m_direction) {
            case Plasma::Down:
                pixmap = m_svg->pixmap("bottom");
                pixmapPosition = QPoint(0, -glowRadius.height());
                break;
            case Plasma::Up:
                pixmap = m_svg->pixmap("top");
                break;
            case Plasma::Right:
                pixmap = m_svg->pixmap("right");
                pixmapPosition = QPoint(-glowRadius.width(), 0);
                break;
            case Plasma::Left:
                pixmap = m_svg->pixmap("left");
                break;
        }

        if (m_direction == Plasma::Left || m_direction == Plasma::Right) {
            p.drawTiledPixmap(QRect(0, pixmapPosition.x(), pixmap.width(), height()), pixmap);
        } else {
            p.drawTiledPixmap(QRect(0, pixmapPosition.y(), width(), pixmap.height()), pixmap);
        }

        p.end();
        p.begin(this);
        p.drawPixmap(QPoint(0, 0), m_buffer);
    }

    QSize sizeHint() const
    {
        return m_svg->elementSize("bottomright") - m_svg->elementSize("hint-glow-radius");
    }

    bool event(QEvent *event)
    {
        if (event->type() == QEvent::Paint) {
            QPainter p(this);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.fillRect(rect(), Qt::transparent);
        }
        return QWidget::event(event);
    }

    void updateStrength(QPoint point)
    {
        QPoint localPoint = mapFromGlobal(point);

        qreal newStrength;
        switch (m_direction) {
        case Plasma::Up: // when the panel is at the bottom.
            newStrength = 1 - qreal(-localPoint.y())/m_triggerDistance;
            break;
        case Plasma::Right:
            newStrength = 1 - qreal(localPoint.x())/m_triggerDistance;
            break;
        case Plasma::Left: // when the panel is right-aligned
            newStrength = 1 - qreal(-localPoint.x())/m_triggerDistance;
            break;
        case Plasma::Down:
        default:
            newStrength = 1- qreal(localPoint.y())/m_triggerDistance;
            break;
        }
        if (qAbs(newStrength - m_strength) > 0.01 && newStrength >= 0 && newStrength <= 1) {
            m_strength = newStrength;
            update();
        }
    }


private:
    static const int m_triggerDistance = 30;
    qreal m_strength;
    Plasma::Svg *m_svg;
    Plasma::Direction m_direction;
    QPixmap m_buffer;
};


class ShadowWindow : public QWidget
{
public:
    ShadowWindow(NetView *panel)
       : QWidget(0),
         m_panel(panel),
         m_valid(false)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground, false);
        setAutoFillBackground(false);
#ifdef Q_WS_X11
        QRegion region(QRect(0,0,1,1));
        XShapeCombineRegion(QX11Info::display(), winId(), ShapeInput, 0, 0,
                            region.handle(), ShapeSet);
#endif

        m_shadow = new Plasma::FrameSvg(this);
    }

    void setSvg(const QString &path)
    {
        m_shadow->setImagePath(path);

        if (!m_shadow->hasElementPrefix("shadow")) {
            hide();
            m_valid = false;
        } else {
            m_valid = true;
        }

        m_shadow->setElementPrefix("shadow");

        adjustMargins(geometry());
    }

    bool isValid() const
    {
        return m_valid;
    }

    void adjustMargins(const QRect &geo)
    {
        QRect screenRect = Kephal::ScreenUtils::screenGeometry(m_panel->screen());

        Plasma::FrameSvg::EnabledBorders enabledBorders = Plasma::FrameSvg::AllBorders;

        if (geo.left() <= screenRect.left()) {
            enabledBorders ^= Plasma::FrameSvg::LeftBorder;
        }
        if (geo.top() <= screenRect.top()) {
            enabledBorders ^= Plasma::FrameSvg::TopBorder;
        }
        if (geo.bottom() >= screenRect.bottom()) {
            enabledBorders ^= Plasma::FrameSvg::BottomBorder;
        }
        if (geo.right() >= screenRect.right()) {
            enabledBorders ^= Plasma::FrameSvg::RightBorder;
        }

        m_shadow->setEnabledBorders(enabledBorders);

        qreal left, top, right, bottom;

        m_shadow->getMargins(left, top, right, bottom);
        setContentsMargins(left, top, right, bottom);
    }

protected:
    bool event(QEvent *event)
    {
        Q_UNUSED(event)

        if (event->type() == QEvent::Paint) {
            QPainter p(this);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.fillRect(rect(), Qt::transparent);
        }
        return QWidget::event(event);
    }

    void resizeEvent(QResizeEvent *event)
    {
        m_shadow->resizeFrame(event->size());

        adjustMargins(geometry());
    }

    void paintEvent(QPaintEvent* e)
    {
        Q_UNUSED(e)

        QPainter p(this);
        //p.setCompositionMode(QPainter::CompositionMode_Source);
        m_shadow->paintFrame(&p);
    }

private:
    Plasma::FrameSvg *m_shadow;
    NetView *m_panel;
    bool m_valid;
};

PlasmaApp::PlasmaApp()
    : KUniqueApplication(),
      m_corona(0),
      m_widgetExplorerView(0),
      m_widgetExplorer(0),
      m_glowBar(0),
      m_mousePollTimer(0),
      m_controlBar(0),
      m_mainView(0),
      m_isDesktop(false),
      m_autoHideControlBar(true),
      m_unHideTimer(0),
      m_shadowWindow(0),
      m_startupSuspendWaitCount(0)
{
    PlasmaApp::suspendStartup(true);
    KGlobal::locale()->insertCatalog("libplasma");
    KGlobal::locale()->insertCatalog("plasmagenericshell");


    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool isDesktop = args->isSet("desktop");
    if (isDesktop) {
        KCrash::setFlags(KCrash::AutoRestart);
    }

    //TODO: decide how to handle the cache size; possibilities:
    //      * % of ram, as in desktop
    //      * fixed size, hardcoded (uck)
    //      * optional size, specified on command line
    //      * optional size, in a config file
    //      * don't do anything special at all
    //QPixmapCache::setCacheLimit(cacheSize);

    KConfigGroup cg(KSharedConfig::openConfig("plasmarc"), "Theme-plasma-netbook");
    const QString themeName = cg.readEntry("name", "air-netbook");
    Plasma::Theme::defaultTheme()->setUseGlobalSettings(false);
    Plasma::Theme::defaultTheme()->setThemeName(themeName);

    cg = KConfigGroup(KGlobal::config(), "General");

    Plasma::Theme::defaultTheme()->setFont(cg.readEntry("desktopFont", font()));

    m_mainView = new NetView(0, NetView::mainViewId(), 0);
    m_mainView->hide();

    connect(m_mainView, SIGNAL(containmentActivated()), this, SLOT(mainContainmentActivated()));
    connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SLOT(positionPanel()));

    bool useGL = args->isSet("opengl");
    m_mainView->setUseGL(useGL);

    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(controlBarVisibilityUpdate()));

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

    m_mainView->setFixedSize(width, height);
    m_mainView->move(0,0);
    setIsDesktop(isDesktop);

    // this line initializes the corona.
    corona();
    //setIsDesktop(isDesktop);
    reserveStruts();

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(checkShadow()));
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
    if (!m_isDesktop) {
        screenRect = m_mainView->geometry();
    }

    //move
    controlBarMoved(m_controlBar);

    if (m_controlBar->formFactor() == Plasma::Horizontal) {
        m_controlBar->setFixedSize(screenRect.width(), m_controlBar->size().height());
    } else if (m_controlBar->formFactor() == Plasma::Vertical) {
        m_controlBar->setFixedSize(m_controlBar->size().width(), screenRect.height());
    }


    m_controlBar->containment()->setMaximumSize(m_controlBar->size());
    m_controlBar->containment()->setMinimumSize(m_controlBar->size());

    if (m_autoHideControlBar && m_controlBar->isVisible()) {
        destroyUnHideTrigger();
        createUnhideTrigger();
    }

    checkShadow();

    emit controlBarChanged();
}

void PlasmaApp::checkShadow()
{
    if (!m_controlBar) {
        return;
    }

    if (KWindowSystem::compositingActive() && m_controlBar->containment()->property("shadowPath").isValid()) {
        if (!m_shadowWindow) {
            m_shadowWindow = new ShadowWindow(m_controlBar);
            KWindowSystem::setOnAllDesktops(m_controlBar->winId(), true);
        }
        KWindowSystem::setType(m_shadowWindow->winId(), NET::Dock);
        KWindowSystem::setState(m_shadowWindow->winId(), NET::KeepBelow);
        KWindowSystem::setOnAllDesktops(m_shadowWindow->winId(), true);
        m_shadowWindow->setSvg(m_controlBar->containment()->property("shadowPath").toString());
        int left, right, top, bottom;
        m_shadowWindow->adjustMargins(m_controlBar->geometry());
        m_shadowWindow->getContentsMargins(&left, &top, &right, &bottom);
        m_shadowWindow->setMinimumSize(-1, -1);
        m_shadowWindow->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        m_shadowWindow->setGeometry(m_controlBar->geometry().adjusted(-left, -top, right, bottom));
        m_shadowWindow->setFixedSize(m_shadowWindow->size());
        if (m_shadowWindow->isValid()) {
            m_shadowWindow->show();
        }
    } else {
        m_shadowWindow->deleteLater();
        m_shadowWindow = 0;
    }
}

void PlasmaApp::mainContainmentActivated()
{
    if (m_mainView->containment()) {
        m_mainView->setWindowTitle(m_mainView->containment()->activity());
    }


    const WId id = m_mainView->effectiveWinId();

    QWidget * activeWindow = QApplication::activeWindow();
    KWindowSystem::raiseWindow(id);

    if (activeWindow) {
        KWindowSystem::raiseWindow(activeWindow->effectiveWinId());
        m_mainView->activateWindow();
        activeWindow->setFocus();
        if (m_shadowWindow) {
            KWindowSystem::clearState(m_shadowWindow->winId(), NET::KeepBelow);
            KWindowSystem::setState(m_shadowWindow->winId(), NET::KeepAbove);
        }
    } else {
        m_mainView->activateWindow();
    }
}

void PlasmaApp::setIsDesktop(bool isDesktop)
{
    m_isDesktop = isDesktop;

    if (isDesktop) {
        KWindowSystem::setType(m_mainView->winId(), NET::Normal);
        m_mainView->setWindowFlags(m_mainView->windowFlags() | Qt::FramelessWindowHint);
        KWindowSystem::setOnAllDesktops(m_mainView->winId(), true);
        if (m_controlBar) {
            KWindowSystem::setOnAllDesktops(m_controlBar->winId(), true);
        }
        m_mainView->show();
    } else {
        m_mainView->setWindowFlags(m_mainView->windowFlags() & ~Qt::FramelessWindowHint);
        KWindowSystem::setOnAllDesktops(m_mainView->winId(), false);
        if (m_controlBar) {
            KWindowSystem::setOnAllDesktops(m_controlBar->winId(), false);
        }
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
    m_mainView->setFixedSize(width, height);
    positionPanel();
    reserveStruts();
}

void PlasmaApp::reserveStruts()
{
    if (!m_controlBar) {
        return;
    }

    if (m_autoHideControlBar || !isDesktop()) {
        KWindowSystem::setExtendedStrut(m_controlBar->winId(),
                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0);
        return;
    }

    NETExtendedStrut strut;

    if (!m_autoHideControlBar || hasForegroundWindows()) {
        switch (m_controlBar->location()) {
        case Plasma::LeftEdge:
            strut.left_width = m_controlBar->width();
            strut.left_start = m_controlBar->y();
            strut.left_end = m_controlBar->y() + m_controlBar->height() - 1;
            break;
        case Plasma::RightEdge:
            strut.right_width = m_controlBar->width();
            strut.right_start = m_controlBar->y();
            strut.right_end = m_controlBar->y() + m_controlBar->height() - 1;
            break;
        case Plasma::TopEdge:
            strut.top_width = m_controlBar->height();
            strut.top_start = m_controlBar->x();
            strut.top_end = m_controlBar->x() + m_controlBar->width() - 1;
            break;
        case Plasma::BottomEdge:
        default:
            strut.bottom_width = m_controlBar->height();
            strut.bottom_start = m_controlBar->x();
            strut.bottom_end = m_controlBar->x() + m_controlBar->width() - 1;
        }
    }

    KWindowSystem::setExtendedStrut(m_controlBar->winId(),
                                    strut.left_width, strut.left_start, strut.left_end,
                                    strut.right_width, strut.right_start, strut.right_end,
                                    strut.top_width, strut.top_start, strut.top_end,
                                    strut.bottom_width, strut.bottom_start, strut.bottom_end);

    //ensure the main view is at the proper position too
    QRect screenRect = Kephal::ScreenUtils::screenGeometry(m_controlBar->screen());
    m_mainView->move(screenRect.topLeft());
}

NetView *PlasmaApp::controlBar() const
{
    return m_controlBar;
}

NetView *PlasmaApp::mainView() const
{
    return m_mainView;
}

QWidget *PlasmaApp::widgetExplorer() const
{
    return m_widgetExplorerView;
}

Plasma::Corona* PlasmaApp::corona()
{
    if (!m_corona) {
        m_corona = new NetCorona(this);
        connect(m_corona, SIGNAL(containmentAdded(Plasma::Containment*)),
                this, SLOT(createView(Plasma::Containment*)));
        connect(m_corona, SIGNAL(configSynced()), this, SLOT(syncConfig()));

        connect(m_corona, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                m_mainView, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));

        m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
        m_corona->initializeLayout();
        m_corona->processUpdateScripts();

        m_mainView->show();
    }

    foreach (Plasma::Containment *containment, m_corona->containments()) {
        if (containment->screen() != -1 && containment->wallpaper()) {
            ++m_startupSuspendWaitCount;
            connect(containment->wallpaper(), SIGNAL(update(QRectF)), this, SLOT(wallpaperCheckedIn()));
        }
    }

    QTimer::singleShot(5000, this, SLOT(wallpaperCheckInTimeout()));

    return m_corona;
}

void PlasmaApp::wallpaperCheckInTimeout()
{
    if (m_startupSuspendWaitCount > 0) {
        m_startupSuspendWaitCount = 0;
        suspendStartup(false);
    }
}

void PlasmaApp::wallpaperCheckedIn()
{
    if (m_startupSuspendWaitCount < 1) {
        return;
    }

    --m_startupSuspendWaitCount;
    if (m_startupSuspendWaitCount < 1) {
        m_startupSuspendWaitCount = 0;
        suspendStartup(false);
    }
}

bool PlasmaApp::hasComposite()
{
    return KWindowSystem::compositingActive();
}

void PlasmaApp::suspendStartup(bool suspend)
{
    org::kde::KSMServerInterface ksmserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());

    const QString startupID("netbook desktop");
    if (suspend) {
        ksmserver.suspendStartup(startupID);
    } else {
        ksmserver.resumeStartup(startupID);
    }
}

void PlasmaApp::createView(Plasma::Containment *containment)
{
    connect(containment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetExplorer()));
    connect(containment, SIGNAL(configureRequested(Plasma::Containment*)),
            this, SLOT(configureContainment(Plasma::Containment*)));
    connect(containment, SIGNAL(toolBoxVisibilityChanged(bool)),
            this, SLOT(updateToolBoxVisibility(bool)));


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


    kDebug() << "new containment" << (QObject*)containment << containment->id() << "view id" << id;

    //is it a desktop -and- is it active?
    if ((m_mainView && id == NetView::mainViewId()) ||
        (containment->containmentType() != Plasma::Containment::PanelContainment &&
         containment->containmentType() != Plasma::Containment::CustomPanelContainment &&
         !viewIds.exists() && containment->screen() == 0)) {
        m_mainView->setContainment(containment);
        containment->setScreen(0);
    //is it a panel?
    } else if (id == NetView::controlBarId()) {
        if (!m_controlBar) {
            m_controlBar = new NetView(0, NetView::controlBarId(), 0);

            Kephal::Screens *screens = Kephal::Screens::self();
            connect(screens, SIGNAL(screenResized(Kephal::Screen*,QSize,QSize)),
                    this, SLOT(adjustSize(Kephal::Screen*)));

            m_controlBar->setAutoFillBackground(false);
            m_controlBar->viewport()->setAutoFillBackground(false);
            m_controlBar->setAttribute(Qt::WA_TranslucentBackground);

            connect(m_controlBar, SIGNAL(locationChanged(const NetView*)), this, SLOT(positionPanel()));
            connect(m_controlBar, SIGNAL(geometryChanged()), this, SLOT(positionPanel()));
            connect(m_controlBar, SIGNAL(containmentActivated()), this, SLOT(showControlBar()));
            connect(m_controlBar, SIGNAL(autoHideChanged(bool)), this, SLOT(setAutoHideControlBar(bool)));
        }

        m_controlBar->setContainment(containment);
        positionPanel();
        setControlBarVisible(true);
        containment->setMaximumSize(m_controlBar->size());
        containment->setMinimumSize(m_controlBar->size());
        containment->setImmutability(Plasma::UserImmutable);

        m_autoHideControlBar = m_controlBar->config().readEntry("panelAutoHide", true);

        setAutoHideControlBar(m_autoHideControlBar);
        emit controlBarChanged();
        setControlBarVisible(true);
    } else {
        containment->setScreen(-1);
    }
}

void PlasmaApp::closeWidgetExplorer()
{
    if (m_widgetExplorer) {
        Plasma::WindowEffects::slideWindow(m_widgetExplorerView, m_controlBar->location());
        m_widgetExplorer->deleteLater();
        m_widgetExplorerView->deleteLater();
    }
}

void PlasmaApp::updateToolBoxVisibility(bool visible)
{
    bool hadToolBoxOpen = false;

    foreach (Plasma::Containment *cont, m_corona->containments()) {
        if (cont->isToolBoxOpen()) {
            hadToolBoxOpen = true;
        }
         cont->setToolBoxOpen(visible);
    }

    if (!visible && hadToolBoxOpen) {
        closeWidgetExplorer();
    }
}

void PlasmaApp::controlBarMoved(const NetView *controlBar)
{
    if (!m_controlBar || controlBar != m_controlBar) {
        return;
    }

    QRect screenRect = Kephal::ScreenUtils::screenGeometry(m_controlBar->screen());

    Plasma::Containment *cont = m_controlBar->containment();

    switch (controlBar->location()) {
    case Plasma::LeftEdge:
        m_controlBar->move(screenRect.topLeft());
        break;
    case Plasma::RightEdge:
        m_controlBar->move(screenRect.topRight()-QPoint(m_controlBar->size().width(), 0));
        break;
    case Plasma::TopEdge:
        m_controlBar->move(screenRect.topLeft());
        break;
    case Plasma::BottomEdge:
        m_controlBar->move(screenRect.bottomLeft()-QPoint(0,m_controlBar->size().height()));
    default:
        break;
    }

    //flip height and width
    if (controlBar->formFactor() == Plasma::Vertical) {
        if (cont && m_controlBar->size().width() > m_controlBar->size().height()) {
            cont->setMinimumSize(cont->size().height(), cont->size().width());
            cont->setMaximumSize(cont->minimumSize());
        }
    } else if (controlBar->formFactor() == Plasma::Horizontal) {
        if (cont && m_controlBar->size().width() < m_controlBar->size().height()) {
            cont->setMinimumSize(cont->size().height(), cont->size().width());
            cont->setMaximumSize(cont->minimumSize());
        }
    }

    reserveStruts();
}

void PlasmaApp::setAutoHideControlBar(bool autoHide)
{
    if (!m_controlBar) {
        return;
    }

    if (autoHide) {
        if (!m_unHideTimer) {
            m_unHideTimer = new QTimer(this);
            m_unHideTimer->setSingleShot(true);
            connect(m_unHideTimer, SIGNAL(timeout()), this, SLOT(controlBarVisibilityUpdate()));
        }

        m_controlBar->installEventFilter(this);
        controlBarVisibilityUpdate();
    } else {
        m_controlBar->removeEventFilter(this);
        destroyUnHideTrigger();
        delete m_unHideTimer;
        m_unHideTimer = 0;
        setControlBarVisible(true);
    }

    m_autoHideControlBar = autoHide;
    reserveStruts();
    m_controlBar->config().writeEntry("panelAutoHide", autoHide);
}

void PlasmaApp::showWidgetExplorer()
{
    Plasma::Containment *containment = dynamic_cast<Plasma::Containment *>(sender());

    if (!containment) {
        return;
    }

    showWidgetExplorer(containment);
}

void PlasmaApp::showWidgetExplorer(Plasma::Containment *containment)
{
    if (!containment) {
        return;
    }

    containment->setToolBoxOpen(true);

    if (!m_widgetExplorerView) {

        m_widgetExplorerView = new Plasma::Dialog();

        KWindowSystem::setOnAllDesktops(m_widgetExplorerView->winId(), true);
        m_widgetExplorerView->show();
        KWindowSystem::activateWindow(m_widgetExplorerView->winId());
        m_widgetExplorerView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        m_widgetExplorerView->setAttribute(Qt::WA_TranslucentBackground);
        m_widgetExplorerView->setAttribute(Qt::WA_DeleteOnClose);
        KWindowSystem::setState(m_widgetExplorerView->winId(), NET::StaysOnTop|NET::KeepAbove);
        connect(m_widgetExplorerView, SIGNAL(destroyed()), this, SLOT(widgetExplorerDestroyed()));

        if (m_controlBar) {
            switch (m_controlBar->location()) {
            case Plasma::TopEdge:
                m_widgetExplorerView->resize(m_mainView->size().width(), KIconLoader::SizeEnormous);
                m_widgetExplorerView->move(m_controlBar->geometry().bottomLeft());
                break;
            case Plasma::LeftEdge:
                m_widgetExplorerView->resize(KIconLoader::SizeEnormous, m_mainView->size().height());
                m_widgetExplorerView->move(m_controlBar->geometry().topRight());
                break;
            case Plasma::RightEdge:
                m_widgetExplorerView->resize(KIconLoader::SizeEnormous, m_mainView->size().height());
                m_widgetExplorerView->move(m_controlBar->geometry().topLeft() - QPoint(m_widgetExplorerView->size().width(), 0));
                break;
            case Plasma::BottomEdge:
            default:
                m_widgetExplorerView->resize(m_mainView->size().width(), KIconLoader::SizeEnormous);
                m_widgetExplorerView->move(m_controlBar->geometry().topLeft() - QPoint(0, m_widgetExplorerView->size().height()));
                break;
            }
        } else {
            m_widgetExplorerView->resize(m_mainView->size().width(), KIconLoader::SizeEnormous);
            m_widgetExplorerView->move(0,0);
        }

    }

    if (!m_widgetExplorer) {
        m_widgetExplorer = new Plasma::WidgetExplorer(m_controlBar->containment());
        connect(m_widgetExplorer, SIGNAL(closeClicked()), this, SLOT(closeWidgetExplorer()));
        m_widgetExplorer->setContainment(m_mainView->containment());
        m_widgetExplorer->populateWidgetList();

        m_corona->addOffscreenWidget(m_widgetExplorer);

        QSize viewSize = m_widgetExplorerView->size();
        m_widgetExplorerView->setGraphicsWidget(m_widgetExplorer);

        m_widgetExplorer->setIconSize(KIconLoader::SizeLarge);
        m_widgetExplorerView->installEventFilter(this);
    }

    m_widgetExplorer->setLocation(m_controlBar->location());

    if (m_widgetExplorer->location() == Plasma::LeftEdge || m_widgetExplorer->location() == Plasma::RightEdge) {
        m_widgetExplorer->setMinimumWidth(-1);
        m_widgetExplorer->setMinimumHeight(m_mainView->size().height());
    } else {
        m_widgetExplorer->setMinimumWidth(m_mainView->size().width());
        m_widgetExplorer->setMinimumHeight(-1);
    }

    positionPanel();

    m_widgetExplorer->show();
    Plasma::WindowEffects::slideWindow(m_widgetExplorerView, m_controlBar->location());
    m_widgetExplorerView->show();
    emit controlBarChanged();
}

void PlasmaApp::widgetExplorerDestroyed()
{
    m_widgetExplorer = 0;
    m_widgetExplorerView = 0;
    positionPanel();
    if (m_mainView->containment()) {
        m_mainView->containment()->setToolBoxOpen(false);
    }
}


void PlasmaApp::configureContainment(Plasma::Containment *containment)
{
    const QString id = "plasma_containment_settings_" + QString::number(containment->id());
    BackgroundDialog *configDialog = qobject_cast<BackgroundDialog*>(KConfigDialog::exists(id));
    kDebug() << configDialog;

    if (configDialog) {
        configDialog->reloadConfig();
    } else {
        const QSize resolution = Kephal::ScreenUtils::screenGeometry(m_mainView->screen()).size();


        KConfigSkeleton *nullManager = new KConfigSkeleton(0);
        configDialog = new BackgroundDialog(resolution, containment, m_mainView, 0, id, nullManager);
        configDialog->setAttribute(Qt::WA_DeleteOnClose);

        connect(configDialog, SIGNAL(destroyed(QObject*)), nullManager, SLOT(deleteLater()));
    }

    configDialog->show();
    KWindowSystem::setOnDesktop(configDialog->winId(), KWindowSystem::currentDesktop());
    KWindowSystem::activateWindow(configDialog->winId());
}

bool PlasmaApp::mainViewOnTop() const
{
    bool onTop = false;

    QSet<WId> ownWindows;
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        ownWindows.insert(widget->winId());
    }

    //search if the main view is actually one of the widgets on top, show the panel only in this case
    QList<WId> windows = KWindowSystem::stackingOrder();
    for (int i = windows.size() - 1; i >= 0; --i) {
        WId window = windows.at(i);

        if (window == m_mainView->winId()) {
            onTop = true;
            break;
        } else if (!ownWindows.contains(window)) {
            break;
        }
    }

    return onTop;
}

bool PlasmaApp::eventFilter(QObject * watched, QEvent *event)
{
    if (watched == m_widgetExplorerView && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            closeWidgetExplorer();
        }
    } else if (watched == m_widgetExplorerView && event->type() == QEvent::Resize) {
         m_widgetExplorer->resize(m_widgetExplorerView->contentsRect().size());
    } else if (!m_isDesktop && watched == m_mainView && event->type() == QEvent::Close) {
        exit();
    }
    return false;
}

bool PlasmaApp::x11EventFilter(XEvent *event)
{

    if (m_controlBar && m_autoHideControlBar && !m_controlBar->isVisible() && event->xcrossing.window == m_unhideTrigger &&
        (event->xany.send_event != True && event->type == EnterNotify)) {
        //delayed show
        if (!m_glowBar && KWindowSystem::compositingActive() && !m_triggerZone.contains(QCursor::pos())) {
            Plasma::Direction direction = Plasma::locationToDirection(m_controlBar->location());
            m_glowBar = new GlowBar(direction, m_triggerZone);
            m_glowBar->show();
            XMoveResizeWindow(QX11Info::display(), m_unhideTrigger, m_triggerZone.x(), m_triggerZone.y(), m_triggerZone.width(), m_triggerZone.height());

            //FIXME: This is ugly as hell but well, yeah
            if (!m_mousePollTimer) {
                m_mousePollTimer = new QTimer(this);
            }

            disconnect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(unhideHintMousePoll()));
            connect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(unhideHintMousePoll()));
            m_mousePollTimer->start(200);
        } else {
            m_unHideTimer->start(400);
        }
    } else if ((event->xany.send_event != True && event->type == FocusOut)) {
        QTimer::singleShot(100, this, SLOT(lowerMainView()));
    } else if (m_controlBar && m_autoHideControlBar && m_controlBar->isVisible() &&
        (event->xany.send_event != True && event->type == LeaveNotify)) {
        if (m_unHideTimer) {
            m_unHideTimer->start(200);
        }
    }

    return KUniqueApplication::x11EventFilter(event);
}

bool PlasmaApp::hasForegroundWindows() const
{
    return QApplication::activeWindow();
}

void PlasmaApp::lowerMainView()
{
    if (m_isDesktop && !hasForegroundWindows()) {
        KWindowSystem::lowerWindow(m_mainView->winId());
    }
    if (m_shadowWindow) {
        KWindowSystem::clearState(m_shadowWindow->winId(), NET::KeepAbove);
        KWindowSystem::setState(m_shadowWindow->winId(), NET::KeepBelow);
    }
}

void PlasmaApp::controlBarVisibilityUpdate()
{
    if (!m_controlBar) {
        return;
    }

    if (!m_autoHideControlBar) {
        setControlBarVisible(true);

        if (m_shadowWindow && m_shadowWindow->isValid()) {
            Plasma::WindowEffects::slideWindow(m_shadowWindow, m_controlBar->location());
            m_shadowWindow->show();
            if (hasForegroundWindows()) {
                KWindowSystem::clearState(m_shadowWindow->winId(), NET::KeepBelow);
                KWindowSystem::setState(m_shadowWindow->winId(), NET::KeepAbove);
            } else {
                KWindowSystem::clearState(m_shadowWindow->winId(), NET::KeepAbove);
                KWindowSystem::setState(m_shadowWindow->winId(), NET::KeepBelow);
            }
            KWindowSystem::setOnAllDesktops(m_shadowWindow->winId(), true);
        }

        return;
    } else if (m_autoHideControlBar && hasForegroundWindows() && m_controlBar->isVisible()) {
        return;
    }

    if (sender() != m_unHideTimer) {
        m_unHideTimer->start(200);
        return;
    }

    //would be nice to avoid this
    QPoint cursorPos = QCursor::pos();

    if (m_triggerZone.adjusted(-1, -1, 1, 1).contains(cursorPos) || hasForegroundWindows()) {
        if (!m_controlBar->isVisible()) {
            destroyUnHideTrigger();
            Plasma::WindowEffects::slideWindow(m_controlBar, m_controlBar->location());
            setControlBarVisible(true);
        }

        if (m_shadowWindow && m_shadowWindow->isValid()) {
            Plasma::WindowEffects::slideWindow(m_shadowWindow, m_controlBar->location());
            if (hasForegroundWindows()) {
                KWindowSystem::clearState(m_shadowWindow->winId(), NET::KeepBelow);
                KWindowSystem::setState(m_shadowWindow->winId(), NET::KeepAbove);
            }
            m_shadowWindow->show();
            KWindowSystem::setOnAllDesktops(m_shadowWindow->winId(), true);
        }
    } else if (!m_controlBar->geometry().contains(cursorPos) && !mainViewOnTop() && !hasForegroundWindows()) {
        Plasma::WindowEffects::slideWindow(m_controlBar, m_controlBar->location());
        m_controlBar->hide();

        if (m_shadowWindow) {
            Plasma::WindowEffects::slideWindow(m_shadowWindow, m_controlBar->location());
            m_shadowWindow->hide();
        }

        createUnhideTrigger();
    }
}

void PlasmaApp::showControlBar()
{
    setControlBarVisible(true);
}

void PlasmaApp::hideControlBar()
{
    setControlBarVisible(false);
}

void PlasmaApp::setControlBarVisible(bool visible)
{
    if (!m_controlBar || m_controlBar->isVisible() == visible) {
        return;
    }

    if (visible) {
        destroyUnHideTrigger();
        Plasma::WindowEffects::slideWindow(m_controlBar, m_controlBar->location());
        m_controlBar->setWindowFlags(m_mainView->windowFlags() | Qt::FramelessWindowHint);
        m_controlBar->setFrameShape(QFrame::NoFrame);
        m_controlBar->show();
        KWindowSystem::setOnAllDesktops(m_controlBar->winId(), m_isDesktop);
        unsigned long state = NET::Sticky | NET::StaysOnTop | NET::KeepAbove;
        KWindowSystem::setState(m_controlBar->effectiveWinId(), state);
        KWindowSystem::setType(m_controlBar->effectiveWinId(), NET::Dock);

        if (m_shadowWindow && m_shadowWindow->isValid()) {
            Plasma::WindowEffects::slideWindow(m_shadowWindow, m_controlBar->location());
            m_shadowWindow->show();
            if (!m_autoHideControlBar) {
                KWindowSystem::setState(m_shadowWindow->winId(), NET::KeepBelow);
            }
            KWindowSystem::setOnAllDesktops(m_shadowWindow->winId(), true);
        }
    } else if (!m_autoHideControlBar) {
        Plasma::WindowEffects::slideWindow(m_controlBar, m_controlBar->location());
        m_controlBar->hide();
        createUnhideTrigger();

        if (m_shadowWindow) {
            Plasma::WindowEffects::slideWindow(m_shadowWindow, m_controlBar->location());
            m_shadowWindow->hide();
        }
    }
}

void PlasmaApp::toggleControlBarVisibility()
{
    setControlBarVisible(!m_controlBar->isVisible());
}

void PlasmaApp::unhideHintMousePoll()
{
#ifdef Q_WS_X11
    QPoint mousePos = QCursor::pos();
    m_glowBar->updateStrength(mousePos);

    if (!m_unhideTriggerGeom.contains(mousePos)) {
        //kDebug() << "hide the glow";
        if (m_mousePollTimer) {
            m_mousePollTimer->stop();
            disconnect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(unhideHintMousePoll()));
        }

        delete m_glowBar;
        m_glowBar = 0;
        XMoveResizeWindow(QX11Info::display(), m_unhideTrigger, m_unhideTriggerGeom.x(), m_unhideTriggerGeom.y(), m_unhideTriggerGeom.width(), m_unhideTriggerGeom.height());
    } else {
        m_unHideTimer->start(0);
    }
#endif
}

void PlasmaApp::createUnhideTrigger()
{
#ifdef Q_WS_X11
    //kDebug() << m_unhideTrigger << None;
    if (!m_autoHideControlBar || m_unhideTrigger != None || !m_controlBar || m_controlBar->isVisible()) {
        return;
    }

    int actualWidth = 1;
    int actualHeight = 1;
    int triggerWidth = 1;
    int triggerHeight = 1;

    if (KWindowSystem::compositingActive()) {
        triggerWidth = 30;
        triggerHeight = 30;
    }

    QPoint actualTriggerPoint;
    QPoint triggerPoint = actualTriggerPoint = QPoint(qMax(0, m_controlBar->pos().x()), qMax(0, m_controlBar->pos().y()));

    switch (m_controlBar->location()) {
        case Plasma::TopEdge:
            actualWidth = triggerWidth = m_controlBar->width() - 1;
            actualHeight = 1;
            triggerPoint += QPoint(1, 0);

            break;
        case Plasma::BottomEdge:
            actualWidth = triggerWidth = m_controlBar->width() - 1;
            actualTriggerPoint = triggerPoint = m_controlBar->geometry().bottomLeft() + QPoint(1, 0);

            break;
        case Plasma::RightEdge:
            actualHeight = triggerHeight = m_controlBar->height() - 1;
            actualTriggerPoint = triggerPoint = m_controlBar->geometry().topRight() + QPoint(0, 1);

            break;
        case Plasma::LeftEdge:
            actualHeight = triggerHeight = m_controlBar->height() - 1;
            triggerPoint += QPoint(0, -1);

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
                            KeyPressMask | ButtonPressMask |
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
