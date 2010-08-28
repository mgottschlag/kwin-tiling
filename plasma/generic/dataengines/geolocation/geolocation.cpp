/*
 *   Copyright (C) 2009 Petri Damsten <damu@iki.fi>
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

#include "geolocation.h"

#include <limits.h>

#include <KDebug>
#include <KServiceTypeTrader>

static const char SOURCE[] = "location";

Geolocation::Geolocation(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args),
      m_networkStatus(false)
{
    Q_UNUSED(args)
    setMinimumPollingInterval(500);
    connect(Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
            this, SLOT(networkStatusChanged()));
    m_updateTimer.setInterval(100);
    m_updateTimer.setSingleShot(true);
    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(actuallySetData()));
}

void Geolocation::init()
{
    m_networkStatus = Solid::Networking::status();

    //TODO: should this be delayed even further, e.g. when the source is requested?
    const KService::List offers = KServiceTypeTrader::self()->query("Plasma/GeolocationProvider");
    QVariantList args;

    foreach (const KService::Ptr service, offers) {
        QString error;
        GeolocationProvider *plugin = service->createInstance<GeolocationProvider>(0, args, &error);
        if (plugin) {
            m_plugins << plugin;
            plugin->init(&m_data, &m_accuracy);
            connect(plugin, SIGNAL(updated()), this, SLOT(pluginUpdated()));
            connect(plugin, SIGNAL(availabilityChanged(GeolocationProvider*)),
                    this, SLOT(pluginAvailabilityChanged(GeolocationProvider*)));
        } else {
            kDebug() << "Failed to load GeolocationProvider:" << error;
        }
    }
}

Geolocation::~Geolocation()
{
    qDeleteAll(m_plugins);
}

QStringList Geolocation::sources() const
{
    return QStringList() << SOURCE;
}

bool Geolocation::updateSourceEvent(const QString &name)
{
    //kDebug() << name;
    if (name == SOURCE) {
        return updatePlugins(GeolocationProvider::SourceEvent);
    }

    return false;
}

bool Geolocation::updatePlugins(GeolocationProvider::UpdateTriggers triggers)
{
    bool changed = false;

    foreach (GeolocationProvider *plugin, m_plugins) {
        changed = plugin->requestUpdate(triggers) || changed;
    }

    if (changed) {
        m_updateTimer.start();
    }

    return changed;
}

bool Geolocation::sourceRequestEvent(const QString &name)
{
    kDebug() << name;
    if (name == SOURCE) {
        updatePlugins(GeolocationProvider::ForcedUpdate);
        setData(SOURCE, m_data);
        return true;
    }

    return false;
}

void Geolocation::networkStatusChanged()
{
    kDebug() << "network status changed";
    m_networkStatus = Solid::Networking::status();
    if ((m_networkStatus == Solid::Networking::Connected) ||
        (m_networkStatus == Solid::Networking::Unknown)) {
        updatePlugins(GeolocationProvider::NetworkConnected);
    }
}

void Geolocation::pluginAvailabilityChanged(GeolocationProvider *provider)
{
    m_data.clear();
    m_accuracy.clear();

    provider->requestUpdate(GeolocationProvider::ForcedUpdate);

    bool changed = false;
    foreach (GeolocationProvider *plugin, m_plugins) {
        changed = plugin->populateSharedData() || changed;
    }

    if (changed) {
        m_updateTimer.start();
    }
}

void Geolocation::pluginUpdated()
{
    m_updateTimer.start();
}

void Geolocation::actuallySetData()
{
    setData(SOURCE, m_data);
}

#include "geolocation.moc"
