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
#include <KDateTime>
#include <KSystemTimeZones>
#include <KHolidays/Holidays>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/Session>
#include <Akonadi/Collection>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include "akonadi/calendar.h"
#include "akonadi/calendarmodel.h"
#include "eventdatacontainer.h"

CalendarEngine::CalendarEngine(QObject* parent, const QVariantList& args)
              : Plasma::DataEngine(parent),
                m_calendar(0)
{
    Q_UNUSED(args);
}

CalendarEngine::~CalendarEngine()
{
    qDeleteAll(m_regions);
}

bool CalendarEngine::sourceRequestEvent(const QString &request)
{
    kDebug() << "Request = " << request << '\n';

    if (request.isEmpty()) {
        return false;
    }

    QStringList requestTokens = request.split(':');
    QString requestKey = requestTokens.takeFirst();

    if (requestKey == "holidaysRegions" ||
        requestKey == "holidaysDefaultRegion" ||
        requestKey == "holidaysIsValidRegion" ||
        requestKey == "holidays" ||
        requestKey == "holidaysInMonth") {
        return holidayCalendarSourceRequest(requestKey, requestTokens, request);
    }

    if (requestKey == "events" || requestKey == "eventsInMonth") {
        return akonadiCalendarSourceRequest(requestKey, requestTokens, request);
    }

    return false;
}

