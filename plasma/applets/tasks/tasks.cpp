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
#include "tasks.h"

// Standard
#include <math.h>

// Qt
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QIcon>
#include <QLinearGradient>
#include <QTimeLine>
#include <QStyleOptionGraphicsItem>
#include <QtDebug>

// KDE
#include <KColorScheme>
#include <KGlobalSettings>
#include <KIcon>
#include <KIconLoader>

// Plasma
#include <plasma/widgets/boxlayout.h>
#include <plasma/widgets/layoutanimator.h>

Tasks::Tasks(QObject* parent , const QVariantList &arguments)
 : Plasma::Applet(parent,arguments)
{
    setDrawStandardBackground(true);

    Plasma::BoxLayout *layout = new Plasma::BoxLayout(Plasma::BoxLayout::LeftToRight,0);
    layout->setMargin(0);
    _rootTaskGroup = new TaskGroupItem(this,this);
    
    // testing
        Plasma::LayoutAnimator* animator = new Plasma::LayoutAnimator;
        animator->setAutoDeleteOnRemoval(true);
        animator->setEffect( Plasma::LayoutAnimator::InsertedState , Plasma::LayoutAnimator::FadeInMoveEffect );
        animator->setEffect( Plasma::LayoutAnimator::StandardState , Plasma::LayoutAnimator::MoveEffect );
        animator->setEffect( Plasma::LayoutAnimator::RemovedState , Plasma::LayoutAnimator::FadeOutMoveEffect );
        animator->setTimeLine(new QTimeLine(300,this));
        _rootTaskGroup->layout()->setAnimator( animator );

    layout->addItem(_rootTaskGroup);
    setLayout(layout);

    // testing
        _rootTaskGroup->setBorderStyle( TaskGroupItem::NoBorder );
       // _rootTaskGroup->setColor( QColor(100,120,130) );
        _rootTaskGroup->setText( "Root Group" );

    // add representations of existing running tasks
    registerWindowTasks();
    registerStartingTasks();
}
Tasks::~Tasks()
{
}
void Tasks::registerStartingTasks()
{
    // listen for addition and removal of starting tasks
    connect(TaskManager::self() , SIGNAL(startupAdded(Startup::StartupPtr)) , this , SLOT(addStartingTask(Startup::StartupPtr)) );
    connect(TaskManager::self() , SIGNAL(startupRemoved(Startup::StartupPtr)) , this , SLOT(removeStartingTask(Startup::StartupPtr)));
}
void Tasks::addStartingTask(Startup::StartupPtr task)
{
    StartupTaskItem* item = new StartupTaskItem(_rootTaskGroup,_rootTaskGroup);
    _startupTaskItems.insert(task,item);

    addItemToRootGroup(item);
}
void Tasks::removeStartingTask(Startup::StartupPtr task)
{
    removeItemFromRootGroup(_startupTaskItems[task]);
}
void Tasks::registerWindowTasks()
{
    TaskManager* manager = TaskManager::self();

    Task::Dict tasks = manager->tasks();
    QMapIterator<WId,Task::TaskPtr> iter(tasks);

    while ( iter.hasNext() )
    {
        iter.next();
        addWindowTask(iter.value());
    }

    // listen for addition and removal of window tasks
    connect(TaskManager::self(), SIGNAL(taskAdded(Task::TaskPtr)), this, SLOT(addWindowTask(Task::TaskPtr)));
    connect(TaskManager::self(), SIGNAL(taskRemoved(Task::TaskPtr)), this , SLOT(removeWindowTask(Task::TaskPtr)));
}
void Tasks::addItemToRootGroup( AbstractTaskItem* item )
{
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    _rootTaskGroup->insertTask(item);
}
void Tasks::removeItemFromRootGroup( AbstractTaskItem* item )
{
    Q_ASSERT( item );
   
    _rootTaskGroup->removeTask(item);

// TEMPORARY
//      scene()->removeItem(item);
//    item->deleteLater();
}
void Tasks::addWindowTask(Task::TaskPtr task)
{
    WindowTaskItem* item = new WindowTaskItem(_rootTaskGroup,_rootTaskGroup);
    item->setWindowTask(task);
    _windowTaskItems.insert(task,item);

    addItemToRootGroup(item);
}
void Tasks::removeWindowTask(Task::TaskPtr task)
{
    removeItemFromRootGroup(_windowTaskItems[task]);
}
AbstractTaskItem::AbstractTaskItem(QGraphicsItem* parent , QObject *parentObject)
    : Widget(parent,parentObject)
    , _flags(0)
    , _fadeTimer(0)
    , _previousDragTarget(0)
{
    setAcceptsHoverEvents(true);
    setAcceptDrops(true);

    _fadeTimer = new QTimeLine();
    _fadeTimer->setCurveShape(QTimeLine::LinearCurve);

    connect( _fadeTimer , SIGNAL(valueChanged(qreal)) ,
            this , SLOT(animationUpdate()) );
}

