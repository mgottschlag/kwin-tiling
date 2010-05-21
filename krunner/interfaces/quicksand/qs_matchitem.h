/*
 *   Copyright (C) 2007-2008 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 */

#ifndef QS_MATCHITEM_H
#define QS_MATCHITEM_H

#include <QGraphicsWidget>
#include <QIcon>

class QGraphicsItemAnimation;
class QGraphicsSceneMouseEvent;

namespace QuickSand
{
    // Deriving directly from QGraphicsItem causes crashes when animating
    class MatchItem : public QGraphicsWidget
    {
        Q_OBJECT
        public:
            MatchItem(const QIcon &icon, const QString &name, const QString &desc = QString(), QGraphicsWidget *parent = 0);
            ~MatchItem();

            void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

            QString id() const
            {
                return m_id;
            }
            QString name() const
            {
                return m_name;
            }
            QString description() const
            {
                return m_desc;
            }
            QIcon icon() const
            {
                return m_icon;
            }

            /**
             * @param create delete the current animation and create a new one
             * @return the animation associated with this item
             */
            QGraphicsItemAnimation* anim(bool create = false);

            static const int ITEM_SIZE = 64;
        signals:
            void activated(MatchItem *item);
        private:
            void mousePressEvent(QGraphicsSceneMouseEvent *e);

            QGraphicsItemAnimation *m_anim;
            QIcon m_icon;
            QString m_id;
            QString m_name;
            QString m_desc;
    };
}

#endif
