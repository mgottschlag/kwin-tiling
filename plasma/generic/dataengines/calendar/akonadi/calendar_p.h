/*
    This file is part of Akonadi.

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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef AKONADI_KCAL_CALENDAR_P_H
#define AKONADI_KCAL_CALENDAR_P_H

#include "calendar.h"
#include "calfilterproxymodel.h"
#include "utils.h"

#include <QObject>
#include <QCoreApplication>
#include <QDBusInterface>
#include <QtCore/QMap>

#include <akonadi/entity.h>
#include <akonadi/collection.h>
#include <akonadi/collectionview.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectiondialog.h>
#include <akonadi/item.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/agentinstance.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agenttype.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/monitor.h>
#include <akonadi/session.h>

#include <KCal/Incidence>
#include <kcal/calfilter.h>
#include <kcal/icaltimezones.h>

#include <KLocalizedString>

using namespace boost;
using namespace Akonadi;

class CalendarCollection : public QObject
{
    Q_OBJECT
  public:
    Calendar *m_calendar;
    Akonadi::Collection m_collection;

    CalendarCollection(Calendar *calendar, const Akonadi::Collection &collection)
      : QObject()
      , m_calendar(calendar)
      , m_collection(collection)
    {
    }

    ~CalendarCollection()
    {
    }
};

namespace Akonadi {
  struct UnseenItem {
    Akonadi::Entity::Id collection;
    QString uid;

    bool operator<( const UnseenItem &other ) const {
      if ( collection != other.collection )
        return collection < other.collection;
      return uid < other.uid;
    }
  };
}

class Akonadi::Calendar::Private : public QObject
{
  Q_OBJECT
private:
  void removeItemFromMaps( const Akonadi::Item &item );
  Calendar* const q;

public:
  explicit Private( QAbstractItemModel* treeModel, QAbstractItemModel* model, Calendar *q );
  ~Private();

  enum UpdateMode {
    DontCare,
    AssertExists,
    AssertNew
  };

  void updateItem( const Akonadi::Item &item, UpdateMode mode );
  void itemChanged( const Akonadi::Item& item );

  void assertInvariants() const;

  //CalendarBase begin

  KDateTime::Spec timeZoneIdSpec( const QString &timeZoneId, bool view );
  QString mProductId;
  KCal::Person mOwner;
  KCal::ICalTimeZones *mTimeZones; // collection of time zones used in this calendar
  KCal::ICalTimeZone mBuiltInTimeZone;   // cached time zone lookup
  KCal::ICalTimeZone mBuiltInViewTimeZone;   // cached viewing time zone lookup
  KDateTime::Spec mTimeSpec;
  mutable KDateTime::Spec mViewTimeSpec;
  bool mModified;
  bool mNewObserver;
  bool mObserversEnabled;
  QList<CalendarObserver*> mObservers;

  KCal::CalFilter *mDefaultFilter;
  //CalendarBase end

  QAbstractItemModel *m_treeModel;
  QAbstractItemModel *m_model;
  Akonadi::CalFilterProxyModel* m_filterProxy;
  QHash<Akonadi::Item::Id, Akonadi::Item> m_itemMap; // akonadi id to items
  QHash<Akonadi::Entity::Id, Akonadi::Collection> m_collectionMap; // akonadi id to collections

  QHash<Akonadi::Item::Id, Akonadi::Item::Id> m_childToParent; // child to parent map, for already cached parents
  QHash<Akonadi::Item::Id, QList<Akonadi::Item::Id> > m_parentToChildren; //parent to children map, for alread cached children
  QMap<UnseenItem, Akonadi::Item::Id> m_uidToItemId;

  QHash<Akonadi::Item::Id, UnseenItem> m_childToUnseenParent; // child to parent map, unknown/not cached parent items
  QMap<UnseenItem, QList<Akonadi::Item::Id> > m_unseenParentToChildren;

  QMultiHash<QString, Akonadi::Item::Id> m_itemIdsForDate; // on start dates/due dates of non-recurring, single-day Incidences
  QHash<Akonadi::Item::Id, QString> m_itemDateForItemId;

  void clear();
  void readFromModel();

public Q_SLOTS:
  void itemsAdded( const Akonadi::Item::List &items );
  void itemsRemoved( const Akonadi::Item::List &items );

  void collectionsAdded( const Akonadi::Collection::List &collections );
  void collectionsRemoved( const Akonadi::Collection::List &collections );

  void rowsInserted( const QModelIndex& index, int start, int end );
  void rowsAboutToBeRemoved( const QModelIndex& index, int start, int end );
  void rowsInsertedInTreeModel( const QModelIndex& index, int start, int end );
  void rowsAboutToBeRemovedInTreeModel( const QModelIndex& index, int start, int end );
  void dataChangedInTreeModel( const QModelIndex& topLeft, const QModelIndex& bottomRight );

  void layoutChanged();
  void modelReset();
  void dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight );
};

#endif
