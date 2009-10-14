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
#include <QtGui/QPainter>
#include <QtGui/QWidget>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QStyleOptionGraphicsItem>

//KDECore
#include <KGlobal>
#include <KDebug>
#include <KConfigDialog>
#include <KConfigGroup>

//Plasma
#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/DataEngine>

#include "ui_calendarConfig.h"

namespace Plasma
{

static const int DISPLAYED_WEEKS = 6;

static QString localeDateNum(int dtnum)
{
    return KGlobal::locale()->convertDigits(QString::number(dtnum), KGlobal::locale()->dateTimeDigitSet());
}

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

        CalendarTablePrivate(CalendarTable *, const QDate &initialDate = QDate::currentDate())
        {
            svg = new Svg();
            svg->setImagePath("widgets/calendar");
            svg->setContainsMultipleImages(true);

            calendarType = "locale";
            calendar = KGlobal::locale()->calendar();
            dataEngine = 0;
            displayHolidays = false;

            setDate(initialDate);

            opacity = 0.5; //transparency for the inactive text
        }

        ~CalendarTablePrivate()
        {
            // Delete the old calendar first if it's not the global calendar
            if ( calendar != KGlobal::locale()->calendar() ) {
                delete calendar;
            }

            delete svg;
        }

        void setDate(const QDate &setDate)
        {
            date = setDate;
            selectedMonth = calendar->month(date);
            selectedYear = calendar->year(date);
            weekDayFirstOfSelectedMonth = weekDayFirstOfMonth(date);
            daysInWeek = calendar->daysInWeek(date);
            daysInSelectedMonth = calendar->daysInMonth(date);
        }

        QRectF hoveredCellRect(CalendarTable *q, const QPointF &hoverPoint)
        {
            hoverDay = -1;
            hoverWeek = -1;

            if (hoverPoint.isNull()) {
                return QRectF();
            }

            if (hoverPoint.x() < centeringSpace + cellW + weekBarSpace) {
                // skip the weekbar
                return QRectF();
            }

            int x = (hoverPoint.x() - centeringSpace) / (cellW + cellSpace);
            int y = (hoverPoint.y() - headerHeight - headerSpace) / (cellH + cellSpace);

            if (x < 1 || x > daysInWeek || y < 0 || y > DISPLAYED_WEEKS) {
                return QRectF();
            }

            //FIXME: this should be a hint or something somewhere
            hoverDay = x - 1;
            hoverWeek = y;
            //kDebug () << x << y;
            return QRectF(q->cellX(x - 1) - glowRadius, q->cellY(y) - glowRadius,
                          cellW + glowRadius * 2, cellH + glowRadius * 2);
        }

        void updateHoveredPainting(CalendarTable *q, const QPointF &hoverPoint)
        {
            QRectF newHoverRect = hoveredCellRect(q, hoverPoint);

            // now update what is needed, and only what is needed!
            if (newHoverRect.isValid() && newHoverRect != hoverRect) {
                if (hoverRect.isValid()) {
                    q->update(hoverRect);
                }
                q->update(newHoverRect);
            }

            hoverRect = newHoverRect;
        }

        //Given a date, return the position in the table as an offset from the upper left cell
        int offsetFromDate(const QDate &cellDate)
        {
            int initialPosition = calendar->day(cellDate);
            int adjustment = (weekDayFirstOfSelectedMonth - calendar->weekStartDay() + daysInWeek) % daysInWeek;

            // make sure at least one day of the previous month is visible.
            // adjust this < 1 if more days should be forced visible:
            if (adjustment < 1) {
                adjustment += daysInWeek;
            }

            return initialPosition + adjustment;
        }

