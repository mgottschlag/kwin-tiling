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
#include <KColorScheme>
#include <KIconLoader>

#include <taskmanager/taskrmbmenu.h>

#include "plasma/theme.h"
#include "plasma/panelsvg.h"

bool WindowTaskItem::s_backgroundCreated = false;
Plasma::PanelSvg* WindowTaskItem::s_taskItemBackground = 0;
qreal WindowTaskItem::s_leftMargin = 0;
qreal WindowTaskItem::s_topMargin = 0;
qreal WindowTaskItem::s_rightMargin = 0;
qreal WindowTaskItem::s_bottomMargin = 0;

WindowTaskItem::WindowTaskItem(QGraphicsItem *parent, const bool showTooltip)
    : QGraphicsWidget(parent),
      _activateTimer(0),
      _flags(0),
      m_animId(-1),
      m_alpha(1),
      m_fadeIn(true),
      m_updateTimerId(-1)
{
    _showTooltip = showTooltip;
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    setAcceptsHoverEvents(true);
    setupBackgroundSvg(0);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(slotUpdate()));
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
    if (_task) {
        _task->activateRaiseOrIconify();
        emit windowSelected(this);
    }
}

void WindowTaskItem::close()
{
    if (_task) {
        _task->close();
    }
}

void WindowTaskItem::setShowTooltip(const bool showit)
{
    _showTooltip = showit;
    updateTask();
}

void WindowTaskItem::setText(const QString &text)
{
    if (_text == text) {
        return;
    }
    _text = text;
}

void WindowTaskItem::setIcon(const QIcon &icon)
{
    _icon = icon; //icon.pixmap(MinTaskIconSize);

}

void WindowTaskItem::setTaskFlags(TaskFlags flags)
{
    if (_flags & TaskWantsAttention != flags & TaskWantsAttention) {
        if (flags & TaskWantsAttention) {
            // start attention getting
        } else {
            // stop attention getting
        }
    }
    _flags = flags;
}

WindowTaskItem::TaskFlags WindowTaskItem::taskFlags() const
{
    return _flags;
}

void WindowTaskItem::queueUpdate()
{
    if (m_updateTimerId != -1) {
        return;
    }

    if (m_lastUpdate.elapsed() < 200) {
        if (m_updateTimerId == -1) {
            m_updateTimerId = startTimer(200);
        }

        return;
    }

    update();
    m_lastUpdate.restart();
}

void WindowTaskItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    const int FadeInDuration = 100;

    if (m_animId != -1) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    m_fadeIn = true;
    m_animId = Plasma::Animator::self()->customAnimation(40 / (1000 / FadeInDuration), FadeInDuration,Plasma::Animator::LinearCurve, this,"animationUpdate");

    QGraphicsWidget::hoverEnterEvent(event);
}

void WindowTaskItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    const int FadeOutDuration = 200;

    if (m_animId != -1) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    m_fadeIn = false;
    m_animId = Plasma::Animator::self()->customAnimation(40 / (1000 / FadeOutDuration), FadeOutDuration,Plasma::Animator::LinearCurve, this,"animationUpdate");

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
        m_updateTimerId = -1;
    }
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

QSize WindowTaskItem::preferredIconSize() const 
{
    return QSize(MinTaskIconSize, MinTaskIconSize);
}

