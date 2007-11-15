/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "core/systemmodel.h"

// Qt
#include <QFile>
#include <QHash>
#include <QtDebug>

// KDE
#include <KDiskFreeSpace>
#include <KLocalizedString>
#include <KIcon>
#include <KGlobal>
#include <KUrl>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>

// Local
#include "core/models.h"


using namespace Kickoff;

class SystemModel::Private
{
public:
    Private(SystemModel *parent)
        :q(parent)
        ,removableStorageItem(0)
        ,fixedStorageItem(0)
    {
    }

    QStandardItem *lookupDeviceByMountPoint(const QString& mountPoint)
    {
        QString mountUrl = KUrl(mountPoint).url();
        foreach(QStandardItem *item,deviceItemById) {
            if (item->data(UrlRole).value<QString>() == mountUrl) {
                return item;
            }
        }
        return 0;
    }
    void addDevice(const Solid::Device& device) 
    {
            const Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
            if (!access) {
                return;
            }

            QStandardItem *deviceItem = StandardItemFactory::createItemForDevice(device);
            if (!deviceItem) {
               return; 
            }

            // start a request to find the available free disk space
            // FIXME: On Unix this is not very efficient as KDiskFreeSpace starts a 'df' process
            // for each drive
            queryFreeSpace(access->filePath());

            Solid::StorageDrive *drive = 0;
            Solid::Device parentDevice = device;
            while (parentDevice.isValid() && !drive) { 
                drive = parentDevice.as<Solid::StorageDrive>();
                parentDevice = parentDevice.parent();
            }

            if (drive && (drive->isHotpluggable() || drive->isRemovable())) { 
                removableStorageItem->appendRow(deviceItem);
            } else {
                fixedStorageItem->appendRow(deviceItem);
            }
            deviceItemById.insert(device.udi(),deviceItem);
    }

    void loadStorageItems()
    {
        // get device list
        QList<Solid::Device> deviceList = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess,QString());

        // add items
        removableStorageItem = new QStandardItem(i18n("Removable Storage"));
        fixedStorageItem = new QStandardItem(i18n("Storage"));

        foreach(const Solid::Device& device,deviceList) {
            addDevice(device);
        }

        q->appendRow(removableStorageItem);
        q->appendRow(fixedStorageItem);
    }

    void loadPlaces()
    {
        QStandardItem *placesItem = new QStandardItem(i18n("Places"));

        QStandardItem *homeItem = StandardItemFactory::createItemForUrl(getenv("HOME"));
        placesItem->appendRow(homeItem);

        QStandardItem *networkItem = StandardItemFactory::createItemForUrl("remote:/");
        placesItem->appendRow(networkItem); 

        KService::Ptr settingsService = KService::serviceByStorageId("kde4-systemsettings.desktop");
        if (settingsService) {
            placesItem->appendRow(StandardItemFactory::createItemForService(settingsService));
        }

        q->appendRow(placesItem);
    }

    void queryFreeSpace(const QString& mountPoint)
    {
        KDiskFreeSpace *freeSpace = KDiskFreeSpace::findUsageInfo(mountPoint);
        connect(freeSpace,SIGNAL(foundMountPoint(QString,quint64,quint64,quint64)),q,
            SLOT(freeSpaceInfoAvailable(QString,quint64,quint64,quint64)));
    }

    SystemModel * const q;
    QStandardItem *removableStorageItem;
    QStandardItem *fixedStorageItem;
    QHash<QString,QStandardItem*> deviceItemById;
};

SystemModel::SystemModel(QObject *parent)
    : QStandardItemModel(parent)
    , d(new Private(this))
{
    d->loadPlaces();
    d->loadStorageItems();

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)), this, SLOT(deviceAdded(QString)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)), this, SLOT(deviceRemoved(QString)));
}

SystemModel::~SystemModel()
{
    delete d;
}

void SystemModel::deviceAdded(const QString& udi)
{
    qDebug() << "SystemModel adding device" << udi;
    d->addDevice(Solid::Device(udi));
}

void SystemModel::deviceRemoved(const QString& udi)
{
    QStandardItem *deviceItem = d->deviceItemById[udi];
    if(deviceItem) {
        Q_ASSERT(deviceItem->parent());
        deviceItem->parent()->removeRow(deviceItem->row());
        d->deviceItemById.remove(udi);
    }
}

void SystemModel::registerDevicePaths()
{
    QList<Solid::Device> deviceList = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess,QString());
    foreach(Solid::Device device,deviceList) {
        Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
        if (access) {
            StandardItemFactory::associateDevice(KUrl(access->filePath()).url(),device);
        }
    }
}

void SystemModel::freeSpaceInfoAvailable(const QString& mountPoint,quint64,quint64 kbUsed,quint64 kbAvailable)
{
    QStandardItem *deviceItem = d->lookupDeviceByMountPoint(mountPoint);
    if (deviceItem) {
        deviceItem->setData(kbUsed,DiskUsedSpaceRole);
        deviceItem->setData(kbAvailable,DiskFreeSpaceRole);
    }
}


#include "systemmodel.moc"
