/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007-2008 Sebastian Kuegler <sebas@kde.org>
 *   CopyRight 2007 Maor Vanmak <mvanmak1@gmail.com>
 *   Copyright 2008 Dario Freddi <drf54321@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "powermanagementengine.h"

//solid specific includes
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/battery.h>
#include <solid/powermanagement.h>
#include <solid/control/powermanager.h>

#include <KDebug>
#include <KLocale>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <Plasma/DataContainer>

PowermanagementEngine::PowermanagementEngine(QObject* parent, const QVariantList& args)
        : Plasma::DataEngine(parent, args)
        , m_acadapter(0)
        , m_sources(0)
        , m_dbus(QDBusConnection::sessionBus())
{
    Q_UNUSED(args)

    m_sources << "Battery" << "AC Adapter" << "Sleepstates" << "PowerDevil";

    // This following call can be removed, but if we do, the
    // data is not shown in the plasmaengineexplorer.
    // sourceRequestEvent("Battery");
}

PowermanagementEngine::~PowermanagementEngine()
{}

void PowermanagementEngine::init()
{
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
            this,                              SLOT(deviceRemoved(QString)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
            this,                              SLOT(deviceAdded(QString)));
    connect(Solid::Control::PowerManager::notifier(), SIGNAL(batteryRemainingTimeChanged(int)),
            this,                                     SLOT(batteryRemainingTimeChanged(int)));

    QStringList modules;
    QDBusInterface kdedInterface("org.kde.kded", "/kded", "org.kde.kded");
    QDBusReply<QStringList> reply = kdedInterface.call("loadedModules");

    if (!reply.isValid()) {
        return;
    }

    modules = reply.value();

    if (modules.contains("powerdevil")) {

        if (!m_dbus.connect("org.kde.kded", "/modules/powerdevil", "org.kde.PowerDevil",
                          "profileChanged", this,
                           SLOT(profilesChanged(const QString&, const QStringList&)))) {
            kDebug() << "error!";
        }

        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.kded", "/modules/powerdevil",
                           "org.kde.PowerDevil", "streamData");
        m_dbus.call(msg);

    }
}

QStringList PowermanagementEngine::sources() const
{
    return m_sources + m_batterySources.values();
}

bool PowermanagementEngine::sourceRequestEvent(const QString &name)
{
    if (name == "Battery") {
        QList<Solid::Device> list_battery =
                        Solid::Device::listFromType(Solid::DeviceInterface::Battery, QString());
        if (list_battery.count() == 0) {
            setData("Battery", "has Battery", false);
            return true;
        }

        uint index = 0;
        QStringList battery_sources;

        foreach (const Solid::Device &device_battery, list_battery) {
            const Solid::Battery* battery = device_battery.as<Solid::Battery>();

            if(battery != 0) {
                if(battery->type() == Solid::Battery::PrimaryBattery) {
                    QString source = QString("Battery%1").arg(index++);

                    battery_sources<<source;

                    m_batterySources[device_battery.udi()] = source;

                    connect(battery, SIGNAL(chargeStateChanged(int, const QString &)), this,
                            SLOT(updateBatteryChargeState(int, const QString &)));
                    connect(battery, SIGNAL(chargePercentChanged(int, const QString &)), this,
                            SLOT(updateBatteryChargePercent(int, const QString &)));
                    connect(battery, SIGNAL(plugStateChanged(bool, const QString &)), this,
                            SLOT(updateBatteryPlugState(bool, const QString &)));

                    // Set initial values
                    updateBatteryChargeState(battery->chargeState(), device_battery.udi());
                    updateBatteryChargePercent(battery->chargePercent(), device_battery.udi());
                    updateBatteryPlugState(battery->isPlugged(), device_battery.udi());
                }
            }
        }

        if (battery_sources.count() > 0) {
            setData("Battery", "has Battery", true);
            setData("Battery", "sources", battery_sources);
            setData("Battery", "remaining_time", Solid::Control::PowerManager::batteryRemainingTime());
        }
    } else if (name == "AC Adapter") {
        // AC Adapter handling
        QList<Solid::Device> list_ac =
                        Solid::Device::listFromType(Solid::DeviceInterface::AcAdapter, QString());
        foreach (Solid::Device device_ac, list_ac) {
            m_acadapter = device_ac.as<Solid::AcAdapter>();
            updateAcPlugState(m_acadapter->isPlugged());
            connect(m_acadapter, SIGNAL(plugStateChanged(bool, const QString &)), this,
                    SLOT(updateAcPlugState(bool)));
        }
    } else if (name == "Sleepstates") {
        QSet<Solid::PowerManagement::SleepState> sleepstates =
                                Solid::PowerManagement::supportedSleepStates();
        // We first set all possible sleepstates to false, then enable the ones that are available
        setData("Sleepstates", "Standby", false);
        setData("Sleepstates", "Suspend", false);
        setData("Sleepstates", "Hibernate", false);

        foreach (const Solid::PowerManagement::SleepState &sleepstate, sleepstates) {
            if (sleepstate == Solid::PowerManagement::StandbyState) {
                setData("Sleepstates", "Supports standby", true);
            } else if (sleepstate == Solid::PowerManagement::SuspendState) {
                setData("Sleepstates", "Supports suspend", true);
            } else if (sleepstate == Solid::PowerManagement::HibernateState) {
                setData("Sleepstates", "Supports hibernate", true);
            }
            kDebug() << "Sleepstate \"" << sleepstate << "\" supported.";
        }
    } else if (name == "PowerDevil") {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.kded", "/modules/powerdevil",
                           "org.kde.PowerDevil", "streamData");
        m_dbus.call(msg);
    } else {
        kDebug() << "Data for '" << name << "' not found";
    }
    return true;
}

