/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *
 *   Moon Phase:
 *   Copyright 1998,2000  Stephan Kulow <coolo@kde.org>
 *   Copyright 2009 by Davide Bettio <davide.bettio@kdemail.net>
 *
 *   Solar position:
 *   Copyright (C) 2009 Petri Damsten <damu@iki.fi>
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

#include "timesource.h"

#include <QDateTime>

#include <KDebug>
#include <KSystemTimeZones>

#include "solarposition.h"

//timezone is defined in msvc
#ifdef timezone
#undef timezone
#endif

TimeSource::TimeSource(const QString &name, QObject *parent)
    : Plasma::DataContainer(parent),
      m_offset(0),
      m_latitude(0),
      m_longitude(0),
      m_moonPhase(false),
      m_solarPosition(false)
{
    setObjectName(name);
    QString timezone = parseName(name);

    if (timezone == I18N_NOOP("Local")) { 
        m_tz = KSystemTimeZones::local();
        timezone = m_tz.name();
    } else {
        m_tz = KSystemTimeZones::zone(timezone);

        if (!m_tz.isValid()) {
            m_tz = KSystemTimeZones::local();
        }
    }

    QString trTimezone = i18n(timezone.toUtf8());
    setData(I18N_NOOP("Timezone"), trTimezone);

    QStringList tzParts = trTimezone.split("/", QString::SkipEmptyParts);
    if (tzParts.count() == 1) {
        // no '/' so just set it as the city
        setData(I18N_NOOP("Timezone City"), trTimezone);
    } else {
        setData(I18N_NOOP("Timezone Continent"), tzParts.value(0));
        setData(I18N_NOOP("Timezone City"), tzParts.value(1));
    }

    updateTime();
}

void TimeSource::updateTime()
{
    bool updateDailies = false;
    QDateTime dt = KDateTime::currentDateTime(m_tz).dateTime();

    if (m_solarPosition || m_moonPhase) {
        QDate prev = data()["Date"].toDate();
        updateDailies = prev != dt.date();
    }

    setData(I18N_NOOP("Time"), dt.time());
    setData(I18N_NOOP("Date"), dt.date());

    int offset = m_tz.currentOffset();
    if (m_offset != offset) {
        m_offset = offset;
        setData(I18N_NOOP("Offset"), m_offset);
    }

    if (m_solarPosition) {
        addSolarPositionData(dt);

        if (updateDailies) {
            addDailySolarPositionData(dt);
        }
    }

    if (m_moonPhase && updateDailies) {
        addMoonPhaseData(dt);
    }
}

QString TimeSource::parseName(const QString &name)
{
    if (!name.contains('|')) {
        // the simple case where it's just a timezone request
        return name;
    }

    // the various keys we recognize
    static const QString latitude = I18N_NOOP("Latitude");
    static const QString longitude = I18N_NOOP("Longitude");
    static const QString solar = I18N_NOOP("Solar");
    static const QString moon = I18N_NOOP("Moon");
    static const QString datetime = I18N_NOOP("DataTime");

    // now parse out what we got handed in
    QStringList list = name.split('|', QString::SkipEmptyParts);

    for (int i = 1; i < list.size(); ++i) {
        const QString arg = list[i];
        int n = arg.indexOf('=');
        if (n != -1) {
            QString key = arg.mid(0, n);
            QString value = arg.mid(n + 1);

            if (key == latitude) {
                m_latitude = value.toDouble();
            } else if (key == longitude) {
                m_longitude = value.toDouble();
            } else if (arg == datetime) {
                QDateTime dt = QDateTime::fromString(value, Qt::ISODate);

                if (dt.isValid()) {
                    setData(I18N_NOOP("Date"), dt.date());
                    setData(I18N_NOOP("Time"), dt.time());
                }
            }
        } else if (arg == solar) {
            m_solarPosition = true;
        } else if (arg == moon) {
            m_moonPhase = true;
        }
    }

    // timezone is first item ...
    return list.at(0);
}


// Moon phase support
time_t JDtoDate(double jd, struct tm *event_date);
double DatetoJD(struct tm *event_date);
double moonphasebylunation(int lun, int phi);
double moonphasebylunation(int lun, int phi);

void TimeSource::addMoonPhaseData(const QDateTime &dt)
{
    time_t time = dt.toTime_t();
    int counter;

    uint lun = 0;
    time_t last_new = 0;
    time_t next_new = 0;

    do {
        double JDE = moonphasebylunation(lun, 0);
        last_new = next_new;
        next_new = JDtoDate(JDE, 0);
        lun++;
    } while (next_new < time);

    lun -= 2;

    QDateTime ln;
    ln.setTime_t(last_new);
    //kDebug() << "last new " << KGlobal::locale()->formatDateTime(ln);

    time_t first_quarter = JDtoDate(moonphasebylunation(lun, 1), 0);
    QDateTime fq;
    fq.setTime_t(first_quarter);
    //kDebug() << "first quarter " << KGlobal::locale()->formatDateTime(fq);

    time_t full_moon = JDtoDate(moonphasebylunation(lun, 2), 0);
    QDateTime fm;
    fm.setTime_t(full_moon);
    //kDebug() << "full moon " << KGlobal::locale()->formatDateTime(fm);

    time_t third_quarter = JDtoDate(moonphasebylunation(lun, 3), 0);
    QDateTime tq;
    tq.setTime_t(third_quarter);
    //kDebug() << "third quarter " << KGlobal::locale()->formatDateTime(tq);

    QDateTime nn;
    nn.setTime_t(next_new);
    //kDebug() << "next new " << KGlobal::locale()->formatDateTime(nn);

    counter = ln.daysTo(dt);
    //kDebug() << "counter " << counter << " " << fm.daysTo(now);

    if (fm.daysTo(dt) == 0) {
        counter = 14;
        ///toolTipData.setMainText(i18n("Full Moon"));
        return;
    } else if (counter <= 15 && counter >= 13) {
        counter = 14 + fm.daysTo(dt);
        //kDebug() << "around full moon " << counter;
    }

    int diff = fq.daysTo(dt);
    if (diff  == 0) {
        counter = 7;
    } else if (counter <= 8 && counter >= 6) {
        counter = 7 + diff;
        //kDebug() << "around first quarter " << counter;
    }

    diff = ln.daysTo(dt);
    if (diff == 0) {
        counter = 0;
    } else if (counter <= 1 || counter >= 28) {
        counter = (29 + diff) % 29;
        diff = -nn.daysTo(dt);
        if (diff == 0) {
            counter = 0;
        } else if (diff < 3) {
            counter = 29 - diff;
        }
        //kDebug() << "around new " << counter << " " << diff;
    }

    if (tq.daysTo(dt) == 0) {
        counter = 21;
    } else if (counter <= 22 && counter >= 20) {
        counter = 21 + tq.daysTo(dt);
        //kDebug() << "around third quarter " << counter;
    }

    //kDebug() << "counter " << counter;
    setData("MoonPhase", counter);
}

