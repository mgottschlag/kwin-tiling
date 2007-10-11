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

/* Ion for Environment Canada XML data */

#include "ion_envcan.h"

class EnvCanadaIon::Private : public QObject
{
public:
    Private() {}
    ~Private() {}

private:
    struct XMLMapInfo {
        QString cityName;
        QString territoryName;
        QString cityCode;
    };

public:
    // Key dicts
    QHash<QString, EnvCanadaIon::Private::XMLMapInfo> m_place;
    QHash<QString, QString> m_locations;
    QString m_code;
    QString m_territory;
    QString m_cityName;

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
EnvCanadaIon::EnvCanadaIon(QObject *parent, const QVariantList &args)
        : IonInterface(parent), d(new Private())
{
    Q_UNUSED(args)
}

EnvCanadaIon::~EnvCanadaIon()
{
    // Destroy each warning stored in a QVector
    foreach(WeatherData item, d->m_weatherData) {
        foreach(WeatherData::WarningInfo *warning, item.warnings) {
            if (warning) {
                delete warning;
            }
        }
        foreach(WeatherData::ForecastInfo *forecast, item.forecasts) {
            if (forecast) {
                delete forecast;
            }
        }
    }

    // Destroy dptr
    delete d;
}

// Get the master list of locations to be parsed
void EnvCanadaIon::init()
{
    // Get the real city XML URL so we can parse this
    getXMLSetup();
}

bool EnvCanadaIon::validLocation(QString keyName)
{
    QHash<QString, QString>::const_iterator it = d->m_locations.find(keyName);
    if (it != d->m_locations.end()) {
        return true;
    }
    return false;
}

// Get a specific Ion's data
void EnvCanadaIon::fetch()
{
    // Go read the city's XML file
    getXMLData();
}

// Parses city list and gets the correct city based on ID number
void EnvCanadaIon::getXMLSetup()
{

    d->m_url = new KUrl("http://dd.weatheroffice.ec.gc.ca/EC_sites/xml/siteList.xml");

    KIO::TransferJob *job = KIO::get(d->m_url->url(), KIO::NoReload, KIO::HideProgressInfo);

    if (job) {
        connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                SLOT(setup_slotDataArrived(KIO::Job *, const QByteArray &)));
        connect(job, SIGNAL(result(KJob *)), this, SLOT(setup_slotJobFinished(KJob *)));
    }
}

// Gets specific city XML data
void EnvCanadaIon::getXMLData()
{
    KUrl url;
    foreach(QString key, this->ionSourceDict()) {
        if (!validLocation(key)) {
            continue;
        } else {
            url = "http://dd.weatheroffice.ec.gc.ca/EC_sites/xml/" + d->m_place[key].territoryName + "/" + d->m_place[key].cityCode + "_e.xml";
        }

        //kDebug() << "URL Location: " << url.url();

        d->m_job = KIO::get(url.url(), KIO::Reload, KIO::HideProgressInfo);
        d->m_jobXml.insert(d->m_job, new QXmlStreamReader);
        d->m_jobList.insert(d->m_job, key);

        if (d->m_job) {
            connect(d->m_job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                    SLOT(slotDataArrived(KIO::Job *, const QByteArray &)));
            connect(d->m_job, SIGNAL(result(KJob *)), this, SLOT(slotJobFinished(KJob *)));
        }
    }
}

void EnvCanadaIon::setup_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job)

    if (data.isEmpty()) {
        return;
    }

    // Send to xml.
    d->m_xmlSetup.addData(data.data());
}

void EnvCanadaIon::slotDataArrived(KIO::Job *job, const QByteArray &data)
{

    if (data.isEmpty() || !d->m_jobXml.contains(job)) {
        return;
    }

    // Send to xml.
    d->m_jobXml[job]->addData(data.data());
}

void EnvCanadaIon::slotJobFinished(KJob *job)
{
    // Dual use method, if we're fetching location data to parse we need to do this first
    readXMLData(d->m_jobList[job], *d->m_jobXml[job]);
    d->m_jobList.remove(job);
    delete d->m_jobXml[job];
    d->m_jobXml.remove(job);
}

void EnvCanadaIon::setup_slotJobFinished(KJob *job)
{
    Q_UNUSED(job)
    readXMLSetup();

    // Do the XML fetching for the correct city.
    getXMLData();
}

// Parse the city list and store into a QMap
bool EnvCanadaIon::readXMLSetup()
{
    QString tmp;
    while (!d->m_xmlSetup.atEnd()) {
        d->m_xmlSetup.readNext();

        if (d->m_xmlSetup.isStartElement()) {

            // XML ID code to match filename
            if (d->m_xmlSetup.name() == "site") {
                d->m_code = d->m_xmlSetup.attributes().value("code").toString();
            }

            if (d->m_xmlSetup.name() == "nameEn") {
                d->m_cityName = d->m_xmlSetup.readElementText(); // Name of cities
            }

            if (d->m_xmlSetup.name() == "provinceCode") {
                d->m_territory = d->m_xmlSetup.readElementText(); // Provinces/Territory list
                tmp = d->m_cityName + ", " + d->m_territory; // Build the key name.

                // Set the mappings
                d->m_place[tmp].cityCode = d->m_code;
                d->m_place[tmp].territoryName = d->m_territory;
                d->m_place[tmp].cityName = d->m_cityName;

                // Set the string list, we will use for the applet to display the available cities.
                d->m_locations[tmp] = tmp;
            }
        }

    }
    return !d->m_xmlSetup.error();
}

