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

#include "ion_noaa.h"

class NOAAIon::Private : public QObject
{
public:
    Private() {}
    ~Private() {}

private:
    struct XMLMapInfo {
        QString stateName;
        QString stationName;
        QString XMLurl;
        QString sourceOptions;
    };

public:
    // Key dicts
    QHash<QString, NOAAIon::Private::XMLMapInfo> m_place;
    QHash<QString, QString> m_locations;
    QString m_state;
    QString m_station_name;
    QString m_xmlurl;

    // Weather information
    QHash<QString, WeatherData> m_weatherData;

    // Store KIO jobs
    QMap<KJob *, QXmlStreamReader*> m_jobXml;
    QMap<KJob *, QString> m_jobList;
    QXmlStreamReader m_xmlSetup;
    KUrl *m_url;
    KIO::TransferJob *m_job;

    int m_timezoneType;  // Ion option: Timezone may be local time or UTC time
    int m_measureType; // Ion option: Units may be Metric or Imperial
    bool m_windInMeters; // Ion option: Display wind format in meters per second only
    bool m_windInKnots; // Ion option: Display wind format in knots
    bool m_windInBft; // ion option: Display wind by the beaufort scale model
    WeatherFormula m_formula;
};


// ctor, dtor
NOAAIon::NOAAIon(QObject *parent, const QVariantList &args)
        : IonInterface(parent), d(new Private())
{
    Q_UNUSED(args)
}

NOAAIon::~NOAAIon()
{
    // Destroy dptr
    delete d;
}

// Get the master list of locations to be parsed
void NOAAIon::init()
{
    // Get the real city XML URL so we can parse this
    getXMLSetup();
}

QStringList NOAAIon::validate(const QString& source) const
{
    QStringList placeList;
    QHash<QString, QString>::const_iterator it = d->m_locations.constBegin();
    while (it != d->m_locations.constEnd()) {
        if (it.value().toLower().contains(source.toLower())) {
            placeList.append(QString("place|%1").arg(it.value().split("|")[1]));
        }
        ++it;
    }

    // Check if placeList is empty if so, return nothing.
    if (placeList.isEmpty()) {
        return QStringList();
    }

    placeList.sort();
    return placeList;
}

bool NOAAIon::updateIonSource(const QString& source)
{
    // We expect the applet to send the source in the following tokenization:
    // ionname:validate:place_name - Triggers validation of place
    // ionname:weather:place_name - Triggers receiving weather of place

    kDebug() << "updateIonSource() SOURCE: " << source;
    QStringList sourceAction = source.split('|');
    if (sourceAction[1] == QString("validate")) {
        kDebug() << "Initiate Validating of place: " << sourceAction[2];
        QStringList result = validate(QString("%1|%2").arg(sourceAction[0]).arg(sourceAction[2]));

        if (result.size() == 1) {
            setData(source, "validate", QString("noaa|valid|single|%1").arg(result.join("|")));
            return true;
        } else if (result.size() > 1) {
            setData(source, "validate", QString("noaa|valid|multiple|%1").arg(result.join("|")));
            return true;
        } else if (result.size() == 0) {
            setData(source, "validate", QString("noaa|invalid|single|%1").arg(sourceAction[2]));
            return true;
        }

    } else if (sourceAction[1] == QString("weather")) {
        getXMLData(QString("%1|%2").arg(sourceAction[0]).arg(sourceAction[2]));
        return true;
    }
    return false;
}

// Parses city list and gets the correct city based on ID number
void NOAAIon::getXMLSetup()
{
    d->m_url = new KUrl("http://www.weather.gov/data/current_obs/index.xml");

    KIO::TransferJob *job = KIO::get(d->m_url->url(), KIO::NoReload, KIO::HideProgressInfo);

    if (job) {
        connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                SLOT(setup_slotDataArrived(KIO::Job *, const QByteArray &)));
        connect(job, SIGNAL(result(KJob *)), this, SLOT(setup_slotJobFinished(KJob *)));
    }
}

