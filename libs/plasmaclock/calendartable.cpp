/*
 *   Copyright 2008,2010 Davide Bettio <davide.bettio@kdemail.net>
 *   Copyright 2009 John Layt <john@layt.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "calendartable.h"
#include "config-calendartable.h"

//Qt
#include <QtCore/QDate>
#include <QtCore/QListIterator>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QWidget>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QStyleOptionGraphicsItem>

//KDECore
#include <KGlobal>
#include <KDateTime>
#include <KDebug>
#include <KConfigDialog>
#include <KConfigGroup>

//Plasma
#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/DataEngine>
#include <Plasma/DataEngineManager>

#ifdef HAVE_KDEPIMLIBS
#include "ui_calendarHolidaysConfig.h"
#else
#include "ui_calendarConfig.h"
#endif

#include <cmath>

namespace Plasma
{

static const int DISPLAYED_WEEKS = 6;

class CalendarCellBorder
{
public:
    CalendarCellBorder(int c, int w, int d, CalendarTable::CellTypes t, QDate dt)
        : cell(c),
          week(w),
          weekDay(d),
          type(t),
          date(dt)
    {
    }

    int cell;
    int week;
    int weekDay;
    CalendarTable::CellTypes type;
    QDate date;
};

class CalendarTablePrivate
{
    public:
        enum Populations { NoPendingPopulation = 0, PopulateHolidays = 1, PopulateEvents = 2 };

        CalendarTablePrivate(CalendarTable *calTable, const QDate &initialDate = QDate::currentDate())
            : q(calTable),
              calendarType("locale"),
              calendar(KGlobal::locale()->calendar()),
              displayEvents(false),
              displayHolidays(false),
              calendarDataEngine(0),
              automaticUpdates(true),
              opacity(0.5),
              pendingPopulations(NoPendingPopulation),
              delayedPopulationTimer(new QTimer())
        {
            KGlobal::locale()->insertCatalog("libkholidays");

            svg = new Svg();
            svg->setImagePath("widgets/calendar");
            svg->setContainsMultipleImages(true);

            delayedPopulationTimer->setInterval(0);
            delayedPopulationTimer->setSingleShot(true);
            QObject::connect(delayedPopulationTimer, SIGNAL(timeout()), q, SLOT(populateCalendar()));

            setDate(initialDate);
        }

        ~CalendarTablePrivate()
        {
            // Delete the old calendar first if it's not the global calendar
            if (calendar != KGlobal::locale()->calendar()) {
                delete calendar;
            }

            if (calendarDataEngine) {
                Plasma::DataEngineManager::self()->unloadEngine("calendar");
            }

            delete svg;
            delete delayedPopulationTimer;
        }

        void setCalendar(const KCalendarSystem *newCalendar)
        {
            // If not the global calendar, delete the old calendar first
            if (calendar != KGlobal::locale()->calendar()) {
                delete calendar;
            }

            calendar = newCalendar;

            if (calendar == KGlobal::locale()->calendar()) {
                calendarType = "locale";
            } else {
                calendarType = calendar->calendarType();
            }

            // Force date update to refresh cached date componants then update display
            setDate(selectedDate);
            updateHoveredPainting(QPointF());
            populateHolidays();
            populateEvents();
            q->update();
        }

        void setDate(const QDate &setDate)
        {
            selectedDate = setDate;
            selectedMonth = calendar->month(setDate);
            selectedYear = calendar->year(setDate);
            weekDayFirstOfSelectedMonth = weekDayFirstOfMonth(setDate);
            daysInWeek = calendar->daysInWeek(setDate);
            daysInSelectedMonth = calendar->daysInMonth(setDate);
            daysShownInPrevMonth = (weekDayFirstOfSelectedMonth - calendar->weekStartDay() + daysInWeek) % daysInWeek;
            // make sure at least one day of the previous month is visible.
            // 1 = minimum number of days to show, increase if more days should be forced visible:
            if (daysShownInPrevMonth < 1) {
                daysShownInPrevMonth += daysInWeek;
            }
            viewStartDate = dateFromRowColumn(0, 0);
            viewEndDate = dateFromRowColumn(DISPLAYED_WEEKS - 1, daysInWeek - 1);
        }

        //Returns the x co-ordinate of a given column to LTR order, column is 0 to (daysInWeek-1)
        //This version does not adjust for RTL, so should not be used directly for drawing
        int columnToX(int column)
        {
            return q->boundingRect().x() +
                   centeringSpace +
                   weekBarSpace +
                   cellW +
                   ((cellW + cellSpace) * column);
        }

        //Returns the y co-ordinate for given row, row is 0 to (DISPLAYED_WEEKS - 1)
        int rowToY(int row)
        {
            return (int) q->boundingRect().y() +
                         headerHeight +
                         headerSpace +
                         ((cellH + cellSpace) * row);
        }

        //Returns the absolute LTR column for a given x co-ordinate, -1 if outside table
        int xToColumn(qreal x)
        {
            if (x >= columnToX(0) && x < columnToX(daysInWeek)) {
                return ((x - centeringSpace) / (cellW + cellSpace)) - 1;
            }
            return -1;
        }

        //Returns the absolute row for a given y co-ordinate, -1 if outside table
        int yToRow(qreal y)
        {
            if (y >= rowToY(0) && y < rowToY(DISPLAYED_WEEKS)) {
                return (y - headerHeight - headerSpace) / (cellH + cellSpace);
            }
            return -1;
        }

        //Convert between column and weekdayColumn depending on LTR or RTL mode
        //Note the same calculation used in both directions
        int adjustColumn(int column)
        {
            if (column >= 0 && column < daysInWeek) {
                if (q->layoutDirection() == Qt::RightToLeft) {
                    return daysInWeek - column - 1;
                } else {
                    return column;
                }
            }
            return -1;
        }

        //Given an x y point in the table return the cell date.
        //Note can be an invalid date in the calendar system
        QDate dateFromPoint(QPointF point)
        {
            if (point.isNull()) {
                return QDate();
            }

            int column = xToColumn(point.x());
            int row = yToRow(point.y());

            if (column < 0 || column >= daysInWeek || row < 0 || row >= DISPLAYED_WEEKS) {
                return QDate();
            }

            return dateFromRowColumn(row, adjustColumn(column));
        }

        //Given a date in the currently selected month, return the position in the table as a
        //row and column. Note no direction is assumed
        void rowColumnFromDate(const QDate &cellDate, int &weekRow, int &weekdayColumn)
        {
            int offset = calendar->day(cellDate) + daysShownInPrevMonth - 1;
            weekRow = offset / daysInWeek;
            weekdayColumn = offset % daysInWeek;
        }

        //Given a position in the table as a 0-indexed row and column, return the cell date.  Makes
        //no assumption about direction.  Date returned can be an invalid date in the calendar
        //system, or simply invalid.
        QDate dateFromRowColumn(int weekRow, int weekdayColumn)
        {
            QDate cellDate;

            //starting from the first of the month, which is known to always be valid, add/subtract
            //number of days to get to the required cell
            if (calendar->setYMD(cellDate, selectedYear, selectedMonth, 1)) {
                cellDate = calendar->addDays(cellDate, (weekRow * daysInWeek) + weekdayColumn - daysShownInPrevMonth);
            }

            return cellDate;
        }

        void updateHoveredPainting(const QPointF &hoverPoint)
        {
            QRectF oldHoverRect = hoverRect;
            hoverRect = QRectF();
            hoverWeekdayColumn = -1;
            hoverWeekRow = -1;

            if (!hoverPoint.isNull()) {
                int column = xToColumn(hoverPoint.x());
                int row = yToRow(hoverPoint.y());

                if (column >= 0 && column < daysInWeek && row >= 0 && row < DISPLAYED_WEEKS) {
                    hoverRect = QRectF(columnToX(column) - glowRadius,
                                       rowToY(row) - glowRadius,
                                       cellW + glowRadius * 2,
                                       cellH + glowRadius * 2).adjusted(-2,-2,2,2);
                    hoverWeekdayColumn = adjustColumn(column);
                    hoverWeekRow = row;
                }
            }

            // now update what is needed, and only what is needed!
            if (hoverRect != oldHoverRect) {
                //FIXME: update only of a piece seems to paint over the old stuff
                /*if (oldHoverRect.isValid()) {
                    q->update(oldHoverRect);
                }
                if (hoverRect.isValid()) {
                    q->update(hoverRect);
                }*/
                emit q->dateHovered(dateFromRowColumn(hoverWeekRow, hoverWeekdayColumn));
                q->update();
            }
        }

        // calculate weekday number of first day of this month, this is the anchor for all calculations
        int weekDayFirstOfMonth(const QDate &cellDate)
        {
            Q_UNUSED(cellDate);
            QDate firstDayOfMonth;
            int weekday = -1;
            if ( calendar->setYMD(firstDayOfMonth, selectedYear, selectedMonth, 1)) {
                weekday = calendar->dayOfWeek(firstDayOfMonth);
            }
            return weekday;
        }

        QString defaultHolidaysRegion()
        {
            //FIXME: get rid of the query; turn it into a proper async request
            return calendarEngine()->query("holidaysDefaultRegion").value("holidaysDefaultRegion").toString();
        }

        bool isValidHolidaysRegion(const QString &holidayRegion)
        {
            //FIXME: get rid of the query; turn it into a proper async request
            QString queryString = "holidaysIsValidRegion" + QString(':') + holidayRegion;
            return calendarEngine()->query(queryString).value(queryString).toBool();
        }

        bool addHolidaysRegion(const QString &holidayRegion, bool daysOff)
        {
            //kDebug() << holidayRegion << holidaysRegions.contains(holidayRegion) << isValidHolidaysRegion(holidayRegion);
            if (!holidaysRegions.contains(holidayRegion) && isValidHolidaysRegion(holidayRegion)) {
                QString queryString = "holidaysRegion" + QString(':') + holidayRegion;
                Plasma::DataEngine::Data regions = calendarEngine()->query(queryString);
                Plasma::DataEngine::DataIterator it(regions);
                while (it.hasNext()) {
                    it.next();
                    Plasma::DataEngine::Data region = it.value().toHash();
                    region.insert("UseForDaysOff", daysOff);
                    holidaysRegions.insert(it.key(), region);
                }

                return true;
            }

            return false;
        }

        bool holidayIsDayOff(Plasma::DataEngine::Data holidayData)
        {
            return (holidayData.value("ObservanceType").toString() == "PublicHoliday" &&
                    holidaysRegions.value(holidayData.value("RegionCode").toString()).value("UseForDaysOff").toBool());
        }


        void insertPimOccurence(const QString &type, const QDate &date, Plasma::DataEngine::Data occurrence)
        {
            if (date >= viewStartDate && date <= viewEndDate) {
                int julian = date.toJulianDay();
                if (type == "Event" && !events.contains(julian, occurrence)) {
                    events.insert(julian, occurrence);
                } else if (type == "Todo" && !todos.contains(julian, occurrence)) {
                    todos.insert(julian, occurrence);
                } else if (type == "Journal" && !journals.contains(julian, occurrence)) {
                    journals.insert(julian, occurrence);
                }
            }
        }

        Plasma::DataEngine *calendarEngine();
        void checkIfCalendarEngineNeeded();
        void populateHolidays();
        void populateEvents();
        void populateCalendar();

        CalendarTable *q;
        QString calendarType;
        const KCalendarSystem *calendar;

        QDate selectedDate;
        QDate currentDate;
        int selectedMonth;
        int selectedYear;
        int weekDayFirstOfSelectedMonth;
        int daysInWeek;
        int daysInSelectedMonth;
        int daysShownInPrevMonth;
        QDate viewStartDate;
        QDate viewEndDate;

        bool displayEvents;
        bool displayHolidays;
        Plasma::DataEngine *calendarDataEngine;
        // List of Holiday Regions to display
        // Hash key: QString = Holiday Region Code, Data = details of Holiday Region
        QHash<QString, Plasma::DataEngine::Data> holidaysRegions;
        // Index to Holidays that occur on a given date.
        // Hash key: int = Julian Day number of holiday observance, int = key of holiday event
        QMultiHash<int, int> holidays;
        // Holiday details.  A holiday may be observed on multiple dates.
        // Hash key: int = key of holiday event, Data = details of holiday
        QHash<int, Plasma::DataEngine::Data> holidayEvents;
        // Index to Events/Todos/Journals that occur on a given date.
        // Hash key: int = Julian Day number of event/todo occurrence, Data = occurence details including start date, end date and Akonadi incidence UID
        QMultiHash<int, Plasma::DataEngine::Data> events;
        QMultiHash<int, Plasma::DataEngine::Data> todos;
        QMultiHash<int, Plasma::DataEngine::Data> journals;
        // Event/Todo/Journal details.  An event may recur on multiple dates.
        // Hash key: QString = Akonadi UID of event/todo/journal, Data = details of event/todo/journal
        QHash<QString, Plasma::DataEngine::Data> pimEvents;
        QString eventsQuery;

        bool automaticUpdates;

        QPointF lastSeenMousePos;

