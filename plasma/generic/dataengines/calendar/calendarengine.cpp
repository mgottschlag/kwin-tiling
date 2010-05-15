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


#include "calendarengine.h"

#include <QtCore/QDate>

#include <KCalendarSystem>
#include <KHolidays/Holidays>

#include <akonadi/changerecorder.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/kcal/incidencemimetypevisitor.h>

#include "calendarmodel.h"
#include "eventdatacontainer.h"

CalendarEngine::CalendarEngine(QObject* parent, const QVariantList& args)
: Plasma::DataEngine(parent)
, m_calendarModel(0)
{
    Q_UNUSED(args);
}

CalendarEngine::~CalendarEngine()
{
    qDeleteAll(m_regions);
}

bool CalendarEngine::sourceRequestEvent(const QString &name)
{
    kDebug() << name << '\n';
    const QStringList tokens = name.split(':');

    if (tokens.count() < 2) {
        if (name == "holidaysRegions") {
            setData(name, KHolidays::HolidayRegion::locations());
            return true;
        } else {
            return false;
        }
    }

    if (tokens.at(0) == "events" || tokens.at(0) == "eventsInMonth") {
        return akonadiCalendarSourceRequest(name, tokens);
    }

    if (tokens.count() < 3) {
        return false;
    }

    //kDebug() << tokens[0];
    //kDebug() << tokens[2];
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
                QString summary;
                foreach (const KHolidays::Holiday &holiday, holidays) {
                    if (holiday.dayType()==1) {
                        if (!summary.isEmpty()) {
                            summary.append("\n");
                        }

                        summary.append(holiday.text());
                }
            }

                data.insert(dateArg.toString(Qt::ISODate), summary);
            }

            dateArg = dateArg.addDays(1);
        }

        setData(name, data);
    } else if (tokens[0] == "isHoliday") {
        setData(name, region->isHoliday(dateArg));
    } else if (tokens[0] == "description") {
        KHolidays::Holiday::List holidays = region->holidays(dateArg);
        QString summary;
        foreach (const KHolidays::Holiday &holiday, holidays) {
            if (!summary.isEmpty()) {
                summary.append("\n");
            }

            summary.append(holiday.text());
        }

        setData(name, summary);
    }

    return true;
}

bool CalendarEngine::akonadiCalendarSourceRequest(const QString& name, const QStringList& tokens)
{
    // figure out what time range was requested from the source string
    QDate start;
    QDate end;
    if (tokens.at(0) == "eventsInMonth") {
        start = QDate::fromString(tokens.at(1), Qt::ISODate);
        start.setDate(start.year(), start.month(), 1);
        end = QDate(start.year(), start.month(), start.daysInMonth());
    } else if (tokens.size() == 2) {
        start = QDate::fromString(tokens.at(1), Qt::ISODate);
        end = start.addDays(1);
    } else {
        start = QDate::fromString(tokens.at(1), Qt::ISODate);
        end = QDate::fromString(tokens.at(2), Qt::ISODate);
    }

    // start akonadi etc if needed
    initAkonadiCalendar();
    kDebug( )<< "Calendar source for" << KDateTime(start) << KDateTime(end);

    // create the corresponding EventDataContainer
    addSource(new EventDataContainer(m_calendarModel, name, KDateTime(start), KDateTime(end)));
    return true;
}

void CalendarEngine::initAkonadiCalendar()
{
    if (m_calendarModel != 0) {
        // we have been initialized already
        return;
    }

    // ask for akonadi events
    Akonadi::ChangeRecorder* monitor = new Akonadi::ChangeRecorder(this);
    Akonadi::ItemFetchScope scope;
    scope.fetchFullPayload( true );
    scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();

    // setup what part of akonadi data we want (calendar incidences)
    monitor->setCollectionMonitored( Akonadi::Collection::root() );
    monitor->fetchCollection( true );
    monitor->setItemFetchScope( scope );
    monitor->setMimeTypeMonitored( QLatin1String("text/calendar"), true );// FIXME: this one should not be needed, in fact it might cause the inclusion of free/busy, notes or other unwanted stuff
    monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::eventMimeType(), true );
    monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::todoMimeType(), true );
    monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::journalMimeType(), true );

    // create the models that contain the data. they will be updated automatically from akonadi.
    m_calendarModel = new Akonadi::CalendarModel(monitor, this);
    m_calendarModel->setCollectionFetchStrategy(Akonadi::EntityTreeModel::InvisibleCollectionFetch);
}

#include "calendarengine.moc"
