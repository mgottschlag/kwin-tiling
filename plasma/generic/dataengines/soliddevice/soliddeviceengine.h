/*
 *   Copyright (C) 2007 Christopher Blauvelt <cblauvelt@gmail.com>
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

#ifndef SOLIDDEVICEENGINE_H
#define SOLIDDEVICEENGINE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QPair>

#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/predicate.h>

#include <Plasma/DataEngine>
#include "devicesignalmapmanager.h"
#include "devicesignalmapper.h"
#include "hddtemp.h"

/**
 * This class evaluates the basic expressions given in the interface.
 */
class SolidDeviceEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    SolidDeviceEngine( QObject* parent, const QVariantList& args);
    ~SolidDeviceEngine();

protected:
    bool sourceRequestEvent(const QString &name);
    bool updateSourceEvent(const QString& source);

private Q_SLOTS:
    void deviceAdded(const QString &udi);
    void deviceRemoved(const QString &udi);
    void deviceChanged(const QString& udi, const QString &property, const QVariant &value);
    void sourceWasRemoved(const QString &source);

private:
    bool populateDeviceData(const QString &name);
    qlonglong freeDiskSpace(const QString &mountPoint);
    bool updateFreeSpace(const QString &udi);
    bool updateHardDiskTemperature(const QString &udi);
    bool updateEmblems(const QString &udi);
    bool forceUpdateAccessibility(const QString &udi);
    void listenForNewDevices();

    //predicate in string form, list of devices by udi
    QMap<QString, QStringList> m_predicatemap;
    //udi, corresponding device
    QMap<QString, Solid::Device> m_devicemap;
    //udi, corresponding encrypted container udi;
    QMap<QString, QString> m_encryptedContainerMap;
    DeviceSignalMapManager *m_signalmanager;

    HddTemp *m_temperature;
    Solid::DeviceNotifier *m_notifier;
};

K_EXPORT_PLASMA_DATAENGINE(soliddevice, SolidDeviceEngine)

#endif
