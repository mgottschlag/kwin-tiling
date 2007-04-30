 /*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <QtDBus/QDBusReply>

#include <kdebug.h>

#include "halpower.h"
#include "halsuspendjob.h"

#include <solid/deviceinterface.h>
#include <solid/acadapter.h>
#include <solid/battery.h>
#include <solid/button.h>

HalPower::HalPower(QObject *parent, const QStringList  & /*args */)
    : PowerManager(parent),
      m_halComputer("org.freedesktop.Hal",
                     "/org/freedesktop/Hal/devices/computer",
                     "org.freedesktop.Hal.Device",
                     QDBusConnection::systemBus()),
      m_halPowerManagement("org.freedesktop.Hal",
                            "/org/freedesktop/Hal/devices/computer",
                            "org.freedesktop.Hal.Device.SystemPowerManagement",
                            QDBusConnection::systemBus()),
      m_halCpuFreq("org.freedesktop.Hal",
                    "/org/freedesktop/Hal/devices/computer",
                    "org.freedesktop.Hal.Device.CPUFreq",
                    QDBusConnection::systemBus())
{
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(const QString &)),
            this, SLOT(slotDeviceRemoved(const QString &)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(newDeviceInterface(const QString &, int)),
            this, SLOT(slotNewDeviceInterface(const QString &, int)));

    m_pluggedAdapterCount = 0;
    computeAcAdapters();

    computeBatteries();
    updateBatteryStats();

    computeButtons();
}

HalPower::~HalPower()
{
    QList<Solid::Device *> devices;

    devices << m_acAdapters.values();
    devices << m_batteries.values();
    devices << m_buttons.values();

    foreach (Solid::Device *dev, devices)
    {
        delete dev;
    }
}

QStringList HalPower::supportedSchemes() const
{
    return QStringList() << "performance" << "powersaving";
}

QString HalPower::schemeDescription(const QString &schemeName) const
{
    if (schemeName=="performance")
    {
        return "Use all the performances of the system";
    }
    else if (schemeName=="powersaving")
    {
        return "Try to keep as much power as possible to improve battery life";
    }
    else
    {
        return QString();
    }

    return QString();
}

QString HalPower::scheme() const
{
    // FIXME: We miss an accessor in HAL to make scheme management useful
    return QString();
}

bool HalPower::setScheme(const QString &name)
{
    bool powersave;

    if (name=="powersaving")
    {
        powersave = true;
    }
    else if (name=="performance")
    {
        powersave = false;
    }
    else
    {
        return false;
    }

    QDBusReply<int> reply = m_halPowerManagement.call("SetPowerSave", powersave);

    if (reply.isValid())
    {
        int code = reply;
        return code==0;
    }
    else
    {
        return false;
    }
}

Solid::Control::PowerManager::BatteryState HalPower::batteryState() const
{
    if (m_batteries.size()==0)
    {
        return Solid::Control::PowerManager::NoBatteryState;
    }
    else if (m_currentBatteryCharge <= m_criticalBatteryCharge)
    {
        return Solid::Control::PowerManager::Critical;
    }
    else if (m_currentBatteryCharge <= m_lowBatteryCharge)
    {
        return Solid::Control::PowerManager::Low;
    }
    else if (m_currentBatteryCharge <= m_warningBatteryCharge)
    {
        return Solid::Control::PowerManager::Warning;
    }
    else
    {
        return Solid::Control::PowerManager::Normal;
    }
}

int HalPower::batteryChargePercent() const
{
    return (m_currentBatteryCharge *100)/m_maxBatteryCharge;
}

Solid::Control::PowerManager::AcAdapterState HalPower::acAdapterState() const
{
    if (m_acAdapters.size()==0)
    {
        return Solid::Control::PowerManager::UnknownAcAdapterState;
    }
    else if (m_pluggedAdapterCount==0)
    {
        return Solid::Control::PowerManager::Unplugged;
    }
    else
    {
        return Solid::Control::PowerManager::Plugged;
    }
}

Solid::Control::PowerManager::SuspendMethods HalPower::supportedSuspendMethods() const
{
    Solid::Control::PowerManager::SuspendMethods supported = Solid::Control::PowerManager::UnknownSuspendMethod;

    QDBusReply<bool> reply = m_halComputer.call("GetPropertyBoolean", "power_management.can_hibernate");

    if (reply.isValid())
    {
        bool can_suspend = reply;
        if (can_suspend)
        {
            supported |= Solid::Control::PowerManager::ToRam;
        }
    }
    else
    {
        kDebug() << reply.error().name() << ": " << reply.error().message() << endl;
    }

    reply = m_halComputer.call("GetPropertyBoolean", "power_management.can_hibernate");

    if (reply.isValid())
    {
        bool can_hibernate = reply;
        if (can_hibernate)
        {
            supported |= Solid::Control::PowerManager::ToDisk;
        }
    }
    else
    {
        kDebug() << reply.error().name() << ": " << reply.error().message() << endl;
    }

    return supported;
}

