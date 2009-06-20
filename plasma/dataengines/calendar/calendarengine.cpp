/*
    Copyright (c) 2009 Davide Bettio <davide.bettio@kdemail.net>

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


#include "calendarengine.h"

#include <QDate>

#include <KCalendarSystem>
#include <KHolidays/Holidays>

CalendarEngine::CalendarEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args);
}

CalendarEngine::~CalendarEngine()
{
    qDeleteAll(m_regions);
}

bool CalendarEngine::sourceRequestEvent(const QString &name)
{
    kDebug() << name << "\n";
    const QStringList tokens = name.split(":");

    if (tokens.count() < 3) {
        if (name == "holidaysRegions") {
            setData(name, KHolidays::HolidayRegion::locations());
            return true;
        } else {
            return false;
        }
    }

    kDebug() << tokens[0];
    kDebug() << tokens[2];
    const QString regionName = tokens[1];
    KHolidays::HolidayRegion *region = m_regions.value(regionName);

    if (!region) {
        region = new KHolidays::HolidayRegion(regionName);
        m_regions.insert(regionName, region);
    }

    QDate dateArg = QDate::fromString(tokens[2], Qt::ISODate);

    if (tokens[0] == "holidaysInMonth") {
        Plasma::DataEngine::Data data;
        const int days = KGlobal::locale()->calendar()->daysInMonth(dateArg);
        dateArg.setDate(dateArg.year(), dateArg.month(), 1);

        for (int i = 0; i < days; ++i) {
            KHolidays::Holiday::List holidays = region->holidays(dateArg);

            if (!holidays.isEmpty()) {
                //FIXME: should show ALL holidays, not just the first
                data.insert(dateArg.toString(Qt::ISODate), holidays[0].text());
            }

            dateArg = dateArg.addDays(1);
        }

        setData(name, data);
    } else if (tokens[0] == "isHoliday") {
        setData(name, region->isHoliday(dateArg));
    } else if (tokens[0] == "description") {
        KHolidays::Holiday::List holidays = region->holidays(dateArg);
        if (!holidays.isEmpty()) {
            setData(name, holidays[0].text());
        } else {
            setData(name, QString());
        }
    }

    return true;
}

#include "calendarengine.moc"