        //Given a position in the table as an offset from the upper left cell,
        //return the cell date.  Note can be an invalid date in the calendar system
        QDate dateFromOffset(int offset)
        {
            QDate cellDate;

            int adjustment = (weekDayFirstOfSelectedMonth - calendar->weekStartDay() + daysInWeek) % daysInWeek;

            // make sure at least one day of the previous month is visible.
            // adjust this < 1 if more days should be forced visible:
            if (adjustment < 1) {
                adjustment += daysInWeek;
            }

            if (calendar->setYMD( cellDate, selectedYear, selectedMonth, 1)) {
                cellDate = calendar->addDays(cellDate, offset - adjustment);
            } else {
                //If first of month is not valid, then that must be before earliestValid Date, so safe to assume next month ok
                if (calendar->setYMD(cellDate, selectedYear, selectedMonth + 1, 1)) {
                    cellDate = calendar->addDays(cellDate, offset - adjustment - daysInSelectedMonth);
                }
            }
            return cellDate;
        }


        // set weekday number of first day of this month, but this may not be a valid date so fake
        // it if needed e.g. in QDate Mon 1 Jan -4713 is not valid when it should be, so fake as day 1
        int weekDayFirstOfMonth(const QDate &cellDate)
        {
            Q_UNUSED(cellDate);
            QDate firstDayOfMonth;
            int weekDay;
            if ( calendar->setYMD(firstDayOfMonth, selectedYear, selectedMonth, 1)) {
                weekDay = calendar->dayOfWeek(firstDayOfMonth);
            } else {
                weekDay = calendar->dayOfWeek(date) - ((calendar->day(date) - 1) % daysInWeek);
                if (weekDay <= 0) {
                    weekDay += daysInWeek;
                }
            }
            return weekDay;
        }

        QString defaultHolidaysRegion()
        {
            QString tmpRegion = KGlobal::locale()->country();
            if (tmpRegion == "C") {
                // we assume 'C' means USA, which isn't the best of assumptions, really
                // the user ought to set their region in system settings, however
                tmpRegion = "us";
            }

            if (dataEngine && dataEngine->query("holidaysRegions").value("holidaysRegions").toStringList().contains(tmpRegion)) {
                return tmpRegion;
            }

            return QString();
        }

        QString calendarType;
        const KCalendarSystem *calendar;

        QDate date;
        int selectedMonth;
        int selectedYear;
        int weekDayFirstOfSelectedMonth;
        int daysInWeek;
        int daysInSelectedMonth;

        bool displayHolidays;
        QString holidaysRegion;
        Plasma::DataEngine *dataEngine;
        // Hash key: int = Julian Day number, QString = what's special
        QHash<int, QString> specialDates;

        Ui::calendarConfig calendarConfigUi;

