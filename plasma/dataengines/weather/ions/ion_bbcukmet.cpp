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

/* Ion for BBC's Weather from the UK Met Office */

#include "ion_bbcukmet.h"

class UKMETIon::Private : public QObject
{
public:
    Private() {}
    ~Private() {}

private:
    struct XMLMapInfo {
        QString place;
        QString XMLurl;
        bool ukPlace;
    };

public:
    // Key dicts
    QHash<QString, UKMETIon::Private::XMLMapInfo> m_place;
    QVector<QString> m_locations;
public:
    // Weather information
    QHash<QString, WeatherData> m_weatherData;

    // Store KIO jobs - Search list
    QMap<KJob *, QXmlStreamReader*> m_jobXml;
    QMap<KJob *, QString> m_jobList;

    QMap<KJob *, QXmlStreamReader*> m_forecastJobXml;
    QMap<KJob *, QString> m_forecastJobList;

    KUrl *m_url;
    KIO::TransferJob *m_job;

    bool m_useUTC;  // Ion option: Timezone may be local time or UTC time
    bool m_useMetric; // Ion option: Units may be Metric or Imperial
    bool m_windInMeters; // Ion option: Display wind format in meters per second only

    WeatherFormula m_formula;
};


// ctor, dtor
UKMETIon::UKMETIon(QObject *parent, const QVariantList &args)
        : IonInterface(parent), d(new Private())
{
    Q_UNUSED(args)
}

UKMETIon::~UKMETIon()
{
    // Destroy dptr
    delete d;
}

// Get the master list of locations to be parsed
void UKMETIon::init()
{
return;
}

// Get a specific Ion's data
bool UKMETIon::updateIonSource(const QString& source)
{
       Q_UNUSED(source)
       //if (!d->m_locations.contains(source)) {
       //    searchPlace(source);
       //} else {
       //    cachedLocation(source);
      // }
return true;
}

// Parses city list and gets the correct city based on ID number
void UKMETIon::searchPlace(const QString& key)
{
    KUrl url;
    url = "http://www.bbc.co.uk/cgi-perl/weather/search/new_search.pl?x=0&y=0&=Submit&search_query=" + key + "&tmpl=wap";
    kDebug() << "URL: " << url;

    d->m_job = KIO::get(url.url(), KIO::Reload, KIO::HideProgressInfo);
    d->m_jobXml.insert(d->m_job, new QXmlStreamReader);
    d->m_jobList.insert(d->m_job, key);
 
    if (d->m_job) {
        connect(d->m_job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                SLOT(slotDataArrived(KIO::Job *, const QByteArray &)));
        connect(d->m_job, SIGNAL(result(KJob *)), this, SLOT(slotJobFinished(KJob *)));
    }
}

