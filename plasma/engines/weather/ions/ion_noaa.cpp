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

    bool m_useUTC;  // Ion option: Timezone may be local time or UTC time
    bool m_useMetric; // Ion option: Units may be Metric or Imperial
    bool m_windInMeters; // Ion option: Display wind format in meters per second only

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

bool NOAAIon::validLocation(QString keyName)
{
    QHash<QString, QString>::const_iterator it = d->m_locations.find(keyName);
    if (it != d->m_locations.end()) {
        return true;
    }
    return false;
}

// Get a specific Ion's data
void NOAAIon::fetch()
{
    // Go read the city's XML file
    getXMLData();
}

// Parses city list and gets the correct city based on ID number
void NOAAIon::getXMLSetup()
{

    d->m_url = new KUrl("http://www.weather.gov/data/current_obs/index.xml");

    KIO::TransferJob *job = KIO::get(d->m_url->url(), false, false);

    if (job) {
        connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                SLOT(setup_slotDataArrived(KIO::Job *, const QByteArray &)));
        connect(job, SIGNAL(result(KJob *)), this, SLOT(setup_slotJobFinished(KJob *)));
    }
}

// Gets specific city XML data
void NOAAIon::getXMLData()
{
    KUrl url;
    foreach(QString key, this->ionSourceDict()) {
        if (!validLocation(key)) {
            continue;
        } else {
            url = d->m_place[key].XMLurl;
        }

        kDebug() << "URL Location: " << url.url();

        d->m_job = KIO::get(url.url(), true, false);
        d->m_jobXml.insert(d->m_job, new QXmlStreamReader);
        d->m_jobList.insert(d->m_job, key);

        if (d->m_job) {
            connect(d->m_job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                    SLOT(slotDataArrived(KIO::Job *, const QByteArray &)));
            connect(d->m_job, SIGNAL(result(KJob *)), this, SLOT(slotJobFinished(KJob *)));
        }
    }
}

void NOAAIon::setup_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job)

    if (data.isEmpty()) {
        return;
    }

    // Send to xml.
    d->m_xmlSetup.addData(data.data());
}