// Gets specific city XML data
void NOAAIon::getXMLData(const QString& source)
{
    KUrl url;
    url = d->m_place[source].XMLurl;

    kDebug() << "URL Location: " << url.url();

    d->m_job = KIO::get(url.url(), KIO::Reload, KIO::HideProgressInfo);
    d->m_jobXml.insert(d->m_job, new QXmlStreamReader);
    d->m_jobList.insert(d->m_job, source);

    if (d->m_job) {
        connect(d->m_job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                SLOT(slotDataArrived(KIO::Job *, const QByteArray &)));
        connect(d->m_job, SIGNAL(result(KJob *)), this, SLOT(slotJobFinished(KJob *)));
    }
}

void NOAAIon::setup_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job)

    if (data.isEmpty()) {
        return;
    }

    // Send to xml.
    d->m_xmlSetup.addData(data);
}

void NOAAIon::slotDataArrived(KIO::Job *job, const QByteArray &data)
{

    if (data.isEmpty() || !d->m_jobXml.contains(job)) {
        return;
    }

    // Send to xml.
    d->m_jobXml[job]->addData(data);
}

void NOAAIon::slotJobFinished(KJob *job)
{
    // Dual use method, if we're fetching location data to parse we need to do this first
    readXMLData(d->m_jobList[job], *d->m_jobXml[job]);
    d->m_jobList.remove(job);
    delete d->m_jobXml[job];
    d->m_jobXml.remove(job);
}

void NOAAIon::setup_slotJobFinished(KJob *job)
{
    Q_UNUSED(job)
    readXMLSetup();
    setInitialized(true);
}

void NOAAIon::parseStationID()
{
    QString tmp;
    while (!d->m_xmlSetup.atEnd()) {
        d->m_xmlSetup.readNext();

        if (d->m_xmlSetup.isEndElement() && d->m_xmlSetup.name() == "station") {
            break;
        }

        if (d->m_xmlSetup.isStartElement()) {
            if (d->m_xmlSetup.name() == "state") {
                d->m_state = d->m_xmlSetup.readElementText();
            } else if (d->m_xmlSetup.name() == "station_name") {
                d->m_station_name = d->m_xmlSetup.readElementText();
            } else if (d->m_xmlSetup.name() == "xml_url") {
                d->m_xmlurl = d->m_xmlSetup.readElementText();

                tmp = "noaa|" + d->m_station_name + ", " + d->m_state; // Build the key name.
                d->m_place[tmp].stateName = d->m_state;
                d->m_place[tmp].stationName = d->m_station_name;
                d->m_place[tmp].XMLurl = d->m_xmlurl.replace("http://", "http://www.");

                d->m_locations[tmp] = tmp;
            } else {
                parseUnknownElement(d->m_xmlSetup);
            }
        }
    }
}

void NOAAIon::parseStationList()
{
    while (!d->m_xmlSetup.atEnd()) {
        d->m_xmlSetup.readNext();

        if (d->m_xmlSetup.isEndElement()) {
            break;
        }

        if (d->m_xmlSetup.isStartElement()) {
            if (d->m_xmlSetup.name() == "station") {
                parseStationID();
            } else {
                parseUnknownElement(d->m_xmlSetup);
            }
        }
    }
}

// Parse the city list and store into a QMap
bool NOAAIon::readXMLSetup()
{
    while (!d->m_xmlSetup.atEnd()) {
        d->m_xmlSetup.readNext();

        if (d->m_xmlSetup.isStartElement()) {
            if (d->m_xmlSetup.name() == "wx_station_index") {
                parseStationList();
            }
        }
    }
    return !d->m_xmlSetup.error();
}

