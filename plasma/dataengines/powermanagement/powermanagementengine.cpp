/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2007 Sebastian Kuegler <sebas@kde.org>
 *   CopyRight (C) 2007 Maor Vanmak <mvanmak1@gmail.com>
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

#include "plasma/datacontainer.h"

PowermanagementEngine::PowermanagementEngine(QObject* parent, const QVariantList& args)
        : Plasma::DataEngine(parent, args)
        , m_acadapter(0)
        , m_sources(0)
{
    Q_UNUSED(args)
        
    m_sources << I18N_NOOP("Battery") << I18N_NOOP("AC Adapter") << I18N_NOOP("Sleepstates");
    
    // This following call can be removed, but if we do, the
    // data is not shown in the plasmaengineexplorer.
    // sourceRequestEvent("Battery");
}

PowermanagementEngine::~PowermanagementEngine()
{}

void PowermanagementEngine::init()
{
}

QStringList PowermanagementEngine::sources() const 
{
    return m_sources;
}

bool PowermanagementEngine::sourceRequestEvent(const QString &name)
{
    if (name == I18N_NOOP("Battery")) {
        QList<Solid::Device> list_battery =
                        Solid::Device::listFromType(Solid::DeviceInterface::Battery, QString());
        if (list_battery.count() == 0) {
            setData(I18N_NOOP("Battery"), I18N_NOOP("has Battery"), false);
            return true;
        }
        
        uint index = 0;
        QStringList battery_sources;
        
        foreach (const Solid::Device &device_battery, list_battery) {
            const Solid::Battery* battery = device_battery.as<Solid::Battery>();

            if(battery != 0) {
                QString source = QString(I18N_NOOP("Battery%1")).arg(index++);

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
        
        if(battery_sources.count() > 0) {
            setData(I18N_NOOP("Battery"), I18N_NOOP("has Battery"), true);
            setData(I18N_NOOP("Battery"), I18N_NOOP("sources"), battery_sources);
        }
    } else if (name == I18N_NOOP("AC Adapter")) {
        // AC Adapter handling
        QList<Solid::Device> list_ac =
                        Solid::Device::listFromType(Solid::DeviceInterface::AcAdapter, QString());
        foreach (Solid::Device device_ac, list_ac) {
            m_acadapter = device_ac.as<Solid::AcAdapter>();
            updateAcPlugState(m_acadapter->isPlugged());
            connect(m_acadapter, SIGNAL(plugStateChanged(bool, const QString &)), this,
                    SLOT(updateAcPlugState(bool)));
        }
    } else if (name == I18N_NOOP("Sleepstates")) {
        QSet<Solid::PowerManagement::SleepState> sleepstates =
                                Solid::PowerManagement::supportedSleepStates();
        // We first set all possible sleepstates to false, then enable the ones that are available
        setData(I18N_NOOP("Sleepstates"), I18N_NOOP("Standby"), false);
        setData(I18N_NOOP("Sleepstates"), I18N_NOOP("Suspend"), false);
        setData(I18N_NOOP("Sleepstates"), I18N_NOOP("Hibernate"), false);

        foreach (Solid::PowerManagement::SleepState sleepstate, sleepstates) {
            if (sleepstate == Solid::PowerManagement::StandbyState) {
                setData(I18N_NOOP("Sleepstates"), I18N_NOOP("Supports standby"), true);
            } else if (sleepstate == Solid::PowerManagement::SuspendState) {
                setData(I18N_NOOP("Sleepstates"), I18N_NOOP("Supports suspend"), true);
            } else if (sleepstate == Solid::PowerManagement::HibernateState) {
                setData(I18N_NOOP("Sleepstates"), I18N_NOOP("Supports hibernate"), true);
            }
            kDebug() << "Sleepstate \"" << sleepstate << "\" supported.";
        }
    } else {
        kDebug() << "Data for '" << name << "' not found";
    }
    return true;
}

void PowermanagementEngine::updateBatteryChargeState(int newState, const QString& udi)
{
    QString state;
    if (newState == Solid::Battery::NoCharge) {
        state = I18N_NOOP("NoCharge");
    } else if (newState == Solid::Battery::Charging) {
        state = I18N_NOOP("Charging");
    } else if (newState == Solid::Battery::Discharging) {
        state = I18N_NOOP("Discharging");
    } else {
        state = I18N_NOOP("Could not determine battery status. Something is fishy here. :o");
    }
    const QString& source = m_batterySources[udi];
    setData(source, I18N_NOOP("State"), state);
    scheduleSourcesUpdated();
}

void PowermanagementEngine::updateBatteryPlugState(bool newState, const QString& udi)
{
    const QString& source = m_batterySources[udi];
    setData(source, I18N_NOOP("Plugged in"), newState);
    scheduleSourcesUpdated();
}

void PowermanagementEngine::updateBatteryChargePercent(int newValue, const QString& udi)
{
    const QString& source = m_batterySources[udi];
    setData(source, I18N_NOOP("Percent"), newValue);
    scheduleSourcesUpdated();
}

void PowermanagementEngine::updateAcPlugState(bool newState)
{
    setData(I18N_NOOP("AC Adapter"), I18N_NOOP("Plugged in"), newState);
    scheduleSourcesUpdated();
}

#include "powermanagementengine.moc"
