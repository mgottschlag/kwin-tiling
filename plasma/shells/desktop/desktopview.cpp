/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "desktopview.h"

#include <KAction>
#include <QFile>
#include <QWheelEvent>
#include <QCoreApplication>

#include <KAuthorized>
#include <KMenu>
#include <KRun>
#include <KToggleAction>
#include <KWindowSystem>
#include <NETRootInfo>

#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/Svg>
#include <Plasma/Wallpaper>
#include <Plasma/Theme>

#include <kephal/screens.h>

#include "dashboardview.h"
#include "plasmaapp.h"
#include "plasma-shell-desktop.h"

#ifdef Q_WS_WIN
#include "windows.h"
#include "windef.h"
#include "wingdi.h"
#include "winuser.h"
#endif

DesktopView::DesktopView(Plasma::Containment *containment, int id, QWidget *parent)
    : Plasma::View(containment, id, parent),
      m_dashboard(0),
      m_dashboardFollowsDesktop(true),
      m_init(false)
{
    setAttribute(Qt::WA_TranslucentBackground, false);
    setFocusPolicy(Qt::NoFocus);
#ifdef Q_WS_WIN
    setWindowFlags(Qt::FramelessWindowHint);
    SetWindowPos(winId(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    HWND hwndDesktop = ::FindWindowW(L"Progman", NULL);
    SetParent(winId(), hwndDesktop);
#else
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#endif

    if (AppSettings::perVirtualDesktopViews()) {
        kDebug() << "setting to desktop" << containment->desktop() + 1;
        KWindowSystem::setOnDesktop(winId(), containment->desktop() + 1);
    } else {
        KWindowSystem::setOnAllDesktops(winId(), true);
    }

    KWindowSystem::setType(winId(), NET::Desktop);
    lower();

    if (containment) {
        containment->enableAction("zoom in", false);
        containment->enableAction("add sibling containment", false);
    }
    //FIXME should we have next/prev or up/down/left/right or what?
    KAction *action = new KAction(i18n("Next Activity"), this);
    action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_D, Qt::Key_Right));
    connect(action, SIGNAL(triggered()), this, SLOT(nextContainment()));
    addAction(action);
    action = new KAction(i18n("Previous Activity"), this);
    action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_D, Qt::Key_Left));
    connect(action, SIGNAL(triggered()), this, SLOT(previousContainment()));
    addAction(action);

    const int w = 25;
    QPixmap tile(w * 2, w * 2);
    tile.fill(palette().base().color());
    QPainter pt(&tile);
    QColor color = palette().mid().color();
    color.setAlphaF(.6);
    pt.fillRect(0, 0, w, w, color);
    pt.fillRect(w, w, w, w, color);
    pt.end();
    QBrush b(tile);
    setBackgroundBrush(tile);

    m_dashboardFollowsDesktop = (dashboardContainment() == 0);

    // since Plasma::View has a delayed init we need to
    // put a delay also for this call in order to be sure
    // to have correct information set (e.g. screen())
    QTimer::singleShot(0, this, SLOT(adjustSize()));

    Kephal::Screens *screens = Kephal::Screens::self();
    connect(screens, SIGNAL(screenResized(Kephal::Screen *, QSize, QSize)),
            this, SLOT(screenResized(Kephal::Screen *)));
    connect(screens, SIGNAL(screenMoved(Kephal::Screen *, QPoint, QPoint)),
            this, SLOT(screenMoved(Kephal::Screen *)));
}

DesktopView::~DesktopView()
{
    delete m_dashboard;
}

void DesktopView::toggleDashboard()
{
    if (!m_dashboard) {
        if (!containment()) {
            return;
        }

        m_dashboardFollowsDesktop = true;
        KConfigGroup cg = config();
        Plasma::Containment *dc = containment();
        if (dashboardContainment()) {
            dc = dashboardContainment();
            dc->resize(containment()->size());
            m_dashboardFollowsDesktop = false;
        }

        m_dashboard = new DashboardView(dc, 0);
        m_dashboard->addActions(actions());
    }

    m_dashboard->toggleVisibility();
    //kDebug() << "toggling dashboard for screen" << screen() << m_dashboard->isVisible();
}

Plasma::Containment *DesktopView::dashboardContainment() const
{
    KConfigGroup cg = config();
    Plasma::Containment *dc = 0;
    int containmentId = cg.readEntry("DashboardContainment", 0);
    if (containmentId > 0) {
        foreach (Plasma::Containment *c, containment()->corona()->containments()) {
            if ((int)c->id() == containmentId) {
                dc = c;
                break;
            }
        }
    }
    return dc;
}

void DesktopView::setDashboardContainment(Plasma::Containment *containment)
{
    if (containment) {
        config().writeEntry("DashboardContainment", containment->id());
        if (m_dashboard) {
            m_dashboard->setContainment(containment);
        }
    } else {
        if (dashboardContainment()) {
            dashboardContainment()->destroy(false);
        }
        config().writeEntry("DashboardContainment", 0);
        if (m_dashboard) {
            m_dashboard->setContainment(View::containment());
        }
    }
}

