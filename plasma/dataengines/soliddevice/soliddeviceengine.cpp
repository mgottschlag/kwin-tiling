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

#include "config-workspace.h"

#include "soliddeviceengine.h"

#include <KDebug>
#include <KLocale>

#include "plasma/datacontainer.h"

// The pattern here is:
//	FreeBSD: param + mount
//	Linux: stat + vfs
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif

#include <sys/stat.h>
#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif

#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif

SolidDeviceEngine::SolidDeviceEngine(QObject* parent, const QVariantList& args)
        : Plasma::DataEngine(parent, args),
          notifier(0)
{
    Q_UNUSED(args)
    signalmanager = new DeviceSignalMapManager(this);
}

SolidDeviceEngine::~SolidDeviceEngine()
{
    disconnect(notifier, SIGNAL(deviceAdded(const QString&)),
            this, SLOT(deviceAdded(const QString&)));
    disconnect(notifier, SIGNAL(deviceRemoved(const QString&)),
            this, SLOT(deviceRemoved(const QString&)));
    delete signalmanager;
}

void SolidDeviceEngine::listenForNewDevices()
{
    if(notifier) {
        return;
    }
    //detect when new devices are added
    notifier = Solid::DeviceNotifier::instance();
    connect(notifier, SIGNAL(deviceAdded(const QString&)),
            this, SLOT(deviceAdded(const QString&)));
    connect(notifier, SIGNAL(deviceRemoved(const QString&)),
            this, SLOT(deviceRemoved(const QString&)));
}

bool SolidDeviceEngine::sourceRequested(const QString &name)
{

    //create a predicate to check for validity
    Solid::Predicate predicate = Solid::Predicate::fromString(name);
    if(predicate.isValid()  && !predicatemap.contains(name)) {
        foreach (Solid::Device device, Solid::Device::listFromQuery(predicate)) {
            predicatemap[name] << device.udi();
            if (!devicemap.contains(device.udi())) {
                devicemap[device.udi()] = device;
            }
        }
        setData(name, predicatemap[name]);
        listenForNewDevices();
        return true;
    } else if (devicemap.contains(name) ) {
            return populateDeviceData(name);
    } else {
            return false;
    }
}

