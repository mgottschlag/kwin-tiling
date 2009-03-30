/***************************************************************************
 *   Copyright 2009 by Aaron Seigo <aseigo@kde.org>                        *
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

#ifndef SELECTIONBAR_H
#define SELECTIONBAR_H

#include <QGraphicsWidget>

class QTimer;

namespace Plasma
{
    class FrameSvg;
} // namespace Plasma

class ResultItem;

class SelectionBar : public QGraphicsWidget
{
    Q_OBJECT

public:
    SelectionBar(QGraphicsWidget *parent);

    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
    void getMargins(qreal &left, qreal &top, qreal &right, qreal &bottom) const;

signals:
    void graphicsChanged();

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);

private:
    void acquireTarget();

private slots:
    void itemSelected();
    void movementFinished(QGraphicsItem *item);
    void frameSvgChanged();
    void disappear();
    void targetDestroyed();

private:
    QTimer *m_hideTimer;
    Plasma::FrameSvg *m_frame;
    int m_animId;
    ResultItem *m_target;
};

#endif
