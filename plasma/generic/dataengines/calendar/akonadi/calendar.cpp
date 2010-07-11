/*
    This file is part of Akonadi.

    Copyright (c) 2009 KDAB
    Author: Sebastian Sauer <sebsauer@kdab.net>
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

#include "calendar.h"
#include "calendar_p.h"
#include <akonadi/agentbase.h>
#include <kcal/incidence.h>
#include <kcal/event.h>
#include <kcal/todo.h>
#include <kcal/journal.h>
#include <kcal/filestorage.h>

#include <QtCore/QDate>
#include <QtCore/QHash>
#include <QtCore/QMultiHash>
#include <QtCore/QString>
#include <QAbstractItemModel>

#include <KDebug>
#include <KDateTime>
#include <KLocale>
#include <KMessageBox>

#include <Akonadi/Collection>
#include <Akonadi/EntityTreeModel>

using namespace Akonadi;

Calendar::Private::Private( QAbstractItemModel* treeModel, QAbstractItemModel *model, Calendar *qq )
  : q( qq ),
    mTimeZones( new KCal::ICalTimeZones ),
    mNewObserver( false ),
    mObserversEnabled( true ),
    mDefaultFilter( new KCal::CalFilter ),
    m_treeModel( treeModel ),
    m_model( model )
{
  // Setup default filter, which does nothing
  mDefaultFilter->setEnabled( false );
  m_filterProxy = new CalFilterProxyModel( q );
  m_filterProxy->setFilter( mDefaultFilter );
  m_filterProxy->setSourceModel( model );

  // user information...
  mOwner.setName( i18n( "Unknown Name" ) );
  mOwner.setEmail( i18n( "unknown@nowhere" ) );

  connect( m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)) );
  connect( m_model, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()) );
  connect( m_model, SIGNAL(modelReset()), this, SLOT(modelReset()) );
  connect( m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)) );
  connect( m_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)) );

  // use the unfiltered model to catch collections
  connect( m_treeModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInsertedInTreeModel(QModelIndex,int,int)) );
  connect( m_treeModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(rowsAboutToBeRemovedInTreeModel(QModelIndex,int,int)) );
  connect( m_treeModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChangedInTreeModel(QModelIndex,QModelIndex)) );
  /*
  connect( m_monitor, SIGNAL(itemLinked(const Akonadi::Item,Akonadi::Collection)),
           this, SLOT(itemAdded(const Akonadi::Item,Akonadi::Collection)) );
  connect( m_monitor, SIGNAL(itemUnlinked(Akonadi::Item,Akonadi::Collection )),
           this, SLOT(itemRemoved(Akonadi::Item,Akonadi::Collection )) );
  */
}

void Calendar::Private::rowsInsertedInTreeModel( const QModelIndex &parent, int start, int end )
{
  collectionsAdded( collectionsFromModel( m_treeModel, parent, start, end ) );
}

void Calendar::Private::rowsAboutToBeRemovedInTreeModel( const QModelIndex &parent, int start, int end )
{
  collectionsRemoved( collectionsFromModel( m_treeModel, parent, start, end ) );
}


void Calendar::Private::rowsInserted( const QModelIndex &parent, int start, int end )
{
  itemsAdded( itemsFromModel( m_model, parent, start, end ) );
}

void Calendar::Private::rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
{
  itemsRemoved( itemsFromModel( m_model, parent, start, end ) );
}

void Calendar::Private::layoutChanged()
{
  kDebug();
}

void Calendar::Private::modelReset()
{
  kDebug();
  clear();
  readFromModel();
}

void Calendar::Private::clear()
{
  itemsRemoved( m_itemMap.values() );
  Q_ASSERT( m_itemMap.isEmpty() );
  m_childToParent.clear();
  m_parentToChildren.clear();
  m_childToUnseenParent.clear();
  m_unseenParentToChildren.clear();
  m_itemIdsForDate.clear();
}

void Calendar::Private::readFromModel()
{
  itemsAdded( itemsFromModel( m_model ) );
}

void Calendar::Private::dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
  kDebug();
  Q_ASSERT( topLeft.row() <= bottomRight.row() );
  const int endRow = bottomRight.row();
  QModelIndex i( topLeft );
  int row = i.row();
  while ( row <= endRow ) {
    const Item item = itemFromIndex( i );
    if ( item.isValid() )
      updateItem( item, AssertExists );
    ++row;
    i = i.sibling( row, topLeft.column() );
  }
  emit q->calendarChanged();
}

void Calendar::Private::dataChangedInTreeModel( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
  Q_ASSERT( topLeft.row() <= bottomRight.row() );
  const int endRow = bottomRight.row();
  QModelIndex i( topLeft );
  int row = i.row();
  while ( row <= endRow ) {
    const Collection col = collectionFromIndex( i );
    if ( col.isValid() ) {
      emit q->calendarChanged();
      return;
    }
    ++row;
    i = i.sibling( row, topLeft.column() );
  }
}

Calendar::Private::~Private()
{
  Q_FOREACH ( const Item &item, m_itemMap ) {
    Akonadi::incidence( item )->unRegisterObserver( q );
  }

  delete mTimeZones;
  delete mDefaultFilter;
}


void Calendar::Private::assertInvariants() const
{
}

