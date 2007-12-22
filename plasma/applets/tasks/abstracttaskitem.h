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

#ifndef ABSTRACTTASKITEM_H
#define ABSTRACTTASKITEM_H

// Qt
#include <QIcon>
#include <QTextLayout>
#include <QTime>

class QTimeLine;

// Plasma
#include <plasma/widgets/widget.h>

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

#endif