AbstractTaskItem::~AbstractTaskItem()
{
    delete _fadeTimer;
}
void AbstractTaskItem::animationUpdate()
{
    // explicit update
    update();
}
void AbstractTaskItem::finished()
{
    // do something here to get the task removed
}
void AbstractTaskItem::setText(const QString& text)
{
    _text = text;

    updateGeometry();
}
QString AbstractTaskItem::text() const { return _text; }

void AbstractTaskItem::setTaskFlags( TaskFlags flags ) { _flags = flags; }
AbstractTaskItem::TaskFlags AbstractTaskItem::taskFlags() const { return _flags; }

void AbstractTaskItem::hoverEnterEvent( QGraphicsSceneHoverEvent* )
{
    const int FadeInDuration = 100;

    _fadeTimer->setDirection(QTimeLine::Forward);
    _fadeTimer->setDuration(FadeInDuration);

    if ( _fadeTimer->state() != QTimeLine::Running )
        _fadeTimer->start();
}
void AbstractTaskItem::hoverLeaveEvent( QGraphicsSceneHoverEvent* )
{
    const int FadeOutDuration = 200;

    _fadeTimer->setDirection(QTimeLine::Backward);
    _fadeTimer->setDuration(FadeOutDuration);

    if ( _fadeTimer->state() != QTimeLine::Running )
    {
        _fadeTimer->setCurrentTime(FadeOutDuration);
        _fadeTimer->start();
    }
}
QSizeF AbstractTaskItem::maximumSize() const
{
    //FIXME HARDCODE
    QSizeF sz = QSizeF(MaxTaskIconSize + QFontMetricsF(QApplication::font()).width(text() + IconTextSpacing),
                  200);

   // qDebug() << "Task max size hint:" << sz;

    return sz;
}
void AbstractTaskItem::setIcon(const QIcon& icon)
{
    _icon = icon; //icon.pixmap(MinTaskIconSize);
    
    updateGeometry();
}
void AbstractTaskItem::drawBackground(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* )
{
    // FIXME  Check the usage of KColorScheme here with various color schemes

    KColorScheme colorScheme;

    if ( option->state & QStyle::State_MouseOver 
         || _fadeTimer->state() == QTimeLine::Running 
         || taskFlags() & TaskHasFocus )
    {
        QLinearGradient background( boundingRect().topLeft() , 
                                  boundingRect().bottomLeft() );

        QColor startColor;
        QColor endColor;

        if ( taskFlags() & TaskHasFocus )  
            startColor = colorScheme.background(KColorScheme::ActiveBackground).color();
        else
            startColor = colorScheme.background(KColorScheme::LinkBackground).color();

        endColor = colorScheme.shade(startColor,KColorScheme::DarkShade);
      
        const qreal pressedAlpha = 0.2;
        const qreal hoverAlpha = 0.4;

        qreal alpha = 0;

        if ( option->state & QStyle::State_Sunken )
            alpha = pressedAlpha;
        else
            alpha = hoverAlpha;
      
        alpha *= _fadeTimer->currentValue();

        startColor.setAlphaF(alpha);
        endColor.setAlphaF( qMin(1.0,startColor.alphaF()+0.2) );

        background.setColorAt(0,startColor);
        background.setColorAt(1,endColor);

        // FIXME HARDCODE
            painter->setPen( QPen(QColor(100,100,100,startColor.alpha())) );
        
        painter->setBrush(background);
        painter->drawRect(option->rect);
    }
}
QRectF AbstractTaskItem::iconRect() const
{
    QSizeF iconSize = _icon.actualSize(boundingRect().size().toSize());
    //iconSize.scale(boundingRect().size(),Qt::KeepAspectRatio);
    
    QRectF rect = QRectF( QPointF(0,0) , iconSize );
    rect.moveTop( (boundingRect().height() - rect.height()) / 2 );

    return rect;
}
QRectF AbstractTaskItem::textRect() const
{
    QRectF text = boundingRect();
    text.setLeft( iconRect().right() + IconTextSpacing );

    text.setTop( text.top()+5 );
    text.setBottom( text.bottom()-5 );

    return text;
}
void AbstractTaskItem::drawTask(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* )
{
    _icon.paint( painter , iconRect().toRect() );

#if 0
    QFont font = painter->font();
    if ( taskFlags() & TaskHasFocus )
    {
        font.setBold(true);
        painter->setFont(font);
    }
#endif

    // FIXME HARDCODE testing
    painter->setPen( QPen(QColor(200,200,200) ,1.0) );
    
    painter->setBrush( QBrush() );
    painter->drawText( textRect() , Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap , _text );
}
void AbstractTaskItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setOpacity(opacity());
    painter->setRenderHint(QPainter::Antialiasing);

    // draw background
    drawBackground(painter,option,widget);

    // draw icon and text
    drawTask(painter,option,widget);
}
void AbstractTaskItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* )
{
    activate();
}
void AbstractTaskItem::mousePressEvent( QGraphicsSceneMouseEvent* )
{
    update();
}
void AbstractTaskItem::close()
{
    finished();
}

