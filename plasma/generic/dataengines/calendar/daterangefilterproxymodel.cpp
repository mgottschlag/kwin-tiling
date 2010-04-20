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

#include "calendarmodel.h"
#include "daterangefilterproxymodel.h"

#include <KDateTime>

#include <QVariant>

using namespace Akonadi;

class DateRangeFilterProxyModel::Private {
  public:
  explicit Private() : mStartColumn( CalendarModel::PrimaryDate ), mEndColumn( CalendarModel::DateTimeEnd ) {}
    int mStartColumn;
    int mEndColumn;
    KDateTime mStart;
    KDateTime mEnd;
};

DateRangeFilterProxyModel::DateRangeFilterProxyModel( QObject* parent ) : QSortFilterProxyModel( parent ), d( new Private )
{
  setFilterRole( CalendarModel::SortRole );
}

DateRangeFilterProxyModel::~DateRangeFilterProxyModel()
{
  delete d;
}

KDateTime DateRangeFilterProxyModel::startDate() const
{
  return d->mStart;
}

void DateRangeFilterProxyModel::setStartDate( const KDateTime &date )
{
  if ( date.isValid() ) {  
    d->mStart = date;
  }
  invalidateFilter();
}

KDateTime DateRangeFilterProxyModel::endDate() const
{
  return d->mEnd;
}

void DateRangeFilterProxyModel::setEndDate( const KDateTime &date )
{
  if ( date.isValid() ) {
    d->mEnd = date.toUtc();
  }

  invalidateFilter();
}
  
int DateRangeFilterProxyModel::startDateColumn() const
{
  return d->mStartColumn;
}

void DateRangeFilterProxyModel::setStartDateColumn( int column )
{
  if ( column == d->mStartColumn )
    return;
  d->mStartColumn = column;
  invalidateFilter();
}

int DateRangeFilterProxyModel::endDateColumn() const
{
  return d->mEndColumn;
}

void DateRangeFilterProxyModel::setEndDateColumn( int column )
{
  if ( column == d->mEndColumn )
    return;
  d->mEndColumn = column;
  invalidateFilter();
}

bool DateRangeFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( d->mEnd.isValid() ) {
    const QModelIndex idx = sourceModel()->index( source_row, d->mStartColumn, source_parent );
    const QVariant v = idx.data( filterRole() );
    const QDateTime start = v.toDateTime();
    if ( start.isValid() && start > d->mEnd.dateTime() ) {
      return false;
    }
  }

  const bool recurs = sourceModel()->index( source_row, 0, source_parent ).data( CalendarModel::RecursRole ).toBool();


  if ( recurs ) {// that's fuzzy and might return events not actually recurring in the range
    return true;
  }

  if ( d->mStart.isValid() ) {
    const QModelIndex idx = sourceModel()->index( source_row, d->mEndColumn, source_parent );
    const QVariant v = idx.data( filterRole() );
    const QDateTime end = v.toDateTime();
    if ( end.isValid() && end < d->mStart.dateTime() ) {
      return false;
    }
  }

    
  return true;
}