void Calendar::Private::updateItem( const Item &item, UpdateMode mode )
{
  assertInvariants();
  const bool alreadyExisted = m_itemMap.contains( item.id() );
  const Item::Id id = item.id();

  kDebug()<<"id="<<item.id()<<"version="<<item.revision()<<"alreadyExisted="
          << alreadyExisted << "; calendar = " << q;
  Q_ASSERT( mode == DontCare || alreadyExisted == ( mode == AssertExists ) );

  const KCal::Incidence::Ptr incidence = Akonadi::incidence( item );
  Q_ASSERT( incidence );

  if ( alreadyExisted ) {

    if ( !m_itemMap.contains( id ) ) {
      // Item was deleted almost at the same time the change was made
      // ignore this change
      return;
    }

    if ( item.storageCollectionId() == -1 ) {
      // A valid item can have an invalid storage id if it was deleted while
      // fetching the ancestor
      return;
    }

    if ( item.storageCollectionId() != m_itemMap.value( id ).storageCollectionId() ) {
      // there was once a bug that resulted in items forget their collectionId...
      kDebug() << "item.storageCollectionId() = " << item.storageCollectionId()
               << "; m_itemMap.value( id ).storageCollectionId() = "
               << m_itemMap.value( id ).storageCollectionId()
               << "; item.isValid() = " << item.isValid()
               << "; calendar = " << q;
      Q_ASSERT_X( false, "updateItem", "updated item has different collection id" );
    }
    // update-only goes here
  } else {
    // new-only goes here
    const Collection::Rights rights = item.parentCollection().rights();
    if ( !( rights & Collection::CanDeleteItem ) &&
         !( rights & Collection::CanChangeItem ) &&
         !incidence->isReadOnly() ) {
      incidence->setReadOnly( true );
    }
  }

  if ( alreadyExisted && m_itemDateForItemId.contains( item.id() )) {
    // for changed items, we must remove existing date entries (they might have changed)
    m_itemIdsForDate.remove( m_itemDateForItemId[item.id()], item.id() );
    m_itemDateForItemId.remove( item.id() );
  }

  QString date;
  if ( const KCal::Todo::Ptr t = Akonadi::todo( item ) ) {
    if ( t->hasDueDate() ) {
      date = t->dtDue().date().toString();
    }
  } else if ( const KCal::Event::Ptr e = Akonadi::event( item ) ) {
    if ( !e->recurs() && !e->isMultiDay() ) {
      date = e->dtStart().date().toString();
    }
  } else if ( const KCal::Journal::Ptr j = Akonadi::journal( item ) ) {
    date = j->dtStart().date().toString();
  }  else {
    Q_ASSERT( false );
    return;
  }

  if ( !m_itemIdsForDate.contains( date, item.id() ) && !date.isEmpty() ) {
    m_itemIdsForDate.insert( date, item.id() );
    m_itemDateForItemId.insert( item.id(), date );
  }

  m_itemMap.insert( id, item );

  UnseenItem ui;
  ui.collection = item.storageCollectionId();
  ui.uid = incidence->uid();

  //REVIEW(AKONADI_PORT)
  //UIDs might be duplicated and thus not unique, so for now we assume that the relatedTo
  // UID refers to an item in the same collection.
  //this might break with virtual collections, so we might fall back to a global UID
  //to akonadi item mapping, and pick just any item (or the first found, or whatever strategy makes sense)
  //from the ones with the same UID
  const QString parentUID = incidence->relatedToUid();
  const bool hasParent = !parentUID.isEmpty();
  UnseenItem parentItem;
  QMap<UnseenItem,Item::Id>::const_iterator parentIt = m_uidToItemId.constEnd();
  bool knowParent = false;
  bool parentNotChanged = false;
  if ( hasParent ) {
    parentItem.collection = item.storageCollectionId();
    parentItem.uid = parentUID;
    parentIt = m_uidToItemId.constFind( parentItem );
    knowParent = parentIt != m_uidToItemId.constEnd();
  }

  if ( alreadyExisted ) {
    if ( m_uidToItemId.value( ui ) != item.id() ) {
      kDebug()<< "item.id() = " << item.id() << "; cached id = " << m_uidToItemId.value( ui )
              << "item uid = "  << ui.uid
              << "; calendar = " << q;
    }

    Q_ASSERT( m_uidToItemId.value( ui ) == item.id() );
    QHash<Item::Id,Item::Id>::Iterator oldParentIt = m_childToParent.find( id );
    if ( oldParentIt != m_childToParent.end() ) {
      const KCal::Incidence::Ptr parentInc = Akonadi::incidence( m_itemMap.value( oldParentIt.value() ) );
      Q_ASSERT( parentInc );
      if ( parentInc->uid() != parentUID ) {
        //parent changed, remove old entries
        Akonadi::incidence( item )->setRelatedTo( 0 );
        QList<Item::Id>& l = m_parentToChildren[oldParentIt.value()];
        l.removeAll( id );
        m_childToParent.remove( id );
      } else {
        parentNotChanged = true;

        // incidences come from akonadi without the relatedTo() pointer set
        // so we have to re-set it after an update
        if ( !incidence->relatedTo() ) {
          incidence->setRelatedTo( parentInc.get() );
        }
      }
    } else { //old parent not seen, maybe unseen?
      QHash<Item::Id,UnseenItem>::Iterator oldUnseenParentIt = m_childToUnseenParent.find( id );
      if ( oldUnseenParentIt != m_childToUnseenParent.end() ) {
        if ( oldUnseenParentIt.value().uid != parentUID ) {
          //parent changed, remove old entries
          QList<Item::Id>& l = m_unseenParentToChildren[oldUnseenParentIt.value()];
          l.removeAll( id );
          m_childToUnseenParent.remove( id );
        } else {
          parentNotChanged = true;
        }
      }
    }

  } else {
    m_uidToItemId.insert( ui, item.id() );

    //check for already known children:
    const QList<Item::Id> orphanedChildren = m_unseenParentToChildren.value( ui );
    if ( !orphanedChildren.isEmpty() ) {
      m_parentToChildren.insert( id, orphanedChildren );
    }

    Q_FOREACH ( const Item::Id &cid, orphanedChildren ) {
      m_childToParent.insert( cid, id );
      Akonadi::incidence( m_itemMap[cid] )->setRelatedTo( incidence.get() );
    }

    m_unseenParentToChildren.remove( ui );
    m_childToUnseenParent.remove( id );
  }

  if ( hasParent && !parentNotChanged ) {
    if ( knowParent ) {
      Q_ASSERT( !m_parentToChildren.value( parentIt.value() ).contains( id ) );
      const KCal::Incidence::Ptr parentInc = Akonadi::incidence( m_itemMap.value( parentIt.value() ) );
      Q_ASSERT( parentInc );
      Akonadi::incidence( item )->setRelatedTo( parentInc.get() );
      m_parentToChildren[parentIt.value()].append( id );
      m_childToParent.insert( id, parentIt.value() );
    } else {
      m_childToUnseenParent.insert( id, parentItem );
      m_unseenParentToChildren[parentItem].append( id );
    }
  }

  if ( !alreadyExisted ) {
    incidence->registerObserver( q );
    q->notifyIncidenceAdded( item );
  } else {

    // The raw incidence's address changed, so we have to update all children
    Q_FOREACH ( const Item::Id &child_id, m_parentToChildren[item.id()] ) {
      Akonadi::incidence( m_itemMap[child_id] )->setRelatedTo( incidence.get() );
    }

    q->notifyIncidenceChanged( item );
  }
  assertInvariants();
}

void Calendar::Private::itemChanged( const Item& item )
{
  kDebug() << "item changed: " << item.id();
  assertInvariants();
  Q_ASSERT( item.isValid() );
  const KCal::Incidence::ConstPtr incidence = Akonadi::incidence( item );
  if ( !incidence )
    return;
  updateItem( item, AssertExists );
  emit q->calendarChanged();
  assertInvariants();
}

void Calendar::Private::itemsAdded( const Item::List &items )
{
  kDebug() << "adding items: " << items.count();
  assertInvariants();
  foreach( const Item &item, items ) {
    Q_ASSERT( item.isValid() );
    if ( !Akonadi::hasIncidence( item ) ) {
      continue;
    }
    updateItem( item, AssertNew );
    const KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
  }
  emit q->calendarChanged();
  assertInvariants();
}

void Calendar::Private::collectionsAdded( const Collection::List &collections )
{
  kDebug() << "adding collections: " << collections.count();
 
  foreach( const Collection &collection, collections ) {
    m_collectionMap[collection.id()] = collection;
  }
}

void Calendar::Private::collectionsRemoved( const Collection::List &collections )
{
  kDebug() << "removing collections: " << collections.count();
  foreach( const Collection &collection, collections ) {
    m_collectionMap.remove( collection.id() );
  }
}

void Calendar::Private::removeItemFromMaps( const Akonadi::Item &item )
{
  UnseenItem unseen_item;
  UnseenItem unseen_parent;

  unseen_item.collection = unseen_parent.collection = item.storageCollectionId();

  unseen_item.uid   = Akonadi::incidence( item )->uid();
  unseen_parent.uid = Akonadi::incidence( item )->relatedToUid();

  if ( m_childToParent.contains( item.id() ) ) {
    Akonadi::Item::Id parentId = m_childToParent.take( item.id() );
    m_parentToChildren[parentId].removeAll( item.id() );
  }

  foreach ( const Akonadi::Item::Id &id, m_parentToChildren[item.id()] ) {
    m_childToUnseenParent[id] = unseen_item;
    m_unseenParentToChildren[unseen_item].push_back( id );
  }

  m_parentToChildren.remove( item.id() );

  m_childToUnseenParent.remove( item.id() );

  m_unseenParentToChildren[unseen_parent].removeAll( item.id() );

  m_uidToItemId.remove( unseen_item );
}

