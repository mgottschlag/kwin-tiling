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
#include <QAction>

#include <KIconLoader>

#include <Plasma/Animation>
#include <Plasma/Containment>
#include <Plasma/IconWidget>
#include <Plasma/ItemBackground>
#include <Plasma/PaintUtils>
#include <Plasma/Svg>


class ToolContainer : public QGraphicsWidget
{
public:
    ToolContainer(QGraphicsWidget *parent)
         : QGraphicsWidget(parent)
    {
        m_itemBackground = new Plasma::ItemBackground(this);
        m_itemBackground->hide();

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
        Q_UNUSED(event);
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

        if (which == Qt::PreferredSize) {
            qreal left, top, right, bottom;
            m_itemBackground->getContentsMargins(&left, &top, &right, &bottom);

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
        if (icon) {
           if (event->type() == QEvent::GraphicsSceneHoverEnter) {
               m_itemBackground->setTargetItem(icon);
           } else if (event->type() == QEvent::Show) {
               //force the newly shown icon to have a sensible size
               icon->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
               layout()->invalidate();
           } else if (event->type() == QEvent::Hide) {
               if (m_location == Plasma::TopEdge || m_location == Plasma::BottomEdge) {
                   icon->setMaximumWidth(0);
               } else {
                   icon->setMaximumHeight(0);
               }
               layout()->invalidate();
           }
        }
        return false;
    }


private:
    Plasma::FrameSvg *m_background;
    Plasma::ItemBackground *m_itemBackground;
    Plasma::Location m_location;
};

NetToolBox::NetToolBox(Plasma::Containment *parent)
   : Plasma::AbstractToolBox(parent)
{
    init();
}

NetToolBox::NetToolBox(QObject *parent, const QVariantList &args)
    : AbstractToolBox(parent, args)
{
    init();
}

NetToolBox::~NetToolBox()
{
}

void NetToolBox::init()
{
    m_containment = containment();
    Q_ASSERT(m_containment);

    m_icon = KIcon("plasma");
    m_closeIcon = KIcon("dialog-close");
    m_iconSize = QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    m_animHighlightFrame = 0;
    m_hovering = false;
    m_showing = false;
    m_location = Plasma::BottomEdge;
    m_newToolsPosition = 0;

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

    m_containment->installEventFilter(this);
    connect(m_containment, SIGNAL(geometryChanged()), this, SLOT(containmentGeometryChanged()));
    containmentGeometryChanged();

    slideAnim = Plasma::Animator::create(Plasma::Animator::SlideAnimation, this);
    slideAnim->setProperty("movementDirection", Plasma::Animation::MoveAny);
    connect(slideAnim, SIGNAL(stateChanged(QAbstractAnimation::State,
                        QAbstractAnimation::State)),
                this, SLOT(onMovement(QAbstractAnimation::State,QAbstractAnimation::State)));
    connect(slideAnim, SIGNAL(finished()), this, SLOT(movementFinished()));

    anim = new QPropertyAnimation(this, "highlight", this);
    anim->setDuration(250);
    anim->setStartValue(0);
    anim->setEndValue(1);
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
        qreal left, top, right, bottom = 0;

        switch (m_location) {
        case Plasma::TopEdge:
            m_toolContainer->setPos(boundingRect().topLeft() - QPoint(0, m_toolContainer->size().height()));
            slideAnim->setProperty("distancePointF", QPointF(0, m_toolContainer->size().height()));
            top = m_toolContainer->size().height();
            break;
        case Plasma::LeftEdge:
            m_toolContainer->setPos(boundingRect().topLeft() - QPoint(m_toolContainer->size().width(), 0));
            slideAnim->setProperty("distancePointF", QPointF(m_toolContainer->size().width(), 0));
            left = m_toolContainer->size().width();
            break;
        case Plasma::RightEdge:
            m_toolContainer->setPos(boundingRect().topRight());
            slideAnim->setProperty("distancePointF", QPointF(-m_toolContainer->size().width(), 0));
            right = m_toolContainer->size().width();
            break;
        case Plasma::BottomEdge:
        default:
            m_toolContainer->setPos(boundingRect().bottomLeft());
            slideAnim->setProperty("distancePointF", QPointF(0, -m_toolContainer->size().height()));
            bottom = m_toolContainer->size().height();
            break;
        }

        slideAnim->setTargetWidget(m_toolContainer);
        slideAnim->setDirection(QAbstractAnimation::Forward);
        slideAnim->start();

        if (m_containment->layout()) {
            m_containment->layout()->setContentsMargins(left, top, right, bottom);
        }

    } else {
        slideAnim->setDirection(QAbstractAnimation::Backward);
        slideAnim->start();

        if (m_containment->layout()) {
            m_containment->layout()->setContentsMargins(0, 0, 0, 0);
        }
    }
}


void NetToolBox::addTool(QAction *action)
{
    Plasma::IconWidget *button = new Plasma::IconWidget(this);
    button->setOrientation(Qt::Horizontal);
    button->setTextBackgroundColor(QColor());
    button->installEventFilter(m_toolContainer);
    button->setAction(action);

    qreal left, top, right, bottom;
    m_toolContainer->itemBackground()->getContentsMargins(&left, &top, &right, &bottom);
    button->setContentsMargins(left, top, right, bottom);

    if (m_location == Plasma::LeftEdge || m_location == Plasma::RightEdge) {
        button->setOrientation(Qt::Vertical);
    } else {
        button->setOrientation(Qt::Horizontal);
    }

    m_actionButtons[action] = button;

    if (action == m_containment->action("remove")) {
        m_toolContainerLayout->addItem(button);
        --m_newToolsPosition;
    } else if (action == m_containment->action("add page")) {
        m_toolContainerLayout->insertItem(m_newToolsPosition+1, button);
        --m_newToolsPosition;
    } else if (action == m_containment->action("add applications")) {
        m_toolContainerLayout->insertItem(1, button);
        --m_newToolsPosition;
    } else {
        m_toolContainerLayout->insertItem(m_newToolsPosition, button);
    }
    ++m_newToolsPosition;

    if (m_toolContainerLayout->count() == 1) {
        m_toolContainer->itemBackground()->setTargetItem(button);
    }
}

