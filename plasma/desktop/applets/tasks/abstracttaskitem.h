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
#include <QFocusEvent>
#include <QGraphicsWidget>
#include <QPropertyAnimation>

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
    Q_PROPERTY(QPointF animationPos READ animationPos WRITE setAnimationPos)
    Q_PROPERTY(qreal backgroundFadeAlpha READ backgroundFadeAlpha WRITE setBackgroundFadeAlpha)

public:
    /** Constructs a new representation for an abstract task. */
    AbstractTaskItem(QGraphicsWidget *parent, Tasks *applet);

     /** Destruct the representation for an abstract task. */
    ~AbstractTaskItem();

    /** The text changed for this task item. */
    void textChanged();

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
    virtual QString text() const;

    /** Returns the current icon for this task. */
    QIcon icon() const;

    virtual void close();

    /** Tells the window manager the minimized task's geometry. */
    virtual void publishIconGeometry() const;
    virtual void publishIconGeometry(const QRect &rect) const;
    QRect iconGeometry() const; // helper for above

    /** Overridden from LayoutItem */
    void setGeometry(const QRectF& geometry);

    /** Convenience Functions to get information about Grouping */
    /** Only true if the task is not only member of rootGroup */
    bool isGrouped() const;
    bool isGroupMember(const TaskGroupItem *group) const;
    TaskGroupItem *parentGroup() const;

    virtual bool isWindowItem() const = 0;
    virtual bool isActive() const = 0;

    virtual void setAdditionalMimeData(QMimeData* mimeData) = 0;

    void setLayoutWidget(LayoutWidget* widget);
    TaskManager::AbstractGroupableItem * abstractItem();

    /** Returns the preferred size calculated on base of the fontsize and the iconsize*/
    QSize basicPreferredSize() const;
    void setPreferredOffscreenSize();
    void setPreferredOnscreenSize();

    //TODO: to be removed when we have proper animated layouts
    QPointF animationPos() const;
    void setAnimationPos(const QPointF &pos);

Q_SIGNALS:
    void activated(AbstractTaskItem *);
    void destroyed(AbstractTaskItem *);

public Q_SLOTS:
    virtual void activate() = 0;
    void toolTipAboutToShow();
    void toolTipHidden();
    void activateWindow(WId id, Qt::MouseButtons buttons);

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);

    // reimplemented
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
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
    void drawTextLayout(QPainter *painter, const QTextLayout &layout, const QRect &rect);

    virtual void updateTask(::TaskManager::TaskChanges changes) = 0; // pure virtual function
    virtual void updateToolTip() = 0; // pure virtual function
    void clearToolTip();
    QString expanderElement() const;
    void stopWindowHoverEffect();
    bool shouldIgnoreDragEvent(QGraphicsSceneDragDropEvent *event);

    // area of item occupied by task's icon
    QRectF iconRect(const QRectF &bounds);
    // area for the expander arrow for group items
    QRectF expanderRect(const QRectF &b);
    // area of item occupied by task's text
    QRectF textRect(const QRectF &bounds);
    // start an animation to chnge the task background
    void fadeBackground(const QString &newBackground, int duration);
    // text color, use this because it could be animated
    QColor textColor() const;
    void resizeBackground(const QSize &size);

    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void setAbstractItem(TaskManager::AbstractGroupableItem *item);

protected Q_SLOTS:
    /** Event compression **/
    void queueUpdate();

    qreal backgroundFadeAlpha() const;
    void setBackgroundFadeAlpha(qreal progress);

    void syncActiveRect();
    void checkSettings();

protected:
    Tasks *m_applet;
    TaskFlags m_flags;

private:
    QPropertyAnimation *m_layoutAnimation;
    QPropertyAnimation *m_backgroundFadeAnim;

    qreal m_alpha;
    QString m_oldBackgroundPrefix;
    QString m_backgroundPrefix;

    QWeakPointer<TaskManager::AbstractGroupableItem> m_abstractItem;
    QPixmap m_cachedShadow;

    QRectF m_activeRect;

    QTime m_lastGeometryUpdate;
    QTime m_lastUpdate;
    QSize m_lastIconSize;
    int m_activateTimerId;
    int m_updateGeometryTimerId;
    int m_updateTimerId;
    int m_hoverEffectTimerId;
    int m_attentionTimerId;
    int m_attentionTicks;

    WId m_lastViewId;

    bool m_showText : 1;
    bool m_layoutAnimationLock : 1;
    bool m_firstGeometryUpdate : 1;
    QPointF m_oldDragPos;
};

#endif