void Calendar::Private::itemsRemoved( const Item::List &items )
{
  assertInvariants();
  foreach( const Item& item, items ) {
    Q_ASSERT( item.isValid() );
    Item ci( m_itemMap.take( item.id() ) );

    removeItemFromMaps( ci );

    kDebug()<<item.id();
    Q_ASSERT( ci.hasPayload<KCal::Incidence::Ptr>() );
    const KCal::Incidence::Ptr incidence = ci.payload<KCal::Incidence::Ptr>();
    kDebug() << "Remove uid=" << incidence->uid() << "summary=" << incidence->summary()
             << "type=" << incidence->type() << "; id= " << item.id() << "; revision=" << item.revision()
             << " calendar = " << q;

    if ( const KCal::Event::Ptr e = dynamic_pointer_cast<KCal::Event>( incidence ) ) {
      if ( !e->recurs() ) {
        m_itemIdsForDate.remove( e->dtStart().date().toString(), item.id() );
      }
    } else if ( const KCal::Todo::Ptr t = dynamic_pointer_cast<KCal::Todo>( incidence ) ) {
      if ( t->hasDueDate() ) {
        m_itemIdsForDate.remove( t->dtDue().date().toString(), item.id() );
      }
    } else if ( const KCal::Journal::Ptr j = dynamic_pointer_cast<KCal::Journal>( incidence ) ) {
      m_itemIdsForDate.remove( j->dtStart().date().toString(), item.id() );
    } else {
      Q_ASSERT( false );
      continue;
    }

    incidence->unRegisterObserver( q );
    q->notifyIncidenceDeleted( item );
  }
  emit q->calendarChanged();
  assertInvariants();
}

Calendar::Calendar( QAbstractItemModel* treeModel, QAbstractItemModel *model, const KDateTime::Spec &timeSpec, QObject *parent )
  : QObject( parent )
  , d( new Private( treeModel, model, this ) )
{
  d->mTimeSpec = timeSpec;
  d->mViewTimeSpec = timeSpec;
  d->readFromModel();
}

Calendar::~Calendar()
{
  delete d;
}

QAbstractItemModel* Calendar::treeModel() const
{
  return d->m_treeModel;
}

QAbstractItemModel* Calendar::model() const
{
  return d->m_filterProxy;
}

QAbstractItemModel* Calendar::unfilteredModel() const
{
  return d->m_model;
}

void Calendar::setUnfilteredModel( QAbstractItemModel *model )
{

  if ( d->m_model == model ) {
    return;
  }

  if ( d->m_model ) {
    disconnect( d->m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), d, SLOT(dataChanged(QModelIndex,QModelIndex)) );
    disconnect( d->m_model, SIGNAL(layoutChanged()), d, SLOT(layoutChanged()) );
    disconnect( d->m_model, SIGNAL(modelReset()), d, SLOT(modelReset()) );
    disconnect( d->m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), d, SLOT(rowsInserted(QModelIndex,int,int)) );
    disconnect( d->m_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), d, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)) );
  }
  d->m_model = model;
  d->m_filterProxy->setSourceModel( model );
  if ( model ) {
    connect( d->m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), d, SLOT(dataChanged(QModelIndex,QModelIndex)) );
    connect( d->m_model, SIGNAL(layoutChanged()), d, SLOT(layoutChanged()) );
    connect( d->m_model, SIGNAL(modelReset()), d, SLOT(modelReset()) );
    connect( d->m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), d, SLOT(rowsInserted(QModelIndex,int,int)) );
    connect( d->m_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), d, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)) );
    d->modelReset();
  }
}

// This method will be called probably multiple times if a series of changes where done. One finished the endChange() method got called.
void Calendar::incidenceUpdated( KCal::IncidenceBase *incidence )
{
  incidence->setLastModified( KDateTime::currentUtcDateTime() );
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.

  // The static_cast is ok as the CalendarLocal only observes Incidence objects
#ifdef AKONADI_PORT_DISABLED
  notifyIncidenceChanged( static_cast<KCal::Incidence *>( incidence ) );
#else
  kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
}

Item Calendar::event( const Item::Id &id ) const
{
  const Item item = d->m_itemMap.value( id );
  if ( Akonadi::event( item ) )
    return item;
  else
    return Item();
}

Item Calendar::todo( const Item::Id &id ) const
{
  const Item item = d->m_itemMap.value( id );
  if ( Akonadi::todo( item ) )
    return item;
  else
    return Item();
}

Item::List Calendar::rawTodos( TodoSortField sortField, SortDirection sortDirection )
{
  kDebug()<<sortField<<sortDirection;
  Item::List todoList;
  QHashIterator<Item::Id, Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if ( Akonadi::todo( i.value() ) ) {
      todoList.append( i.value() );
    }
  }
  return sortTodos( todoList, sortField, sortDirection );
}



Item::List Calendar::rawTodosForDate( const QDate &date )
{
  kDebug()<<date.toString();
  Item::List todoList;
  QString dateStr = date.toString();
  QMultiHash<QString, Item::Id>::const_iterator it = d->m_itemIdsForDate.constFind( dateStr );
  while ( it != d->m_itemIdsForDate.constEnd() && it.key() == dateStr ) {
    if ( Akonadi::todo( d->m_itemMap[it.value()] ) ) {
      todoList.append( d->m_itemMap[it.value()] );
    }
    ++it;
  }
  return todoList;
}

KCal::Alarm::List Calendar::alarmsTo( const KDateTime &to )
{
  kDebug();
  return alarms( KDateTime( QDate( 1900, 1, 1 ) ), to );
}

KCal::Alarm::List Calendar::alarms( const KDateTime &from, const KDateTime &to )
{
  kDebug() << "Alarms:" << d->m_itemMap.count();
  KCal::Alarm::List alarmList;
  QHashIterator<Item::Id, Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    const Item item = i.next().value();
    KCal::Incidence::Ptr incidence = Akonadi::incidence( item );
    if ( !incidence ) {
      continue;
    }

    if ( incidence->recurs() ) {
      appendRecurringAlarms( alarmList, item, from, to );
    } else {
      appendAlarms( alarmList, item, from, to );
    }
  }
  return alarmList;
}

Item::List Calendar::rawEventsForDate( const QDate &date,
                                       const KDateTime::Spec &timespec,
                                       EventSortField sortField,
                                       SortDirection sortDirection )
{
  kDebug()<<date.toString();
  Item::List eventList;
  // Find the hash for the specified date
  QString dateStr = date.toString();
  // Iterate over all non-recurring, single-day events that start on this date
  QMultiHash<QString, Item::Id>::const_iterator it = d->m_itemIdsForDate.constFind( dateStr );
  KDateTime::Spec ts = timespec.isValid() ? timespec : timeSpec();
  KDateTime kdt( date, ts );
  while ( it != d->m_itemIdsForDate.constEnd() && it.key() == dateStr ) {
    if ( KCal::Event::Ptr ev = Akonadi::event( d->m_itemMap[it.value()] ) ) {
      KDateTime end( ev->dtEnd().toTimeSpec( ev->dtStart() ) );
      if ( ev->allDay() ) {
        end.setDateOnly( true );
      } else {
        end = end.addSecs( -1 );
      }
      if ( end >= kdt ) {
        eventList.append( d->m_itemMap[it.value()] );
      }
    }
    ++it;
  }
  // Iterate over all events. Look for recurring events that occur on this date
  QHashIterator<Item::Id, Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if ( KCal::Event::Ptr ev = Akonadi::event( i.value() ) ) {
      if ( ev->recurs() ) {
        if ( ev->isMultiDay() ) {
          int extraDays = ev->dtStart().date().daysTo( ev->dtEnd().date() );
          for ( int j = 0; j <= extraDays; ++j ) {
            if ( ev->recursOn( date.addDays( -j ), ts ) ) {
              eventList.append( i.value() );
              break;
            }
          }
        } else {
          if ( ev->recursOn( date, ts ) ) {
            eventList.append( i.value() );
          }
        }
      } else {
        if ( ev->isMultiDay() ) {
          if ( ev->dtStart().date() <= date && ev->dtEnd().date() >= date )
            eventList.append( i.value() );
        }
      }
    }
  }
  return sortEvents( eventList, sortField, sortDirection );
}