void NetToolBox::removeTool(QAction *action)
{
    if (m_actionButtons.contains(action)) {
        Plasma::IconWidget *button = m_actionButtons.value(action);
        m_toolContainerLayout->removeItem(button);
        m_actionButtons.remove(action);
        button->deleteLater();
        if (action != m_containment->action("remove") ||
            action != m_containment->action("add page")) {
            --m_newToolsPosition;
        }
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
        setPos(m_containment->contentsRect().topLeft());
        break;
    case Plasma::BottomEdge:
        m_toolContainer->resize(m_containment->size().width(), m_toolContainer->effectiveSizeHint(Qt::PreferredSize).height());
        m_toolContainer->setPos(0, size().height()-m_toolContainer->size().height());
        setPos(m_containment->contentsRect().left(), m_containment->contentsRect().bottom()-size().height()+1);
        break;
    case Plasma::LeftEdge:
        m_toolContainer->resize(m_toolContainer->effectiveSizeHint(Qt::PreferredSize).width(), m_containment->size().height());
        m_toolContainer->setPos(0, 0);
        setPos(m_containment->contentsRect().topLeft());
        break;
    case Plasma::RightEdge:
        m_toolContainer->resize(m_toolContainer->effectiveSizeHint(Qt::PreferredSize).width(), m_containment->size().height());
        m_toolContainer->setPos(size().width()-m_toolContainer->size().width(), 0);
        setPos(m_containment->contentsRect().right()-size().width()+1, m_containment->contentsRect().top());
        break;
    default:
        m_toolContainer->resize(m_toolContainer->effectiveSizeHint(Qt::PreferredSize));
        m_toolContainer->setPos(m_containment->contentsRect().left(), size().height()-m_toolContainer->size().height());
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

    KIcon icon;
    if (isShowing()) {
        icon = m_closeIcon;
    } else {
        icon = m_icon;
    }

    if (qFuzzyCompare(qreal(1.0), m_animHighlightFrame)) {
       icon.paint(painter, QRect(iconPos, m_iconSize));
    } else if (qFuzzyCompare(qreal(1.0), 1 + m_animHighlightFrame)) {
        icon.paint(painter, QRect(iconPos, m_iconSize),
                      Qt::AlignCenter, QIcon::Disabled, QIcon::Off);
    } else {
        QPixmap disabled = icon.pixmap(m_iconSize, QIcon::Disabled, QIcon::Off);
        QPixmap enabled = icon.pixmap(m_iconSize);
        QPixmap result = Plasma::PaintUtils::transition(
            icon.pixmap(m_iconSize, QIcon::Disabled, QIcon::Off),
            icon.pixmap(m_iconSize), m_animHighlightFrame);
        painter->drawPixmap(QRect(iconPos, m_iconSize), result);
    }
}

bool NetToolBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_containment && event->type() == QEvent::ContentsRectChange) {
        qreal left, top, right, bottom;
        m_containment->getContentsMargins(&left, &top, &right, &bottom);

        //left preferred over right
        if (left > top && left > right && left > bottom) {
            setLocation(Plasma::RightEdge);
        } else if (right > top && right >= left && right > bottom) {
            setLocation(Plasma::LeftEdge);
        } else if (bottom > top && bottom > left && bottom > right) {
            setLocation(Plasma::TopEdge);
        //bottom is the default
        } else {
            setLocation(Plasma::BottomEdge);
        }
    }

    return AbstractToolBox::eventFilter(watched, event);
}

void NetToolBox::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (isShowing() || m_hovering) {
        QGraphicsWidget::hoverEnterEvent(event);
        return;
    }

    highlight(true);

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

    highlight(false);

    QGraphicsWidget::hoverLeaveEvent(event);
}

void NetToolBox::highlight(bool highlighting)
{
    if (m_hovering == highlighting) {
        return;
    }

    m_hovering = highlighting;

    if (anim->state() != QAbstractAnimation::Stopped) {
        anim->stop();
    }

    anim->start();
}

void NetToolBox::setHighlight(qreal progress)
{
    if (m_hovering) {
        m_animHighlightFrame = progress;
    } else {
        m_animHighlightFrame = 1.0 - progress;
    }

    update();
}

qreal NetToolBox::highlight()
{
    return m_animHighlightFrame;
}

void NetToolBox::movementFinished()
{
    if (slideAnim) {
        if (slideAnim->property("direction") == QAbstractAnimation::Forward) {
            slideAnim->setProperty("direction", QAbstractAnimation::Backward);
        } else {
            slideAnim->setProperty("direction", QAbstractAnimation::Forward);
        }
    }
    m_toolContainer->setVisible(m_showing);
}

void NetToolBox::onMovement(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    Q_UNUSED(newState);
    Q_UNUSED(oldState);
    m_toolContainer->show();
}

#include "nettoolbox.moc"
