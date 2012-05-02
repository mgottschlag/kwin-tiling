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

#ifndef PLASMA_CALENDARWIDGET_H
#define PLASMA_CALENDARWIDGET_H

#include <QtGui/QGraphicsWidget>

#include "plasmaclock_export.h"

#include <KCalendarSystem>
#include <Plasma/DataEngine>

class KConfigDialog;
class KConfigGroup;

namespace Plasma
{

class CalendarTablePrivate;
class DataEngine;

class PLASMACLOCK_EXPORT CalendarTable : public QGraphicsWidget
{
    Q_OBJECT

public:
    enum CellType { NoType = 0,
                    Today = 1,
                    Selected = 2,
                    Hovered = 4,
                    Holiday = 8,
                    NotInCurrentMonth = 16,
                    InvalidDate = 32,
                    Event = 64,
                    PublicHoliday = 128};
    Q_DECLARE_FLAGS(CellTypes, CellType)

    explicit CalendarTable(QGraphicsWidget *parent = 0);
    explicit CalendarTable(const QDate &, QGraphicsWidget *parent = 0);
    ~CalendarTable();

    void setCalendar(const QString &newCalendarType = "locale");
    void setCalendar(const KCalendarSystem *calendar);
    const KCalendarSystem *calendar () const;

    void setDate(const QDate &date);
    const QDate& date() const;

    void setDisplayEvents(bool display);
    bool displayEvents();

    void setDisplayHolidays(bool showHolidays);
    bool displayHolidays();

    void clearHolidaysRegions();
    void addHolidaysRegion(const QString &regionCode, bool daysOff);
    QStringList holidaysRegions() const;
    QStringList holidaysRegionsDaysOff() const;

    void clearHolidays();
    void addHoliday(Plasma::DataEngine::Data holidayData);
    bool dateHasDetails(const QDate &date) const;
    QStringList dateDetails(const QDate &date) const;

    void setAutomaticUpdateEnabled(bool enabled);
    bool isAutomaticUpdateEnabled() const;

    void setCurrentDate(const QDate &date);
    const QDate& currentDate() const;

    QDate startDate() const;
    QDate endDate() const;

    void applyConfiguration(KConfigGroup cg);
    void writeConfiguration(KConfigGroup cg);
    void createConfigurationInterface(KConfigDialog *parent);
    void configAccepted(KConfigGroup cg);

Q_SIGNALS:
    void dateChanged(const QDate &newDate, const QDate &oldDate);
    void dateChanged(const QDate &newDate);
    void dateSelected(const QDate &date);
    void dateHovered(const QDate &date);
    void tableClicked();
    void eventsChanged();

public Q_SLOTS:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

protected:
    int cellX(int weekDay);
    int cellY(int week);

    void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void wheelEvent(QGraphicsSceneWheelEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent * event);

    virtual void paintCell(QPainter *p, int cell, int week, int weekDay, CellTypes type, const QDate &cellDate);
    virtual void paintBorder(QPainter *p, int cell, int week, int weekDay, CellTypes type, const QDate &cellDate);

private:
    QString buildOccurrenceDescription(const Plasma::DataEngine::Data &occurrence) const;

    friend class CalendarTablePrivate;
    CalendarTablePrivate* const d;

    Q_PRIVATE_SLOT(d, void populateCalendar())
    Q_PRIVATE_SLOT(d, void settingsChanged(int category))
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::CalendarTable::CellTypes)

#endif