#ifdef HAVE_KDEPIMLIBS
        Ui::calendarHolidaysConfig calendarConfigUi;
#else
        Ui::calendarConfig calendarConfigUi;
#endif

        Plasma::Svg *svg;
        float opacity; //transparency for the inactive text
        QRectF hoverRect;
        int hoverWeekRow;
        int hoverWeekdayColumn;
        int centeringSpace;
        int cellW;
        int cellH;
        int cellSpace;
        int headerHeight;
        int headerSpace;
        int weekBarSpace;
        int glowRadius;

        QFont weekDayFont;
        QFont dateFontBold;
        QFont dateFont;

        int pendingPopulations;
        QTimer *delayedPopulationTimer;
};

CalendarTable::CalendarTable(const QDate &date, QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarTablePrivate(this, date))
{
    setAcceptHoverEvents(true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

CalendarTable::CalendarTable(QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarTablePrivate(this))
{
    setAcceptHoverEvents(true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

CalendarTable::~CalendarTable()
{
    delete d;
}

void CalendarTable::setCalendar(const QString &newCalendarType)
{
    if (newCalendarType == d->calendarType) {
        return;
    }

    if (newCalendarType == "locale") {
        d->setCalendar(KGlobal::locale()->calendar());
    } else {
        d->setCalendar(KCalendarSystem::create(newCalendarType));
    }

    // Signal out date change so any dependents will update as well
    emit dateChanged(date(), date());
    emit dateChanged(date());
}

void CalendarTable::setCalendar(const KCalendarSystem *newCalendar)
{
    if (newCalendar == d->calendar) {
        return;
    }

    d->setCalendar(newCalendar);

    // Signal out date change so any dependents will update as well
    emit dateChanged(date(), date());
    emit dateChanged(date());
}

const KCalendarSystem *CalendarTable::calendar() const
{
    return d->calendar;
}

void CalendarTable::setDate(const QDate &newDate)
{
    // New date must be valid in the current calendar system
    if (!calendar()->isValid(newDate)) {
        return;
    }

    // If new date is the same as old date don't actually need to do anything
    if (newDate == date()) {
        return;
    }

    int oldYear = d->selectedYear;
    int oldMonth = d->selectedMonth;
    QDate oldDate = date();

    // now change the date
    d->setDate(newDate);

    d->updateHoveredPainting(d->lastSeenMousePos);
    update(); //FIXME: we shouldn't need this update here, but something in Qt is broken (but with plasmoidviewer everything work)

    if (oldYear != d->selectedYear || oldMonth != d->selectedMonth) {
        d->populateHolidays();
        d->populateEvents();
    } else {
        // only update the old and the new areas
        int row, column;
        d->rowColumnFromDate(oldDate, row, column);
        update(cellX(column) - d->glowRadius, cellY(row) - d->glowRadius,
               d->cellW + d->glowRadius * 2, d->cellH + d->glowRadius * 2);

        d->rowColumnFromDate(newDate, row, column);
        update(cellX(column) - d->glowRadius, cellY(row) - d->glowRadius,
               d->cellW + d->glowRadius * 2, d->cellH + d->glowRadius * 2);
    }

    emit dateChanged(newDate, oldDate);
    emit dateChanged(newDate);
}

const QDate& CalendarTable::date() const
{
    return d->selectedDate;
}

void CalendarTable::setDisplayHolidays(bool showHolidays)
{
    if (showHolidays) {
        if (d->holidaysRegions.isEmpty()) {
            d->addHolidaysRegion(d->defaultHolidaysRegion(), true);
        }

        QMutableHashIterator<QString, Plasma::DataEngine::Data> it(d->holidaysRegions);
        while (it.hasNext()) {
            it.next();
            if (!d->isValidHolidaysRegion(it.key())) {
                it.remove();
            }
        }
    } else {
        clearHolidays();
        d->checkIfCalendarEngineNeeded();
    }

    if (d->displayHolidays != showHolidays) {
        d->displayHolidays = showHolidays;
        d->populateHolidays();
    }
}

bool CalendarTable::displayHolidays()
{
    return d->displayHolidays && !d->holidaysRegions.isEmpty();
}

bool CalendarTable::displayEvents()
{
    return d->displayEvents;
}

void CalendarTable::setDisplayEvents(bool display)
{
    if (d->displayEvents == display) {
        return;
    }

    d->displayEvents = display;
    if (display) {
        d->populateEvents();
    } else {
        if (d->calendarDataEngine) {
            d->calendarDataEngine->disconnectSource(d->eventsQuery, this);
        }
        d->events.clear();
        d->todos.clear();
        d->journals.clear();
        d->pimEvents.clear();
        d->checkIfCalendarEngineNeeded();
    }
}

void CalendarTable::clearHolidaysRegions()
{
    d->holidaysRegions.clear();
    clearHolidays();
}

void CalendarTable::addHolidaysRegion(const QString &region, bool daysOff)
{
    if (d->displayHolidays && d->addHolidaysRegion(region, daysOff)) {
        d->populateHolidays();
    }
}

QStringList CalendarTable::holidaysRegions() const
{
    return d->holidaysRegions.keys();
}

QStringList CalendarTable::holidaysRegionsDaysOff() const
{
    QStringList regions;
    QHashIterator<QString, Plasma::DataEngine::Data> it(d->holidaysRegions);
    while (it.hasNext()) {
        it.next();
        if (it.value().value("UseForDaysOff").toBool()) {
            regions.append(it.key());
        }
    }
    return regions;
}

void CalendarTable::clearHolidays()
{
    d->holidayEvents.clear();
    d->holidays.clear();
    update();
}

void CalendarTable::addHoliday(Plasma::DataEngine::Data holidayData)
{
    QDate holidayStartDate = QDate::fromString(holidayData.value("ObservanceStartDate").toString(), Qt::ISODate);
    int holidayDuration = holidayData.value("ObservanceDuration").toInt();
    int uid = d->holidayEvents.count();
    d->holidayEvents.insert(uid, holidayData);
    for (int i = 0; i < holidayDuration; ++i) {
        d->holidays.insertMulti(holidayStartDate.addDays(i).toJulianDay(), uid);
    }
}

bool CalendarTable::dateHasDetails(const QDate &date) const
{
    int julian = date.toJulianDay();
    return d->holidays.contains(julian) ||
           d->events.contains(julian) ||
           d->todos.contains(julian) ||
           d->journals.contains(julian);
}

QStringList CalendarTable::dateDetails(const QDate &date) const
{
    QStringList details;
    const int julian = date.toJulianDay();

    if (d->holidays.contains(julian)) {
        foreach (int holidayUid, d->holidays.values(julian)) {
            Plasma::DataEngine::Data holidayData = d->holidayEvents.value(holidayUid);
            QString region = holidayData.value("RegionCode").toString();

            if (d->holidayIsDayOff(holidayData)) {
                details << i18nc("Day off: Holiday name (holiday region)",
                                 "<i>Holiday</i>: %1 (%2)",
                                 holidayData.value("Name").toString(),
                                 d->holidaysRegions.value(region).value("Name").toString());
            } else {
                QString region = holidayData.value("RegionCode").toString();
                details << i18nc("Not day off: Holiday name (holiday region)",
                                    "%1 (%2)",
                                    holidayData.value("Name").toString(),
                                    d->holidaysRegions.value(region).value("Name").toString());
            }
        }
    }

    if (d->events.contains(julian)) {
        foreach (const Plasma::DataEngine::Data &occurrence, d->events.values(julian)) {
            details << i18n("<i>Event</i>: %1", buildOccurrenceDescription(occurrence));
        }
    }

    if (d->todos.contains(julian)) {
        foreach (const Plasma::DataEngine::Data &occurrence, d->todos.values(julian)) {
            //TODO add Status and Percentage Complete when strings not frozen
            details << i18n("<i>Todo</i>: %1", buildOccurrenceDescription(occurrence));
        }
    }

    return details;
}

QString CalendarTable::buildOccurrenceDescription(const Plasma::DataEngine::Data &occurrence) const
{
    const QString uid = occurrence.value("OccurrenceUid").toString();
    const KDateTime occStartDate = occurrence.value("OccurrenceStartDate").value<KDateTime>();
    const KDateTime occEndDate = occurrence.value("OccurrenceEndDate").value<KDateTime>();
    const Plasma::DataEngine::Data details = d->pimEvents.value(uid);

    if (details.value("AllDay").toBool()) {
        return i18nc("All-day calendar event summary", "<br>%1", details.value("Summary").toString());
    } else if (!occEndDate.isValid() || occStartDate == occEndDate) {
        return i18nc("Time and summary for a calendarevent", "%1<br>%2",
                     KGlobal::locale()->formatTime(occStartDate.time()),
                     details.value("Summary").toString());
    }

    return i18nc("Start and end time and summary for a calendar event", "%1 - %2<br>%3",
                 KGlobal::locale()->formatTime(occStartDate.time()),
                 KGlobal::locale()->formatTime(occEndDate.time()),
                 details.value("Summary").toString());
}

void CalendarTable::setAutomaticUpdateEnabled(bool enabled)
{
    d->automaticUpdates = enabled;
}

bool CalendarTable::isAutomaticUpdateEnabled() const
{
    return d->automaticUpdates;
}

void CalendarTable::setCurrentDate(const QDate &date)
{
    d->currentDate = date;
}

const QDate& CalendarTable::currentDate() const
{
    return d->currentDate;
}

QDate CalendarTable::startDate() const
{
    return d->viewStartDate;
}

QDate CalendarTable::endDate() const
{
    return d->viewEndDate;
}

Plasma::DataEngine *CalendarTablePrivate::calendarEngine()
{
    if (!calendarDataEngine) {
        calendarDataEngine = Plasma::DataEngineManager::self()->loadEngine("calendar");
    }

    return calendarDataEngine;
}

void CalendarTablePrivate::checkIfCalendarEngineNeeded()
{
    if (calendarDataEngine && !displayHolidays && !displayEvents) {
        calendarDataEngine = Plasma::DataEngineManager::self()->loadEngine("calendar");
    }
}

void CalendarTablePrivate::populateHolidays()
{
    pendingPopulations |= PopulateHolidays;
    delayedPopulationTimer->start();
}

void CalendarTablePrivate::populateCalendar()
{
    if (pendingPopulations & PopulateHolidays) {
        holidayEvents.clear();
        holidays.clear();

        if (q->displayHolidays()) {
            // Just fetch the days displayed in the grid
            //FIXME: queries are bad, use an async method
            QString queryString = "holidays" + QString(':') +
                                  q->holidaysRegions().join(QString(',')) +
                                  QString(':') + viewStartDate.toString(Qt::ISODate) +
                                  QString(':') + viewEndDate.toString(Qt::ISODate);
            QList<QVariant> holidays = calendarEngine()->query(queryString).value(queryString).toList();

            QMutableListIterator<QVariant> i(holidays);
            while (i.hasNext()) {
                q->addHoliday(i.next().toHash());
            }
        }

        q->update();
        kDebug() << "repainting due to holiday population";
    }

    if (pendingPopulations & PopulateEvents) {
        events.clear();
        todos.clear();
        journals.clear();
        pimEvents.clear();

        if (calendarDataEngine && !eventsQuery.isEmpty()) {
            calendarDataEngine->disconnectSource(eventsQuery, q);
        }

        if (displayEvents) {
            // Just fetch the days displayed in the grid
            eventsQuery = "events" + QString(':') + viewStartDate.toString(Qt::ISODate) +
                          QString(':') + viewEndDate.toString(Qt::ISODate);
            kDebug() << "connecting to .. " << eventsQuery;
            calendarEngine()->connectSource(eventsQuery, q);
        } else {
            eventsQuery.clear();
        }

        q->update();
    }

    pendingPopulations = NoPendingPopulation;
    emit q->eventsChanged();
}

void CalendarTablePrivate::populateEvents()
{
    pendingPopulations |= PopulateEvents;
    delayedPopulationTimer->start();
}

void CalendarTable::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)
    d->events.clear();
    d->todos.clear();
    d->journals.clear();
    d->pimEvents.clear();

    foreach (const QVariant &v, data) {
        Plasma::DataEngine::Data pimData = v.toHash();
        QString type = pimData.value("Type").toString();
        QString uid = pimData.value("UID").toString();
        QDate startDate = pimData.value("StartDate").value<KDateTime>().date();

        d->pimEvents.insert(uid, pimData);

        QList<QVariant> occurrenceList = pimData.value("Occurrences").toList();
        foreach (const QVariant &occurrence, occurrenceList) {
            QDate occStartDate = occurrence.toHash().value("OccurrenceStartDate").value<KDateTime>().date();
            if (pimData.value("EventMultiDay").toBool() == true) {
                QDate occEndDate = occurrence.toHash().value("OccurrenceEndDate").value<KDateTime>().date();
                QDate multiDate = occStartDate;
                while (multiDate <= occEndDate) {
                    d->insertPimOccurence(type, multiDate, occurrence.toHash());
                    multiDate = multiDate.addDays(1);
                }
            } else {
                d->insertPimOccurence(type, occStartDate, occurrence.toHash());
            }
        }
    }

    emit eventsChanged();
    update();
}

void CalendarTable::applyConfiguration(KConfigGroup cg)
{
    setCalendar(cg.readEntry("calendarType", "locale"));
    const bool holidays = cg.readEntry("displayHolidays", true);
    clearHolidaysRegions();
    if (holidays) {
        QStringList regions = cg.readEntry("holidaysRegions", QStringList(d->defaultHolidaysRegion()));
        QStringList daysOff = cg.readEntry("holidaysRegionsDaysOff", QStringList(d->defaultHolidaysRegion()));
        clearHolidaysRegions();
        foreach (const QString &region, regions) {
            d->addHolidaysRegion(region, daysOff.contains(region));
        }

        d->populateHolidays();
    }

    setDisplayHolidays(holidays);
    setDisplayEvents(cg.readEntry("displayEvents", true));
}

void CalendarTable::writeConfiguration(KConfigGroup cg)
{
    cg.writeEntry("calendarType", d->calendarType);
    cg.writeEntry("holidaysRegions", holidaysRegions());
    cg.writeEntry("holidaysRegionsDaysOff", holidaysRegionsDaysOff());
    cg.writeEntry("displayHolidays", d->displayHolidays);
    cg.writeEntry("displayEvents", d->displayEvents);
}

void CalendarTable::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *calendarConfigWidget = new QWidget();
    d->calendarConfigUi.setupUi(calendarConfigWidget);
    parent->addPage(calendarConfigWidget, i18n("Calendar"), "view-pim-calendar");

    QStringList calendars = KCalendarSystem::calendarSystems();
    d->calendarConfigUi.calendarComboBox->addItem( i18n("Local"), QVariant( "locale" ) );
    foreach ( const QString &cal, calendars ) {
        d->calendarConfigUi.calendarComboBox->addItem( KCalendarSystem::calendarLabel( cal ), QVariant( cal ) );
    }
    d->calendarConfigUi.calendarComboBox->setCurrentIndex( d->calendarConfigUi.calendarComboBox->findData( QVariant( d->calendarType ) ) );

    d->calendarConfigUi.displayEvents->setChecked(d->displayEvents);

#ifdef HAVE_KDEPIMLIBS
    QHashIterator<QString, Plasma::DataEngine::Data> it(d->holidaysRegions);
    while (it.hasNext()) {
        it.next();
        if (it.value().value("UseForDaysOff").toBool()) {
            d->calendarConfigUi.holidayRegionWidget->setRegionUseFlags(it.key(), KHolidays::HolidayRegionSelector::UseDaysOff);
        } else {
            d->calendarConfigUi.holidayRegionWidget->setRegionUseFlags(it.key(), KHolidays::HolidayRegionSelector::UseInformationOnly);
        }
    }
    d->calendarConfigUi.holidayRegionWidget->setDescriptionHidden(true);

    connect(d->calendarConfigUi.holidayRegionWidget, SIGNAL(selectionChanged()), parent, SLOT(settingsModified()));
#endif

    connect(d->calendarConfigUi.calendarComboBox, SIGNAL(activated(int)), parent, SLOT(settingsModified()));
    connect(d->calendarConfigUi.displayEvents, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
}

void CalendarTable::configAccepted(KConfigGroup cg)
{
    setCalendar(d->calendarConfigUi.calendarComboBox->itemData(d->calendarConfigUi.calendarComboBox->currentIndex()).toString());
    setDisplayEvents(d->calendarConfigUi.displayEvents->isChecked());

#ifdef HAVE_KDEPIMLIBS
    clearHolidaysRegions();
    QHash<QString, KHolidays::HolidayRegionSelector::RegionUseFlags> regions = d->calendarConfigUi.holidayRegionWidget->regionUseFlags();
    QHashIterator<QString, KHolidays::HolidayRegionSelector::RegionUseFlags> it(regions);
    bool displayHolidays = false;
    while (it.hasNext()) {
        it.next();
        if (it.value() == KHolidays::HolidayRegionSelector::UseDaysOff) {
            addHolidaysRegion(it.key(), true);
            displayHolidays = true;
        } else if (it.value() == KHolidays::HolidayRegionSelector::UseInformationOnly) {
            addHolidaysRegion(it.key(), false);
            displayHolidays = true;
        }
    }
    setDisplayHolidays(displayHolidays);
#endif

    writeConfiguration(cg);
}

//Returns the x co-ordinate for drawing the day cell on the widget given the weekday column
//Note weekdayColumn is 0 to (daysInWeek -1) and is not a real weekDay number (i.e. NOT Monday=1).
//Adjusts automatically for RTL mode, so don't use to calculate absolute positions
int CalendarTable::cellX(int weekdayColumn)
{
    return d->columnToX(d->adjustColumn(weekdayColumn));
}

//Returns the y co-ordinate for drawing the day cell on the widget given the weekRow
//weekRow is 0 to (DISPLAYED_WEEKS - 1)
int CalendarTable::cellY(int weekRow)
{
    return d->rowToY(weekRow);
}

void CalendarTable::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    if (event->delta() < 0) {
        setDate(calendar()->addMonths(date(), 1));
    } else if (event->delta() > 0) {
        setDate(calendar()->addMonths(date(), -1));
    }
}

void CalendarTable::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    d->lastSeenMousePos = event->pos();

    event->accept();
    QDate date = d->dateFromPoint(event->pos());
    setDate(date);
    emit dateSelected(date);
}

