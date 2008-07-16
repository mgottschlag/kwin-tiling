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


#ifndef WINDOWTASKITEM_H
#define WINDOWTASKITEM_H

// KDE
#include <taskmanager/taskmanager.h>

// Qt
#include <QIcon>
#include <QTextLayout>
#include <QTime>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>

#include <KColorScheme>

class QTimeLine;

// Plasma
#include <plasma/animator.h>

namespace Plasma
{
    class PanelSvg;
}

class Tasks;

/**
 * A task item for a task which represents a window on the desktop.
 */
class WindowTaskItem : public QGraphicsWidget
{
    Q_OBJECT

public:
    /** Constructs a new representation for a window task. */
    WindowTaskItem(Tasks *parent, const bool showTooltip);

    /**Destruct the representation of the window task */
    ~WindowTaskItem();

    /** Sets the starting task represented by this item. */
    void setStartupTask(TaskManager::StartupPtr task);

    /** Sets the window represented by this task. */
    void setWindowTask(TaskManager::TaskPtr task);
    /** Returns the window represented by this task. */
    TaskManager::TaskPtr windowTask() const;

    /** Tells the window manager the minimized task's geometry. */
    void publishIconGeometry();

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
    void setTaskFlags(TaskFlags flags);

    /** Returns the task's current flags. */
    TaskFlags taskFlags() const;

    /** Returns current text for this task. */
    QString text() const;

    /** Returns the current icon for this task. */
    QIcon icon() const;

    virtual void close();

    /** Overrided from LayoutItem */
    void setGeometry(const QRectF& geometry);

signals:
    /** Emitted when a window is selected for activation, minimization, iconification */
    void windowSelected(WindowTaskItem *);

    void activated(WindowTaskItem *);

public slots:
    virtual void activate();

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);

    /** Event compression **/
    void queueUpdate();

    // reimplemented
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void timerEvent(QTimerEvent *event);
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
    virtual void paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget);

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


private slots:
    void updateTask();
    void animationUpdate(qreal progress);

private:
    Tasks *m_applet;
    TaskManager::TaskPtr m_task;
    QTimer* m_activateTimer;

    bool m_showTooltip;
    // area of item occupied by task's icon
    QRectF iconRect(const QRectF &bounds) const;
    // area of item occupied by task's text
    QRectF textRect(const QRectF &bounds) const;

    TaskFlags m_flags;

    QIcon m_icon;
    QString m_text;

    int m_animId;
    qreal m_alpha;
    bool m_fadeIn;

    QPointF _dragOffset;
    QTime m_lastUpdate;
    int m_updateTimerId;
    int m_attentionTimerId;
    int m_attentionTicks;

    // distance (in pixels) between a task's icon and its text
    static const int IconTextSpacing = 4;

    static const int TaskItemHorizontalMargin = 4;
    static const int TaskItemVerticalMargin = 4;
};

#endif
