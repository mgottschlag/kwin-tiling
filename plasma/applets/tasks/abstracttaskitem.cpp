/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

// Own
#include "abstracttaskitem.h"

// Qt
#include <QGraphicsSceneContextMenuEvent>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsView>
#include <QTimer>
#include <QApplication>
#include <QTextLayout>
#include <QGraphicsLinearLayout>

// KDE
#include <KAuthorized>
#include <KDebug>
#include <KIcon>
#include <KIconEffect>
#include <KLocalizedString>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KColorUtils>

#include <taskmanager/task.h>
#include <taskmanager/taskmanager.h>
#include <taskmanager/taskgroup.h>

#include <Plasma/Theme>
#include <Plasma/PaintUtils>
#include <Plasma/FrameSvg>
#include <Plasma/ToolTipManager>

#include "tasks.h"
#include "taskgroupitem.h"
#include "layoutwidget.h"

AbstractTaskItem::AbstractTaskItem(QGraphicsWidget *parent, Tasks *applet, const bool showTooltip)
    : QGraphicsWidget(parent),
      m_abstractItem(0),
      m_applet(applet),
      m_activateTimer(0),
      m_flags(0),
      m_animId(0),
      m_alpha(1),
      m_backgroundPrefix("normal"),
      m_updateTimerId(0),
      m_attentionTimerId(0),
      m_attentionTicks(0),
      m_fadeIn(true),
      m_showTooltip(showTooltip),
      m_showingTooltip(false)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    setAcceptsHoverEvents(true);
    setAcceptDrops(true);

    Plasma::ToolTipManager::self()->registerWidget(this);
    setPreferredSize(basicPreferredSize());
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(syncActiveRect()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(queueUpdate()));
    connect(applet, SIGNAL(settingsChanged()), this, SLOT(checkSettings()));
}

QSize AbstractTaskItem::basicPreferredSize() const
{
    QFontMetrics fm(KGlobalSettings::taskbarFont());
    QSize mSize = fm.size(0, "M");
    int iconsize = KIconLoader::SizeSmall;
    
    //the 4 should be the default spacing between layout items, is there a way to fetch it without hardcoding?
    // in small panels, we'll reduce the spacing a bit so it's easier to cramp the text in and still get two rows
    int topMargin = m_applet->itemTopMargin();
    int bottomMargin = m_applet->itemBottomMargin();
    if (m_applet->size().height() < 44) {
        topMargin = 1; 
        bottomMargin = 1; 
    } else if (m_applet->size().height() < 64) {
        topMargin = qMax(1, topMargin/2); 
        bottomMargin = qMax(1, bottomMargin/2); 
    }
    return QSize(mSize.width()*12 + m_applet->itemLeftMargin() + m_applet->itemRightMargin() + KIconLoader::SizeSmall,
                           qMax(mSize.height(), iconsize) + topMargin + bottomMargin);
}

AbstractTaskItem::~AbstractTaskItem()
{
    if (m_animId) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    Plasma::ToolTipManager::self()->unregisterWidget(this);
}

void AbstractTaskItem::checkSettings()
{
    TaskGroupItem *group = qobject_cast<TaskGroupItem *>(this);

    if (group && !group->collapsed()) {
        m_showTooltip = false;
    } else if (m_showTooltip != m_applet->showTooltip()) {
        m_showTooltip = !m_showTooltip;
    }
}

void AbstractTaskItem::setShowTooltip(const bool showit)
{
    m_showTooltip = showit;
}

void AbstractTaskItem::setText(const QString &text)
{
    m_text = text;
}

void AbstractTaskItem::setIcon(const QIcon &icon)
{
    m_icon = icon; //icon.pixmap(MinTaskIconSize);
}

QIcon AbstractTaskItem::icon() const
{
    return m_icon;
}

QString AbstractTaskItem::text() const
{
    return m_text;
}

