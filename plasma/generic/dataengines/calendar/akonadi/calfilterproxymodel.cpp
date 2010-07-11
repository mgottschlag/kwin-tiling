/*
    Copyright (c) 2009 KDAB
    Author: Frank Osterfeld <osterfeld@kde.org>

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

#include "calfilterproxymodel.h"
#include "calendarmodel.h"

#include <akonadi/entitytreemodel.h>
#include <Akonadi/Item>

#include <KCal/CalFilter>
#include <KCal/Incidence>

using namespace Akonadi;
using namespace KCal;

class CalFilterProxyModel::Private {
  public:
    explicit Private() : filter( 0 ) {}
    CalFilter* filter;
};

CalFilterProxyModel::CalFilterProxyModel( QObject* parent ) : QSortFilterProxyModel( parent ), d( new Private ) {
  setSortRole( CalendarModel::SortRole );
  setFilterKeyColumn( 0 );
}

CalFilterProxyModel::~CalFilterProxyModel() {
  delete d;
}

CalFilter* CalFilterProxyModel::filter() const {
  return d->filter;
}

void CalFilterProxyModel::setFilter( CalFilter* filter ) {
  if ( filter == d->filter )
    return;
  d->filter = filter;
  invalidateFilter();
}


bool CalFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex& source_parent ) const {
  if ( !d->filter )
    return true;
  const QModelIndex idx = sourceModel()->index( source_row, 0, source_parent );
  if ( !idx.isValid() )
    return false;
  const Item item = idx.data( EntityTreeModel::ItemRole ).value<Item>();
  if ( !item.isValid() || !item.hasPayload<Incidence::Ptr>() )
    return false;
  const Incidence::Ptr inc = item.payload<Incidence::Ptr>();
  if ( !inc )
    return false;
  return d->filter->filterIncidence( inc.get() );
}