bool SolidDeviceEngine::populateDeviceData(const QString &name)
{
    Solid::Device device = devicemap[name];
    if (!device.isValid()) {
        return false;
    }

    QStringList devicetypes;
    setData(name, I18N_NOOP("Parent UDI"), device.parentUdi());
    setData(name, I18N_NOOP("Vendor"), device.vendor());
    setData(name, I18N_NOOP("Product"), device.product());
    setData(name, I18N_NOOP("Icon"), device.icon());

    if (device.is<Solid::Processor>()) {
        Solid::Processor *processor = device.as<Solid::Processor>();
        if (processor == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Processor");
        setData(name, I18N_NOOP("Number"), processor->number());
        setData(name, I18N_NOOP("Max Speed"), processor->maxSpeed());
        setData(name, I18N_NOOP("Can Change Frequency"), processor->canChangeFrequency());
    }
    if (device.is<Solid::Block>()) {
        Solid::Block *block = device.as<Solid::Block>();
        if (block == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Block");
        setData(name, I18N_NOOP("Major"), block->deviceMajor());
        setData(name, I18N_NOOP("Minor"), block->deviceMajor());
        setData(name, I18N_NOOP("Device"), block->device());
    }
    if (device.is<Solid::StorageAccess>()) {
        Solid::StorageAccess *storageaccess = device.as<Solid::StorageAccess>();
        if (storageaccess == 0) return false;

        devicetypes << I18N_NOOP("Storage Access");
        setData(name, I18N_NOOP("Accessible"), storageaccess->isAccessible());
        setData(name, I18N_NOOP("File Path"), storageaccess->filePath());
        QVariant freeDiskVar;
        qlonglong freeDisk = freeDiskSpace(storageaccess->filePath());
        if ( freeDisk != -1 ) {
            freeDiskVar.setValue( freeDisk );
        }
        setData(name, I18N_NOOP("Free Space"), freeDiskVar );
        //signalmanager->mapDevice(storageaccess, device.udi());
    }
    if (device.is<Solid::StorageDrive>()) {
        Solid::StorageDrive *storagedrive = device.as<Solid::StorageDrive>();
        if (storagedrive == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Storage Drive");

        QStringList bus;
        bus << I18N_NOOP("Ide") << I18N_NOOP("Usb") << I18N_NOOP("Ieee1394") << I18N_NOOP("Scsi") << I18N_NOOP("Sata") << I18N_NOOP("Platform");
        QStringList drivetype;
        drivetype << I18N_NOOP("Hard Disk") <<  I18N_NOOP("Cdrom Drive") <<  I18N_NOOP("Floppy") <<  I18N_NOOP("Tape") <<  I18N_NOOP("Compact Flash") <<  I18N_NOOP("Memory Stick") <<  I18N_NOOP("Smart Media") <<  I18N_NOOP("SdMmc") <<  I18N_NOOP("Xd");

        setData(name, I18N_NOOP("Bus"), bus.at((int)storagedrive->bus()));
        setData(name, I18N_NOOP("Drive Type"), drivetype.at((int)storagedrive->driveType()));
        setData(name, I18N_NOOP("Removable"), storagedrive->isRemovable());
        setData(name, I18N_NOOP("Hotpluggable"), storagedrive->isHotpluggable());
    }
    if (device.is<Solid::OpticalDrive>()) {
        Solid::OpticalDrive *opticaldrive = device.as<Solid::OpticalDrive>();
        if (opticaldrive == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Optical Drive");

        QStringList supportedtypes;
        Solid::OpticalDrive::MediumTypes mediatypes = opticaldrive->supportedMedia();
        if (mediatypes & Solid::OpticalDrive::Cdr) {
            supportedtypes << I18N_NOOP("Cdr");
        }
        if (mediatypes & Solid::OpticalDrive::Cdrw) {
            supportedtypes << I18N_NOOP("Cdrw");
        }
        if (mediatypes & Solid::OpticalDrive::Dvd) {
            supportedtypes << I18N_NOOP("Dvd");
        }
        if (mediatypes & Solid::OpticalDrive::Dvdr) {
            supportedtypes << I18N_NOOP("Dvdr");
        }
        if (mediatypes & Solid::OpticalDrive::Dvdrw) {
            supportedtypes << I18N_NOOP("Dvdrw");
        }
        if (mediatypes & Solid::OpticalDrive::Dvdram) {
            supportedtypes << I18N_NOOP("Dvdram");
        }
        if (mediatypes & Solid::OpticalDrive::Dvdplusr) {
            supportedtypes << I18N_NOOP("Dvdplusr");
        }
        if (mediatypes & Solid::OpticalDrive::Dvdplusrw) {
            supportedtypes << I18N_NOOP("Dvdplusrw");
        }
        if (mediatypes & Solid::OpticalDrive::Dvdplusdl) {
            supportedtypes << I18N_NOOP("Dvdplusdl");
        }
        if (mediatypes & Solid::OpticalDrive::Dvdplusdlrw) {
            supportedtypes << I18N_NOOP("Dvdplusdlrw");
        }
        if (mediatypes & Solid::OpticalDrive::Bd) {
            supportedtypes << I18N_NOOP("Bd");
        }
        if (mediatypes & Solid::OpticalDrive::Bdr) {
            supportedtypes << I18N_NOOP("Bdr");
        }
        if (mediatypes & Solid::OpticalDrive::Bdre) {
            supportedtypes << I18N_NOOP("Bdre");
        }
        if (mediatypes & Solid::OpticalDrive::HdDvd) {
            supportedtypes << I18N_NOOP("HdDvd");
        }
        if (mediatypes & Solid::OpticalDrive::HdDvdr) {
            supportedtypes << I18N_NOOP("HdDvdr");
        }
        if (mediatypes & Solid::OpticalDrive::HdDvdrw) {
            supportedtypes << I18N_NOOP("HdDvdrw");
        }
        setData(name, I18N_NOOP("Supported Media"), supportedtypes);

        setData(name, I18N_NOOP("Read Speed"), opticaldrive->readSpeed());
        setData(name, I18N_NOOP("Write Speed"), opticaldrive->writeSpeed());

        //the following method return QList<int> so we need to convert it to QList<QVariant>
        QList<int> writespeeds = opticaldrive->writeSpeeds();
        QList<QVariant> variantlist = QList<QVariant>();
        foreach(int num, writespeeds) {
            variantlist << QVariant(num);
        }
        setData(name, I18N_NOOP("Write Speeds"), variantlist);

    }
    if (device.is<Solid::StorageVolume>()) {
        Solid::StorageVolume *storagevolume = device.as<Solid::StorageVolume>();
        if (storagevolume == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Storage Volume");

        QStringList usagetypes;
        usagetypes << I18N_NOOP("File System") << I18N_NOOP("Partition Table") << I18N_NOOP("Raid") << I18N_NOOP("Other") << I18N_NOOP("Unused");

        setData(name, I18N_NOOP("Ignored"), storagevolume->isIgnored());
        setData(name, I18N_NOOP("Usage"), usagetypes.at((int)storagevolume->usage()));
        setData(name, I18N_NOOP("File System Type"), storagevolume->fsType());
        setData(name, I18N_NOOP("Label"), storagevolume->label());
        setData(name, I18N_NOOP("Uuid"), storagevolume->uuid());
        setData(name, I18N_NOOP("Size"), storagevolume->size());
    }
    if (device.is<Solid::OpticalDisc>()) {
        Solid::OpticalDisc *opticaldisc = device.as<Solid::OpticalDisc>();
        if (opticaldisc == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("OpticalDisc");

        //get the content types
        QStringList contenttypelist;
        Solid::OpticalDisc::ContentTypes contenttypes = opticaldisc->availableContent();
        if (contenttypes & Solid::OpticalDisc::Audio) {
            contenttypelist << I18N_NOOP("Audio");
        }
        if (contenttypes & Solid::OpticalDisc::Audio) {
            contenttypelist << I18N_NOOP("Data");
        }
        if (contenttypes & Solid::OpticalDisc::Audio) {
            contenttypelist << I18N_NOOP("Video Cd");
        }
        if (contenttypes & Solid::OpticalDisc::Audio) {
            contenttypelist << I18N_NOOP("Super Video Cd");
        }
        if (contenttypes & Solid::OpticalDisc::Audio) {
            contenttypelist << I18N_NOOP("Video Dvd");
        }
        setData(name, I18N_NOOP("Available Content"), contenttypelist);

        QStringList disctypes;
        disctypes << I18N_NOOP("Unknown Disc Type") << I18N_NOOP("CD Rom") << I18N_NOOP("CD Recordable")
                << I18N_NOOP("CD Rewritable") << I18N_NOOP("DVD Rom") << I18N_NOOP("DVD Ram")
                << I18N_NOOP("DVD Recordable") << I18N_NOOP("DVD Rewritable") << I18N_NOOP("DVD Plus Recordable")
                << I18N_NOOP("DVD Plus Rewritable") << I18N_NOOP("DVD Plus Recordable Duallayer")
                << I18N_NOOP("DVD Plus Rewritable Duallayer") << I18N_NOOP("Blu Ray Rom") << I18N_NOOP("Blu Ray Recordable")
                << I18N_NOOP("Blu Ray Rewritable") << I18N_NOOP("HD DVD Rom") <<  I18N_NOOP("HD DVD Recordable")
                << I18N_NOOP("HD DVD Rewritable");
        //+1 because the enum starts at -1
        setData(name, I18N_NOOP("Disc Type"), disctypes.at((int)opticaldisc->discType() + 1));
        setData(name, I18N_NOOP("Appendable"), opticaldisc->isAppendable());
        setData(name, I18N_NOOP("Blank"), opticaldisc->isBlank());
        setData(name, I18N_NOOP("Rewritable"), opticaldisc->isRewritable());
        setData(name, I18N_NOOP("Capacity"), opticaldisc->capacity());
    }
    if (device.is<Solid::Camera>()) {
        Solid::Camera *camera = device.as<Solid::Camera>();
        if (camera == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Camera");

        setData(name, I18N_NOOP("Supported Protocols"), camera->supportedProtocols());
        setData(name, I18N_NOOP("Supported Drivers"), camera->supportedDrivers());
    }
    if (device.is<Solid::PortableMediaPlayer>()) {
        Solid::PortableMediaPlayer *mediaplayer = device.as<Solid::PortableMediaPlayer>();
        if (mediaplayer == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Portable Media Player");

        setData(name, I18N_NOOP("Supported Protocols"), mediaplayer->supportedProtocols());
        setData(name, I18N_NOOP("Supported Drivers"), mediaplayer->supportedDrivers());
    }
    if (device.is<Solid::NetworkInterface>()) {
        Solid::NetworkInterface *networkinterface = device.as<Solid::NetworkInterface>();
        if (networkinterface == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Network Interface");

        setData(name, I18N_NOOP("Interface Name"), networkinterface->ifaceName());
        setData(name, I18N_NOOP("Wireless"), networkinterface->isWireless());
        setData(name, I18N_NOOP("Hardware Address"), networkinterface->hwAddress());
        setData(name, I18N_NOOP("Mac Address"), networkinterface->macAddress());
    }
    if (device.is<Solid::AcAdapter>()) {
        Solid::AcAdapter *ac = device.as<Solid::AcAdapter>();
        if (ac == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("AD Adapter");

        setData(name, I18N_NOOP("Plugged In"), ac->isPlugged());
        signalmanager->mapDevice(ac, device.udi());
    }
    if (device.is<Solid::Battery>()) {
        Solid::Battery *battery = device.as<Solid::Battery>();
        if (battery == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Battery");

        QStringList batterytype;
        batterytype << I18N_NOOP("Unknown Battery") << I18N_NOOP("PDA Battery") << I18N_NOOP("UPS Battery")
                << I18N_NOOP("Primary Battery") << I18N_NOOP("Mouse Battery") << I18N_NOOP("Keyboard Battery")
                << I18N_NOOP("Keyboard Mouse Battery") << I18N_NOOP("Camera Battery");

        QStringList chargestate;
        chargestate << I18N_NOOP("Fully Charged") << I18N_NOOP("Charging") << I18N_NOOP("Discharging");

        setData(name, I18N_NOOP("Plugged In"), battery->isPlugged());
        setData(name, I18N_NOOP("Type"), batterytype.at((int)battery->type()));
        setData(name, I18N_NOOP("Charge Percent"), battery->chargePercent());
        setData(name, I18N_NOOP("Rechargeable"), battery->isRechargeable());
        setData(name, I18N_NOOP("Charge State"), chargestate.at((int)battery->chargeState()));

        signalmanager->mapDevice(battery, device.udi());
    }
    if (device.is<Solid::Button>()) {
        Solid::Button *button = device.as<Solid::Button>();
        if (button == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Button");

        QStringList buttontype;
        buttontype << I18N_NOOP("Lid Button") << I18N_NOOP("Power Button") << I18N_NOOP("Sleep Button")
                << I18N_NOOP("Unknown Button Type");

        setData(name, I18N_NOOP("Type"), buttontype.at((int)button->type()));
        setData(name, I18N_NOOP("Has State"), button->hasState());
        setData(name, I18N_NOOP("State Value"), button->stateValue());
        setData(name, I18N_NOOP("Pressed"), false);  //this is an extra value that is tracked by the button signals

        signalmanager->mapDevice(button, device.udi());
    }
    if (device.is<Solid::AudioInterface>()) {
        Solid::AudioInterface *audiointerface = device.as<Solid::AudioInterface>();
        if (audiointerface == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Audio Interface");

        QStringList audiodriver;
        audiodriver << I18N_NOOP("ALSA") << I18N_NOOP("Open Sound System") << I18N_NOOP("Unknown Audio Driver");

        setData(name, I18N_NOOP("Driver"), audiodriver.at((int)audiointerface->driver()));
        setData(name, I18N_NOOP("Driver Handle"), audiointerface->driverHandle());
        setData(name, I18N_NOOP("Name"), audiointerface->name());

        //get AudioInterfaceTypes
        QStringList audiointerfacetypes;
        Solid::AudioInterface::AudioInterfaceTypes devicetypes = audiointerface->deviceType();
        if (devicetypes & Solid::AudioInterface::UnknownAudioInterfaceType) {
            audiointerfacetypes << I18N_NOOP("Unknown Audio Interface Type");
        }
        if (devicetypes & Solid::AudioInterface::AudioControl) {
            audiointerfacetypes << I18N_NOOP("Audio Control");
        }
        if (devicetypes & Solid::AudioInterface::AudioInput) {
            audiointerfacetypes << I18N_NOOP("Audio Input");
        }
        if (devicetypes & Solid::AudioInterface::AudioOutput) {
            audiointerfacetypes << I18N_NOOP("Audio Output");
        }
        setData(name, I18N_NOOP("Audio Device Type"), audiointerfacetypes);

        //get SoundCardTypes
        QStringList soundcardtype;
        soundcardtype << I18N_NOOP("Internal Soundcard") << I18N_NOOP("USB Soundcard") << I18N_NOOP("Firewire Soundcard")
                << I18N_NOOP("Headset") << I18N_NOOP("Modem");
        setData(name, I18N_NOOP("Soundcard Type"), soundcardtype.at((int)audiointerface->soundcardType()));
    }
    if (device.is<Solid::DvbInterface>()) {
        Solid::DvbInterface *dvbinterface = device.as<Solid::DvbInterface>();
        if (dvbinterface == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("DVB Interface");

        setData(name, I18N_NOOP("Device"), dvbinterface->device());
        setData(name, I18N_NOOP("Device Adapter"), dvbinterface->deviceAdapter());

        //get devicetypes
        QStringList dvbdevicetypes;
        dvbdevicetypes << I18N_NOOP("DVB Unknown") << I18N_NOOP("DVB Audio") << I18N_NOOP("DVB Ca")
                << I18N_NOOP("DVB Demux") << I18N_NOOP("DVB DVR") << I18N_NOOP("DVB Frontend")
                << I18N_NOOP("DVB Net") << I18N_NOOP("DVB OSD") << I18N_NOOP("DVB Sec") << I18N_NOOP("DVB Video");

        setData(name, I18N_NOOP("DVB Device Type"), dvbdevicetypes.at((int)dvbinterface->deviceType()));
        setData(name, I18N_NOOP("Device Index"), dvbinterface->deviceIndex());
    }
    setData(name, I18N_NOOP("Device Types"), devicetypes);
    return true;
}

void SolidDeviceEngine::deviceAdded(const QString& udi)
{
    Solid::Device device(udi);

    foreach (QString query, predicatemap.keys()) {
        Solid::Predicate predicate = Solid::Predicate::fromString(query);
        if (predicate.matches(device)) {
            predicatemap[query] << udi;
            setData(query, predicatemap[query]);
            if (!devicemap.contains(udi)) {
                devicemap[udi] = device;
            }
        }
    }

    checkForUpdates();
}

qlonglong SolidDeviceEngine::freeDiskSpace(const QString &mountPoint)
{
    //determine the free space available on the device
    const char *path=mountPoint.toAscii().constData();

#ifdef HAVE_STATVFS
    struct statvfs fs_obj;
    if (statvfs(path,&fs_obj) < 0) {
        return -1;
    } else {
        return (qlonglong)fs_obj.f_bfree*(qlonglong)fs_obj.f_frsize;
    }
#elif defined(HAVE_STATFS) && !defined(USE_SOLARIS)
    struct statfs fs_obj;
    if (statfs(path,&fs_obj) < 0){
        return -1;
    }
    else{
        return (qlonglong)fs_obj.f_bfree*(qlonglong)fs_obj.f_bsize;
    }
#else
#ifdef __GNUC__
#warning "This system does not support statfs or statvfs - freeDiskSpace() will return -1"
    return -1;
#endif
#endif
}

bool SolidDeviceEngine::updateFreeSpace(const QString &udi)
{
    Solid::Device device = devicemap[udi];
    if (!device.is<Solid::StorageAccess>()) {
        return false;
    }

    Solid::StorageAccess *storageaccess = device.as<Solid::StorageAccess>();
    if (storageaccess == 0) return false;
    
    QVariant freeSpaceVar;
    qlonglong freeSpace = freeDiskSpace(storageaccess->filePath());
    if ( freeSpace != -1 ) {
        freeSpaceVar.setValue( freeSpace );
    }
    setData(udi, I18N_NOOP("Free Space"), freeSpaceVar );
    return true;
}

bool SolidDeviceEngine::updateSource(const QString& source)
{
    return updateFreeSpace(source);
}

void SolidDeviceEngine::deviceRemoved(const QString& udi)
{
    foreach (QString query, predicatemap.keys()) {
        predicatemap[query].removeAll(udi);
        devicemap.remove(udi);
        setData(query, predicatemap[query]);
    }
    
    removeSource(udi);
    checkForUpdates();
}

void SolidDeviceEngine::deviceChanged(const QString& udi, const QString &property, const QVariant &value)
{
    setData(udi, property, value);
    checkForUpdates();
}

#include "soliddeviceengine.moc"
