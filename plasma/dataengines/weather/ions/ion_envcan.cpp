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
        QString sourceOptions;
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

    int m_timezoneType;  // Ion option: Timezone may be local time or UTC time
    int m_measureType; // Ion option: Units may be Metric or Imperial
    bool m_windInMeters; // Ion option: Display wind format in meters per second only
    bool m_windInKnots; // Ion option: Display wind format in knots
    bool m_windInBft; // ion option: Display wind by the beaufort scale model
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

QStringList EnvCanadaIon::validate(const QString& source) const
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

// Get a specific Ion's data
bool EnvCanadaIon::updateIonSource(const QString& source)
{
    kDebug() << "updateIonSource() SOURCE: " << source;
    // We expect the applet to send the source in the following tokenization:
    // ionname|validate|place_name - Triggers validation of place
    // ionname|weather|place_name - Triggers receiving weather of place

    QStringList sourceAction = source.split('|');
    if (sourceAction[1] == QString("validate")) {
        kDebug() << "Initiate Validating of place: " << sourceAction[2];

        QStringList result = validate(QString("%1|%2").arg(sourceAction[0]).arg(sourceAction[2]));

        if (result.size() == 1) {
            setData(source, "validate", QString("envcan|valid|single|%1").arg(result.join("|")));
            return true;
        } else if (result.size() > 1) {
            setData(source, "validate", QString("envcan|valid|multiple|%1").arg(result.join("|")));
            return true;
        } else if (result.size() == 0) {
            setData(source, "validate", QString("envcan|invalid|single|%1").arg(sourceAction[2]));
            return true;
        }

    } else if (sourceAction[1] == QString("weather")) {
        getXMLData(QString("%1|%2").arg(sourceAction[0]).arg(sourceAction[2]));
        return true;
    }
    return false;
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
void EnvCanadaIon::getXMLData(const QString& source)
{
    KUrl url;
    url = "http://dd.weatheroffice.ec.gc.ca/EC_sites/xml/" + d->m_place[source].territoryName + "/" + d->m_place[source].cityCode + "_e.xml";

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

void EnvCanadaIon::setup_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job)

    if (data.isEmpty()) {
        return;
    }

    // Send to xml.
    d->m_xmlSetup.addData(data);
}

void EnvCanadaIon::slotDataArrived(KIO::Job *job, const QByteArray &data)
{

    if (data.isEmpty() || !d->m_jobXml.contains(job)) {
        return;
    }

    // Send to xml.
    d->m_jobXml[job]->addData(data);
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
    setInitialized(true);
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
                tmp = "envcan|" + d->m_cityName + ", " + d->m_territory; // Build the key name.

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
                // Cleanup warning list on update
                data.warnings.clear();
                parseWarnings(data, xml);
            } else if (xml.name() == "currentConditions") {
                parseConditions(data, xml);
            } else if (xml.name() == "forecastGroup") {
                // Clean up forecast list on update
                data.forecasts.clear();
                parseWeatherForecast(data, xml);
            } else if (xml.name() == "yesterdayConditions") {
                parseYesterdayWeather(data, xml);
            } else if (xml.name() == "riseSet") {
                parseAstronomicals(data, xml);
            } else if (xml.name() == "almanac") {
                parseWeatherRecords(data, xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }
    return data;
}

