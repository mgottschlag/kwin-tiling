/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight                                   *
 *   robertknight@gmail.com                                                *
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
#include "taskgroupitem.h"

// Qt
#include <QPainter>
#include <QGraphicsScene>

// KDE
#include <KColorScheme>
#include <KDebug>
#include <KGlobalSettings>
#include <QStyleOptionGraphicsItem>

// Plasma
#include "plasma/theme.h"

TaskGroupItem::TaskGroupItem(QGraphicsItem *parent, QObject *parentObject)
    : AbstractTaskItem(parent, parentObject),
      _activeTask(-1),
      _borderStyle(NoBorder),
      //_potentialDropAction(NoAction),
      _caretIndex(0),
      _allowSubGroups(true),
      m_geometryUpdateTimerId(-1)
{
   //setAcceptDrops(true);

   m_layout = new Plasma::BoxLayout(Plasma::BoxLayout::LeftToRight, this);
   m_layout->setMargin(0);
   m_layout->setSpacing(5);
   m_layout->setMultiRow(true);
}

QSizeF TaskGroupItem::maximumSize() const
{
    return Widget::maximumSize();
}

void TaskGroupItem::setAllowSubGroups(bool allow)
{
    _allowSubGroups = allow;
}

bool TaskGroupItem::allowSubGroups() const
{
    return _allowSubGroups;
}

QList<AbstractTaskItem*> TaskGroupItem::tasks() const
{
    QList<AbstractTaskItem*> items;
    foreach(TaskEntry entry, _tasks) {
        items << entry.task;
    }

    return items;
}

void TaskGroupItem::insertTask(AbstractTaskItem *item, int index)
{
    if (index == -1) {
        index = _tasks.count();
    }

    Q_ASSERT( index >= 0 && index <= _tasks.count() );

    // remove item from existing group if any
    TaskGroupItem *parentGroup = dynamic_cast<TaskGroupItem*>(item->parentItem());
    if (parentGroup) {
        parentGroup->removeTask(item);
    }

    connect(item, SIGNAL(activated(AbstractTaskItem*)),
            this, SLOT(updateActive(AbstractTaskItem*)));
    connect(item, SIGNAL(windowSelected(AbstractTaskItem*)),
            this, SIGNAL(activated(AbstractTaskItem*)));

    item->setParentItem(this);
    _tasks.insert(index, item);

    layout()->addItem(item);
    queueGeometryUpdate();
}

void TaskGroupItem::removeTask(AbstractTaskItem *item)
{
    bool found = false;
    for (int i = 0; i < _tasks.count(); i++) {
        if (_tasks[i].task == item) {
            _tasks.removeAt(i);
            found = true;
            break;
        }
    }

    if (!found) {
        return;
    }

    layout()->removeItem(item);
    item->setParentItem(0);
    queueGeometryUpdate();

    // if the group is now empty then ask the parent to remove it
    if (_tasks.count() == 0) {
        TaskGroupItem* parentGroup = dynamic_cast<TaskGroupItem*>(parentItem());
        if (parentGroup) {
            parentGroup->removeTask(this);
            scene()->removeItem(this);
            deleteLater();
        }
    }

    disconnect(item, SIGNAL(activated(AbstractTaskItem*)),
               this, SIGNAL(activated(AbstractTaskItem*)));
    disconnect(item, SIGNAL(windowSelected(AbstractTaskItem*)),
               this, SIGNAL(activated(AbstractTaskItem*)));
}

void TaskGroupItem::updateActive(AbstractTaskItem *task)
{
    _activeTask = _tasks.indexOf(TaskEntry(task));
}

void TaskGroupItem::cycle(int delta)
{
    //only cycle active task if there are 2 tasks or more
    if (_tasks.count() < 2) {
        return;
    }

    if (_activeTask == -1) {
        _tasks[0].task->activate();
    }
    //cycle active task with the circular array tecnique
    else if (delta < 0) {
        //if _activeTask < _tasks.count() the new _activeTask
        //will be _activeTask+1, else it will be 1
        _tasks[(_activeTask+1)%_tasks.count()].task->activate();
    }else{
        //if _activeTask > 1 the new _activeTask
        //will be _activeTask-1, else it will be _tasks.count()
        _tasks[(_tasks.count() + _activeTask -1 )%_tasks.count()].task->activate();
    }
}

void TaskGroupItem::reorderTasks(int from, int to)
{
    Q_ASSERT( from >= 0 && from < _tasks.size() );
    Q_ASSERT( to >= 0 && to < _tasks.size() );

    kDebug() << "Reordering from" << from << "to" << to;

    AbstractTaskItem* task = _tasks.takeAt(from).task;
    _tasks.insert(to,TaskEntry(task));
}