void AbstractTaskItem::setTaskFlags(const TaskFlags flags)
{
    if (((m_flags & TaskWantsAttention) != 0) != ((flags & TaskWantsAttention) != 0)) {
        //kDebug() << "task attention state changed" << m_attentionTimerId;
        if (flags & TaskWantsAttention) {
            // start attention getting
            if (!m_attentionTimerId) {
                m_attentionTimerId = startTimer(500);
            }
        } else if (m_attentionTimerId) {
            killTimer(m_attentionTimerId);
            m_attentionTimerId = 0;
            // stop attention getting
        }
    }

    m_flags = flags;

    QString newBackground;
    if (m_flags & TaskIsMinimized) {
        newBackground = "minimized";
    } else if (m_flags & TaskHasFocus) {
        newBackground = "focus";
    } else {
        newBackground = "normal";
    }

    if (newBackground != m_backgroundPrefix) {
        fadeBackground(newBackground, 100, true);
    }
}

void AbstractTaskItem::fadeBackground(const QString &newBackground, int duration, bool fadeIn)
{
    m_oldBackgroundPrefix = m_backgroundPrefix;
    m_backgroundPrefix = newBackground;

    if (m_animId) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    m_fadeIn = fadeIn;
    m_animId = Plasma::Animator::self()->customAnimation(40 / (1000 / duration), duration,Plasma::Animator::LinearCurve, this, "animationUpdate");
}


AbstractTaskItem::TaskFlags AbstractTaskItem::taskFlags() const
{
    return m_flags;
}

void AbstractTaskItem::toolTipAboutToShow()
{
    if (m_showTooltip) {
        m_showingTooltip = true;
        updateToolTip();
    } else {
        Plasma::ToolTipManager::self()->clearContent(this);
    }
}

void AbstractTaskItem::toolTipHidden()
{
    m_showingTooltip = false;
    Plasma::ToolTipManager::self()->clearContent(this);
}

void AbstractTaskItem::queueUpdate()
{
    if (m_updateTimerId || m_attentionTimerId) {
        return;
    }

    if (m_lastUpdate.elapsed() < 200) {
        m_updateTimerId = startTimer(200);
        return;
    }

    update();
    m_lastUpdate.restart();
}

void AbstractTaskItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    fadeBackground("hover", 175, true);
}

void AbstractTaskItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    QString backgroundPrefix;

    if (m_flags & TaskWantsAttention) {
        backgroundPrefix = "attention";
    } else if (m_flags & TaskIsMinimized) {
        backgroundPrefix = "minimized";
    } else if (m_flags & TaskHasFocus) {
        backgroundPrefix = "focus";
    } else {
        backgroundPrefix = "normal";
    }

    fadeBackground(backgroundPrefix, 150, false);
}

void AbstractTaskItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton/* || event->button() == Qt::MidButton*/) {
        activate();
    }
}

void AbstractTaskItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    update();
}

void AbstractTaskItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //kDebug();
    if (QPoint(event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < QApplication::startDragDistance()) {
        return;
    } //Wait a bit before starting drag

  /*  if((m_applet->taskSortOrder() != Tasks::NoSorting) && (m_applet->taskSortOrder() != Tasks::GroupSorting)){ //FIXME check somhow if drag is allowed
        return;
    }*/

    QByteArray data;
    data.resize(sizeof(AbstractTaskItem*));
    AbstractTaskItem *selfPtr = this;
    memcpy(data.data(), &selfPtr, sizeof(AbstractTaskItem*));

    QMimeData* mimeData = new QMimeData();
    setAdditionalMimeData(mimeData);

    QDrag *drag = new QDrag(event->widget());
    drag->setMimeData(mimeData);
    drag->setPixmap(m_icon.pixmap(20));
   // drag->setDragCursor( set the correct cursor //TODO
    drag->exec();
}

void AbstractTaskItem::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_updateTimerId) {
        killTimer(m_updateTimerId);
        update();
        m_updateTimerId = 0;
    } else if (event->timerId() == m_attentionTimerId) {
        ++m_attentionTicks;
        if (m_attentionTicks > 6) {
            killTimer(m_attentionTimerId);
            m_attentionTimerId = 0;
            m_attentionTicks = 0;
        }

        if (m_attentionTicks % 2 == 0) {
            fadeBackground("attention", 100, false);
        } else {
            fadeBackground("normal", 150, false);
        }

        update();
    }
}

void AbstractTaskItem::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem *option,
                             QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);

    // draw background
    drawBackground(painter, option, widget);

    // draw icon and text
    drawTask(painter, option, widget);
}