// Parse Weather data main loop, from here we have to decend into each tag pair
bool EnvCanadaIon::readXMLData(const QString& source, QXmlStreamReader& xml)
{
    WeatherData data;
    data.comforttemp = "N/A";
    data.recordHigh = 0.0;
    data.recordLow = 0.0;
    data.shortTerritoryName = d->m_place[source].territoryName;
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

    d->m_weatherData[source] = data;
    updateWeather(source);
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
            else if (xml.name() == "timeStamp")
                xml.readElementText();
            else if (xml.name() == "textSummary") {
                if (timezone() && dateZone == "UTC") {
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

void EnvCanadaIon::setMeasureUnit(const QString& unitType)
{
    d->m_measureType = unitType.toInt();
}

void EnvCanadaIon::setTimezoneFormat(const QString& tz)
{
    d->m_timezoneType = tz.toInt(); // Boolean
}

bool EnvCanadaIon::metricUnit()
{
    if (d->m_measureType == KLocale::Metric) {
        return true;
    }

    // Imperial units
    return false;
}

bool EnvCanadaIon::timezone()
{
    if (d->m_timezoneType) {
        return true;
    }

    // Not UTC, local time
    return false;
}

// Custom options, for now this just handles wind type.
bool EnvCanadaIon::options(const QString& source)
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

void EnvCanadaIon::updateWeather(const QString& source)
{
    QMap<QString, QString> dataFields;
    QStringList fieldList;
    QVector<QString> forecastList;
    int i = 0;

    QString weatherSource = source;
    weatherSource.replace("envcan|", "envcan|weather|");
    options(weatherSource);

    setData(weatherSource, "Country", country(source));
    setData(weatherSource, "Place", QString("%1, %2").arg(city(source)).arg(territory(source)));
    setData(weatherSource, "Region", region(source));
    setData(weatherSource, "Station", station(source));

    // Real weather - Current conditions
    setData(weatherSource, "Observation Period", observationTime(source));
    setData(weatherSource, "Current Conditions", condition(source));
    dataFields = temperature(source);
    setData(weatherSource, "Temperature", dataFields["temperature"]);

    // Do we have a comfort temperature? if so display it
    if (dataFields["comfortTemperature"] != "N/A" && !dataFields["comfortTemperature"].isEmpty()) {
        if (dataFields["comfortTemperature"].toFloat() <= 0 || (dataFields["comfortTemperature"].toFloat() <= 32 && !metricUnit())) {
            setData(weatherSource, "Windchill", QString("%1%2").arg(dataFields["comfortTemperature"]).arg(QChar(176)));
            setData(weatherSource, "Humidex", "N/A");
        } else {
            setData(weatherSource, "Humidex", QString("%1%2").arg(dataFields["comfortTemperature"]).arg(QChar(176)));
            setData(weatherSource, "Windchill", "N/A");
        }
    } else {
        setData(weatherSource, "Windchill", "N/A");
        setData(weatherSource, "Humidex", "N/A");
    }

    setData(weatherSource, "Temperature Unit", dataFields["temperatureUnit"]);

    setData(weatherSource, "Dewpoint", dewpoint(source));
    if (dewpoint(source) != "N/A") {
        setData(weatherSource, "Dewpoint Unit", dataFields["temperatureUnit"]);
    }

    dataFields = pressure(source);
    setData(weatherSource, "Pressure", dataFields["pressure"]);

    if (dataFields["pressure"] != "N/A") {
        setData(weatherSource, "Pressure Tendency", dataFields["pressureTendency"]);
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
    if (dataFields["windSpeed"] != "N/A") {
        setData(weatherSource, "Wind Speed Unit", dataFields["windUnit"]);
    }
    setData(weatherSource, "Wind Gust", dataFields["windGust"]);
    setData(weatherSource, "Wind Direction", dataFields["windDirection"]);
    setData(weatherSource, "Wind Gust Unit", dataFields["windGustUnit"]);

    dataFields = regionalTemperatures(source);
    setData(weatherSource, "Normal High", dataFields["normalHigh"]);
    setData(weatherSource, "Normal Low", dataFields["normalLow"]);
    if (dataFields["normalHigh"] != "N/A" && dataFields["normalLow"] != "N/A") {
        setData(weatherSource, "Regional Temperature Unit", dataFields["regionalTempUnit"]);
    }

    // Check if UV index is available for the location
    dataFields = uvIndex(source);
    setData(weatherSource, "UV Index", dataFields["uvIndex"]);
    if (dataFields["uvIndex"] != "N/A") {
        setData(weatherSource, "UV Rating", dataFields["uvRating"]);
    }

    dataFields = warnings(source);
    // Check if we have warnings or watches

    for (int i = 0; i < EnvCanadaIon::MAX_WARNINGS; i++) {
        if (!dataFields[QString("watch %1").arg(i)].isEmpty()) {
            fieldList = dataFields[QString("watch %1").arg(i)].split('|');
            setData(weatherSource, QString("Watch Priority %1").arg(i), fieldList[0]);
            setData(weatherSource, QString("Watch Description %1").arg(i), fieldList[1]);
            setData(weatherSource, QString("Watch Info %1").arg(i), fieldList[2]);
            setData(weatherSource, QString("Watch Timestamp %1").arg(i), fieldList[3]);
        }
        if (!dataFields[QString("warning %1").arg(i)].isEmpty()) {
            fieldList = dataFields[QString("warning %1").arg(i)].split('|');
            setData(weatherSource, QString("Warning Priority %1").arg(i), fieldList[0]);
            setData(weatherSource, QString("Warning Description %1").arg(i), fieldList[1]);
            setData(weatherSource, QString("Warning Info %1").arg(i), fieldList[2]);
            setData(weatherSource, QString("Warning Timestamp %1").arg(i), fieldList[3]);
        }
    }

    forecastList = forecasts(source);
    foreach(QString forecastItem, forecastList) {
        fieldList = forecastItem.split('|');

        // TODO: We don't convert the wind format (Knots, meteres per second, bft) for the Long Forecast yet. These are not used in the applet (for now).
        if (metricUnit()) {
            setData(weatherSource, QString("Short Forecast Day %1").arg(i), QString("%1|%2|%3|%4|%5") \
                    .arg(fieldList[0]).arg(fieldList[1]).arg(fieldList[3]).arg(fieldList[4]).arg(fieldList[5]));

            setData(weatherSource, QString("Long Forecast Day %1").arg(i), QString("%1|%2|%3|%4|%5|%6|%7|%8") \
                    .arg(fieldList[0]).arg(fieldList[2]).arg(fieldList[3]).arg(fieldList[4]).arg(fieldList[6]) \
                    .arg(fieldList[7]).arg(fieldList[8]).arg(fieldList[9]));
        } else {
            setData(weatherSource, QString("Short Forecast Day %1").arg(i), QString("%1|%2|%3|%4|%5") \
                    .arg(fieldList[0]).arg(fieldList[1]).arg(fieldList[3] == "N/A" ? "N/A" : \
                            QString::number(d->m_formula.celsiusToF(fieldList[3].toFloat()), 'd', 0)) \
                    .arg(fieldList[4] == "N/A" ? "N/A" : QString::number(d->m_formula.celsiusToF(fieldList[4].toFloat()), 'd', 0)).arg(fieldList[5]));

            setData(weatherSource, QString("Long Forecast Day %1").arg(i), QString("%1|%2|%3|%4|%5|%6|%7|%8") \
                    .arg(fieldList[0]).arg(fieldList[2]).arg(fieldList[3] == "N/A" ? "N/A" : \
                            QString::number(d->m_formula.celsiusToF(fieldList[3].toFloat()), 'd', 0)) \
                    .arg(fieldList[4] == "N/A" ? "N/A" : QString::number(d->m_formula.celsiusToF(fieldList[4].toFloat()), 'd', 0)).arg(fieldList[6]).arg(fieldList[7]) \
                    .arg(fieldList[8]).arg(fieldList[9]));
        }

        i++;
    }

    dataFields = yesterdayWeather(source);
    setData(weatherSource, "Yesterday High", dataFields["prevHigh"]);
    setData(weatherSource, "Yesterday Low", dataFields["prevLow"]);

    if (dataFields["prevHigh"] != "N/A" && dataFields["prevLow"] != "N/A") {
        setData(weatherSource , "Yesterday Temperature Unit", dataFields["yesterdayTempUnit"]);
    }

    setData(weatherSource, "Yesterday Precip Total", dataFields["prevPrecip"]);
    setData(weatherSource, "Yesterday Precip Unit", dataFields["prevPrecipUnit"]);

    dataFields = sunriseSet(source);
    setData(weatherSource, "Sunrise At", dataFields["sunrise"]);
    setData(weatherSource, "Sunset At", dataFields["sunset"]);

    dataFields = moonriseSet(source);
    setData(weatherSource, "Moonrise At", dataFields["moonrise"]);
    setData(weatherSource, "Moonset At", dataFields["moonset"]);

    dataFields = weatherRecords(source);
    setData(weatherSource, "Record High Temperature", dataFields["recordHigh"]);
    setData(weatherSource, "Record Low Temperature", dataFields["recordLow"]);
    if (dataFields["recordHigh"] != "N/A" && dataFields["recordLow"] != "N/A") {
        setData(weatherSource, "Record Temperature Unit", dataFields["recordTempUnit"]);
    }

    setData(weatherSource, "Record Rainfall", dataFields["recordRain"]);
    setData(weatherSource, "Record Rainfall Unit", dataFields["recordRainUnit"]);
    setData(weatherSource, "Record Snowfall", dataFields["recordSnow"]);
    setData(weatherSource, "Record Snowfall Unit", dataFields["recordSnowUnit"]);

    setData(weatherSource, "Credit", "Meteorological data is provided by Environment Canada");
}

QString EnvCanadaIon::country(const QString& source)
{
    return d->m_weatherData[source].countryName;
}
QString EnvCanadaIon::territory(const QString& source)
{
    return d->m_weatherData[source].shortTerritoryName;
}
QString EnvCanadaIon::city(const QString& source)
{
    return d->m_weatherData[source].cityName;
}
QString EnvCanadaIon::region(const QString& source)
{
    return d->m_weatherData[source].regionName;
}
QString EnvCanadaIon::station(const QString& source)
{
    if (!d->m_weatherData[source].stationID.isEmpty()) {
        return d->m_weatherData[source].stationID.toUpper();
    }

    return QString("N/A");
}

QString EnvCanadaIon::observationTime(const QString& source)
{
    return d->m_weatherData[source].obsTimestamp;
}
QString EnvCanadaIon::condition(const QString& source)
{
    if (d->m_weatherData[source].condition.isEmpty()) {
        d->m_weatherData[source].condition = "N/A";
    }
    return d->m_weatherData[source].condition;
}

QString EnvCanadaIon::dewpoint(const QString& source)
{
    if (metricUnit()) {
        if (!d->m_weatherData[source].dewpoint.isEmpty()) {
            return QString::number(d->m_weatherData[source].dewpoint.toFloat(), 'f', 1);
        }
    }

    if (!d->m_weatherData[source].dewpoint.isEmpty()) {
        return QString::number(d->m_formula.celsiusToF(d->m_weatherData[source].dewpoint.toFloat()), 'f', 1);
    }

    return QString("N/A");
}

QString EnvCanadaIon::humidity(const QString& source)
{
    if (!d->m_weatherData[source].humidity.isEmpty()) {
        return QString("%1%").arg(d->m_weatherData[source].humidity);
    }
    return QString("N/A");
}

QMap<QString, QString> EnvCanadaIon::visibility(const QString& source)
{
    QMap<QString, QString> visibilityInfo;

    if (!d->m_weatherData[source].visibility == 0) {
        if (metricUnit()) {
            visibilityInfo.insert("visibility", QString::number(d->m_weatherData[source].visibility, 'f', 1));
            visibilityInfo.insert("visibilityUnit", "km");
        } else {
            visibilityInfo.insert("visibility", QString::number(d->m_formula.kilometersToMI(d->m_weatherData[source].visibility), 'f', 2));
            visibilityInfo.insert("visibilityUnit", "mi");
        }
    } else {
        visibilityInfo.insert("visibility", "N/A");
    }
    return visibilityInfo;
}

QMap<QString, QString> EnvCanadaIon::temperature(const QString& source)
{
    QMap<QString, QString> temperatureInfo;
    if (metricUnit()) {
        if (!d->m_weatherData[source].temperature.isEmpty()) {
            temperatureInfo.insert("temperature", QString::number(d->m_weatherData[source].temperature.toFloat(), 'f', 1));
        }
        temperatureInfo.insert("temperatureUnit", QString("%1C").arg(QChar(176)));
    } else {
        if (!d->m_weatherData[source].temperature.isEmpty()) {
            temperatureInfo.insert("temperature", QString::number(d->m_formula.celsiusToF(d->m_weatherData[source].temperature.toFloat()), 'f', 1));
        } else {
            temperatureInfo.insert("temperature", "N/A");
        }
        temperatureInfo.insert("temperatureUnit", QString("%1F").arg(QChar(176)));
    }
    temperatureInfo.insert("comfortTemperature", "N/A");

    if (d->m_weatherData[source].comforttemp != "N/A") {
        if (metricUnit()) {
            temperatureInfo.insert("comfortTemperature", d->m_weatherData[source].comforttemp);
        } else {
            if (!d->m_weatherData[source].comforttemp.isEmpty()) {
                temperatureInfo.insert("comfortTemperature", QString::number(d->m_formula.celsiusToF(d->m_weatherData[source].comforttemp.toFloat()), 'f', 1));
            }
        }
    }
    return temperatureInfo;
}

QMap<QString, QString> EnvCanadaIon::warnings(const QString& source)
{
    QMap<QString, QString> warningData;
    QString warnType;
    for (int i = 0; i < d->m_weatherData[source].warnings.size(); ++i) {
        if (d->m_weatherData[source].warnings[i]->type == "watch") {
            warnType = QString("watch %1").arg(i);
        } else {
            warnType = QString("warning %1").arg(i);
        }
        warningData[warnType] = QString("%1|%2|%3|%4").arg(d->m_weatherData[source].warnings[i]->priority) \
                                .arg(d->m_weatherData[source].warnings[i]->description) \
                                .arg(d->m_weatherData[source].warnings[i]->url) \
                                .arg(d->m_weatherData[source].warnings[i]->timestamp);
    }
    return warningData;
}

QVector<QString> EnvCanadaIon::forecasts(const QString& source)
{
    QVector<QString> forecastData;

    // Do some checks for empty data
    for (int i = 0; i < d->m_weatherData[source].forecasts.size(); ++i) {
        if (d->m_weatherData[source].forecasts[i]->forecastPeriod.isEmpty()) {
            d->m_weatherData[source].forecasts[i]->forecastPeriod = "N/A";
        }
        if (d->m_weatherData[source].forecasts[i]->shortForecast.isEmpty()) {
            d->m_weatherData[source].forecasts[i]->shortForecast = "N/A";
        }
        if (d->m_weatherData[source].forecasts[i]->forecastSummary.isEmpty()) {
            d->m_weatherData[source].forecasts[i]->forecastSummary = "N/A";
        }
        if (d->m_weatherData[source].forecasts[i]->forecastTempHigh.isEmpty()) {
            d->m_weatherData[source].forecasts[i]->forecastTempHigh = "N/A";
        }
        if (d->m_weatherData[source].forecasts[i]->forecastTempLow.isEmpty()) {
            d->m_weatherData[source].forecasts[i]->forecastTempLow = "N/A";
        }
        if (d->m_weatherData[source].forecasts[i]->popPrecent.isEmpty()) {
            d->m_weatherData[source].forecasts[i]->popPrecent = "N/A";
        }
        if (d->m_weatherData[source].forecasts[i]->windForecast.isEmpty()) {
            d->m_weatherData[source].forecasts[i]->windForecast = "N/A";
        }
        if (d->m_weatherData[source].forecasts[i]->precipForecast.isEmpty()) {
            d->m_weatherData[source].forecasts[i]->precipForecast = "N/A";
        }
        if (d->m_weatherData[source].forecasts[i]->precipType.isEmpty()) {
            d->m_weatherData[source].forecasts[i]->precipType = "N/A";
        }
        if (d->m_weatherData[source].forecasts[i]->precipTotalExpected.isEmpty()) {
            d->m_weatherData[source].forecasts[i]->precipTotalExpected = "N/A";
        }
    }

    for (int i = 0; i < d->m_weatherData[source].forecasts.size(); ++i) {
        forecastData.append(QString("%1|%2|%3|%4|%5|%6|%7|%8|%9|%10") \
                            .arg(d->m_weatherData[source].forecasts[i]->forecastPeriod) \
                            .arg(d->m_weatherData[source].forecasts[i]->shortForecast) \
                            .arg(d->m_weatherData[source].forecasts[i]->forecastSummary) \
                            .arg(d->m_weatherData[source].forecasts[i]->forecastTempHigh) \
                            .arg(d->m_weatherData[source].forecasts[i]->forecastTempLow) \
                            .arg(d->m_weatherData[source].forecasts[i]->popPrecent) \
                            .arg(d->m_weatherData[source].forecasts[i]->windForecast) \
                            .arg(d->m_weatherData[source].forecasts[i]->precipForecast) \
                            .arg(d->m_weatherData[source].forecasts[i]->precipType) \
                            .arg(d->m_weatherData[source].forecasts[i]->precipTotalExpected));
    }
    return forecastData;
}

QMap<QString, QString> EnvCanadaIon::pressure(const QString& source)
{
    QMap<QString, QString> pressureInfo;

    if (d->m_weatherData[source].pressure == 0) {
        pressureInfo.insert("pressure", "N/A");
        return pressureInfo;
    } else {
        if (metricUnit()) {
            pressureInfo.insert("pressure", QString::number(d->m_weatherData[source].pressure, 'f', 1));
            pressureInfo.insert("pressureUnit", "kPa");
        } else {
            pressureInfo.insert("pressure", QString::number(d->m_formula.kilopascalsToInches(d->m_weatherData[source].pressure), 'f', 2));
            pressureInfo.insert("pressureUnit", "in");
        }
        pressureInfo.insert("pressureTendency", d->m_weatherData[source].pressureTendency);
    }
    return pressureInfo;
}

QMap<QString, QString> EnvCanadaIon::wind(const QString& source)
{
    QMap<QString, QString> windInfo;

    // May not have any winds
    if (d->m_weatherData[source].windSpeed.isEmpty()) {
        windInfo.insert("windSpeed", "N/A");
        windInfo.insert("windUnit", "N/A");
    } else if (d->m_weatherData[source].windSpeed.toInt() == 0) {
        windInfo.insert("windSpeed", "Calm");
        windInfo.insert("windUnit", "N/A");
    } else {
        if (metricUnit()) {
            if (d->m_windInMeters) {
                windInfo.insert("windSpeed", QString::number(d->m_formula.kilometersToMS(d->m_weatherData[source].windSpeed.toInt()), 'f', 2));
                windInfo.insert("windUnit", "m/s");
            } else if (d->m_windInKnots) {
                windInfo.insert("windSpeed", QString::number(d->m_formula.kilometersToKT(d->m_weatherData[source].windSpeed.toFloat()), 'f', 1));
                windInfo.insert("windUnit", "kt");
            } else if (d->m_windInBft) {
                windInfo.insert("windSpeed", QString::number(d->m_formula.kilometersToBF(d->m_weatherData[source].windSpeed.toInt())));
                windInfo.insert("windUnit", "bft");
            } else {
                windInfo.insert("windSpeed", QString::number(d->m_weatherData[source].windSpeed.toInt()));
                windInfo.insert("windUnit", "km/h");
            }
        } else {
            if (d->m_windInKnots) {
                windInfo.insert("windSpeed", QString::number(d->m_formula.kilometersToKT(d->m_weatherData[source].windSpeed.toFloat()), 'f', 1));
                windInfo.insert("windUnit", "kt");
            } else if (d->m_windInBft) {
                windInfo.insert("windSpeed", QString::number(d->m_formula.kilometersToBF(d->m_weatherData[source].windSpeed.toInt())));
                windInfo.insert("windUnit", "bft");
            } else {
                windInfo.insert("windSpeed", QString::number(d->m_formula.kilometersToMI(d->m_weatherData[source].windSpeed.toInt()), 'f', 1));
                windInfo.insert("windUnit", "mph");
            }
        }
    }

    // May not always have gusty winds
    if (d->m_weatherData[source].windGust.isEmpty()) {
        windInfo.insert("windGust", "N/A");
        windInfo.insert("windGustUnit", "N/A");
    } else {
        if (metricUnit()) {
            if (d->m_windInMeters) {
                windInfo.insert("windGust", QString::number(d->m_formula.kilometersToMS(d->m_weatherData[source].windGust.toInt()), 'f', 2));
                windInfo.insert("windGustUnit", "m/s");
            } else if (d->m_windInKnots) {
                windInfo.insert("windGust", QString::number(d->m_formula.kilometersToKT(d->m_weatherData[source].windGust.toFloat()), 'f', 1));
                windInfo.insert("windGustUnit", "kt");
            } else if (d->m_windInBft) {
                windInfo.insert("windGust", QString::number(d->m_formula.kilometersToBF(d->m_weatherData[source].windGust.toInt())));
                windInfo.insert("windGustUnit", "bft");
            } else {
                windInfo.insert("windGust", QString::number(d->m_weatherData[source].windGust.toInt()));
                windInfo.insert("windGustUnit", "km/h");
            }
        } else {
            if (d->m_windInKnots) {
                windInfo.insert("windGust", QString::number(d->m_formula.kilometersToKT(d->m_weatherData[source].windGust.toInt()), 'f', 1));
                windInfo.insert("windGustUnit", "kt");
            } else if (d->m_windInBft) {
                windInfo.insert("windGust", QString::number(d->m_formula.kilometersToBF(d->m_weatherData[source].windGust.toInt())));
                windInfo.insert("windGustUnit", "bft");
            } else {
                windInfo.insert("windGust", QString::number(d->m_formula.kilometersToMI(d->m_weatherData[source].windGust.toInt()), 'f', 1));
                windInfo.insert("windGustUnit", "mph");
            }
        }
    }

    if (d->m_weatherData[source].windDirection.isEmpty() && d->m_weatherData[source].windSpeed.isEmpty()) {
        windInfo.insert("windDirection", "N/A");
    } else if (d->m_weatherData[source].windSpeed.toInt() == 0) {
        windInfo.insert("windDirection", "VR");
    } else {
        windInfo.insert("windDirection", d->m_weatherData[source].windDirection);
    }
    return windInfo;
}

QMap<QString, QString> EnvCanadaIon::uvIndex(const QString& source)
{
    QMap<QString, QString> uvInfo;

    if (d->m_weatherData[source].UVRating.isEmpty()) {
        uvInfo.insert("uvRating", "N/A");
    } else {
        uvInfo.insert("uvRating", d->m_weatherData[source].UVRating);
    }

    if (d->m_weatherData[source].UVIndex.isEmpty()) {
        uvInfo.insert("uvIndex", "N/A");
    } else {
        uvInfo.insert("uvIndex", d->m_weatherData[source].UVIndex);
    }

    return uvInfo;
}

QMap<QString, QString> EnvCanadaIon::regionalTemperatures(const QString& source)
{
    QMap<QString, QString> regionalTempInfo;

    if (d->m_weatherData[source].normalHigh.isEmpty()) {
        regionalTempInfo.insert("normalHigh", "N/A");
    } else {
        if (metricUnit()) {
            regionalTempInfo.insert("normalHigh", d->m_weatherData[source].normalHigh);
        } else {
            regionalTempInfo.insert("normalHigh", QString("%1").arg(d->m_formula.celsiusToF(d->m_weatherData[source].normalHigh.toFloat())));
        }
    }

    if (d->m_weatherData[source].normalLow.isEmpty()) {
        regionalTempInfo.insert("normalLow", "N/A");
    } else {
        if (metricUnit()) {
            regionalTempInfo.insert("normalLow", d->m_weatherData[source].normalLow);
        } else {
            regionalTempInfo.insert("normalLow", QString("%1").arg(d->m_formula.celsiusToF(d->m_weatherData[source].normalLow.toFloat())));
        }
    }

    if (metricUnit()) {
        regionalTempInfo.insert("regionalTempUnit", QString("%1C").arg(QChar(176)));
    } else {
        regionalTempInfo.insert("regionalTempUnit", QString("%1F").arg(QChar(176)));
    }

    return regionalTempInfo;
}

QMap<QString, QString> EnvCanadaIon::yesterdayWeather(const QString& source)
{
    QMap<QString, QString> yesterdayInfo;

    if (d->m_weatherData[source].prevHigh.isEmpty()) {
        yesterdayInfo.insert("prevHigh", "N/A");
    } else {
        if (metricUnit()) {
            yesterdayInfo.insert("prevHigh", d->m_weatherData[source].prevHigh);
        } else {
            yesterdayInfo.insert("prevHigh", QString::number(d->m_formula.celsiusToF(d->m_weatherData[source].prevHigh.toFloat())));
        }
    }

    if (d->m_weatherData[source].prevLow.isEmpty()) {
        yesterdayInfo.insert("prevLow", "N/A");
    } else {
        if (metricUnit()) {
            yesterdayInfo.insert("prevLow", d->m_weatherData[source].prevLow);
        } else {
            yesterdayInfo.insert("prevLow", QString::number(d->m_formula.celsiusToF(d->m_weatherData[source].prevLow.toFloat()), 'f', 1));
        }
    }

    if (metricUnit()) {
        yesterdayInfo.insert("yesterdayTempUnit", QString("%1C").arg(QChar(176)));
    } else {
        yesterdayInfo.insert("yesterdayTempUnit", QString("%1F").arg(QChar(176)));
    }

    if (d->m_weatherData[source].prevPrecipTotal == "Trace") {
        yesterdayInfo.insert("prevPrecip", "Trace");
        return yesterdayInfo;
    }

    if (d->m_weatherData[source].prevPrecipTotal.isEmpty()) {
        yesterdayInfo.insert("prevPrecip", "N/A");
    } else {
        if (metricUnit()) {
            yesterdayInfo.insert("prevPrecipTotal", d->m_weatherData[source].prevPrecipTotal);
            yesterdayInfo.insert("prevPrecipUnit", d->m_weatherData[source].prevPrecipType);
        } else {
            yesterdayInfo.insert("prevPrecipTotal", QString::number(d->m_formula.millimetersToIN(d->m_weatherData[source].prevPrecipTotal.toFloat()), 'f', 1));
            yesterdayInfo.insert("prevPrecipUnit", QString("in"));
        }
    }

    return yesterdayInfo;
}

QMap<QString, QString> EnvCanadaIon::sunriseSet(const QString& source)
{
    QMap<QString, QString> sunInfo;

    if (d->m_weatherData[source].sunriseTimestamp.isEmpty()) {
        sunInfo.insert("sunrise", "N/A");
    } else {
        sunInfo.insert("sunrise", d->m_weatherData[source].sunriseTimestamp);
    }

    if (d->m_weatherData[source].sunsetTimestamp.isEmpty()) {
        sunInfo.insert("sunset", "N/A");
    } else {
        sunInfo.insert("sunset", d->m_weatherData[source].sunsetTimestamp);
    }

    return sunInfo;
}

QMap<QString, QString> EnvCanadaIon::moonriseSet(const QString& source)
{
    QMap<QString, QString> moonInfo;

    if (d->m_weatherData[source].moonriseTimestamp.isEmpty()) {
        moonInfo.insert("moonrise", "N/A");
    } else {
        moonInfo.insert("moonrise", d->m_weatherData[source].moonriseTimestamp);
    }

    if (d->m_weatherData[source].moonsetTimestamp.isEmpty()) {
        moonInfo.insert("moonset", "N/A");
    } else {
        moonInfo.insert("moonset", d->m_weatherData[source].moonsetTimestamp);
    }

    return moonInfo;
}

QMap<QString, QString> EnvCanadaIon::weatherRecords(const QString& source)
{
    QMap<QString, QString> recordInfo;

    if (d->m_weatherData[source].recordHigh == 0) {
        recordInfo.insert("recordHigh", "N/A");
    } else {
        if (metricUnit()) {
            recordInfo.insert("recordHigh", QString("%1").arg(d->m_weatherData[source].recordHigh));
        } else {
            recordInfo.insert("recordHigh", QString::number(d->m_formula.celsiusToF(d->m_weatherData[source].recordHigh), 'f', 1));
        }
    }

    if (d->m_weatherData[source].recordLow == 0) {
        recordInfo.insert("recordLow", "N/A");
    } else {
        if (metricUnit()) {
            recordInfo.insert("recordLow", QString("%1").arg(d->m_weatherData[source].recordLow));
        } else {
            recordInfo.insert("recordLow", QString::number(d->m_formula.celsiusToF(d->m_weatherData[source].recordLow), 'f', 1));
        }

    }

    if (metricUnit()) {
        recordInfo.insert("recordTempUnit", QString("%1C").arg(QChar(176)));
    } else {
        recordInfo.insert("recordTempUnit", QString("%1F").arg(QChar(176)));
    }

    if (d->m_weatherData[source].recordRain == 0) {
        recordInfo.insert("recordRain", "N/A");
    } else {
        if (metricUnit()) {
            recordInfo.insert("recordRain", QString("%1").arg(d->m_weatherData[source].recordRain));
            recordInfo.insert("recordRainUnit", QString("mm"));
        } else {
            recordInfo.insert("recordRain", QString::number(d->m_formula.millimetersToIN(d->m_weatherData[source].recordRain), 'f', 1));
            recordInfo.insert("recordRainUnit", QString("in"));
        }
    }

    if (d->m_weatherData[source].recordSnow == 0) {
        recordInfo.insert("recordSnow", "N/A");
    } else {
        if (metricUnit()) {
            recordInfo.insert("recordSnow", QString("%1").arg(d->m_weatherData[source].recordSnow));
            recordInfo.insert("recordSnowUnit", QString("cm"));
        } else {
            recordInfo.insert("recordSnow", QString::number(d->m_formula.centimetersToIN(d->m_weatherData[source].recordSnow), 'f', 1));
            recordInfo.insert("recordSnowUnit", QString("in"));
        }
    }

    return recordInfo;
}

#include "ion_envcan.moc"