qreal TaskGroupItem::titleHeight() const
{
    if (_borderStyle != CaptionBorder) {
        return 0;
    }

    QFontMetrics titleFontMetrics(titleFont());
    return titleFontMetrics.height();
}

QFont TaskGroupItem::titleFont()
{
    return KGlobalSettings::smallestReadableFont();
}

void TaskGroupItem::setColor(const QColor &color)
{
    _color = color;
}

QColor TaskGroupItem::color() const
{
    return _color;
}

void TaskGroupItem::setBorderStyle(BorderStyle style)
{
    _borderStyle = style;
}

TaskGroupItem::BorderStyle TaskGroupItem::borderStyle() const
{
    return _borderStyle;
}

void TaskGroupItem::setDirection(Plasma::BoxLayout::Direction dir)
{
    m_layout->setDirection(dir);

    m_layout->setMultiRow(dir != Plasma::BoxLayout::TopToBottom);
}

Plasma::BoxLayout::Direction TaskGroupItem::direction()
{
    return m_layout->direction();
}

void TaskGroupItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);

    // draw group border
    // adjust area slightly to get a sharp border with anti-aliasing enabled
    drawBorder(painter, option, QRectF(option->rect).adjusted(0.5,0.5,-0.5,-0.5));
}

void TaskGroupItem::activate()
{
    if (!dynamic_cast<TaskGroupItem*>(parentItem())) {
        // this is the root group, and we don't want to activate *every* window!
        return;
    }

    // this is a task sub-group, so let's activate the windows in it
    foreach(TaskEntry entry, _tasks) {
        entry.task->activate();
    }
}

void TaskGroupItem::close()
{
    foreach(TaskEntry entry, _tasks) {
        entry.task->close();
    }
    finished();
}

void TaskGroupItem::drawBorder(QPainter *painter,
                               const QStyleOptionGraphicsItem *option,
                               const QRectF &area)
{
    QFont smallFont = KGlobalSettings::smallestReadableFont();
    QFontMetrics smallFontMetrics(smallFont);

    if (_color.isValid()) {
        painter->setPen(QPen(Qt::NoPen));
        painter->setBrush(_color);
        painter->drawRect(area);
    }

    if (_borderStyle == CaptionBorder) {
         const QRectF titleArea(area.top(),area.left(),area.width(),titleHeight());

#if 0
         const int cornerSize = 5;
         QPainterPath borderPath(area.topLeft() + QPointF(0,titleSize));
         borderPath.quadTo(area.topLeft(), area.topLeft() + QPointF(cornerSize,0));
         borderPath.lineTo(area.topRight() - QPointF(cornerSize,0));
         borderPath.quadTo(area.topRight(), area.topRight() + QPointF(0,titleSize) );
         borderPath.lineTo(area.bottomRight());
         borderPath.lineTo(area.bottomLeft());
         borderPath.lineTo(area.topLeft() + QPointF(0,titleSize));
         borderPath.lineTo(area.topRight() + QPointF(0,titleSize));


         painter->drawPath(borderPath);
#endif

         // FIXME Check KColorScheme usage here

         QLinearGradient titleGradient(titleArea.topLeft(), titleArea.bottomLeft());
         KColorScheme colorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::self()->colors());        
         titleGradient.setColorAt(0, colorScheme.shade(_color,KColorScheme::DarkShade));
         titleGradient.setColorAt(1, colorScheme.shade(_color,KColorScheme::MidShade));

         painter->setBrush(titleGradient);
         painter->drawRect(titleArea);

         // draw caption
         painter->setPen(QPen(option->palette.text(),1.0));
         QFontMetricsF titleFontMetrics(titleFont());
         painter->drawText(area.topLeft() + QPointF(0,titleArea.height()
                                          - titleFontMetrics.descent()),
                           text());
    }
}

void TaskGroupItem::queueGeometryUpdate()
{
    if (m_geometryUpdateTimerId == -1) {
        m_geometryUpdateTimerId = startTimer(200);
    }
}

void TaskGroupItem::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_geometryUpdateTimerId) {
        killTimer(m_geometryUpdateTimerId);
        updateGeometry();
        m_geometryUpdateTimerId = -1;
    }
}

void TaskGroupItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Widget::hoverEnterEvent(event);
}

void TaskGroupItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Widget::hoverLeaveEvent(event);
}

#include "taskgroupitem.moc"
