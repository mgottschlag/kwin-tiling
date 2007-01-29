/**
  * This file is part of the KDE project
  * Copyright (C) 2007, 2006 Rafael Fernández López <ereslibre@gmail.com>
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

#include "appletlistmodel.h"
#include "appletitemdelegate.h"

class AppletListModel::Private
{
public:
	Private()
	{
	}

	~Private()
	{
	}

	AppletInfo::List appletInfoList;
};

AppletListModel::AppletListModel(const AppletInfo::List &appletInfoList, QObject *parent)
	: QAbstractListModel(parent)
	, d(new Private)
{
	d->appletInfoList = appletInfoList;
}

AppletListModel::~AppletListModel()
{
	delete d;
}

QVariant AppletListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	AppletInfo *myAppletInfo = static_cast<AppletInfo*>(index.internalPointer());

	QVariant returnValue;

	switch (role)
	{
		case Qt::DisplayRole:
			returnValue = myAppletInfo->name();
			break;

		case Qt::DecorationRole:
			returnValue = KIcon(myAppletInfo->icon());
			break;

		case AppletItemDelegate::SecondaryDisplayRole:
			returnValue = myAppletInfo->comment();
			break;
	}

	return returnValue;
}

QModelIndex AppletListModel::index(int row, int column, const QModelIndex &parent) const
{
	if (row < 0)
		row = 0;

	if (row < d->appletInfoList.count())
	{
		return createIndex(row, column, &d->appletInfoList[row]);
	}
	else if (!d->appletInfoList.empty())
	{
		int lastRow = d->appletInfoList.count() - 1;
		return createIndex(lastRow, column, &d->appletInfoList[lastRow]);
	}

	return QModelIndex(); // We have an empty list
}

int AppletListModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	
	return d->appletInfoList.count();
}

#include "appletlistmodel.moc"
