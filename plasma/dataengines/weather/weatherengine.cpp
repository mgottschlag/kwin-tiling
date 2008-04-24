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
#include "ions/ion.h"

class WeatherEngine::Private
{
public:
    Private() {}
    ~Private() {
        qDeleteAll(m_ions);
    }

    /**
     * Get instance of a loaded ion.
     * @returns a IonInterface instance of a loaded plugin.
     */
    IonInterface* ionForSource(const QString& name) {
        int offset = name.indexOf('|');

        if (offset < 1) {
            return NULL;
        }

        QString ionName = name.left(offset);


        if (m_ions.contains(ionName)) {
            return m_ions[ionName];
        }

        return NULL;
    }

    /**
     * Get plugin name from datasource.
     * @returns The plugin name given a datasource.
     */
    QString ionNameForSource(const QString& source) {
        int offset = source.indexOf('|');
        if (offset < 1) {
            return QString();
        }

        return QString(source.left(offset));
    }

    KService::List m_ionServices;
    IonInterface::IonDict m_ions;
    KDateTime m_localTime;
};

/**
 * Returns an instance of an ion plugin loaded.
 */
IonInterface* WeatherEngine::Ion(const QString& name) const
{
    IonInterface::IonDict::const_iterator it = d->m_ions.find(name);
    if (it != d->m_ions.end()) {
        return *it;
    }

    return NULL;
}

/**
 * Loads an ion plugin given a plugin name found via KService.
 */
IonInterface* WeatherEngine::loadIon(const QString& plugName)
{
    IonInterface *ion = 0;
    KService::Ptr foundPlugin;

    foreach(KService::Ptr service, d->m_ionServices) {
        if (service->property("X-IonName").toString() == plugName) {
            foundPlugin = service;
            break;
        }
    }

    // Check if the plugin is already loaded if so, return the plugin thats already loaded.
    IonInterface::IonDict::const_iterator it = d->m_ions.find(plugName);

    if (it != d->m_ions.end()) {
        ion = *it;
        ion->ref();
        return ion;
    }

    QString error;

    // Do we have a valid plugin?
    if (!foundPlugin) {
        return NULL;
    }

    // Load the Ion plugin, store it into a QMap to handle multiple ions.
    ion = foundPlugin->createInstance<IonInterface>(0, QVariantList(), &error);
    ion->setObjectName(plugName);
    if (!ion) {
        kDebug() << "weatherengine: Couldn't load ion \"" << plugName << "\"!" << error;
        return NULL;
    }

    // Increment counter of ion.
    ion->init();
    ion->ref();

    connect(ion, SIGNAL(sourceAdded(QString)), this, SLOT(newIonSource(QString)));

    /* Set system related properties for the ion
     * timezone is displaying the time/date in UTC or user's local time
     * unit is setting the weather units used, Celsius/Fahrenheit, Kilopascals/Inches of Mercury, etc
     */

    ion->setProperty("timezone", d->m_localTime.isUtc());
    ion->setProperty("unit", KGlobal::locale()->measureSystem());

    // Assign the instantiated ion the key of the name of the ion.
    if (!d->m_ions.contains(plugName)) {
        d->m_ions[plugName] = ion;
    }

    return ion;
}

/**
 * Unload an Ion plugin given a Ion plugin name.
 */
void WeatherEngine::unloadIon(const QString &name)
{
    IonInterface *ion = Ion(name);

    if (ion) {
        ion->deref();
        kDebug() << "Unloading Plugin: " << name;
        if (!ion->isUsed()) {
            kDebug() << "It's not used anymore, delete it!";
            d->m_ions.remove(name);
            delete ion;
        }
    }
}

/**
 * Return a list of Ion plugins found.
 */
KService::List WeatherEngine::knownIons()
{
    KService::List offers = KServiceTypeTrader::self()->query("WeatherEngine/Ion");

    if (offers.isEmpty()) {
        kDebug() << "weatherengine: No plugins to load!";
        return KService::List();
    }

    foreach(KService::Ptr service, offers) {
        setData("ions", service->property("X-IonName").toString(), QString("%1|%2").arg(service->property("Name").toString()).arg(service->property("X-IonName").toString()));
    }

    return offers;
}

/**
 * SLOT: Get data from a new source
 */
void WeatherEngine::newIonSource(const QString& source)
{
    IonInterface *ion = qobject_cast<IonInterface*>(sender());

    kDebug() << "New Ion Source" << source;
    if (!ion) {
        return;
    }

    ion->connectSource(source, this);
}

/**
 * SLOT: Remove the datasource from the ion and unload plugin if needed
 */
void WeatherEngine::removeIonSource(const QString& source)
{
    IonInterface *ion = d->ionForSource(source);
    if (ion) {
        ion->removeSource(source);
        // If plugin has no more sources let's unload the plugin
        if (ion->isEmpty()) {
            kDebug() << "No more Sources found for this plugin let's unload it!";
            unloadIon(d->ionNameForSource(source));
        }
    }
}

/**
 * SLOT: Push out new data to applet
 */
void WeatherEngine::dataUpdated(const QString& source, Plasma::DataEngine::Data data)
{
    kDebug() << "data updated" << source;
    setData(source, data);
}

// Constructor
WeatherEngine::WeatherEngine(QObject *parent, const QVariantList& args)
        :  Plasma::DataEngine(parent), d(new Private())
{
    Q_UNUSED(args)

    // Set any local properties for Ion to use
    d->m_localTime = KDateTime::currentDateTime(KDateTime::LocalZone);

    // Get the list of available plugins but don't load them
    d->m_ionServices = knownIons();

    // Globally notify all plugins to remove their sources (and unload plugin)
    connect(this, SIGNAL(sourceRemoved(QString)), this, SLOT(removeIonSource(QString)));
}

// Destructor
WeatherEngine::~WeatherEngine()
{
    // Cleanup all private data.
    delete d;
}

/**
 * SLOT: Set up each Ion for the first time and get any data
 */
bool WeatherEngine::sourceRequested(const QString &source)
{
    kDebug() << "sourceRequested()" << source;
    IonInterface *ion = d->ionForSource(source);

    if (!ion) {
        kDebug() << "sourceRequested(): No Ion Found, load it up!";
        ion = loadIon(d->ionNameForSource(source));
        if (!ion) {
            return false;
        }
    }

    QByteArray str = source.toLocal8Bit();

    ion->connectSource(source, this);
    if (!containerForSource(source)) {
        // it is an async reply, we need to set up the data anyways
        kDebug() << "no item?";
        setData(source, Data());
    }
    return true;
}

/**
 * SLOT: update the Applet with new data from all ions loaded.
 */
bool WeatherEngine::updateSource(const QString& source)
{
    IonInterface *ion = d->ionForSource(source);

    ion->setProperty("timezone", d->m_localTime.isUtc());
    ion->setProperty("unit", KGlobal::locale()->measureSystem());
    QByteArray str = source.toLocal8Bit();

    kDebug() << "updateSource()";
    if (!ion) {
        return false;
    }

    if (ion->updateSource(source)) {
        return true;
    } else {
        return false;
    }
}

#include "weatherengine.moc"