Item::List Calendar::rawEvents( const QDate &start, const QDate &end, const KDateTime::Spec &timespec, bool inclusive )
{
  kDebug()<<start.toString()<<end.toString()<<inclusive;
  Item::List eventList;
  KDateTime::Spec ts = timespec.isValid() ? timespec : timeSpec();
  KDateTime st( start, ts );
  KDateTime nd( end, ts );
  KDateTime yesterStart = st.addDays( -1 );
  // Get non-recurring events
  QHashIterator<Item::Id, Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if ( KCal::Event::Ptr event = Akonadi::event( i.value() ) ) {
      KDateTime rStart = event->dtStart();
      if ( nd < rStart ) continue;
      if ( inclusive && rStart < st ) continue;
      if ( !event->recurs() ) { // non-recurring events
        KDateTime rEnd = event->dtEnd();
        if ( rEnd < st ) continue;
        if ( inclusive && nd < rEnd ) continue;
      } else { // recurring events
        switch( event->recurrence()->duration() ) {
        case -1: // infinite
          if ( inclusive ) continue;
          break;
        case 0: // end date given
        default: // count given
          KDateTime rEnd( event->recurrence()->endDate(), ts );
          if ( !rEnd.isValid() ) continue;
          if ( rEnd < st ) continue;
          if ( inclusive && nd < rEnd ) continue;
          break;
        } // switch(duration)
      } //if (recurs)
      eventList.append( i.value() );
    }
  }
  return eventList;
}

Item::List Calendar::rawEventsForDate( const KDateTime &kdt )
{
  kDebug();
  return rawEventsForDate( kdt.date(), kdt.timeSpec() );
}

Item::List Calendar::rawEvents( EventSortField sortField, SortDirection sortDirection )
{
  kDebug()<<sortField<<sortDirection;
  Item::List eventList;
  QHashIterator<Item::Id, Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if ( Akonadi::event( i.value() ) )
      eventList.append( i.value() );
  }
  return sortEvents( eventList, sortField, sortDirection );
}


Item Calendar::journal( const Item::Id &id ) const
{
  const Item item = d->m_itemMap.value( id );
  if ( Akonadi::journal( item ) ) {
    return item;
  } else {
    return Item();
  }
}

Item::List Calendar::rawJournals( JournalSortField sortField, SortDirection sortDirection )
{
  Item::List journalList;
  QHashIterator<Item::Id, Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if ( Akonadi::journal( i.value() ) ) {
      journalList.append( i.value() );
    }
  }
  return sortJournals( journalList, sortField, sortDirection );
}

Item::List Calendar::rawJournalsForDate( const QDate &date )
{
  Item::List journalList;
  QString dateStr = date.toString();
  QMultiHash<QString, Item::Id>::const_iterator it = d->m_itemIdsForDate.constFind( dateStr );
  while ( it != d->m_itemIdsForDate.constEnd() && it.key() == dateStr ) {
    if ( Akonadi::journal( d->m_itemMap[it.value()] ) ) {
      journalList.append( d->m_itemMap[it.value()] );
    }
    ++it;
  }
  return journalList;
}

Item Calendar::findParent( const Item &child ) const
{
  return d->m_itemMap.value( d->m_childToParent.value( child.id() ) );
}

Item::List Calendar::findChildren( const Item &parent ) const
{
  Item::List l;
  Q_FOREACH( const Item::Id &id, d->m_parentToChildren.value( parent.id() ) ) {
    l.push_back( d->m_itemMap.value( id ) );
  }
  return l;
}

bool Calendar::isChild( const Item &parent, const Item &child ) const
{
  return d->m_childToParent.value( child.id() ) == parent.id();
}

Item::Id Calendar::itemIdForIncidenceUid( const QString &uid ) const
{
  QHashIterator<Item::Id, Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    const Item item = i.value();
    Q_ASSERT( item.isValid());
    Q_ASSERT( item.hasPayload<KCal::Incidence::Ptr>());
    KCal::Incidence::Ptr inc = item.payload<KCal::Incidence::Ptr>();
    if ( inc->uid() == uid )
        return item.id();
  }
  kWarning() << "Failed to find Akonadi::Item for KCal uid " << uid;
  return -1;
}

Item Calendar::itemForIncidenceUid( const QString &uid ) const
{
  return incidence( itemIdForIncidenceUid( uid ) );
}

// calendarbase.cpp


KCal::Person Calendar::owner() const
{
  return d->mOwner;
}

void Calendar::setOwner( const KCal::Person &owner )
{
  d->mOwner = owner;
}

void Calendar::setTimeSpec( const KDateTime::Spec &timeSpec )
{
  d->mTimeSpec = timeSpec;
  d->mBuiltInTimeZone = KCal::ICalTimeZone();
  setViewTimeSpec( timeSpec );

  doSetTimeSpec( d->mTimeSpec );
}

KDateTime::Spec Calendar::timeSpec() const
{
  return d->mTimeSpec;
}

void Calendar::setTimeZoneId( const QString &timeZoneId )
{
  d->mTimeSpec = d->timeZoneIdSpec( timeZoneId, false );
  d->mViewTimeSpec = d->mTimeSpec;
  d->mBuiltInViewTimeZone = d->mBuiltInTimeZone;

  doSetTimeSpec( d->mTimeSpec );
}

//@cond PRIVATE
KDateTime::Spec Calendar::Private::timeZoneIdSpec( const QString &timeZoneId,
                                                   bool view )
{
  if ( view ) {
    mBuiltInViewTimeZone = KCal::ICalTimeZone();
  } else {
    mBuiltInTimeZone = KCal::ICalTimeZone();
  }
  if ( timeZoneId == QLatin1String( "UTC" ) ) {
    return KDateTime::UTC;
  }
  KCal::ICalTimeZone tz = mTimeZones->zone( timeZoneId );
  if ( !tz.isValid() ) {
    KCal::ICalTimeZoneSource tzsrc;
#ifdef AKONADI_PORT_DISABLED
    tz = tzsrc.parse( icaltimezone_get_builtin_timezone( timeZoneId.toLatin1() ) );
#else
    kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
    if ( view ) {
      mBuiltInViewTimeZone = tz;
    } else {
      mBuiltInTimeZone = tz;
    }
  }
  if ( tz.isValid() ) {
    return tz;
  } else {
    return KDateTime::ClockTime;
  }
}
//@endcond

QString Calendar::timeZoneId() const
{
  KTimeZone tz = d->mTimeSpec.timeZone();
  return tz.isValid() ? tz.name() : QString();
}

void Calendar::setViewTimeSpec( const KDateTime::Spec &timeSpec ) const
{
  d->mViewTimeSpec = timeSpec;
  d->mBuiltInViewTimeZone = KCal::ICalTimeZone();
}

void Calendar::setViewTimeZoneId( const QString &timeZoneId ) const
{
  d->mViewTimeSpec = d->timeZoneIdSpec( timeZoneId, true );
}

KDateTime::Spec Calendar::viewTimeSpec() const
{
  return d->mViewTimeSpec;
}

QString Calendar::viewTimeZoneId() const
{
  KTimeZone tz = d->mViewTimeSpec.timeZone();
  return tz.isValid() ? tz.name() : QString();
}

