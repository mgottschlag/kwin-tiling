/***************************************************************************
 *   Copyright (C) 2007-2008 by Shawn Starr <shawn.starr@rogers.com>       *
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
    Private() { m_url = 0; }
    ~Private() { delete m_url; }

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
    
    QDateTime m_dateFormat;
};


// ctor, dtor
EnvCanadaIon::EnvCanadaIon(QObject *parent, const QVariantList &args)
        : IonInterface(parent, args), d(new Private())
{
}

EnvCanadaIon::~EnvCanadaIon()
{
    // Destroy each watch/warning stored in a QVector
    foreach(const WeatherData &item, d->m_weatherData) {
        foreach(WeatherData::WeatherEvent *warning, item.warnings) {
            if (warning) {
                delete warning;
            }
        }

        foreach(WeatherData::WeatherEvent *watch, item.watches) {
            if (watch) {
                delete watch;
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

QMap<QString,IonInterface::ConditionIcons> EnvCanadaIon::setupDayIconMappings(void)
{
//    ClearDay, FewCloudsDay, PartlyCloudyDay, Overcast,
//    Showers, ScatteredShowers, Thunderstorm, Snow,
//    FewCloudsNight, PartlyCloudyNight, ClearNight,
//    Mist, NotAvailable
//
    QMap<QString,ConditionIcons> dayList;
    dayList["sunny"] = ClearDay;
    dayList["mainly sunny"] = FewCloudsDay;
    dayList["partly cloudy"] = PartlyCloudyDay;
    dayList["mostly cloudy"] = PartlyCloudyDay;
    dayList["cloudy periods"] = PartlyCloudyDay;
    dayList["cloudy with sunny periods"] = PartlyCloudyDay;
    dayList["increasing cloudiness"] = Overcast;
    dayList["cloudy"] = Overcast;
    dayList["overcast"] = Overcast;
    dayList["light snow"] = LightSnow;
    dayList["snow grains"] = Flurries;
    dayList["light rainshower"] = LightRain;
    dayList["light rain"] = LightRain;
    dayList["light drizzle"] = LightRain;
    dayList["rain"] = Rain;
    dayList["heavy rain"] = Rain;
    dayList["rain at times heavy"] = Rain;
    dayList["periods of rain"] = Rain;
    dayList["periods of drizzle"] = LightRain;
    dayList["recent thunderstorm"] = Thunderstorm;
    dayList["showers"] = Showers;
    dayList["chance of showers"] = ChanceShowersDay;
    dayList["chance of showers or drizzle"] = ChanceShowersDay;
    dayList["periods of rain or drizzle"] = Rain;
    dayList["chance of flurries"] = Flurries; // ChanceFlurriesDay
    dayList["a few clouds"] = FewCloudsDay;
    dayList["a few showers"] = ChanceShowersDay;
    dayList["a few rain showers or flurries"] = NotAvailable; 
    dayList["chance of flurries or rain showers"] = NotAvailable;
    dayList["a few flurries"] = Flurries;
    dayList["hail"] = Hail;
    dayList["fog patches"] = Mist;
    dayList["fog"] = Mist;

    // forecasts that are explicit on period.
    dayList["a mix of sun and cloud"] = PartlyCloudyDay;
    dayList["sunny with cloudy periods"] = PartlyCloudyDay;
    dayList["cloudy with sunny periods"] = PartlyCloudyDay;

    return dayList;
}

QMap<QString,IonInterface::ConditionIcons> EnvCanadaIon::setupNightIconMappings(void)
{
    QMap<QString,ConditionIcons> nightList;
    nightList["clear"] = ClearNight;
    nightList["mainly clear"] = FewCloudsNight;
    nightList["clearing"] = ClearNight;
    nightList["partly cloudy"] = PartlyCloudyNight;
    nightList["mostly cloudy"] = PartlyCloudyNight;
    nightList["increasing cloudiness"] = Overcast;
    nightList["cloudy periods"] = PartlyCloudyNight;
    nightList["cloudy"] = Overcast;
    nightList["overcast"] = Overcast;
    nightList["light snow"] = LightSnow;
    nightList["snow grains"] = LightSnow;
    nightList["light rainshower"] = LightRain;
    nightList["light rain"] = LightRain;
    nightList["light drizzle"] = LightRain;
    nightList["rain"] = Rain;
    nightList["rain at times heavy"] = Rain;
    nightList["showers"] = LightRain;
    nightList["heavy rain"] = Rain;
    nightList["periods of rain"] = Rain;
    nightList["periods of drizzle"] = LightRain;
    nightList["periods of rain or drizzle"] = Rain;
    nightList["recent thunderstorm"] = Thunderstorm;
    nightList["chance of showers"] = ChanceShowersNight;
    nightList["chance of showers or drizzle"] = ChanceShowersNight;
    nightList["chance of flurries"] = Flurries; // ChanceFlurriesNight
    nightList["a few clouds"] = FewCloudsNight;
    nightList["a few showers"] = ChanceShowersNight;
    nightList["a few rain showers or flurries"] = NotAvailable;
    nightList["chance of flurries or rain showers"] = NotAvailable;
    nightList["a few flurries"] = Flurries;
    nightList["hail"] = Hail;
    nightList["fog patches"] = Mist;
    nightList["fog"] = Mist;

    return nightList;
}

QMap<QString,IonInterface::ConditionIcons> const& EnvCanadaIon::dayIcons(void)
{
    static QMap<QString,ConditionIcons> const dval = setupDayIconMappings();
    return dval;
}

QMap<QString,IonInterface::ConditionIcons> const& EnvCanadaIon::nightIcons(void)
{
    static QMap<QString,ConditionIcons> const nval = setupNightIconMappings();
    return nval;
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
    // We expect the applet to send the source in the following tokenization:
    // ionname|validate|place_name - Triggers validation of place
    // ionname|weather|place_name - Triggers receiving weather of place

    QStringList sourceAction = source.split('|');
    if (sourceAction[1] == QString("validate")) {
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
        getXMLData(source);
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
    
    // Demunge source name for key only.
    QString dataKey = source;
    dataKey.replace("|weather", "");
    url = "http://dd.weatheroffice.ec.gc.ca/EC_sites/xml/" + d->m_place[dataKey].territoryName + "/" + d->m_place[dataKey].cityCode + "_e.xml";
    //url="file:///home/spstarr/Desktop/s0000649_e.xml";

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
    setData(d->m_jobList[job], Data());
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
                data.watches.clear();
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
   
    QString dataKey = source;
    dataKey.replace("|weather", "");
    data.shortTerritoryName = d->m_place[dataKey].territoryName;
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

void EnvCanadaIon::parseDateTime(WeatherData& data, QXmlStreamReader& xml, WeatherData::WeatherEvent *event)
{

    Q_ASSERT(xml.isStartElement() && xml.name() == "dateTime");

    // What kind of date info is this?
    QString dateType = xml.attributes().value("name").toString();
    QString dateZone = xml.attributes().value("zone").toString();

    QString selectTimeStamp;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (dateType == "xmlCreation") {
                return;
            }
            if (dateZone == "UTC") {
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
                selectTimeStamp = xml.readElementText();
            else if (xml.name() == "textSummary") {
                    if (dateType == "eventIssue") {
                        if (event) {
                            event->timestamp = xml.readElementText();
                        }
                    } else if (dateType == "observation") {
                        xml.readElementText();
                        d->m_dateFormat = QDateTime::fromString(selectTimeStamp,"yyyyMMddHHmmss");
                        data.obsTimestamp = d->m_dateFormat.toString("dd.MM.yyyy @ hh:mm ap");
                        data.iconPeriodHour = d->m_dateFormat.toString("hh").toInt();
                        data.iconPeriodAP = d->m_dateFormat.toString("ap");
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
    WeatherData::WeatherEvent *watch = new WeatherData::WeatherEvent;
    WeatherData::WeatherEvent *warning = new WeatherData::WeatherEvent;

    Q_ASSERT(xml.isStartElement() && xml.name() == "warnings");
    QString eventURL = xml.attributes().value("url").toString();
    int flag = 0;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "warnings") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "dateTime") {
                if (flag == 1) {
                    parseDateTime(data, xml, watch);
                } 
                if (flag == 2) {
                    parseDateTime(data, xml, warning);
                }

                if (!warning->timestamp.isEmpty() && !warning->url.isEmpty())  {
                    data.warnings.append(warning);
                    warning = new WeatherData::WeatherEvent;
                }
                if (!watch->timestamp.isEmpty() && !watch->url.isEmpty()) {
                    data.watches.append(watch);
                    watch = new WeatherData::WeatherEvent;
                }

            } else if (xml.name() == "event") {
                // Append new event to list.
                QString eventType = xml.attributes().value("type").toString();
                if (eventType == "watch") {
                    watch->url = eventURL;
                    watch->type = eventType;
                    watch->priority = xml.attributes().value("priority").toString();
                    watch->description = xml.attributes().value("description").toString();
                    flag = 1;
                 }

                if (eventType == "warning") {
                    warning->url = eventURL; 
                    warning->type = eventType;
                    warning->priority = xml.attributes().value("priority").toString();
                    warning->description = xml.attributes().value("description").toString();
                    flag = 2;
                }
            } else {
                if (xml.name() != "dateTime") {
                    parseUnknownElement(xml);
                }
              }
        }
    }
    delete watch;
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
                forecast->forecastPeriod = xml.attributes().value("textForecastName").toString();
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

    QString shortText;

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
                shortText = xml.readElementText();
                if ((forecast->forecastPeriod == "tonight") || (forecast->forecastPeriod.contains("night"))) {
                     forecast->iconName = getWeatherIcon(nightIcons(), shortText.toLower());
                     forecast->shortForecast = shortText;
                } else {
                     forecast->iconName = getWeatherIcon(dayIcons(), shortText.toLower());
                     forecast->shortForecast = shortText;
                }
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
                    data.prevPrecipType = QString::number(WeatherUtils::NoUnit);
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

void EnvCanadaIon::updateWeather(const QString& source)
{
    QMap<QString, QString> dataFields;
    QStringList fieldList;
    QVector<QString> forecastList;
    int i = 0;

    setData(source, "Country", country(source));
    setData(source, "Place", QString("%1, %2").arg(city(source)).arg(territory(source)));
    setData(source, "Region", region(source));
    setData(source, "Station", station(source));

    // Real weather - Current conditions
    setData(source, "Observation Period", observationTime(source));
    setData(source, "Current Conditions", condition(source));

    // Tell applet which icon to use for conditions and provide mapping for condition type to the icons to display
    if (night(source) && periodHour(source) >= 16) {  // 24 hour time
        setData(source, "Condition Icon", getWeatherIcon(nightIcons(), condition(source)));
    } else {
        setData(source, "Condition Icon", getWeatherIcon(dayIcons(), condition(source)));
    }

    dataFields = temperature(source);
    setData(source, "Temperature", dataFields["temperature"]);

    // Do we have a comfort temperature? if so display it
    if (dataFields["comfortTemperature"] != "N/A" && !dataFields["comfortTemperature"].isEmpty()) {
        if (dataFields["comfortTemperature"].toFloat() <= 0) {
            setData(source, "Windchill", QString("%1%2").arg(dataFields["comfortTemperature"]).arg(QChar(176)));
            setData(source, "Humidex", "N/A");
        } else {
            setData(source, "Humidex", QString("%1%2").arg(dataFields["comfortTemperature"]).arg(QChar(176)));
            setData(source, "Windchill", "N/A");
        }
    } else {
        setData(source, "Windchill", "N/A");
        setData(source, "Humidex", "N/A");
    }

    setData(source, "Temperature Unit", dataFields["temperatureUnit"]);

    setData(source, "Dewpoint", dewpoint(source));
    if (dewpoint(source) != "N/A") {
        setData(source, "Dewpoint Unit", dataFields["temperatureUnit"]);
    }

    dataFields = pressure(source);
    setData(source, "Pressure", dataFields["pressure"]);

    if (dataFields["pressure"] != "N/A") {
        setData(source, "Pressure Tendency", dataFields["pressureTendency"]);
        setData(source, "Pressure Unit", dataFields["pressureUnit"]);
    }

    dataFields = visibility(source);
    setData(source, "Visibility", dataFields["visibility"]);
    if (dataFields["visibility"] != "N/A") {
        setData(source, "Visibility Unit", dataFields["visibilityUnit"]);
    }

    setData(source, "Humidity", humidity(source));

    dataFields = wind(source);
    setData(source, "Wind Speed", dataFields["windSpeed"]);
    if (dataFields["windSpeed"] != "N/A") {
        setData(source, "Wind Speed Unit", dataFields["windUnit"]);
    }
    setData(source, "Wind Gust", dataFields["windGust"]);
    setData(source, "Wind Direction", dataFields["windDirection"]);
    setData(source, "Wind Gust Unit", dataFields["windGustUnit"]);

    dataFields = regionalTemperatures(source);
    setData(source, "Normal High", dataFields["normalHigh"]);
    setData(source, "Normal Low", dataFields["normalLow"]);
    if (dataFields["normalHigh"] != "N/A" && dataFields["normalLow"] != "N/A") {
        setData(source, "Regional Temperature Unit", dataFields["regionalTempUnit"]);
    }

    // Check if UV index is available for the location
    dataFields = uvIndex(source);
    setData(source, "UV Index", dataFields["uvIndex"]);
    if (dataFields["uvIndex"] != "N/A") {
        setData(source, "UV Rating", dataFields["uvRating"]);
    }

    dataFields = watches(source);

    // Set number of forecasts per day/night supported
    setData(source, QString("Total Watches Issued"), d->m_weatherData[source].watches.size());

    // Check if we have warnings or watches
    for (int i = 0; i < d->m_weatherData[source].watches.size(); i++) {
         fieldList = dataFields[QString("watch %1").arg(i)].split('|');
         setData(source, QString("Watch Priority %1").arg(i), fieldList[0]);
         setData(source, QString("Watch Description %1").arg(i), fieldList[1]);
         setData(source, QString("Watch Info %1").arg(i), fieldList[2]);
         setData(source, QString("Watch Timestamp %1").arg(i), fieldList[3]);
    }

    dataFields = warnings(source);

    setData(source, QString("Total Warnings Issued"), d->m_weatherData[source].warnings.size());

    for (int k = 0; k < d->m_weatherData[source].warnings.size(); k++) {
         fieldList = dataFields[QString("warning %1").arg(k)].split('|');
         setData(source, QString("Warning Priority %1").arg(k), fieldList[0]);
         setData(source, QString("Warning Description %1").arg(k), fieldList[1]);
         setData(source, QString("Warning Info %1").arg(k), fieldList[2]);
         setData(source, QString("Warning Timestamp %1").arg(k), fieldList[3]);
    }

    forecastList = forecasts(source);

    // Set number of forecasts per day/night supported
    setData(source, QString("Total Weather Days"), d->m_weatherData[source].forecasts.size());

    foreach(const QString &forecastItem, forecastList) {
        fieldList = forecastItem.split('|');

        setData(source, QString("Short Forecast Day %1").arg(i), QString("%1|%2|%3|%4|%5|%6") \
                .arg(fieldList[0]).arg(fieldList[1]).arg(fieldList[2]).arg(fieldList[3]).arg(fieldList[4]).arg(fieldList[5]));

/*
        setData(source, QString("Long Forecast Day %1").arg(i), QString("%1|%2|%3|%4|%5|%6|%7|%8") \
                .arg(fieldList[0]).arg(fieldList[2]).arg(fieldList[3]).arg(fieldList[4]).arg(fieldList[6]) \
                .arg(fieldList[7]).arg(fieldList[8]).arg(fieldList[9]));
*/
        i++;
    }

    dataFields = yesterdayWeather(source);
    setData(source, "Yesterday High", dataFields["prevHigh"]);
    setData(source, "Yesterday Low", dataFields["prevLow"]);

    if (dataFields["prevHigh"] != "N/A" && dataFields["prevLow"] != "N/A") {
        setData(source , "Yesterday Temperature Unit", dataFields["yesterdayTempUnit"]);
    }

    setData(source, "Yesterday Precip Total", dataFields["prevPrecip"]);
    setData(source, "Yesterday Precip Unit", dataFields["prevPrecipUnit"]);

    dataFields = sunriseSet(source);
    setData(source, "Sunrise At", dataFields["sunrise"]);
    setData(source, "Sunset At", dataFields["sunset"]);

    dataFields = moonriseSet(source);
    setData(source, "Moonrise At", dataFields["moonrise"]);
    setData(source, "Moonset At", dataFields["moonset"]);

    dataFields = weatherRecords(source);
    setData(source, "Record High Temperature", dataFields["recordHigh"]);
    setData(source, "Record Low Temperature", dataFields["recordLow"]);
    if (dataFields["recordHigh"] != "N/A" && dataFields["recordLow"] != "N/A") {
        setData(source, "Record Temperature Unit", dataFields["recordTempUnit"]);
    }

    setData(source, "Record Rainfall", dataFields["recordRain"]);
    setData(source, "Record Rainfall Unit", dataFields["recordRainUnit"]);
    setData(source, "Record Snowfall", dataFields["recordSnow"]);
    setData(source, "Record Snowfall Unit", dataFields["recordSnowUnit"]);

    setData(source, "Credit", "Meteorological data is provided by Environment Canada");
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

