/*
    Copyright (C) 2007 Robert Knight <robertknight@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 */

#ifndef TASKS_H
#define TASKS_H

// Qt
#include <QIcon>
#include <QPainter>
#include <QGraphicsItem>

// Plasma
#include <plasma/applet.h>
#include <taskmanager/taskmanager.h>

class QGraphicsSceneDragDropEvent;
class QTimeLine;
class QTextLayout;
class QTextOption;

class AbstractTaskItem;
class AbstractGroupingStrategy;
class TaskGroupItem;

/**
 * An applet which provides a visual representation of running
 * graphical tasks (ie. tasks that have some form of visual interface),
 * and allows the user to perform various actions on those tasks such
 * as bringing them to the foreground, sending them to the background
 * or closing them.
 */
class Tasks : public Plasma::Applet
{
    Q_OBJECT
public:
        /**
         * Constructs a new tasks applet
         * With the specified parent.
         */
        Tasks(QObject *parent, const QVariantList &args = QVariantList());

        void init();

        /**
         * Sets the strategy used to automatically group tasks
         * together.
         *
         * In addition to manual grouping of tasks which the user can
         * do by dragging tasks on top of one another, the Tasks
         * applet supports automatic grouping of tasks.
         */
        void setGroupingStrategy(AbstractGroupingStrategy* strategy);

        // we re-implement boundingRect() instead of contentSizeHint() because
        // of issues with using childrenBoundingRect().size().  It shouldn't
        // cause a problem as long as we don't ask Plasma::Applet to draw
        // a background
        //QRectF boundingRect() const;

        void constraintsUpdated(Plasma::Constraints constraints);

protected slots:
        virtual void wheelEvent(QGraphicsSceneWheelEvent *);

private slots:
        void addWindowTask(Task::TaskPtr);
        void removeWindowTask(Task::TaskPtr);

        void addStartingTask(Startup::StartupPtr);
        void removeStartingTask(Startup::StartupPtr);

private:
        // creates task representations for existing windows
        // and sets up connections to listen for addition or removal
        // of windows
        void registerWindowTasks();

        // creates task representations for tasks which are in
        // the process of being started
        // this allows some indication that the task is loading
        // to be displayed until the window associated with the task
        // appears
        void registerStartingTasks();

        void addItemToRootGroup(AbstractTaskItem* item);
        void removeItemFromRootGroup(AbstractTaskItem* item);

        TaskGroupItem* _rootTaskGroup;
        AbstractGroupingStrategy* _strategy;

        QHash<Task::TaskPtr,AbstractTaskItem*> _windowTaskItems;
        QHash<Startup::StartupPtr,AbstractTaskItem*> _startupTaskItems;
};


/**
 * A graphical representation of a task, consisting of an icon
 * and a caption.
 *
 * AbstractTaskItem should be sub-classed to provide a task item
 * which sets the text, icon, state flags and other properties
 * in response to updates from the source which provides information
 * about the task.
 *
 * //TODO: Clarify the above sentence
 *
 * Sub-classes can change the task's text and icon using the
 * setText() and setIcon() methods.  They can also set flags
 * to indicate the task's state using setTaskFlags().
 */
class AbstractTaskItem : public Plasma::Widget
{
    Q_OBJECT

public:
    virtual ~AbstractTaskItem();

    /** Sets the text for this task item. */
    void setText(const QString &text);

    /** Sets the icon for this task item. */
    void setIcon(const QIcon &icon);

    /**
     * This enum describes the generic flags which are currently
     * set by the task.
     */
    enum TaskFlag
    {
        /**
         * This flag is set by the task to indicate that it wants
         * the user's attention.
         */
        TaskWantsAttention = 1,
        /**
         * Indicates that the task's window has the focus
         */
        TaskHasFocus       = 2
    };
    Q_DECLARE_FLAGS(TaskFlags, TaskFlag)

    /** Sets the task flags for this item. */
    void setTaskFlags(TaskFlags flags);

    /** Returns the task's current flags. */
    TaskFlags taskFlags() const;

    /** Returns current text for this task. */
    QString text() const;

    /** Returns the current icon for this task. */
    QIcon icon() const;

    /**
     * Returns the overlays for this task.  Overlays
     * are arbitrary QGraphicsItem instances which are arranged
     * on top of the task's icon to display additional status
     * information about the task.
     */
    //QList<QGraphicsItem*> overlays() const;

    /**
     * Called when the user clicks on the task to activate it.
     * This usually means bringing the window containing the task
     * to the foreground.
     */
    virtual void activate() = 0;

