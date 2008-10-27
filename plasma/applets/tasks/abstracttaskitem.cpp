/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
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

#include <taskmanager/task.h>
#include <taskmanager/taskmanager.h>
#include <taskmanager/taskgroup.h>

#include "plasma/theme.h"
#include "plasma/paintutils.h"
#include "plasma/panelsvg.h"
#include "plasma/tooltipmanager.h"

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
}

QSize AbstractTaskItem::basicPreferredSize() const
{
    QFontMetrics fm(KGlobalSettings::taskbarFont());
    QSize mSize = fm.size(0, "M");
    //the 4 should be the default spacing between layout items, is there a way to fetch it without hardcoding?
    return QSize(mSize.width()*12 + m_applet->itemLeftMargin() + m_applet->itemRightMargin() + KIconLoader::SizeSmall,
                           KIconLoader::SizeSmall + m_applet->itemTopMargin() + m_applet->itemBottomMargin() + 4);
}
AbstractTaskItem::~AbstractTaskItem()
{
    Plasma::ToolTipManager::self()->unregisterWidget(this);
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
    Plasma::ToolTipManager::Content data;
    Plasma::ToolTipManager::self()->setContent(this, data);
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

    const int FadeInDuration = 75;

    if (m_animId) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    m_fadeIn = true;
    m_animId = Plasma::Animator::self()->customAnimation(40 / (1000 / FadeInDuration), FadeInDuration,Plasma::Animator::LinearCurve, this, "animationUpdate");
}

void AbstractTaskItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    const int FadeOutDuration = 150;

    if (m_animId) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    m_fadeIn = false;
    m_animId = Plasma::Animator::self()->customAnimation(40 / (1000 / FadeOutDuration), FadeOutDuration,Plasma::Animator::LinearCurve, this, "animationUpdate");
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
    mimeData->setData("taskbar/taskItem", data);

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

        update();
    }
}

void AbstractTaskItem::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    //kDebug();
    // we need to lose precision here because all our drawing later on
    // is done with ints, not floats
    m_applet->resizeItemBackground(event->newSize().toSize());
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

