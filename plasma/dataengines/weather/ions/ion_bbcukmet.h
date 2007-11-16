/***************************************************************************
 *   Copyright (C) 2007 by Shawn Starr <shawn.starr@rogers.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

/* Ion for BBC Weather from UKMET Office */

#ifndef _ION_UKMET_H
#define _ION_UKMET_H

#include <QtXml/QXmlStreamReader>
#include <QtCore/QStringList>
#include <QDebug>
#include <kurl.h>
#include <kio/job.h>
#include <kio/scheduler.h>
#include <kdemacros.h>
#include <plasma/dataengine.h>
#include "ion.h"
#include "weather_formula.h"

class WeatherData
{

public:
    QString title;

    // Current observation information.
    QString conditionTime;
    QString weather;
    QString observations;
};

class KDE_EXPORT UKMETIon : public IonInterface
{
    Q_OBJECT

public:
    UKMETIon(QObject *parent, const QVariantList &args);
    ~UKMETIon();
    void init();  // Setup the city location, fetching the correct URL name.
    void option(int option, QVariant value);
    bool updateIonSource(const QString& source);
    void updateWeather(const QString& source); // Sync data source with Applet

protected slots:
    void slotDataArrived(KIO::Job *, const QByteArray &);
    void slotJobFinished(KJob *);
    void forecast_slotDataArrived(KIO::Job *, const QByteArray &);
    void forecast_slotJobFinished(KJob *);

private:
    /* UKMET Methods - Internal for Ion */

    // Load and Parse the place search XML listings
    void validate(const QString& place, const QString& source);
    bool readSearchXMLData(const QString& source, QXmlStreamReader& xml);
    void parseSearchLocations(const QString& source, QXmlStreamReader& xml);

    // Observation parsing methods
    bool readObservationXMLData(QString& source, QXmlStreamReader& xml);
    void parsePlaceObservation(WeatherData& data, QXmlStreamReader& xml);
    void parseWeatherChannel(WeatherData& data, QXmlStreamReader& xml);
    void parseWeatherObservation(WeatherData& data, QXmlStreamReader& xml);
    void parseUnknownElement(QXmlStreamReader& xml);

private:
    class Private;
    Private *const d;
};

K_EXPORT_PLASMA_ION(bbcukmet, UKMETIon)

#endif
