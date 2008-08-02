/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 Alex Merry <alex.merry@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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

#include "timeengine.h"

#include <QDate>
#include <QStringList>
#include <QTime>

#include <KLocale>
#include <KSystemTimeZones>
#include <KDateTime>

//timezone is defined in msvc
#ifdef timezone
#undef timezone
#endif

TimeEngine::TimeEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
{
    Q_UNUSED(args)
    setMinimumPollingInterval(333);

    // To have translated timezone names
    // (effectively a noop if the catalog is already present).
    KGlobal::locale()->insertCatalog("timezones4");
}

QStringList TimeEngine::sources() const
{
    QStringList timezones(KSystemTimeZones::zones().keys());
    timezones << "Local";
    return timezones;
}

bool TimeEngine::sourceRequestEvent(const QString &name)
{
    return updateSourceEvent(name);
}

bool TimeEngine::updateSourceEvent(const QString &tz)
{
    QString timezone;

    static const QString localName = I18N_NOOP("Local");
    if (tz == localName) {
        setData(localName, I18N_NOOP("Time"), QTime::currentTime());
        setData(localName, I18N_NOOP("Date"), QDate::currentDate());
        // this is relatively cheap - KSTZ::local() is cached
        timezone = KSystemTimeZones::local().name();
    } else {
        KTimeZone newTz = KSystemTimeZones::zone(tz);
        if (!newTz.isValid()) {
            return false;
        }

        KDateTime dt = KDateTime::currentDateTime(newTz);
        setData(tz, I18N_NOOP("Time"), dt.time());
        setData(tz, I18N_NOOP("Date"), dt.date());
        timezone = tz;
    }

    QString trTimezone = i18n(timezone.toUtf8());
    setData(tz, I18N_NOOP("Timezone"), trTimezone);
    QStringList tzParts = trTimezone.split("/");

    setData(tz, I18N_NOOP("Timezone Continent"), tzParts.value(0));
    setData(tz, I18N_NOOP("Timezone City"), tzParts.value(1));

    return true;
}


K_EXPORT_PLASMA_DATAENGINE(time, TimeEngine)

#include "timeengine.moc"