void Calendar::shiftTimes( const KDateTime::Spec &oldSpec,
                           const KDateTime::Spec &newSpec )
{
  setTimeSpec( newSpec );
  int i, end;
  Item::List ev = events();
  for ( i = 0, end = ev.count();  i < end;  ++i ) {
    Akonadi::event( ev[i] )->shiftTimes( oldSpec, newSpec );
  }

  Item::List to = todos();
  for ( i = 0, end = to.count();  i < end;  ++i ) {
    Akonadi::todo( to[i] )->shiftTimes( oldSpec, newSpec );
  }

  Item::List jo = journals();
  for ( i = 0, end = jo.count();  i < end;  ++i ) {
    Akonadi::journal( jo[i] )->shiftTimes( oldSpec, newSpec );
  }
}

void Calendar::setFilter( KCal::CalFilter *filter )
{
  d->m_filterProxy->setFilter( filter ? filter : d->mDefaultFilter );
}

KCal::CalFilter *Calendar::filter()
{
  return d->m_filterProxy->filter();
}

QStringList Calendar::categories( Calendar* cal )
{
  Item::List rawInc( cal->rawIncidences() );
  QStringList cats, thisCats;
  // @TODO: For now just iterate over all incidences. In the future,
  // the list of categories should be built when reading the file.
  Q_FOREACH( const Item &i, rawInc ) {
    thisCats = Akonadi::incidence( i )->categories();
    for ( QStringList::ConstIterator si = thisCats.constBegin();
          si != thisCats.constEnd(); ++si ) {
      if ( !cats.contains( *si ) ) {
        cats.append( *si );
      }
    }
  }
  return cats;
}

Item::List Calendar::incidences( const QDate &date )
{
  return mergeIncidenceList( events( date ), todos( date ), journals( date ) );
}

Item::List Calendar::incidences()
{
  return itemsFromModel( d->m_filterProxy );
}

Item::List Calendar::rawIncidences()
{
  return itemsFromModel( d->m_model );
}

Item::List Calendar::sortEvents( const Item::List &eventList_,
                                  EventSortField sortField,
                                  SortDirection sortDirection )
{
  Item::List eventList = eventList_;
  Item::List eventListSorted;
  Item::List tempList, t;
  Item::List alphaList;
  Item::List::Iterator sortIt;
  Item::List::Iterator eit;

  // Notice we alphabetically presort Summaries first.
  // We do this so comparison "ties" stay in a nice order.

  switch( sortField ) {
  case EventSortUnsorted:
    eventListSorted = eventList;
    break;

  case EventSortStartDate:
    alphaList = sortEvents( eventList, EventSortSummary, sortDirection );
    for ( eit = alphaList.begin(); eit != alphaList.end(); ++eit) {
      KCal::Event::Ptr e = Akonadi::event( *eit );
      Q_ASSERT( e );
      if ( e->dtStart().isDateOnly() ) {
        tempList.append( *eit );
        continue;
      }
      sortIt = eventListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != eventListSorted.end() &&
                e->dtStart() >= Akonadi::event(*sortIt)->dtStart() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != eventListSorted.end() &&
                e->dtStart() < Akonadi::event(*sortIt)->dtStart() ) {
          ++sortIt;
        }
      }
      eventListSorted.insert( sortIt, *eit );
    }
    if ( sortDirection == SortDirectionAscending ) {
      // Prepend the list of Events without End DateTimes
      tempList += eventListSorted;
      eventListSorted = tempList;
    } else {
      // Append the list of Events without End DateTimes
      eventListSorted += tempList;
    }
    break;

  case EventSortEndDate:
    alphaList = sortEvents( eventList, EventSortSummary, sortDirection );
    for ( eit = alphaList.begin(); eit != alphaList.end(); ++eit ) {
      KCal::Event::Ptr e = Akonadi::event( *eit );
      Q_ASSERT( e );
      if ( e->hasEndDate() ) {
        sortIt = eventListSorted.begin();
        if ( sortDirection == SortDirectionAscending ) {
          while ( sortIt != eventListSorted.end() &&
                  e->dtEnd() >= Akonadi::event(*sortIt)->dtEnd() ) {
            ++sortIt;
          }
        } else {
          while ( sortIt != eventListSorted.end() &&
                  e->dtEnd() < Akonadi::event(*sortIt)->dtEnd() ) {
            ++sortIt;
          }
        }
      } else {
        // Keep a list of the Events without End DateTimes
        tempList.append( *eit );
      }
      eventListSorted.insert( sortIt, *eit );
    }
    if ( sortDirection == SortDirectionAscending ) {
      // Append the list of Events without End DateTimes
      eventListSorted += tempList;
    } else {
      // Prepend the list of Events without End DateTimes
      tempList += eventListSorted;
      eventListSorted = tempList;
    }
    break;

  case EventSortSummary:
    for ( eit = eventList.begin(); eit != eventList.end(); ++eit ) {
      KCal::Event::Ptr e = Akonadi::event( *eit );
      Q_ASSERT( e );
      sortIt = eventListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != eventListSorted.end() &&
                e->summary() >= Akonadi::event(*sortIt)->summary() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != eventListSorted.end() &&
                e->summary() < Akonadi::event(*sortIt)->summary() ) {
          ++sortIt;
        }
      }
      eventListSorted.insert( sortIt, *eit );
    }
    break;
  }

  return eventListSorted;
}

Item::List Calendar::events( const QDate &date,
                              const KDateTime::Spec &timeSpec,
                              EventSortField sortField,
                              SortDirection sortDirection )
{
  const Item::List el = rawEventsForDate( date, timeSpec, sortField, sortDirection );
  return Akonadi::applyCalFilter( el, filter() );
}


Item::List Calendar::events( const KDateTime &dt )
{
  const Item::List el = rawEventsForDate( dt );
  return Akonadi::applyCalFilter( el, filter() );
}


Item::List Calendar::events( const QDate &start, const QDate &end,
                              const KDateTime::Spec &timeSpec,
                              bool inclusive )
{
  const Item::List el = rawEvents( start, end, timeSpec, inclusive );
  return Akonadi::applyCalFilter( el, filter() );
}

Item::List Calendar::events( EventSortField sortField,
                              SortDirection sortDirection )
{
  const Item::List el = rawEvents( sortField, sortDirection );
  return Akonadi::applyCalFilter( el, filter() );
}

