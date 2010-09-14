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

#include <QGraphicsItemAnimation>
#include <QIcon>
#include <QPainter>

#include <KIcon>

#include "qs_matchitem.h"

namespace QuickSand
{

MatchItem::MatchItem(const QIcon &icon, const QString &name, const QString &desc, QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_anim(0),
      m_name(name),
      m_desc(desc)
{
    if (icon.isNull()) {
        m_icon = KIcon(QLatin1String( "unknown" ));
    } else {
        m_icon = icon;
    }
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    resize(ITEM_SIZE, ITEM_SIZE);
    setToolTip(QString(QLatin1String( "%1: %2" )).arg(name).arg(desc));
}

MatchItem::~MatchItem()
{
    delete m_anim;
}

QGraphicsItemAnimation* MatchItem::anim(bool create)
{
    if (create) {
        delete m_anim;
        m_anim = new QGraphicsItemAnimation();
        m_anim->setItem(this);
    }
    return m_anim;
}

void MatchItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->setRenderHint(QPainter::Antialiasing);

    if (hasFocus() || isSelected()) {
        painter->drawPixmap(0, 0, m_icon.pixmap(64, 64, QIcon::Active));
    } else {
        painter->drawPixmap(0, 0, m_icon.pixmap(64, 64, QIcon::Disabled));
    }
    //TODO: Make items glow on hover and draw text over them
}

void MatchItem::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    Q_UNUSED(e)
    emit activated(this);
}

} // namespace QuickSand
