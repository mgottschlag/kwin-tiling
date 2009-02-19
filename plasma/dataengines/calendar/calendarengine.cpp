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

#include <kholidays/holidays.h>

CalendarEngine::CalendarEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args);
}

CalendarEngine::~CalendarEngine()
{
}

bool CalendarEngine::sourceRequestEvent(const QString &name)
{
    kDebug() << name << "\n";
    QStringList tokens = name.split(":");

    if (tokens.count() < 3) {
        return false;
    }

    kDebug() << tokens[0] << "\n";
    kDebug() << tokens[2] << "\n";
    KHolidays::HolidayRegion region(tokens[1]);
    QDate dateArg = QDate::fromString(tokens[2], Qt::ISODate);

    if (tokens[0] == "isHoliday") {
        setData(name, region.isHoliday(dateArg));
    } else if (tokens[0] == "description") {
        KHolidays::Holiday::List holidays = region.holidays(dateArg);
        if (holidays.size() > 0){
            setData(name, holidays[0].text());
        } else {
            setData(name, QString());
        }
    }

    return true;
}

#include "calendarengine.moc"