void AbstractTaskItem::syncActiveRect()
{
    Plasma::FrameSvg *itemBackground = m_applet->itemBackground();

    if (!itemBackground) {
        return;
    }

    itemBackground->setElementPrefix("normal");

    qreal left, top, right, bottom;
    itemBackground->getMargins(left, top, right, bottom);

    itemBackground->setElementPrefix("focus");
    qreal activeLeft, activeTop, activeRight, activeBottom;
    itemBackground->getMargins(activeLeft, activeTop, activeRight, activeBottom);

    m_activeRect = QRectF(QPointF(0, 0), size());
    m_activeRect.adjust(left - activeLeft, top - activeTop,
                        -(right - activeRight), -(bottom - activeBottom));

    itemBackground->setElementPrefix(m_backgroundPrefix);
}

void AbstractTaskItem::drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    // Do not paint with invalid sizes, the happens when the layout is being initialized
    if (!option->rect.isValid()) {
        return;
    }

    /*FIXME -could be done more elegant with caching in tasks in a qhash <size,svg>.
    -do not use size() directly because this introduces the blackline syndrome.
    -This line is only needed when we have different items in the taskbar because of an expanded group for example. otherwise the resizing in the resizeEvent is sufficient
    */
    Plasma::FrameSvg *itemBackground = m_applet->itemBackground();

    if (!itemBackground) {
        return;
    }

    //if the size is changed have to resize all the elements
    if (itemBackground->size() != size().toSize() && itemBackground->size() != m_activeRect.size().toSize()) {
        syncActiveRect();

        itemBackground->setElementPrefix("focus");
        m_applet->resizeItemBackground(m_activeRect.size().toSize());
        itemBackground->setElementPrefix("normal");
        m_applet->resizeItemBackground(size().toSize());
        itemBackground->setElementPrefix("minimized");
        m_applet->resizeItemBackground(size().toSize());
        itemBackground->setElementPrefix("attention");
        m_applet->resizeItemBackground(size().toSize());
        itemBackground->setElementPrefix("hover");
        m_applet->resizeItemBackground(m_activeRect.size().toSize());

        //restore the prefix
        itemBackground->setElementPrefix(m_backgroundPrefix);
    }

   

    if (!m_animId && ~option->state & QStyle::State_Sunken) {
        itemBackground->setElementPrefix(m_backgroundPrefix);
        if (itemBackground->frameSize() == m_activeRect.size().toSize()) {
            itemBackground->paintFrame(painter, m_activeRect.topLeft());
        } else {
            itemBackground->paintFrame(painter);
        }
        //itemBackground->paintFrame(painter, backgroundPosition);
        return;
    }

    itemBackground->setElementPrefix(m_oldBackgroundPrefix);
    QPixmap oldBackground;

    if (option->state & QStyle::State_Sunken) {
        oldBackground = QPixmap(m_activeRect.size().toSize());
        oldBackground.fill(Qt::transparent);
        m_alpha = 0.4;
    } else {
        oldBackground = itemBackground->framePixmap();
    }

    itemBackground->setElementPrefix(m_backgroundPrefix);
    QPixmap result = Plasma::PaintUtils::transition(oldBackground, itemBackground->framePixmap(), m_alpha);

    if (result.size() == m_activeRect.size().toSize()) {
        painter->drawPixmap(m_activeRect.topLeft(), result);
    } else {
        painter->drawPixmap(QPoint(0,0), result);
    }

    return;
}

