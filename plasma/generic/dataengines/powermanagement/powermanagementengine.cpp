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

#include <KDebug>
#include <KLocale>
#include <KStandardDirs>

#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusReply>

#include <Plasma/DataContainer>
#include "powermanagementservice.h"

typedef QMap< QString, QString > StringStringMap;
Q_DECLARE_METATYPE(StringStringMap)

PowermanagementEngine::PowermanagementEngine(QObject* parent, const QVariantList& args)
        : Plasma::DataEngine(parent, args)
        , m_sources(basicSourceNames())
{
    Q_UNUSED(args)
    qDBusRegisterMetaType< StringStringMap >();
}

PowermanagementEngine::~PowermanagementEngine()
{}

void PowermanagementEngine::init()
{
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
            this,                              SLOT(deviceRemoved(QString)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
            this,                              SLOT(deviceAdded(QString)));

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
        if (!QDBusConnection::sessionBus().connect("org.kde.Solid.PowerManagement",
                                                   "/org/kde/Solid/PowerManagement",
                                                   "org.kde.Solid.PowerManagement",
                                                   "profileChanged", this,
                                                   SLOT(profileChanged(QString)))) {
            kDebug() << "error connecting to Profile changes via dbus";
        }
        if (!QDBusConnection::sessionBus().connect("org.kde.Solid.PowerManagement",
                                                   "/org/kde/Solid/PowerManagement",
                                                   "org.kde.Solid.PowerManagement",
                                                   "batteryRemainingTimeChanged", this,
                                                   SLOT(batteryRemainingTimeChanged(qulonglong)))) {
            kDebug() << "error connecting to remaining time changes";
        }
        // Listen to profile changes
        if (!QDBusConnection::sessionBus().connect("org.kde.Solid.PowerManagement",
                                                   "/org/kde/Solid/PowerManagement",
                                                   "org.kde.Solid.PowerManagement",
                                                   "configurationReloaded", this,
                                                   SLOT(availableProfilesChanged()))) {
            kDebug() << "error connecting to configuration changes";
        }

        setData("PowerDevil", DataEngine::Data());

        reloadPowerDevilData();
    }
}

QStringList PowermanagementEngine::basicSourceNames() const
{
    QStringList sources;
    sources << "Battery" << "AC Adapter" << "Sleep States" << "PowerDevil";
    return sources;
}

QStringList PowermanagementEngine::sources() const
{
    return m_sources;
}

bool PowermanagementEngine::sourceRequestEvent(const QString &name)
{
    if (name == "Battery") {
        const QList<Solid::Device> listBattery =
                        Solid::Device::listFromType(Solid::DeviceInterface::Battery, QString());
        m_batterySources.clear();

        if (listBattery.isEmpty()) {
            setData("Battery", "Has battery", false);
            return true;
        }

        uint index = 0;
        QStringList batterySources;

        foreach (const Solid::Device &deviceBattery, listBattery) {
            const Solid::Battery* battery = deviceBattery.as<Solid::Battery>();

            if (battery && (battery->type() == Solid::Battery::PrimaryBattery ||
                            battery->type() == Solid::Battery::UpsBattery)) {
                const QString source = QString("Battery%1").arg(index++);

                batterySources << source;
                m_batterySources[deviceBattery.udi()] = source;

                connect(battery, SIGNAL(chargeStateChanged(int,QString)), this,
                        SLOT(updateBatteryChargeState(int,QString)));
                connect(battery, SIGNAL(chargePercentChanged(int,QString)), this,
                        SLOT(updateBatteryChargePercent(int,QString)));
                connect(battery, SIGNAL(plugStateChanged(bool,QString)), this,
                        SLOT(updateBatteryPlugState(bool,QString)));

                // Set initial values
                updateBatteryChargeState(battery->chargeState(), deviceBattery.udi());
                updateBatteryChargePercent(battery->chargePercent(), deviceBattery.udi());
                updateBatteryPlugState(battery->isPlugged(), deviceBattery.udi());
            }
        }

        setData("Battery", "Has battery", !batterySources.isEmpty());
        if (!batterySources.isEmpty()) {
            setData("Battery", "Sources", batterySources);
            QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                              "/org/kde/Solid/PowerManagement",
                                                              "org.kde.Solid.PowerManagement",
                                                              "batteryRemainingTime");
            QDBusPendingReply< int > reply = QDBusConnection::sessionBus().asyncCall(msg);
            reply.waitForFinished();
            if (reply.isValid()) {
                //kDebug() << "Remaining time 1:" << reply.value();
                setData("Battery", "Remaining msec", reply.value());
            }
        }

        m_sources = basicSourceNames() + batterySources;
    } else if (name == "AC Adapter") {
        bool isPlugged = false;

        const QList<Solid::Device> list_ac = Solid::Device::listFromType(Solid::DeviceInterface::AcAdapter, QString());
        foreach (Solid::Device device_ac, list_ac) {
            Solid::AcAdapter* acadapter = device_ac.as<Solid::AcAdapter>();
            isPlugged |= acadapter->isPlugged();
            connect(acadapter, SIGNAL(plugStateChanged(bool,QString)), this,
                    SLOT(updateAcPlugState(bool)), Qt::UniqueConnection);
        }

        updateAcPlugState(isPlugged);
    } else if (name == "Sleep States") {
        const QSet<Solid::PowerManagement::SleepState> sleepstates =
                                Solid::PowerManagement::supportedSleepStates();
        // We first set all possible sleepstates to false, then enable the ones that are available
        setData("Sleep States", "Standby", false);
        setData("Sleep States", "Suspend", false);
        setData("Sleep States", "Hibernate", false);

        foreach (const Solid::PowerManagement::SleepState &sleepstate, sleepstates) {
            if (sleepstate == Solid::PowerManagement::StandbyState) {
                setData("Sleep States", "Standby", true);
            } else if (sleepstate == Solid::PowerManagement::SuspendState) {
                setData("Sleep States", "Suspend", true);
            } else if (sleepstate == Solid::PowerManagement::HibernateState) {
                setData("Sleep States", "Hibernate", true);
            }
            //kDebug() << "Sleepstate \"" << sleepstate << "\" supported.";
        }
    } else if (name == "PowerDevil") {
        setData("PowerDevil", DataEngine::Data());
        reloadPowerDevilData();
    } else {
        kDebug() << "Data for '" << name << "' not found";
        return false;
    }
    return true;
}

