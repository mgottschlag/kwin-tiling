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

#include "weatherengine.h"
#include <KServiceTypeTrader>
#include <KDateTime>
#include <KLocale>
#include <QTimer>
#include "ions/ion.h"

class WeatherEngine::Private
{
public:
    Private() {}
    ~Private() {
        m_ions.clear();
    }

    QTimer *m_timer;
    IonInterface::IonDict m_ions;
    KDateTime m_localTime;
};

// Returns an instance of an Ion plugin loaded.
IonInterface* WeatherEngine::Ion(const QString& name) const
{
    IonInterface::IonDict::const_iterator it = d->m_ions.find(name);
    if (it != d->m_ions.end()) {
        return *it;
    }

    return NULL;
}

// Loads an Ion plugin given a plugin name found via KService.
IonInterface* WeatherEngine::loadIon(const QString& name)
{

    IonInterface *ion = 0;
    IonInterface::IonDict::const_iterator it = d->m_ions.find(name);

    if (it != d->m_ions.end()) {
        ion = *it;
        ion->ref();
        return ion;
    }

    QString error;
    QString tag = QString("[X-IonName] == '%1'").arg(name);

    // Load the Ion plugin, store it into a QMap to handle multiple ions.
    ion = KServiceTypeTrader::createInstanceFromQuery<IonInterface>("WeatherEngine/Ion", tag, 0, QVariantList(), &error);
    if (!ion) {
        kDebug() << "weatherengine: Couldn't load ion \"" << name << "\"!" << error;
        return 0;
    }

    // Increment counter of ions.
    ion->ref();

    // Set the Ion's long name
    //ion->setObjectName(offers.first()->name());
    connect(ion, SIGNAL(newSource(QString)), this, SLOT(newIonSource(QString)));
    connect(ion, SIGNAL(sourceRemoved(QString)), this, SLOT(removeIonSource(QString)));

    /* Set properties for the ion
     *
     * TIMEFORMAT is displaying the time/date in UTC or user's local time
     * UNITS is setting the weather units used, Celsius/Fahrenheit, Kilopascals/Inches of Mercury, etc
     * WINDFORMAT enable winds to be displayed as meters per second (m/s) some countries display winds like this
     */

    ion->option(IonInterface::TIMEFORMAT, QVariant(d->m_localTime.isUtc()));
    ion->option(IonInterface::UNITS, KGlobal::locale()->measureSystem());
    ion->option(IonInterface::WINDFORMAT, QVariant(true)); // FIXME: Should be configurable by applet

    // Assign the instantiated ion the key of the name of the ion.
    d->m_ions[name] = ion;

    return ion;
}

// Unload an Ion plugin given a Ion plugin name.
void WeatherEngine::unloadIon(const QString &name)
{
    IonInterface *ion = Ion(name);
    if (ion) {
        ion->deref();

        if (!ion->isUsed()) {
            d->m_ions.remove(name);
            delete ion;
        }
    }
}

// Return a list of Ion plugins found.
KService::List WeatherEngine::knownIons()
{
    KService::List offers = KServiceTypeTrader::self()->query("WeatherEngine/Ion");
    foreach(KService::Ptr service, offers) {
        setData("ions", service->property("X-IonName").toString(), service->property("Name").toString());
    }

    return offers;
}

void WeatherEngine::newIonSource(const QString& source)
{
    IonInterface *ion = qobject_cast<IonInterface*>(sender());

    if (!ion) {
        return;
    }
    ion->connectSource(source, this);
    ion->fetch();
}

void WeatherEngine::removeIonSource(const QString& source)
{
    IonInterface *ion = qobject_cast<IonInterface*>(sender());
    if (!ion) {
        return;
    }
    ion->disconnectSource(source, this);
}

void WeatherEngine::updated(const QString& source, Plasma::DataEngine::Data data)
{
    setData(source, data);
}

// ctor
WeatherEngine::WeatherEngine(QObject *parent, const QVariantList& args)
        :  Plasma::DataEngine(parent), d(new Private())
{
    Q_UNUSED(args)

    // Set any local properties for Ion to use
    d->m_localTime = KDateTime::currentDateTime(KDateTime::LocalZone);

    /* FIXME: For now we just load them all as we find them, we'll need to make this configurable
              somehow. No point in loading all plugins if your not interested in certain cities.
    */
    foreach(KService::Ptr service, knownIons()) {
        loadIon(service->property("X-IonName").toString());
    }

    // Setup a master time to ping each Ion for new data.
    d->m_timer = new QTimer(this);
    d->m_timer->setSingleShot(false);
    connect(d->m_timer, SIGNAL(timeout()), this, SLOT(updateData()));
}

// dtor
WeatherEngine::~WeatherEngine()
{
    // Cleanup all private data.
    delete d;
}

// Setup each Ion for the first time
bool WeatherEngine::sourceRequested(const QString &name)
{
    // Pass values to set when getting data from the ion.
    foreach(IonInterface *ion, d->m_ions) {
        // Before we use the timer, get the data.
        ion->setSource(name);
        kDebug() << "sourceRequested()";
    }

    /* FIXME: Make the timer value configurable in 30 minute intervals via Applet */
    if (!d->m_timer->isActive())
        d->m_timer->start(50000);

    updateData();

    return true;
}

// SLOT: update the Applet with new data from all ions loaded.
void WeatherEngine::updateData()
{
    foreach(IonInterface *ion, d->m_ions) {
        ion->fetch();
        ion->updateData();
    }
}

#include "weatherengine.moc"
