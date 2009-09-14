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

#include "timesource.h"

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

TimeEngine::~TimeEngine()
{
}

void TimeEngine::init()
{
    //QDBusInterface *ktimezoned = new QDBusInterface("org.kde.kded", "/modules/ktimezoned", "org.kde.KTimeZoned");
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect(QString(), QString(), "org.kde.KTimeZoned", "configChanged", this, SLOT(tzConfigChanged()));
}

void TimeEngine::tzConfigChanged()
{
    TimeSource *s = qobject_cast<TimeSource *>(containerForSource("Local"));

    if (s) {
        s->setTimeZone("Local");
    }

    updateAllSources();
}

QStringList TimeEngine::sources() const
{
    QStringList timezones(KSystemTimeZones::zones().keys());
    timezones << "Local";
    return timezones;
}

bool TimeEngine::sourceRequestEvent(const QString &name)
{
    addSource(new TimeSource(name, this));
    return true;
}

bool TimeEngine::updateSourceEvent(const QString &tz)
{
    TimeSource *s = qobject_cast<TimeSource *>(containerForSource(tz));

    if (s) {
        s->updateTime();
        scheduleSourcesUpdated();
        return true;
    }

    return false;
}

K_EXPORT_PLASMA_DATAENGINE(time, TimeEngine)

#include "timeengine.moc"