void CalendarTable::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    mousePressEvent(event);
}

void CalendarTable::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    d->lastSeenMousePos = event->pos();

    emit tableClicked();
}

void CalendarTable::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    d->lastSeenMousePos = event->pos();

    d->updateHoveredPainting(event->pos());
}

void CalendarTable::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    Q_UNUSED(event);

    QRectF r = contentsRect();
    int numCols = d->daysInWeek + 1;
    int rectSizeH = int(r.height() / (DISPLAYED_WEEKS + 1));
    int rectSizeW = int(r.width() / numCols);

    //Using integers to help to keep things aligned to the grid
    //kDebug() << r.width() << rectSize;
    d->cellSpace = qMax(1, qMin(4, qMin(rectSizeH, rectSizeW) / 20));
    d->headerSpace = d->cellSpace * 2;
    d->weekBarSpace = d->cellSpace * 2 + 1;
    d->cellH = rectSizeH - d->cellSpace;
    d->cellW = rectSizeW - d->cellSpace;
    d->glowRadius = d->cellW * .1;
    d->headerHeight = (int) (d->cellH / 1.5);
    d->centeringSpace = qMax(0, int((r.width() - (rectSizeW * numCols) - (d->cellSpace * (numCols -1))) / 2));

    // Relative to the cell size
    const qreal weekSize = .75;
    const qreal dateSize = .8;

    d->weekDayFont = Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    d->weekDayFont.setPixelSize(d->cellH * weekSize);

    d->dateFontBold = Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    d->dateFontBold.setPixelSize(d->cellH * dateSize);
    d->dateFontBold.setBold(true);

    d->dateFont = Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);

