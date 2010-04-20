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

class QModelIndex;
class KDescendantsProxyModel;

namespace Akonadi {
    class CalendarModel;
}

namespace KHolidays
{
    class HolidayRegion;
} // namespace KHolidays

/**
    The calendar data engine delivers calendar events.
    It can be queried for holidays or the akonadi calendar.
    Supported requests are:
        holidaysRegions - list of regions for which holiday information is available
        holidaysInMonth:region:YYYY-MM-DD
        isHoliday:region:YYYY-MM-DD
        description:region:YYYY-MM-DD

        eventsInMonth:YYYY-MM-DD
        events:YYYY-MM-DD:YYYY-MM-DD
        events:YYYY-MM-DD
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
        /// a request for data that comes from akonadi
        /// creates EventDataContainers as needed
        bool akonadiCalendarSourceRequest(const QString& name, const QStringList& tokens);

        /// this will start akonadi if necessary and init m_calendarModel
        void initAkonadiCalendar();

        /// this is the representation of the root calendar itself. it contains everything (calendars, incidences)
        Akonadi::CalendarModel* m_calendarModel;

        /// this is used to flatten m_calendarModel to a list. it will still contain items representing the calendars
        KDescendantsProxyModel* m_descendantsModel;

        /// holiday calendar
        QHash<QString, KHolidays::HolidayRegion *> m_regions;
};

K_EXPORT_PLASMA_DATAENGINE(calendar, CalendarEngine)

#endif