void AbstractTaskItem::drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
// FIXME  Check the usage of KColorScheme here with various color schemes

    // Do not paint with invalid sizes, the happens when the layout is being initialized
    if (!option->rect.isValid()) {
        return;
    }

    const qreal hoverAlpha = 0.4;
    bool hasSvg = false;

    /*FIXME -could be done more elegant with caching in tasks in a qhash <size,svg>.
    -do not use size() directly because this introduces the blackline syndrome.
    -This line is only needed when we have different items in the taskbar because of an expanded group for example. otherwise the resizing in the resizeEvent is sufficient
    */
    m_applet->resizeItemBackground(geometry().size().toSize());
    Plasma::PanelSvg *itemBackground = m_applet->itemBackground();

    if ((m_flags & TaskWantsAttention) && !(m_attentionTicks % 2)) {
        if (itemBackground && itemBackground->hasElementPrefix("attention")) {
            //Draw task background from theme svg "attention" element
            itemBackground->setElementPrefix("attention");
            hasSvg = true;
        } else {
            //Draw task background without svg theming
            QColor background = m_applet->colorScheme()->background(KColorScheme::ActiveBackground).color();
            background.setAlphaF(hoverAlpha+0.2);
            painter->setBrush(QBrush(background));
            painter->drawPath(Plasma::PaintUtils::roundedRectangle(option->rect, 6));
        }
    } else if (m_flags & TaskIsMinimized) {
        if (itemBackground && itemBackground->hasElementPrefix("minimized")) {
            //Draw task background from theme svg "attention" element
            itemBackground->setElementPrefix("minimized");
            hasSvg = true;
        } else {
            //Not painting anything by default
            painter->setBrush(QBrush());
        }
    } else if (m_flags & TaskHasFocus) {
            if (itemBackground && itemBackground->hasElementPrefix("focus")) {
                //Draw task background from theme svg "focus" element
                itemBackground->setElementPrefix("focus");
                hasSvg = true;
            } else {
                //Draw task background without svg theming
                QLinearGradient background(boundingRect().topLeft(), boundingRect().bottomLeft());

                QColor startColor = m_applet->colorScheme()->background(KColorScheme::NormalBackground).color();
                QColor endColor = m_applet->colorScheme()->shade(startColor,KColorScheme::DarkShade);

                endColor.setAlphaF(qMin(0.8,startColor.alphaF()+0.2));
                startColor.setAlphaF(0);

                background.setColorAt(0, startColor);
                background.setColorAt(1, endColor);

                painter->setBrush(background);
                painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));

                painter->drawPath(Plasma::PaintUtils::roundedRectangle(option->rect, 6));
            }
    //Default is a normal task
    } else {
        if (itemBackground && itemBackground->hasElementPrefix("normal")) {
            //Draw task background from theme svg "normal" element
            itemBackground->setElementPrefix("normal");
            hasSvg = true;
        } else {
            //Draw task background without svg theming
            KColorScheme *colorScheme = m_applet->colorScheme();
            QColor background = colorScheme->shade(colorScheme->background(KColorScheme::AlternateBackground).color(),
                                                   KColorScheme::DarkShade);
            background.setAlphaF(0.2);
            painter->setBrush(QBrush(background));
            painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));

            painter->drawPath(Plasma::PaintUtils::roundedRectangle(option->rect, 6));
        }
    }

    //Draw task background fading away if needed
    if (hasSvg && !m_animId && ~option->state & QStyle::State_MouseOver) {
        itemBackground->paintPanel(painter);
    }

    if (option->state & QStyle::State_MouseOver || m_animId) {
        if (itemBackground && itemBackground->hasElementPrefix("hover")) {
            if ((!m_animId || m_alpha == 1) && (~option->state & QStyle::State_Sunken)) {
                itemBackground->setElementPrefix("hover");
                itemBackground->paintPanel(painter);
            } else {
                if (hasSvg) {
                    QPixmap normal(itemBackground->panelPixmap());
                    itemBackground->setElementPrefix("hover");
                    QPixmap result = Plasma::PaintUtils::transition(normal, itemBackground->panelPixmap(), m_alpha);
                    painter->drawPixmap(QPoint(0, 0), result);
                } else {
                    //Draw task background from theme svg "hover" element
                    QPixmap *alphaPixmap = m_applet->taskAlphaPixmap(itemBackground->panelSize().toSize());

                    if (option->state & QStyle::State_Sunken) {
                        alphaPixmap->fill(QColor(0, 0, 0, 50));
                    } else if (m_alpha < 0.9) {
                        alphaPixmap->fill(QColor(0, 0, 0, 255 * m_alpha));
                    } else {
                        alphaPixmap->fill(Qt::transparent);
                    }

                    {
                        QPainter buffPainter(alphaPixmap);
                        buffPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
                        itemBackground->setElementPrefix("hover");
                        itemBackground->paintPanel(&buffPainter);
                    }

                    painter->drawPixmap(QPoint(0, 0), *alphaPixmap);
                }
            }
        } else {
            //Draw task background without svg theming
            QLinearGradient background(boundingRect().topLeft(),
                                       boundingRect().bottomLeft());

            QColor startColor = m_applet->colorScheme()->background(KColorScheme::AlternateBackground).color();
            QColor endColor = m_applet->colorScheme()->shade(startColor,KColorScheme::DarkShade);

            const qreal pressedAlpha = 0.2;

            qreal alpha = 0;

            if (option->state & QStyle::State_Sunken) {
                alpha = pressedAlpha;
            } else {
                alpha = hoverAlpha;
            }

            startColor.setAlphaF(alpha);
            endColor.setAlphaF(m_alpha);

            background.setColorAt(0, startColor);
            background.setColorAt(1, endColor);

            painter->setBrush(background);
            painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));

            painter->drawPath(Plasma::PaintUtils::roundedRectangle(option->rect, 6));
        }
    }
}