WeatherData NOAAIon::parseWeatherSite(WeatherData& data, QXmlStreamReader& xml)
{
    data.temperature_C = "N/A";
    data.temperature_F = "N/A";
    data.dewpoint_C = "N/A";
    data.dewpoint_F = "N/A";
    data.weather = "N/A";
    data.stationID = "N/A";
    data.pressure = "N/A";
    data.visibility = "N/A";
    data.humidity = "N/A";
    data.windSpeed = "N/A";
    data.windGust = "N/A";
    data.windchill_F = "N/A";
    data.windchill_C = "N/A";
    data.heatindex_F = "N/A";
    data.heatindex_C = "N/A";

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == "location") {
                data.locationName = xml.readElementText();
            } else if (xml.name() == "station_id") {
                data.stationID = xml.readElementText();
            } else if (xml.name() == "observation_time") {
                data.observationTime = xml.readElementText();
            } else if (xml.name() == "weather") {
                data.weather = xml.readElementText();
            } else if (xml.name() == "temp_f") {
                data.temperature_F = xml.readElementText();
            } else if (xml.name() == "temp_c") {
                data.temperature_C = xml.readElementText();
            } else if (xml.name() == "relative_humidity") {
                data.humidity = xml.readElementText();
            } else if (xml.name() == "wind_dir") {
                data.windDirection = xml.readElementText();
            } else if (xml.name() == "wind_mph") {
                data.windSpeed = xml.readElementText();
            } else if (xml.name() == "wind_gust_mph") {
                data.windGust = xml.readElementText();
            } else if (xml.name() == "pressure_in") {
                data.pressure = xml.readElementText();
            } else if (xml.name() == "dewpoint_f") {
                data.dewpoint_F = xml.readElementText();
            } else if (xml.name() == "dewpoint_c") {
                data.dewpoint_C = xml.readElementText();
            } else if (xml.name() == "heat_index_f") {
                data.heatindex_F = xml.readElementText();
            } else if (xml.name() == "heat_index_c") {
                data.heatindex_C = xml.readElementText();
            } else if (xml.name() == "windchill_f") {
                data.windchill_F = xml.readElementText();
            } else if (xml.name() == "windchill_c") {
                data.windchill_C = xml.readElementText();
            } else if (xml.name() == "visibility_mi") {
                data.visibility = xml.readElementText();
            } else {
                parseUnknownElement(xml);
            }
        }
    }
    return data;
}

// Parse Weather data main loop, from here we have to decend into each tag pair
bool NOAAIon::readXMLData(const QString& source, QXmlStreamReader& xml)
{
    WeatherData data;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "current_observation") {
                data = parseWeatherSite(data, xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }

    d->m_weatherData[source] = data;

    updateWeather(source);
    return !xml.error();
}

// handle when no XML tag is found
void NOAAIon::parseUnknownElement(QXmlStreamReader& xml)
{

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            parseUnknownElement(xml);
        }
    }
}

void NOAAIon::setMeasureUnit(const QString& unitType)
{
    d->m_measureType = unitType.toInt();
}

// Not used in this ion yet.
void NOAAIon::setTimezoneFormat(const QString& tz)
{
    d->m_timezoneType = tz.toInt(); // Boolean
}

bool NOAAIon::metricUnit()
{
    if (d->m_measureType == KLocale::Metric) {
        return true;
    }

    // Imperial units
    return false;
}

// Not used in this ion yet.
bool NOAAIon::timezone()
{
    if (d->m_timezoneType) {
        return true;
    }

    // Not UTC, local time
    return false;
}

// Custom options, for now this just handles wind type.
bool NOAAIon::options(const QString& source)
{
    QByteArray str = source.toLocal8Bit();
    // Get the dynamic Q_PROPERTY based on the datasource as property name.
    d->m_place[source].sourceOptions = property(str.data()).toString();
    QStringList option = d->m_place[source].sourceOptions.split("|");
    kDebug() << "========> Available Options: " << d->m_place[source].sourceOptions;

    d->m_windInMeters = false;
    d->m_windInKnots = false;
    d->m_windInBft = false;

    // Metastring format:  option|type
    if (option[0] == "WINDFORMAT") {
        if (option[1] == "MS") {
            d->m_windInMeters = true;
        } else if (option[1] == "KNOTS") {
            d->m_windInKnots = true;
        } else if (option[1] == "BEAUFORT") {
            d->m_windInBft = true;
        }
    }
    return true;
}

