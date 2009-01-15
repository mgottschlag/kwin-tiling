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

#include <Plasma/DataEngineManager>

#include "ions/ion.h"

class WeatherEngine::Private
{
public:
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
        return qobject_cast<IonInterface *>(Plasma::DataEngineManager::self()->engine(ionName));
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

    KDateTime m_localTime;
    QStringList m_ions;
};

/**
 * Loads an ion plugin given a plugin name found via KService.
 */
Plasma::DataEngine *WeatherEngine::loadIon(const QString& plugName)
{
    KPluginInfo foundPlugin;

    foreach (const KPluginInfo &info, Plasma::DataEngineManager::listEngineInfo("weatherengine")) {
        if (info.pluginName() == plugName) {
            foundPlugin = info;
            break;
        }
    }

    if (!foundPlugin.isValid()) {
        return NULL;
    }

    // Load the Ion plugin, store it into a QMap to handle multiple ions.
    Plasma::DataEngine *ion = Plasma::DataEngineManager::self()->loadEngine(foundPlugin.pluginName());
    ion->setObjectName(plugName);
    connect(ion, SIGNAL(sourceAdded(QString)), this, SLOT(newIonSource(QString)));

    /* Set system related properties for the ion
     * timezone is displaying the time/date in UTC or user's local time
     * unit is setting the weather units used, Celsius/Fahrenheit, Kilopascals/Inches of Mercury, etc
     */

    ion->setProperty("timezone", d->m_localTime.isUtc());
    ion->setProperty("unit", KGlobal::locale()->measureSystem());
    d->m_ions << plugName;

    return ion;
}

/**
 * Unload an Ion plugin given a Ion plugin name.
 */
void WeatherEngine::unloadIon(const QString &name)
{
    Plasma::DataEngineManager::self()->unloadEngine(name);
    d->m_ions.removeOne(name);
}

void WeatherEngine::init()
{
    // Get the list of available plugins but don't load them
    foreach(const KPluginInfo &info, Plasma::DataEngineManager::listEngineInfo("weatherengine")) {
        setData("ions", info.pluginName(),
                QString("%1|%2").arg(info.property("Name").toString()).arg(info.pluginName()));
    }
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
        :  Plasma::DataEngine(parent, args), d(new Private())
{
    Q_UNUSED(args)

    // Set any local properties for Ion to use
    d->m_localTime = KDateTime::currentDateTime(KDateTime::LocalZone);

    // Globally notify all plugins to remove their sources (and unload plugin)
    connect(this, SIGNAL(sourceRemoved(QString)), this, SLOT(removeIonSource(QString)));
}

// Destructor
WeatherEngine::~WeatherEngine()
{
    // Cleanup all private data.
    foreach (const QString &ion, d->m_ions) {
        Plasma::DataEngineManager::self()->unloadEngine(ion);
    }

    delete d;
}

/**
 * SLOT: Set up each Ion for the first time and get any data
 */
bool WeatherEngine::sourceRequestEvent(const QString &source)
{
    kDebug() << "sourceRequestEvent()" << source;
    Plasma::DataEngine *ion = d->ionForSource(source);

    if (!ion) {
        kDebug() << "sourceRequestEvent(): No Ion Found, load it up!";
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
bool WeatherEngine::updateSourceEvent(const QString& source)
{
    IonInterface *ion = d->ionForSource(source);

    QByteArray str = source.toLocal8Bit();

    kDebug() << "updateSourceEvent()";
    if (!ion) {
        return false;
    }

    ion->setProperty("timezone", d->m_localTime.isUtc());
    ion->setProperty("unit", KGlobal::locale()->measureSystem());

    if (ion->updateSourceEvent(source)) {
        return true;
    } else {
        return false;
    }
}

#include "weatherengine.moc"