bool UKMETIon::readSearchXMLData(const QString& key, QXmlStreamReader& xml)
{
    while (!xml.atEnd()) {
        xml.readNext();
 
        if (xml.isEndElement()) {
            break;
        }
 
        if (xml.isStartElement()) {
            kDebug() << "XML TAG: " << xml.name().toString();
            if (xml.name() == "wml") {
                parseSearchLocations(key, xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }

return !xml.error();
}

void UKMETIon::cachedLocation(const QString& key)
{
    d->m_job = 0;
    kDebug() << "cachedLocation: d->m_place[key].place = " << d->m_place[key].place;
    if (d->m_place.contains(key)) {
        d->m_job = KIO::get(d->m_place[key].XMLurl, KIO::Reload, KIO::HideProgressInfo);
        kDebug() << "URL: " << d->m_place[key].XMLurl;

        if (d->m_job) {
             d->m_forecastJobXml.insert(d->m_job, new QXmlStreamReader);
             d->m_forecastJobList.insert(d->m_job, key);
             kDebug() << "CACHE FORECAST FOR " << d->m_forecastJobList[d->m_job];
             connect(d->m_job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                     SLOT(forecast_slotDataArrived(KIO::Job *, const QByteArray &)));
             connect(d->m_job, SIGNAL(result(KJob *)), this, SLOT(forecast_slotJobFinished(KJob *)));
        }
    }
}
 
void UKMETIon::parseSearchLocations(const QString& source, QXmlStreamReader& xml)
{ 
    Q_UNUSED(source) 
    int flag = 0;
    QString url;
    QString place;
    QStringList tokens;
    Q_ASSERT(xml.isStartElement() && xml.name() == "wml");
   
    while (!xml.atEnd()) {
        xml.readNext();
  
        if (xml.isEndElement() && xml.name() == "wml") {
           break;
        }
     
        if (xml.isStartElement()) {
            if (xml.name() == "a" && !xml.attributes().value("href").isEmpty()) {
                if (xml.attributes().value("href").toString().contains("5day.wml")) {

                    // Split URL to determine station ID number
                    tokens = xml.attributes().value("href").toString().split("=");
                    if (xml.attributes().value("href").toString().contains("world")) {
                        url = "http://feeds.bbc.co.uk/weather/feeds/obs/world/" + tokens[1] + ".xml";
                        flag = 0;
                    } else {
                        url = "http://feeds.bbc.co.uk/weather/feeds/obs/id/" + tokens[1] + ".xml";
                        flag = 1;
                    }
                    place = xml.readElementText();
  
                    if (!d->m_locations.contains(place)) {
                        if (flag) {  // This is a UK specific location
                            d->m_place[place].XMLurl = url;
                            d->m_place[place].place = place;
                            d->m_place[place].ukPlace = true;
                        } else {
                            d->m_place[place].XMLurl = url;
                            d->m_place[place].place = place;
                            d->m_place[place].ukPlace = false;
                        }
                        d->m_locations.append(place);
                    }
                }
            }
        } 
    }
    // All Locations
    if (d->m_place[source].ukPlace) {
        //kDebug() << "UKMET: LIST OF UK PLACE: " << source;
        setData("FoundPlaces", source, QString("%1|%2").arg(source).arg("Local"));
        //kDebug() << "UKMET: URL OF UK PLACE: " << d->m_place[source].XMLurl;
    }

    if (!d->m_place[source].ukPlace) {
        //kDebug() << "UKMET: LIST OF WORLD PLACE: " << source;
        setData("FoundPlaces", source, QString("%1|%2").arg(source).arg("World"));
        //kDebug() << "UKMET: URL OF WORLD PLACE: " << d->m_place[source].XMLurl;
    }
}

// handle when no XML tag is found
void UKMETIon::parseUnknownElement(QXmlStreamReader& xml)
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
    
void UKMETIon::slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    if (data.isEmpty() || !d->m_jobXml.contains(job)) {
        return;
    }

    // Send to xml.
    d->m_jobXml[job]->addData(data.data());
}

void UKMETIon::slotJobFinished(KJob *job)
{
    readSearchXMLData(d->m_jobList[job], *d->m_jobXml[job]);
    d->m_jobList.remove(job);
    delete d->m_jobXml[job];
    d->m_jobXml.remove(job);
}

void UKMETIon::forecast_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    kDebug() << "UKMET: RECEIVING FORECAST INFORMATION\n";
    if (data.isEmpty() || !d->m_forecastJobXml.contains(job)) {
        return;
    }
  
    // Send to xml.
    d->m_forecastJobXml[job]->addData(data.data());
}

void UKMETIon::forecast_slotJobFinished(KJob *job)
{
    kDebug() << "UKMET: FORECAST INFO FOR " << d->m_forecastJobList[job] << " FINISHED\n";
    readObservationXMLData(d->m_forecastJobList[job], *d->m_forecastJobXml[job]);
    d->m_forecastJobList.remove(job);
    delete d->m_forecastJobXml[job];
    d->m_forecastJobXml.remove(job);
}

void UKMETIon::parsePlaceObservation(WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "rss");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "rss") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "channel") {
                parseWeatherChannel(data, xml);
            }
        }
    }
}

void UKMETIon::parseWeatherChannel(WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "channel");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "channel") {
            break;
        }
  
        if (xml.isStartElement()) {
            if (xml.name() == "title") {
                kDebug() << "PLACE NAME: " << xml.readElementText();
            } else if (xml.name() == "item") {
                parseWeatherObservation(data, xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

void UKMETIon::parseWeatherObservation(WeatherData& data, QXmlStreamReader& xml)
{   
    Q_UNUSED(data)
    Q_ASSERT(xml.isStartElement() && xml.name() == "item");
 
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "item") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "title") {
                kDebug() << "CONDITIONS: " << xml.readElementText();
            } else if (xml.name() == "description") {
                kDebug() << "OBSERVATIONS: " << xml.readElementText();
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

bool UKMETIon::readObservationXMLData(QString& key, QXmlStreamReader& xml)
{
    WeatherData data;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            kDebug() << "XML TAG: " << xml.name().toString();
            if (xml.name() == "rss") {
                parsePlaceObservation(data, xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }

    d->m_weatherData[key] = data;
    return !xml.error();
}

// User toggleable values set from the dataengine <-> Plasma Applet
void UKMETIon::option(int option, QVariant value)
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

bool UKMETIon::validLocation(QString keyName)
{
    if (d->m_locations.contains(keyName)) {
        return true;
    }
    return false;
}

void UKMETIon::updateWeather(const QString& source)
{
    Q_UNUSED(source)
    return;
}

#include "ion_bbcukmet.moc"