void NOAAIon::updateWeather(const QString& source)
{
    QString weatherSource = source;

    weatherSource.replace("noaa|", "noaa|weather|");
    options(weatherSource);

    QMap<QString, QString> dataFields;
    QStringList fieldList;

    setData(weatherSource, "Country", country(source));
    setData(weatherSource, "Place", place(source));
    setData(weatherSource, "Station", station(source));

    // Real weather - Current conditions
    setData(weatherSource, "Observation Period", observationTime(source));
    setData(weatherSource, "Current Conditions", condition(source));
    dataFields = temperature(source);
    setData(weatherSource, "Temperature", dataFields["temperature"]);

    if (dataFields["temperature"] != "N/A") {
        setData(weatherSource, "Temperature Unit", dataFields["temperatureUnit"]);
    }

    // Do we have a comfort temperature? if so display it
    if (dataFields["comfortTemperature"] != "N/A") {
        if (d->m_weatherData[source].windchill_F != "NA") {
            setData(weatherSource, "Windchill", QString("%1%2").arg(dataFields["comfortTemperature"]).arg(QChar(176)));
            setData(weatherSource, "Humidex", "N/A");
        }
        if (d->m_weatherData[source].heatindex_F != "NA" && d->m_weatherData[source].temperature_F.toInt() != d->m_weatherData[source].heatindex_F.toInt()) {
            setData(weatherSource, "Humidex", QString("%1%2").arg(dataFields["comfortTemperature"]).arg(QChar(176)));
            setData(weatherSource, "Windchill", "N/A");
        }
    } else {
        setData(weatherSource, "Windchill", "N/A");
        setData(weatherSource, "Humidex", "N/A");
    }

    setData(weatherSource, "Dewpoint", dewpoint(source));
    if (dewpoint(source) != "N/A") {
        setData(weatherSource, "Dewpoint Unit", dataFields["temperatureUnit"]);
    }

    dataFields = pressure(source);
    setData(weatherSource, "Pressure", dataFields["pressure"]);

    if (dataFields["pressure"] != "N/A") {
        setData(weatherSource, "Pressure Unit", dataFields["pressureUnit"]);
    }

    dataFields = visibility(source);
    setData(weatherSource, "Visibility", dataFields["visibility"]);

    if (dataFields["visibility"] != "N/A") {
        setData(weatherSource, "Visibility Unit", dataFields["visibilityUnit"]);
    }

    setData(weatherSource, "Humidity", humidity(source));

    dataFields = wind(source);
    setData(weatherSource, "Wind Speed", dataFields["windSpeed"]);

    if (dataFields["windSpeed"] != "Calm") {
        setData(weatherSource, "Wind Speed Unit", dataFields["windUnit"]);
    }

    setData(weatherSource, "Wind Gust", dataFields["windGust"]);
    setData(weatherSource, "Wind Gust Unit", dataFields["windGustUnit"]);
    setData(weatherSource, "Wind Direction", dataFields["windDirection"]);
    setData(weatherSource, "Credit", "Data provided by NOAA National Weather Service");
}

QString NOAAIon::country(const QString& source)
{
    Q_UNUSED(source);
    return QString("USA");
}
QString NOAAIon::place(const QString& source)
{
    return d->m_weatherData[source].locationName;
}
QString NOAAIon::station(const QString& source)
{
    return d->m_weatherData[source].stationID;
}