WeatherData EnvCanadaIon::parseWeatherSite(WeatherData& data, QXmlStreamReader& xml)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == "license") {
                xml.readElementText();
            } else if (xml.name() == "location") {
                parseLocations(data, xml);
            } else if (xml.name() == "warnings") {
                parseWarnings(data, xml);
            } else if (xml.name() == "currentConditions") {
                parseConditions(data, xml);
            } else if (xml.name() == "forecastGroup") {
                parseWeatherForecast(data, xml);
            } else if (xml.name() == "yesterdayConditions") {
                parseYesterdayWeather(data, xml);
            }
            else if (xml.name() == "riseSet") {
                parseAstronomicals(data, xml);
            }
            else if (xml.name() == "almanac") {
              parseWeatherRecords(data, xml);
            }
            else {
                parseUnknownElement(xml);
            }
        }
    }
    return data;
}

// Parse Weather data main loop, from here we have to decend into each tag pair
bool EnvCanadaIon::readXMLData(QString key, QXmlStreamReader& xml)
{
    WeatherData data;
    data.comforttemp = "N/A";
    data.shortTerritoryName = d->m_place[key].territoryName;
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "siteData") {
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

void EnvCanadaIon::parseDateTime(WeatherData& data, QXmlStreamReader& xml, WeatherData::WarningInfo *warning)
{

    Q_ASSERT(xml.isStartElement() && xml.name() == "dateTime");

    // What kind of date info is this?
    QString dateType = xml.attributes().value("name").toString();
    QString dateZone = xml.attributes().value("zone").toString();


    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (dateType == "xmlCreation") {
                return;
            }
            if (xml.name() == "year") {
                xml.readElementText();
            } else if (xml.name() == "month") {
                xml.readElementText();
            } else if (xml.name() == "day") {
                xml.readElementText();
            } else if (xml.name() == "hour")
                xml.readElementText();
            else if (xml.name() == "minute")
                xml.readElementText();
            else if (xml.name() == "timeStamp") {
                if (d->m_useUTC && dateZone == "UTC") {
                    // Which timestamp are we for?

                    if (dateType == "eventIssue") {
                        if (warning) {
                            warning->timestamp = xml.readElementText();
                        }
                    } else if (dateType == "observation") {
                        data.obsTimestamp = xml.readElementText();
                    } else if (dateType == "forecastIssue") {
                        data.forecastTimestamp = xml.readElementText();
                    } else if (dateType == "sunrise") {
                        data.sunriseTimestamp = xml.readElementText();
                    } else if (dateType == "sunset") {
                        data.sunsetTimestamp = xml.readElementText();
                    } else if (dateType == "moonrise") {
                        data.moonriseTimestamp = xml.readElementText();
                    } else if (dateType == "moonset") {
                        data.moonsetTimestamp = xml.readElementText();
                    }

                } else if (dateZone != "UTC") {
                    if (dateType == "eventIssue") {
                        if (warning) {
                            warning->timestamp = xml.readElementText();
                        }
                    } else if (dateType == "observation") {
                        data.obsTimestamp = xml.readElementText();
                    } else if (dateType == "forecastIssue") {
                        data.forecastTimestamp = xml.readElementText();
                    } else if (dateType == "sunrise") {
                        data.sunriseTimestamp = xml.readElementText();
                    } else if (dateType == "sunset") {
                        data.sunsetTimestamp = xml.readElementText();
                    } else if (dateType == "moonrise") {
                        data.moonriseTimestamp = xml.readElementText();
                    } else if (dateType == "moonset") {
                        data.moonsetTimestamp = xml.readElementText();
                    }
                }
            } else if (xml.name() == "textSummary") {
                xml.readElementText();
            }
        }
    }
}

void EnvCanadaIon::parseLocations(WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "location");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "country") {
                data.countryName = xml.readElementText();
            } else if (xml.name() == "province" || xml.name() == "territory") {
                data.longTerritoryName = xml.readElementText();
            } else if (xml.name() == "name") {
                data.cityName = xml.readElementText();
            } else if (xml.name() == "region") {
                data.regionName = xml.readElementText();
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

void EnvCanadaIon::parseWindInfo(WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "wind");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "speed") {
                data.windSpeed = xml.readElementText();
            } else if (xml.name() == "gust") {
                data.windGust = xml.readElementText();
            } else if (xml.name() == "direction") {
                data.windDirection = xml.readElementText();
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

void EnvCanadaIon::parseConditions(WeatherData& data, QXmlStreamReader& xml)
{

    Q_ASSERT(xml.isStartElement() && xml.name() == "currentConditions");
    data.temperature = "N/A";
    data.dewpoint = "N/A";
    data.condition = "N/A";
    data.comforttemp = "N/A";
    data.stationID = "N/A";
    data.pressure = 0.0;
    data.pressureTendency = "N/A";
    data.visibility = 0;
    data.humidity = "N/A";
    data.windSpeed = "N/A";
    data.windGust = "N/A";

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "currentConditions")
            break;

        if (xml.isStartElement()) {
            if (xml.name() == "station") {
                data.stationID = xml.attributes().value("code").toString();
            } else if (xml.name() == "dateTime") {
                parseDateTime(data, xml);
            } else if (xml.name() == "condition") {
                data.condition = xml.readElementText();
            } else if (xml.name() == "temperature") {
                data.temperature = xml.readElementText();;
            } else if (xml.name() == "dewpoint") {
                data.dewpoint = xml.readElementText();
            } else if (xml.name() == "humidex" || xml.name() == "windChill") {
                data.comforttemp = xml.readElementText();
            } else if (xml.name() == "pressure") {
                data.pressureTendency = xml.attributes().value("tendency").toString();
                if (data.pressureTendency.isEmpty()) {
                    data.pressureTendency = "steady";
                }
                data.pressure = xml.readElementText().toFloat();
            } else if (xml.name() == "visibility") {
                data.visibility = xml.readElementText().toFloat();
            } else if (xml.name() == "relativeHumidity") {
                data.humidity = xml.readElementText();
            } else if (xml.name() == "wind") {
                parseWindInfo(data, xml);
            }
            //} else {
            //    parseUnknownElement(xml);
            //}
        }
    }
}

