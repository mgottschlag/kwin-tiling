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

#ifndef HALPOWER_H
#define HALPOWER_H

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>

#include <kdemacros.h>

#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/button.h>

#include <solid/control/ifaces/powermanager.h>

class KDE_EXPORT HalPower : public Solid::Control::Ifaces::PowerManager
{
    Q_OBJECT

public:
    HalPower(QObject *parent, const QStringList &args);
    virtual ~HalPower();

    virtual QStringList supportedSchemes() const;
    virtual QString schemeDescription(const QString &schemeName) const;
    virtual QString scheme() const;
    virtual bool setScheme(const QString &name);

    virtual Solid::Control::PowerManager::BatteryState batteryState() const;
    virtual int batteryChargePercent() const;
    virtual Solid::Control::PowerManager::AcAdapterState acAdapterState() const;

    virtual Solid::Control::PowerManager::SuspendMethods supportedSuspendMethods() const;
    virtual KJob *suspend(Solid::Control::PowerManager::SuspendMethod method) const;

    virtual Solid::Control::PowerManager::CpuFreqPolicies supportedCpuFreqPolicies() const;
    virtual Solid::Control::PowerManager::CpuFreqPolicy cpuFreqPolicy() const;
    virtual bool setCpuFreqPolicy(Solid::Control::PowerManager::CpuFreqPolicy newPolicy);
    virtual bool canDisableCpu(int cpuNum) const;
    virtual bool setCpuEnabled(int cpuNum, bool enabled);

    virtual Solid::Control::PowerManager::BrightnessControlsList brightnessControlsAvailable();
    virtual float brightness(const QString &device);
    virtual bool setBrightness(float brightness, const QString &device);

private:
    void computeAcAdapters();
    void computeBatteries();
    void computeButtons();

private slots:
    void updateBatteryStats();
    void slotPlugStateChanged(bool newState);
    void slotButtonPressed(Solid::Button::ButtonType type);
    void slotDeviceAdded(const QString &udi);
    void slotDeviceRemoved(const QString &udi);

private:
    QMap<QString, Solid::Device *> m_acAdapters;
    QMap<QString, Solid::Device *> m_batteries;
    QMap<QString, Solid::Device *> m_buttons;

    int m_pluggedAdapterCount;

    int m_currentBatteryCharge;
    int m_maxBatteryCharge;
    int m_warningBatteryCharge;
    int m_lowBatteryCharge;
    int m_criticalBatteryCharge;

    mutable QDBusInterface m_halComputer;
    mutable QDBusInterface m_halPowerManagement;
    mutable QDBusInterface m_halCpuFreq;
    mutable QDBusInterface m_halManager;
};

#endif
