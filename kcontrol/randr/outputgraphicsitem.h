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

#ifndef OUTPUTGRAPHICSITEM_H
#define OUTPUTGRAPHICSITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

#include "randr.h"

class OutputConfig;

class OutputGraphicsItem : public QObject, public QGraphicsRectItem
{
	Q_OBJECT
public:
	OutputGraphicsItem(OutputConfig *config);
	~OutputGraphicsItem();

	void configUpdated(); // updates from OutputConfig

	OutputGraphicsItem *left() const;
	OutputGraphicsItem *right() const;
	OutputGraphicsItem *top() const;
	OutputGraphicsItem *bottom() const;

	void setLeft(OutputGraphicsItem *output);
	void setRight(OutputGraphicsItem *output);
	void setTop(OutputGraphicsItem *output);
	void setBottom(OutputGraphicsItem *output);

	bool isConnected();
	bool isPrimary() const;
	void setPrimary(bool);
    
protected:
	void disconnect();
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:
	void itemChanged(OutputGraphicsItem *item);

private:
        void calculateSetRect( OutputConfig* config );
	OutputGraphicsItem *m_left;
	OutputGraphicsItem *m_right;
	OutputGraphicsItem *m_top;
	OutputGraphicsItem *m_bottom;

	OutputConfig *m_config;
	QGraphicsTextItem *m_text;


};

#endif