void EnvCanadaIon::parseWarnings(WeatherData &data, QXmlStreamReader& xml)
{
    WeatherData::WarningInfo* warning = new WeatherData::WarningInfo;

    Q_ASSERT(xml.isStartElement() && xml.name() == "warnings");
    QString warningURL = xml.attributes().value("url").toString();
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "warnings") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "dateTime") {
                parseDateTime(data, xml, warning);
                if (!warning->timestamp.isEmpty() && !warning->url.isEmpty())  {
                    data.warnings.append(warning);
                    warning = new WeatherData::WarningInfo;
                }
            } else if (xml.name() == "event") {
                // Append new event to list.
                warning->url = warningURL;
                warning->type = xml.attributes().value("type").toString();
                warning->priority = xml.attributes().value("priority").toString();
                warning->description = xml.attributes().value("description").toString();
            } else {
                if (xml.name() != "dateTime") {
                    parseUnknownElement(xml);
                }
            }
        }
    }
    delete warning;
}


void EnvCanadaIon::parseWeatherForecast(WeatherData& data, QXmlStreamReader& xml)
{
    WeatherData::ForecastInfo* forecast = new WeatherData::ForecastInfo;
    Q_ASSERT(xml.isStartElement() && xml.name() == "forecastGroup");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "forecastGroup") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "dateTime") {
                parseDateTime(data, xml);
            } else if (xml.name() == "regionalNormals") {
                parseRegionalNormals(data, xml);
            } else if (xml.name() == "forecast") {
                parseForecast(data, xml, forecast);
                forecast = new WeatherData::ForecastInfo;
            } else {
                parseUnknownElement(xml);
            }
        }
    }
    delete forecast;
}

void EnvCanadaIon::parseRegionalNormals(WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "regionalNormals");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "textSummary") {
                xml.readElementText();
            } else if (xml.name() == "temperature" && xml.attributes().value("class") == "high") {
                data.normalHigh = xml.readElementText();
            } else if (xml.name() == "temperature" && xml.attributes().value("class") == "low") {
                data.normalLow = xml.readElementText();
            }
        }
    }
}

void EnvCanadaIon::parseForecast(WeatherData& data, QXmlStreamReader& xml, WeatherData::ForecastInfo *forecast)
{

    Q_ASSERT(xml.isStartElement() && xml.name() == "forecast");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "forecast") {
            data.forecasts.append(forecast);
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "period") {
                forecast->forecastPeriod = xml.readElementText();
            } else if (xml.name() == "textSummary") {
                forecast->forecastSummary = xml.readElementText();
            } else if (xml.name() == "abbreviatedForecast") {
                parseShortForecast(forecast, xml);
            } else if (xml.name() == "temperatures") {
                parseForecastTemperatures(forecast, xml);
            } else if (xml.name() == "winds") {
                parseWindForecast(forecast, xml);
            } else if (xml.name() == "precipitation") {
                parsePrecipitationForecast(forecast, xml);
            } else if (xml.name() == "uv") {
                data.UVRating = xml.attributes().value("category").toString();
                parseUVIndex(data, xml);
                // else if (xml.name() == "frost") { FIXME: Wait until winter to see what this looks like.
                //  parseFrost(xml, forecast);
            } else {
                if (xml.name() != "forecast") {
                    parseUnknownElement(xml);
                }
            }
        }
    }
}

void EnvCanadaIon::parseShortForecast(WeatherData::ForecastInfo *forecast, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "abbreviatedForecast");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "abbreviatedForecast") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "pop") {
                forecast->popPrecent = xml.readElementText();
            }
            if (xml.name() == "textSummary") {
                forecast->shortForecast = xml.readElementText();
            }
        }
    }
}

void EnvCanadaIon::parseUVIndex(WeatherData& data, QXmlStreamReader& xml)
{
    Q_UNUSED(data);
    Q_ASSERT(xml.isStartElement() && xml.name() == "uv");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "uv") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "index") {
                data.UVIndex = xml.readElementText();
            }
            if (xml.name() == "textSummary") {
                xml.readElementText();
            }
        }
    }
}

void EnvCanadaIon::parseForecastTemperatures(WeatherData::ForecastInfo *forecast, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "temperatures");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "temperatures") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "temperature" && xml.attributes().value("class") == "low") {
                forecast->forecastTempLow = xml.readElementText();
            } else if (xml.name() == "temperature" && xml.attributes().value("class") == "high") {
                forecast->forecastTempHigh = xml.readElementText();
            } else if (xml.name() == "textSummary") {
                xml.readElementText();
            }
        }
    }
}

void EnvCanadaIon::parsePrecipitationForecast(WeatherData::ForecastInfo *forecast, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "precipitation");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "precipitation") {
            break;
        }

        if (xml.isStartElement()) {
            //kDebug() << "parsePrecipitationForecast() ====> TAG: " << xml.name().toString();
            if (xml.name() == "textSummary") {
                forecast->precipForecast = xml.readElementText();
            } else if (xml.name() == "precipType") {
                forecast->precipType = xml.readElementText();
            } else if (xml.name() == "accumulation") {
                parsePrecipTotals(forecast, xml);
            }
        }
    }
}