QString NOAAIon::observationTime(const QString& source)
{
    return d->m_weatherData[source].observationTime;
}
QString NOAAIon::condition(const QString& source)
{
    if (d->m_weatherData[source].weather.isEmpty() || d->m_weatherData[source].weather == "NA") {
        d->m_weatherData[source].weather = "N/A";
    }
    return d->m_weatherData[source].weather;
}

QString NOAAIon::dewpoint(const QString& source)
{
    if (metricUnit()) {
        return d->m_weatherData[source].dewpoint_C;
    }
    return d->m_weatherData[source].dewpoint_F;
}

QString NOAAIon::humidity(const QString& source)
{
    if (d->m_weatherData[source].humidity == "NA") {
        return QString("N/A");
    } else {
        return QString("%1%").arg(d->m_weatherData[source].humidity);
    }
}

QMap<QString, QString> NOAAIon::visibility(const QString& source)
{
    QMap<QString, QString> visibilityInfo;
    if (d->m_weatherData[source].visibility.isEmpty()) {
        visibilityInfo.insert("visibility", QString("N/A"));
        return visibilityInfo;
    }
    if (metricUnit()) {
        visibilityInfo.insert("visibility", QString::number(d->m_formula.milesToKM(d->m_weatherData[source].visibility.toFloat()), 'f', 1));
        visibilityInfo.insert("visibilityUnit", "km");
        return visibilityInfo;
    }
    visibilityInfo.insert("visibility", d->m_weatherData[source].visibility);
    visibilityInfo.insert("visibilityUnit", "mi");
    return visibilityInfo;
}

QMap<QString, QString> NOAAIon::temperature(const QString& source)
{
    QMap<QString, QString> temperatureInfo;
    if (metricUnit()) {
        temperatureInfo.insert("temperature", d->m_weatherData[source].temperature_C);
        temperatureInfo.insert("temperatureUnit", QString("%1C").arg(QChar(176)));
    } else {
        temperatureInfo.insert("temperature", d->m_weatherData[source].temperature_F);
        temperatureInfo.insert("temperatureUnit", QString("%1F").arg(QChar(176)));
    }
    temperatureInfo.insert("comfortTemperature", "N/A");

    if (d->m_weatherData[source].heatindex_F != "NA" && d->m_weatherData[source].windchill_F == "NA") {
        if (metricUnit()) {
            temperatureInfo.insert("comfortTemperature", d->m_weatherData[source].heatindex_C);
        } else {
            temperatureInfo.insert("comfortTemperature", d->m_weatherData[source].heatindex_F);
        }
    }
    if (d->m_weatherData[source].windchill_F != "NA" && d->m_weatherData[source].heatindex_F == "NA") {
        if (metricUnit()) {
            temperatureInfo.insert("comfortTemperature", d->m_weatherData[source].windchill_C);
        } else {
            temperatureInfo.insert("comfortTemperature", d->m_weatherData[source].windchill_F);
        }
    }

    return temperatureInfo;
}

QMap<QString, QString> NOAAIon::pressure(const QString& source)
{
    QMap<QString, QString> pressureInfo;
    if (d->m_weatherData[source].pressure.isEmpty()) {
        pressureInfo.insert("pressure", "N/A");
        return pressureInfo;
    }
    if (metricUnit()) {
        pressureInfo.insert("pressure", QString::number(d->m_formula.inchesToKilopascals(d->m_weatherData[source].pressure.toFloat()), 'f', 1));
        pressureInfo.insert("pressureUnit", "kPa");
    } else {
        pressureInfo.insert("pressure", d->m_weatherData[source].pressure);
        pressureInfo.insert("pressureUnit", "in");
    }
    return pressureInfo;
}