#if QT_VERSION >= 0x040800
    d->weekDayFont.setHintingPreference(QFont::PreferNoHinting);
    d->dateFontBold.setHintingPreference(QFont::PreferNoHinting);
    d->dateFont.setHintingPreference(QFont::PreferNoHinting);
#endif

    QFontMetrics fm(d->weekDayFont);

    int width = 0;
    for (int i = 0; i < d->daysInWeek; i++) {
        const QString name = calendar()->weekDayName(i, KCalendarSystem::ShortDayName);
        width = qMax(width, fm.width(name));
    }

    if (width > d->cellW * weekSize) {
        d->weekDayFont.setPixelSize(d->weekDayFont.pixelSize() * ((d->cellW * weekSize) / width));
    }

    fm = QFontMetrics(d->dateFontBold);
    width = 0;
    for (int i = 10; i <= 52; i++) {
        width = qMax(width, fm.width(QString::number(i)));
    }

    if (width > d->cellW * dateSize) {
        d->dateFontBold.setPixelSize(d->dateFontBold.pixelSize() * ((d->cellW * dateSize) / width));
    }

    d->dateFont.setPixelSize(d->dateFontBold.pixelSize());
}

void CalendarTable::paintCell(QPainter *p, int cell, int weekRow, int weekdayColumn, CellTypes type, const QDate &cellDate)
{
    Q_UNUSED(cell);

    QString cellSuffix = type & NotInCurrentMonth ? "inactive" : "active";
    QRectF cellArea = QRectF(cellX(weekdayColumn), cellY(weekRow), d->cellW, d->cellH);

    d->svg->paint(p, cellArea, cellSuffix); // draw background

    QColor numberColor = Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    if (type & NotInCurrentMonth || type & InvalidDate) {
        p->setOpacity(d->opacity);
    }

    p->setPen(numberColor);
    p->setFont(type & Event ? d->dateFontBold : d->dateFont);
    if (!(type & InvalidDate)) {
        p->drawText(cellArea, Qt::AlignCenter, calendar()->dayString(cellDate, KCalendarSystem::ShortFormat), &cellArea); //draw number
    }
    p->setOpacity(1.0);
}