    /**
     * Called when the user requests to close the task.
     * Sub-classes must ensure that finished() is called when
     * the task is closed.
     *
     * The default implementation simply calls finished()
     */
    virtual void close();

    // reimplemented from LayoutItem
    virtual QSizeF maximumSize() const;

signals:
    void activated(AbstractTaskItem *);

protected:
    /** Constructs a new task item. */
    AbstractTaskItem(QGraphicsItem *parent, QObject *parentObject);

    /** Sub-classes should call this method when the task closes. */
    void finished();

    /** Event compression **/
    void queueUpdate();

    // reimplemented
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void timerEvent(QTimerEvent *event);
    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget);

    /** TODO: Document me. */
   QSize preferredIconSize() const {
       return QSize(MinTaskIconSize, MinTaskIconSize);
   }

    /** Draws the background for the task item. */
    virtual void drawBackground(QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                QWidget *widget);
    /** Draws the icon and text which represent the task item. */
    virtual void drawTask(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget);

    /** Returns a QTextOption object for the icon label QTtextLayout.*/
    QTextOption textOption() const;

    /**
     * Lays the text out in the text layout using the constraints, and returns the actual
     * size required. The returned size may be wider than the constraints if the text
     * contains a non-breakable word that is wider than the maximum width.
     * If more height is needed than what's available, the last line that will fit will be
     * extended to hold the remainder of the text.
     */
    QSize layoutText(QTextLayout &layout, const QString &text, const QSize &constraints) const;

    /**
     * Draws the text layout (which must already have the text layed out) in the rect using
     * the supplied painter. If the layout contains text lines that are longer than the rect
     * is wide, they will be elided by fading the text out.
     */
    void drawTextLayout(QPainter *painter, const QTextLayout &layout, const QRect &rect) const;

private slots:
    void animationUpdate();

private:
    // area of item occupied by task's icon
    QRectF iconRect() const;
    // area of item occupied by task's text
    QRectF textRect() const;

    TaskFlags _flags;

    QIcon _icon;
    QString _text;

    QTimeLine* _fadeTimer;

    QPointF _dragOffset;
    AbstractTaskItem* _previousDragTarget;
    int m_updateTimerId;
    QTime m_lastUpdate;

    // minimum size (in pixels) of a task's icon
    static const int MinTaskIconSize = 48;
    // maximum size (in pixels) of a task's icon
    static const int MaxTaskIconSize = 48;

    // distance (in pixels) between a task's icon and its text
    static const int IconTextSpacing = 4;

    static const int TaskItemHorizontalMargin = 4;
    static const int TaskItemVerticalMargin = 4;
};

/**
 * A task item for a task which represents a window on the desktop.
 */
class WindowTaskItem : public AbstractTaskItem
{
    Q_OBJECT

public:
    /** Constructs a new representation for a window task. */
    WindowTaskItem(QGraphicsItem *parent, QObject *parentObject);

    /** Sets the window represented by this task. */
    void setWindowTask(Task::TaskPtr task);
    /** Returns the window represented by this task. */
    Task::TaskPtr windowTask() const;
    /** Tells the window manager the minimized task's geometry. */
    void publishIconGeometry();

    virtual void activate();
    virtual void close();

    /** Overrided from LayoutItem */
    void setGeometry(const QRectF& geometry);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private slots:
    void updateTask();

private:
    Task::TaskPtr _task;
};

/**
 * A task item which represents a newly started task which does not yet have
 * a window visible on screen.
 *
 * Startup tasks are short-lived and disappear if
 */
class StartupTaskItem : public AbstractTaskItem
{
public:
    /** Constructs a new representation for a starting task. */
    StartupTaskItem(QGraphicsItem *parent, QObject *parentObject);

    /** Sets the starting task represented by this item. */
    void setStartupTask(Startup::StartupPtr task);
    /** Returns the starting task represented by this item. */
    Startup::StartupPtr startupTask() const;

    // reimplemented, does nothing because there is no window to show
    virtual void activate() {};

private:
    Startup::StartupPtr _task;
};

/**
 * A graphics item which holds a group of tasks.
 * To add a task to a group, set the AbstractTaskItem's parent
 * to the TaskGroupItem instance.
 *
 * Task groups can also contain other task groups.
 */
class TaskGroupItem : public AbstractTaskItem
{
    Q_OBJECT

public:
    /** Constructs a new task group with the specified parent. */
    TaskGroupItem(QGraphicsItem *parent, QObject *parentObject);

    /**
     * Specifies whether this group may contain sub-groups.
     * Defaults to true.
     */
    void setAllowSubGroups(bool subGroups);
    bool allowSubGroups() const;