TaskGroupItem::TaskGroupItem(QGraphicsItem* parent,QObject *parentObject)
    : AbstractTaskItem(parent,parentObject)
    , _borderStyle(NoBorder)
    , _potentialDropAction(NoAction)
    , _caretIndex(0)
    , _allowSubGroups(true)
{
    setAcceptDrops(true);

   setLayout(new Plasma::BoxLayout(Plasma::BoxLayout::LeftToRight,0));
   layout()->setMargin(0);
   layout()->setSpacing(5);
}
QSizeF TaskGroupItem::maximumSize() const
{
    return Widget::maximumSize();
}
void TaskGroupItem::setAllowSubGroups(bool allow) { _allowSubGroups = allow; }
bool TaskGroupItem::allowSubGroups() const { return _allowSubGroups; }

QList<AbstractTaskItem*> TaskGroupItem::tasks() const
{
    QList<AbstractTaskItem*> items;
    foreach( TaskEntry entry , _tasks )
        items << entry.task;

    return items;
}
void TaskGroupItem::insertTask( AbstractTaskItem* item , int index )
{
    if ( index == -1 )
        index = _tasks.count();
   
    Q_ASSERT( index >= 0 && index <= _tasks.count() );

    // remove item from existing group if any
    TaskGroupItem* parentGroup = dynamic_cast<TaskGroupItem*>(item->parentItem());
    if ( parentGroup )
        parentGroup->removeTask(item);

    item->setParentItem(this);
    _tasks.insert(index,item);

    layout()->addItem(item);
    layout()->update();
}
void TaskGroupItem::removeTask( AbstractTaskItem* item )
{
    for ( int i = 0 ; i < _tasks.count() ; i++ )
    {
        if ( _tasks[i].task == item )
            _tasks.removeAt(i);
    }

    layout()->removeItem(item);
    layout()->update();

    // if the group is now empty then ask the parent to remove it
    if ( _tasks.count() == 0 )
    {
        TaskGroupItem* parentGroup = dynamic_cast<TaskGroupItem*>(parentItem());
        if ( parentGroup )
        {
            parentGroup->removeTask(this);
            scene()->removeItem(this);
            deleteLater();
        }
    }
}
void TaskGroupItem::reorderTasks( int from , int to )
{
    Q_ASSERT( from >= 0 && from < _tasks.size() );
    Q_ASSERT( to >= 0 && to < _tasks.size() );

    qDebug() << "Reordering from" << from << "to" << to;

    AbstractTaskItem* task = _tasks.takeAt(from).task;
    _tasks.insert(to,TaskEntry(task));
}
qreal TaskGroupItem::titleHeight() const 
{
    if ( _borderStyle != CaptionBorder )
        return 0;

    QFontMetrics titleFontMetrics(titleFont());
    return titleFontMetrics.height();
}
QFont TaskGroupItem::titleFont()
{
    return KGlobalSettings::smallestReadableFont();
}
void TaskGroupItem::setColor( const QColor& color )
{
    _color = color;
}
QColor TaskGroupItem::color() const
{
    return _color;
}

void TaskGroupItem::setBorderStyle( BorderStyle style )
{
    _borderStyle = style;
}
TaskGroupItem::BorderStyle TaskGroupItem::borderStyle() const
{
    return _borderStyle;
}
void TaskGroupItem::paint( QPainter* painter , const QStyleOptionGraphicsItem* option,
                           QWidget* )
{
    painter->setRenderHint(QPainter::Antialiasing);

    // draw group border
    // adjust area slightly to get a sharp border with anti-aliasing enabled
    drawBorder(painter,option,QRectF(option->rect).adjusted(0.5,0.5,-0.5,-0.5));  
}