void DesktopView::screenResized(Kephal::Screen *s)
{
    if (s->id() == screen()) {
        kDebug() << screen();
        adjustSize();
    }
}

void DesktopView::screenMoved(Kephal::Screen *s)
{
    if (s->id() == screen()) {
        kDebug() << screen();
        adjustSize();
    }
}

void DesktopView::adjustSize()
{
    // adapt to screen resolution changes
    QRect geom = Kephal::ScreenUtils::screenGeometry(screen());
    kDebug() << "screen" << screen() << "geom" << geom;
    setGeometry(geom);
    containment()->resize(geom.size());
    kDebug() << "Containment's geom after resize" << containment()->geometry(); 

    if (m_dashboard) {
        m_dashboard->setGeometry(geom);
    }

    kDebug() << "Done" << screen() << geometry();
}

bool DesktopView::isDashboardVisible() const
{
    return m_dashboard && m_dashboard->isVisible();
}

bool DesktopView::dashboardFollowsDesktop() const
{
    return m_dashboardFollowsDesktop;
}

void DesktopView::setDashboardFollowsDesktop(bool follow)
{
    m_dashboardFollowsDesktop = follow;

    if (follow) {
        config().writeEntry("DashboardContainment", containment()->id());
    } else {
        config().writeEntry("DashboardContainment", 0);
    }
}

void DesktopView::setContainment(Plasma::Containment *containment)
{
    Plasma::Containment *oldContainment = this->containment();
    if (m_init && containment == oldContainment) {
        return;
    }

    m_init = true;
    Plasma::ZoomLevel zoomLevel = PlasmaApp::self()->desktopZoomLevel();
    if (zoomLevel == Plasma::DesktopZoom && containment) {
        //make sure actions are up-to-date
        //this is icky but necessary to have the toolbox show the right actions for the zoom level
        containment->enableAction("zoom in", false);
        containment->enableAction("add sibling containment", false);
    }

    if (m_dashboard && m_dashboardFollowsDesktop) {
        m_dashboard->setContainment(containment);
    }

    if (oldContainment) {
        disconnect(oldContainment, SIGNAL(toolBoxToggled()), this, SLOT(toolBoxOpened()));
        disconnect(oldContainment, SIGNAL(toolBoxToggled()), this, SLOT(toolBoxClosed()));
        if (zoomLevel == Plasma::DesktopZoom) {
            //make sure actions are up-to-date
            oldContainment->enableAction("zoom in", false);
            oldContainment->enableAction("add sibling containment", false);
        }
    }

    connect(containment, SIGNAL(toolBoxToggled()), this, SLOT(toolBoxOpened()));
    View::setContainment(containment);
}

void DesktopView::toolBoxOpened()
{
    NETRootInfo info(QX11Info::display(), NET::Supported);
    if (!info.isSupported(NET::WM2ShowingDesktop)) {
        return;
    }

    Plasma::Containment *c = containment();
    disconnect(c, SIGNAL(toolBoxToggled()), this, SLOT(toolBoxOpened()));
    connect(c, SIGNAL(toolBoxToggled()), this, SLOT(toolBoxClosed()));
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
            this, SLOT(showDesktopUntoggled()));

    info.setShowingDesktop(true);
}

void DesktopView::toolBoxClosed()
{
    NETRootInfo info(QX11Info::display(), NET::Supported);
    if (!info.isSupported(NET::WM2ShowingDesktop)) {
        return;
    }

    Plasma::Containment *c = containment();
    disconnect(c, SIGNAL(toolBoxToggled()), this, SLOT(toolBoxClosed()));
    disconnect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
               this, SLOT(showDesktopUntoggled()));
    connect(c, SIGNAL(toolBoxToggled()), this, SLOT(toolBoxOpened()));

    info.setShowingDesktop(false);
}

void DesktopView::showDesktopUntoggled()
{
    disconnect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
               this, SLOT(showDesktopUntoggled()));

    Plasma::Containment *c = containment();
    if (c) {
        disconnect(c, SIGNAL(toolBoxToggled()), this, SLOT(toolBoxClosed()));
        connect(c, SIGNAL(toolBoxToggled()), this, SLOT(toolBoxOpened()));
        containment()->setToolBoxOpen(false);
    }
}

void DesktopView::zoomIn(Plasma::ZoomLevel zoomLevel)
{
    if (zoomLevel == Plasma::DesktopZoom) {
        setDragMode(NoDrag);
        qreal factor = Plasma::scalingFactor(zoomLevel) / matrix().m11();
        scale(factor, factor);
        if (containment()) {
            //disconnect from other containments
            Plasma::Corona *corona = containment()->corona();
            if (corona) {
                QList<Plasma::Containment*> containments = corona->containments();
                foreach (Plasma::Containment *c, containments) {
                    if (c == containment() || PlasmaApp::isPanelContainment(c)) {
                        continue;
                    }
                    disconnect(c, 0, this, 0);
                }
            }
            setSceneRect(containment()->geometry());
        }

        toolBoxClosed();
    } else if (zoomLevel == Plasma::GroupZoom) {
        qreal factor = Plasma::scalingFactor(zoomLevel);
        factor = factor / matrix().m11();
        scale(factor, factor);
        setSceneRect(QRectF(0, 0, scene()->sceneRect().right(), scene()->sceneRect().bottom() + TOOLBOX_MARGIN));
    } else {
        setDragMode(NoDrag);
    }
}