void EnvCanadaIon::parsePrecipTotals(WeatherData::ForecastInfo *forecast, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "accumulation");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "accumulation") {
            break;
        }

        if (xml.name() == "name") {
            xml.readElementText();
        } else if (xml.name() == "amount") {
            forecast->precipTotalExpected = xml.readElementText();
        }
    }
}

void EnvCanadaIon::parseWindForecast(WeatherData::ForecastInfo *forecast, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "winds");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "winds") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "textSummary") {
                forecast->windForecast = xml.readElementText();
            } else {
                if (xml.name() != "winds") {
                    parseUnknownElement(xml);
                }
            }
        }
    }
}

void EnvCanadaIon::parseYesterdayWeather(WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "yesterdayConditions");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "temperature" && xml.attributes().value("class") == "high") {
                data.prevHigh = xml.readElementText();
            } else if (xml.name() == "temperature" && xml.attributes().value("class") == "low") {
                data.prevLow = xml.readElementText();
            } else if (xml.name() == "precip") {
                data.prevPrecipType = xml.attributes().value("units").toString();
                if (data.prevPrecipType.isEmpty()) {
                    data.prevPrecipType = "N/A";
                }
                data.prevPrecipTotal = xml.readElementText();
            }
        }
    }
}

void EnvCanadaIon::parseWeatherRecords(WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "almanac");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "almanac") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "temperature" && xml.attributes().value("class") == "extremeMax") {
                data.recordHigh = xml.readElementText().toFloat();
            } else if (xml.name() == "temperature" && xml.attributes().value("class") == "extremeMin") {
                data.recordLow = xml.readElementText().toFloat();
            } else if (xml.name() == "precipitation" && xml.attributes().value("class") == "extremeRainfall") {
                data.recordRain = xml.readElementText().toFloat();
            } else if (xml.name() == "precipitation" && xml.attributes().value("class") == "extremeSnowfall") {
                data.recordSnow = xml.readElementText().toFloat();
            }
        }
    }
}

void EnvCanadaIon::parseAstronomicals(WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "riseSet");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "riseSet") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "disclaimer") {
                xml.readElementText();
            } else if (xml.name() == "dateTime") {
                parseDateTime(data, xml);
            }
        }
    }
}

// handle when no XML tag is found
void EnvCanadaIon::parseUnknownElement(QXmlStreamReader& xml)
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
void EnvCanadaIon::option(int option, QVariant value)
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
        } else {
           d->m_windInMeters = false;
        }
        break;
    }
}

