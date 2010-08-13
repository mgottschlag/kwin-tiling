/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
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

#ifndef NETTOOLBOX_H
#define NETTOOLBOX_H

#include <QGraphicsWidget>
#include <QPropertyAnimation>
#include <KIcon>

#include <plasma/abstracttoolbox.h>
#include <plasma/containment.h>


class QGraphicsLinearLayout;

namespace Plasma
{
    class Containment;
    class IconWidget;
    class Svg;
}

class ToolContainer;

class NetToolBox : public Plasma::AbstractToolBox
{
    Q_OBJECT
    Q_PROPERTY(bool showing READ isShowing WRITE setShowing )
    Q_PROPERTY(qreal highlight READ highlight WRITE setHighlight)
public:
    explicit NetToolBox(Plasma::Containment *parent = 0);
    explicit NetToolBox(QObject *parent, const QVariantList &args);
    ~NetToolBox();

    bool isShowing() const;
    void setShowing(const bool show);

    /**
     * create a toolbox tool from the given action
     * @p action the action to associate the tool with
     */
    void addTool(QAction *action);
    /**
     * remove the tool associated with this action
     */
    void removeTool(QAction *action);

    QRectF expandedGeometry() const;

    void setLocation(Plasma::Location location);
    Plasma::Location location() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

protected:
    void init();

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private Q_SLOTS:
    void containmentGeometryChanged();
    void setHighlight(qreal progress);
    qreal highlight();
    void movementFinished();
    void onMovement(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);

private:
    void highlight(bool highlighting);
    ToolContainer *m_toolContainer;
    QGraphicsLinearLayout *m_toolContainerLayout;
    QHash<QAction *, Plasma::IconWidget *> m_actionButtons;
    Plasma::Containment *m_containment;
    Plasma::Svg *m_background;
    KIcon m_icon;
    KIcon m_closeIcon;
    QSize m_iconSize;
    Plasma::Animation *slideAnim;
    QPropertyAnimation *anim;
    qreal m_animHighlightFrame;
    bool m_hovering;
    bool m_showing;
    Plasma::Location m_location;
    int m_newToolsPosition;
};

K_EXPORT_PLASMA_TOOLBOX(nettoolbox, NetToolBox)

#endif
