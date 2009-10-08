/*
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

#include "nettoolbox.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>

#include <KIconLoader>

#include <Plasma/Animator>
#include <Plasma/Frame>
#include <Plasma/Containment>
#include <Plasma/IconWidget>
#include <Plasma/PaintUtils>
#include <Plasma/Svg>


NetToolBox::NetToolBox(Plasma::Containment *parent)
   : QGraphicsWidget(parent),
     m_containment(parent),
     m_icon("plasma"),
     m_iconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall),
     m_animHighlightId(0),
     m_animHighlightFrame(0),
     m_hovering(false)
{
    setZValue(9000);
    resize(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
    setAcceptHoverEvents(true);

    m_toolContainer = new Plasma::Frame(this);
    m_toolContainer->setFrameShadow(Plasma::Frame::Raised);
    m_toolContainer->hide();
    m_toolContainer->setFlag(QGraphicsWidget::ItemStacksBehindParent);
    m_toolContainerLayout = new QGraphicsLinearLayout(m_toolContainer);
    m_toolContainerLayout->setContentsMargins(size().width(), 0, 0, 0);

    m_background = new Plasma::Svg(this);
    m_background->setImagePath("widgets/toolbox");
    m_background->setContainsMultipleImages(true);

    connect(m_containment, SIGNAL(geometryChanged()), this, SLOT(containmentGeometryChanged()));
    containmentGeometryChanged();
}

NetToolBox::~NetToolBox()
{
}

bool NetToolBox::showing() const
{
    return m_toolContainer->isVisible();
}

void NetToolBox::setShowing(const bool show)
{
    if (show != m_toolContainer->isVisible()) {
        emit toggled();
        emit visibilityChanged(show);
    }
    m_toolContainer->setVisible(show);
}


void NetToolBox::addTool(QAction *action)
{
    Plasma::IconWidget *button = new Plasma::IconWidget(this);
    button->setOrientation(Qt::Horizontal);
    button->setDrawBackground(true);
    button->setTextBackgroundColor(QColor());
    button->setAction(action);
    m_actionButtons[action] = button;
    m_toolContainerLayout->addItem(button);
}

void NetToolBox::removeTool(QAction *action)
{
    if (m_actionButtons.contains(action)) {
        Plasma::IconWidget *button = m_actionButtons.value(action);
        m_toolContainerLayout->removeItem(button);
        m_actionButtons.remove(action);
        button->deleteLater();
    }
}

QRectF NetToolBox::expandedGeometry() const
{
    QRectF containerGeometry = m_toolContainer->boundingRect();
    containerGeometry.moveBottomLeft(geometry().bottomLeft());
    return containerGeometry;
}

void NetToolBox::containmentGeometryChanged()
{
    //TODO: hardcode--
    m_toolContainer->resize(m_containment->size().width(), KIconLoader::SizeLarge);
    m_toolContainer->setPos(0, size().height()-m_toolContainer->size().height());
    setPos(0, m_containment->size().height()-size().height());
}

void NetToolBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void NetToolBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    setShowing(!showing());
}

void NetToolBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    m_background->paint(painter, boundingRect(), "desktop-southwest");

    QPoint iconPos(2, size().height() - m_iconSize.height() - 2);

    if (qFuzzyCompare(qreal(1.0), m_animHighlightFrame)) {
        m_icon.paint(painter, QRect(iconPos, m_iconSize));
    } else if (qFuzzyCompare(qreal(1.0), 1 + m_animHighlightFrame)) {
        m_icon.paint(painter, QRect(iconPos, m_iconSize),
                      Qt::AlignCenter, QIcon::Disabled, QIcon::Off);
    } else {
        QPixmap disabled = m_icon.pixmap(m_iconSize, QIcon::Disabled, QIcon::Off);
        QPixmap enabled = m_icon.pixmap(m_iconSize);
        QPixmap result = Plasma::PaintUtils::transition(
            m_icon.pixmap(m_iconSize, QIcon::Disabled, QIcon::Off),
            m_icon.pixmap(m_iconSize), m_animHighlightFrame);
        painter->drawPixmap(QRect(iconPos, m_iconSize), result);
    }
}

void NetToolBox::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (showing() || m_hovering) {
        QGraphicsWidget::hoverEnterEvent(event);
        return;
    }
    Plasma::Animator *animdriver = Plasma::Animator::self();
    if (m_animHighlightId) {
        animdriver->stopCustomAnimation(m_animHighlightId);
    }
    m_hovering = true;
    m_animHighlightId =
        animdriver->customAnimation(
            10, 240, Plasma::Animator::EaseInCurve, this, "animateHighlight");

    QGraphicsWidget::hoverEnterEvent(event);
}

void NetToolBox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //kDebug() << event->pos() << event->scenePos()
    //         << d->toolBacker->rect().contains(event->scenePos().toPoint());
    if (!m_hovering || showing()) {
        QGraphicsWidget::hoverLeaveEvent(event);
        return;
    }

    Plasma::Animator *animdriver = Plasma::Animator::self();
    if (m_animHighlightId) {
        animdriver->stopCustomAnimation(m_animHighlightId);
    }
    m_hovering = false;
    m_animHighlightId =
        animdriver->customAnimation(
            10, 240, Plasma::Animator::EaseOutCurve, this, "animateHighlight");

    QGraphicsWidget::hoverLeaveEvent(event);
}

void NetToolBox::animateHighlight(qreal progress)
{
    if (m_hovering) {
        m_animHighlightFrame = progress;
    } else {
        m_animHighlightFrame = 1.0 - progress;
    }

    if (progress >= 1) {
        m_animHighlightId = 0;
    }

    update();
}

#include "nettoolbox.moc"
