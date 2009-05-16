/***************************************************************************
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *   Copyright (C) 2009 by Aaron Seigo <aseigo@kde.org>                    *
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

#include <QCoreApplication>

#include <KServiceTypeTrader>
#include <KDateTime>
#include <KLocale>
#include <QTimer>

#include <Plasma/DataEngineManager>

#include "ions/ion.h"

class WeatherEngine::Private
{
public:
    Private()
            : m_networkAvailable(false) {
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

    QStringList m_ions;
    bool m_networkAvailable;
};

/**
 * Loads an ion plugin given a plugin name found via KService.
 */
Plasma::DataEngine *WeatherEngine::loadIon(const QString& plugName)
{
    KPluginInfo foundPlugin;

    foreach(const KPluginInfo &info, Plasma::DataEngineManager::listEngineInfo("weatherengine")) {
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
    connect(ion, SIGNAL(resetCompleted(IonInterface *, bool)), this, SLOT(resetCompleted(IonInterface *, bool)));

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

    Solid::Networking::Status status = Solid::Networking::status();
    d->m_networkAvailable = (status == Solid::Networking::Connected ||
                             status == Solid::Networking::Unknown);
    connect(Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
            this, SLOT(networkStatusChanged(Solid::Networking::Status)));

    kDebug() << "init()";
}

/**
 * SLOT: Get data from a new source
 */
void WeatherEngine::newIonSource(const QString& source)
{
    IonInterface *ion = qobject_cast<IonInterface*>(sender());

    if (!ion) {
        return;
    }

    kDebug() << "newIonSource()";
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
            unloadIon(d->ionNameForSource(source));
        }
    }
    kDebug() << "removeIonSource()";
}

/**
 * SLOT: Push out new data to applet
 */
void WeatherEngine::dataUpdated(const QString& source, Plasma::DataEngine::Data data)
{
    kDebug() << "dataUpdated()";
    setData(source, data);
}

// Constructor
WeatherEngine::WeatherEngine(QObject *parent, const QVariantList& args)
        :  Plasma::DataEngine(parent, args), d(new Private())
{
    Q_UNUSED(args)

    // Globally notify all plugins to remove their sources (and unload plugin)
    connect(this, SIGNAL(sourceRemoved(QString)), this, SLOT(removeIonSource(QString)));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(unloadIons()));
}

// Destructor
WeatherEngine::~WeatherEngine()
{
    // Cleanup all private data.
    unloadIons();
    delete d;
}

void WeatherEngine::unloadIons()
{
    foreach(const QString &ion, d->m_ions) {
        Plasma::DataEngineManager::self()->unloadEngine(ion);
    }

    d->m_ions.clear();
}

/**
 * SLOT: Set up each Ion for the first time and get any data
 */
bool WeatherEngine::sourceRequestEvent(const QString &source)
{
    Plasma::DataEngine *ion = d->ionForSource(source);

    if (!ion) {
        ion = loadIon(d->ionNameForSource(source));
        if (!ion) {
            return false;
        }
    }

    kDebug() << "sourceRequestEvent(): Network is: " << d->m_networkAvailable;
    if (!d->m_networkAvailable) {
        setData(source, Data());
        return true;
    }

    QByteArray str = source.toLocal8Bit();

    ion->connectSource(source, this);
    if (!containerForSource(source)) {
        // it is an async reply, we need to set up the data anyways
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

    if (!ion) {
        return false;
    }

    kDebug() << "updateSourceEvent(): Network is: " << d->m_networkAvailable;
    if (!d->m_networkAvailable) {
        return false;
    }

    return ion->updateSourceEvent(source);
}

void WeatherEngine::triggerReset()
{
    kDebug() << "triggerReset()";
    foreach(const QString &i, d->m_ions) {
        IonInterface * ion = qobject_cast<IonInterface *>(Plasma::DataEngineManager::self()->engine(i));
        if (ion) {
            ion->reset();
        }
    }
}

void WeatherEngine::networkStatusChanged(Solid::Networking::Status status)
{
    d->m_networkAvailable = (status == Solid::Networking::Connected || status == Solid::Networking::Unknown);
    kDebug() << "networkStatusChanged(): Status changed: " << d->m_networkAvailable << "state: " << status;

    if (d->m_networkAvailable) {
        QTimer::singleShot(6000, this, SLOT(triggerReset()));
    }
}

void WeatherEngine::resetCompleted(IonInterface * i, bool b)
{
    disconnect(i, SIGNAL(resetCompleted(IonInterface*, bool)), this, SLOT(resetCompleted(IonInterface *, bool)));
    if (b) {
        foreach(const QString &source, sources()) {
            IonInterface *ion = d->ionForSource(source);
            if (ion == i) {
                ion->updateSourceEvent(source);
            }
        }
    }
}

#include "weatherengine.moc"