void EnvCanadaIon::updateData()
{
    QVector<QString> sources;
    QMap<QString, QString> dataFields;
    QStringList fieldList;
    QVector<QString> forecastList;
    int i = 0;

    sources = this->ionSourceDict();
    foreach(QString keyname, sources) {
        // Does this place exist?
        if (!this->validLocation(keyname)) {
            kDebug() << "EnvCanadaIon::updateData() " << keyname << " is not a valid location";
            //setData(keyname, "Invalid Place", "Invalid location");
            continue;
        }
        setData(keyname, "Country", this->country(keyname));
        setData(keyname, "Place", QString("%1, %2").arg(this->city(keyname)).arg(this->territory(keyname)));
        setData(keyname, "Region", this->region(keyname));
        setData(keyname, "Airport Code", this->station(keyname));

        // Real weather - Current conditions
        setData(keyname, "Observations At", this->observationTime(keyname));
        setData(keyname, "Current Conditions", this->condition(keyname));
        dataFields = this->temperature(keyname);
        setData(keyname, "Temperature", dataFields["temperature"]);

        // Do we have a comfort temperature? if so display it
        if (dataFields["comfortTemperature"] != "N/A" && !dataFields["comfortTemperature"].isEmpty()) {
            if (dataFields["comfortTemperature"].toFloat() <= 0 || (dataFields["comfortTemperature"].toFloat() <= 32 && !d->m_useMetric)) {
                setData(keyname, "Windchill", QString("%1%2").arg(dataFields["comfortTemperature"]).arg(QChar(176)));
            } else {
                setData(keyname, "Humidex", QString("%1%2").arg(dataFields["comfortTemperature"]).arg(QChar(176)));
            }
        }

        setData(keyname, "Dewpoint", this->dewpoint(keyname));
        dataFields = this->pressure(keyname);
        setData(keyname, "Pressure", dataFields["pressure"]);
        setData(keyname, "Pressure Tendency", dataFields["pressureTendency"]);
        setData(keyname, "Visibility", this->visibility(keyname));
        setData(keyname, "Humidity", this->humidity(keyname));
        dataFields = this->wind(keyname);
        setData(keyname, "Wind Speed", dataFields["windSpeed"]);
	setData(keyname, "Wind Speed Unit", dataFields["windUnit"]);
        setData(keyname, "Wind Gust", dataFields["windGust"]);
	setData(keyname, "Wind Gust Unit", dataFields["windGustUnit"]);
        setData(keyname, "Wind Direction", dataFields["windDirection"]);

        dataFields = this->regionalTemperatures(keyname);
        setData(keyname, "Normal High", dataFields["normalHigh"]);
        setData(keyname, "Normal Low", dataFields["normalLow"]);

        // Check if UV index is available for the location
        dataFields = this->uvIndex(keyname);
        if (!dataFields["uvRating"].isEmpty()) {
            setData(keyname, "UV Rating", dataFields["uvRating"]);
            setData(keyname, "UV Index", dataFields["uvIndex"]);
        }

        dataFields = this->warnings(keyname);
        // Check if we have warnings or watches
        if (!dataFields["watch"].isEmpty()) {
            fieldList = dataFields["watch"].split('|');
            setData(keyname, "Watch Priority", fieldList[0]);
            setData(keyname, "Watch Description", fieldList[1]);
            setData(keyname, "Watch Info", fieldList[2]);
            setData(keyname, "Watch Timestamp", fieldList[3]);
        }
        if (!dataFields["warning"].isEmpty()) {
            fieldList = dataFields["warning"].split('|');
            setData(keyname, "Warning Priority", fieldList[0]);
            setData(keyname, "Warning Description", fieldList[1]);
            setData(keyname, "Warning Info", fieldList[2]);
            setData(keyname, "Warning Timestamp", fieldList[3]);
        }

        forecastList = this->forecasts(keyname);
        foreach(QString forecastItem, forecastList) {
            fieldList = forecastItem.split('|');

            if (d->m_useMetric) {
                setData(keyname, QString("Short Forecast Day %1").arg(i), QString("%1|%2|%3|%4|%5") \
                         .arg(fieldList[0]).arg(fieldList[1]).arg(fieldList[3]).arg(fieldList[4]).arg(fieldList[5]));

                setData(keyname, QString("Long Forecast Day %1").arg(i), QString("%1|%2|%3|%4|%5|%6|%7|%8") \
                         .arg(fieldList[0]).arg(fieldList[2]).arg(fieldList[3]).arg(fieldList[4]).arg(fieldList[6]) \
                         .arg(fieldList[7]).arg(fieldList[8]).arg(fieldList[9]));
            } else {
               setData(keyname, QString("Short Forecast Day %1").arg(i), QString("%1|%2|%3|%4|%5") \
                         .arg(fieldList[0]).arg(fieldList[1]).arg(fieldList[3] == "N/A" ? "N/A" : \
                         QString::number(d->m_formula.celsiusToF(fieldList[3].toFloat()), 'd', 0)) \
                         .arg(fieldList[4] == "N/A" ? "N/A" : QString::number(d->m_formula.celsiusToF(fieldList[4].toFloat()),'d',0)).arg(fieldList[5]));

               setData(keyname, QString("Long Forecast Day %1").arg(i), QString("%1|%2|%3|%4|%5|%6|%7|%8") \
                         .arg(fieldList[0]).arg(fieldList[2]).arg(fieldList[3] == "N/A" ? "N/A" : \
                         QString::number(d->m_formula.celsiusToF(fieldList[3].toFloat()),'d', 0)) \
                         .arg(fieldList[4] == "N/A" ? "N/A" : QString::number(d->m_formula.celsiusToF(fieldList[4].toFloat()),'d',0)).arg(fieldList[6]).arg(fieldList[7]) \
                         .arg(fieldList[8]).arg(fieldList[9]));
            }

            i++;
        }

        dataFields = this->yesterdayWeather(keyname);
        setData(keyname, "Yesterday High", dataFields["prevHigh"]);
        setData(keyname, "Yesterday Low", dataFields["prevLow"]);
        setData(keyname, "Yesterday Precip Total", dataFields["prevPrecip"]);
        setData(keyname, "Yesterday Precip Unit", dataFields["prevPrecipUnit"]);

        dataFields = this->sunriseSet(keyname);
        setData(keyname, "Sunrise At", dataFields["sunrise"]);
        setData(keyname, "Sunset At", dataFields["sunset"]);

        dataFields = this->moonriseSet(keyname);
        setData(keyname, "Moonrise At", dataFields["moonrise"]);
        setData(keyname, "Moonset At", dataFields["moonset"]);

        dataFields = this->weatherRecords(keyname);
        setData(keyname, "Record High Temperature", dataFields["recordHigh"]);
        setData(keyname, "Record Low Temperature", dataFields["recordLow"]);
        setData(keyname, "Record Rainfall", dataFields["recordRain"]);
        setData(keyname, "Record Rainfall Unit", dataFields["recordRainUnit"]);
        setData(keyname, "Record Snowfall", dataFields["recordSnow"]);
        setData(keyname, "Record Snowfall Unit", dataFields["recordSnowUnit"]);

        setData(keyname, "Credit", "Environment Canada");
        i = 0;
    }
}

QString EnvCanadaIon::country(QString key)
{
    return d->m_weatherData[key].countryName;
}
QString EnvCanadaIon::territory(QString key)
{
    return d->m_weatherData[key].shortTerritoryName;
}
QString EnvCanadaIon::city(QString key)
{
    return d->m_weatherData[key].cityName;
}
QString EnvCanadaIon::region(QString key)
{
    return d->m_weatherData[key].regionName;
}
QString EnvCanadaIon::station(QString key)
{
    if (!d->m_weatherData[key].stationID.isEmpty()) {
         return d->m_weatherData[key].stationID;
    }
    
    return QString("N/A");
}

QString EnvCanadaIon::observationTime(QString key)
{
    return d->m_weatherData[key].obsTimestamp;
}
QString EnvCanadaIon::condition(QString key)
{
    if (d->m_weatherData[key].condition.isEmpty()) {
        d->m_weatherData[key].condition = "N/A";
    }
    return d->m_weatherData[key].condition;
}

QString EnvCanadaIon::dewpoint(QString key)
{
    if (d->m_useMetric) {
        if (!d->m_weatherData[key].dewpoint.isEmpty()) {
            return QString("%1").arg(QString::number(d->m_weatherData[key].dewpoint.toFloat(), 'f', 1));
        }
    }
 
    if (!d->m_weatherData[key].dewpoint.isEmpty()) {
         return QString("%1").arg(QString::number(d->m_formula.celsiusToF(d->m_weatherData[key].dewpoint.toFloat()), 'f', 1));
    }

    return QString("N/A");
}