void CalendarTable::paintBorder(QPainter *p, int cell, int weekRow, int weekdayColumn, CellTypes type, const QDate &cellDate)
{
    Q_UNUSED(cell);
    Q_UNUSED(cellDate);

    if (type & Hovered) {
        d->svg->paint(p, QRect(cellX(weekdayColumn), cellY(weekRow), d->cellW, d->cellH), "hoverHighlight");
    }

    QString elementId;

    if (type & Today) {
        elementId = "today";
    } else if (type & Selected) {
        elementId = "selected";
    } else if (type & PublicHoliday) {
        elementId = "red";
    } else if (type & Holiday) {
        elementId = "green";
    } else {
        return;
    }

    d->svg->paint(p, QRectF(cellX(weekdayColumn) - 1, cellY(weekRow) - 1, d->cellW + 1, d->cellH + 2), elementId);
}

void CalendarTable::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    // Draw weeks numbers column and day header
    QRectF r = boundingRect();
    d->svg->paint(p, QRectF(r.x() + d->centeringSpace, cellY(0), d->cellW,
                  cellY(DISPLAYED_WEEKS) - cellY(0) - d->cellSpace),  "weeksColumn");
    d->svg->paint(p, QRectF(r.x() + d->centeringSpace, r.y(),
                  cellX(d->daysInWeek) - r.x() - d->cellSpace - d->centeringSpace, d->headerHeight), "weekDayHeader");

    QList<CalendarCellBorder> borders;
    QList<CalendarCellBorder> hovers;
    if (d->automaticUpdates) {
        d->currentDate = QDate::currentDate();
    }

    //weekRow and weekDaycolumn of table are 0 indexed and are not equivalent to weekday or week
    //numbers.  In LTR mode we count/paint row and column from top-left corner, in RTL mode we
    //count/paint from top-right corner, but we don't need to know as cellX() calculates the actual
    //painting position for us depending on the mode.
    for (int weekRow = 0; weekRow < DISPLAYED_WEEKS; weekRow++) {
        for (int weekdayColumn = 0; weekdayColumn < d->daysInWeek; weekdayColumn++) {

            int x = cellX(weekdayColumn);
            int y = cellY(weekRow);

            QRectF cellRect(x, y, d->cellW, d->cellH);
            if (!cellRect.intersects(option->exposedRect)) {
                continue;
            }

            QDate cellDate = d->dateFromRowColumn(weekRow, weekdayColumn);
            CalendarTable::CellTypes type(CalendarTable::NoType);
            // get cell info
            const int cellDay = calendar()->day(cellDate);
            const int julian = cellDate.toJulianDay();

            // check what kind of cell we are
            if (calendar()->month(cellDate) != d->selectedMonth) {
                type |= CalendarTable::NotInCurrentMonth;
            }

            if (!calendar()->isValid(cellDate)) {
                type |= CalendarTable::InvalidDate;
            }

            if (cellDate == d->currentDate) {
                type |= CalendarTable::Today;
            }

            if (cellDate == date()) {
                type |= CalendarTable::Selected;
            }

            foreach (int holidayUid, d->holidays.values(julian)) {
                if (d->holidayIsDayOff(d->holidayEvents.value(holidayUid))) {
                    type |= CalendarTable::PublicHoliday;
                } else {
                    type |= CalendarTable::Holiday;
                }
            }

            if (d->events.contains(julian) || d->todos.contains(julian)) {
                type |= CalendarTable::Event;
            }

            if (type != CalendarTable::NoType && type != CalendarTable::NotInCurrentMonth) {
                borders.append(CalendarCellBorder(cellDay, weekRow, weekdayColumn, type, cellDate));
            }

            if (weekRow == d->hoverWeekRow && weekdayColumn == d->hoverWeekdayColumn) {
                type |= CalendarTable::Hovered;
                hovers.append(CalendarCellBorder(cellDay, weekRow, weekdayColumn, type, cellDate));
            }

            paintCell(p, cellDay, weekRow, weekdayColumn, type, cellDate);

            // FIXME: modify svg to allow for a wider week number cell
            // a temporary workaround is to paint only one week number (weekString) when the cell is small
            // and both week numbers (accurateWeekString) when there is enough room
            if (weekdayColumn == 0) {
                QRectF cellRect(r.x() + d->centeringSpace, y, d->cellW, d->cellH);
                p->setPen(Theme::defaultTheme()->color(Plasma::Theme::TextColor));
                p->setFont(d->dateFont);
                p->setOpacity(d->opacity);
                QString weekString;
                QString accurateWeekString;
                if (calendar()->isValid(cellDate)) {
                    weekString = calendar()->weekNumberString(cellDate);
                    accurateWeekString = weekString;
                    if (calendar()->dayOfWeek(cellDate) != Qt::Monday) {
                        QDate nextWeekDate = calendar()->addDays(cellDate, d->daysInWeek);
                        if (calendar()->isValid(nextWeekDate)) {
                            if (layoutDirection() == Qt::RightToLeft) {
                                accurateWeekString.prepend("/").prepend(calendar()->weekNumberString(nextWeekDate));
                            } else {
                                accurateWeekString.append("/").append(calendar()->weekNumberString(nextWeekDate));
                            }
                        }
                        // ensure that weekString is the week number that has the most amout of days in the row
                        QDate middleWeekDate = calendar()->addDays(cellDate, floor(static_cast<float>(d->daysInWeek / 2)));
                        if (calendar()->isValid(middleWeekDate)) {
                            QString middleWeekString = calendar()->weekNumberString(middleWeekDate);
                            if (weekString != middleWeekString) {
                                weekString = middleWeekString;
                            }
                        }
                    }
                }
                QFontMetrics fontMetrics(d->dateFont);
                if (fontMetrics.width(accurateWeekString) > d->cellW) {
                    p->drawText(cellRect, Qt::AlignCenter, weekString); //draw number
                } else {
                    p->drawText(cellRect, Qt::AlignCenter, accurateWeekString); //draw number
                }
                p->setOpacity(1.0);
            }
        }
    }

    // Draw days
    if (option->exposedRect.intersects(QRect(r.x(), r.y(), r.width(), d->headerHeight))) {
        p->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
        int weekStartDay = calendar()->weekStartDay();
        for (int i = 0; i < d->daysInWeek; i++){
            int weekDay = ((i + weekStartDay - 1) % d->daysInWeek) + 1;
            QString dayName = calendar()->weekDayName(weekDay, KCalendarSystem::ShortDayName);
            p->setFont(d->weekDayFont);
            p->drawText(QRectF(cellX(i), r.y(), d->cellW, d->headerHeight),
                        Qt::AlignCenter | Qt::AlignVCenter, dayName);
        }
    }

    // Draw hovers
    foreach (const CalendarCellBorder &border, hovers) {
        p->save();
        paintBorder(p, border.cell, border.week, border.weekDay, border.type, border.date);
        p->restore();
    }

    // Draw borders
    foreach (const CalendarCellBorder &border, borders) {
        p->save();
        paintBorder(p, border.cell, border.week, border.weekDay, border.type, border.date);
        p->restore();
    }
}

} //namespace Plasma

#include "calendartable.moc"
