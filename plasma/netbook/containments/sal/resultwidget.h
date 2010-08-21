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

#ifndef PLASMA_RESULTWIDGET_H
#define PLASMA_RESULTWIDGET_H


#include <Plasma/IconWidget>

class QPropertyAnimation;

class ResultWidget : public Plasma::IconWidget
{
    Q_OBJECT
public:
    ResultWidget(QGraphicsItem *parent);
    ~ResultWidget();

    void animateHide();
    void animatePos(const QPointF &point);


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

protected Q_SLOTS:
    void animationFinished();

Q_SIGNALS:
    void dragStartRequested(ResultWidget *);

private:
    QPropertyAnimation *m_animation;
    bool m_shouldBeVisible;
};

#endif