QString EnvCanadaIon::humidity(QString key)
{
    if (!d->m_weatherData[key].humidity.isEmpty()) {
        return QString("%1%").arg(d->m_weatherData[key].humidity);
    }
    return QString("N/A");
}

QString EnvCanadaIon::visibility(QString key)
{
    if (!d->m_weatherData[key].visibility == 0) {
        if (d->m_useMetric) {
            return QString("%1").arg(QString::number(d->m_weatherData[key].visibility, 'f', 1));
        } else {
            return QString("%1").arg(QString::number(d->m_formula.kilometersToMI(d->m_weatherData[key].visibility), 'f', 2));
        }
    }
    return QString("N/A");
}

QMap<QString, QString> EnvCanadaIon::temperature(QString key)
{
    QMap<QString, QString> temperatureInfo;
    if (d->m_useMetric) {
        if (!d->m_weatherData[key].temperature.isEmpty()) {
            temperatureInfo.insert("temperature", QString("%1").arg(QString::number(d->m_weatherData[key].temperature.toFloat(), 'f', 1)));
        }
    }
    else {
        if (!d->m_weatherData[key].temperature.isEmpty()) {
            temperatureInfo.insert("temperature", QString("%1").arg(QString::number(d->m_formula.celsiusToF(d->m_weatherData[key].temperature.toFloat()), 'f', 1)));
        } else {
            temperatureInfo.insert("temperature", "N/A");
        }
    }  
    temperatureInfo.insert("comfortTemperature", "N/A");

    if (d->m_weatherData[key].comforttemp != "N/A") {
        if (d->m_useMetric) {
            temperatureInfo.insert("comfortTemperature", d->m_weatherData[key].comforttemp);
        }
        else {
            if (!d->m_weatherData[key].comforttemp.isEmpty()) {
                temperatureInfo.insert("comfortTemperature", QString::number(d->m_formula.celsiusToF(d->m_weatherData[key].comforttemp.toFloat()), 'f', 1));
            }
        }
    }
    return temperatureInfo;
}

QMap<QString, QString> EnvCanadaIon::warnings(QString key)
{
    QMap<QString, QString> warningData;
    QString warnType;
    for (int i = 0; i < d->m_weatherData[key].warnings.size(); ++i) {
        if (d->m_weatherData[key].warnings[i]->type == "watch") {
            warnType = "watch";
        } else {
            warnType = "warning";
        }
        warningData[warnType] = QString("%1|%2|%3|%4").arg(d->m_weatherData[key].warnings[i]->priority) \
                                .arg(d->m_weatherData[key].warnings[i]->description) \
                                .arg(d->m_weatherData[key].warnings[i]->url) \
                                .arg(d->m_weatherData[key].warnings[i]->timestamp);
    }
    return warningData;
}

QVector<QString> EnvCanadaIon::forecasts(QString key)
{
    QVector<QString> forecastData;

    // Do some checks for empty data
    for (int i = 0; i < d->m_weatherData[key].forecasts.size(); ++i) {
        if (d->m_weatherData[key].forecasts[i]->forecastPeriod.isEmpty()) {
            d->m_weatherData[key].forecasts[i]->forecastPeriod = "N/A";
        }
        if (d->m_weatherData[key].forecasts[i]->shortForecast.isEmpty()) {
            d->m_weatherData[key].forecasts[i]->shortForecast = "N/A";
        }
        if (d->m_weatherData[key].forecasts[i]->forecastSummary.isEmpty()) {
            d->m_weatherData[key].forecasts[i]->forecastSummary = "N/A";
        }
        if (d->m_weatherData[key].forecasts[i]->forecastTempHigh.isEmpty()) {
            d->m_weatherData[key].forecasts[i]->forecastTempHigh = "N/A";
        }
        if (d->m_weatherData[key].forecasts[i]->forecastTempLow.isEmpty()) {
            d->m_weatherData[key].forecasts[i]->forecastTempLow = "N/A";
        }
        if (d->m_weatherData[key].forecasts[i]->popPrecent.isEmpty()) {
            d->m_weatherData[key].forecasts[i]->popPrecent = "N/A";
        }
        if (d->m_weatherData[key].forecasts[i]->windForecast.isEmpty()) {
            d->m_weatherData[key].forecasts[i]->windForecast = "N/A";
        }
        if (d->m_weatherData[key].forecasts[i]->precipForecast.isEmpty()) {
            d->m_weatherData[key].forecasts[i]->precipForecast = "N/A";
        }
        if (d->m_weatherData[key].forecasts[i]->precipType.isEmpty()) {
            d->m_weatherData[key].forecasts[i]->precipType = "N/A";
        }
        if (d->m_weatherData[key].forecasts[i]->precipTotalExpected.isEmpty()) {
            d->m_weatherData[key].forecasts[i]->precipTotalExpected = "N/A";
        }
    }

    for (int i = 0; i < d->m_weatherData[key].forecasts.size(); ++i) {
        forecastData.append(QString("%1|%2|%3|%4|%5|%6|%7|%8|%9|%10") \
                            .arg(d->m_weatherData[key].forecasts[i]->forecastPeriod) \
                            .arg(d->m_weatherData[key].forecasts[i]->shortForecast) \
                            .arg(d->m_weatherData[key].forecasts[i]->forecastSummary) \
                            .arg(d->m_weatherData[key].forecasts[i]->forecastTempHigh) \
                            .arg(d->m_weatherData[key].forecasts[i]->forecastTempLow) \
                            .arg(d->m_weatherData[key].forecasts[i]->popPrecent) \
                            .arg(d->m_weatherData[key].forecasts[i]->windForecast) \
                            .arg(d->m_weatherData[key].forecasts[i]->precipForecast) \
                            .arg(d->m_weatherData[key].forecasts[i]->precipType) \
                            .arg(d->m_weatherData[key].forecasts[i]->precipTotalExpected));
    }
    return forecastData;
}