void TimeSource::addSolarPositionData(const QDateTime &dt)
{
    //QTime time;
    //time.start();
    double zone = m_offset / -3600.0;

    double jd;
    double century;
    double eqTime;
    double solarDec;
    double azimuth;
    double zenith;

    NOAASolarCalc::calc(dt, m_longitude, m_latitude, zone, &jd, &century, &eqTime,
                        &solarDec, &azimuth, &zenith);
    setData("Equation of Time", eqTime);
    setData("Solar Declination", solarDec);
    setData("Azimuth", azimuth);
    setData("Zenith", zenith);
    setData("Corrected Elevation", NOAASolarCalc::calcElevation(zenith));
}

void TimeSource::addDailySolarPositionData(const QDateTime &dt)
{
    double jd;
    double century;
    double eqTime;
    double solarDec;
    double azimuth;
    double zenith;
    double minutes;
    double zone = m_offset / -3600.0;

    NOAASolarCalc::calc(dt, m_longitude, m_latitude, zone, &jd, &century, &eqTime,
                        &solarDec, &azimuth, &zenith);

    double jd2 = jd;
    NOAASolarCalc::calcTimeUTC(90.833, true, &jd2, &minutes, m_latitude, m_longitude);
    setData("Apparent Sunrise", NOAASolarCalc::calcDateFromJD(jd2, minutes, zone));

    jd2 = jd;
    NOAASolarCalc::calcTimeUTC(90.833, false, &jd2, &minutes, m_latitude, m_longitude);
    setData("Apparent Sunset", NOAASolarCalc::calcDateFromJD(jd2, minutes, zone));

    jd2 = jd;
    NOAASolarCalc::calcTimeUTC(90.0, true, &jd2, &minutes, m_latitude, m_longitude);
    setData("Sunrise", NOAASolarCalc::calcDateFromJD(jd2, minutes, zone));
    jd2 = jd;
    NOAASolarCalc::calcTimeUTC(90.0, false, &jd2, &minutes, m_latitude, m_longitude);
    setData("Sunset", NOAASolarCalc::calcDateFromJD(jd2, minutes, zone));

    jd2 = jd;
    NOAASolarCalc::calcTimeUTC(96.0, true, &jd2, &minutes, m_latitude, m_longitude);
    setData("Civil Dawn", NOAASolarCalc::calcDateFromJD(jd2, minutes, zone));
    jd2 = jd;
    NOAASolarCalc::calcTimeUTC(96.0, false, &jd2, &minutes, m_latitude, m_longitude);
    setData("Civil Dusk", NOAASolarCalc::calcDateFromJD(jd2, minutes, zone));

    jd2 = jd;
    NOAASolarCalc::calcTimeUTC(102.0, true, &jd2, &minutes, m_latitude, m_longitude);
    setData("Nautical Dawn", NOAASolarCalc::calcDateFromJD(jd2, minutes, zone));
    jd2 = jd;
    NOAASolarCalc::calcTimeUTC(102.0, false, &jd2, &minutes, m_latitude, m_longitude);
    setData("Nautical Dusk", NOAASolarCalc::calcDateFromJD(jd2, minutes, zone));

    jd2 = jd;
    NOAASolarCalc::calcTimeUTC(108.0, true, &jd2, &minutes, m_latitude, m_longitude);
    setData("Astronomical Dawn", NOAASolarCalc::calcDateFromJD(jd2, minutes, zone));
    jd2 = jd;
    NOAASolarCalc::calcTimeUTC(108.0, false, &jd2, &minutes, m_latitude, m_longitude);
    setData("Astronomical Dusk", NOAASolarCalc::calcDateFromJD(jd2, minutes, zone));

    century = NOAASolarCalc::calcTimeJulianCent(jd);
    minutes = NOAASolarCalc::calcSolNoonUTC(century, m_longitude);
    QDateTime dtFromJD = NOAASolarCalc::calcDateFromJD(jd, minutes, zone);
    NOAASolarCalc::calc(dtFromJD, m_longitude, m_latitude, zone, &jd, &century, &eqTime,
                        &solarDec, &azimuth, &zenith);
    setData("Solar Noon", dtFromJD);
    setData("Min Zenith", zenith);
    setData("Max Corrected Elevation", NOAASolarCalc::calcElevation(zenith));
}