KJob *HalPower::suspend(Solid::Control::PowerManager::SuspendMethod method) const
{
    return new HalSuspendJob(m_halPowerManagement,
                             method, supportedSuspendMethods());
}

Solid::Control::PowerManager::CpuFreqPolicies HalPower::supportedCpuFreqPolicies() const
{
    QDBusReply<QStringList> reply = m_halCpuFreq.call("GetCPUFreqAvailableGovernors");

    if (!reply.isValid())
    {
        return Solid::Control::PowerManager::UnknownCpuFreqPolicy;
    }
    else
    {
        QStringList governors = reply;
        Solid::Control::PowerManager::CpuFreqPolicies policies = Solid::Control::PowerManager::UnknownCpuFreqPolicy;

        foreach (QString governor, governors)
        {
            if (governor == "ondemand")
            {
                policies|= Solid::Control::PowerManager::OnDemand;
            }
            else if (governor == "userspace")
            {
                policies|= Solid::Control::PowerManager::Userspace;
            }
            else if (governor == "powersave")
            {
                policies|= Solid::Control::PowerManager::Powersave;
            }
            else if (governor == "performance")
            {
                policies|= Solid::Control::PowerManager::Performance;
            }
            else
            {
                kWarning() << "Unknown governor: " << governor << endl;
            }
        }

        return policies;
    }
}

Solid::Control::PowerManager::CpuFreqPolicy HalPower::cpuFreqPolicy() const
{
    QDBusReply<QString> reply = m_halCpuFreq.call("GetCPUFreqGovernor");

    if (!reply.isValid())
    {
        return Solid::Control::PowerManager::UnknownCpuFreqPolicy;
    }
    else
    {
        QString governor = reply;

        if (governor == "ondemand")
        {
            return Solid::Control::PowerManager::OnDemand;
        }
        else if (governor == "userspace")
        {
            return Solid::Control::PowerManager::Userspace;
        }
        else if (governor == "powersave")
        {
            return Solid::Control::PowerManager::Powersave;
        }
        else if (governor == "performance")
        {
            return Solid::Control::PowerManager::Performance;
        }
        else
        {
            return Solid::Control::PowerManager::UnknownCpuFreqPolicy;
        }
    }
}

bool HalPower::setCpuFreqPolicy(Solid::Control::PowerManager::CpuFreqPolicy newPolicy)
{
    QString governor;

    switch(newPolicy)
    {
    case Solid::Control::PowerManager::OnDemand:
        governor = "ondemand";
        break;
    case Solid::Control::PowerManager::Userspace:
        governor = "userspace";
        break;
    case Solid::Control::PowerManager::Powersave:
        governor = "powersave";
        break;
    case Solid::Control::PowerManager::Performance:
        governor = "performance";
        break;
    default:
        return false;
    }

    QDBusReply<int> reply = m_halCpuFreq.call("SetCPUFreqGovernor", governor);

    if (reply.isValid())
    {
        int code = reply;
        return code==0;
    }
    else
    {
        return false;
    }
}

bool HalPower::canDisableCpu(int /*cpuNum */) const
{
    return false;
}

bool HalPower::setCpuEnabled(int /*cpuNum */, bool /*enabled */)
{
    return false;
}

void HalPower::computeAcAdapters()
{
    Solid::DeviceList adapters
        = Solid::Device::listFromType(Solid::DeviceInterface::AcAdapter);

    foreach (Solid::Device adapter, adapters)
    {
        m_acAdapters[adapter.udi()] = new Solid::Device(adapter);
        connect(m_acAdapters[adapter.udi()]->as<Solid::AcAdapter>(), SIGNAL(plugStateChanged(bool)),
                 this, SLOT(slotPlugStateChanged(bool)));

        if (m_acAdapters[adapter.udi()]->as<Solid::AcAdapter>()!=0
          && m_acAdapters[adapter.udi()]->as<Solid::AcAdapter>()->isPlugged())
        {
            m_pluggedAdapterCount++;
        }
    }
}

void HalPower::computeBatteries()
{
    QString predicate = "Battery.type == %1";

    predicate = predicate.arg((int)Solid::Battery::PrimaryBattery);

    Solid::DeviceList batteries
        = Solid::Device::listFromType(Solid::DeviceInterface::Battery,
                                                     predicate);

    foreach (Solid::Device battery, batteries)
    {
        m_batteries[battery.udi()] = new Solid::Device(battery);
        connect(m_batteries[battery.udi()]->as<Solid::Battery>(), SIGNAL(chargePercentChanged(int)),
                 this, SLOT(updateBatteryStats()));
    }

    updateBatteryStats();
}