QMap<QString, QString> EnvCanadaIon::pressure(QString key)
{
    QMap<QString, QString> pressureInfo;

    if (d->m_weatherData[key].pressure == 0) {
        pressureInfo.insert("pressure", "N/A");
        pressureInfo.insert("pressureTendency", "N/A");
    } else {
        if (d->m_useMetric) {
            pressureInfo.insert("pressure", QString("%1").arg(QString::number(d->m_weatherData[key].pressure, 'f', 1)));
        } else {
            pressureInfo.insert("pressure", QString("%1").arg(QString::number(d->m_formula.kilopascalsToInches(d->m_weatherData[key].pressure), 'f', 2)));
        }
        pressureInfo.insert("pressureTendency", d->m_weatherData[key].pressureTendency);
    }
    return pressureInfo;
}

QMap<QString, QString> EnvCanadaIon::wind(QString key)
{
    QMap<QString, QString> windInfo;

    // May not have any winds
    if (d->m_weatherData[key].windSpeed.isEmpty()) {
        windInfo.insert("windSpeed", "N/A");
        windInfo.insert("windUnit", "N/A");
    } else if (d->m_weatherData[key].windSpeed.toInt() == 0) {
        windInfo.insert("windSpeed", "Calm");
        windInfo.insert("windUnit", "N/A");
    } else {
        if (d->m_useMetric) {
            if (d->m_windInMeters) {
                windInfo.insert("windSpeed", QString("%1").arg(QString::number(d->m_formula.kilometersToMS(d->m_weatherData[key].windSpeed.toInt()), 'f', 2)));
                windInfo.insert("windUnit", "m/s");
            } else {
                windInfo.insert("windSpeed", QString("%1").arg(QString::number(d->m_weatherData[key].windSpeed.toInt())));
                windInfo.insert("windUnit", "km/h");
            }
        } else {
            windInfo.insert("windSpeed", QString("%1").arg(QString::number(d->m_formula.kilometersToMI(d->m_weatherData[key].windSpeed.toInt()), 'f', 1)));
            windInfo.insert("windUnit", "mph");
        }
    }

    // May not always have gusty winds
    if (d->m_weatherData[key].windGust.isEmpty()) {
        windInfo.insert("windGust", "N/A");
        windInfo.insert("windGustUnit", "N/A");
    } else {
        if (d->m_useMetric) {
            if (d->m_windInMeters) { 
                windInfo.insert("windGust", QString("%1").arg(QString::number(d->m_formula.kilometersToMS(d->m_weatherData[key].windGust.toInt()), 'f', 2)));
                windInfo.insert("windGustUnit", "m/s");
            } else { 
                windInfo.insert("windGust", QString("%1").arg(QString::number(d->m_weatherData[key].windGust.toInt())));
                windInfo.insert("windGustUnit", "km/h");
            }
        } else {
            windInfo.insert("windGust", QString("%1").arg(QString::number(d->m_formula.kilometersToMI(d->m_weatherData[key].windGust.toInt()), 'f', 1)));
            windInfo.insert("windGustUnit", "mph");
        }
    }

    if (d->m_weatherData[key].windDirection.isEmpty() && d->m_weatherData[key].windSpeed.isEmpty()) {
        windInfo.insert("windDirection", "N/A");
    } else if (d->m_weatherData[key].windSpeed.toInt() == 0) {
        windInfo.insert("windDirection", "VR");
    } else {
        windInfo.insert("windDirection", d->m_weatherData[key].windDirection);
    }
    return windInfo;
}

QMap<QString, QString> EnvCanadaIon::uvIndex(QString key)
{
    QMap<QString, QString> uvInfo;

    if (d->m_weatherData[key].UVRating.isEmpty()) {
        uvInfo.insert("uvRating", "N/A");
    } else {
        uvInfo.insert("uvRating", d->m_weatherData[key].UVRating);
    }

    if (d->m_weatherData[key].UVIndex.isEmpty()) {
        uvInfo.insert("uvIndex", "N/A");
    } else {
        uvInfo.insert("uvIndex", d->m_weatherData[key].UVIndex);
    }

    return uvInfo;
}

QMap<QString, QString> EnvCanadaIon::regionalTemperatures(QString key)
{
    QMap<QString, QString> regionalTempInfo;

    if (d->m_weatherData[key].normalHigh.isEmpty()) {
        regionalTempInfo.insert("normalHigh", "N/A");
    } else {
        if (d->m_useMetric) {
            regionalTempInfo.insert("normalHigh", QString("%1").arg(d->m_weatherData[key].normalHigh));
        } else {
            regionalTempInfo.insert("normalHigh", QString("%1").arg(d->m_formula.celsiusToF(d->m_weatherData[key].normalHigh.toFloat())));
        }
    }

    if (d->m_weatherData[key].normalLow.isEmpty()) {
        regionalTempInfo.insert("normalLow", "N/A");
    } else {
        if (d->m_useMetric) {
            regionalTempInfo.insert("normalLow", QString("%1").arg(d->m_weatherData[key].normalLow));
        } else {
            regionalTempInfo.insert("normalLow", QString("%1").arg(d->m_formula.celsiusToF(d->m_weatherData[key].normalLow.toFloat())));
        }
    }

    return regionalTempInfo;
}