bool CalendarEngine::holidayCalendarSourceRequest(const QString& key, const QStringList& args, const QString& request)
{
    if (key == "holidaysRegions") {
        QStringList regionList = KHolidays::HolidayRegion::regionCodes();
        Plasma::DataEngine::Data data;
        foreach (const QString &regionCode, regionList) {
            Plasma::DataEngine::Data regionData;
            regionData.insert("name", KHolidays::HolidayRegion::name(regionCode));
            regionData.insert("description", KHolidays::HolidayRegion::description(regionCode));
            QString countryCode = KHolidays::HolidayRegion::countryCode(regionCode);
            regionData.insert("countryCode", countryCode);
            regionData.insert("location", countryCode.left(2));
            regionData.insert("languageCode", KHolidays::HolidayRegion::languageCode(regionCode));
            data.insert(regionCode, regionData);
        }
        setData(request, data);
        return true;
    }

    if (key == "holidaysDefaultRegion") {
        // If not set or the locale has changed since last set, then try determine a default region.
        // Try to match against the users country and language, or failing that the language country.
        // Scan through all the regions finding the first match for each possible default
        // Holiday Region Country Code can be a country subdivision or the country itself,
        // e.g. US or US-CA for California, so we can try match on both but an exact match has priority
        // The Holiday Region file is in one language only, so give priority to any file in the
        // users language, e.g. bilingual countries with a separate file for each language
        // Locale language can have a country code embedded in it e.g. en_GB, which we can try use if
        // no country set, but a lot of countries use en_GB so it's a lower priority option
        // Finally, default to US if available, but I don't think that's a great idea.
        if(m_defaultHolidayRegion.isEmpty() ||
            m_defaultHolidayRegionCountry != KGlobal::locale()->country() ||
            m_defaultHolidayRegionLanguage != KGlobal::locale()->language()) {

            m_defaultHolidayRegion = QString();
            m_defaultHolidayRegionCountry = KGlobal::locale()->country();
            m_defaultHolidayRegionLanguage = KGlobal::locale()->language();

            QString localeCountry = m_defaultHolidayRegionCountry.toLower();
            QString localeLanguage = m_defaultHolidayRegionLanguage.toLower();
            QString localeLanguageCountry;
            if (localeLanguage.split('_').count() > 1) {
                localeLanguageCountry = localeLanguage.split('_').at(1);
            }

            QString countryAndLanguageMatch, countryOnlyMatch, subdivisionAndLanguageMatch,
                    subdivisionOnlyMatch, languageCountryAndLanguageMatch, languageCountryOnlyMatch,
                    languageSubdivisionAndLanguageMatch, languageSubdivisionOnlyMatch, usaMatch;

             QStringList regionList = KHolidays::HolidayRegion::regionCodes();

            foreach (const QString &regionCode, regionList) {
                QString regionCountry = KHolidays::HolidayRegion::countryCode(regionCode).toLower();
                QString regionSubdivisionCountry;
                if (regionCountry.split('-').count() > 1) {
                    regionSubdivisionCountry = regionCountry.split('-').at(0);
                }
                QString regionLanguage = KHolidays::HolidayRegion::languageCode(regionCode).toLower();

                if (regionCountry == localeCountry && regionLanguage == localeLanguage) {
                    countryAndLanguageMatch = regionCode;
                    break; // exact match so don't look further
                } else if (regionCountry == localeCountry) {
                    if (countryOnlyMatch.isEmpty()) {
                        countryOnlyMatch = regionCode;
                    }
                } else if (!regionSubdivisionCountry.isEmpty() && regionSubdivisionCountry == localeCountry && regionLanguage == localeLanguage) {
                    if (subdivisionAndLanguageMatch.isEmpty()) {
                        subdivisionAndLanguageMatch = regionCode;
                    }
                } else if (!regionSubdivisionCountry.isEmpty() && regionSubdivisionCountry == localeCountry) {
                    if (subdivisionOnlyMatch.isEmpty()) {
                        subdivisionOnlyMatch = regionCode;
                    }
                } else if (!localeLanguageCountry.isEmpty() && regionCountry == localeLanguageCountry && regionLanguage == localeLanguage) {
                    if (languageCountryAndLanguageMatch.isEmpty()) {
                        languageCountryAndLanguageMatch = regionCode;
                    }
                } else if (!localeLanguageCountry.isEmpty() && regionCountry == localeLanguageCountry) {
                    if (languageCountryOnlyMatch.isEmpty()) {
                        languageCountryOnlyMatch = regionCode;
                    }
                } else if (!regionSubdivisionCountry.isEmpty() && !localeLanguageCountry.isEmpty() &&
                           regionSubdivisionCountry == localeLanguageCountry && regionLanguage == localeLanguage) {
                    if (languageSubdivisionAndLanguageMatch.isEmpty()) {
                        languageSubdivisionAndLanguageMatch = regionCode;
                    }
                } else if (!regionSubdivisionCountry.isEmpty() && !localeLanguageCountry.isEmpty() &&
                           regionSubdivisionCountry == localeLanguageCountry) {
                    if (languageSubdivisionOnlyMatch.isEmpty()) {
                        languageSubdivisionOnlyMatch = regionCode;
                    }
                }

                if (regionCountry == "us" && usaMatch.isEmpty()) {
                    usaMatch = regionCode;
                }
            }

            if (!countryAndLanguageMatch.isEmpty()) {
                m_defaultHolidayRegion = countryAndLanguageMatch;
            } else if (!countryOnlyMatch.isEmpty()) {
                m_defaultHolidayRegion = countryOnlyMatch;
            } else if (!subdivisionAndLanguageMatch.isEmpty()) {
                m_defaultHolidayRegion = subdivisionAndLanguageMatch;
            } else if (!subdivisionOnlyMatch.isEmpty()) {
                m_defaultHolidayRegion = subdivisionOnlyMatch;
            } else if (!languageCountryAndLanguageMatch.isEmpty()) {
                m_defaultHolidayRegion = languageCountryAndLanguageMatch;
            } else if (!languageCountryOnlyMatch.isEmpty()) {
                m_defaultHolidayRegion = languageCountryOnlyMatch;
            } else if (!languageSubdivisionAndLanguageMatch.isEmpty()) {
                m_defaultHolidayRegion = languageSubdivisionAndLanguageMatch;
            } else if (!languageSubdivisionOnlyMatch.isEmpty()) {
                m_defaultHolidayRegion = languageSubdivisionOnlyMatch;
            } else if (!usaMatch.isEmpty()) {
                m_defaultHolidayRegion = usaMatch;
            }

        }

        setData(request, m_defaultHolidayRegion);
        return true;
    }


    int argsCount = args.count();
    if (argsCount < 1) {
        return false;
    }

    const QString regionCode = args.at(0);

    if (key == "holidaysIsValidRegion") {
        if (m_regions.contains(regionCode)) {
            setData(request, m_regions.value(regionCode)->isValid());
        } else {
            setData(request, KHolidays::HolidayRegion::isValid(regionCode));
        }
        return true;
    }

    if (argsCount < 2) {
        return false;
    }

    KHolidays::HolidayRegion *region = m_regions.value(regionCode);
    if (!region || !region->isValid()) {
        region = new KHolidays::HolidayRegion(regionCode);
        if (region->isValid()) {
            m_regions.insert(regionCode, region);
        } else {
            delete region;
            return false;
        }
    }

    QDate dateArg = QDate::fromString(args.at(1), Qt::ISODate);
    if (!dateArg.isValid()) {
        return false;
    }

    if (key == "holidaysInMonth" || key == "holidays") {
        KHolidays::Holiday::List holidays;
        if (key == "holidays" && argsCount == 2) {
            holidays = region->holidays(dateArg);
        } else {
            QDate startDate, endDate;
            if (key == "holidaysInMonth") {
                int requestYear, requestMonth;
                KGlobal::locale()->calendar()->getDate(dateArg, &requestYear, &requestMonth, 0);
                int lastDay = KGlobal::locale()->calendar()->daysInMonth(dateArg);
                KGlobal::locale()->calendar()->setDate(startDate, requestYear, requestMonth, 1);
                KGlobal::locale()->calendar()->setDate(endDate, requestYear, requestMonth, lastDay);
            } else { // holidays
                if (argsCount < 3) {
                    return false;
                }
                startDate = dateArg;
                endDate = QDate::fromString(args.at(2), Qt::ISODate);
            }
            if (!startDate.isValid() || !endDate.isValid()) {
                return false;
            }
            holidays = region->holidays(startDate, endDate);
        }

        QList<QVariant> holidayList;
        foreach (const KHolidays::Holiday &holiday, holidays) {
            if (!holiday.text().isEmpty()) {
                Plasma::DataEngine::Data holidayData;
                holidayData.insert("date", holiday.date().toString(Qt::ISODate));
                holidayData.insert("name", holiday.text());
                // It's a blunt tool for now, we only know if it's a full public holiday or not
                if ( holiday.dayType() == KHolidays::Holiday::NonWorkday ) {
                    holidayData.insert("observanceType", "PublicHoliday");
                } else {
                    holidayData.insert("observanceType", "Other");
                }
                holidayList.append(QVariant(holidayData));
            }
        }

        setData(request, QVariant(holidayList));
        return true;
    }

    if (key == "isHoliday") {
        setData(request, region->isHoliday(dateArg));
        return true;
    }

    if (key == "description") {
        KHolidays::Holiday::List holidays = region->holidays(dateArg);
        QString summary;
        foreach (const KHolidays::Holiday &holiday, holidays) {
            if (!summary.isEmpty()) {
                summary.append("\n");
            }

            summary.append(holiday.text());
        }

        setData(request, summary);
        return true;
    }

    return false;
}