KCal::Incidence::Ptr Calendar::dissociateOccurrence( const Item &item,
                                           const QDate &date,
                                           const KDateTime::Spec &spec,
                                           bool single )
{
  if ( !item.isValid() ) {
    return KCal::Incidence::Ptr();
  }

  const KCal::Incidence::Ptr incidence = Akonadi::incidence( item );
  if ( !incidence || !incidence->recurs() ) {
    return KCal::Incidence::Ptr();
  }

  KCal::Incidence * newInc =  incidence->clone();
  newInc->recreate();
  // Do not call setRelatedTo() when dissociating recurring to-dos, otherwise the new to-do
  // will appear as a child.  Originally, we planned to set a relation with reltype SIBLING
  // when dissociating to-dos, but currently kcal only supports reltype PARENT.
  // We can uncomment the following line when we support the PARENT reltype.
  //newInc->setRelatedTo( incidence );
  KCal::Recurrence *recur = newInc->recurrence();
  if ( single ) {
    recur->clear();
  } else {
    // Adjust the recurrence for the future incidences. In particular adjust
    // the "end after n occurrences" rules! "No end date" and "end by ..."
    // don't need to be modified.
    int duration = recur->duration();
    if ( duration > 0 ) {
      int doneduration = recur->durationTo( date.addDays( -1 ) );
      if ( doneduration >= duration ) {
        kDebug() << "The dissociated event already occurred more often"
                 << "than it was supposed to ever occur. ERROR!";
        recur->clear();
      } else {
        recur->setDuration( duration - doneduration );
      }
    }
  }
  // Adjust the date of the incidence
  if ( incidence->type() == "Event" ) {
    KCal::Event *ev = static_cast<KCal::Event *>( newInc );
    KDateTime start( ev->dtStart() );
    int daysTo = start.toTimeSpec( spec ).date().daysTo( date );
    ev->setDtStart( start.addDays( daysTo ) );
    ev->setDtEnd( ev->dtEnd().addDays( daysTo ) );
  } else if ( incidence->type() == "Todo" ) {
    KCal::Todo *td = static_cast<KCal::Todo *>( newInc );
    bool haveOffset = false;
    int daysTo = 0;
    if ( td->hasDueDate() ) {
      KDateTime due( td->dtDue() );
      daysTo = due.toTimeSpec( spec ).date().daysTo( date );
      td->setDtDue( due.addDays( daysTo ), true );
      haveOffset = true;
    }
    if ( td->hasStartDate() ) {
      KDateTime start( td->dtStart() );
      if ( !haveOffset ) {
        daysTo = start.toTimeSpec( spec ).date().daysTo( date );
      }
      td->setDtStart( start.addDays( daysTo ) );
      haveOffset = true;
    }
  }
  recur = incidence->recurrence();
  if ( recur ) {
    if ( single ) {
      recur->addExDate( date );
    } else {
      // Make sure the recurrence of the past events ends
      // at the corresponding day
      recur->setEndDate( date.addDays(-1) );
    }
  }
  return KCal::Incidence::Ptr( newInc );
}

Item Calendar::incidence( const Item::Id &uid ) const
{
  Item i = event( uid );
  if ( i.isValid() ) {
    return i;
  }

  i = todo( uid );
  if ( i.isValid() ) {
    return i;
  }

  i = journal( uid );
  return i;
}


Item::List Calendar::incidencesFromSchedulingID( const QString &sid )
{
  Item::List result;
  const Item::List incidences = rawIncidences();
  Item::List::const_iterator it = incidences.begin();
  for ( ; it != incidences.end(); ++it ) {
    if ( Akonadi::incidence(*it)->schedulingID() == sid ) {
      result.append( *it );
    }
  }
  return result;
}

Item Calendar::incidenceFromSchedulingID( const QString &UID )
{
  const Item::List incidences = rawIncidences();
  Item::List::const_iterator it = incidences.begin();
  for ( ; it != incidences.end(); ++it ) {
    if ( Akonadi::incidence(*it)->schedulingID() == UID ) {
      // Touchdown, and the crowd goes wild
      return *it;
    }
  }
  // Not found
  return Item();
}

Item::List Calendar::sortTodos( const Item::List &todoList_,
                                TodoSortField sortField,
                                SortDirection sortDirection )
{
  Item::List todoList( todoList_ );
  Item::List todoListSorted;
  Item::List tempList, t;
  Item::List alphaList;
  Item::List::Iterator sortIt;
  Item::List::ConstIterator eit;

  // Notice we alphabetically presort Summaries first.
  // We do this so comparison "ties" stay in a nice order.

  // Note that To-dos may not have Start DateTimes nor due DateTimes.

  switch( sortField ) {
  case TodoSortUnsorted:
    todoListSorted = todoList;
    break;

  case TodoSortStartDate:
    alphaList = sortTodos( todoList, TodoSortSummary, sortDirection );
    for ( eit = alphaList.constBegin(); eit != alphaList.constEnd(); ++eit ) {
      const KCal::Todo::Ptr e = Akonadi::todo( *eit );
      if ( e->hasStartDate() ) {
        sortIt = todoListSorted.begin();
        if ( sortDirection == SortDirectionAscending ) {
          while ( sortIt != todoListSorted.end() &&
                  e->dtStart() >= Akonadi::todo(*sortIt)->dtStart() ) {
            ++sortIt;
          }
        } else {
          while ( sortIt != todoListSorted.end() &&
                  e->dtStart() < Akonadi::todo(*sortIt)->dtStart() ) {
            ++sortIt;
          }
        }
        todoListSorted.insert( sortIt, *eit );
      } else {
        // Keep a list of the To-dos without Start DateTimes
        tempList.append( *eit );
      }
    }
    if ( sortDirection == SortDirectionAscending ) {
      // Append the list of To-dos without Start DateTimes
      todoListSorted += tempList;
    } else {
      // Prepend the list of To-dos without Start DateTimes
      tempList += todoListSorted;
      todoListSorted = tempList;
    }
    break;

  case TodoSortDueDate:
    alphaList = sortTodos( todoList, TodoSortSummary, sortDirection );
    for ( eit = alphaList.constBegin(); eit != alphaList.constEnd(); ++eit ) {
      const KCal::Todo::Ptr e = Akonadi::todo( *eit );
      if ( e->hasDueDate() ) {
        sortIt = todoListSorted.begin();
        if ( sortDirection == SortDirectionAscending ) {
          while ( sortIt != todoListSorted.end() &&
                  e->dtDue() >= Akonadi::todo( *sortIt )->dtDue() ) {
            ++sortIt;
          }
        } else {
          while ( sortIt != todoListSorted.end() &&
                  e->dtDue() < Akonadi::todo( *sortIt )->dtDue() ) {
            ++sortIt;
          }
        }
        todoListSorted.insert( sortIt, *eit );
      } else {
        // Keep a list of the To-dos without Due DateTimes
        tempList.append( *eit );
      }
    }
    if ( sortDirection == SortDirectionAscending ) {
      // Append the list of To-dos without Due DateTimes
      todoListSorted += tempList;
    } else {
      // Prepend the list of To-dos without Due DateTimes
      tempList += todoListSorted;
      todoListSorted = tempList;
    }
    break;

  case TodoSortPriority:
    alphaList = sortTodos( todoList, TodoSortSummary, sortDirection );
    for ( eit = alphaList.constBegin(); eit != alphaList.constEnd(); ++eit ) {
      const KCal::Todo::Ptr e = Akonadi::todo( *eit );
      sortIt = todoListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != todoListSorted.end() &&
                e->priority() >= Akonadi::todo(*sortIt)->priority() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != todoListSorted.end() &&
                e->priority() < Akonadi::todo(*sortIt)->priority() ) {
          ++sortIt;
        }
      }
      todoListSorted.insert( sortIt, *eit );
    }
    break;

  case TodoSortPercentComplete:
    alphaList = sortTodos( todoList, TodoSortSummary, sortDirection );
    for ( eit = alphaList.constBegin(); eit != alphaList.constEnd(); ++eit ) {
      const KCal::Todo::Ptr e = Akonadi::todo( *eit );
      sortIt = todoListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != todoListSorted.end() &&
                e->percentComplete() >= Akonadi::todo(*sortIt)->percentComplete() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != todoListSorted.end() &&
                e->percentComplete() < Akonadi::todo(*sortIt)->percentComplete() ) {
          ++sortIt;
        }
      }
      todoListSorted.insert( sortIt, *eit );
    }
    break;

  case TodoSortSummary:
    for ( eit = todoList.constBegin(); eit != todoList.constEnd(); ++eit ) {
      const KCal::Todo::Ptr e = Akonadi::todo( *eit );
      sortIt = todoListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != todoListSorted.end() &&
                e->summary() >= Akonadi::todo(*sortIt)->summary() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != todoListSorted.end() &&
                e->summary() < Akonadi::todo(*sortIt)->summary() ) {
          ++sortIt;
        }
      }
      todoListSorted.insert( sortIt, *eit );
    }
    break;
  }

  return todoListSorted;
}

