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
#include "windowtaskitem.h"

// Qt
#include <QGraphicsSceneContextMenuEvent>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsView>
#include <QTimer>
#include <QApplication>

// KDE
#include <KAuthorized>
#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <KGlobalSettings>
#include <KIconLoader>

#include <taskmanager/taskrmbmenu.h>

#include "plasma/theme.h"
#include "plasma/paintutils.h"
#include "plasma/panelsvg.h"

#include "tasks.h"

WindowTaskItem::WindowTaskItem(Tasks *parent, const bool showTooltip)
    : QGraphicsWidget(parent),
      m_applet(parent),
      m_activateTimer(0),
      m_flags(0),
      m_animId(0),
      m_alpha(1),
      m_fadeIn(true),
      m_updateTimerId(0),
      m_attentionTimerId(0),
      m_attentionTicks(0)
{
    m_showTooltip = showTooltip;
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    setAcceptsHoverEvents(true);
    setAcceptDrops(true);
    
    QFontMetrics fm(KGlobalSettings::taskbarFont());
    QSize mSize = fm.size(0, "M");
    setPreferredSize(QSize(mSize.width()*15 + m_applet->itemLeftMargin() + m_applet->itemRightMargin() + IconSize(KIconLoader::Panel),
                           mSize.height()*3 + m_applet->itemTopMargin() + m_applet->itemBottomMargin()));
}

void WindowTaskItem::activate()
{
    // the Task class has a method called activateRaiseOrIconify() which
    // should perform the required action here.
    //
    // however it currently does not minimize the task's window if the item
    // is clicked whilst the window is active probably because the active window by
    // the time the mouse is released over the window task button is not the
    // task's window but instead the desktop window
    //
    // TODO: the Kicker panel in KDE 3.x has a feature whereby clicking on it
    // does not take away the focus from the active window (unless clicking
    // in a widget such as a line edit which does accept the focus)
    // this needs to be implemented for Plasma's own panels.
    if (m_task) {
        m_task->activateRaiseOrIconify();
        emit windowSelected(this);
    }
}

void WindowTaskItem::close()
{
    if (m_task) {
        m_task->close();
    }
}

void WindowTaskItem::setShowTooltip(const bool showit)
{
    m_showTooltip = showit;
    updateTask();
}

void WindowTaskItem::setText(const QString &text)
{
    m_text = text;
}

void WindowTaskItem::setIcon(const QIcon &icon)
{
    m_icon = icon; //icon.pixmap(MinTaskIconSize);
}

void WindowTaskItem::setTaskFlags(TaskFlags flags)
{
    if ((m_flags & TaskWantsAttention != 0) != (flags & TaskWantsAttention != 0)) {
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

WindowTaskItem::TaskFlags WindowTaskItem::taskFlags() const
{
    return m_flags;
}

void WindowTaskItem::queueUpdate()
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

void WindowTaskItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    const int FadeInDuration = 75;

    if (m_animId) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    m_fadeIn = true;
    m_animId = Plasma::Animator::self()->customAnimation(40 / (1000 / FadeInDuration), FadeInDuration,Plasma::Animator::LinearCurve, this, "animationUpdate");

    QGraphicsWidget::hoverEnterEvent(event);
}

void WindowTaskItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    const int FadeOutDuration = 150;

    if (m_animId) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    m_fadeIn = false;
    m_animId = Plasma::Animator::self()->customAnimation(40 / (1000 / FadeOutDuration), FadeOutDuration,Plasma::Animator::LinearCurve, this, "animationUpdate");

    QGraphicsWidget::hoverLeaveEvent(event);
}

void WindowTaskItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    activate();
}

void WindowTaskItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    update();
}

void WindowTaskItem::timerEvent(QTimerEvent *event)
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

void WindowTaskItem::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    m_applet->resizeItemBackground(event->newSize());
}

void WindowTaskItem::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem *option,
                             QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);

    // draw background
    drawBackground(painter, option, widget);

    // draw icon and text
    drawTask(painter, option, widget);
}

