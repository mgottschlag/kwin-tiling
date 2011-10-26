/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
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

#include "internaltoolbox.h"

#include <QAction>
#include <QApplication>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QRadialGradient>

#include <KColorScheme>
#include <KConfigGroup>
#include <KIconLoader>
#include <KDebug>

#include <Plasma/Corona>
#include <Plasma/Theme>
#include <Plasma/IconWidget>


InternalToolBox::InternalToolBox(Plasma::Containment *parent)
    : AbstractToolBox(parent),
      m_containment(parent),
      m_corner(InternalToolBox::TopRight),
      m_size(KIconLoader::SizeSmallMedium),
      m_iconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall),
      m_hidden(false),
      m_showing(false),
      m_movable(false),
      m_dragging(false),
      m_userMoved(false),
      m_iconic(true)
{
    init();
}

InternalToolBox::InternalToolBox(QObject *parent, const QVariantList &args)
    : AbstractToolBox(parent, args),
      m_containment(qobject_cast<Plasma::Containment *>(parent)),
      m_corner(InternalToolBox::TopRight),
      m_size(KIconLoader::SizeSmallMedium),
      m_iconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall),
      m_hidden(false),
      m_showing(false),
      m_movable(false),
      m_dragging(false),
      m_userMoved(false),
      m_iconic(true)
{
    init();
}

InternalToolBox::~InternalToolBox()
{
}

void InternalToolBox::init()
{
    if (m_containment) {
        connect(m_containment, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
                this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));
    }

    setAcceptsHoverEvents(true);
}

Plasma::Containment *InternalToolBox::containment()
{
    return m_containment;
}

QPoint InternalToolBox::toolPosition(int toolHeight)
{
    switch (corner()) {
    case TopRight:
        return QPoint(boundingRect().width(), -toolHeight);
    case Top:
        return QPoint((int)boundingRect().center().x() - boundingRect().width(), -toolHeight);
    case TopLeft:
        return QPoint(-boundingRect().width(), -toolHeight);
    case Left:
        return QPoint(-boundingRect().width(), (int)boundingRect().center().y() - boundingRect().height());
    case Right:
        return QPoint(boundingRect().width(), (int)boundingRect().center().y() - boundingRect().height());
    case BottomLeft:
        return QPoint(-boundingRect().width(), toolHeight);
    case Bottom:
        return QPoint((int)boundingRect().center().x() - m_iconSize.width(), toolHeight);
    case BottomRight:
    default:
        return QPoint(boundingRect().width(), toolHeight);
    }
}

QGraphicsWidget *InternalToolBox::toolParent()
{
    return this;
}

QList<QAction *> InternalToolBox::actions() const
{
    return m_actions;
}

void InternalToolBox::addTool(QAction *action)
{
    if (!action) {
        return;
    }

    if (m_actions.contains(action)) {
        return;
    }

    connect(action, SIGNAL(destroyed(QObject*)), this, SLOT(actionDestroyed(QObject*)));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(toolTriggered(bool)));
    m_actions.append(action);
}

void InternalToolBox::removeTool(QAction *action)
{
    disconnect(action, 0, this, 0);
    m_actions.removeAll(action);
}

void InternalToolBox::actionDestroyed(QObject *object)
{
    m_actions.removeAll(static_cast<QAction*>(object));
}

bool InternalToolBox::isEmpty() const
{
    return m_actions.isEmpty();
}

void InternalToolBox::toolTriggered(bool)
{
}

int InternalToolBox::size() const
{
    return  m_size;
}

void InternalToolBox::setSize(const int newSize)
{
    m_size = newSize;
}

QSize InternalToolBox::iconSize() const
{
    return m_iconSize;
}

void InternalToolBox::setIconSize(const QSize newSize)
{
    m_iconSize = newSize;
}

bool InternalToolBox::isShowing() const
{
    return m_showing;
}

void InternalToolBox::setShowing(const bool show)
{
    if (show) {
        showToolBox();
    } else {
        hideToolBox();
    }
    m_showing = show;
}

void InternalToolBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        event->accept();
        // set grab position relative to toolbox
        m_dragStartRelative = mapToParent(event->pos()).toPoint() - pos().toPoint();
    } else {
        event->ignore();
    }
}

QSize InternalToolBox::cornerSize() const
{
    return boundingRect().size().toSize();
}

QSize InternalToolBox::fullWidth() const
{
    return boundingRect().size().toSize();
}

QSize  InternalToolBox::fullHeight() const
{
    return boundingRect().size().toSize();
}

void InternalToolBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_movable || (!m_dragging && boundingRect().contains(event->pos()))) {
        return;
    }

    m_dragging = true;
    m_userMoved = true;
    const QPoint newPos = mapToParent(event->pos()).toPoint();
    const QPoint curPos = pos().toPoint();

    const QSize cSize = cornerSize();
    const QSize fHeight = fullHeight();
    const QSize fWidth = fullWidth();
    const int h = fHeight.height();
    const int w = fWidth.width();

    const int areaWidth = parentWidget()->size().width();
    const int areaHeight = parentWidget()->size().height();

    int x = curPos.x();
    int y = curPos.y();

    // jump to the nearest desktop border
    int distanceToLeft = newPos.x() - m_dragStartRelative.x();
    int distanceToRight = areaWidth - w - distanceToLeft;
    int distanceToTop = newPos.y() - m_dragStartRelative.y();
    int distanceToBottom = areaHeight - h - distanceToTop;

    int distancetoHorizontalMiddle = qAbs((newPos.x() + boundingRect().size().width()/2) - areaWidth/2 - m_dragStartRelative.x());
    int distancetoVerticalMiddle = qAbs((newPos.y() + boundingRect().size().height()/2) - areaHeight/2 - m_dragStartRelative.y());

    if (distancetoHorizontalMiddle < 10) {
        x = areaWidth/2 - boundingRect().size().width()/2;
    } else if (distancetoVerticalMiddle < 10) {
        y = areaHeight/2 - boundingRect().size().height()/2;
    } else {
        // decide which border is the nearest
        if (distanceToLeft < distanceToTop && distanceToLeft < distanceToRight &&
            distanceToLeft < distanceToBottom ) {
            x = 0;
            y = (newPos.y() - m_dragStartRelative.y());
        } else if (distanceToRight < distanceToTop && distanceToRight < distanceToLeft &&
                distanceToRight < distanceToBottom) {
            x = areaWidth - w;
            y = (newPos.y() - m_dragStartRelative.y());
        } else if (distanceToTop < distanceToLeft && distanceToTop < distanceToRight &&
                distanceToTop < distanceToBottom ) {
            y = 0;
            x = (newPos.x() - m_dragStartRelative.x());
        } else if (distanceToBottom < distanceToLeft && distanceToBottom < distanceToRight &&
                distanceToBottom < distanceToTop) {
            y = areaHeight - h;
            x = (newPos.x() - m_dragStartRelative.x());
        }
    }


    x = qBound(0, x, areaWidth - w);
    y = qBound(0, y, areaHeight - h);

    Corner newCorner = corner();
    if (x == 0) {
        if (y == 0) {
            newCorner = TopLeft;
        } else if (areaHeight - cSize.height() < newPos.y()) {
            y = areaHeight - cSize.height();
            newCorner = BottomLeft;
        } else {
            newCorner = Left;
        }
    } else if (y == 0) {
        if (areaWidth - cSize.width() < newPos.x()) {
            x = areaWidth - cSize.width();
            newCorner = TopRight;
        } else {
            newCorner = Top;
        }
    } else if (x + w >= areaWidth) {
        if (areaHeight - cSize.height() < newPos.y()) {
            y = areaHeight - cSize.height();
            x = areaWidth - cSize.width();
            newCorner = BottomRight;
        } else {
            x = areaWidth - fHeight.width();
            newCorner = Right;
        }
    } else {
        y = areaHeight - fWidth.height();
        newCorner = Bottom;
    }

    if (newCorner != corner()) {
        prepareGeometryChange();
        setCorner(newCorner);
    }

    setPos(x, y);
}

void InternalToolBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !m_dragging && boundingRect().contains(event->pos())) {
        emit toggled();
        return;
    }

    m_dragging = false;
    KConfigGroup cg(m_containment->config());
    save(cg);
}

bool InternalToolBox::isMovable() const
{
    return m_movable;
}

void InternalToolBox::setIsMovable(bool movable)
{
    m_movable = movable;
}

void InternalToolBox::setCorner(const Corner corner)
{
    m_corner = corner;
}

InternalToolBox::Corner InternalToolBox::corner() const
{
    return m_corner;
}

void InternalToolBox::save(KConfigGroup &cg) const
{
    if (!m_movable) {
        return;
    }

    KConfigGroup group(&cg, "ToolBox");
    if (!m_userMoved) {
        group.deleteGroup();
        return;
    }

    int offset = 0;
    if (corner() == InternalToolBox::Left ||
        corner() == InternalToolBox::Right) {
        offset = y();
    } else if (corner() == InternalToolBox::Top ||
               corner() == InternalToolBox::Bottom) {
        offset = x();
    }

    group.writeEntry("corner", int(corner()));
    group.writeEntry("offset", offset);
}

