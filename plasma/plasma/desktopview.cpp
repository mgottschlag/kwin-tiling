/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2007 Matt Broadstone <mbroadst@gmail.com>
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

#include <KAuthorized>
#include <KMenu>
#include <KRun>

#include "plasma/applet.h"
#include "plasma/svg.h"
#include "plasma/corona.h"

#include "krunner_interface.h"

DesktopView::DesktopView(QWidget *parent)
    : QGraphicsView(parent),
      m_background(0),
      m_bitmapBackground(0)
{
    setFrameShape(QFrame::NoFrame);
    setAutoFillBackground(true);

    setScene(new Plasma::Corona(rect(), this));
    scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
    //TODO: Figure out a way to use rubberband and ScrollHandDrag
    setDragMode(QGraphicsView::RubberBandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // Why isn't this working???
    setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //TODO: make this a real background renderer
    KConfigGroup config(KGlobal::config(), "General");
    m_wallpaperPath = config.readEntry("wallpaper", QString());

    //kDebug() << "wallpaperPath is " << m_wallpaperPath << " " << QFile::exists(m_wallpaperPath) << endl;
    if (m_wallpaperPath.isEmpty() ||
        !QFile::exists(m_wallpaperPath)) {
        m_background = new Plasma::Svg("widgets/wallpaper", this);
    }

    //TODO: should we delay the init of the actions until we actually need them?
    engineExplorerAction = new QAction(i18n("Engine Explorer"), this);
    connect(engineExplorerAction, SIGNAL(triggered(bool)), this, SLOT(launchExplorer()));
    runCommandAction = new QAction(i18n("Run Command..."), this);
    connect(runCommandAction, SIGNAL(triggered(bool)), this, SLOT(runCommand()));
}

DesktopView::~DesktopView()
{
}

void DesktopView::zoomIn()
{
    //TODO: Change level of detail when zooming
    // 10/8 == 1.25
    scale(1.25, 1.25);
}

void DesktopView::zoomOut()
{
    // 8/10 == .8
    scale(.8, .8);
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

Plasma::Corona* DesktopView::corona()
{
    return static_cast<Plasma::Corona*>(scene());
}

void DesktopView::drawBackground(QPainter * painter, const QRectF & rect)
{
    if (m_background) {
        m_background->paint(painter, rect);
    } else if (m_bitmapBackground) {
        painter->drawPixmap(rect, *m_bitmapBackground, rect);
    }
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
    //kDebug() << "context menu event " << immutable << endl;
    if (!applet) {
        if (corona() && corona()->isImmutable()) {
            QGraphicsView::contextMenuEvent(event);
            return;
        }

        //FIXME: change this to show this only in debug mode (or not at all?)
        //       before final release
        desktopMenu.addAction(engineExplorerAction);

        if (KAuthorized::authorizeKAction("run_command")) {
            desktopMenu.addAction(runCommandAction);
        }
    } else if (applet->isImmutable()) {
        QGraphicsView::contextMenuEvent(event);
        return;
    } else {
        //desktopMenu.addSeparator();
        bool hasEntries = false;
        if (applet->hasConfigurationInterface()) {
            QAction* configureApplet = new QAction(i18n("%1 Settings...", applet->name()), this);
            connect(configureApplet, SIGNAL(triggered(bool)),
                    applet, SLOT(showConfigurationInterface()));
            desktopMenu.addAction(configureApplet);
            hasEntries = true;
        }

        if (!corona() || !corona()->isImmutable()) {
            QAction* closeApplet = new QAction(i18n("Close this %1", applet->name()), this);
            connect(closeApplet, SIGNAL(triggered(bool)),
                    applet, SLOT(deleteLater()));
            desktopMenu.addAction(closeApplet);
            hasEntries = true;
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

