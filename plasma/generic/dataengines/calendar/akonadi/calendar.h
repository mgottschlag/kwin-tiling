/*
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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef CALENDARSUPPORT_CALENDAR_H
#define CALENDARSUPPORT_CALENDAR_H

#include <Akonadi/Item>

#include <KCalCore/CustomProperties>
#include <KCalCore/Incidence>

#include <KDateTime>

#include <QObject>

namespace KCalCore {
  class CalFilter;
}

class QAbstractItemModel;

namespace CalendarSupport {

/**
   Calendar KCalCore::Incidence sort directions.
*/
enum SortDirection {
  SortDirectionAscending,  /**< Sort in ascending order (first to last) */
  SortDirectionDescending  /**< Sort in descending order (last to first) */
};

/**
   Calendar Event sort keys.
*/
enum EventSortField {
  EventSortUnsorted,       /**< Do not sort Events */
  EventSortStartDate,      /**< Sort Events chronologically, by start date */
  EventSortEndDate,        /**< Sort Events chronologically, by end date */
  EventSortSummary         /**< Sort Events alphabetically, by summary */
};

/**
   Calendar Todo sort keys.
*/
enum TodoSortField {
  TodoSortUnsorted,        /**< Do not sort Todos */
  TodoSortStartDate,       /**< Sort Todos chronologically, by start date */
  TodoSortDueDate,         /**< Sort Todos chronologically, by due date */
  TodoSortPriority,        /**< Sort Todos by priority */
  TodoSortPercentComplete, /**< Sort Todos by percentage completed */
  TodoSortSummary          /**< Sort Todos alphabetically, by summary */
};

/**
  Calendar Journal sort keys.
*/
enum JournalSortField {
  JournalSortUnsorted,     /**< Do not sort Journals */
  JournalSortDate,         /**< Sort Journals chronologically by date */
  JournalSortSummary       /**< Sort Journals alphabetically, by summary */
};

/**
 * Implements a KCalCore::Calendar that uses Akonadi as backend.
 */