Item::List Calendar::todos( TodoSortField sortField,
                            SortDirection sortDirection )
{
  const Item::List tl = rawTodos( sortField, sortDirection );
  return Akonadi::applyCalFilter( tl, filter() );
}

Item::List Calendar::todos( const QDate &date )
{
  Item::List el = rawTodosForDate( date );
  return Akonadi::applyCalFilter( el, filter() );
}

Item::List Calendar::sortJournals( const Item::List &journalList_,
                                      JournalSortField sortField,
                                      SortDirection sortDirection )
{
  Item::List journalList( journalList_ );
  Item::List journalListSorted;
  Item::List::Iterator sortIt;
  Item::List::ConstIterator eit;

  switch( sortField ) {
  case JournalSortUnsorted:
    journalListSorted = journalList;
    break;

  case JournalSortDate:
    for ( eit = journalList.constBegin(); eit != journalList.constEnd(); ++eit ) {
      const KCal::Journal::Ptr e = Akonadi::journal( *eit );
      sortIt = journalListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != journalListSorted.end() &&
                e->dtStart() >= Akonadi::journal(*sortIt)->dtStart() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != journalListSorted.end() &&
                e->dtStart() < Akonadi::journal(*sortIt)->dtStart() ) {
          ++sortIt;
        }
      }
      journalListSorted.insert( sortIt, *eit );
    }
    break;

  case JournalSortSummary:
    for ( eit = journalList.constBegin(); eit != journalList.constEnd(); ++eit ) {
      const KCal::Journal::Ptr e = Akonadi::journal( *eit );
      sortIt = journalListSorted.begin();
      if ( sortDirection == SortDirectionAscending ) {
        while ( sortIt != journalListSorted.end() &&
                e->summary() >= Akonadi::journal(*sortIt)->summary() ) {
          ++sortIt;
        }
      } else {
        while ( sortIt != journalListSorted.end() &&
                e->summary() < Akonadi::journal(*sortIt)->summary() ) {
          ++sortIt;
        }
      }
      journalListSorted.insert( sortIt, *eit );
    }
    break;
  }

  return journalListSorted;
}

Item::List Calendar::journals( JournalSortField sortField,
                               SortDirection sortDirection )
{
  const Item::List jl = rawJournals( sortField, sortDirection );
  return Akonadi::applyCalFilter( jl, filter() );
}

Item::List Calendar::journals( const QDate &date )
{
  Item::List el = rawJournalsForDate( date );
  return Akonadi::applyCalFilter( el, filter() );
}

void Calendar::beginBatchAdding()
{
  emit batchAddingBegins();
}

void Calendar::endBatchAdding()
{
  emit batchAddingEnds();
}

#ifdef AKONADI_PORT_DISABLED

void Calendar::setupRelations( const Item &forincidence )
{
  if ( !forincidence ) {
    return;
  }

  QString uid = forincidence->uid();

  // First, go over the list of orphans and see if this is their parent
  QList<KCal::Incidence*> l = d->mOrphans.values( uid );
  d->mOrphans.remove( uid );
  for ( int i = 0, end = l.count();  i < end;  ++i ) {
    l[i]->setRelatedTo( forincidence );
    forincidence->addRelation( l[i] );
    d->mOrphanUids.remove( l[i]->uid() );
  }

  // Now see about this incidences parent
  if ( !forincidence->relatedTo() && !forincidence->relatedToUid().isEmpty() ) {
    // Incidence has a uid it is related to but is not registered to it yet.
    // Try to find it
    KCal::Incidence *parent = incidence( forincidence->relatedToUid() );
    if ( parent ) {
      // Found it
      forincidence->setRelatedTo( parent );
      parent->addRelation( forincidence );
    } else {
      // Not found, put this in the mOrphans list
      // Note that the mOrphans dict might contain multiple entries with the
      // same key! which are multiple children that wait for the parent
      // incidence to be inserted.
      d->mOrphans.insert( forincidence->relatedToUid(), forincidence );
      d->mOrphanUids.insert( forincidence->uid(), forincidence );
    }
  }
}
#endif // AKONADI_PORT_DISABLED

#ifdef AKONADI_PORT_DISABLED
// If a to-do with sub-to-dos is deleted, move it's sub-to-dos to the orphan list
void Calendar::removeRelations( const Item &incidence )
{
  if ( !incidence ) {
    kDebug() << "Warning: incidence is 0";
    return;
  }

  QString uid = incidence->uid();
  foreach ( KCal::Incidence *i, incidence->relations() ) {
    if ( !d->mOrphanUids.contains( i->uid() ) ) {
      d->mOrphans.insert( uid, i );
      d->mOrphanUids.insert( i->uid(), i );
      i->setRelatedTo( 0 );
      i->setRelatedToUid( uid );
    }
  }

  // If this incidence is related to something else, tell that about it
  if ( incidence->relatedTo() ) {
    incidence->relatedTo()->removeRelation( incidence );
  }

  // Remove this one from the orphans list
  if ( d->mOrphanUids.remove( uid ) ) {
    // This incidence is located in the orphans list - it should be removed
    // Since the mOrphans dict might contain the same key (with different
    // child incidence pointers!) multiple times, take care that we remove
    // the correct one. So we need to remove all items with the given
    // parent UID, and readd those that are not for this item. Also, there
    // might be other entries with differnet UID that point to this
    // incidence (this might happen when the relatedTo of the item is
    // changed before its parent is inserted. This might happen with
    // groupware servers....). Remove them, too
    QStringList relatedToUids;

    // First, create a list of all keys in the mOrphans list which point
    // to the removed item
    relatedToUids << incidence->relatedToUid();
    for ( QMultiHash<QString, Incidence*>::Iterator it = d->mOrphans.begin();
          it != d->mOrphans.end(); ++it ) {
      if ( it.value()->uid() == uid ) {
        relatedToUids << it.key();
      }
    }

    // now go through all uids that have one entry that point to the incidence
    for ( QStringList::const_iterator uidit = relatedToUids.constBegin();
          uidit != relatedToUids.constEnd(); ++uidit ) {
      Incidence::List tempList;
      // Remove all to get access to the remaining entries
      QList<KCal::Incidence*> l = d->mOrphans.values( *uidit );
      d->mOrphans.remove( *uidit );
      foreach ( Incidence *i, l ) {
        if ( i != incidence ) {
          tempList.append( i );
        }
      }
      // Readd those that point to a different orphan incidence
      for ( KCal::Incidence::List::Iterator incit = tempList.begin();
            incit != tempList.end(); ++incit ) {
        d->mOrphans.insert( *uidit, *incit );
      }
    }
  }

  // Make sure the deleted incidence doesn't relate to a non-deleted incidence,
  // since that would cause trouble in CalendarLocal::close(), as the deleted
  // incidences are destroyed after the non-deleted incidences. The destructor
  // of the deleted incidences would then try to access the already destroyed
  // non-deleted incidence, which would segfault.
  //
  // So in short: Make sure dead incidences don't point to alive incidences
  // via the relation.
  //
  // This crash is tested in CalendarLocalTest::testRelationsCrash().
  incidence->setRelatedTo( 0 );
}
#endif // AKONADI_PORT_DISABLED


void Calendar::CalendarObserver::calendarIncidenceAdded( const Item &incidence )
{
  Q_UNUSED( incidence );
}

