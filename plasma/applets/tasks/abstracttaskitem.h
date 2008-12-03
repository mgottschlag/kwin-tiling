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


#ifndef ABSTRACTTASKITEM_H
#define ABSTRACTTASKITEM_H

// KDE
#include <KColorScheme>
// Own
#include <taskmanager/taskgroup.h>

// Qt
#include <QTime>
#include <QIcon>
#include <QGraphicsWidget>

class QTextOption;
class QTextLayout;
class QString;


// Plasma
#include <Plasma/Animator>

class Tasks;
class TaskGroupItem;
class LayoutWidget;

/**
 * A baseclass for a task
 */
class AbstractTaskItem : public QGraphicsWidget
{
    Q_OBJECT

public:
    /** Constructs a new representation for an abstract task. */
    AbstractTaskItem(QGraphicsWidget *parent, Tasks *applet, const bool showTooltip);

     /** Destruct the representation for an abstract task. */
    ~AbstractTaskItem();

    /** Switch on/off tooltips above tasks */
    void setShowTooltip(const bool showit);

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
        TaskHasFocus       = 2,
        /**
         * Indicates that the task is iconified
         */
        TaskIsMinimized    = 4
    };
    Q_DECLARE_FLAGS(TaskFlags, TaskFlag)

    /** Sets the task flags for this item. */
    void setTaskFlags(const TaskFlags flags);

    /** Returns the task's current flags. */
    TaskFlags taskFlags() const;

    /** Returns current text for this task. */
    QString text() const;

    /** Returns the current icon for this task. */
    QIcon icon() const;

    virtual void close() = 0;

    /** Tells the window manager the minimized task's geometry. */
    virtual void publishIconGeometry(){};

    /** Overridden from LayoutItem */
    void setGeometry(const QRectF& geometry);

    /** Convenience Functions to get information about Grouping */
    /** Only true if the task is not only member of rootGroup */
    bool isGrouped() const;
    bool isGroupMember(const TaskGroupItem *group) const;
    TaskGroupItem *parentGroup() const;

    virtual bool isWindowItem() const = 0;
    virtual bool isActive() const = 0;

    void setLayoutWidget(LayoutWidget* widget);
    TaskManager::AbstractItemPtr abstractItem();

    /** Returns the preferred size calculated on base of the fontsize and the iconsize*/
    QSize basicPreferredSize() const;

Q_SIGNALS:
    void activated(AbstractTaskItem *);

public Q_SLOTS:
    virtual void activate() = 0;
    void toolTipAboutToShow();
    void toolTipHidden();

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);

    /** Event compression **/
    void queueUpdate();

    // reimplemented
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void timerEvent(QTimerEvent *event);
    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget);

    /** Draws the background for the task item. */
    virtual void drawBackground(QPainter *painter,const QStyleOptionGraphicsItem *option,
                                QWidget *widget);
    /** Draws the icon and text which represent the task item. */
    virtual void drawTask(QPainter *painter,const QStyleOptionGraphicsItem *option,
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
    * Draws the text layout (which must already have the text laid out) in the rect using
    * the supplied painter. If the layout contains text lines that are longer than the rect
    * is wide, they will be elided by fading the text out.
    */
    void drawTextLayout(QPainter *painter, const QTextLayout &layout, const QRect &rect) const;

    AbstractTaskItem * decodeMimedata(const QMimeData *mime);
    virtual void updateTask(::TaskManager::TaskChanges changes) = 0; // pure virtual function
    virtual void updateToolTip() = 0; // pure virtual function
    QString expanderElement() const;

protected Q_SLOTS:
    void animationUpdate(qreal progress);
    void syncActiveRect();
    void checkSettings();

protected:
    // area of item occupied by task's icon
    QRectF iconRect(const QRectF &bounds) const;
    // area for the expander arrow for group items
    QRectF expanderRect(const QRectF &b) const;
    // area of item occupied by task's text
    QRectF textRect(const QRectF &bounds) const;
    // start an animation to chnge the task background
    void fadeBackground(const QString &newBackground, int duration, bool fadeIn);
    // text color, use this because it could be animated
    QColor textColor() const;

    TaskManager::AbstractItemPtr m_abstractItem;
    LayoutWidget *m_layoutWidget;

    Tasks *m_applet;
    LayoutWidget *m_parentWidget;
    QTimer* m_activateTimer;


    TaskFlags m_flags;

    QIcon m_icon;
    QString m_text;

    int m_animId;
    qreal m_alpha;
    QString m_oldBackgroundPrefix;
    QString m_backgroundPrefix;
    QRectF m_activeRect;

    QPointF _dragOffset;
    QTime m_lastUpdate;
    int m_updateTimerId;
    int m_attentionTimerId;
    int m_attentionTicks;

    bool m_fadeIn : 1;
    bool m_showTooltip : 1;
    bool m_showingTooltip : 1;
    // distance (in pixels) between a task's icon and its text
    static const int IconTextSpacing = 4;

    static const int TaskItemHorizontalMargin = 4;
    static const int TaskItemVerticalMargin = 4;
};

#endif