void AbstractTaskItem::drawTask(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    Q_UNUSED(option)

    QRectF bounds = boundingRect().adjusted(m_applet->itemLeftMargin(), m_applet->itemTopMargin(), -m_applet->itemRightMargin(), -m_applet->itemBottomMargin());

    if ((!m_animId && ~option->state & QStyle::State_MouseOver) ||
         (m_oldBackgroundPrefix != "hover" && m_backgroundPrefix != "hover")) {
        m_icon.paint(painter, iconRect(bounds).toRect());
    } else {
        KIconEffect *effect = KIconLoader::global()->iconEffect();
        QPixmap result = m_icon.pixmap(iconRect(bounds).toRect().size());

        if (effect->hasEffect(KIconLoader::Desktop, KIconLoader::ActiveState)) {
            if (qFuzzyCompare(qreal(1.0), m_alpha)) {
                result = effect->apply(result, KIconLoader::Desktop, KIconLoader::ActiveState);
            } else {
                result = Plasma::PaintUtils::transition(
                    result,
                    effect->apply(result, KIconLoader::Desktop,
                                  KIconLoader::ActiveState), m_fadeIn?m_alpha:1-m_alpha);
            }
        }
        painter->drawPixmap(iconRect(bounds).topLeft(), result);
    }

    painter->setPen(QPen(textColor(), 1.0));

    QRect rect = textRect(bounds).toRect();
    if (rect.height() > 20) {
        rect.adjust(2, 2, -2, -2); // Create a text margin
    }
    QTextLayout layout;
    layout.setFont(KGlobalSettings::taskbarFont());
    layout.setTextOption(textOption());

    layoutText(layout, m_text, rect.size());
    drawTextLayout(painter, layout, rect);

    TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(this);
    if (groupItem) {
        QFont font(KGlobalSettings::smallestReadableFont());
        QFontMetrics fm(font);
        QRectF rect(expanderRect(bounds));

        Plasma::FrameSvg *itemBackground = m_applet->itemBackground();

        if (itemBackground && itemBackground->hasElement(expanderElement())) {
            QSizeF arrowSize(itemBackground->elementSize(expanderElement()));
            QRectF arrowRect(rect.center()-QPointF(arrowSize.width()/2, arrowSize.height()+fm.xHeight()/2), arrowSize);
            itemBackground->paint(painter, arrowRect, expanderElement());

            painter->setFont(font);
            rect.setTop(arrowRect.bottom());
            painter->drawText(rect, Qt::AlignHCenter|Qt::AlignTop, QString::number(groupItem->memberList().count()));
        } else {
            painter->setFont(font);
            painter->drawText(rect, Qt::AlignCenter, QString::number(groupItem->memberList().count()));
        }
    }
}

QTextOption AbstractTaskItem::textOption() const
{
    Qt::LayoutDirection direction = QApplication::layoutDirection();
    Qt::Alignment alignment = QStyle::visualAlignment(direction, Qt::AlignLeft | Qt::AlignVCenter);

    QTextOption option;
    option.setTextDirection(direction);
    option.setAlignment(alignment);

    return option;
}

QSize AbstractTaskItem::layoutText(QTextLayout &layout, const QString &text,
                                   const QSize &constraints) const
{
    QFontMetrics metrics(layout.font());
    int leading     = metrics.leading();
    int height      = 0;
    int maxWidth    = constraints.width();
    int widthUsed   = 0;
    int lineSpacing = metrics.lineSpacing();
    QTextLine line;

    layout.setText(text);

    layout.beginLayout();
    while ((line = layout.createLine()).isValid()) {
        height += leading;

        // Make the last line that will fit infinitely long.
        // drawTextLayout() will handle this by fading the line out
        // if it won't fit in the contraints.
        if (height + 2 * lineSpacing > constraints.height()) {
            line.setPosition(QPoint(0, height));
            break;
        }

        line.setLineWidth(maxWidth);
        line.setPosition(QPoint(0, height));

        height += int(line.height());
        widthUsed = int(qMax(qreal(widthUsed), line.naturalTextWidth()));
    }
    layout.endLayout();

    return QSize(widthUsed, height);
}

