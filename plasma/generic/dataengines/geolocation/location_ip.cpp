/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
 *                  - Original Implementation.
 *                 2009 Andrew Coles  <andrew.coles@yahoo.co.uk>
 *                  - Extension to iplocationtools engine.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "location_ip.h"
#include <KDebug>
#include <KJob>
#include <KIO/Job>
#include <KIO/TransferJob>

#include <QtXml/QXmlStreamReader>

class Ip::Private : public QObject {

public:
    QXmlStreamReader m_xmlReader;

    void populateDataEngineData(Plasma::DataEngine::Data & outd)
    {
        QString country, countryCode, city, latitude, longitude;
        while (!m_xmlReader.atEnd()) {
            m_xmlReader.readNext();
            if (m_xmlReader.isEndElement() && m_xmlReader.name() == "Response") {
                break;
            }

            if (m_xmlReader.isStartElement()) {
                if (m_xmlReader.name() == "Status") {
                    QString tmp = m_xmlReader.readElementText();
                    if (tmp != "OK") return;
                } else if (m_xmlReader.name() == "CountryCode") {
                    countryCode = m_xmlReader.readElementText();
                } else if (m_xmlReader.name() == "CountryName") {
                    country = m_xmlReader.readElementText();
                } else if (m_xmlReader.name() == "City") {
                    city = m_xmlReader.readElementText();
                } else if (m_xmlReader.name() == "Latitude") {
                    latitude = m_xmlReader.readElementText();
                } else if (m_xmlReader.name() == "Longitude") {
                    longitude = m_xmlReader.readElementText();
                } else { // for fields such as 'IP'...
                    m_xmlReader.readElementText();
                }
            }
        }

        // ordering of first three to preserve backwards compatibility

        outd["accuracy"] = 40000;
        outd["country"] = country;
        outd["country code"] = countryCode;
        outd["city"] = city;
        outd["latitude"] = latitude;
        outd["longitude"] = longitude;

    }

};

Ip::Ip(QObject* parent, const QVariantList& args)
    : GeolocationProvider(parent, args), d(new Private())
{
    setUpdateTriggers(SourceEvent | NetworkConnected);
}

Ip::~Ip()
{
    delete d;
}

void Ip::update()
{
    d->m_xmlReader.clear();

    KIO::TransferJob *datajob = KIO::get(KUrl("http://ipinfodb.com/ip_query.php"),
                                         KIO::NoReload, KIO::HideProgressInfo);

    if (datajob) {
        kDebug() << "Fetching http://iplocationtools.com/ip_query.php";
        connect(datajob, SIGNAL(data(KIO::Job*,QByteArray)), this,
                SLOT(readData(KIO::Job*,QByteArray)));
        connect(datajob, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
    } else {
        kDebug() << "Could not create job";
    }
}

void Ip::readData(KIO::Job* job, const QByteArray& data)
{
    Q_UNUSED(job)

    if (data.isEmpty()) {
        return;
    }

    d->m_xmlReader.addData(data);
}

void Ip::result(KJob* job)
{
    Plasma::DataEngine::Data outd;

    if(job && !job->error()) {

        while (!d->m_xmlReader.atEnd()) {
            d->m_xmlReader.readNext();

            if (d->m_xmlReader.isStartElement()) {
                if (d->m_xmlReader.name() == "Response") {
                    d->populateDataEngineData(outd);
                    break;
                }
            }
        }
//        kDebug() << "Done reading XML";
    } else {
        kDebug() << "error";
    }

    setData(outd);
}

K_EXPORT_PLASMA_GEOLOCATIONPROVIDER(ip, Ip)

#include "location_ip.moc"
