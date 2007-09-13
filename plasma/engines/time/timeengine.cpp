/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
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
#include <QTime>
#include <QTimer>

#include <KDebug>
#include <KLocale>
#include <KSystemTimeZones>
#include <KDateTime>

#include "plasma/datacontainer.h"

TimeEngine::TimeEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args)
    setMinimumUpdateInterval(333);
}

TimeEngine::~TimeEngine()
{
}

bool TimeEngine::sourceRequested(const QString &name)
{
    //kDebug() << "TimeEngine::sourceRequested " << name;
    if (name == "Local") {
        setData(I18N_NOOP("Local"), I18N_NOOP("Time"), QTime::currentTime());
        setData(I18N_NOOP("Local"), I18N_NOOP("Date"), QDate::currentDate());

        return true;
    }

    KTimeZone newTz  = KSystemTimeZones::zone(name);
    if (!newTz.isValid()) {
        return false;
    }

    KDateTime dt = KDateTime::currentDateTime(newTz);
    setData(name, I18N_NOOP("Time"), dt.time());
    setData(name, I18N_NOOP("Date"), dt.date());

    return true;
}

bool TimeEngine::updateSource(const QString &tz)
{
    //kDebug() << "TimeEngine::updateTime()";

    QDateTime dt = QDateTime::currentDateTime();
    KTimeZone local = KSystemTimeZones::local();
    static const QString localName = I18N_NOOP("Local");
    if (tz == localName) {
        setData(tz, I18N_NOOP("Time"), dt.time());
        setData(tz, I18N_NOOP("Date"), dt.date());
    } else {
        KTimeZone newTz = KSystemTimeZones::zone(tz);
        QDateTime localizeDt = local.convert(newTz, dt);
        setData(tz, I18N_NOOP("Time"), localizeDt.time());
        setData(tz, I18N_NOOP("Date"), localizeDt.date());
    }

    return true;
}

#include "timeengine.moc"
