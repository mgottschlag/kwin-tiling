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
#include <Plasma/Containment>
#include <Plasma/IconWidget>
#include <Plasma/ItemBackground>
#include <Plasma/PaintUtils>
#include <Plasma/FrameSvg>
#include <Plasma/Svg>


class ToolContainer : public QGraphicsWidget
{
public:
    ToolContainer(QGraphicsWidget *parent)
         : QGraphicsWidget(parent)
    {
        m_itemBackground = new Plasma::ItemBackground(this);

        m_background = new Plasma::FrameSvg(this);
        m_background->setImagePath("widgets/frame");
        m_background->setElementPrefix("raised");
        setLocation(Plasma::BottomEdge);
        setAcceptHoverEvents(true);
    }

    ~ToolContainer()
    {
    }

    void setLocation(Plasma::Location location)
    {
        m_location = location;
        switch (location) {
        case Plasma::TopEdge:
            m_background->setEnabledBorders(Plasma::FrameSvg::BottomBorder);
            break;
        case Plasma::BottomEdge:
            m_background->setEnabledBorders(Plasma::FrameSvg::TopBorder);
            break;
        case Plasma::LeftEdge:
            m_background->setEnabledBorders(Plasma::FrameSvg::RightBorder);
            break;
        case Plasma::RightEdge:
            m_background->setEnabledBorders(Plasma::FrameSvg::LeftBorder);
            break;
        default:
            m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
            break;
        }
        qreal left, top, right, bottom;
        m_background->getMargins(left, top, right, bottom);
        setContentsMargins(left, top, right, bottom);
    }

    Plasma::ItemBackground *itemBackground() const
    {
        return m_itemBackground;
    }

    Plasma::Location location() const
    {
        return m_location;
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event)
    {
        event->accept();
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
    {
        m_itemBackground->hide();
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option)
        Q_UNUSED(widget)

        m_background->paintFrame(painter);
    }

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event)
    {
        m_background->resizeFrame(event->newSize());
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        event->accept();
    }

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
    {
        QSizeF hint = QGraphicsWidget::sizeHint(which, constraint);

        qreal left, top, right, bottom;
        m_itemBackground->getContentsMargins(&left, &top, &right, &bottom);

        if (which == Qt::PreferredSize) {
            if (m_location == Plasma::TopEdge) {
                hint.setHeight(KIconLoader::SizeSmallMedium + m_background->marginSize(Plasma::BottomMargin) + top + bottom);
            } else if (m_location == Plasma::BottomEdge) {
                hint.setHeight(KIconLoader::SizeSmallMedium + m_background->marginSize(Plasma::TopMargin) + top + bottom);
            }
        }

        return hint;
    }

    bool eventFilter(QObject *watched, QEvent *event)
    {
        Plasma::IconWidget *icon = qobject_cast<Plasma::IconWidget *>(watched);
        if (icon && event->type() == QEvent::GraphicsSceneHoverEnter) {
            m_itemBackground->setTargetItem(icon);
        } else if (icon && event->type() == QEvent::Show) {
            //force the newly shown icon to have a sensible size
            icon->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            layout()->invalidate();
        } else if (icon && event->type() == QEvent::Hide) {
            if (m_location == Plasma::TopEdge || m_location == Plasma::BottomEdge) {
                icon->setMaximumWidth(0);
            } else {
                icon->setMaximumHeight(0);
            }
            layout()->invalidate();
        }
        return false;
    }

private:
    Plasma::FrameSvg *m_background;
    Plasma::ItemBackground *m_itemBackground;
    Plasma::Location m_location;
};

