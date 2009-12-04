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

#ifndef PLASMA_CALENDARWIDGET_H
#define PLASMA_CALENDARWIDGET_H

#include <QtGui/QGraphicsWidget>

#include "plasmaclock_export.h"

#include <KDE/KCalendarSystem>

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
                    InvalidDate = 32 };
    Q_DECLARE_FLAGS(CellTypes, CellType)

    explicit CalendarTable(QGraphicsWidget *parent = 0);
    explicit CalendarTable(const QDate &, QGraphicsWidget *parent = 0);
    ~CalendarTable();

    bool setCalendar(const QString &newCalendarType = "locale");
    bool setCalendar(const KCalendarSystem *calendar);
    const KCalendarSystem *calendar () const;

    bool setDate(const QDate &date);
    const QDate& date() const;

    void setDataEngine(Plasma::DataEngine *dataEngine);
    const Plasma::DataEngine *dataEngine() const;

    bool setDisplayHolidays(bool showHolidays);
    bool displayHolidays();

    bool setHolidaysRegion(const QString &region);
    QString holidaysRegion() const;

    void clearDateProperties();
    void setDateProperty(QDate date, const QString &reason);
    QString dateProperty(QDate date) const;

    void applyConfiguration(KConfigGroup cg);
    void writeConfiguration(KConfigGroup cg);
    void createConfigurationInterface(KConfigDialog *parent);
    void applyConfigurationInterface();
    void configAccepted(KConfigGroup cg);

Q_SIGNALS:
    void dateChanged(const QDate &newDate, const QDate &oldDate);
    void dateChanged(const QDate &newDate);
    void tableClicked();

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
    void populateHolidays();
    friend class CalendarTablePrivate;
    CalendarTablePrivate* const d;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::CalendarTable::CellTypes)

#endif
