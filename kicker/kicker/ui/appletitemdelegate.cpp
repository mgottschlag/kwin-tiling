/**
  * This file is part of the KDE project
  * Copyright (C) 2006 Rafael Fernández López <ereslibre@gmail.com>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License version 2 as published by the Free Software Foundation.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#include "appletitemdelegate.h"

AppletItemDelegate::AppletItemDelegate(QObject *parent)
	: QItemDelegate(parent)
{
}

void AppletItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (option.state & QStyle::State_Selected)
		painter->fillRect(option.rect, option.palette.highlight());

	QVariant icon = index.model()->data(index, Qt::DecorationRole);
	QPixmap iconPixmap = icon.value<QIcon>().pixmap(48, 48);

	QFont title(painter->font());
	QFont previousFont(painter->font());

	title.setPointSize(title.pointSize() + 2);
	title.setWeight(QFont::Bold);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setFont(title);
	painter->drawText(40 + iconPixmap.width(), 30 + option.rect.top(), index.model()->data(index, Qt::DisplayRole).toString());
	painter->setFont(previousFont);
	painter->drawText(40 + iconPixmap.width(), 60  + option.rect.top(), index.model()->data(index, SecondaryDisplayRole).toString());
	painter->drawPixmap(20, (option.rect.height() / 2) + option.rect.top() - (iconPixmap.height() / 2), iconPixmap);
	painter->restore();

}

QSize AppletItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(78, 78);
}

#include "appletitemdelegate.moc"
