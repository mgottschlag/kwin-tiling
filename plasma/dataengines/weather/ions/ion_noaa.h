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

/* Ion for NOAA's National Weather Service XML data */

#ifndef _ION_NOAA_H
#define _ION_NOAA_H

#include <QtXml/QXmlStreamReader>
#include <QtCore/QStringList>
#include <QDebug>
#include <kurl.h>
#include <kio/job.h>
#include <kio/scheduler.h>
#include <kdemacros.h>
#include <plasma/dataengine.h>
#include "ion.h"
#include "formulas.h"

class WeatherData
{

public:
    //QString countryName; // USA
    QString locationName;
    QString stationID;
    QString stateName;

    // Current observation information.
    QString observationTime;
    QString weather;
    QString temperature_F;
    QString temperature_C;
    QString humidity;
    QString windString;
    QString windDirection;
    QString windSpeed; // Float value
    QString windGust; // Float value
    QString pressure;
    QString dewpoint_F;
    QString dewpoint_C;
    QString heatindex_F;
    QString heatindex_C;
    QString windchill_F;
    QString windchill_C;
    QString visibility;
};

class KDE_EXPORT NOAAIon : public IonInterface
{
    Q_OBJECT

public:
    NOAAIon(QObject *parent, const QVariantList &args);
    ~NOAAIon();
    void init(void);  // Setup the city location, fetching the correct URL name.
    bool metricUnit(void);
    bool timezone(void);
    void setMeasureUnit(const QString& unit);
    void setTimezoneFormat(const QString& tz);
    bool updateIonSource(const QString& source); // Sync data source with Applet
    void updateWeather(const QString& source);

protected slots:
    void setup_slotDataArrived(KIO::Job *, const QByteArray &);
    void setup_slotJobFinished(KJob *);

    void slotDataArrived(KIO::Job *, const QByteArray &);
    void slotJobFinished(KJob *);

private:
    /* NOAA Methods - Internal for Ion */

    // Place information
    QString country(const QString& source);
    QString place(const QString& source);
    QString station(const QString& source);

    // Current Conditions Weather info
    QString observationTime(const QString& source);
    QString condition(const QString& source);
    QMap<QString, QString> temperature(const QString& source);
    QString dewpoint(const QString& source);
    QString humidity(const QString& source);
    QMap<QString, QString> visibility(const QString& source);
    QMap<QString, QString> pressure(const QString& source);
    QMap<QString, QString> wind(const QString& source);

    // Load and Parse the place XML listing
    void getXMLSetup(void);
    bool readXMLSetup(void);

    // Load and parse the specific place(s)
    void getXMLData(const QString& source);
    bool readXMLData(const QString& source, QXmlStreamReader& xml);

    // Check if place specified is valid or not
    QStringList validate(const QString& source) const;

    // Catchall for unknown XML tags
    void parseUnknownElement(QXmlStreamReader& xml);

    // Parse weather XML data
    WeatherData parseWeatherSite(WeatherData& data, QXmlStreamReader& xml);
    void parseStationID(void);
    void parseStationList(void);

private:
    class Private;
    Private *const d;
};

K_EXPORT_PLASMA_ION(noaa, NOAAIon)

#endif
