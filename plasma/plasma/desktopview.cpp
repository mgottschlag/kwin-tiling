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

#include <QAction>
#include <QFile>
#include <QWheelEvent>
#include <QCoreApplication>

#include <KAuthorized>
#include <KMenu>
#include <KRun>
#include <KToggleAction>

#include "plasma/applet.h"
#include "plasma/corona.h"
#include "plasma/svg.h"

#include "krunner_interface.h"
#include "plasmaapp.h"

DesktopView::DesktopView(QWidget *parent)
    : QGraphicsView(parent),
      m_background(0),
      m_bitmapBackground(0),
      m_zoomLevel(Plasma::DesktopZoom)
{
    setFrameShape(QFrame::NoFrame);
    setAutoFillBackground(true);
    initializeWallpaper();
    setDragMode(QGraphicsView::RubberBandDrag);
    setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setScene(PlasmaApp::self()->corona());

    //TODO: should we delay the init of the actions until we actually need them?
    m_engineExplorerAction = new QAction(i18n("Engine Explorer"), this);
    connect(m_engineExplorerAction, SIGNAL(triggered(bool)), this, SLOT(launchExplorer()));
    m_runCommandAction = new QAction(i18n("Run Command..."), this);
    connect(m_runCommandAction, SIGNAL(triggered(bool)), this, SLOT(runCommand()));

    m_zoomInAction = new QAction(i18n("Zoom In"), this);
    connect(m_zoomInAction, SIGNAL(triggered(bool)), this, SLOT(zoomIn()));
    m_zoomInAction->setEnabled(false);
    m_zoomOutAction = new QAction(i18n("Zoom Out"), this);
    connect(m_zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(zoomOut()));
}

DesktopView::~DesktopView()
{
}

void DesktopView::zoomIn()
{
    if (m_zoomLevel == Plasma::GroupZoom) {
        m_zoomLevel = Plasma::DesktopZoom;
        m_zoomInAction->setEnabled(false);
        m_zoomOutAction->setEnabled(true);
    } else if (m_zoomLevel == Plasma::OverviewZoom) {
        m_zoomLevel = Plasma::GroupZoom;
        m_zoomInAction->setEnabled(true);
        m_zoomOutAction->setEnabled(true);
    }

    qreal s = Plasma::scalingFactor(m_zoomLevel) / matrix().m11();
    scale(s, s);
}

void DesktopView::zoomOut()
{
    if (m_zoomLevel == Plasma::DesktopZoom) {
        m_zoomLevel = Plasma::GroupZoom;
        m_zoomInAction->setEnabled(true);
        m_zoomOutAction->setEnabled(true);
    } else if (m_zoomLevel == Plasma::GroupZoom) {
        m_zoomLevel = Plasma::OverviewZoom;
        m_zoomInAction->setEnabled(true);
        m_zoomOutAction->setEnabled(false);
    }

    qreal s = Plasma::scalingFactor(m_zoomLevel) / matrix().m11();
    scale(s, s);
}

void DesktopView::launchExplorer()
{
    KRun::run("plasmaengineexplorer", KUrl::List(), 0);
}

void DesktopView::runCommand()
{
    if (!KAuthorized::authorizeKAction("run_command")) {
        return;
    }

    QString interface("org.kde.krunner");
    org::kde::krunner::Interface krunner(interface, "/Interface",
                                         QDBusConnection::sessionBus());
    krunner.display();
}

void DesktopView::initializeWallpaper()
{
    //TODO: make this a real background renderer
    KConfigGroup config(KGlobal::config(), "General");
    m_wallpaperPath = config.readEntry("wallpaper", QString());

    //kDebug() << "wallpaperPath is " << m_wallpaperPath << QFile::exists(m_wallpaperPath);
    if (m_wallpaperPath.isEmpty() ||
        !QFile::exists(m_wallpaperPath)) {
        m_background = new Plasma::Svg("widgets/wallpaper", this);
    }

    //FIXME: this probably doesn't give us an immediate update when called after construction!
}