void DesktopView::zoomOut(Plasma::ZoomLevel zoomLevel)
{
    setDragMode(ScrollHandDrag);
    qreal factor = Plasma::scalingFactor(zoomLevel);
    qreal s = factor / matrix().m11();
    scale(s, s);
    setSceneRect(QRectF(0, 0, scene()->sceneRect().right(), scene()->sceneRect().bottom() + TOOLBOX_MARGIN));

    if (containment()) {
        ensureVisible(containment()->sceneBoundingRect());
    }
}

void DesktopView::wheelEvent(QWheelEvent* event)
{
    QGraphicsItem * item = scene() ? scene()->itemAt(sceneRect().topLeft() + event->pos()) : 0;

    if ((!item || item == (QGraphicsItem*)containment()) && event->modifiers() & Qt::ControlModifier) {
        if (event->modifiers() & Qt::ControlModifier) {
            if (event->delta() < 0) {
                PlasmaApp::self()->zoom(containment(), Plasma::ZoomOut);
            } else {
                PlasmaApp::self()->zoom(containment(), Plasma::ZoomIn);
            }
        }

        event->accept();
        return;
    }

    Plasma::View::wheelEvent(event);
}

// This function is reimplemented from QGraphicsView to work around the problem
// that QPainter::fillRect(QRectF/QRect, QBrush), which QGraphicsView uses, is
// potentially slow when the anti-aliasing hint is set and as implemented won't
// hit accelerated code at all when it isn't set.  This implementation avoids
// the problem by using integer coordinates and by using drawTiledPixmap() in
// the case of a texture brush, and fillRect(QRect, QColor) in the case of a
// solid pattern.  As an additional optimization it draws the background with
// CompositionMode_Source.
void DesktopView::drawBackground(QPainter *painter, const QRectF &rect)
{
    const QPainter::CompositionMode savedMode = painter->compositionMode();
    const QBrush brush = backgroundBrush();

    switch (brush.style())
    {
    case Qt::TexturePattern:
    {
        // Note: this assumes that the brush origin is (0, 0), and that
        //       the brush has an identity transformation matrix.
        const QPixmap texture = brush.texture();
        QRect r = rect.toAlignedRect();
        r.setLeft(r.left() - (r.left() % texture.width()));
        r.setTop(r.top() - (r.top() % texture.height()));
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->drawTiledPixmap(r, texture);
        painter->setCompositionMode(savedMode);
        return;
    }

    case Qt::SolidPattern:
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect.toAlignedRect(), brush.color());
        painter->setCompositionMode(savedMode);
        return;

    default:
        QGraphicsView::drawBackground(painter, rect);
    }
}

void DesktopView::screenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment* newContainment)
{
    //kDebug() << "was:" << wasScreen << "is:" << isScreen << "my screen:" << screen() << "containment:" << (QObject*)newContainment << newContainment->activity() << "myself:" << (QObject*)this <<"containment desktop:"<<newContainment->desktop() << "my desktop:"<<containment()->desktop();
    if (PlasmaApp::isPanelContainment(newContainment)) {
        // we don't care about panel containments changing screens on us
        return;
    }

    if (AppSettings::perVirtualDesktopViews() && containment() && newContainment->desktop() != containment()->desktop()) {
        return;
    }

    if (wasScreen == screen() && this->containment() == newContainment) {
        setContainment(0);
    }

    if (isScreen == screen()) {
        setContainment(newContainment);
    }
}

void DesktopView::nextContainment()
{
    QList<Plasma::Containment*> containments = containment()->corona()->containments();
    int start = containments.indexOf(containment());
    int i = (start + 1) % containments.size();
    //FIXME this is a *horrible* way of choosing a "next" containment.
    while (i != start) {
        if (!PlasmaApp::isPanelContainment(containments.at(i)) &&
            containments.at(i)->screen() == -1) {
            break;
        }

        i = (i + 1) % containments.size();
    }

    Plasma::Containment *c = containments.at(i);
    setContainment(c);
}

void DesktopView::previousContainment()
{
    QList<Plasma::Containment*> containments = containment()->corona()->containments();
    int start = containments.indexOf(containment());
    //fun fact: in c++, (-1 % foo) == -1
    int i = start - 1;
    if (i < 0) {
        i += containments.size();
    }

    //FIXME this is a *horrible* way of choosing a "previous" containment.
    while (i != start) {
        if (!PlasmaApp::isPanelContainment(containments.at(i)) &&
            containments.at(i)->screen() == -1) {
            break;
        }

        if (--i < 0) {
            i += containments.size();
        }
    }

    Plasma::Containment *c = containments.at(i);
    setContainment(c);
}

#include "desktopview.moc"

