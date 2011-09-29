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
#include "outputconfig.h"
#include "randr.h"

#include <QPen>
#include <QBrush>
#include <QFont>
#include <QGraphicsScene>
#include <KGlobalSettings>

OutputGraphicsItem::OutputGraphicsItem(OutputConfig *config)
	: m_config( config )
{
	m_left = m_right = m_top = m_bottom = NULL;

	setPen(QPen(Qt::black));

	setFlag(QGraphicsItem::ItemIsMovable, false);
// FIXME not implemented yet	setFlag(QGraphicsItem::ItemIsSelectable, true);
	
	m_text = new QGraphicsTextItem(QString(), this);
	
	QFont font = KGlobalSettings::generalFont();
	font.setPixelSize(72);
	m_text->setFont(font);
	setVisible( false );
	m_text->setVisible( false );
        calculateSetRect( config );
}

OutputGraphicsItem::~OutputGraphicsItem()
{
	disconnect();
}

void OutputGraphicsItem::configUpdated()
{
	if( !m_config->isActive()) {
		setVisible( false );
		m_text->setVisible( false );
		return;
	}
	setVisible( true );
	m_text->setVisible( true );
	calculateSetRect( m_config );
	setBrush(QColor(0, 255, 0, 128));
	setObjectName(m_config->output()->name());
	
	// An example of this description text with radeonhd on randr 1.2:
	// DVI-I_2/digital
	// 1680x1050 (60.0 Hz)
	QString refresh = QString::number(m_config->refreshRate(), 'f', 1);
	
	m_text->setPlainText( i18nc("Configuration options. Output name, width x height (refresh rate Hz)", "%1\n%2x%3 (%4 Hz)",
		m_config->output()->name(), m_config->rect().width(), m_config->rect().height(), refresh) );
	// more accurate text centering
	QRectF textRect = m_text->boundingRect();
	m_text->setPos( rect().x() + (rect().width() - textRect.width()) / 2,
	                rect().y() + (rect().height() - textRect.height()) / 2);
}

void OutputGraphicsItem::calculateSetRect( OutputConfig* config )
{
    switch( config->rotation() & RandR::RotateMask )
    {
        case RandR::Rotate0:
        case RandR::Rotate180:
            setRect( config->rect());
            break;
        case RandR::Rotate90:
        case RandR::Rotate270:
            setRect( config->rect().x(), config->rect().y(), config->rect().height(), config->rect().width());
            break;
    }
}

OutputGraphicsItem *OutputGraphicsItem::left() const
{
	return m_left;
}

OutputGraphicsItem *OutputGraphicsItem::right() const
{
	return m_right;
}

OutputGraphicsItem *OutputGraphicsItem::top() const
{
	return m_top;
}

OutputGraphicsItem *OutputGraphicsItem::bottom() const
{
	return m_bottom;
}

void OutputGraphicsItem::setTop(OutputGraphicsItem *output)
{
	// if we already have that output at top, then just return
	if (m_top == output)
	return;
   
   OutputGraphicsItem *oldTop = m_top;
   m_top = output;

   // if we currently have a top item, set the given item as the bottom of our old item
   if (oldTop)
	   oldTop->setBottom(output);

   // check whether we have a left->top or a right->top to update the pointers
   if (m_left && m_left->top())
   {
	   OutputGraphicsItem *item = m_left->top();
	   if (item->right())
	   qDebug("Oops, this should not happen");
	item->setRight(output);
	if (output)
		output->setLeft(item);
   }

   if (m_right && m_right->top())
   {
	   OutputGraphicsItem *item = m_right->top();
	   if (item->left())
	   qDebug("Oops, this should not happen");
	   item->setLeft(output);
	   if (output)
		   output->setRight(item);
   }
}