        Plasma::Svg *svg;
        float opacity;
        QRectF hoverRect;
        int hoverWeek;
        int hoverDay;
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

bool CalendarTable::setCalendar(const QString &newCalendarType)
{
    if (newCalendarType == d->calendarType) {
        return true;
    }

    // If not the global calendar, delete the old calendar first
    if (d->calendar != KGlobal::locale()->calendar()) {
        delete d->calendar;
    }

    if (newCalendarType == "locale") {
        d->calendar = KGlobal::locale()->calendar();
        d->calendarType = "locale";
    } else {
        d->calendar = KCalendarSystem::create(newCalendarType);
        d->calendarType = calendar()->calendarType();
    }

    // Force date update to refresh cached date componants then update display
    d->setDate(date());
    d->updateHoveredPainting(this, QPointF());
    populateHolidays();
    update();

    // Signal out date change so any dependents will update as well
    emit dateChanged(date(), date());
    emit dateChanged(date());

    return true;
}

const KCalendarSystem *CalendarTable::calendar() const
{
    return d->calendar;
}

bool CalendarTable::setDate(const QDate &newDate)
{
    // New date must be valid in the current calendar system
    if (!calendar()->isValid(newDate)) {
        return false;
    }

    // If new date is the same as old date don't actually need to do anything
    if (newDate == date()) {
        return true;
    }

    int oldYear = d->selectedYear;
    int oldMonth = d->selectedMonth;
    //int oldDay = calendar()->day(date());
    QDate oldDate = date();

    // now change the date
    //kDebug( )<< "setting date to" << newDate;
    d->setDate(newDate);

    d->updateHoveredPainting(this, QPointF());

    if (oldYear != d->selectedYear || oldMonth != d->selectedMonth) {
        populateHolidays();
        update();
    } else {
        // only update the old and the new areas
        int offset = d->offsetFromDate(oldDate);
        int x = offset % d->daysInWeek;
        int y = offset / d->daysInWeek;
        update(cellX(x - 1) - d->glowRadius, cellY(y) - d->glowRadius,
               d->cellW + d->glowRadius * 2, d->cellH + d->glowRadius * 2);

        offset = d->offsetFromDate(newDate);
        x = offset % d->daysInWeek;
        y = offset / d->daysInWeek;
        update(cellX(x - 1) - d->glowRadius, cellY(y) - d->glowRadius,
               d->cellW + d->glowRadius * 2, d->cellH + d->glowRadius * 2);

    }

    emit dateChanged(newDate, oldDate);
    emit dateChanged(newDate);
    return true;
}

const QDate& CalendarTable::date() const
{
    return d->date;
}

void CalendarTable::setDataEngine(Plasma::DataEngine *dataEngine)
{
    // JPL What happens to the old data engine, who cleans it up, do we need to delete first?
    if (d->dataEngine != dataEngine) {
        d->dataEngine = dataEngine;
        populateHolidays();
    }
}

const Plasma::DataEngine *CalendarTable::dataEngine() const
{
    return d->dataEngine;
}

bool CalendarTable::setDisplayHolidays(bool showHolidays)
{
    if (showHolidays) {

        if (!dataEngine() ||
        !dataEngine()->query("holidaysRegions").value("holidaysRegions").toStringList().contains(holidaysRegion())) {
            return false;
        }

        if (holidaysRegion().isEmpty()) {
            setHolidaysRegion(d->defaultHolidaysRegion());
        }

    }

    if (d->displayHolidays != showHolidays) {
        d->displayHolidays = showHolidays;
        populateHolidays();
    }

    return true;
}

bool CalendarTable::displayHolidays()
{
    return d->displayHolidays && !holidaysRegion().isEmpty();
}

bool CalendarTable::setHolidaysRegion(const QString &region)
{
    if(!dataEngine()->query("holidaysRegions").value("holidaysRegions").toStringList().contains(region)){
        return false;
    }

    if (d->holidaysRegion != region) {
        d->holidaysRegion = region;
        populateHolidays();
    }
    return true;
}

QString CalendarTable::holidaysRegion() const
{
    return d->holidaysRegion;
}

void CalendarTable::clearDateProperties()
{
    d->specialDates.clear();
}

void CalendarTable::setDateProperty(QDate date, const QString &description)
{
    d->specialDates.insert(date.toJulianDay(), description);
}

QString CalendarTable::dateProperty(QDate date) const
{
    return d->specialDates.value(date.toJulianDay());
}

//JPL Looks to work OK even though non-Gregorian months do not match up, probably due to fetching 3 months worth
void CalendarTable::populateHolidays()
{
    clearDateProperties();

    if (!displayHolidays() || !dataEngine() || holidaysRegion().isEmpty()) {
        return;
    }

    QDate queryDate = date();
    QString prevMonthString = queryDate.addMonths(-1).toString(Qt::ISODate);
    QString nextMonthString = queryDate.addMonths(1).toString(Qt::ISODate);

    Plasma::DataEngine::Data prevMonth = d->dataEngine->query("holidaysInMonth:" + holidaysRegion() +
    ':' + prevMonthString);
    for (int i = -10; i < 0; i++) {
        QDate tempDate = queryDate.addDays(i);
        QString reason = prevMonth.value(tempDate.toString(Qt::ISODate)).toString();
        if (!reason.isEmpty()) {
            setDateProperty(tempDate, reason);
        }
    }

    queryDate.setDate(queryDate.year(), queryDate.month(), 1);
    Plasma::DataEngine::Data thisMonth = d->dataEngine->query("holidaysInMonth:" + holidaysRegion() +
    ':' + queryDate.toString(Qt::ISODate));
    int numDays = calendar()->daysInMonth(queryDate);
    for (int i = 0; i < numDays; i++) {
        QDate tempDate = queryDate.addDays(i);
        QString reason = thisMonth.value(tempDate.toString(Qt::ISODate)).toString();
        if (!reason.isEmpty()) {
            setDateProperty(tempDate, reason);
        }
    }

    queryDate = queryDate.addMonths(1);
    Plasma::DataEngine::Data nextMonth = d->dataEngine->query("holidaysInMonth:" + holidaysRegion() +
    ':' + nextMonthString);
    for (int i = 0; i < 10; i++) {
        QDate tempDate = queryDate.addDays(i);
        QString reason = nextMonth.value(tempDate.toString(Qt::ISODate)).toString();
        if (!reason.isEmpty()) {
            setDateProperty(tempDate, reason);
        }
    }
}

void CalendarTable::applyConfiguration(KConfigGroup cg)
{
    setCalendar(cg.readEntry("calendarType", "locale"));
    setHolidaysRegion(cg.readEntry("holidaysRegion", QString()));
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
    foreach ( QString cal, calendars ) {
        d->calendarConfigUi.calendarComboBox->addItem( KCalendarSystem::calendarLabel( cal ), QVariant( cal ) );
    }
    d->calendarConfigUi.calendarComboBox->setCurrentIndex( d->calendarConfigUi.calendarComboBox->findData( QVariant( d->calendarType ) ) );

    QStringList regions = dataEngine()->query("holidaysRegions").value("holidaysRegions").toStringList();
    QMap<QString, QString> regionMap;
    foreach ( QString region, regions ) {
        QString label = KGlobal::locale()->countryCodeToName( region );
        if (label.isEmpty()) {
            label = region;
        }
        regionMap.insert(label, region);
    }
    d->calendarConfigUi.regionComboBox->addItem(i18n("Do not show holidays"), QString());
    QMapIterator<QString, QString> i(regionMap);
    while (i.hasNext()) {
        i.next();
        d->calendarConfigUi.regionComboBox->addItem( i.key(), QVariant( i.value() ) );
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

//Returns the x co-ordinate for drawing the day cell on the widget given the day column
//Note dayColumn is 0-daysInWeek and is not a real weekDay number (i.e. NOT Monday=1).
int CalendarTable::cellX(int dayColumn)
{
    return boundingRect().x() + d->centeringSpace + d->weekBarSpace + d->cellW
            + ((d->cellW + d->cellSpace) * (dayColumn));
}

//Returns the y co-ordinate for drawing the day cell on the widget given the weekRow
//weekRow is 0-DISPLAYED_WEEKS
int CalendarTable::cellY(int weekRow)
{
    return (int) boundingRect().y() + ((d->cellH + d->cellSpace) * (weekRow))
            + d->headerHeight + d->headerSpace;
}

void CalendarTable::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    //bool changed = false;

    if (event->delta() < 0) {
        setDate(calendar()->addMonths(date(), 1));
    } else if (event->delta() > 0) {
        setDate(calendar()->addMonths(date(), -1));
        update();
    }
}

void CalendarTable::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
//JPL should be able to simplify this, check KDateTable
    event->accept();

    if ((event->pos().x() >= cellX(0)) && (event->pos().x() <= cellX(d->daysInWeek) - d->cellSpace) &&
        (event->pos().y() >= cellY(0)) && (event->pos().y() <= cellY(DISPLAYED_WEEKS) - d->cellSpace)){

        int week = -1;
        int weekDay = -1;

        for (int i = 0; i < d->daysInWeek; i++) {
            if ((event->pos().x() >= cellX(i)) && (event->pos().x() <= cellX(i + 1) - d->cellSpace))
                weekDay = i;
        }

        for (int i = 0; i < DISPLAYED_WEEKS; i++) {
            if ((event->pos().y() >= cellY(i)) && (event->pos().y() <= cellY(i + 1) - d->cellSpace))
                week = i;
        }

        if ((week >= 0) && (weekDay >= 0)) {
            d->hoverDay = -1;
            d->hoverWeek = -1;
            setDate(d->dateFromOffset((week * d->daysInWeek) + weekDay));
        }
    }
}

void CalendarTable::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    mousePressEvent(event);
}