QMap<QString, QString> NOAAIon::wind(const QString& source)
{
    QMap<QString, QString> windInfo;

    // May not have any winds
    if (d->m_weatherData[source].windSpeed == "NA") {
        windInfo.insert("windSpeed", "Calm");
        windInfo.insert("windUnit", "N/A");
    } else {
        if (metricUnit()) {
            if (d->m_windInMeters) {
                windInfo.insert("windSpeed", QString::number(d->m_formula.milesToMS(d->m_weatherData[source].windSpeed.toFloat()), 'f', 2));
                windInfo.insert("windUnit", "m/s");
            } else if (d->m_windInKnots) {
                windInfo.insert("windSpeed", QString::number(d->m_formula.milesToKT(d->m_weatherData[source].windSpeed.toFloat()), 'f', 1));
                windInfo.insert("windUnit", "kt");
            } else if (d->m_windInBft) {
                windInfo.insert("windSpeed", QString::number(d->m_formula.milesToBF(d->m_weatherData[source].windSpeed.toInt())));
                windInfo.insert("windUnit", "bft");
            } else {
                windInfo.insert("windSpeed", QString::number(d->m_formula.milesToKM(d->m_weatherData[source].windSpeed.toFloat()), 'f', 1));
                windInfo.insert("windUnit", "km/h");
            }
        } else {
            if (d->m_windInKnots) {
                windInfo.insert("windSpeed", QString::number(d->m_formula.milesToKT(d->m_weatherData[source].windSpeed.toFloat()), 'f', 1));
                windInfo.insert("windUnit", "kt");
            } else if (d->m_windInBft) {
                windInfo.insert("windSpeed", QString::number(d->m_formula.milesToBF(d->m_weatherData[source].windSpeed.toInt())));
                windInfo.insert("windUnit", "bft");
            } else {
                windInfo.insert("windSpeed", QString::number(d->m_weatherData[source].windSpeed.toFloat(), 'f', 1));
                windInfo.insert("windUnit", "mph");
            }
        }
    }

    // May not always have gusty winds
    if (d->m_weatherData[source].windGust == "NA") {
        windInfo.insert("windGust", "N/A");
        windInfo.insert("windGustUnit", "N/A");
    } else {
        if (metricUnit()) {
            if (d->m_windInMeters) {
                windInfo.insert("windGust", QString::number(d->m_formula.milesToMS(d->m_weatherData[source].windGust.toFloat()), 'f', 2));
                windInfo.insert("windGustUnit", "m/s");
            } else if (d->m_windInKnots) {
                windInfo.insert("windGust", QString::number(d->m_formula.milesToKT(d->m_weatherData[source].windGust.toFloat()), 'f', 1));
                windInfo.insert("windGustUnit", "kt");
            } else if (d->m_windInBft) {
                windInfo.insert("windGust", QString::number(d->m_formula.milesToBF(d->m_weatherData[source].windGust.toInt())));
                windInfo.insert("windGustunit", "bft");
            } else {
                windInfo.insert("windGust", QString::number(d->m_formula.milesToKM(d->m_weatherData[source].windGust.toFloat()), 'f', 1));
                windInfo.insert("windGustUnit", "km/h");
            }
        } else {
            if (d->m_windInKnots) {
                windInfo.insert("windGust", QString::number(d->m_formula.milesToKT(d->m_weatherData[source].windGust.toFloat()), 'f', 1));
                windInfo.insert("windGustUnit", "kt");
            } else if (d->m_windInBft) {
                windInfo.insert("windGust", QString::number(d->m_formula.milesToBF(d->m_weatherData[source].windGust.toInt())));
                windInfo.insert("windGustunit", "bft");
            } else {
                windInfo.insert("windGust", QString::number(d->m_weatherData[source].windGust.toFloat(), 'f', 1));
                windInfo.insert("windGustUnit", "mph");
            }
        }
    }

    if (d->m_weatherData[source].windDirection.isEmpty()) {
        windInfo.insert("windDirection", "N/A");
    } else {
        windInfo.insert("windDirection", d->m_weatherData[source].windDirection);
    }
    return windInfo;
}

#include "ion_noaa.moc"