void WindowTaskItem::drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    // FIXME  Check the usage of KColorScheme here with various color schemes

    const qreal hoverAlpha = 0.4;

    KColorScheme colorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme());

    if (taskFlags() & TaskWantsAttention) {
        if (s_taskItemBackground) {
            //Draw task background from theme svg "attention" element
            s_taskItemBackground->setElementPrefix("attention");
            s_taskItemBackground->resizePanel(option->rect.size());
            s_taskItemBackground->paintPanel(painter, option->rect);
        } else {
            //Draw task background without svg theming
            QColor background = colorScheme.background(KColorScheme::ActiveBackground).color();
            background.setAlphaF(hoverAlpha+0.2);
            painter->setBrush(QBrush(background));
            painter->drawPath(Plasma::roundedRectangle(option->rect, 6));
        }
    } else if (taskFlags() & TaskIsMinimized) {
            //Not painting anything for iconified tasks for now
            painter->setBrush(QBrush());
    } else {
        if (s_taskItemBackground) {
            //Draw task background from theme svg "normal" element
            s_taskItemBackground->setElementPrefix("normal");
            s_taskItemBackground->resizePanel(option->rect.size());
            //get the margins now
            s_taskItemBackground->getMargins(s_leftMargin, s_topMargin, s_rightMargin, s_bottomMargin);
            s_taskItemBackground->paintPanel(painter, option->rect);
        } else {
            //Draw task background without svg theming
            QColor background = colorScheme.shade(colorScheme.background(KColorScheme::AlternateBackground).color(),
                                              KColorScheme::DarkShade);
            background.setAlphaF(0.2);
            painter->setBrush(QBrush(background));
            painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
        }
    }

    if (option->state & QStyle::State_MouseOver
         || m_animId != -1
         || taskFlags() & TaskHasFocus)
    {

        if (s_taskItemBackground) {
            if (taskFlags() & TaskHasFocus) {
                //Draw task background from theme svg "focus" element
                s_taskItemBackground->setElementPrefix("focus");
                s_taskItemBackground->resizePanel(option->rect.size());
                s_taskItemBackground->paintPanel(painter, option->rect);
            }

            if (option->state & QStyle::State_MouseOver && m_alpha > 0) {
                //Draw task background from theme svg "hover" element
                painter->save();
                painter->setOpacity(m_alpha);

                s_taskItemBackground->setElementPrefix("hover");
                s_taskItemBackground->resizePanel(option->rect.size());
                s_taskItemBackground->paintPanel(painter, option->rect);

                painter->restore();
            }

        } else {
            //Draw task background without svg theming
            QLinearGradient background(boundingRect().topLeft(),
                                   boundingRect().bottomLeft());

            QColor startColor;
            QColor endColor;

            if (taskFlags() & TaskHasFocus) {
                startColor = colorScheme.background(KColorScheme::NormalBackground).color();
            } else {
                startColor = colorScheme.background(KColorScheme::AlternateBackground).color();
            }

            endColor = colorScheme.shade(startColor,KColorScheme::DarkShade);

            const qreal pressedAlpha = 0.2;

            qreal alpha = 0;

            if (option->state & QStyle::State_Sunken) {
                alpha = pressedAlpha;
            } else {
                alpha = hoverAlpha;
            }

            if (!(taskFlags() & TaskHasFocus)) {
                alpha *= m_alpha;
            }

            startColor.setAlphaF(alpha);
            endColor.setAlphaF(qMin(1.0,startColor.alphaF()+0.2));

            background.setColorAt(0, startColor);
            background.setColorAt(1, endColor);

            painter->setBrush(background);
            painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
        }
    }

    if (!s_taskItemBackground) {
        painter->drawPath(Plasma::roundedRectangle(option->rect, 6));
    }
}

void WindowTaskItem::drawTask(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *)
{
    Q_UNUSED(option)

    _icon.paint( painter , iconRect().toRect() );

    painter->setPen(QPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor), 1.0));

    QRect rect = textRect().toRect();
    rect.adjust(2, 2, -2, -2); // Create a text margin

    QTextLayout layout;
    layout.setFont(KGlobalSettings::taskbarFont());
    layout.setTextOption(textOption());

    layoutText(layout, _text, rect.size());
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
        foreach (const QRect &rect, fadeRects)
            p.fillRect(rect, alphaGradient);
    }

    p.end();

    painter->drawPixmap(rect.topLeft(), pixmap);
}

void WindowTaskItem::updateTask()
{
    Q_ASSERT( _task );

    // task flags
    if (_task->isActive()) {
        setTaskFlags(taskFlags() | TaskHasFocus);
        emit activated(this);
    } else {
        setTaskFlags(taskFlags() & ~TaskHasFocus);
    }

    if (_task->demandsAttention()) {
        setTaskFlags(taskFlags() | TaskWantsAttention);
    } else {
        setTaskFlags(taskFlags() & ~TaskWantsAttention);
    }

    if (_task->isMinimized()) {
        setTaskFlags(taskFlags() | TaskIsMinimized);
    } else {
        setTaskFlags(taskFlags() & ~TaskIsMinimized);
    }

    // basic title and icon
    QPixmap iconPixmap = _task->icon(preferredIconSize().width(),
                                     preferredIconSize().height(),
                                     true);
#ifdef TOOLTIP_MANAGER
    if (_showTooltip) {
      Plasma::ToolTipData data;
      data.mainText = _task->visibleName();
      data.subText = i18nc("Which virtual desktop a window is currently on", "On %1", KWindowSystem::desktopName(_task->desktop()));
      data.image = iconPixmap;
      data.windowToPreview = _task->window();
      setToolTip(data);
    } else {
        Plasma::ToolTipData data;
        setToolTip(data); // Clear
    }
#endif
    setIcon(QIcon(iconPixmap));
    setText(_task->visibleName());
    //redraw
    queueUpdate();
}

