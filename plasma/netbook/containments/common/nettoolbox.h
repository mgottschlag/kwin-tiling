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

#include <KIcon>

class QGraphicsLinearLayout;

namespace Plasma
{
    class Containment;
    class Frame;
    class IconWidget;
    class Svg;
};

class NetToolBox : public QGraphicsWidget
{
    Q_OBJECT
public:
    NetToolBox(Plasma::Containment *parent = 0);
    ~NetToolBox();

    bool showing() const;
    void setShowing(const bool show);

    /**
     * create a toolbox tool from the given action
     * @p action the action to associate hte tool with
     */
    void addTool(QAction *action);
    /**
     * remove the tool associated with this action
     */
    void removeTool(QAction *action);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

Q_SIGNALS:
    void toggled();
    void visibilityChanged(bool);

private Q_SLOTS:
    void containmentGeometryChanged();
    void animateHighlight(qreal progress);

private:
    Plasma::Frame *m_toolContainer;
    QGraphicsLinearLayout *m_toolContainerLayout;
    QHash<QAction *, Plasma::IconWidget *> m_actionButtons;
    Plasma::Containment *m_containment;
    Plasma::Svg *m_background;
    KIcon m_icon;
    QSize m_iconSize;
    int m_animHighlightId;
    qreal m_animHighlightFrame;
    bool m_hovering;
};

#endif