void NOAAIon::slotDataArrived(KIO::Job *job, const QByteArray &data)
{

    if (data.isEmpty() || !d->m_jobXml.contains(job)) {
        return;
    }

    // Send to xml.
    d->m_jobXml[job]->addData(data.data());
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

    // Do the XML fetching for the correct city.
    getXMLData();
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
                d->m_station_name= d->m_xmlSetup.readElementText();
            } else if (d->m_xmlSetup.name() == "xml_url") {
                d->m_xmlurl = d->m_xmlSetup.readElementText();

                tmp = d->m_station_name + ", " + d->m_state; // Build the key name.
                d->m_place[tmp].stateName = d->m_state;
                d->m_place[tmp].stationName = d->m_station_name;
	        d->m_place[tmp].XMLurl = d->m_xmlurl;

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
bool NOAAIon::readXMLData(QString key, QXmlStreamReader& xml)
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

    d->m_weatherData[key] = data;
    updateData();
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

// User toggleable values set from the dataengine <-> Plasma Applet
void NOAAIon::option(int option, QVariant value)
{
    switch (option) {
    case IonInterface::UNITS:
        // Set the Units used (Depends on Ion)
        if (value.toInt() == KLocale::Metric) {
            d->m_useMetric = true;
        }
        if (value.toInt() == KLocale::Imperial) {
            d->m_useMetric = false;
        }
        break;
    case IonInterface::TIMEFORMAT:
        if (value.toBool()) {
            d->m_useUTC = true;
        }
        break;
    case IonInterface::WINDFORMAT:
        if (value.toBool()) {
           d->m_windInMeters = true;
        }
        break;
    }
}

void NOAAIon::updateData()
{
    QVector<QString> sources;
    QMap<QString, QString> dataFields;
    QStringList fieldList;

    sources = this->ionSourceDict();
    foreach(QString keyname, sources) {
        // Does this place exist?
        if (!this->validLocation(keyname)) {
            kDebug() << "NOAAIon::updateData() " << keyname << " is not a valid location";
            //setData(keyname, "Invalid Place", "Invalid location");
            continue;
        }
        setData(keyname, "Country", this->country(keyname));
        setData(keyname, "Place", this->place(keyname));
        setData(keyname, "Airport Code", this->station(keyname));

        // Real weather - Current conditions
        setData(keyname, "Observations At", this->observationTime(keyname));
        setData(keyname, "Current Conditions", this->condition(keyname));
        dataFields = this->temperature(keyname);
        setData(keyname, "Temperature", dataFields["temperature"]);

        // Do we have a comfort temperature? if so display it
        if (dataFields["comfortTemperature"] != "N/A") {
           if (d->m_weatherData[keyname].windchill_F != "NA") {
               setData(keyname, "Windchill", QString("%1%2").arg(dataFields["comfortTemperature"]).arg(QChar(176)));
           }
           if (d->m_weatherData[keyname].heatindex_F != "NA" && d->m_weatherData[keyname].temperature_F.toInt() != d->m_weatherData[keyname].heatindex_F.toInt()) {
               setData(keyname, "Humidex", QString("%1%2").arg(dataFields["comfortTemperature"]).arg(QChar(176)));
           }
        }

        setData(keyname, "Dewpoint", this->dewpoint(keyname));
        setData(keyname, "Pressure", this->pressure(keyname));
        setData(keyname, "Visibility", this->visibility(keyname));
        setData(keyname, "Humidity", this->humidity(keyname));

        dataFields = this->wind(keyname);
        setData(keyname, "Wind Speed", dataFields["windSpeed"]);
        setData(keyname, "Wind Speed Unit", dataFields["windUnit"]);
        setData(keyname, "Wind Gust", dataFields["windGust"]);
        setData(keyname, "Wind Gust Unit", dataFields["windGustUnit"]);
        setData(keyname, "Wind Direction", dataFields["windDirection"]);

        setData(keyname, "Credit", "NOAA National Weather Service");
    }
}

QString NOAAIon::country(QString key)
{
    Q_UNUSED(key);
    return QString("USA");
}
QString NOAAIon::place(QString key)
{
    return d->m_weatherData[key].locationName;
}
QString NOAAIon::station(QString key)
{
    return d->m_weatherData[key].stationID;
}

QString NOAAIon::observationTime(QString key)
{
    return d->m_weatherData[key].observationTime;
}
QString NOAAIon::condition(QString key)
{
    if (d->m_weatherData[key].weather.isEmpty() || d->m_weatherData[key].weather == "NA") {
        d->m_weatherData[key].weather = "N/A";
    }
    return d->m_weatherData[key].weather;
}

QString NOAAIon::dewpoint(QString key)
{
    if (d->m_useMetric) {
        return QString("%1").arg(d->m_weatherData[key].dewpoint_C);
    }
    return QString("%1").arg(d->m_weatherData[key].dewpoint_F);
}

QString NOAAIon::humidity(QString key)
{
   if (d->m_weatherData[key].humidity == "NA") {
       return QString("N/A");
   } else {
       return QString("%1%").arg(d->m_weatherData[key].humidity);
   }
}

QString NOAAIon::visibility(QString key)
{
    if (d->m_weatherData[key].visibility.isEmpty()) {
        return QString("N/A");
    }
    if (d->m_useMetric) {
        return QString("%1").arg(QString::number(d->m_formula.milesToKM(d->m_weatherData[key].visibility.toFloat()), 'f', 1));
    } 
    return QString("%1").arg(d->m_weatherData[key].visibility);
}

QMap<QString, QString> NOAAIon::temperature(QString key)
{
    QMap<QString, QString> temperatureInfo;
    if (d->m_useMetric) {
        temperatureInfo.insert("temperature", QString("%1").arg(d->m_weatherData[key].temperature_C));
    } else {
        temperatureInfo.insert("temperature", QString("%1").arg(d->m_weatherData[key].temperature_F));
    }
    temperatureInfo.insert("comfortTemperature", "N/A");

    if (d->m_weatherData[key].heatindex_F != "NA" && d->m_weatherData[key].windchill_F == "NA") {
        if (d->m_useMetric) {
            temperatureInfo.insert("comfortTemperature", d->m_weatherData[key].heatindex_C);
        } else {
            temperatureInfo.insert("comfortTemperature", d->m_weatherData[key].heatindex_F);
        }
    }
    if (d->m_weatherData[key].windchill_F != "NA" && d->m_weatherData[key].heatindex_F == "NA") {
        if (d->m_useMetric) {
            temperatureInfo.insert("comfortTemperature", d->m_weatherData[key].windchill_C);
        } else {
            temperatureInfo.insert("comfortTemperature", d->m_weatherData[key].windchill_F);
        }
    }

    return temperatureInfo;
}

QString NOAAIon::pressure(QString key)
{
    if (d->m_weatherData[key].pressure.isEmpty()) {
        return QString("N/A");
    } 
    if (d->m_useMetric) {
        return QString("%1").arg(QString::number(d->m_formula.inchesToKilopascals(d->m_weatherData[key].pressure.toFloat()), 'f', 1));
    } else {
        return QString("%1").arg(d->m_weatherData[key].pressure);
    }
}

QMap<QString, QString> NOAAIon::wind(QString key)
{
    QMap<QString, QString> windInfo;

    // May not have any winds
    if (d->m_weatherData[key].windSpeed == "NA") {
        windInfo.insert("windSpeed", "Calm");
    } else {
        if (d->m_useMetric) {
            if (d->m_windInMeters) {
                windInfo.insert("windSpeed", QString("%1").arg(QString::number(d->m_formula.milesToMS(d->m_weatherData[key].windSpeed.toFloat()), 'f', 2)));
                windInfo.insert("windUnit", "m/s");
            } else {
                windInfo.insert("windSpeed", QString("%1").arg(QString::number(d->m_formula.milesToKM(d->m_weatherData[key].windSpeed.toFloat()), 'f', 1)));
                windInfo.insert("windUnit", "km/h");
            }
        } else {
            windInfo.insert("windSpeed", QString("%1").arg(QString::number(d->m_weatherData[key].windSpeed.toFloat(), 'f', 1)));
            windInfo.insert("windUnit", "mph");
        }
    }

    // May not always have gusty winds
    if (d->m_weatherData[key].windGust == "NA") {
        windInfo.insert("windGust", "N/A");
    } else {
        if (d->m_useMetric) {
            if (d->m_windInMeters) {
                windInfo.insert("windGust", QString("%1").arg(QString::number(d->m_formula.milesToMS(d->m_weatherData[key].windGust.toFloat()), 'f', 2)));
                windInfo.insert("windGustUnit", "m/s");
            } else {
                windInfo.insert("windGust", QString("%1").arg(QString::number(d->m_formula.milesToKM(d->m_weatherData[key].windGust.toFloat()), 'f', 1)));
                windInfo.insert("windGustUnit", "km/h");
            }
        } else {
            windInfo.insert("windGust", QString("%1").arg(QString::number(d->m_weatherData[key].windGust.toFloat(), 'f', 1)));
            windInfo.insert("windGustUnit", "mph");
        }
    }

    if (d->m_weatherData[key].windDirection.isEmpty()) {
        windInfo.insert("windDirection", "N/A");
    } else {
        windInfo.insert("windDirection", d->m_weatherData[key].windDirection);
    }
    return windInfo;
}

#include "ion_noaa.moc"
