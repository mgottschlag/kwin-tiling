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

#ifndef APPLETITEMDELEGATE_H
#define APPLETITEMDELEGATE_H

#include <QItemDelegate>
#include <QModelIndex>
#include <QPixmap>
#include <QPainter>
#include <kicon.h>

class AppletItemDelegate
	: public QItemDelegate
{
	Q_OBJECT
	Q_ENUMS(AppletRole)

public:
	/**
	  * @brief Constructor for the item delegate.
	  */
	AppletItemDelegate(QObject *parent = 0);


	/**
	  * @brief Paints the item delegate.
	  */
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	/**
	  * @brief Gets the size of the item delegate.
	  */
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;


	enum AppletRole
	{
		SecondaryDisplayRole = 33
	};
};

#endif