void PowermanagementEngine::updateBatteryChargeState(int newState, const QString& udi)
{
    QString state;
    if (newState == Solid::Battery::NoCharge) {
        state = "NoCharge";
    } else if (newState == Solid::Battery::Charging) {
        state = "Charging";
    } else if (newState == Solid::Battery::Discharging) {
        state = "Discharging";
    } else {
        state = "Could not determine battery status. Something is fishy here. :o";
    }
    const QString& source = m_batterySources[udi];
    setData(source, "State", state);
    scheduleSourcesUpdated();
}

void PowermanagementEngine::updateBatteryPlugState(bool newState, const QString& udi)
{
    const QString& source = m_batterySources[udi];
    setData(source, "Plugged in", newState);
    scheduleSourcesUpdated();
}

void PowermanagementEngine::updateBatteryChargePercent(int newValue, const QString& udi)
{
    const QString& source = m_batterySources[udi];
    setData(source, "Percent", newValue);
    scheduleSourcesUpdated();
}

void PowermanagementEngine::updateAcPlugState(bool newState)
{
    setData("AC Adapter", "Plugged in", newState);
    scheduleSourcesUpdated();
}

void PowermanagementEngine::deviceRemoved(const QString& udi)
{
    if (m_batterySources.contains(udi)) {
        QString source = m_batterySources[udi];
        m_batterySources.remove(udi);
        removeSource(source);

        QStringList sourceNames(m_batterySources.values());
        sourceNames.removeAll(source);
        setData("Battery", "sources", sourceNames);
    }
}

void PowermanagementEngine::deviceAdded(const QString& udi)
{
    Solid::Device device(udi);
    if (device.isValid()) {
        const Solid::Battery* battery = device.as<Solid::Battery>();

        if (battery != 0) {
            int index = 0;
            QStringList sourceNames(m_batterySources.values());
            while (sourceNames.contains(QString("Battery%1").arg(index))) {
                index++;
            }

            QString source = QString("Battery%1").arg(index);
            sourceNames << source;
            m_batterySources[device.udi()] = source;

            connect(battery, SIGNAL(chargeStateChanged(int, const QString &)), this,
                    SLOT(updateBatteryChargeState(int, const QString &)));
            connect(battery, SIGNAL(chargePercentChanged(int, const QString &)), this,
                    SLOT(updateBatteryChargePercent(int, const QString &)));
            connect(battery, SIGNAL(plugStateChanged(bool, const QString &)), this,
                    SLOT(updateBatteryPlugState(bool, const QString &)));

            // Set initial values
            updateBatteryChargeState(battery->chargeState(), device.udi());
            updateBatteryChargePercent(battery->chargePercent(), device.udi());
            updateBatteryPlugState(battery->isPlugged(), device.udi());

            setData("Battery", "sources", sourceNames);
        }
    }
}

void PowermanagementEngine::profilesChanged( const QString &current, const QStringList &profiles )
{
    setData("PowerDevil", "currentProfile", current);
    setData("PowerDevil", "availableProfiles", profiles);
    scheduleSourcesUpdated();
}

void PowermanagementEngine::batteryRemainingTimeChanged(int time)
{
    setData("Battery", "remaining_time", time);
    scheduleSourcesUpdated();
}

#include "powermanagementengine.moc"
