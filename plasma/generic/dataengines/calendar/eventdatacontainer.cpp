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

#include "eventdatacontainer.h"

#include <KDateTime>
#include <akonadi/kcal/incidencemimetypevisitor.h>

#include "daterangefilterproxymodel.h"
#include "calendarmodel.h"

EventDataContainer::EventDataContainer(QAbstractItemModel* sourceModel, const QString& name, const KDateTime& start, const KDateTime& end, QObject* parent)
    : Plasma::DataContainer(parent)
    , m_name(name)
{
    // name under which this dataEngine source appears
    setObjectName(name);

    // set up a proxy model to filter for the interesting dates only
    m_calendarDateRangeProxy = new Akonadi::DateRangeFilterProxyModel(this);
    m_calendarDateRangeProxy->setStartDate(start);
    m_calendarDateRangeProxy->setEndDate(end);
    m_calendarDateRangeProxy->setDynamicSortFilter(true);
    m_calendarDateRangeProxy->setSourceModel(sourceModel);

    // connect to react to model changes
    connect(m_calendarDateRangeProxy, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(calendarDataChanged(QModelIndex,QModelIndex)));
    connect(m_calendarDateRangeProxy, SIGNAL(rowsInserted(QModelIndex, int , int )),
            this, SLOT(rowsInserted(QModelIndex,int,int)));
    connect(m_calendarDateRangeProxy, SIGNAL(rowsRemoved(QModelIndex, int , int )),
            this, SLOT(rowsRemoved(QModelIndex,int,int)));

    // create the initial data
    updateData(m_calendarDateRangeProxy, QModelIndex());
}

void EventDataContainer::calendarDataChanged(const QModelIndex& topLeft, const QModelIndex&)
{
    Akonadi::DateRangeFilterProxyModel* calendarProxy = qobject_cast<Akonadi::DateRangeFilterProxyModel*>(sender());
    if (calendarProxy) {
        updateData(calendarProxy, topLeft);
    }
}

void EventDataContainer::rowsInserted(const QModelIndex& parent, int, int)
{
    Akonadi::DateRangeFilterProxyModel* calendarProxy = qobject_cast<Akonadi::DateRangeFilterProxyModel*>(sender());
    if (calendarProxy) {
        updateData(calendarProxy, parent);
    }
}

void EventDataContainer::rowsRemoved(const QModelIndex& parent, int, int)
{
    Akonadi::DateRangeFilterProxyModel* calendarProxy = qobject_cast<Akonadi::DateRangeFilterProxyModel*>(sender());
    if (calendarProxy) {
        updateData(calendarProxy, parent);
    }
}

void EventDataContainer::updateData(Akonadi::DateRangeFilterProxyModel* model, const QModelIndex& parent)
{
    for(int row = 0; row < model->rowCount(parent); ++row) {
        QVariantMap eventData;
        eventData["Type"] = model->index(row, Akonadi::CalendarModel::Type).data();
        eventData["StartDate"] = model->index(row, Akonadi::CalendarModel::DateTimeStart).data(Akonadi::CalendarModel::SortRole).toDateTime();
        eventData["EndDate"] = model->index(row, Akonadi::CalendarModel::DateTimeEnd).data(Akonadi::CalendarModel::SortRole).toDateTime();
        eventData["Summary"] = model->index(row, Akonadi::CalendarModel::Summary).data().toString();
        QVariant collection = model->index(row, Akonadi::CalendarModel::Type).data(Akonadi::EntityTreeModel::ParentCollectionRole);
        eventData["Source"] = collection.value<Akonadi::Collection>().name();
        setData(model->index(row, Akonadi::CalendarModel::Uid).data().toString(), eventData);
    }
    checkForUpdate();
}

#include "eventdatacontainer.moc"
