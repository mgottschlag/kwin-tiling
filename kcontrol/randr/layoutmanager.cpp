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

#include "layoutmanager.h"
#include "randr.h"
#include "randrscreen.h"
#include "randroutput.h"
#include "outputgraphicsitem.h"

#include <QGraphicsScene>
#include <cmath>
#include <math.h>

LayoutManager::LayoutManager(RandRScreen *screen, QGraphicsScene *scene)
: QObject(screen)
{
	m_screen = screen;
	m_scene = scene;
}

LayoutManager::~LayoutManager()
{
}

void LayoutManager::slotAdjustOutput(OutputGraphicsItem *output)
{
	QPointF p = output->pos();
	float nearest = m_scene->width() * m_scene->height();
	OutputGraphicsItem *selected = NULL;

	OutputGraphicsItem *mouseGrabber = dynamic_cast<OutputGraphicsItem*>(m_scene->mouseGrabberItem());
	// find the nearest item
	QList<QGraphicsItem *> itemList = m_scene->items();

	foreach(QGraphicsItem *current, itemList)
	{
		OutputGraphicsItem *cur = dynamic_cast<OutputGraphicsItem*>(current);
		if (cur == output || cur == mouseGrabber)
			continue;

		QPointF pos = cur->pos();
		float distance = (p.x() - pos.x())*(p.x()-pos.x()) + (p.y() - pos.y())*(p.y() - pos.y());
		if (distance <=nearest)
		{
			nearest = distance;
			selected  = cur;
		}
	}

	if (selected)
	{
		// find in which side this
		QRectF s = selected->boundingRect();
		QRectF i = output->boundingRect();

		s.translate(selected->scenePos());
		i.translate(output->scenePos());

		// calculate the distances
		float top = fabsf(i.top() - s.bottom());
		float bottom = fabsf(i.bottom() - s.top());
		float left = fabsf(i.left() - s.right());
		float right = fabsf(i.right() - s.left());

		// choose top
		if (top <= bottom && top <= left && top <= right)
		{
			output->setTop(selected);
			selected->setBottom(output);
		}
		// choose bottom
		else if (bottom < top && bottom <= left && bottom <= right)
		{
			output->setBottom(selected);
			selected->setTop(output);
		}
		// choose left
		else if (left < top && left < bottom && left <= right)
		{
			output->setLeft(selected);
			selected->setRight(output);
		}
		// choose right
		else
		{
			output->setRight(selected);
			selected->setLeft(output);
		}
	}

	// now visit all the outputs on the screen to adjust their positions
	// starting by the item selected to be the parent of the current item
	QList<OutputGraphicsItem *> visitedList;

	// FIXME: after adjusting the scene, we have to translate everything back to
	// the 0,0 position so that everything is onscreen
	output->setPos(0,0);
	// call a recursive function to adjust the outputs
	adjustScene(output, visitedList);
}

void LayoutManager::adjustScene(OutputGraphicsItem *current, QList<OutputGraphicsItem*> &visited)
{
	visited.append(current);

	OutputGraphicsItem *item;
	item = current->left();
	if (item && visited.indexOf(item) == -1)
	{
	item->setPos(current->x() - item->boundingRect().width(), current->y());
	adjustScene(item, visited);
	}

	item = current->right();
	if (item && visited.indexOf(item) == -1)
	{
	item->setPos(current->x() + current->boundingRect().width(), current->y());
	adjustScene(item, visited);
	}

	item = current->top();
	if (item && visited.indexOf(item) == -1)
	{
	item->setPos(current->x(), current->y() - item->boundingRect().height());
	adjustScene(item, visited);
	}

	item = current->bottom();
	if (item && visited.indexOf(item) == -1)
	{
	item->setPos(current->x(), current->y() + current->boundingRect().height());
	adjustScene(item, visited);
	}
	
}
