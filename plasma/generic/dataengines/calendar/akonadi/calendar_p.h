/*
  Copyright (c) 2009 KDAB
  Authors: Sebastian Sauer <sebsauer@kdab.net>
           Till Adam <till@kdab.net>
           Frank Osterfeld <frank@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef CALENDARSUPPORT_CALENDAR_P_H
#define CALENDARSUPPORT_CALENDAR_P_H

#include "calendar.h"
#include "calfilterproxymodel.h"

#include <Akonadi/Collection>

#include <KCalCore/CalFilter>
#include <KCalCore/ICalTimeZones>

#include <QObject>

namespace CalendarSupport {

class CalendarCollection : public QObject
{
  Q_OBJECT
  public:
    Calendar *m_calendar;
    Akonadi::Collection m_collection;

    CalendarCollection( Calendar *calendar, const Akonadi::Collection &collection )
      : QObject(), m_calendar(calendar), m_collection(collection)
    {
    }

    ~CalendarCollection()
    {
    }
};

struct UnseenItem
{
  Akonadi::Entity::Id collection;
  QString uid;

  bool operator<( const UnseenItem &other ) const
  {
    if ( collection != other.collection ) {
      return collection < other.collection;
    }
    return uid < other.uid;
  }
};

class Calendar::Private : public QObject
{
  Q_OBJECT
  private:
    void removeItemFromMaps( const Akonadi::Item &item );
    Calendar *const q;

  public:
    explicit Private( QAbstractItemModel *treeModel, QAbstractItemModel *model, Calendar *q );
    ~Private();

    enum UpdateMode {
      DontCare,
      AssertExists,
      AssertNew
    };

    void updateItem( const Akonadi::Item &item, UpdateMode mode );
    void itemChanged( const Akonadi::Item &item );

    void assertInvariants() const;
    void appendVirtualItems( Akonadi::Item::List &itemList );
    //CalendarBase begin

    KDateTime::Spec timeZoneIdSpec( const QString &timeZoneId, bool view );
    QString mProductId;
    KCalCore::Person mOwner;
    KCalCore::ICalTimeZones *mTimeZones; // collection of time zones used in this calendar
    KCalCore::ICalTimeZone mBuiltInTimeZone;   // cached time zone lookup
    KCalCore::ICalTimeZone mBuiltInViewTimeZone;   // cached viewing time zone lookup
    KDateTime::Spec mTimeSpec;
    mutable KDateTime::Spec mViewTimeSpec;
    bool mModified;
    bool mNewObserver;
    bool mObserversEnabled;
    QList<CalendarObserver*> mObservers;

    KCalCore::CalFilter *mDefaultFilter;
    //CalendarBase end

    QAbstractItemModel *m_treeModel;
    QAbstractItemModel *m_model;
    CalFilterProxyModel *m_filterProxy;
    QHash<Akonadi::Item::Id, Akonadi::Item> m_itemMap; // akonadi id to items
    QHash<Akonadi::Entity::Id, Akonadi::Collection> m_collectionMap; // akonadi id to collections

    // child to parent map, for already cached parents
    QHash<Akonadi::Item::Id, Akonadi::Item::Id> m_childToParent;

    //parent to children map for alread cached children
    QHash<Akonadi::Item::Id, QList<Akonadi::Item::Id> > m_parentToChildren;

    QMap<UnseenItem, Akonadi::Item::Id> m_uidToItemId;

    // child to parent map, unknown/not cached parent items
    QHash<Akonadi::Item::Id, UnseenItem> m_childToUnseenParent;

    QMap<UnseenItem, QList<Akonadi::Item::Id> > m_unseenParentToChildren;

    // on start dates/due dates of non-recurring, single-day Incidences
    QMultiHash<QString, Akonadi::Item::Id> m_itemIdsForDate;

    QHash<Akonadi::Item::Id, QString> m_itemDateForItemId;

    // From search folders.
    QHash<Akonadi::Item::Id, QList<Akonadi::Item> > m_virtualItems;

    void clear();
    void readFromModel();

  public Q_SLOTS:
    void itemsAdded( const Akonadi::Item::List &items );
    void itemsRemoved( const Akonadi::Item::List &items );

    void collectionsAdded( const Akonadi::Collection::List &collections );
    void collectionsRemoved( const Akonadi::Collection::List &collections );

    void rowsInserted( const QModelIndex &index, int start, int end );
    void rowsAboutToBeRemoved( const QModelIndex &index, int start, int end );
    void rowsInsertedInTreeModel( const QModelIndex &index, int start, int end );
    void rowsAboutToBeRemovedInTreeModel( const QModelIndex &index, int start, int end );
    void dataChangedInTreeModel( const QModelIndex &topLeft, const QModelIndex &bottomRight );

    void layoutChanged();
    void modelReset();
    void dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );

    void onRowsMovedInTreeModel( const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                                 const QModelIndex &destinationParent, int destinationRow );
};

}

#endif
