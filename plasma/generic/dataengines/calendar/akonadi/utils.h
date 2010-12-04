/*
  Copyright (c) 2009, 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (C) 2009 KDAB (author: Frank Osterfeld <osterfeld@kde.org>)
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef CALENDARSUPPORT_UTILS_H
#define CALENDARSUPPORT_UTILS_H

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <KCalCore/Event>
#include <KCalCore/Incidence>
#include <KCalCore/Journal>
#include <KCalCore/ScheduleMessage>
#include <KCalCore/Todo>

#include <KDateTime>

#include <QModelIndex>

namespace KPIMIdentities {
class IdentityManager;
}

namespace KCalCore {
  class CalFilter;
}

class QAbstractItemModel;
class QDrag;
class QMimeData;

typedef QList<QModelIndex> QModelIndexList;

namespace CalendarSupport
{

class Calendar;

  /**
   * returns the incidence from an akonadi item, or a null pointer if the item has no such payload
   */
  KCalCore::Incidence::Ptr incidence( const Akonadi::Item &item );

  /**
   * returns the event from an akonadi item, or a null pointer if the item has no such payload
   */
  KCalCore::Event::Ptr event( const Akonadi::Item &item );

  /**
   * returns event pointers from an akonadi item, or a null pointer if the item has no such payload
   */
  KCalCore::Event::List eventsFromItems(
    const Akonadi::Item::List &items );

  /**
   * returns incidence pointers from an akonadi item.
   */
  KCalCore::Incidence::List incidencesFromItems(
    const Akonadi::Item::List &items );


  /**
  * returns the todo from an akonadi item, or a null pointer if the item has no such payload
  */
  KCalCore::Todo::Ptr todo( const Akonadi::Item &item );

  /**
  * returns the journal from an akonadi item, or a null pointer if the item has no such payload
  */
  KCalCore::Journal::Ptr journal( const Akonadi::Item &item );

  /**
   * returns whether an Akonadi item contains an incidence
   */
  bool hasIncidence( const Akonadi::Item &item );

  /**
   * returns whether an Akonadi item contains an event
   */
  bool hasEvent( const Akonadi::Item &item );

  /**
   * returns whether an Akonadi item contains a todo
   */
  bool hasTodo( const Akonadi::Item &item );

  /**
   * returns whether an Akonadi item contains a journal
   */
  bool hasJournal( const Akonadi::Item &item );

  /**
   * returns whether this item can be deleted
   */
  bool hasDeleteRights( const Akonadi::Item &item );

  /**
   * returns whether this item can be changed
   */
  bool hasChangeRights( const Akonadi::Item &item );

  /**
  * returns @p true if the URL represents an Akonadi item and has one of the given mimetypes.
  */
  bool isValidIncidenceItemUrl( const KUrl &url,
                                                       const QStringList &supportedMimeTypes );

  bool isValidIncidenceItemUrl( const KUrl &url );

  /**
  * returns @p true if the mime data object contains any of the following:
  *
  * * An akonadi item with a supported KCal mimetype
  * * an iCalendar
  * * a VCard
  */
  bool canDecode( const QMimeData *mimeData );

  QList<KUrl> incidenceItemUrls( const QMimeData *mimeData );

  QList<KUrl> todoItemUrls( const QMimeData *mimeData );

  bool mimeDataHasTodo( const QMimeData *mimeData );

  KCalCore::Todo::List todos( const QMimeData *mimeData,
                                                     const KDateTime::Spec &timeSpec );

  /**
  * returns @p true if the URL represents an Akonadi item and has one of the given mimetypes.
  */
  bool isValidTodoItemUrl( const KUrl &url );

  /**
  * creates mime data object for dragging an akonadi item containing an incidence
  */
  QMimeData *createMimeData( const Akonadi::Item &item,
                                                    const KDateTime::Spec &timeSpec );

  /**
  * creates mime data object for dragging akonadi items containing an incidence
  */
  QMimeData *createMimeData( const Akonadi::Item::List &items,
                                                    const KDateTime::Spec &timeSpec );

#ifndef QT_NO_DRAGANDDROP
  /**
  * creates a drag object for dragging an akonadi item containing an incidence
  */
  QDrag *createDrag( const Akonadi::Item &item,
                                            const KDateTime::Spec &timeSpec, QWidget *parent );

  /**
  * creates a drag object for dragging akonadi items containing an incidence
  */
  QDrag *createDrag( const Akonadi::Item::List &items,
                                            const KDateTime::Spec &timeSpec, QWidget *parent );
#endif
  /**
    Applies a filter to a list of items containing incidences.
    Items not containing incidences or not matching the filter are removed.
    Helper method anologous to KCalCore::CalFilter::apply()
    @see KCalCore::CalFilter::apply()
    @param items the list of items to filter
    @param filter the filter to apply to the list of items
    @return the filtered list of items
  */
  Akonadi::Item::List applyCalFilter( const Akonadi::Item::List &items,
                                                             const KCalCore::CalFilter *filter );

  /**
    Shows a modal dialog that allows to select a collection.

    @param will contain the dialogCode, QDialog::Accepted if the user pressed Ok,
    QDialog::Rejected otherwise
    @param parent The optional parent of the modal dialog.
    @return The select collection or an invalid collection if
    there was no collection selected.
  */
  Akonadi::Collection selectCollection(
    QWidget *parent, int &dialogCode,
    const QStringList &mimeTypes,
    const Akonadi::Collection &defaultCollection = Akonadi::Collection() );

  Akonadi::Item itemFromIndex( const QModelIndex &index );

  Akonadi::Item::List itemsFromModel(
    const QAbstractItemModel *model,
    const QModelIndex &parentIndex = QModelIndex(),
    int start = 0,
    int end = -1 );

  Akonadi::Collection::List collectionsFromModel(
    const QAbstractItemModel *model,
    const QModelIndex &parentIndex = QModelIndex(),
    int start = 0,
    int end = -1 );

  Akonadi::Collection collectionFromIndex( const QModelIndex &index );

  Akonadi::Collection::Id collectionIdFromIndex( const QModelIndex &index );

  Akonadi::Collection::List collectionsFromIndexes(
    const QModelIndexList &indexes );

  QString displayName( const Akonadi::Collection &coll );

  QString subMimeTypeForIncidence(
    const KCalCore::Incidence::Ptr &incidence );

  /**
      Returns a list containing work days between @p start and @end.
  */
  QList<QDate> workDays( const QDate &start, const QDate &end );

  /**
    Returns a list of holidays that occur at @param date.
  */
  QStringList holiday( const QDate &date );

  void sendAsICalendar( const Akonadi::Item& item, KPIMIdentities::IdentityManager *identityManager, QWidget* parentWidget = 0 );

  void publishItemInformation( const Akonadi::Item& item, Calendar* calendar, QWidget* parentWidget = 0 );

  void scheduleiTIPMethods( KCalCore::iTIPMethod method, const Akonadi::Item &item, Calendar* calendar, QWidget *parentWidget = 0 );

  void saveAttachments( const Akonadi::Item& item, QWidget* parentWidget = 0 );

}

#endif