void AbstractTaskItem::drawTextLayout(QPainter *painter, const QTextLayout &layout, const QRect &rect) const
{
    if (rect.width() < 1 || rect.height() < 1) {
        return;
    }

    QPixmap pixmap(rect.size());
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setPen(painter->pen());

    // Create the alpha gradient for the fade out effect
    QLinearGradient alphaGradient(0, 0, 1, 0);
    alphaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    if (layout.textOption().textDirection() == Qt::LeftToRight)
    {
        alphaGradient.setColorAt(0, QColor(0, 0, 0, 255));
        alphaGradient.setColorAt(1, QColor(0, 0, 0, 0));
    } else
    {
        alphaGradient.setColorAt(0, QColor(0, 0, 0, 0));
        alphaGradient.setColorAt(1, QColor(0, 0, 0, 255));
    }

    QFontMetrics fm(layout.font());
    int textHeight = layout.lineCount() * fm.lineSpacing();

    QPointF position = textHeight < rect.height() ?
            QPointF(0, (rect.height() - textHeight) / 2 + (fm.tightBoundingRect("M").height() - fm.xHeight())) : QPointF(0, 0);
    QList<QRect> fadeRects;
    int fadeWidth = 30;

    // Draw each line in the layout
    for (int i = 0; i < layout.lineCount(); i++)
    {
        QTextLine line = layout.lineAt(i);
        line.draw(&p, position);

        // Add a fade out rect to the list if the line is too long
        if (line.naturalTextWidth() > rect.width())
        {
            int x = int(qMin(line.naturalTextWidth(), (qreal)pixmap.width())) - fadeWidth;
            int y = int(line.position().y() + position.y());
            QRect r = QStyle::visualRect(layout.textOption().textDirection(), pixmap.rect(),
                                         QRect(x, y, fadeWidth, int(line.height())));
            fadeRects.append(r);
        }
    }

    // Reduce the alpha in each fade out rect using the alpha gradient
    if (!fadeRects.isEmpty())
    {
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        foreach (const QRect &rect, fadeRects) {
            p.fillRect(rect, alphaGradient);
        }
    }

    p.end();


    QColor shadowColor;
    if (textColor().value() < 128) {
        shadowColor = Qt::white;
    } else {
        shadowColor = Qt::black;
    }

    QImage shadow = pixmap.toImage();
    Plasma::PaintUtils::shadowBlur(shadow, 2, shadowColor);

    painter->drawImage(rect.topLeft() + QPoint(1,2), shadow);
    painter->drawPixmap(rect.topLeft(), pixmap);
}



void AbstractTaskItem::animationUpdate(qreal progress)
{
    if (qFuzzyCompare(qreal(1.0), progress)) {
        m_animId = 0;
        m_fadeIn = true;
    }

    m_alpha = progress;

    // explicit update
    update();
}

void AbstractTaskItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat(TaskManager::Task::mimetype()) ||
        event->mimeData()->hasFormat(TaskManager::Task::groupMimetype())) {
        event->ignore(); //ignore it so the taskbar gets the event
        return;
    }

    event->accept();

    if (!m_activateTimer) {
        m_activateTimer = new QTimer(this);
        m_activateTimer->setSingleShot(true);
        m_activateTimer->setInterval(300);
        connect(m_activateTimer, SIGNAL(timeout()), this, SLOT(activate()));
    }
    m_activateTimer->start();
}

void AbstractTaskItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    // restart the timer so that activate() is only called after the mouse
    // stops moving
    if (m_activateTimer) {
        m_activateTimer->start();
    }
}

void AbstractTaskItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    if (m_activateTimer) {
        delete m_activateTimer;
        m_activateTimer = 0;
    }
}

QRect AbstractTaskItem::iconGeometry() const
{
    if (!scene() || !boundingRect().isValid()) {
        return QRect();
    }

    QGraphicsView *parentView = 0;
    QGraphicsView *possibleParentView = 0;
    // The following was taken from Plasma::Applet, it doesn't make sense to make the item an applet, and this was the easiest way around it.
    foreach (QGraphicsView *view, scene()->views()) {
        if (view->sceneRect().intersects(sceneBoundingRect()) ||
            view->sceneRect().contains(scenePos())) {
            if (view->isActiveWindow()) {
                parentView = view;
                break;
            } else {
                possibleParentView = view;
            }
        }
    }

    if (!parentView) {
        parentView = possibleParentView;

        if (!parentView) {
            return QRect();
        }
    }

    QRect rect = parentView->mapFromScene(mapToScene(boundingRect())).boundingRect().adjusted(0, 0, 1, 1);
    rect.moveTopLeft(parentView->mapToGlobal(rect.topLeft()));
    return rect;
}

void AbstractTaskItem::publishIconGeometry() const
{
}

void AbstractTaskItem::publishIconGeometry(const QRect &rect) const
{
}

