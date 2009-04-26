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
#include <QDBusConnection>
#include <QStringList>
#include <QTime>

#include <KLocale>
#include <KSystemTimeZones>
#include <KDateTime>

#include "moonphase.h"
#include "solarposition.h"

//timezone is defined in msvc
#ifdef timezone
#undef timezone
#endif

TimeEngine::TimeEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
    , solarPosition(0)
{
    Q_UNUSED(args)
    setMinimumPollingInterval(333);

    // To have translated timezone names
    // (effectively a noop if the catalog is already present).
    KGlobal::locale()->insertCatalog("timezones4");
}

TimeEngine::~TimeEngine()
{
    delete solarPosition;
}

void TimeEngine::init()
{
    //QDBusInterface *ktimezoned = new QDBusInterface("org.kde.kded", "/modules/ktimezoned", "org.kde.KTimeZoned");
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect(QString(), QString(), "org.kde.KTimeZoned", "configChanged", this, SLOT(updateAllSources()));
}

QStringList TimeEngine::sources() const
{
    QStringList timezones(KSystemTimeZones::zones().keys());
    timezones << "Local";
    return timezones;
}

QString TimeEngine::parse(const QString &tz, QHash<QString, QString>* args)
{
    if (tz.indexOf('|') > 0) {
        QStringList list = tz.split('|', QString::SkipEmptyParts);

        for (int i = 1; i < list.size(); ++i) {
            const QString arg = list[i];
            int n = arg.indexOf('=');
            if (n >= 0) {
                args->insert(arg.mid(0, n), arg.mid(n + 1));
            } else {
                args->insert(arg, QString());
            }
        }
        return list[0];
    }
    return tz;
}

bool TimeEngine::sourceRequestEvent(const QString &name)
{
    return updateSourceEvent(name);
}

bool TimeEngine::updateSourceEvent(const QString &tz)
{
    QHash<QString, QString> args;
    QString timezone = parse(tz, &args);
    DataEngine::Data data;

    static const QString localName = I18N_NOOP("Local");
    if (timezone == localName) {
        data[I18N_NOOP("Time")] = QTime::currentTime();
        data[I18N_NOOP("Date")] = QDate::currentDate();
        data[I18N_NOOP("Offset")] = KSystemTimeZones::local().currentOffset();
        // this is relatively cheap - KSTZ::local() is cached
        timezone = KSystemTimeZones::local().name();
    } else {
        KTimeZone newTz = KSystemTimeZones::zone(tz);
        if (!newTz.isValid()) {
            return false;
        }
        KDateTime dt = KDateTime::currentDateTime(newTz);
        data[I18N_NOOP("Time")] = dt.time();
        data[I18N_NOOP("Date")] = dt.date();
        data[I18N_NOOP("Offset")] = newTz.currentOffset();
    }

    QString trTimezone = i18n(timezone.toUtf8());
    data[I18N_NOOP("Timezone")] = trTimezone;
    
    QStringList tzParts = trTimezone.split("/");
    data[I18N_NOOP("Timezone Continent")] = tzParts.value(0);
    data[I18N_NOOP("Timezone City")] = tzParts.value(1);

    if (args.count() > 0) {
        static const QString latitude = I18N_NOOP("Latitude");
        static const QString longitude = I18N_NOOP("Longitude");
        
        data[latitude] = args[latitude].toDouble();
        data[longitude] = args[longitude].toDouble();
        QDateTime dt = QDateTime::fromString(args[I18N_NOOP("DateTime")], Qt::ISODate);
        if (dt.date().isValid()) {
            data[I18N_NOOP("Date")] = dt.date();
        }
        if (dt.time().isValid()) {
            data[I18N_NOOP("Time")] = dt.time();
        }
        if (args.contains(I18N_NOOP("Solar"))) {
            if (!solarPosition) {
                solarPosition = new SolarPosition;
            }
            solarPosition->appendData(data);
        }
        if (args.contains(I18N_NOOP("Moon"))) {
            appendMoonphase(data);
        }
    }
    setData(tz, data);
    return true;
}


K_EXPORT_PLASMA_DATAENGINE(time, TimeEngine)

#include "timeengine.moc"
