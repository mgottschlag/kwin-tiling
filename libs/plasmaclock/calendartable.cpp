/*
 *   Copyright 2008 Davide Bettio <davide.bettio@kdemail.net>
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

//Qt
#include <QtCore/QDate>
#include <QtCore/QListIterator>
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

#include "ui_calendarConfig.h"

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

        CalendarTablePrivate(CalendarTable *calTable, const QDate &initialDate = QDate::currentDate())
            : q(calTable),
              calendarType("locale"),
              calendar(KGlobal::locale()->calendar()),
              displayEvents(true),
              displayHolidays(true),
              dataEngine(0),
              opacity(0.5)
        {
            svg = new Svg();
            svg->setImagePath("widgets/calendar");
            svg->setContainsMultipleImages(true);

            setDate(initialDate);
        }

        ~CalendarTablePrivate()
        {
            // Delete the old calendar first if it's not the global calendar
            if ( calendar != KGlobal::locale()->calendar() ) {
                delete calendar;
            }

            delete svg;
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
            q->populateHolidays();
            q->populateEvents();
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
            return dataEngine->query("holidaysDefaultRegion").value("holidaysDefaultRegion").toString();
        }

        CalendarTable *q;
        QString calendarType;
        const KCalendarSystem *calendar;

        QDate selectedDate;
        int selectedMonth;
        int selectedYear;
        int weekDayFirstOfSelectedMonth;
        int daysInWeek;
        int daysInSelectedMonth;
        int daysShownInPrevMonth;

        bool displayEvents;
        bool displayHolidays;
        QString holidaysRegion;
        Plasma::DataEngine *dataEngine;
        // Hash key: int = Julian Day number of holiday/event/todo, Data = details of holiday/event/todo
        QMultiHash<int, Plasma::DataEngine::Data> holidays;
        QMultiHash<int, Plasma::DataEngine::Data> events;
        QMultiHash<int, Plasma::DataEngine::Data> todos;
        QString eventsQuery;

        QPointF lastSeenMousePos;

        Ui::calendarConfig calendarConfigUi;

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
    
    if (oldYear != d->selectedYear || oldMonth != d->selectedMonth) {
        populateHolidays();
        populateEvents();
        update();
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

void CalendarTable::setDataEngine(Plasma::DataEngine *dataEngine)
{
    // JPL What happens to the old data engine, who cleans it up, do we need to delete first?
    if (d->dataEngine != dataEngine) {
        d->dataEngine = dataEngine;
        populateHolidays();
        populateEvents();
    }
}

const Plasma::DataEngine *CalendarTable::dataEngine() const
{
    return d->dataEngine;
}

void CalendarTable::setDisplayHolidays(bool showHolidays)
{
    if (showHolidays) {
        if (!dataEngine()) {
            clearHolidays();
            return;
        }

        if (holidaysRegion().isEmpty()) {
            setHolidaysRegion(d->defaultHolidaysRegion());
        }

        QString queryString = "holidaysIsValidRegion:" + holidaysRegion();
        if (!dataEngine()->query(queryString).value(queryString).toBool()) {
            return;
        }
    }

    if (d->displayHolidays != showHolidays) {
        d->displayHolidays = showHolidays;
        populateHolidays();
    }
}

bool CalendarTable::displayHolidays()
{
    return d->displayHolidays && !holidaysRegion().isEmpty();
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
        populateEvents();
    } else {
        if (d->dataEngine) {
            d->dataEngine->disconnectSource(d->eventsQuery, this);
        }
        d->events.clear();
        d->todos.clear();
    }
}

void CalendarTable::setHolidaysRegion(const QString &region)
{
    QString queryString = "holidaysIsValidRegion:" + region;
    if (!dataEngine()->query(queryString).value(queryString).toBool()) {
        return;
    }

    if (d->holidaysRegion != region) {
        d->holidaysRegion = region;
        populateHolidays();
    }
}

QString CalendarTable::holidaysRegion() const
{
    return d->holidaysRegion;
}

void CalendarTable::clearHolidays()
{
    d->holidays.clear();
}

void CalendarTable::addHoliday(const QDate &date,  Plasma::DataEngine::Data holidayData)
{
    d->holidays.insertMulti(date.toJulianDay(), holidayData);
}

bool CalendarTable::dateHasDetails(const QDate &date) const
{
    const int julian = date.toJulianDay();
    return d->holidays.contains(julian) || d->events.contains(julian) || d->todos.contains(julian);
}

QString CalendarTable::dateDetails(const QDate &date) const
{
    QString details;
    const int julian = date.toJulianDay();

    if (d->holidays.contains(julian)) {
        details += "<br>";

        foreach (Plasma::DataEngine::Data holidayData, d->holidays.values(julian)) {
            if (holidayData.value("observanceType").toString() == "PublicHoliday") {
                details += i18n("<i>Holiday</i>: %1", holidayData.value("name").toString());
                details += "<br>";
            }
        }

        foreach (Plasma::DataEngine::Data holidayData, d->holidays.values(julian)) {
            if (holidayData.value("observanceType").toString() == "Other") {
                //TODO add a type when strings not frozen
                details += holidayData.value("name").toString();
                details += "<br>";
            }
        }
    }

    if (d->events.contains(julian)) {
        details += "<br>";

        foreach (Plasma::DataEngine::Data eventData, d->events.values(julian)) {
            KDateTime startDate = KDateTime(eventData.value("StartDate").toDateTime());
            KDateTime endDate = KDateTime(eventData.value("EndDate").toDateTime());
            QTime startTime, endTime;
            if (startDate.date() < date) {
                startTime.setHMS(0, 0, 0);
            } else {
                startTime = startDate.time();
            }
            if (endDate.date() > date) {
                endTime.setHMS(23, 59, 59);
            } else {
                endTime = endDate.time();
            }
            //TODO translate this layout once strings not frozen
            QString description = QString("%1 - %2<br>%3").arg(KGlobal::locale()->formatTime(startTime))
                                                          .arg(KGlobal::locale()->formatTime(endTime))
                                                          .arg(eventData.value("Summary").toString());
            details += i18n("<i>Event</i>: %1<br>", description);
        }
    }

    if (d->todos.contains(julian)) {
        details += "<br>";

        foreach (Plasma::DataEngine::Data todoData, d->todos.values(julian)) {
            //TODO add Priority and Percentage Complete when strings not frozen
            QString description = QString("%1").arg(todoData.value("Summary").toString());
            details += i18n("<i>Todo</i>: %1<br>", description);
        }
    }

    return details;
}

void CalendarTable::populateHolidays()
{
    clearHolidays();

    if (!displayHolidays() || !dataEngine() || holidaysRegion().isEmpty()) {
        return;
    }

    // Just fetch the days displayed in the grid
    QDate startDate = d->dateFromRowColumn(0, 0);
    QDate endDate = d->dateFromRowColumn(DISPLAYED_WEEKS - 1, d->daysInWeek - 1);
    QString queryString = "holidays:" + holidaysRegion() + ':' + startDate.toString(Qt::ISODate)
                          + ':' + endDate.toString(Qt::ISODate);
    QList<QVariant> holidays = d->dataEngine->query(queryString).value(queryString).toList();

    QMutableListIterator<QVariant> i(holidays);
    while (i.hasNext()) {
        Plasma::DataEngine::Data holidayData = i.next().toHash();
        QDate holidayDate = QDate::fromString(holidayData.take("date").toString(), Qt::ISODate);
        addHoliday(holidayDate, holidayData);
    }
}

void CalendarTable::populateEvents()
{
    d->events.clear();
    d->todos.clear();

    if (!d->displayEvents || !d->dataEngine) {
        return;
    }

    //kDebug() << "WOOOHOOOO!";
    QDate queryDate = date();
    queryDate.setDate(queryDate.year(), queryDate.month(), 1);
    d->dataEngine->disconnectSource(d->eventsQuery, this);
    d->eventsQuery = "eventsInMonth:" + queryDate.toString(Qt::ISODate);
    d->dataEngine->connectSource(d->eventsQuery, this);
}

void CalendarTable::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)
    d->events.clear();
    d->todos.clear();
    foreach (const QVariant &v, data) {
        Plasma::DataEngine::Data pimData = v.toHash();
        QString type = pimData.take("Type").toString();
        if (type == "Todo") {
            d->todos.insert(pimData.value("TodoDueDate").toDateTime().date().toJulianDay(), pimData);
        } else if (type == "Event") {
            d->events.insert(pimData.value("StartDate").toDateTime().date().toJulianDay(), pimData);
        } // Ignore Journals
    }
    update();
}

void CalendarTable::applyConfiguration(KConfigGroup cg)
{
    setCalendar(cg.readEntry("calendarType", "locale"));
    setHolidaysRegion(cg.readEntry("holidaysRegion", d->defaultHolidaysRegion()));
    setDisplayHolidays(cg.readEntry("displayHolidays", true));
}

void CalendarTable::writeConfiguration(KConfigGroup cg)
{
    cg.writeEntry("calendarType", d->calendarType);
    cg.writeEntry("holidaysRegion", d->holidaysRegion);
    cg.writeEntry("displayHolidays", d->displayHolidays);
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

    Plasma::DataEngine::Data regions = dataEngine()->query("holidaysRegions");
    QMap<QString, QString> regionsMap;
    Plasma::DataEngine::DataIterator i(regions);
    while (i.hasNext()) {
        i.next();
        Plasma::DataEngine::Data regionData = i.value().toHash();
        QString name = regionData.value("name").toString();
        QString languageName = KGlobal::locale()->languageCodeToName(regionData.value("languageCode").toString());
        QString label;
        if (languageName.isEmpty()) {
            label = name;
        } else {
            // Need to get permission to break string freeze, in the meantime don't translate!
            //label = i18nc("Holday region, region language", "%1 (%2)", name, languageName);
            label = QString("%1 (%2)").arg(name).arg(languageName);
        }
        regionsMap.insert(label, i.key());
    }

    d->calendarConfigUi.regionComboBox->addItem(i18n("Do not show holidays"), QString());
    QMapIterator<QString, QString> j(regionsMap);
    while ( j.hasNext() ) {
        j.next();
        d->calendarConfigUi.regionComboBox->addItem(j.key(), QVariant(j.value()));
    }
    d->calendarConfigUi.regionComboBox->setCurrentIndex( d->calendarConfigUi.regionComboBox->findData( QVariant( d->holidaysRegion ) ) );
}

void CalendarTable::applyConfigurationInterface()
{
    setCalendar(d->calendarConfigUi.calendarComboBox->itemData(d->calendarConfigUi.calendarComboBox->currentIndex()).toString());
    setHolidaysRegion(d->calendarConfigUi.regionComboBox->itemData(d->calendarConfigUi.regionComboBox->currentIndex()).toString());
    setDisplayHolidays(!d->calendarConfigUi.regionComboBox->itemData(d->calendarConfigUi.regionComboBox->currentIndex()).toString().isEmpty());
}

void CalendarTable::configAccepted(KConfigGroup cg)
{
    applyConfigurationInterface();
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
    QFont font = Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    if (type & Event) {
        font.setBold(true);
    }
    font.setPixelSize(cellArea.height() * 0.7);
    p->setFont(font);
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
    QDate currentDate = QDate::currentDate(); //FIXME: calendar timezone

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

            if (cellDate == currentDate) {
                type |= CalendarTable::Today;
            }

            if (cellDate == date()) {
                type |= CalendarTable::Selected;
            }

            foreach (Plasma::DataEngine::Data holidayData, d->holidays.values(julian)) {
                if (holidayData.value("observanceType").toString() == "PublicHoliday") {
                    type |= CalendarTable::PublicHoliday;
                } else {
                    type |= CalendarTable::Holiday;
                }
            }

            if (d->events.contains(julian) || d->events.contains(julian)) {
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
                QFont font = Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
                font.setPixelSize(cellRect.height() * 0.7);
                p->setFont(font);
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
                QFontMetrics fontMetrics(font);
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
            QFont font = Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
            font.setPixelSize(d->headerHeight * 0.9);
            p->setFont(font);
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