NetToolBox::NetToolBox(Plasma::Containment *parent)
   : QGraphicsWidget(parent),
     m_containment(parent),
     m_icon("plasma"),
     m_iconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall),
     m_animHighlightId(0),
     m_animHighlightFrame(0),
     m_hovering(false),
     m_showing(false),
     m_location(Plasma::BottomEdge)
{
    setZValue(9000);
    resize(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
    setAcceptHoverEvents(true);

    m_toolContainer = new ToolContainer(this);
    m_toolContainer->hide();
    m_toolContainer->setFlag(QGraphicsWidget::ItemStacksBehindParent);
    m_toolContainerLayout = new QGraphicsLinearLayout(m_toolContainer);
    m_toolContainerLayout->addStretch();

    m_background = new Plasma::Svg(this);
    m_background->setImagePath("widgets/toolbox");
    m_background->setContainsMultipleImages(true);
    setLocation(Plasma::BottomEdge);

    connect(m_containment, SIGNAL(geometryChanged()), this, SLOT(containmentGeometryChanged()));
    containmentGeometryChanged();

    connect(Plasma::Animator::self(), SIGNAL(movementFinished(QGraphicsItem*)),
            this, SLOT(movementFinished(QGraphicsItem*)));
}

NetToolBox::~NetToolBox()
{
}

bool NetToolBox::isShowing() const
{
    return m_showing;
}

void NetToolBox::setShowing(const bool show)
{
    m_showing = show;

    if (show != m_toolContainer->isVisible()) {
        emit toggled();
        emit visibilityChanged(show);
    }

    if (show) {
        QPoint finalPos;
        switch (m_location) {
        case Plasma::TopEdge:
            m_toolContainer->setPos(boundingRect().topLeft() - QPoint(0, m_toolContainer->size().height()));
            finalPos = QPoint(0, 0);
            break;
        case Plasma::LeftEdge:
            m_toolContainer->setPos(boundingRect().topLeft() - QPoint(m_toolContainer->size().width(), 0));
            finalPos = QPoint(0, 0);
            break;
        case Plasma::RightEdge:
            m_toolContainer->setPos(boundingRect().topRight());
            finalPos = QPoint(size().width()-m_toolContainer->size().width(), 0);
            break;
        case Plasma::BottomEdge:
        default:
            m_toolContainer->setPos(boundingRect().bottomLeft());
            finalPos = QPoint(0, size().height()-m_toolContainer->size().height());
            break;
        }

        m_toolContainer->show();

        if (m_animSlideId) {
            Plasma::Animator::self()->stopItemMovement(m_animSlideId);
        }

        m_animSlideId = Plasma::Animator::self()->moveItem(m_toolContainer, Plasma::Animator::SlideOutMovement, finalPos);
    } else {
        QPoint finalPos;
        switch (m_location) {
        case Plasma::TopEdge:
            finalPos = QPoint((boundingRect().topLeft() - QPoint(0, m_toolContainer->size().height())).toPoint());
            m_toolContainer->setPos(0, 0);
            break;
        case Plasma::LeftEdge:
            finalPos = QPoint(boundingRect().topLeft().toPoint() - QPoint(m_toolContainer->size().width(), 0));
            m_toolContainer->setPos(size().width()-m_toolContainer->size().width(), 0);
            break;
        case Plasma::RightEdge:
            finalPos = QPoint(boundingRect().topRight().toPoint());
            m_toolContainer->setPos(0, 0);
            break;
        case Plasma::BottomEdge:
        default:
            finalPos = QPoint(boundingRect().bottomLeft().toPoint());
            m_toolContainer->setPos(0, size().height()-m_toolContainer->size().height());
            break;
        }

        if (m_animSlideId) {
            Plasma::Animator::self()->stopItemMovement(m_animSlideId);
        }

        m_animSlideId = Plasma::Animator::self()->moveItem(m_toolContainer, Plasma::Animator::SlideInMovement, finalPos);
    }
}


void NetToolBox::addTool(QAction *action)
{
    Plasma::IconWidget *button = new Plasma::IconWidget(this);
    button->setOrientation(Qt::Horizontal);
    button->setTextBackgroundColor(QColor());
    button->setAction(action);
    button->installEventFilter(m_toolContainer);

    qreal left, top, right, bottom;
    m_toolContainer->itemBackground()->getContentsMargins(&left, &top, &right, &bottom);
    button->setContentsMargins(left, top, right, bottom);

    if (m_location == Plasma::LeftEdge || m_location == Plasma::RightEdge) {
        button->setOrientation(Qt::Vertical);
    } else {
        button->setOrientation(Qt::Horizontal);
    }

    m_actionButtons[action] = button;
    //FIXME: in 4.6 m_toolContainerLayout->count()-1 adds two items before the spacer: intended or qt bug?
    m_toolContainerLayout->insertItem(m_toolContainerLayout->count() , button);
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

void NetToolBox::setLocation(Plasma::Location location)
{
    m_location = location;
    m_toolContainer->setLocation(location);
    if (location == Plasma::LeftEdge || location == Plasma::RightEdge) {
        m_toolContainerLayout->setOrientation(Qt::Vertical);
        m_toolContainerLayout->setContentsMargins(0, size().height(), 0, 0);
        foreach (Plasma::IconWidget *icon, m_actionButtons) {
            icon->setOrientation(Qt::Vertical);
        }
    } else {
        m_toolContainerLayout->setOrientation(Qt::Horizontal);
        m_toolContainerLayout->setContentsMargins(size().width(), 0, 0, 0);
        foreach (Plasma::IconWidget *icon, m_actionButtons) {
            icon->setOrientation(Qt::Horizontal);
        }
    }
    containmentGeometryChanged();
}

Plasma::Location NetToolBox::location() const
{
    return m_location;
}

void NetToolBox::containmentGeometryChanged()
{
    m_toolContainerLayout->invalidate();
    m_toolContainerLayout->activate();

    switch (m_location) {
    case Plasma::TopEdge:
        m_toolContainer->resize(m_containment->size().width(), m_toolContainer->effectiveSizeHint(Qt::PreferredSize).height());
        m_toolContainer->setPos(0, 0);
        setPos(0, 0);
        break;
    case Plasma::BottomEdge:
        m_toolContainer->resize(m_containment->size().width(), m_toolContainer->effectiveSizeHint(Qt::PreferredSize).height());
        m_toolContainer->setPos(0, size().height()-m_toolContainer->size().height());
        setPos(0, m_containment->size().height()-size().height());
        break;
    case Plasma::LeftEdge:
        m_toolContainer->resize(m_toolContainer->effectiveSizeHint(Qt::PreferredSize).width(), m_containment->size().height());
        m_toolContainer->setPos(0, 0);
        setPos(0, 0);
        break;
    case Plasma::RightEdge:
        m_toolContainer->resize(m_toolContainer->effectiveSizeHint(Qt::PreferredSize).width(), m_containment->size().height());
        m_toolContainer->setPos(size().width()-m_toolContainer->size().width(), 0);
        setPos(m_containment->size().width()-size().width(), 0);
        break;
    default:
        m_toolContainer->resize(m_toolContainer->effectiveSizeHint(Qt::PreferredSize));
        m_toolContainer->setPos(0, size().height()-m_toolContainer->size().height());
        break;
    }
}

void NetToolBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void NetToolBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    setShowing(!isShowing());
}

void NetToolBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    QPoint iconPos;
    QString svgElement;

    switch (m_location) {
    case Plasma::TopEdge:
    case Plasma::LeftEdge:
        iconPos = QPoint(2, 2);
        svgElement = "desktop-northwest";
        break;
    case Plasma::RightEdge:
        iconPos = QPoint(size().width() - m_iconSize.width() - 2, 2);
        svgElement = "desktop-northeast";
        break;
    case Plasma::BottomEdge:
    default:
        iconPos = QPoint(2, size().height() - m_iconSize.height() - 2);
        svgElement = "desktop-southwest";
        break;
    }

    m_background->paint(painter, boundingRect(), svgElement);

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
    if (isShowing() || m_hovering) {
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
    if (!m_hovering || isShowing()) {
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

void NetToolBox::movementFinished(QGraphicsItem *item)
{
    Q_UNUSED(item)

    m_animSlideId = 0;
    m_toolContainer->setVisible(m_showing);
}

#include "nettoolbox.moc"
