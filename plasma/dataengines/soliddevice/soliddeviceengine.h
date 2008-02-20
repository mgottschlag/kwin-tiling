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

#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/predicate.h>

#include "plasma/dataengine.h"
#include "devicesignalmapmanager.h"
#include "devicesignalmapper.h"

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
    bool sourceRequested(const QString &name);
    bool updateSource(const QString& source);

private:
    bool populateDeviceData(const QString &name);
    qlonglong freeDiskSpace(const QString &mountPoint);
    bool updateFreeSpace(const QString &udi);
    void listenForNewDevices();

    QStringList devicelist;

    //predicate in string form, list of devices by udi
    QMap<QString, QStringList> predicatemap;
    //udi, corresponding device
    QMap<QString, Solid::Device> devicemap;
    DeviceSignalMapManager *signalmanager;

    Solid::DeviceNotifier *notifier;

private Q_SLOTS:
    void deviceAdded(const QString &udi);
    void deviceRemoved(const QString &udi);
    void deviceChanged(const QString& udi, const QString &property, const QVariant &value);
};

K_EXPORT_PLASMA_DATAENGINE(soliddevice, SolidDeviceEngine)

#endif
