/*
 *   Copyright (C) 2007 Menard Alexis <darktears31@gmail.com>
 *
 * This program is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "hotplugengine.h"

#include <QTimer>

#include <KDirWatch>
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>
#include <KStandardDirs>
#include <KDesktopFile>
#include <Plasma/DataContainer>

//solid specific includes
#include <Solid/DeviceNotifier>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/StorageDrive>
#include <Solid/StorageVolume>

//#define HOTPLUGENGINE_TIMING

HotplugEngine::HotplugEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args),
      m_dirWatch(new KDirWatch(this))
{
    QStringList folders = KGlobal::dirs()->findDirs("data", "solid/actions/");
    foreach (const QString &folder, folders) {
        m_dirWatch->addDir(folder, KDirWatch::WatchFiles);
    }
    connect(m_dirWatch, SIGNAL(dirty(QString)), this, SLOT(updatePredicates(QString)));
}

HotplugEngine::~HotplugEngine()
{

}

void HotplugEngine::init()
{
    findPredicates();

    Solid::Predicate p(Solid::DeviceInterface::StorageAccess);
    p |= Solid::Predicate(Solid::DeviceInterface::StorageDrive);
    p |= Solid::Predicate(Solid::DeviceInterface::StorageVolume);
    p |= Solid::Predicate(Solid::DeviceInterface::OpticalDrive);
    p |= Solid::Predicate(Solid::DeviceInterface::PortableMediaPlayer);
    p |= Solid::Predicate(Solid::DeviceInterface::SmartCardReader);
    p |= Solid::Predicate(Solid::DeviceInterface::Camera);
    QList<Solid::Device> devices = Solid::Device::listFromQuery(p);
    foreach (const Solid::Device &dev, devices) {
        m_startList.insert(dev.udi(), dev);
    }

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
            this, SLOT(onDeviceAdded(QString)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
            this, SLOT(onDeviceRemoved(QString)));

    m_encryptedPredicate = Solid::Predicate("StorageVolume", "usage", "Encrypted");

    processNextStartupDevice();
}

void HotplugEngine::processNextStartupDevice()
{
    if (!m_startList.isEmpty()) {
        QHash<QString, Solid::Device>::iterator it = m_startList.begin();
        //Solid::Device dev = const_cast<Solid::Device &>(m_startList.takeFirst());
        onDeviceAdded(it.value(), false);
        m_startList.erase(it);
    }

    if (m_startList.isEmpty()) {
        m_predicates.clear();
    } else {
        QTimer::singleShot(0, this, SLOT(processNextStartupDevice()));
    }
}

void HotplugEngine::findPredicates()
{
    m_predicates.clear();

    foreach (const QString &path, KGlobal::dirs()->findAllResources("data", "solid/actions/")) {
        KDesktopFile cfg(path);
        const QString string_predicate = cfg.desktopGroup().readEntry("X-KDE-Solid-Predicate");
        //kDebug() << path << string_predicate;
        m_predicates.insert(KUrl(path).fileName(), Solid::Predicate::fromString(string_predicate));
    }

    if (m_predicates.isEmpty()) {
        m_predicates.insert(QString(), Solid::Predicate::fromString(QString()));
    }
}

void HotplugEngine::updatePredicates(const QString &path)
{
    Q_UNUSED(path)

    findPredicates();

    QHashIterator<QString, Solid::Device> it(m_devices);
    while (it.hasNext()) {
        it.next();
        Solid::Device device(it.value());
        QString udi(it.key());

        const QStringList predicates = predicatesForDevice(device);
        if (!predicates.isEmpty()) {
            if (sources().contains(udi)) {
                Plasma::DataEngine::Data data;
                data.insert("predicateFiles", predicates);
                setData(udi, data);
            } else {
                onDeviceAdded(device, false);
            }
        } else if (!m_encryptedPredicate.matches(device) && sources().contains(udi)) {
            removeSource(udi);
            scheduleSourcesUpdated();
        }
    }
}

QStringList HotplugEngine::predicatesForDevice(Solid::Device &device) const
{
    QStringList interestingDesktopFiles;
    //search in all desktop configuration file if the device inserted is a correct device
    QHashIterator<QString, Solid::Predicate> it(m_predicates);
    //kDebug() << "=================" << udi;
    while (it.hasNext()) {
        it.next();
        if (it.value().matches(device)) {
            //kDebug() << "     hit" << it.key();
            interestingDesktopFiles << it.key();
        }
    }

    return interestingDesktopFiles;
}

void HotplugEngine::onDeviceAdded(const QString &udi)
{
    Solid::Device device(udi);
    onDeviceAdded(device);
}

void HotplugEngine::onDeviceAdded(Solid::Device &device, bool added)
{
    //kDebug() << "adding" << device.udi();
#ifdef HOTPLUGENGINE_TIMING
    QTime t;
    t.start();
#endif
    // Skip things we know we don't care about
    if (device.isDeviceInterface(Solid::DeviceInterface::StorageDrive)) {
        Solid::DeviceInterface *dev = device.asDeviceInterface(Solid::DeviceInterface::StorageDrive);
        Solid::StorageDrive *drive = static_cast<Solid::StorageDrive *>(dev);
        if (!drive->isHotpluggable()) {
#ifdef HOTPLUGENGINE_TIMING
            kDebug() << "storage, but not pluggable, returning" << t.restart();
#endif
            return;
        }
    } else if (device.isDeviceInterface(Solid::DeviceInterface::StorageVolume)) {
        Solid::DeviceInterface *dev = device.asDeviceInterface(Solid::DeviceInterface::StorageVolume);
        Solid::StorageVolume *volume = static_cast<Solid::StorageVolume *>(dev);
        Solid::StorageVolume::UsageType type = volume->usage();
        if (type == Solid::StorageVolume::Unused ||
            type == Solid::StorageVolume::PartitionTable) {
#ifdef HOTPLUGENGINE_TIMING
            kDebug() << "storage volume, but not of interest" << t.restart();
#endif
            return;
        }
    }

    m_devices.insert(device.udi(), device);

    if (m_predicates.isEmpty()) {
        findPredicates();
    }

    const QStringList interestingDesktopFiles = predicatesForDevice(device);
    const bool isEncryptedContainer = m_encryptedPredicate.matches(device);

    if (!interestingDesktopFiles.isEmpty() || isEncryptedContainer) {
        //kDebug() << device.product();
        //kDebug() << device.vendor();
        //kDebug() << "number of interesting desktop file : " << interestingDesktopFiles.size();
        Plasma::DataEngine::Data data;
        data.insert("added", added);
        data.insert("udi", device.udi());

        if (!device.description().isEmpty()) {
            data.insert("text", device.description());
        } else {
            data.insert("text", QString(device.vendor() + QLatin1Char(' ') + device.product()));
        }
        data.insert("icon", device.icon());
        data.insert("emblems", device.emblems());
        data.insert("predicateFiles", interestingDesktopFiles);
        data.insert("isEncryptedContainer", isEncryptedContainer);

        setData(device.udi(), data);
        //kDebug() << "add hardware solid : " << udi;
    }

#ifdef HOTPLUGENGINE_TIMING
    kDebug() << "total time" << t.restart();
#endif
}

void HotplugEngine::onDeviceRemoved(const QString &udi)
{
    //kDebug() << "remove hardware:" << udi;

    if (m_startList.contains(udi)) {
        m_startList.remove(udi);
        return;
    }

    m_devices.remove(udi);
    removeSource(udi);
    scheduleSourcesUpdated();
}

#include "hotplugengine.moc"
