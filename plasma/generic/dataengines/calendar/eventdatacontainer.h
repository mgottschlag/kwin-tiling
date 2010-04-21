/*
    Copyright (c) 2010 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef EVENTDATACONTAINER_H
#define EVENTDATACONTAINER_H

#include <plasma/datacontainer.h>

class QAbstractItemModel;
class QModelIndex;
class QString;

namespace Akonadi {
    class DateRangeFilterProxyModel;
}

class EventDataContainer :public Plasma::DataContainer
{
    Q_OBJECT
public:
    EventDataContainer(QAbstractItemModel* sourceModel, const QString& name, const KDateTime& start, const KDateTime& end, QObject* parent = 0);

private Q_SLOTS:
    // when the model changes, we want to update the data for the applets,
    // therefor these are connected to the model:
    void calendarDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void rowsInserted(const QModelIndex &parent, int first, int last);
    void rowsRemoved(const QModelIndex &parent, int first, int last);

private:
    // update the list of incidents
    void updateData(Akonadi::DateRangeFilterProxyModel* model, const QModelIndex &parent);

    Akonadi::DateRangeFilterProxyModel* m_calendarDateRangeProxy;
    QString m_name;
};

#endif