void OutputGraphicsItem::setBottom(OutputGraphicsItem *output)
{
	// if we already have that output at bottom, just return
	if (m_bottom == output)
	return;
   
   OutputGraphicsItem *oldBottom = m_bottom;
   m_bottom = output;

   // if we currently have a bottom item, set the given item as the top of our old item
   if (oldBottom)
	oldBottom->setTop(output);

   // check whether we have a left->bottom or a right->bottom to update the pointers
   if (m_left && m_left->bottom())
   {
	   OutputGraphicsItem *item = m_left->bottom();
	   if (item->right())
	   qDebug("Oops, this should not happen");
	item->setRight(output);
	if (output)
		output->setLeft(item);
   }

   if (m_right && m_right->bottom())
   {
	   OutputGraphicsItem *item = m_right->bottom();
	   if (item->left())
	   qDebug("Oops, this should not happen");
	   item->setLeft(output);
	   if (output)
		   output->setRight(item);
   }
}

void OutputGraphicsItem::setLeft(OutputGraphicsItem *output)
{
	// if we already have that output at left, then just return
	if (m_left == output)
	return;
   
   OutputGraphicsItem *oldLeft = m_left;
   m_left = output;

   // if we currently have a left item, set the given item as the right of our old item
   if (oldLeft)
	oldLeft->setRight(output);

   // check whether we have a top->left or a bottom->left to update the pointers
   if (m_top && m_top->left())
   {
	   OutputGraphicsItem *item = m_top->left();
	   if (item->bottom())
	   qDebug("Oops, this should not happen");
	item->setBottom(output);
	if (output)
		output->setTop(item);
   }

   if (m_bottom && m_bottom->left())
   {
	   OutputGraphicsItem *item = m_bottom->left();
	   if (item->top())
	   qDebug("Oops, this should not happen");
	   item->setTop(output);
	   if (output)
		   output->setBottom(item);
   }
}

void OutputGraphicsItem::setRight(OutputGraphicsItem *output)
{
	// if we already have that output at right, then just return
	if (m_right == output)
	return;
   
   OutputGraphicsItem *oldRight = m_right;
   m_right = output;

   // if we currently have a right item, set the given item as the left of our old item
   if (oldRight)
	oldRight->setLeft(output);

   // check whether we have a top->right or a bottom->right to update the pointers
   if (m_top && m_top->right())
   {
	   OutputGraphicsItem *item = m_top->right();
	   if (item->bottom())
	   qDebug("Oops, this should not happen");
	item->setBottom(output);
	if (output)
		output->setTop(item);
   }

   if (m_bottom && m_bottom->right())
   {
	   OutputGraphicsItem *item = m_bottom->right();
	   if (item->top())
	   qDebug("Oops, this should not happen");
	   item->setTop(output);
	   if (output)
		   output->setBottom(item);
   }
}

void OutputGraphicsItem::disconnect()
{
	// for now just disconnect everything
	if (m_top)
	{
		m_top->m_bottom = NULL;
		if (!m_top->isConnected())
			emit itemChanged(m_top);
	}
	if (m_bottom)
	{
		m_bottom->m_top = NULL;
		if (!m_bottom->isConnected())
			emit itemChanged(m_bottom);
	}
	if (m_left)
	{
		m_left->m_right = NULL;
		if (!m_left->isConnected())
			emit itemChanged(m_left);
	}
	if (m_right)
	{
		m_right->m_left = NULL;
		if (!m_right->isConnected())
			emit itemChanged(m_right);
	}

	m_top = m_bottom = m_left = m_right = NULL;
}

void OutputGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	// disconnect from the current layout
	disconnect();

	QGraphicsRectItem::mousePressEvent(event);
}

void OutputGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   QGraphicsRectItem::mouseReleaseEvent(event);
   emit itemChanged(this);
}

bool OutputGraphicsItem::isConnected()
{
	return (m_top != NULL || m_bottom != NULL || m_left != NULL || m_right != NULL);
}

void OutputGraphicsItem::setPrimary(bool primary)
{
	QPen p=pen();
	p.setWidth(primary ? rect().width()/100 : 0);
	setPen(p);
}

bool OutputGraphicsItem::isPrimary() const
{
	return pen().width()>0;
}

#include "outputgraphicsitem.moc"