Plasma::Service* PowermanagementEngine::serviceForSource(const QString &source)
{
    if (source == "PowerDevil") {
        PowerManagementService *service = new PowerManagementService(source);
        service->setParent(this);
        return service;
    }

    return 0;
}

void PowermanagementEngine::updateBatteryChargeState(int newState, const QString& udi)
{
    QString state("Unknown");
    if (newState == Solid::Battery::NoCharge) {
        state = "NoCharge";
    } else if (newState == Solid::Battery::Charging) {
        state = "Charging";
    } else if (newState == Solid::Battery::Discharging) {
        state = "Discharging";
    }

    const QString source = m_batterySources[udi];
    setData(source, "State", state);
}

void PowermanagementEngine::updateBatteryPlugState(bool newState, const QString& udi)
{
    const QString source = m_batterySources[udi];
    setData(source, "Plugged in", newState);
}

void PowermanagementEngine::updateBatteryChargePercent(int newValue, const QString& udi)
{
    const QString source = m_batterySources[udi];
    setData(source, "Percent", newValue);
}

void PowermanagementEngine::updateAcPlugState(bool newState)
{
    setData("AC Adapter", "Plugged in", newState);
}

void PowermanagementEngine::deviceRemoved(const QString& udi)
{
    if (m_batterySources.contains(udi)) {
        const QString source = m_batterySources[udi];
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

        if (battery && (battery->type() == Solid::Battery::PrimaryBattery ||
                        battery->type() == Solid::Battery::UpsBattery)) {
            int index = 0;
            QStringList sourceNames(m_batterySources.values());
            while (sourceNames.contains(QString("Battery%1").arg(index))) {
                index++;
            }

            const QString source = QString("Battery%1").arg(index);
            sourceNames << source;
            m_batterySources[device.udi()] = source;

            connect(battery, SIGNAL(chargeStateChanged(int,QString)), this,
                    SLOT(updateBatteryChargeState(int,QString)));
            connect(battery, SIGNAL(chargePercentChanged(int,QString)), this,
                    SLOT(updateBatteryChargePercent(int,QString)));
            connect(battery, SIGNAL(plugStateChanged(bool,QString)), this,
                    SLOT(updateBatteryPlugState(bool,QString)));

            // Set initial values
            updateBatteryChargeState(battery->chargeState(), device.udi());
            updateBatteryChargePercent(battery->chargePercent(), device.udi());
            updateBatteryPlugState(battery->isPlugged(), device.udi());

            setData("Battery", "sources", sourceNames);
        }
    }
}

void PowermanagementEngine::profileChanged(const QString &current)
{
    setData("PowerDevil", "Current profile", current);
}

void PowermanagementEngine::batteryRemainingTimeChanged(qulonglong time)
{
    //kDebug() << "Remaining time 2:" << time;
    setData("Battery", "Remaining msec", time);
}

void PowermanagementEngine::availableProfilesChanged()
{
    // Request profiles to the daemon
    QDBusMessage call = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement",
                                                       "org.kde.Solid.PowerManagement", "availableProfiles");
    QDBusPendingReply< StringStringMap > reply = QDBusConnection::sessionBus().asyncCall(call);
    reply.waitForFinished();

    if (!reply.isValid()) {
        kDebug() << "Error contacting the daemon!";
        return;
    }

    StringStringMap profiles = reply.value();

    if (profiles.isEmpty()) {
        kDebug() << "No available profiles!";
        return;
    }

    setData("PowerDevil", "Available profiles", QVariant::fromValue(profiles));
}

void PowermanagementEngine::reloadPowerDevilData()
{
    // Init data
    {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                          "/org/kde/Solid/PowerManagement",
                                                          "org.kde.Solid.PowerManagement",
                                                          "currentProfile");
        QDBusPendingReply< QString > reply = QDBusConnection::sessionBus().asyncCall(msg);
        reply.waitForFinished();
        if (reply.isValid()) {
            profileChanged(reply.value());
        }
    }

    {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                          "/org/kde/Solid/PowerManagement",
                                                          "org.kde.Solid.PowerManagement",
                                                          "batteryRemainingTime");
        QDBusPendingReply< int > reply = QDBusConnection::sessionBus().asyncCall(msg);
        reply.waitForFinished();
        if (reply.isValid()) {
            batteryRemainingTimeChanged(reply.value());
        }
    }

    availableProfilesChanged();
}

K_EXPORT_PLASMA_DATAENGINE(powermanagement, PowermanagementEngine)

#include "powermanagementengine.moc"