void AbstractTaskItem::setGeometry(const QRectF& geometry)
{
    QGraphicsWidget::setGeometry(geometry);
    publishIconGeometry();
}

QRectF AbstractTaskItem::iconRect(const QRectF &b) const
{
    QRectF bounds(b);
    const int right = bounds.right();
    //leave enough space for the text. useful in vertical panel
    bounds.setWidth(qMax(bounds.width() / 3, qMin(minimumSize().height(), bounds.width())));

    //restore right position if the layout is RTL
    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        bounds.moveRight(right);
    }

    QSize iconSize = m_icon.actualSize(bounds.size().toSize());

    return QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter,
                               iconSize, bounds.toRect());
}

QRectF AbstractTaskItem::expanderRect(const QRectF &bounds) const
{
    const TaskGroupItem *groupItem = qobject_cast<const TaskGroupItem *>(this);
    if (!groupItem) {
        return QRectF();
    }

    QFontMetrics fm(KGlobalSettings::smallestReadableFont());
    Plasma::FrameSvg *itemBackground = m_applet->itemBackground();

    QSize expanderSize(qMax(fm.width(QString::number(groupItem->memberList().count())),
                       itemBackground ?  itemBackground->elementSize(expanderElement()).width() : 0),
                       size().height());

    return QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignRight | Qt::AlignVCenter,
                               expanderSize, bounds.toRect());
}

QRectF AbstractTaskItem::textRect(const QRectF &bounds) const
{
    QSize size(bounds.size().toSize());
    QRectF effectiveBounds(bounds);

    size.rwidth() -= int(iconRect(bounds).width()) + qMax(0, IconTextSpacing - 2);
    if (!isWindowItem()) {
        size.rwidth() -= int(expanderRect(bounds).width()) + qMax(0, IconTextSpacing - 2);

        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            effectiveBounds.setLeft(expanderRect(bounds).right());
        } else {
            effectiveBounds.setRight(expanderRect(bounds).left());
        }
    }

    return QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignRight | Qt::AlignVCenter,
                                     size, effectiveBounds.toRect());
}

QColor AbstractTaskItem::textColor() const
{
    QColor color;
    qreal bias;
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();

    if ((m_oldBackgroundPrefix == "attention" || m_backgroundPrefix == "attention") &&
        m_applet->itemBackground() && m_applet->itemBackground()->hasElement("hint-attention-button-color")) {
        if (!m_animId && m_backgroundPrefix != "attention") {
            color = theme->color(Plasma::Theme::TextColor);
        } else if (!m_animId) {
            color = theme->color(Plasma::Theme::ButtonTextColor);
        } else {
            if (m_oldBackgroundPrefix == "attention") {
                bias = 1 - m_alpha;
            } else {
                bias = m_alpha;
            }

            color = KColorUtils::mix(theme->color(Plasma::Theme::TextColor),
                                     theme->color(Plasma::Theme::ButtonTextColor), bias);
        }
    } else {
        color = theme->color(Plasma::Theme::TextColor);
    }

    if (m_flags & TaskIsMinimized) {
        color.setAlphaF(0.85);
    }

    return color;
}

QString AbstractTaskItem::expanderElement() const
{
    switch (m_applet->location()) {
    case Plasma::TopEdge:
        return "group-expander-top";
    case Plasma::RightEdge:
        return "group-expander-right";
    case Plasma::LeftEdge:
        return "group-expander-left";
    case Plasma::BottomEdge:
    default:
        return "group-expander-bottom";
    }
}


bool AbstractTaskItem::isGroupMember(const TaskGroupItem *group) const
{
    if (!m_abstractItem || !group) {
        kDebug() <<"no task";
        return false;
    }

    return m_abstractItem->isGroupMember(group->group());

}

bool AbstractTaskItem::isGrouped() const
{
    if (!m_abstractItem) {
        kDebug() <<"no item";
        return false;
    }
    return m_abstractItem->isGrouped();
}

TaskGroupItem * AbstractTaskItem::parentGroup() const
{
    return qobject_cast<TaskGroupItem*>(parentWidget());
}

TaskManager::AbstractItemPtr AbstractTaskItem::abstractItem()
{
    return m_abstractItem;
}

#include "abstracttaskitem.moc"