void AbstractTaskItem::drawTask(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *)
{
    Q_UNUSED(option)

    QRectF bounds = boundingRect().adjusted(m_applet->itemLeftMargin(), m_applet->itemTopMargin(), -m_applet->itemRightMargin(), -m_applet->itemBottomMargin());

    if (!m_animId && !(option->state & QStyle::State_MouseOver)) {
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
                                  KIconLoader::ActiveState), m_alpha);
            }
        }
        painter->drawPixmap(iconRect(bounds).topLeft(), result);
    }

    if (m_flags & TaskHasFocus && m_applet->itemBackground()->hasElement("hint-focus-is-button")) {
        painter->setPen(QPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor), 1.0));
    } else {
        painter->setPen(QPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor), 1.0));
    }

    QRect rect = textRect(bounds).toRect();
    if (rect.height() > 20) {
        rect.adjust(2, 2, -2, -2); // Create a text margin
    }
    QTextLayout layout;
    layout.setFont(KGlobalSettings::taskbarFont());
    layout.setTextOption(textOption());

    layoutText(layout, m_text, rect.size());
    drawTextLayout(painter, layout, rect);

    if (!isWindowItem()) {
        m_applet->itemBackground()->paint(painter, expanderRect(bounds), expanderElement());
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
    while ((line = layout.createLine()).isValid())
    {
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
            QPointF(0, (rect.height() - textHeight) / 2) : QPointF(0, 0);
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
    if (m_flags & TaskHasFocus && m_applet->itemBackground()->hasElement("hint-focus-is-button")) {
        shadowColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonBackgroundColor);
    } else {
        shadowColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    }

    QImage shadow = pixmap.toImage();
    Plasma::PaintUtils::shadowBlur(shadow, 3, shadowColor);

    painter->drawImage(rect.topLeft() + QPoint(2,2), shadow);
    painter->drawPixmap(rect.topLeft(), pixmap);
}



void AbstractTaskItem::animationUpdate(qreal progress)
{
    if (progress == 1) {
        m_animId = 0;
        m_fadeIn = true;
    }

    m_alpha = m_fadeIn ? progress : 1 - progress;

    // explicit update
    update();
}






void AbstractTaskItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{

    if (event->mimeData()->hasFormat("taskbar/taskItem")) {
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

AbstractTaskItem * AbstractTaskItem::decodeMimedata(const QMimeData *mime)
{
    // Although we use the a baseclass pointer, all virtual functions are called
    // in the derived class version, but really only the virtual ones!
    AbstractTaskItem *taskItem = 0;
    if (mime->hasFormat("taskbar/taskItem")) { //make modifier configurable
        QByteArray data(mime->data("taskbar/taskItem"));

        if (data.size() == sizeof(AbstractTaskItem*)) {
            memcpy(&taskItem, data.data(), sizeof(AbstractTaskItem*));
        }
    }

    return taskItem;
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
    QSize expanderSize = m_applet->itemBackground()->elementSize(expanderElement());

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

//inform parent about removal
void AbstractTaskItem::finished()
{
    //kDebug();
   // if (hasGroup())
    m_applet->removeItem(this);
    deleteLater();
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
    if (!m_abstractItem) {
        kDebug() << "no task";
        return 0;
    }
    //return dynamic_cast<TaskManager::TaskGroup *>(m_abstractItem->parentGroup());
    TaskGroupItem * group = m_applet->groupItem(m_abstractItem->parentGroup());
    /*
    if (!group) {
        kDebug() << "null group";
    }
    */

    return group;
}


TaskManager::AbstractItemPtr AbstractTaskItem::abstractItem()
{
    return m_abstractItem;
}

#include "abstracttaskitem.moc"