void CalendarTable::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    emit tableClicked();
}

void CalendarTable::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    d->updateHoveredPainting(this, event->pos());
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

void CalendarTable::paintCell(QPainter *p, int cell, int week, int weekDay, CellTypes type, const QDate &cellDate)
{
    Q_UNUSED(cellDate);

    QString cellSuffix = type & NotInCurrentMonth ? "inactive" : "active";
    QRectF cellArea = QRectF(cellX(weekDay), cellY(week), d->cellW, d->cellH);

    d->svg->paint(p, cellArea, cellSuffix); // draw background

    QColor numberColor = Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    if (type & NotInCurrentMonth || type & InvalidDate) {
        p->setOpacity(d->opacity);
    }

    p->setPen(numberColor);
    QFont font = Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    font.setBold(true);
    font.setPixelSize(cellArea.height() * 0.7);
    p->setFont(font);
    if (!(type & InvalidDate)) {
        p->drawText(cellArea, Qt::AlignCenter, localeDateNum(cell), &cellArea); //draw number
    }
    p->setOpacity(1.0);
}

void CalendarTable::paintBorder(QPainter *p, int cell, int week, int weekDay, CellTypes type, const QDate &cellDate)
{
    Q_UNUSED(cell);
    Q_UNUSED(cellDate);

    if (type & Hovered) {
        d->svg->paint(p, QRect(cellX(weekDay), cellY(week), d->cellW, d->cellH), "hoverHighlight");
    }

    QString elementId;

    if (type & Today) {
        elementId = "today";
    } else if (type & Selected) {
        elementId = "selected";
    } else if (type & Holiday) {
        elementId = "red";
    } else {
        return;
    }

    d->svg->paint(p, QRectF(cellX(weekDay) - 1, cellY(week) - 1, d->cellW + 1, d->cellH + 2), elementId);
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

    //kDebug() << "exposed: " << option->exposedRect;
    for (int week = 0; week < DISPLAYED_WEEKS; week++) {
        for (int weekDay = 0; weekDay < d->daysInWeek; weekDay++) {
            int x = cellX(weekDay);
            int y = cellY(week);

            QRectF cellRect(x, y, d->cellW, d->cellH);
            if (!cellRect.intersects(option->exposedRect)) {
                continue;
            }

            QDate cellDate = d->dateFromOffset((week * d->daysInWeek) + weekDay);
            CalendarTable::CellTypes type(CalendarTable::NoType);
            // get cell info
            int cell = calendar()->day(cellDate);

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

            if (d->specialDates.contains(cellDate.toJulianDay())) {
                type |= CalendarTable::Holiday;
            }

            if (type != CalendarTable::NoType && type != CalendarTable::NotInCurrentMonth) {
                borders.append(CalendarCellBorder(cell, week, weekDay, type, cellDate));
            }

            if (week == d->hoverWeek && weekDay == d->hoverDay) {
                type |= CalendarTable::Hovered;
                hovers.append(CalendarCellBorder(cell, week, weekDay, type, cellDate));
            }

            paintCell(p, cell, week, weekDay, type, cellDate);

            if (weekDay == 0) {
                QRectF cellRect(r.x() + d->centeringSpace, y, d->cellW, d->cellH);
                p->setPen(Theme::defaultTheme()->color(Plasma::Theme::TextColor));
                QFont font = Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
                font.setPixelSize(cellRect.height() * 0.7);
                p->setFont(font);
                p->setOpacity(d->opacity);
                QString weekString;
                if (calendar()->isValid(cellDate)) {
                    weekString = localeDateNum(calendar()->weekNumber(cellDate));
                }
                if (cellDate.dayOfWeek() != Qt::Monday) {
                    weekString += '/';
                    QDate date(cellDate);
// JPL What's this 8?  Columns incl Week No?
                    date = date.addDays(8 - cellDate.dayOfWeek());
                    if (calendar()->isValid(cellDate)) {
                        weekString += localeDateNum(calendar()->weekNumber(date));
                    }
                }
                p->drawText(cellRect, Qt::AlignCenter, weekString); //draw number
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

    /*
    p->save();
    p->setPen(Qt::red);
    p->drawRect(option->exposedRect.adjusted(1, 1, -2, -2));
    p->restore();
    */
}

} //namespace Plasma

#include "calendartable.moc"
