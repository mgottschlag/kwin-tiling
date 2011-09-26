/*  This file is part of the KDE project
    Copyright (C) 2010 Lamarque Souza <lamarque@gmail.com>

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

#include <KLocale>

#include "modeminterface.h"
#include "modeminterface_p.h"

#include "soliddefs_p.h"
#include "ifaces/modeminterface.h"

Solid::Control::ModemInterface::ModemInterface(QObject *backendObject)
    : QObject(), d_ptr(new ModemInterfacePrivate(this))
{
    Q_D(ModemInterface); d->setBackendObject(backendObject);
}

Solid::Control::ModemInterface::ModemInterface(const ModemInterface &other)
    : QObject(), d_ptr(new ModemInterfacePrivate(this))
{
    Q_D(ModemInterface);
    d->setBackendObject(other.d_ptr->backendObject());
}

Solid::Control::ModemInterface::ModemInterface(ModemInterfacePrivate &dd, QObject *backendObject)
    : QObject(), d_ptr(&dd)
{
    Q_UNUSED(backendObject);
}

Solid::Control::ModemInterface::ModemInterface(ModemInterfacePrivate &dd, const ModemInterface &other)
    : d_ptr(&dd)
{
    Q_UNUSED(other);
}

Solid::Control::ModemInterface::~ModemInterface()
{
    delete d_ptr;
}

QString Solid::Control::ModemInterface::udi() const
{
    Q_D(const ModemInterface);
    return_SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), QString(), udi());
}

void Solid::Control::ModemInterface::enable(const bool enable) const
{
    Q_D(const ModemInterface);
    SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), enable(enable));
}

void Solid::Control::ModemInterface::connectModem(const QString & number) const
{
    Q_D(const ModemInterface);
    SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), connectModem(number));
}

void Solid::Control::ModemInterface::connectModem(const QVariantMap & properties) const
{
    Q_D(const ModemInterface);
    SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), connectModem(properties));
}

void Solid::Control::ModemInterface::disconnectModem() const
{
    Q_D(const ModemInterface);
    SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), disconnectModem());
}

Solid::Control::ModemInterface::Ip4ConfigType Solid::Control::ModemInterface::getIp4Config() const
{
    Q_D(const ModemInterface);
    return_SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), Ip4ConfigType(), getIp4Config());
}

Solid::Control::ModemInterface::InfoType Solid::Control::ModemInterface::getInfo() const
{
    Q_D(const ModemInterface);
    return_SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), InfoType(), getInfo());
}

QVariantMap Solid::Control::ModemInterface::getStatus() const
{
    Q_D(const ModemInterface);
    return_SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), QVariantMap(), getStatus());
}

QString Solid::Control::ModemInterface::device() const
{
    Q_D(const ModemInterface);
    return_SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), QString(), device());
}

QString Solid::Control::ModemInterface::masterDevice() const
{
    Q_D(const ModemInterface);
    return_SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), QString(), masterDevice());
}

QString Solid::Control::ModemInterface::driver() const
{
    Q_D(const ModemInterface);
    return_SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), QString(), driver());
}

Solid::Control::ModemInterface::Type Solid::Control::ModemInterface::type() const
{
    Q_D(const ModemInterface);
    return_SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), Solid::Control::ModemInterface::UnknownType, type());
}

bool Solid::Control::ModemInterface::enabled() const
{
    Q_D(const ModemInterface);
    return_SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), false, enabled());
}

QString Solid::Control::ModemInterface::unlockRequired() const
{
    Q_D(const ModemInterface);
    return_SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), QString(), unlockRequired());
}

Solid::Control::ModemInterface::Method Solid::Control::ModemInterface::ipMethod() const
{
    Q_D(const ModemInterface);
    return_SOLID_CALL(Ifaces::ModemInterface *, d->backendObject(), UnknownMethod, ipMethod());
}

void Solid::Control::ModemInterface::slotDeviceChanged(const QString & device)
{
    emit deviceChanged(device);
}

void Solid::Control::ModemInterface::slotMasterDeviceChanged(const QString & masterDevice)
{
    emit masterDeviceChanged(masterDevice);
}

void Solid::Control::ModemInterface::slotDriverChanged(const QString & driver)
{
    emit driverChanged(driver);
}

void Solid::Control::ModemInterface::slotUnlockRequiredChanged(const QString & codeRequired)
{
    emit unlockRequiredChanged(codeRequired);
}

void Solid::Control::ModemInterfacePrivate::setBackendObject(QObject *object)
{
    FrontendObjectPrivate::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(deviceChanged(QString)),
                         parent(), SLOT(slotDeviceChanged(QString)));
        QObject::connect(object, SIGNAL(masterDeviceChanged(QString)),
                         parent(), SLOT(slotMasterDeviceChanged(QString)));
        QObject::connect(object, SIGNAL(driverChanged(QString)),
                         parent(), SLOT(slotDriverChanged(QString)));
        QObject::connect(object, SIGNAL(typeChanged(Solid::Control::ModemInterface::Type)),
                         parent(), SIGNAL(typeChanged(Solid::Control::ModemInterface::Type)));
        QObject::connect(object, SIGNAL(enabledChanged(bool)),
                         parent(), SIGNAL(enabledChanged(bool)));
        QObject::connect(object, SIGNAL(unlockRequiredChanged(QString)),
                         parent(), SLOT(slotUnlockRequiredChanged(QString)));
        QObject::connect(object, SIGNAL(ipMethodChanged(Solid::Control::ModemInterface::Method)),
                         parent(), SIGNAL(ipMethodChanged(Solid::Control::ModemInterface::Method)));
    }
}

QString Solid::Control::ModemInterface::convertTypeToString(const Solid::Control::ModemInterface::Type type)
{
    switch (type) {
        case UnknownType: return i18nc("Unknown cellular type","Unknown");
        case GsmType: return i18nc("Gsm cellular type","Gsm");
        case CdmaType: return i18nc("Cdma cellular type","Cdma");
    }

    return i18nc("Unknown cellular type","Unknown");
}

QString Solid::Control::ModemInterface::convertBandToString(const Solid::Control::ModemInterface::Band band)
{
    switch (band) {
        case UnknownBand: return i18nc("Unknown cellular frequency band","Unknown");
        case AnyBand: return i18nc("Any cellular frequency band","Any");
        case Egsm: return i18nc("Cellular frequency band","GSM/GPRS/EDGE 900 MHz");
        case Dcs: return i18nc("Cellular frequency band","GSM/GPRS/EDGE 1800 MHz");
        case Pcs: return i18nc("Cellular frequency band","GSM/GPRS/EDGE 1900 MHz");
        case G850: return i18nc("Cellular frequency band","GSM/GPRS/EDGE 850 MHz");
        case U2100: return i18nc("Cellular frequency band","WCDMA 2100 MHz (Class I)");
        case U1800: return i18nc("Cellular frequency band","WCDMA 3GPP 1800 MHz (Class III)");
        case U17IV: return i18nc("Cellular frequency band","WCDMA 3GPP AWS 1700/2100 MHz (Class IV)");
        case U800: return i18nc("Cellular frequency band","WCDMA 3GPP UMTS 800 MHz (Class VI)");
        case U850: return i18nc("Cellular frequency band","WCDMA 3GPP UMTS 850 MHz (Class V)");
        case U900: return i18nc("Cellular frequency band","WCDMA 3GPP UMTS 900 MHz (Class VIII)");
        case U17IX: return i18nc("Cellular frequency band","WCDMA 3GPP UMTS 1700 MHz (Class IX)");
        case U19IX: return i18nc("Cellular frequency band","WCDMA 3GPP UMTS 1900 MHz (Class II)");
    }

    return i18nc("Unknown cellular frequency band","Unknown");
}

QString Solid::Control::ModemInterface::convertAllowedModeToString(const Solid::Control::ModemInterface::AllowedMode mode)
{
    switch (mode) {
        case AnyModeAllowed: return i18nc("Allowed Gsm modes (2G/3G/any)","Any");
        case Prefer2g: return i18nc("Allowed Gsm modes (2G/3G/any)","Prefer 2G");
        case Prefer3g: return i18nc("Allowed Gsm modes (2G/3G/any)","Prefer 3G");
        case UseOnly2g: return i18nc("Allowed Gsm modes (2G/3G/any)","Only 2G");
        case UseOnly3g: return i18nc("Allowed Gsm modes (2G/3G/any)","Only 3G");
    }

    return i18nc("Allowed Gsm modes (2G/3G/any)","Any");
}

QString Solid::Control::ModemInterface::convertAccessTechnologyToString(const Solid::Control::ModemInterface::AccessTechnology tech)
{
    switch (tech) {
        case UnknownTechnology: return i18nc("Unknown cellular access technology","Unknown");
        case Gsm: return i18nc("Cellular access technology","GSM");
        case GsmCompact: return i18nc("Cellular access technology","Compact GSM");
        case Gprs: return i18nc("Cellular access technology","GPRS");
        case Edge: return i18nc("Cellular access technology","EDGE");
        case Umts: return i18nc("Cellular access technology","UMTS");
        case Hsdpa: return i18nc("Cellular access technology","HSDPA");
        case Hsupa: return i18nc("Cellular access technology","HSUPA");
        case Hspa: return i18nc("Cellular access technology","HSPA");
    }

    return i18nc("Unknown cellular access technology","Unknown");
}

#include "modeminterface.moc"