bool EnvCanadaIon::night(const QString& source)
{
    if (d->m_weatherData[source].iconPeriodAP == "pm") {
        return true;
    }
    return false;
}

int EnvCanadaIon::periodHour(const QString& source)
{
    return d->m_weatherData[source].iconPeriodHour;
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
    if (!d->m_weatherData[source].dewpoint.isEmpty()) {
        return QString::number(d->m_weatherData[source].dewpoint.toFloat(), 'f', 1);
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
        visibilityInfo.insert("visibility", QString::number(d->m_weatherData[source].visibility, 'f', 1));
        visibilityInfo.insert("visibilityUnit", QString::number(WeatherUtils::Kilometers));
    } else {
        visibilityInfo.insert("visibility", "N/A");
    }
    return visibilityInfo;
}

QMap<QString, QString> EnvCanadaIon::temperature(const QString& source)
{
    QMap<QString, QString> temperatureInfo;
    if (!d->m_weatherData[source].temperature.isEmpty()) {
        temperatureInfo.insert("temperature", QString::number(d->m_weatherData[source].temperature.toFloat(), 'f', 1));
    }
    temperatureInfo.insert("temperatureUnit", QString::number(WeatherUtils::Celsius));
    temperatureInfo.insert("comfortTemperature", "N/A");

    if (d->m_weatherData[source].comforttemp != "N/A") {
        temperatureInfo.insert("comfortTemperature", d->m_weatherData[source].comforttemp);
    }
    return temperatureInfo;
}

