/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "outputgraphicsitem.h"

#include <QPen>
#include <QBrush>
#include <QFont>
#include "randroutput.h"
#include "randr.h"
#ifdef HAS_RANDR_1_2

OutputGraphicsItem::OutputGraphicsItem(RandROutput *output)
: QGraphicsRectItem(output->rect())
{
	setPen(QPen(Qt::black));
	setBrush(QColor(0,255,0,128));
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	m_text = new QGraphicsTextItem(output->name(), this);

	QFont f = m_text->font();
	f.setPixelSize(72);
	m_text->setFont(f);

	QRect r = output->rect();
	m_text->setPos(r.width()/2,r.height()/2);
}

OutputGraphicsItem::~OutputGraphicsItem()
{
}

#endif