QMap<QString, QString> EnvCanadaIon::yesterdayWeather(QString key)
{
    QMap<QString, QString> yesterdayInfo;

    if (d->m_weatherData[key].prevHigh.isEmpty()) {
        yesterdayInfo.insert("prevHigh", "N/A");
    } else {
        if (d->m_useMetric) {
            yesterdayInfo.insert("prevHigh", QString("%1").arg(d->m_weatherData[key].prevHigh)); 
        } else {
            yesterdayInfo.insert("prevHigh", QString("%1").arg(QString::number(d->m_formula.celsiusToF(d->m_weatherData[key].prevHigh.toFloat()))));
        }
    }

    if (d->m_weatherData[key].prevLow.isEmpty()) {
        yesterdayInfo.insert("prevLow", "N/A");
    } else {
        if (d->m_useMetric) {
            yesterdayInfo.insert("prevLow", QString("%1").arg(d->m_weatherData[key].prevLow));
        } else {
            yesterdayInfo.insert("prevLow", QString("%1").arg(QString::number(d->m_formula.celsiusToF(d->m_weatherData[key].prevLow.toFloat()), 'f', 1)));
        }
    }

    if (d->m_weatherData[key].prevPrecipTotal == "Trace") {
            yesterdayInfo.insert("prevPrecip", "Trace");
            return yesterdayInfo;
    }

    if (d->m_weatherData[key].prevPrecipTotal.isEmpty()) {
        yesterdayInfo.insert("prevPrecip", "N/A");
    } else {
        if (d->m_useMetric) {
            yesterdayInfo.insert("prevPrecipTotal", QString("%1").arg(d->m_weatherData[key].prevPrecipTotal));
            yesterdayInfo.insert("prevPrecipUnit", d->m_weatherData[key].prevPrecipType);
        } else {
            yesterdayInfo.insert("prevPrecipTotal", QString("%1").arg(QString::number(d->m_formula.millimetersToIN(d->m_weatherData[key].prevPrecipTotal.toFloat()), 'f', 1)));
            yesterdayInfo.insert("prevPrecipUnit", QString("in"));
        }
    }

    return yesterdayInfo;
}

QMap<QString, QString> EnvCanadaIon::sunriseSet(QString key)
{
    QMap<QString, QString> sunInfo;
  
    if (d->m_weatherData[key].sunriseTimestamp.isEmpty()) {
        sunInfo.insert("sunrise", "N/A");
    } else {
        sunInfo.insert("sunrise", d->m_weatherData[key].sunriseTimestamp);
    }
 
    if (d->m_weatherData[key].sunsetTimestamp.isEmpty()) {
        sunInfo.insert("sunset", "N/A");
    } else {
        sunInfo.insert("sunset", d->m_weatherData[key].sunsetTimestamp);
    }

    return sunInfo;
}

QMap<QString, QString> EnvCanadaIon::moonriseSet(QString key)
{
    QMap<QString, QString> moonInfo;
 
    if (d->m_weatherData[key].moonriseTimestamp.isEmpty()) {
        moonInfo.insert("moonrise", "N/A");
    } else {
        moonInfo.insert("moonrise", d->m_weatherData[key].moonriseTimestamp);
    }
   
    if (d->m_weatherData[key].moonsetTimestamp.isEmpty()) {
        moonInfo.insert("moonset", "N/A");
    } else {
        moonInfo.insert("moonset", d->m_weatherData[key].moonsetTimestamp);
    }
   
    return moonInfo;
}

QMap<QString, QString> EnvCanadaIon::weatherRecords(QString key)
{
    QMap<QString, QString> recordInfo;

    if (d->m_weatherData[key].recordHigh == 0) {
        recordInfo.insert("recordHigh", "N/A");
    } else {
        if (d->m_useMetric) {
            recordInfo.insert("recordHigh", QString("%1").arg(d->m_weatherData[key].recordHigh));
        } else {
            recordInfo.insert("recordHigh", QString("%1").arg(QString::number(d->m_formula.celsiusToF(d->m_weatherData[key].recordHigh), 'f', 1)));
        }
    }

    if (d->m_weatherData[key].recordLow == 0) {
        recordInfo.insert("recordLow", "N/A");
    } else {
        if (d->m_useMetric) {
            recordInfo.insert("recordLow", QString("%1").arg(d->m_weatherData[key].recordLow));
        } else {
            recordInfo.insert("recordLow", QString("%1").arg(QString::number(d->m_formula.celsiusToF(d->m_weatherData[key].recordLow), 'f', 1)));
        }
       
    }

    if (d->m_weatherData[key].recordRain == 0) {
        recordInfo.insert("recordRain", "N/A");
    } else {
        if (d->m_useMetric) {
            recordInfo.insert("recordRain", QString("%1").arg(d->m_weatherData[key].recordRain));
            recordInfo.insert("recordRainUnit", QString("mm"));
        } else {
            recordInfo.insert("recordRain", QString("%1").arg(QString::number(d->m_formula.millimetersToIN(d->m_weatherData[key].recordRain), 'f', 1)));
            recordInfo.insert("recordRainUnit", QString("in"));
        }
    }

    if (d->m_weatherData[key].recordSnow == 0) {
        recordInfo.insert("recordSnow", "N/A");
    } else {
        if (d->m_useMetric) {
            recordInfo.insert("recordSnow", QString("%1").arg(d->m_weatherData[key].recordSnow));
            recordInfo.insert("recordSnowUnit", QString("cm"));
        } else {
            recordInfo.insert("recordSnow", QString("%1").arg(QString::number(d->m_formula.centimetersToIN(d->m_weatherData[key].recordSnow), 'f', 1)));
            recordInfo.insert("recordSnowUnit", QString("in"));
        }
    }

    return recordInfo;
}
 
#include "ion_envcan.moc"