class Calendar : public QObject,
                                        public KCalCore::CustomProperties,
                                        public KCalCore::IncidenceBase::IncidenceObserver
{
  Q_OBJECT

  Q_PROPERTY( int incidencesCount READ incidencesCount NOTIFY calendarChanged )
  public:

    /**
      Sets the calendar Product ID to @p id.
      @param id is a string containing the Product ID.
      @see productId() const
    */
    void setProductId( const QString &id );

    /**
      Returns the calendar's Product ID.
      @see setProductId()
    */
    QString productId() const;

    /**
      Sets the owner of the calendar to @p owner.
      @param owner is a Person object.
      @see owner()
    */
    void setOwner( const KCalCore::Person &owner );

    /**
      Returns the owner of the calendar.
      @return the owner Person object.
      @see setOwner()
    */
    KCalCore::Person owner() const;

    /**
      Sets the default time specification (time zone, etc.) used for creating
      or modifying incidences in the Calendar.

      The method also calls setViewTimeSpec(@p timeSpec).

      @param timeSpec time specification
    */
    void setTimeSpec( const KDateTime::Spec &timeSpec );

    /**
       Get the time specification (time zone etc.) used for creating or
       modifying incidences in the Calendar.
       @return time specification
    */
    KDateTime::Spec timeSpec() const;

    /**
      Sets the time zone ID used for creating or modifying incidences in the
      Calendar. This method has no effect on existing incidences.

      The method also calls setViewTimeZoneId(@p timeZoneId).

      @param timeZoneId is a string containing a time zone ID, which is
      assumed to be valid. The time zone ID is used to set the time zone
      for viewing KCalCore::Incidence date/times. If no time zone is found, the
      viewing time specification is set to local clock time.
      @e Example: "Europe/Berlin"
      @see setTimeSpec()
    */
    void setTimeZoneId( const QString &timeZoneId );

    /**
      Returns the time zone ID used for creating or modifying incidences in
      the calendar.

      @return the string containing the time zone ID, or empty string if the
      creation/modification time specification is not a time zone.
    */
    QString timeZoneId() const;

    /**
      Notes the time specification which the client application intends to
      use for viewing the incidences in this calendar. This is simply a
      convenience method which makes a note of the new time zone so that
      it can be read back by viewTimeSpec(). The client application must
      convert date/time values to the desired time zone itself.

      The time specification is not used in any way by the Calendar or its
      incidences; it is solely for use by the client application.

      @param timeSpec time specification

      @see viewTimeSpec()
    */
    void setViewTimeSpec( const KDateTime::Spec &timeSpec ) const;

    /**
      Notes the time zone Id which the client application intends to use for
      viewing the incidences in this calendar. This is simply a convenience
      method which makes a note of the new time zone so that it can be read
      back by viewTimeId(). The client application must convert date/time
      values to the desired time zone itself.

      The Id is not used in any way by the Calendar or its incidences.
      It is solely for use by the client application.

      @param timeZoneId is a string containing a time zone ID, which is
      assumed to be valid. The time zone ID is used to set the time zone
      for viewing KCalCore::Incidence date/times. If no time zone is found, the
      viewing time specification is set to local clock time.
      @e Example: "Europe/Berlin"

      @see viewTimeZoneId()
    */
    void setViewTimeZoneId( const QString &timeZoneId ) const;

    /**
      Returns the time specification used for viewing the incidences in
      this calendar. This simply returns the time specification last
      set by setViewTimeSpec().
      @see setViewTimeSpec().
    */
    KDateTime::Spec viewTimeSpec() const;

    /**
      Returns the time zone Id used for viewing the incidences in this
      calendar. This simply returns the time specification last set by
      setViewTimeSpec().
      @see setViewTimeZoneId().
    */
    QString viewTimeZoneId() const;

    /**
      Shifts the times of all incidences so that they appear at the same clock
      time as before but in a new time zone. The shift is done from a viewing
      time zone rather than from the actual incidence time zone.

      For example, shifting an incidence whose start time is 09:00 America/New York,
      using an old viewing time zone (@p oldSpec) of Europe/London, to a new time
      zone (@p newSpec) of Europe/Paris, will result in the time being shifted
      from 14:00 (which is the London time of the incidence start) to 14:00 Paris
      time.

      @param oldSpec the time specification which provides the clock times
      @param newSpec the new time specification

      @see isLocalTime()
    */
    void shiftTimes( const KDateTime::Spec &oldSpec, const KDateTime::Spec &newSpec );

    /**
      Returns a list of all categories used by KCalCore::Incidences in the calendar @p cal.

      @param cal the calendar to return incidences from
      @return a QStringList containing all the categories.
    */
    static QStringList categories( Calendar *cal );

// KCalCore::Incidence Specific Methods //

    /**
      Returns a filtered list of all KCalCore::Incidences for this Calendar.
      @deprecated:

      @return the list of all filtered KCalCore::Incidences.
    */
    Akonadi::Item::List incidences();

    /**
      Returns a filtered list of all KCalCore::Incidences which occur on the given date.

      @param date request filtered KCalCore::Incidence list for this QDate only.
      @deprecated:

      @return the list of filtered KCalCore::Incidences occurring on the specified date.
    */
    Akonadi::Item::List incidences( const QDate &date );

    /**
      Returns an unfiltered list of all KCalCore::Incidences for this Calendar.
      @deprecated:

      @return the list of all unfiltered KCalCore::Incidences.
    */
    Akonadi::Item::List rawIncidences();

    /**
      Returns the KCalCore::Incidence associated with the given unique identifier.

      @param uid is a unique identifier string.
      @deprecated:

      @return a pointer to the KCalCore::Incidence.
      A null pointer is returned if no such KCalCore::Incidence exists.
    */
    Akonadi::Item incidence( Akonadi::Item::Id id ) const;

    Akonadi::Collection collection( const Akonadi::Entity::Id &id ) const;

    /**
      Returns the KCalCore::Incidence associated with the given scheduling identifier.

      @param sid is a unique scheduling identifier string.
      @deprecated:

      @return a pointer to the KCalCore::Incidence.
      A null pointer is returned if no such KCalCore::Incidence exists.
    */
    Akonadi::Item incidenceFromSchedulingID( const QString &sid );

    /**
      Searches all events and todos for an incidence with this
      scheduling identifiere. Returns a list of matching results.
      @deprecated:

      @param sid is a unique scheduling identifier string.
    */
    Akonadi::Item::List incidencesFromSchedulingID( const QString &sid );

    /**
      Create a merged list of KCalCore::Events, KCalCore::Todos, and KCalCore::Journals.

      @param events is an KCalCore::Event list to merge.
      @param todos is a KCalCore::Todo list to merge.
      @param journals is a KCalCore::Journal list to merge.
      @deprecated:

      @return a list of merged KCalCore::Incidences.
    */
    static Akonadi::Item::List mergeIncidenceList( const Akonadi::Item::List &events,
                                                   const Akonadi::Item::List &todos,
                                                   const Akonadi::Item::List &journals );

    /**
      Dissociate an KCalCore::Incidence from a recurring KCalCore::Incidence.
      By default, only one single KCalCore::Incidence for the specified @a date
      will be dissociated and returned.  If @a single is false, then
      the recurrence will be split at @a date, the old KCalCore::Incidence will
      have its recurrence ending at @a date and the new KCalCore::Incidence
      will have all recurrences past the @a date.

      @param incidence is a pointer to a recurring KCalCore::Incidence.
      @param date is the QDate within the recurring KCalCore::Incidence on which
      the dissociation will be performed.
      @param spec is the spec in which the @a date is formulated.
      @param single is a flag meaning that a new KCalCore::Incidence should be created
      from the recurring KCalCore::Incidences after @a date.
      @deprecated:

      @return a pointer to a new recurring KCalCore::Incidence if @a single is false.
    */
    KCalCore::Incidence::Ptr dissociateOccurrence( const Akonadi::Item &incidence,
                                                   const QDate &date,
                                                   const KDateTime::Spec &spec,
                                                   bool single = true );

// KCalCore::Event Specific Methods //

    /**
      Sort a list of KCalCore::Events.

      @param eventList is a pointer to a list of KCalCore::Events.
      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.
      @deprecated:

      @return a list of KCalCore::Events sorted as specified.
    */
    static Akonadi::Item::List sortEvents( const Akonadi::Item::List &eventList,
                                           EventSortField sortField,
                                           SortDirection sortDirection );

    /**
      Returns a sorted, filtered list of all KCalCore::Events for this Calendar.

      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.
      @deprecated:

      @return the list of all filtered KCalCore::Events sorted as specified.
    */
    virtual Akonadi::Item::List events(
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      Returns a filtered list of all KCalCore::Events which occur on the given timestamp.

      @param dt request filtered KCalCore::Event list for this KDateTime only.
      @deprecated:

      @return the list of filtered KCalCore::Events occurring on the specified timestamp.
    */
    Akonadi::Item::List events( const KDateTime &dt );

    /**
      Returns a filtered list of all KCalCore::Events occurring within a date range.

      @param start is the starting date.
      @param end is the ending date.
      @param timeSpec time zone etc. to interpret @p start and @p end,
      or the calendar's default time spec if none is specified
      @param inclusive if true only KCalCore::Events which are completely included
      within the date range are returned.
      @deprecated:

      @return the list of filtered KCalCore::Events occurring within the specified
      date range.
    */
    Akonadi::Item::List events( const QDate &start, const QDate &end,
                                const KDateTime::Spec &timeSpec = KDateTime::Spec(),
                                bool inclusive = false );

    /**
      Returns a sorted, filtered list of all KCalCore::Events which occur on the given
      date.  The KCalCore::Events are sorted according to @a sortField and
      @a sortDirection.

      @param date request filtered KCalCore::Event list for this QDate only.
      @param timeSpec time zone etc. to interpret @p start and @p end,
      or the calendar's default time spec if none is specified
      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.
      @deprecated:

      @return the list of sorted, filtered KCalCore::Events occurring on @a date.
    */
    Akonadi::Item::List events(
      const QDate &date,
      const KDateTime::Spec &timeSpec = KDateTime::Spec(),
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

// KCalCore::Todo Specific Methods //

    /**
      Sort a list of KCalCore::Todos.

      @param todoList is a pointer to a list of KCalCore::Todos.
      @param sortField specifies the TodoSortField.
      @param sortDirection specifies the SortDirection.
      @deprecated:

      @return a list of KCalCore::Todos sorted as specified.
    */
    static Akonadi::Item::List sortTodos( const Akonadi::Item::List &todoList,
                                          TodoSortField sortField,
                                          SortDirection sortDirection );

    /**
      Returns a sorted, filtered list of all KCalCore::Todos for this Calendar.

      @param sortField specifies the TodoSortField.
      @param sortDirection specifies the SortDirection.
      @deprecated:

      @return the list of all filtered KCalCore::Todos sorted as specified.
    */
    virtual Akonadi::Item::List todos(
      TodoSortField sortField = TodoSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      Returns a filtered list of all KCalCore::Todos which are due on the specified date.

      @param date request filtered KCalCore::Todos due on this QDate.
      @deprecated:

      @return the list of filtered KCalCore::Todos due on the specified date.
    */
    virtual Akonadi::Item::List todos( const QDate &date );

// KCalCore::Journal Specific Methods //

    /**
      Sort a list of KCalCore::Journals.

      @param journalList is a pointer to a list of KCalCore::Journals.
      @param sortField specifies the JournalSortField.
      @param sortDirection specifies the SortDirection.
      @deprecated:

      @return a list of KCalCore::Journals sorted as specified.
    */
    static Akonadi::Item::List sortJournals( const Akonadi::Item::List &journalList,
                                             JournalSortField sortField,
                                             SortDirection sortDirection );

    /**
      Returns a sorted, filtered list of all KCalCore::Journals for this Calendar.

      @param sortField specifies the JournalSortField.
      @param sortDirection specifies the SortDirection.
      @deprecated:

      @return the list of all filtered KCalCore::Journals sorted as specified.
    */
    virtual Akonadi::Item::List journals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      Returns a filtered list of all KCalCore::Journals for on the specified date.

      @param date request filtered KCalCore::Journals for this QDate only.
      @deprecated:

      @return the list of filtered KCalCore::Journals for the specified date.
    */
    virtual Akonadi::Item::List journals( const QDate &date );

    /**
      Emits the beginBatchAdding() signal.

      This should be called before adding a batch of incidences with
      addIncidence( KCalCore::Incidence::Ptr ), addTodo( KCalCore::Todo::Ptr ),
      addEvent( KCalCore::Event::Ptr ) or addJournal( KCalCore::Journal::Ptr ).
      Some Calendars are connected to this signal, e.g: CalendarResources uses
      it to know a series of incidenceAdds are related so the user isn't prompted
      multiple times which resource to save the incidence to

      @since 4.4
    */
    void beginBatchAdding();

    /**
      Emits the endBatchAdding() signal.

      Used with beginBatchAdding(). Should be called after
      adding all incidences.

      @since 4.4
    */
    void endBatchAdding();

// Filter Specific Methods //

    /**
      Sets the calendar filter.

      @param filter a pointer to a CalFilter object which will be
      used to filter Calendar KCalCore::Incidences.
      @deprecated:

      @see filter()
    */
    void setFilter( KCalCore::CalFilter *filter );

    /**
      Returns the calendar filter.

      @return a pointer to the calendar CalFilter.
      A null pointer is returned if no such CalFilter exists.
      @deprecated:

      @see setFilter()
    */
    KCalCore::CalFilter *filter();

// Observer Specific Methods //

    /**
      @class CalendarObserver

      The CalendarObserver class.
    */
    class CalendarObserver //krazy:exclude=dpointer
    {
      public:
        /**
          Destructor.
        */
        virtual ~CalendarObserver() {}

        /**
          Notify the Observer that an KCalCore::Incidence has been inserted.
          @deprecated:

          @param incidence is a pointer to the KCalCore::Incidence that was inserted.
        */
        virtual void calendarIncidenceAdded( const Akonadi::Item &incidence );

        /**
          Notify the Observer that an KCalCore::Incidence has been modified.
          @deprecated:

          @param incidence is a pointer to the KCalCore::Incidence that was modified.
        */
        virtual void calendarIncidenceChanged( const Akonadi::Item  &incidence );

        /**
          Notify the Observer that an KCalCore::Incidence has been removed.
          @deprecated:

          @param incidence is a pointer to the KCalCore::Incidence that was removed.
        */
        virtual void calendarIncidenceDeleted( const Akonadi::Item &incidence );

    };

    /**
      Registers an Observer for this Calendar.

      @param observer is a pointer to an Observer object that will be
      watching this Calendar.

      @see unregisterObserver()
    */
    void registerObserver( CalendarObserver *observer );

    /**
      Unregisters an Observer for this Calendar.

      @param observer is a pointer to an Observer object that has been
      watching this Calendar.

      @see registerObserver()
     */
    void unregisterObserver( CalendarObserver *observer );

    /**
      Returns if the parent collection's rights allow deleting this item.
      Isn't merged with hasChangeRights() for convenience.
    */
    bool hasDeleteRights( const Akonadi::Item &item ) const;

    /**
      Returns if the parent collection's rights allow changing this item.
      Isn't merged with hasDeleteRights() for convenience.
    */
    bool hasChangeRights( const Akonadi::Item &item ) const;

  Q_SIGNALS:
    /**
      Signals that the calendar has been modified.
    */
    void calendarChanged();

    /**
      @see beginBatchAdding()
      @since 4.4
    */
    void batchAddingBegins();

    /**
      @see endBatchAdding()
      @since 4.4
    */
    void batchAddingEnds();

  protected:
    /**
      Let Calendar subclasses set the time specification.
      @param timeSpec is the time specification (time zone, etc.) for
      viewing KCalCore::Incidence dates.\n
    */
    virtual void doSetTimeSpec( const KDateTime::Spec &timeSpec );

    /**
      Let Calendar subclasses notify that they inserted an KCalCore::Incidence.
      @deprecated:
      @param incidence is a pointer to the KCalCore::Incidence object that was inserted.
    */
    void notifyIncidenceAdded( const Akonadi::Item &incidence );

    /**
      Let Calendar subclasses notify that they modified an KCalCore::Incidence.
      @deprecated:
      @param incidence is a pointer to the KCalCore::Incidence object that was modified.
    */
    void notifyIncidenceChanged( const Akonadi::Item &incidence );

    /**
      Let Calendar subclasses notify that they removed an KCalCore::Incidence.
      @deprecated:
      @param incidence is a pointer to the KCalCore::Incidence object that was removed.
    */
    void notifyIncidenceDeleted( const Akonadi::Item &incidence );

    /**
      @copydoc
      CustomProperties::customPropertyUpdated()
    */
    virtual void customPropertyUpdated();

    /**
      Let Calendar subclasses notify that they enabled an Observer.
      @param enabled if true tells the calendar that a subclass has
      enabled an Observer.
    */
    void setObserversEnabled( bool enabled );

    /**
      Appends alarms of incidence in interval to list of alarms.

     @param alarms is a List of KCalCore::Alarms to be appended onto.
     @param incidence is a pointer to an KCalCore::Incidence containing the KCalCore::Alarm
     to be appended.
     @param from is the lower range of the next KCalCore::Alarm repitition.
     @param to is the upper range of the next KCalCore::Alarm repitition.
     @deprecated:
    */
    void appendAlarms( KCalCore::Alarm::List &alarms, const Akonadi::Item &incidence,
                       const KDateTime &from, const KDateTime &to );

    /**
      Appends alarms of recurring events in interval to list of alarms.

      @param alarms is a List of KCalCore::Alarms to be appended onto.
      @param incidence is a pointer to an KCalCore::Incidence containing the KCalCore::Alarm
      to be appended.
      @param from is the lower range of the next KCalCore::Alarm repitition.
      @param to is the upper range of the next KCalCore::Alarm repitition.
      @deprecated:
    */
    void appendRecurringAlarms( KCalCore::Alarm::List &alarms, const Akonadi::Item &incidence,
                                const KDateTime &from, const KDateTime &to );
  public:
    explicit Calendar( QAbstractItemModel *treeModel, QAbstractItemModel *model,
                       const KDateTime::Spec &timeSpec, QObject *parent=0 );
    ~Calendar();

    QAbstractItemModel *model() const;

    QAbstractItemModel *unfilteredModel() const;
    void setUnfilteredModel( QAbstractItemModel *model );

    QAbstractItemModel *treeModel() const;

    void incidenceUpdated( const QString &uid, const KDateTime &recurrenceId );
    void incidenceUpdate( const QString &uid, const KDateTime &recurrenceId );

    Akonadi::Item ::List rawEvents( EventSortField sortField = EventSortUnsorted,
                                    SortDirection sortDirection = SortDirectionAscending );

    Akonadi::Item ::List rawEvents( const QDate &start, const QDate &end,
                                    const KDateTime::Spec &timeSpec = KDateTime::Spec(),
                                    bool inclusive = false );

    Akonadi::Item ::List rawEventsForDate( const QDate &date,
                                           const KDateTime::Spec &timeSpec = KDateTime::Spec(),
                                           EventSortField sortField = EventSortUnsorted,
                                           SortDirection sortDirection = SortDirectionAscending );

    Akonadi::Item::List rawEventsForDate( const KDateTime &dt );

    Akonadi::Item event( Akonadi::Item::Id id ) const;

    Akonadi::Item::List rawTodos( TodoSortField sortField = TodoSortUnsorted,
                                  SortDirection sortDirection = SortDirectionAscending );

    Akonadi::Item::List rawTodosForDate( const QDate &date );

    Akonadi::Item todo( Akonadi::Item::Id uid ) const;

    Akonadi::Item::List rawJournals( JournalSortField sortField = JournalSortUnsorted,
                                     SortDirection sortDirection = SortDirectionAscending );

    Akonadi::Item::List rawJournalsForDate( const QDate &date );

    Akonadi::Item journal( Akonadi::Item::Id id ) const;

    KCalCore::Alarm::List alarms( const KDateTime &from, const KDateTime &to, bool excludeBlockedAlarms = false );
    KCalCore::Alarm::List alarmsTo( const KDateTime &to );

    Akonadi::Item findParent( const Akonadi::Item &item ) const;

    Akonadi::Item::List findChildren( const KCalCore::Incidence::Ptr &incidence ) const;
    Akonadi::Item::List findChildren( const Akonadi::Item &item ) const;
    bool isChild( const Akonadi::Item &parent, const Akonadi::Item &child ) const;

    Akonadi::Item::Id itemIdForIncidenceUid( const QString &uid ) const;
    Akonadi::Item itemForIncidenceUid( const QString &uid ) const;

    /**
     * Returns the number of incidences in the calendar.
     */
    int incidencesCount() const;

    using QObject::event;   // prevent warning about hidden virtual method

  Q_SIGNALS:
    void signalErrorMessage( const QString & );

  private:
    Q_DISABLE_COPY( Calendar )
    class Private;
    Private *const d;
};

}

#endif