void WindowTaskItem::animationUpdate(qreal progress)
{
    if (progress == 1) {
        m_animId = -1;
        m_fadeIn = true;
    }

    m_alpha = m_fadeIn ? progress : 1 - progress;

    // explicit update
    update();
}

void WindowTaskItem::slotUpdate()
{
    setupBackgroundSvg(0);
    QGraphicsWidget::update();
}

void WindowTaskItem::setStartupTask(TaskManager::StartupPtr task)
{
    setText(task->text());
    setIcon(KIcon(task->icon()));
#ifdef TOOLTIP_MANAGER
    if (_showTooltip) {
        Plasma::ToolTipData tip;
        tip.mainText = task->text();
        tip.image = task->icon();
        setToolTip(tip);
    }
#endif
}

void WindowTaskItem::setWindowTask(TaskManager::TaskPtr task)
{
    if (_task)
        disconnect(_task.constData(), 0, this, 0);

    _task = task;

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
    return _task;
}

void WindowTaskItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
{
    if (!KAuthorized::authorizeKAction("kwin_rmb") || _task.isNull()) {
        QGraphicsWidget::contextMenuEvent(e);
        return;
    }

    TaskManager::TaskRMBMenu menu(_task);
    menu.exec(e->screenPos());
}

void WindowTaskItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
    if (!_activateTimer) {
        _activateTimer = new QTimer();
        _activateTimer->setSingleShot(true);
        _activateTimer->setInterval(300);
        connect(_activateTimer, SIGNAL(timeout()), this, SLOT(activate()));
    }
    _activateTimer->start();
}

void WindowTaskItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    // restart the timer so that activate() is only called after the mouse
    // stops moving
    _activateTimer->start();
}

void WindowTaskItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    delete _activateTimer;
    _activateTimer = 0;
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
    if (!parentView || !_task) {
        return;
    }
    if( !boundingRect().isValid() )
        return;

    QRect rect = parentView->mapFromScene(mapToScene(boundingRect())).boundingRect().adjusted(0, 0, 1, 1);
    rect.moveTopLeft(parentView->mapToGlobal(rect.topLeft()));
    if (_task) {
        _task->publishIconGeometry(rect);
    }
}

void WindowTaskItem::setupBackgroundSvg(QObject *parent)
{
    if (s_backgroundCreated) {
        return;
    }

    //TODO: we should probably reset this when the theme changes
    s_backgroundCreated = true;
    QString tasksThemePath = Plasma::Theme::defaultTheme()->imagePath("widgets/tasks");
    s_taskItemBackground = 0;

    if (!tasksThemePath.isEmpty()) {
        while (parent && parent->parent()) {
            parent = parent->parent();
        }

        s_taskItemBackground = new Plasma::PanelSvg(parent);
        s_taskItemBackground->setImagePath(tasksThemePath);
        s_taskItemBackground->setCacheAllRenderedPanels(true);
    }
}

QRectF WindowTaskItem::iconRect() const
{
    QRectF bounds = boundingRect().adjusted(s_leftMargin, s_topMargin, -s_rightMargin, -s_bottomMargin);
    //leave enough space for the text. usefull in vertical panel
    bounds.setWidth(qMax(bounds.width() / 3, qMin(minimumSize().height(), bounds.width())));

    QSize iconSize = _icon.actualSize(bounds.size().toSize());

    return QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter,
                               iconSize, bounds.toRect());
}

QRectF WindowTaskItem::textRect() const
{
    QSize size(boundingRect().size().toSize());
    size.rwidth() -= int(iconRect().width()) + qMin(0, IconTextSpacing - 2);

    return QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignRight | Qt::AlignVCenter,
                                     size, boundingRect().toRect());
}

#include "windowtaskitem.moc"