void DesktopView::drawBackground(QPainter * painter, const QRectF & rect)
{
    // avoid expensive operations to the painter if there is nothing to paint
    if (!m_background && !m_bitmapBackground)
        return;

    // draw the background untransformed (saves lots of per-pixel-math)
    painter->save();
    painter->resetTransform();
    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    if (m_background) {
        // Plasma::Svg doesn't support drawing only part of the image (it only
        // supports drawing the whole image to a rect), so we blit to 0,0-w,h
        m_background->paint(painter, 0, 0);
    } else if (m_bitmapBackground) {
        // for pixmaps we draw only the exposed part (untransformed since the
        // bitmapBackground already has the size of the viewport)
        QRect exposedRect = mapFromScene(rect).boundingRect();
        painter->drawPixmap(exposedRect, *m_bitmapBackground, exposedRect);
    }

    // restore transformation and composition mode
    painter->restore();
}

void DesktopView::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
    if (testAttribute(Qt::WA_PendingResizeEvent)) {
        return; // lets not do this more than necessary, shall we?
    }

    scene()->setSceneRect(rect());

    if (m_background) {
        m_background->resize(width(), height());
    } else if (!m_wallpaperPath.isEmpty()) {
        delete m_bitmapBackground;
        m_bitmapBackground = new QPixmap(m_wallpaperPath);
        (*m_bitmapBackground) = m_bitmapBackground->scaled(size());
    }
}

void DesktopView::wheelEvent(QWheelEvent* event)
{
    if (scene() && scene()->itemAt(event->pos())) {
        QGraphicsView::wheelEvent(event);
        return;
    }

    if (event->modifiers() & Qt::ControlModifier) {
        if (event->delta() < 0) {
            zoomOut();
        } else {
            zoomIn();
        }
    }
}

void DesktopView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!scene() || !KAuthorized::authorizeKAction("desktop_contextmenu")) {
        QGraphicsView::contextMenuEvent(event);
        return;
    }

    QPointF point = event->pos();
    /*
    * example for displaying the SuperKaramba context menu
    QGraphicsItem *item = itemAt(point);
    if(item) {
    QObject *object = dynamic_cast<QObject*>(item->parentItem());
    if(object && object->objectName().startsWith("karamba")) {
    QContextMenuEvent event(QContextMenuEvent::Mouse, point);
    contextMenuEvent(&event);
    return;
}
}
    */
    QGraphicsItem* item = scene()->itemAt(point);
    Plasma::Applet* applet = 0;

    while (item) {
        applet = qgraphicsitem_cast<Plasma::Applet*>(item);
        if (applet) {
            break;
        }

        item = item->parentItem();
    }

    KMenu desktopMenu;
    //kDebug() << "context menu event " << immutable;
    if (m_zoomLevel != Plasma::DesktopZoom) {
        desktopMenu.addAction(m_zoomInAction);
        desktopMenu.addAction(m_zoomOutAction);
    }
    else if (!applet) {
        if (PlasmaApp::self()->corona()->isImmutable()) {
            QGraphicsView::contextMenuEvent(event);
            return;
        }

        //FIXME: change this to show this only in debug mode (or not at all?)
        //       before final release
        desktopMenu.addAction(m_engineExplorerAction);

        if (KAuthorized::authorizeKAction("run_command")) {
            desktopMenu.addAction(m_runCommandAction);
        }

        desktopMenu.addSeparator();
        desktopMenu.addAction(m_zoomInAction);
        desktopMenu.addAction(m_zoomOutAction);
    } else if (applet->isImmutable()) {
        QGraphicsView::contextMenuEvent(event);
        return;
    } else {
        bool hasEntries = false;
        if (applet->hasConfigurationInterface()) {
            QAction* configureApplet = new QAction(i18n("%1 Settings...", applet->name()), this);
            connect(configureApplet, SIGNAL(triggered(bool)),
                    applet, SLOT(showConfigurationInterface()));
            desktopMenu.addAction(configureApplet);
            hasEntries = true;
        }

        if (!PlasmaApp::self()->corona()->isImmutable()) {
            QAction* closeApplet = new QAction(i18n("Remove this %1", applet->name()), this);
            connect(closeApplet, SIGNAL(triggered(bool)),
                    applet, SLOT(destroy()));
            desktopMenu.addAction(closeApplet);
            hasEntries = true;
        }

        QList<QAction*> actions = applet->contextActions();
        if (!actions.isEmpty()) {
            desktopMenu.addSeparator();
            foreach(QAction* action, actions) {
                desktopMenu.addAction(action);
                hasEntries = true;
            }
        }

        if (!hasEntries) {
            QGraphicsView::contextMenuEvent(event);
            return;
        }
    }

    event->accept();
    desktopMenu.exec(point.toPoint());
}

#include "desktopview.moc"