QMap<QString, QString> EnvCanadaIon::watches(const QString& source)
{
    QMap<QString, QString> watchData;
    QString watchType;
    for (int i = 0; i < d->m_weatherData[source].watches.size(); ++i) {
         watchType = QString("watch %1").arg(i);
         watchData[watchType] = QString("%1|%2|%3|%4").arg(d->m_weatherData[source].watches[i]->priority) \
                                .arg(d->m_weatherData[source].watches[i]->description) \
                                .arg(d->m_weatherData[source].watches[i]->url) \
                                .arg(d->m_weatherData[source].watches[i]->timestamp);
    }
    return watchData;
}

QMap<QString, QString> EnvCanadaIon::warnings(const QString& source)
{
    QMap<QString, QString> warningData;
    QString warnType;
    for (int i = 0; i < d->m_weatherData[source].warnings.size(); ++i) {
         warnType = QString("warning %1").arg(i);
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
        if (d->m_weatherData[source].forecasts[i]->iconName.isEmpty()) {
            d->m_weatherData[source].forecasts[i]->iconName = "N/A";
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
        // We need to shortform the day/night strings.
        if (d->m_weatherData[source].forecasts[i]->forecastPeriod.contains("Tonight")) {
            d->m_weatherData[source].forecasts[i]->forecastPeriod.replace("Tonight", "nite");
        }
       
        if (d->m_weatherData[source].forecasts[i]->forecastPeriod.contains("night")) {
            d->m_weatherData[source].forecasts[i]->forecastPeriod.replace("night","nt");
        }
 
        if (d->m_weatherData[source].forecasts[i]->forecastPeriod.contains("Saturday")) {
            d->m_weatherData[source].forecasts[i]->forecastPeriod.replace("Saturday", "Sat");
        }
        
        if (d->m_weatherData[source].forecasts[i]->forecastPeriod.contains("Sunday")) {
            d->m_weatherData[source].forecasts[i]->forecastPeriod.replace("Sunday", "Sun");
        }
        
        if (d->m_weatherData[source].forecasts[i]->forecastPeriod.contains("Monday")) {
            d->m_weatherData[source].forecasts[i]->forecastPeriod.replace("Monday", "Mon");
        }

        if (d->m_weatherData[source].forecasts[i]->forecastPeriod.contains("Tuesday")) {
            d->m_weatherData[source].forecasts[i]->forecastPeriod.replace("Tuesday", "Tue");
        }

        if (d->m_weatherData[source].forecasts[i]->forecastPeriod.contains("Wednesday")) {
            d->m_weatherData[source].forecasts[i]->forecastPeriod.replace("Wednesday", "Wed");
        }

        if (d->m_weatherData[source].forecasts[i]->forecastPeriod.contains("Thursday")) {
            d->m_weatherData[source].forecasts[i]->forecastPeriod.replace("Thursday", "Thu");
        }
        if (d->m_weatherData[source].forecasts[i]->forecastPeriod.contains("Friday")) {
            d->m_weatherData[source].forecasts[i]->forecastPeriod.replace("Friday", "Fri");
        }

        forecastData.append(QString("%1|%2|%3|%4|%5|%6") \
                             .arg(d->m_weatherData[source].forecasts[i]->forecastPeriod) \
                             .arg(d->m_weatherData[source].forecasts[i]->iconName) \
                             .arg(d->m_weatherData[source].forecasts[i]->shortForecast) \
                             .arg(d->m_weatherData[source].forecasts[i]->forecastTempHigh) \
                             .arg(d->m_weatherData[source].forecasts[i]->forecastTempLow) \
                             .arg(d->m_weatherData[source].forecasts[i]->popPrecent));
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
        pressureInfo.insert("pressure", QString::number(d->m_weatherData[source].pressure, 'f', 1));
        pressureInfo.insert("pressureUnit", QString::number(WeatherUtils::Kilopascals));
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
        windInfo.insert("windUnit", QString::number(WeatherUtils::NoUnit));
    } else if (d->m_weatherData[source].windSpeed.toInt() == 0) {
        windInfo.insert("windSpeed", "Calm");
        windInfo.insert("windUnit", QString::number(WeatherUtils::NoUnit));
    } else {
        windInfo.insert("windSpeed", QString::number(d->m_weatherData[source].windSpeed.toInt()));
        windInfo.insert("windUnit", QString::number(WeatherUtils::Kilometers));
    }

    // May not always have gusty winds
    if (d->m_weatherData[source].windGust.isEmpty()) {
        windInfo.insert("windGust", "N/A");
        windInfo.insert("windGustUnit", QString::number(WeatherUtils::NoUnit));
    } else {
        windInfo.insert("windGust", QString::number(d->m_weatherData[source].windGust.toInt()));
        windInfo.insert("windGustUnit", QString::number(WeatherUtils::Kilometers));
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
        regionalTempInfo.insert("normalHigh", d->m_weatherData[source].normalHigh);
    }

    if (d->m_weatherData[source].normalLow.isEmpty()) {
        regionalTempInfo.insert("normalLow", "N/A");
    } else {
        regionalTempInfo.insert("normalLow", d->m_weatherData[source].normalLow);
    }

    regionalTempInfo.insert("regionalTempUnit", QString::number(WeatherUtils::Celsius));
    return regionalTempInfo;
}

QMap<QString, QString> EnvCanadaIon::yesterdayWeather(const QString& source)
{
    QMap<QString, QString> yesterdayInfo;

    if (d->m_weatherData[source].prevHigh.isEmpty()) {
        yesterdayInfo.insert("prevHigh", "N/A");
    } else {
        yesterdayInfo.insert("prevHigh", d->m_weatherData[source].prevHigh);
    }

    if (d->m_weatherData[source].prevLow.isEmpty()) {
        yesterdayInfo.insert("prevLow", "N/A");
    } else {
        yesterdayInfo.insert("prevLow", d->m_weatherData[source].prevLow);
    }

    yesterdayInfo.insert("yesterdayTempUnit", QString::number(WeatherUtils::Celsius));

    if (d->m_weatherData[source].prevPrecipTotal == "Trace") {
        yesterdayInfo.insert("prevPrecip", "Trace");
        return yesterdayInfo;
    }

    if (d->m_weatherData[source].prevPrecipTotal.isEmpty()) {
        yesterdayInfo.insert("prevPrecip", "N/A");
    } else {
        yesterdayInfo.insert("prevPrecipTotal", d->m_weatherData[source].prevPrecipTotal);
        if (d->m_weatherData[source].prevPrecipType == "mm") {
            yesterdayInfo.insert("prevPrecipUnit", QString::number(WeatherUtils::Millimeters));
        } else if (d->m_weatherData[source].prevPrecipType == "cm") {
            yesterdayInfo.insert("prevPrecipUnit", QString::number(WeatherUtils::Centimeters));
        } else {
            yesterdayInfo.insert("prevPrecipUnit", QString::number(WeatherUtils::NoUnit)); 
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
        recordInfo.insert("recordHigh", QString("%1").arg(d->m_weatherData[source].recordHigh));
    }

    if (d->m_weatherData[source].recordLow == 0) {
        recordInfo.insert("recordLow", "N/A");
    } else {
        recordInfo.insert("recordLow", QString("%1").arg(d->m_weatherData[source].recordLow));
    }

    recordInfo.insert("recordTempUnit", QString::number(WeatherUtils::Celsius));

    if (d->m_weatherData[source].recordRain == 0) {
        recordInfo.insert("recordRain", "N/A");
    } else {
        recordInfo.insert("recordRain", QString("%1").arg(d->m_weatherData[source].recordRain));
        recordInfo.insert("recordRainUnit", QString::number(WeatherUtils::Millimeters));
    }

    if (d->m_weatherData[source].recordSnow == 0) {
        recordInfo.insert("recordSnow", "N/A");
    } else {
        recordInfo.insert("recordSnow", QString("%1").arg(d->m_weatherData[source].recordSnow));
        recordInfo.insert("recordSnowUnit", QString::number(WeatherUtils::Centimeters));
    }

    return recordInfo;
}

#include "ion_envcan.moc"