    /**
     * Returns the group's tasks in the order in which they are laid out,
     * top-to-bottom for vertical groups and left-to-right for horizontal
     * groups.
     */
    QList<AbstractTaskItem*> tasks() const;

    /**
     * Inserts a new task item into the group.  The task is
     * removed from its existing group (if any).
     *
     * @param item The task item to insert into the group
     * @param index The position in which to insert the item.
     * If this is 0 the item will be the first in the group.
     * If this is tasks().count() or -1 the item will be
     * the last in the group.
     */
    void insertTask(AbstractTaskItem *item, int index = -1);

    /** Removes a task item from the group. */
    void removeTask(AbstractTaskItem *item);

    /**
     * Reorders a task item within a group.
     *
     * @param from The current position of the task to move
     * @param to The new position of the task to move
     */
    void reorderTasks(int from, int to);

    /**
     * The enum describes the available styles for the border
     * which can surround the group.
     */
    enum BorderStyle
    {
        NoBorder,
        CaptionBorder
    };
    /** Sets the style of the border which surrounds the group. */
    void setBorderStyle(BorderStyle style);
    /** Returns the style of the border which surrounds the group. */
    BorderStyle borderStyle() const;

    /**
     * Sets the color of the group.  This is used to render
     * the border and tint the background.
     */
    void setColor(const QColor &color);

    /** Returns the color of the group.  See setColor() */
    QColor color() const;

    /**
     * Sets whether the task group is collapsed.
     * TODO: Document me
     */
    void setCollapsed(bool collapsed);
    /** Returns whether the task group is collapsed. */
    bool collapsed() const;

    /**
     * Cycle the active task in a circular behaviour
     *
     * @param delta  if >0 go to the previous one, else go to the successive one
     */
    void cycle(int delta);

    // reimplemented
    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget);

    virtual void activate();
    virtual void close();
    virtual QSizeF maximumSize() const;

    /** Event compression **/
    void queueGeometryUpdate();

public slots:
    void updateActive(AbstractTaskItem *task);

protected:
    /** Reimplemented **/
    virtual void timerEvent(QTimerEvent *event);

private:
    enum DropAction
    {
        NoAction,
        InsertTaskAction, // insert the dropped task into the group
        GroupTasksAction  // group the dropped task together with the
                          // task underneath it and insert the
                          // group at the event's position
    };
    DropAction dropAction(const QPointF &pos) const;

    void drawBorder(QPainter *painter,
                    const QStyleOptionGraphicsItem *option,
                    const QRectF &area);
    qreal titleHeight() const;
    static QFont titleFont();

    class TaskEntry
    {
    public:
        TaskEntry(AbstractTaskItem *taskItem,
                  const QRectF &taskArea = QRectF())
        : task(taskItem),
          rect(taskArea)
        {}

        bool operator==(const TaskEntry entry) const { return entry.task == task; }

        AbstractTaskItem *task;
        QRectF rect;
    };
    QList<TaskEntry> _tasks;
    int _activeTask;
    BorderStyle _borderStyle;
    QColor _color;
    DropAction _potentialDropAction;
    int _caretIndex;
    bool _allowSubGroups;
    int m_geometryUpdateTimerId;

    static const int CaretWidth = 5;
    static const int GroupBorderWidth = 16;
};


/**
 * Base class for strategies which can be used to
 * automatically group tasks.
 */
class AbstractGroupingStrategy
{
public:
    virtual ~AbstractGroupingStrategy() {};

    /**
     * Specifies a suggested grouping of tasks
     */
    class GroupSuggestion
    {
    public:
        /**
         * Constructs a new GroupSuggestion for @p tasks
         * with a suggested @p name
         */
        GroupSuggestion(const QString &name,
                        const QSet<AbstractTaskItem*> &tasks);

        /** A suggested name for the group. */
        QString name() const;
        /** The tasks to group. */
        QSet<AbstractTaskItem*> tasks() const;
    private:
        QString _name;
        QSet<AbstractTaskItem*> _tasks;
    };

    /**
     * Examines a set of @p tasks and returns a list of
     * suggested named sub-groups of tasks.
     *
     * The suggested groups may include all, some or none from the
     * @p tasks set.
     *
     * Sub-classes must re-implement this method to arrange tasks
     * into groups according to various criteria.
     */
    virtual QList<GroupSuggestion> suggestGroups(const QSet<AbstractTaskItem*> tasks) = 0;
};

K_EXPORT_PLASMA_APPLET(tasks, Tasks)

#endif