void WindowTaskItem::drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    // FIXME  Check the usage of KColorScheme here with various color schemes

    //Don't paint with invalid sizes, the happens when the layout i's being initialized
    if (!option->rect.isValid()) {
        return;
    }

    const qreal hoverAlpha = 0.4;
    bool hasSvg = false;

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
    if (hasSvg) {
        if (!m_animId) {
             if (~option->state & QStyle::State_MouseOver) {
                 itemBackground->paintPanel(painter, option->rect);
             }
        } else {
            QPixmap *alphaPixmap = m_applet->taskAlphaPixmap(option->rect.size());
            //kDebug() << (QObject*)this << "setting alpha to" << (255 * (1.0 - m_alpha)) << m_alpha;
            alphaPixmap->fill(QColor(0, 0, 0, 255 * (1.0 - m_alpha)));

            {
                QPainter buffPainter(alphaPixmap);
                buffPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
                itemBackground->paintPanel(&buffPainter, option->rect);
            }

            painter->drawPixmap(option->rect.topLeft(), *alphaPixmap);
        }
    }

    if (option->state & QStyle::State_MouseOver || m_animId) {
        if (itemBackground && itemBackground->hasElementPrefix("hover")) {
            if ((!m_animId || m_alpha == 1) && (~option->state & QStyle::State_Sunken)) {
                itemBackground->setElementPrefix("hover");
                itemBackground->paintPanel(painter, option->rect);
            } else {
                //Draw task background from theme svg "hover" element
                QPixmap *alphaPixmap = m_applet->taskAlphaPixmap(option->rect.size());

                if (option->state & QStyle::State_Sunken) {
                    alphaPixmap->fill(QColor(0, 0, 0, 50));
                } else {
                    alphaPixmap->fill(QColor(0, 0, 0, 255 * m_alpha));
                }

                {
                    QPainter buffPainter(alphaPixmap);
                    buffPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
                    itemBackground->setElementPrefix("hover");
                    itemBackground->paintPanel(&buffPainter, option->rect);
                }

                painter->drawPixmap(option->rect.topLeft(), *alphaPixmap);
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

void WindowTaskItem::drawTask(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *)
{
    Q_UNUSED(option)

    QRectF bounds = boundingRect().adjusted(m_applet->itemLeftMargin(), m_applet->itemTopMargin(), -m_applet->itemRightMargin(), -m_applet->itemBottomMargin());
    m_icon.paint(painter, iconRect(bounds).toRect());

    painter->setPen(QPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor), 1.0));

    QRect rect = textRect(bounds).toRect();
    if (rect.height() > 20) {
        rect.adjust(2, 2, -2, -2); // Create a text margin
    }
    QTextLayout layout;
    layout.setFont(KGlobalSettings::taskbarFont());
    layout.setTextOption(textOption());

    layoutText(layout, m_text, rect.size());
    drawTextLayout(painter, layout, rect);
}

QTextOption WindowTaskItem::textOption() const
{
    Qt::LayoutDirection direction = QApplication::layoutDirection();
    Qt::Alignment alignment = QStyle::visualAlignment(direction, Qt::AlignLeft | Qt::AlignVCenter);

    QTextOption option;
    option.setTextDirection(direction);
    option.setAlignment(alignment);

    return option;
}

QSize WindowTaskItem::layoutText(QTextLayout &layout, const QString &text,
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

void WindowTaskItem::drawTextLayout(QPainter *painter, const QTextLayout &layout, const QRect &rect) const
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

    painter->drawPixmap(rect.topLeft(), pixmap);
}

void WindowTaskItem::updateTask()
{
    Q_ASSERT(m_task);

    // task flags
    TaskFlags flags = m_flags;
    if (m_task->isActive()) {
        flags |= TaskHasFocus;
        emit activated(this);
    } else {
        flags &= ~TaskHasFocus;
    }

    if (m_task->demandsAttention()) {
        flags |= TaskWantsAttention;
    } else {
        flags &= ~TaskWantsAttention;
    }

    if (m_task->isMinimized()) {
        flags |= TaskIsMinimized;
    } else {
        flags &= ~TaskIsMinimized;
    }

    setTaskFlags(flags);

    // basic title and icon
    QIcon taskIcon;
    taskIcon.addPixmap(m_task->icon(KIconLoader::SizeSmall, KIconLoader::SizeSmall, false));
    taskIcon.addPixmap(m_task->icon(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium, false));
    taskIcon.addPixmap(m_task->icon(KIconLoader::SizeMedium, KIconLoader::SizeMedium, false));
    taskIcon.addPixmap(m_task->icon(KIconLoader::SizeLarge, KIconLoader::SizeLarge, false));

#ifdef TOOLTIP_MANAGER
    if (m_showTooltip) {
      Plasma::ToolTipData data;
      data.mainText = m_task->visibleName();
      data.subText = i18nc("Which virtual desktop a window is currently on", "On %1", KWindowSystem::desktopName(m_task->desktop()));
      data.image = iconPixmap;
      data.windowToPreview = m_task->window();
      setToolTip(data);
    } else {
        Plasma::ToolTipData data;
        setToolTip(data); // Clear
    }
#endif
    setIcon(taskIcon);
    setText(m_task->visibleName());
    //redraw
    queueUpdate();
}

void WindowTaskItem::animationUpdate(qreal progress)
{
    if (progress == 1) {
        m_animId = 0;
        m_fadeIn = true;
    }

    m_alpha = m_fadeIn ? progress : 1 - progress;

    // explicit update
    update();
}

void WindowTaskItem::setStartupTask(TaskManager::StartupPtr task)
{
    setText(task->text());
    setIcon(KIcon(task->icon()));
#ifdef TOOLTIP_MANAGER
    if (m_showTooltip) {
        Plasma::ToolTipData tip;
        tip.mainText = task->text();
        tip.image = task->icon();
        setToolTip(tip);
    }
#endif
}

void WindowTaskItem::setWindowTask(TaskManager::TaskPtr task)
{
    if (m_task) {
        disconnect(m_task.constData(), 0, this, 0);
    }

    m_task = task;

    connect(task.constData(), SIGNAL(changed()),
            this, SLOT(updateTask()));
    connect(task.constData(), SIGNAL(iconChanged()),
            this, SLOT(updateTask()));

    updateTask();
    publishIconGeometry();

    //kDebug() << "Task added, isActive = " << task->isActive();
}

TaskManager::TaskPtr WindowTaskItem::windowTask() const
{
    return m_task;
}

void WindowTaskItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
{
    if (!KAuthorized::authorizeKAction("kwin_rmb") || m_task.isNull()) {
        QGraphicsWidget::contextMenuEvent(e);
        return;
    }

    TaskManager::TaskRMBMenu menu(m_task);
    menu.exec(e->screenPos());
}

void WindowTaskItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
    if (!m_activateTimer) {
        m_activateTimer = new QTimer(this);
        m_activateTimer->setSingleShot(true);
        m_activateTimer->setInterval(300);
        connect(m_activateTimer, SIGNAL(timeout()), this, SLOT(activate()));
    }
    m_activateTimer->start();
}

void WindowTaskItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    // restart the timer so that activate() is only called after the mouse
    // stops moving
    m_activateTimer->start();
}

void WindowTaskItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    delete m_activateTimer;
    m_activateTimer = 0;
}

void WindowTaskItem::setGeometry(const QRectF& geometry)
{
    QGraphicsWidget::setGeometry(geometry);
    publishIconGeometry();
}

void WindowTaskItem::publishIconGeometry()
{
    if (!scene()) {
        return;
    }

    QGraphicsView *parentView = 0L;
    // The following was taken from Plasma::Applet, it doesn't make sense to make the item an applet, and this was the easiest way around it.
    foreach (QGraphicsView *view, scene()->views()) {
        if (view->sceneRect().intersects(sceneBoundingRect()) ||
            view->sceneRect().contains(scenePos())) {
            parentView = view;
        }
    }
    if (!parentView || !m_task) {
        return;
    }
    if( !boundingRect().isValid() )
        return;

    QRect rect = parentView->mapFromScene(mapToScene(boundingRect())).boundingRect().adjusted(0, 0, 1, 1);
    rect.moveTopLeft(parentView->mapToGlobal(rect.topLeft()));
    if (m_task) {
        m_task->publishIconGeometry(rect);
    }
}

QRectF WindowTaskItem::iconRect(const QRectF &b) const
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

QRectF WindowTaskItem::textRect(const QRectF &bounds) const
{
    QSize size(bounds.size().toSize());
    size.rwidth() -= int(iconRect(bounds).width()) + qMax(0, IconTextSpacing - 2);

    return QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignRight | Qt::AlignVCenter,
                                     size, bounds.toRect());
}

#include "windowtaskitem.moc"
