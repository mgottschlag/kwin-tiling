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
#ifndef AKONADI_KCAL_DATERANGEFILTERPROXYMODEL_H
#define AKONADI_KCAL_DATERANGEFILTERPROXYMODEL_H

#include <QtGui/QSortFilterProxyModel>

class KDateTime;
namespace Akonadi {
  class DateRangeFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
  public:
    explicit DateRangeFilterProxyModel( QObject* parent=0 );
    ~DateRangeFilterProxyModel();
    
    KDateTime startDate() const;
    void setStartDate( const KDateTime& date );

    KDateTime endDate() const;
    void setEndDate( const KDateTime& date );
    
    int startDateColumn() const;
    void setStartDateColumn( int column );

    int endDateColumn() const;
    void setEndDateColumn( int column );

  protected:
    /* reimp */ bool filterAcceptsRow( int source_row, const QModelIndex& source_parent ) const;
    
  private:
    class Private;
    Private* const d;
  };
}

#endif // AKONADI_KCAL_DATERANGEFILTERPROXYMODEL_H