void HalPower::computeButtons()
{
    Solid::DeviceList buttons
        = Solid::Device::listFromType(Solid::DeviceInterface::Button);

    foreach (Solid::Device button, buttons)
    {
        m_buttons[button.udi()] = new Solid::Device(button);
        connect(m_buttons[button.udi()]->as<Solid::Button>(), SIGNAL(pressed(int)),
                 this, SLOT(slotButtonPressed(int)));
    }
}

void HalPower::updateBatteryStats()
{
    m_currentBatteryCharge = 0;
    m_maxBatteryCharge = 0;
    m_warningBatteryCharge = 0;
    m_lowBatteryCharge = 0;
    m_criticalBatteryCharge = 0;

    foreach (Solid::Device *d, m_batteries.values())
    {
        Solid::Battery *battery = d->as<Solid::Battery>();

        if (battery == 0) continue;

        m_currentBatteryCharge+= battery->chargeValue(Solid::Battery::CurrentLevel);
        m_maxBatteryCharge+= battery->chargeValue(Solid::Battery::LastFullLevel);
        m_warningBatteryCharge+= battery->chargeValue(Solid::Battery::WarningLevel);
        m_lowBatteryCharge+= battery->chargeValue(Solid::Battery::LowLevel);
    }

    m_criticalBatteryCharge = m_lowBatteryCharge/2;
}

void HalPower::slotPlugStateChanged(bool newState)
{
    if (newState)
    {
        m_pluggedAdapterCount++;
    }
    else
    {
        m_pluggedAdapterCount--;
    }
}

void HalPower::slotButtonPressed(int type)
{
    Solid::Button *button = qobject_cast<Solid::Button *>(sender());

    if (button == 0) return;

    switch(type)
    {
    case Solid::Button::PowerButton:
        emit buttonPressed(Solid::Control::PowerManager::PowerButton);
        break;
    case Solid::Button::SleepButton:
        emit buttonPressed(Solid::Control::PowerManager::SleepButton);
        break;
    case Solid::Button::LidButton:
        if (button->stateValue())
        {
            emit buttonPressed(Solid::Control::PowerManager::LidClose);
        }
        else
        {
            emit buttonPressed(Solid::Control::PowerManager::LidOpen);
        }
        break;
    default:
        kWarning() << "Unknown button type" << endl;
        break;
    }
}

void HalPower::slotNewDeviceInterface(const QString &udi, int type)
{
    switch (type)
    {
    case Solid::DeviceInterface::AcAdapter:
        m_acAdapters[udi] = new Solid::Device(udi);
        connect(m_acAdapters[udi]->as<Solid::AcAdapter>(), SIGNAL(plugStateChanged(bool)),
                 this, SLOT(slotPlugStateChanged(bool)));

        if (m_acAdapters[udi]->as<Solid::AcAdapter>()!=0
          && m_acAdapters[udi]->as<Solid::AcAdapter>()->isPlugged())
        {
            m_pluggedAdapterCount++;
        }
        break;
    case Solid::DeviceInterface::Battery:
        m_batteries[udi] = new Solid::Device(udi);
        connect(m_batteries[udi]->as<Solid::Battery>(), SIGNAL(chargePercentChanged(int)),
                 this, SLOT(updateBatteryStats()));
        break;
    case Solid::DeviceInterface::Button:
        m_buttons[udi] = new Solid::Device(udi);
        connect(m_buttons[udi]->as<Solid::Button>(), SIGNAL(pressed(int)),
                 this, SLOT(slotButtonPressed(int)));
        break;
    default:
        break;
    }
}

void HalPower::slotDeviceRemoved(const QString &udi)
{
    Solid::Device *device = 0;

    device = m_acAdapters.take(udi);

    if (device!=0)
    {
        delete device;

        m_pluggedAdapterCount = 0;

        foreach (Solid::Device *d, m_acAdapters.values())
        {
            if (d->as<Solid::AcAdapter>()!=0
              && d->as<Solid::AcAdapter>()->isPlugged())
            {
                m_pluggedAdapterCount++;
            }
        }

        return;
    }

    device = m_batteries.take(udi);

    if (device!=0)
    {
        delete device;
        updateBatteryStats();
        return;
    }

    device = m_buttons.take(udi);

    if (device!=0)
    {
        delete device;
        return;
    }
}

#include "halpower.moc"
