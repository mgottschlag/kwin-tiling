/*
    Copyright (c) 2009 Davide Bettio <davide.bettio@kdemail.net>
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


#ifndef CALENDARENGINE_H
#define CALENDARENGINE_H

#include <Plasma/DataEngine>

namespace Akonadi {
    class Calendar;
}

namespace KHolidays
{
    class HolidayRegion;
} // namespace KHolidays

/**
    The calendar data engine delivers calendar events.
    It can be queried for holidays or the akonadi calendar.

    Supported Holiday requests are:

        holidaysRegions
            * Returns a list of available Holiday Regions
              Each Holiday Region is a pair of a QString containing the regionCode and a Data
              containing value pairs for "name", "description", "countryCode", "location", "languageCode"
        holdaysIsValidRegion:[regionCode]
            * Returns a bool if given Holiday Region is valid
        holidaysDefaultRegion:[regionCode]
            * Returns a QString of a sensible default Holiday Region
        holidays:[regionCode]:[YYYY-MM-DD]:[YYYY-MM-DD]
            * Returns a QList of all holidays in a Holiday Region between two given dates.
              Note that multiple holidays can be returned for each date.
              Each holiday is a Data containing string value pairs for "date", "name" and
              "observanceType".  The date is Gregorian in ISO format. The "observanceType"
              is either "PublicHoliday" or "Other".
        holidays:[regionCode]:[YYYY-MM-DD]
            * Returns a QList of all holidays  in a Holiday Region on a given day
        holidaysInMonth:[regionCode]:[YYYY-MM-DD]
            * Returns a QList of all holidays in a Holiday Region in a given month
        isHoliday:[regionCode]:[YYYY-MM-DD]
            * Returns a bool if a given date is a Holiday in the given Holiday Region
        description:[regionCode]:[YYYY-MM-DD]
            * Returns a QString of all holiday names in a given Holiday Region on a given date

    Supported Akonadi requests are:

        eventsInMonth:[YYYY-MM-DD]
        events:[YYYY-MM-DD]:[YYYY-MM-DD]
        events:[YYYY-MM-DD]

        The returned data contains (not all fields guaranteed to be populated):

            "UID"                     QString
            "Type"                    QString        "Event", "Todo", Journal"
            "Summary"                 QString
            "Comments"                QStringList
            "Location"                QString
            "OrganizerName"           QString
            "OrganizerEmail"          QString
            "Priority"                int
            "StartDate"               KDateTime
            "EndDate"                 KDateTime
            "RecurrenceDates"         QList(QVariant(KDateTime))
            "Recurs"                  bool
            "AllDay"                  bool
            "Categories"              QStringList
            "Resources"               QStringList
            "DurationDays"            int
            "DurationSeconds"         int
            "Status"                  QString         "None", "Tentative", "Confirmed", "Draft",
                                                      "Final", "Completed", "InProcess",
                                                      "Cancelled", "NeedsAction", "NonStandard",
                                                      "Unknown"
            "StatusName"              QString         translated Status
            "Secrecy"                 QString         "Public", "Private", "Confidential", "Unknown"
            "SecrecyName"             QString         translated Secrecy
            "Occurrences"             QList(QVariant(Plasma::DataEngine::Data))
                where each Data contains details of an occurence of the event:
                    "OccurrenceUid"          QString      for convenience, same as UID
                    "OccurrenceStartDate"    KDateTime
                    "OccurrenceEndDate"      KDateTime

        Event type specific data keys:
            "EventMultiDay"           bool
            "EventHasEndDate"         bool
            "EventTransparency"       QString         "Opaque", "Transparent", "Unknown"

        Todo type specific data keys:
            "TodoHasStartDate"        bool
            "TodoIsOpenEnded"         bool
            "TodoHasDueDate"          bool
            "TodoDueDate"             KDateTime
            "TodoIsCompleted"         bool
            "TodoIsInProgress"        bool
            "TodoIsNotStarted"        bool
            "TodoPercentComplete"     int
            "TodoHasCompletedDate"    bool
            "TodoCompletedDate"       bool

        Fields still to be done:
            Attendees
            Attachments
            Relations
            Alarms
            Custom Properties
            Lat/Lon
            Collection/Source

*/
class CalendarEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        CalendarEngine( QObject* parent, const QVariantList& args );
        ~CalendarEngine();

    protected:
        /// general request for data from this data engine
        bool sourceRequestEvent(const QString &name);

    private:
        /// a request for holidays data
        bool holidayCalendarSourceRequest(const QString& key, const QStringList& args, const QString& request);

        /// a request for data that comes from akonadi
        /// creates EventDataContainers as needed
        bool akonadiCalendarSourceRequest(const QString& key, const QStringList& args, const QString& request);

        /// this will start akonadi if necessary and init m_calendarModel
        void initAkonadiCalendar();

        /// this is the representation of the root calendar itself. it contains everything (calendars, incidences)
        Akonadi::Calendar *m_calendar;

        /// holiday calendar
        QHash<QString, KHolidays::HolidayRegion *> m_regions;
        QString m_defaultHolidayRegion; // Cached value of default holiday region
        QString m_defaultHolidayRegionCountry; // The locale country when the cached default calculated
        QString m_defaultHolidayRegionLanguage; // The locale language when the cached default calculated
};

K_EXPORT_PLASMA_DATAENGINE(calendar, CalendarEngine)

#endif
