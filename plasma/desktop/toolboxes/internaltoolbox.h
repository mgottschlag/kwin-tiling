/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_INTERNALTOOLBOX_P_H
#define PLASMA_INTERNALTOOLBOX_P_H

#include <QGraphicsWidget>

#include <Plasma/Plasma>
#include <Plasma/Containment>
#include <Plasma/AbstractToolBox>

class QAction;

class KConfigGroup;



class IconWidget;
class InternalToolBoxPrivate;

class InternalToolBox : public Plasma::AbstractToolBox
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    enum Corner {
        Top = 0,
        TopRight,
        TopLeft,
        Left,
        Right,
        Bottom,
        BottomRight,
        BottomLeft
    };

    explicit InternalToolBox(Plasma::Containment *parent);
    explicit InternalToolBox(QObject *parent = 0, const QVariantList &args = QVariantList());
    ~InternalToolBox();

    /**
     * create a toolbox tool from the given action
     * @p action the action to associate hte tool with
     */
    void addTool(QAction *action);
    /**
     * remove the tool associated with this action
     */
    void removeTool(QAction *action);
    bool isEmpty() const;
    int size() const;
    void setSize(const int newSize);
    QSize iconSize() const;
    void  setIconSize(const QSize newSize);
    bool isShowing() const;
    void setShowing(const bool show);

    virtual QGraphicsWidget *toolParent();

    virtual void setCorner(const Corner corner);
    virtual Corner corner() const;

    bool isMovable() const;
    void setIsMovable(bool movable);

    virtual QSize fullWidth() const;
    virtual QSize fullHeight() const;
    virtual QSize cornerSize() const;
    virtual void updateToolBox() {}

    void setIconic(bool iconic);
    bool iconic() const;

    virtual void showToolBox() = 0;
    virtual void hideToolBox() = 0;

    QList<QAction *> actions() const;

public Q_SLOTS:
    void save(KConfigGroup &cg) const;
    void restore(const KConfigGroup &containmentGroup);
    void reposition();

protected:
    Plasma::Containment *containment();
    QPoint toolPosition(int toolHeight);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

protected Q_SLOTS:
    virtual void toolTriggered(bool);
    void actionDestroyed(QObject *object);
    void immutabilityChanged(Plasma::ImmutabilityType immutability);

private:
    void init();

    Plasma::Containment *m_containment;
    InternalToolBox::Corner m_corner;
    int m_size;
    QSize m_iconSize;
    QPoint m_dragStartRelative;
    QTransform m_viewTransform;
    QList<QAction *> m_actions;
    bool m_hidden : 1;
    bool m_showing : 1;
    bool m_movable : 1;
    bool m_dragging : 1;
    bool m_userMoved : 1;
    bool m_iconic : 1;
};



#endif // PLASMA_INTERNALTOOLBOX_P_H