void Calendar::CalendarObserver::calendarIncidenceChanged( const Item &incidence )
{
  Q_UNUSED( incidence );
}

void Calendar::CalendarObserver::calendarIncidenceDeleted( const Item &incidence )
{
  Q_UNUSED( incidence );
}

void Calendar::registerObserver( CalendarObserver *observer )
{
  if ( !d->mObservers.contains( observer ) ) {
    d->mObservers.append( observer );
  }
  d->mNewObserver = true;
}

void Calendar::unregisterObserver( CalendarObserver *observer )
{
  d->mObservers.removeAll( observer );
}


void Calendar::doSetTimeSpec( const KDateTime::Spec &timeSpec )
{
  Q_UNUSED( timeSpec );
}


void Calendar::notifyIncidenceAdded( const Item &i )
{
  if ( !d->mObserversEnabled ) {
    return;
  }

  foreach ( CalendarObserver *observer, d->mObservers ) {
    observer->calendarIncidenceAdded( i );
  }
}

void Calendar::notifyIncidenceChanged( const Item &i )
{
  if ( !d->mObserversEnabled ) {
    return;
  }

  foreach ( CalendarObserver *observer, d->mObservers ) {
    observer->calendarIncidenceChanged( i );
  }
}


void Calendar::notifyIncidenceDeleted( const Item &i )
{
  if ( !d->mObserversEnabled ) {
    return;
  }

  foreach ( CalendarObserver *observer, d->mObservers ) {
    observer->calendarIncidenceDeleted( i );
  }
}

void Calendar::customPropertyUpdated()
{
}

void Calendar::setProductId( const QString &id )
{
  d->mProductId = id;
}

QString Calendar::productId() const
{
  return d->mProductId;
}

Item::List Calendar::mergeIncidenceList( const Item::List &events,
                                         const Item::List &todos,
                                         const Item::List &journals )
{
  Item::List incidences;

  int i, end;
  for ( i = 0, end = events.count();  i < end;  ++i ) {
    incidences.append( events[i] );
  }

  for ( i = 0, end = todos.count();  i < end;  ++i ) {
    incidences.append( todos[i] );
  }

  for ( i = 0, end = journals.count();  i < end;  ++i ) {
    incidences.append( journals[i] );
  }

  return incidences;
}

void Calendar::setObserversEnabled( bool enabled )
{
  d->mObserversEnabled = enabled;
}

void Calendar::appendAlarms( KCal::Alarm::List &alarms, const Item &item,
                             const KDateTime &from, const KDateTime &to )
{
  const KCal::Incidence::Ptr incidence = Akonadi::incidence( item );
  Q_ASSERT( incidence );

  KDateTime preTime = from.addSecs(-1);

  KCal::Alarm::List alarmlist = incidence->alarms();
  for ( int i = 0, iend = alarmlist.count();  i < iend;  ++i ) {
    if ( alarmlist[i]->enabled() ) {
      KDateTime dt = alarmlist[i]->nextRepetition( preTime );
      if ( dt.isValid() && dt <= to ) {
        kDebug() << incidence->summary() << "':" << dt.toString();
        alarms.append( alarmlist[i] );
      }
    }
  }
}

void Calendar::appendRecurringAlarms( KCal::Alarm::List &alarms,
                                      const Item &item,
                                      const KDateTime &from,
                                      const KDateTime &to )
{
  KDateTime dt;
  bool endOffsetValid = false;
  KCal::Duration endOffset( 0 );
  KCal::Duration period( from, to );

  const KCal::Incidence::Ptr incidence = Akonadi::incidence( item );
  Q_ASSERT( incidence );


  KCal::Alarm::List alarmlist = incidence->alarms();
  for ( int i = 0, iend = alarmlist.count();  i < iend;  ++i ) {
    KCal::Alarm *a = alarmlist[i];
    if ( a->enabled() ) {
      if ( a->hasTime() ) {
        // The alarm time is defined as an absolute date/time
        dt = a->nextRepetition( from.addSecs(-1) );
        if ( !dt.isValid() || dt > to ) {
          continue;
        }
      } else {
        // Alarm time is defined by an offset from the event start or end time.
        // Find the offset from the event start time, which is also used as the
        // offset from the recurrence time.
        KCal::Duration offset( 0 );
        if ( a->hasStartOffset() ) {
          offset = a->startOffset();
        } else if ( a->hasEndOffset() ) {
          offset = a->endOffset();
          if ( !endOffsetValid ) {
            endOffset = KCal::Duration( incidence->dtStart(), incidence->dtEnd() );
            endOffsetValid = true;
          }
        }

        // Find the incidence's earliest alarm
        KDateTime alarmStart =
          offset.end( a->hasEndOffset() ? incidence->dtEnd() : incidence->dtStart() );
//        KDateTime alarmStart = incidence->dtStart().addSecs( offset );
        if ( alarmStart > to ) {
          continue;
        }
        KDateTime baseStart = incidence->dtStart();
        if ( from > alarmStart ) {
          alarmStart = from;   // don't look earlier than the earliest alarm
          baseStart = (-offset).end( (-endOffset).end( alarmStart ) );
        }

        // Adjust the 'alarmStart' date/time and find the next recurrence at or after it.
        // Treate the two offsets separately in case one is daily and the other not.
        dt = incidence->recurrence()->getNextDateTime( baseStart.addSecs(-1) );
        if ( !dt.isValid() ||
             ( dt = endOffset.end( offset.end( dt ) ) ) > to ) // adjust 'dt' to get the alarm time
        {
          // The next recurrence is too late.
          if ( !a->repeatCount() ) {
            continue;
          }

          // The alarm has repetitions, so check whether repetitions of previous
          // recurrences fall within the time period.
          bool found = false;
          KCal::Duration alarmDuration = a->duration();
          for ( KDateTime base = baseStart;
                ( dt = incidence->recurrence()->getPreviousDateTime( base ) ).isValid();
                base = dt ) {
            if ( a->duration().end( dt ) < base ) {
              break;  // this recurrence's last repetition is too early, so give up
            }

            // The last repetition of this recurrence is at or after 'alarmStart' time.
            // Check if a repetition occurs between 'alarmStart' and 'to'.
            int snooze = a->snoozeTime().value();   // in seconds or days
            if ( a->snoozeTime().isDaily() ) {
              KCal::Duration toFromDuration( dt, base );
              int toFrom = toFromDuration.asDays();
              if ( a->snoozeTime().end( from ) <= to ||
                   ( toFromDuration.isDaily() && toFrom % snooze == 0 ) ||
                   ( toFrom / snooze + 1 ) * snooze <= toFrom + period.asDays() ) {
                found = true;
#ifndef NDEBUG
                // for debug output
                dt = offset.end( dt ).addDays( ( ( toFrom - 1 ) / snooze + 1 ) * snooze );
#endif
                break;
              }
            } else {
              int toFrom = dt.secsTo( base );
              if ( period.asSeconds() >= snooze ||
                   toFrom % snooze == 0 ||
                   ( toFrom / snooze + 1 ) * snooze <= toFrom + period.asSeconds() )
              {
                found = true;
#ifndef NDEBUG
                // for debug output
                dt = offset.end( dt ).addSecs( ( ( toFrom - 1 ) / snooze + 1 ) * snooze );
#endif
                break;
              }
            }
          }
          if ( !found ) {
            continue;
          }
        }
      }
      kDebug() << incidence->summary() << "':" << dt.toString();
      alarms.append( a );
    }
  }

}

Collection Calendar::collection( const Akonadi::Collection::Id &id )
{
  if ( d->m_collectionMap.contains( id ) ) {
    return d->m_collectionMap[id];
  } else {
    return Collection();
  }
}