void TaskGroupItem::activate()
{
    foreach( TaskEntry entry , _tasks )
    {
        entry.task->activate();
    }
}
void TaskGroupItem::close()
{
    foreach( TaskEntry entry , _tasks )
    {
        entry.task->close();
    }
    finished();
}

void TaskGroupItem::drawBorder(QPainter* painter , const QStyleOptionGraphicsItem* option,  const QRectF& area)
{
    QFont smallFont = KGlobalSettings::smallestReadableFont();
    QFontMetrics smallFontMetrics(smallFont);
         
    if ( _color.isValid() )
    {
        painter->setPen( QPen(Qt::NoPen) );
        painter->setBrush( _color );
        painter->drawRect( area );
    }

    if ( _borderStyle == CaptionBorder )
    {
         const QRectF titleArea(area.top(),area.left(),area.width(),titleHeight());

#if 0   
         const int cornerSize = 5;
         QPainterPath borderPath(area.topLeft() + QPointF(0,titleSize));
         borderPath.quadTo( area.topLeft() , area.topLeft() + QPointF(cornerSize,0) );
         borderPath.lineTo( area.topRight() - QPointF(cornerSize,0) );
         borderPath.quadTo( area.topRight() , area.topRight() + QPointF(0,titleSize) );
         borderPath.lineTo( area.bottomRight() );
         borderPath.lineTo( area.bottomLeft() );
         borderPath.lineTo( area.topLeft() + QPointF(0,titleSize) );
         borderPath.lineTo( area.topRight() + QPointF(0,titleSize) );

         
         painter->drawPath(borderPath);
#endif

         // FIXME Check KColorScheme usage here

         QLinearGradient titleGradient(titleArea.topLeft(),titleArea.bottomLeft());

         titleGradient.setColorAt(0,KColorScheme::shade(_color,KColorScheme::DarkShade));
         titleGradient.setColorAt(1,KColorScheme::shade(_color,KColorScheme::MidShade));

         painter->setBrush( titleGradient );
         painter->drawRect( titleArea );

         // draw caption
         painter->setPen( QPen(option->palette.text(),1.0) );
         QFontMetricsF titleFontMetrics(titleFont());
         painter->drawText( area.topLeft() + 
                            QPointF(0,titleArea.height() - titleFontMetrics.descent())
                            , text() );
    }
}

StartupTaskItem::StartupTaskItem(QGraphicsItem* parent , QObject *parentObject)
    : AbstractTaskItem(parent,parentObject)
{
}

void StartupTaskItem::setStartupTask(Startup::StartupPtr task) 
{ 
    _task = task;

    setText( task->text() );
    setIcon( KIcon(task->icon()) );
}
Startup::StartupPtr StartupTaskItem::startupTask() const { return _task; }

WindowTaskItem::WindowTaskItem(QGraphicsItem* parent , QObject *parentObject)
    : AbstractTaskItem(parent,parentObject)
{
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
    if ( _task )
    {
        _task->activateRaiseOrIconify();
    }
}
void WindowTaskItem::close()
{
    if ( _task )
    {
        _task->close();
    }
    finished();
}
void WindowTaskItem::updateTask()
{
    Q_ASSERT( _task );

    // task flags
    if ( _task->isActive() )
        setTaskFlags( taskFlags() | TaskHasFocus );
    else
        setTaskFlags( taskFlags() & ~TaskHasFocus );

    if ( _task->demandsAttention() )
        setTaskFlags( taskFlags() | TaskWantsAttention );
    else
        setTaskFlags( taskFlags() & ~TaskWantsAttention );

    // basic title and icon
    QPixmap iconPixmap = _task->icon( preferredIconSize().width() ,
                                     preferredIconSize().height() ,
                                     true);
    setIcon(QIcon(iconPixmap));
    setText(_task->visibleName());
}
void WindowTaskItem::setWindowTask( Task::TaskPtr task )
{
    if ( _task )
        disconnect( _task.constData() , 0 , this , 0 );

    _task = task;

    connect( task.constData() , SIGNAL(changed()) , this , 
             SLOT(updateTask()) );
    connect( task.constData() , SIGNAL(iconChanged()) ,
            this , SLOT(updateTask()) );

    updateTask();

    qDebug() << "Task added, isActive = " << task->isActive();
}
Task::TaskPtr WindowTaskItem::windowTask() const
{
    return _task;
}

#include "tasks.moc"

