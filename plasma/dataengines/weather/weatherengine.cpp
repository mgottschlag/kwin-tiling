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

    IonInterface* ionForSource(const QString& name)
    {
        int offset = name.indexOf(':');

        if (offset < 1) {
            return 0;
        }

        QString ionName = name.left(offset);

        if (!this->m_ions.contains(ionName)) {
            return 0;
        }

        return this->m_ions[ionName];
    }

    QString ionNameForSource(const QString& source) 
    {
        int offset = source.indexOf(':'); 
        if (offset < 1) {
            return QString();
        }
  
        return QString(source.left(offset));
    }

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
IonInterface* WeatherEngine::loadIon(const QString& plugName)
{
    IonInterface *ion = 0;
    KService::Ptr foundPlugin;
  
      
   foreach(KService::Ptr service, knownIons()) {
      if (service->property("X-IonName").toString() == plugName) {
          kDebug() << "!!!!!!!!!!!!!!!! FOUND PLUGIN TO LOAD: " << plugName;
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

    // Load the Ion plugin, store it into a QMap to handle multiple ions.
    ion = foundPlugin->createInstance<IonInterface>(0, QVariantList(), &error);
    ion->setObjectName(plugName);
    if (!ion) {
        kDebug() << "weatherengine: Couldn't load ion \"" << plugName << "\"!" << error;
        return 0;
    }

    // Increment counter of ions.
    ion->ref();

    connect(ion, SIGNAL(newSource(QString)), this, SLOT(newIonSource(QString)));
    connect(this, SIGNAL(sourceRemoved(QString)), this, SLOT(removeIonSource(QString)));

    /* Set properties for the ion
     *
     * TIMEFORMAT is displaying the time/date in UTC or user's local time
     * UNITS is setting the weather units used, Celsius/Fahrenheit, Kilopascals/Inches of Mercury, etc
     * WINDFORMAT enable winds to be displayed as meters per second (m/s) some countries display winds like this
     */

    ion->option(IonInterface::TIMEFORMAT, QVariant(d->m_localTime.isUtc()));
    ion->option(IonInterface::UNITS, KGlobal::locale()->measureSystem());
    ion->option(IonInterface::WINDFORMAT, QVariant(false)); // FIXME: Should be configurable by applet

    // Assign the instantiated ion the key of the name of the ion.
    d->m_ions[plugName] = ion;

    return ion;
}

// Unload an Ion plugin given a Ion plugin name.
void WeatherEngine::unloadIon(const QString &name)
{
    IonInterface *ion = Ion(name);

    if (ion) {
        kDebug() << "Unloading Plugin: " << name;
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

    if (offers.isEmpty()) {
        kDebug() << "weatherengine: No plugins to load!";
        return KService::List();
    }

    foreach(KService::Ptr service, offers) {
        setData("ions", service->property("X-IonName").toString(), QString("%1:%2").arg(service->property("Name").toString()).arg(service->property("X-IonName").toString()));
    }

    return offers;
}

void WeatherEngine::newIonSource(const QString& source)
{
    IonInterface *ion = qobject_cast<IonInterface*>(sender());

    kDebug() << "New Ion Source" << source;
    if (!ion) {
        return;
    }
    ion->connectSource(source, this);
}

void WeatherEngine::removeIonSource(const QString& source)
{
    kDebug() << "Removing source from WeatherEngine: " << source;
    kDebug() << "Plugin to remove source from: " << d->ionForSource(source)->objectName();
    d->ionForSource(source)->removeSource(source);
    
    // If plugin has no more sources let's unload the plugin

    if (d->ionForSource(source)->isEmpty()) {
        kDebug() << "No more Sources found for this plugin let's unload it!";
        unloadIon(d->ionNameForSource(source)); 
    }
}

void WeatherEngine::dataUpdated(const QString& source, Plasma::DataEngine::Data data)
{
    kDebug() << "data updated" << source;
    setData(source, data);
}

// ctor
WeatherEngine::WeatherEngine(QObject *parent, const QVariantList& args)
        :  Plasma::DataEngine(parent), d(new Private())
{
    Q_UNUSED(args)

    // Set any local properties for Ion to use
    d->m_localTime = KDateTime::currentDateTime(KDateTime::LocalZone);
    
    // Get the list of available plugins but don't load them
    knownIons();
}

// dtor
WeatherEngine::~WeatherEngine()
{
    // Cleanup all private data.
    delete d;
}

// Setup each Ion for the first time
bool WeatherEngine::sourceRequested(const QString &source)
{
    kDebug() << "SOURCES FOUND: " << sources();
    
    kDebug() << "sourceRequested()" << source;
    IonInterface *ion = d->ionForSource(source);

    if (!ion) {
        kDebug() << "sourceRequested(): No Ion Found, load it up!";
        ion = loadIon(d->ionNameForSource(source));
        if (!ion) {
            return false;
        }
    }

    kDebug() << "About to connect Source" << source << " To the Ion!";
    kDebug() << "IONS SOURCES: " << ion->sources();
    ion->connectSource(source, this);
    if (!containerForSource(source)) {
        // it is an async reply, we need to set up the data anyways
        kDebug() << "no item?";
        setData(source, Data());
    }
    return true;
}

// SLOT: update the Applet with new data from all ions loaded.

bool WeatherEngine::updateSource(const QString& source)
{
    IonInterface *ion = d->ionForSource(source);
 
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