void InternalToolBox::restore(const KConfigGroup &containmentGroup)
{
    if (!m_movable) {
        return;
    }

    KConfigGroup group = KConfigGroup(&containmentGroup, "ToolBox");

    if (!group.hasKey("corner")) {
        return;
    }

    m_userMoved = true;
    setCorner(Corner(group.readEntry("corner", int(corner()))));

    const int offset = group.readEntry("offset", 0);
    const int w = boundingRect().width();
    const int h = boundingRect().height();
    const int maxW = m_containment ? m_containment->geometry().width() - w : offset;
    const int maxH = m_containment ? m_containment->geometry().height() - h : offset;
    switch (corner()) {
        case InternalToolBox::TopLeft:
            setPos(0, 0);
            break;
        case InternalToolBox::Top:
            setPos(qMin(offset, maxW), 0);
            break;
        case InternalToolBox::TopRight:
            setPos(m_containment->size().width() - boundingRect().width(), 0);
            break;
        case InternalToolBox::Right:
            setPos(m_containment->size().width() - boundingRect().width(), qMin(offset, maxH));
            break;
        case InternalToolBox::BottomRight:
            setPos(m_containment->size().width() - boundingRect().width(), m_containment->size().height() - boundingRect().height());
            break;
        case InternalToolBox::Bottom:
            setPos(qMin(offset, maxW), m_containment->size().height() - boundingRect().height());
            break;
        case InternalToolBox::BottomLeft:
            setPos(0, m_containment->size().height() - boundingRect().height());
            break;
        case InternalToolBox::Left:
            setPos(0, qMin(offset, maxH));
            break;
    }
    //kDebug() << "marked as user moved" << pos()
    //         << (m_containment->containmentType() == Containment::PanelContainment);
}

void InternalToolBox::immutabilityChanged(Plasma::ImmutabilityType immutability)
{
    const bool unlocked = immutability == (Plasma::Mutable);

    if (containment() &&
        (containment()->type() == Plasma::Containment::PanelContainment ||
         containment()->type() == Plasma::Containment::CustomPanelContainment)) {
        setVisible(unlocked);
    } else {
        setIsMovable(unlocked);
    }
}

void InternalToolBox::reposition()
{
    updateToolBox();

    if (m_userMoved) {
        restore(m_containment->config());
        return;
    }

    if (m_containment->containmentType() == Plasma::Containment::PanelContainment ||
        m_containment->containmentType() == Plasma::Containment::CustomPanelContainment) {
        QRectF rect = boundingRect();
        if (m_containment->formFactor() == Plasma::Vertical) {
            setCorner(InternalToolBox::Bottom);
            setPos(m_containment->geometry().width() / 2 - rect.width() / 2,
                   m_containment->geometry().height() - rect.height());
        } else {
            //defaulting to Horizontal right now
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                setPos(m_containment->geometry().left(),
                       m_containment->geometry().height() / 2 - rect.height() / 2);
                setCorner(InternalToolBox::Left);
            } else {
                setPos(m_containment->geometry().width() - rect.width(),
                       m_containment->geometry().height() / 2 - rect.height() / 2);
                setCorner(InternalToolBox::Right);
            }
        }

        //kDebug() << "got ourselves a panel containment, moving to" << pos();
    } else if (m_containment->corona()) {
        //kDebug() << "desktop";

        int screen = m_containment->screen();
        QRectF avail = m_containment->geometry();
        QRectF screenGeom = avail;

        if (screen > -1 && screen < m_containment->corona()->numScreens()) {
            avail = m_containment->corona()->availableScreenRegion(screen).boundingRect();
            screenGeom = m_containment->corona()->screenGeometry(screen);
            avail.translate(-screenGeom.topLeft());
        }

        // Transform to the containment's coordinate system.
        screenGeom.moveTo(0, 0);

        if (!m_containment->view() || !m_containment->view()->transform().isScaling()) {
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                if (avail.top() > screenGeom.top()) {
                    setPos(avail.topLeft() - QPoint(0, avail.top()));
                    setCorner(InternalToolBox::Left);
                } else if (avail.left() > screenGeom.left()) {
                    setPos(avail.topLeft() - QPoint(boundingRect().width(), 0));
                    setCorner(InternalToolBox::Top);
                } else {
                    setPos(avail.topLeft());
                    setCorner(InternalToolBox::Top);
                }
            } else {
                if (avail.top() > screenGeom.top()) {
                    setPos(avail.topRight() - QPoint(boundingRect().width(), -avail.top()));
                    setCorner(InternalToolBox::Right);
                } else if (avail.right() < screenGeom.right()) {
                    setPos(avail.topRight() - QPoint(boundingRect().width(), 0));
                    setCorner(InternalToolBox::Top);
                } else {
                    setPos(avail.topRight() - QPoint(boundingRect().width(), 0));
                    setCorner(InternalToolBox::Top);
                }
            }
        } else {
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                setPos(m_containment->mapFromScene(QPointF(m_containment->geometry().topLeft())));
                setCorner(InternalToolBox::Top);
            } else {
                setPos(m_containment->mapFromScene(QPointF(m_containment->geometry().topRight())));
                setCorner(InternalToolBox::Top);
            }
        }
    }
}

#include "internaltoolbox.moc"