bool CalendarEngine::akonadiCalendarSourceRequest(const QString& key, const QStringList& args, const QString& request)
{
    // figure out what time range was requested from the source string
    QDate start;
    QDate end;
    if (key == "eventsInMonth") {
        if (args.count() < 1) {
            return false;
        }
        start = QDate::fromString(args.at(0), Qt::ISODate);
        start.setDate(start.year(), start.month(), 1);
        end = QDate(start.year(), start.month(), start.daysInMonth());
    } else if (key == "events") {
        if (args.count() == 1) {
            start = QDate::fromString(args.at(0), Qt::ISODate);
            end = start.addDays(1);
        } else {
            if (args.count() < 2) {
                return false;
            }
            start = QDate::fromString(args.at(0), Qt::ISODate);
            end = QDate::fromString(args.at(1), Qt::ISODate);
        }
    } else {
        return false;
    }

    if (!start.isValid() || !end.isValid()) {
        return false;
    }

    // start akonadi etc if needed
    initAkonadiCalendar();

    // create the corresponding EventDataContainer
    addSource(new EventDataContainer(m_calendar, request, KDateTime(start, QTime(0, 0, 0)), KDateTime(end, QTime(23, 59, 59))));
    return true;
}

void CalendarEngine::initAkonadiCalendar()
{
    if (m_calendar != 0) {
        // we have been initialized already
        return;
    }

    // ask for akonadi events
    Akonadi::Session *session = new Akonadi::Session("PlasmaCalendarEngine", this);
    Akonadi::ChangeRecorder* monitor = new Akonadi::ChangeRecorder(this);
    Akonadi::ItemFetchScope scope;
    scope.fetchFullPayload(true);
    scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();

    // setup what part of akonadi data we want (calendar incidences)
    monitor->setSession(session);
    monitor->setCollectionMonitored(Akonadi::Collection::root());
    monitor->fetchCollection(true);
    monitor->setItemFetchScope(scope);
    monitor->setMimeTypeMonitored(Akonadi::IncidenceMimeTypeVisitor::eventMimeType(), true);
    monitor->setMimeTypeMonitored(Akonadi::IncidenceMimeTypeVisitor::todoMimeType(), true);
    monitor->setMimeTypeMonitored(Akonadi::IncidenceMimeTypeVisitor::journalMimeType(), true);

    // create the models that contain the data. they will be updated automatically from akonadi.
    Akonadi::CalendarModel *calendarModel = new Akonadi::CalendarModel(monitor, this);
    calendarModel->setCollectionFetchStrategy(Akonadi::EntityTreeModel::InvisibleCollectionFetch);
    m_calendar = new Akonadi::Calendar(calendarModel, calendarModel, KSystemTimeZones::local());
}

#include "calendarengine.moc"
