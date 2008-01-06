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
        : Plasma::DataEngine(parent)
{
    Q_UNUSED(args)
    signalmanager = new DeviceSignalMapManager(this);
}

SolidDeviceEngine::~SolidDeviceEngine()
{
    delete signalmanager;
}

void SolidDeviceEngine::fillDevices()
{
    if (devicelist.isEmpty()) {
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Processor)) {
            processorlist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Block)) {
            blocklist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess)) {
            storageaccesslist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive)) {
            storagedrivelist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::OpticalDrive)) {
            opticaldrivelist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume)) {
            storagevolumelist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc)) {
            opticaldisclist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Camera)) {
            cameralist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::PortableMediaPlayer)) {
            portablemediaplayerlist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::NetworkInterface)) {
            networkinterfacelist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::AcAdapter)) {
            acadapterlist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Battery)) {
            batterylist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Button)) {
            buttonlist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::AudioInterface)) {
            audiointerfacelist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::DvbInterface)) {
            dvbinterfacelist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Unknown)) {
            unknownlist << device.udi();
            if (!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        //detect when new devices are added
        Solid::DeviceNotifier *notifier = Solid::DeviceNotifier::instance();
        connect(notifier, SIGNAL(deviceAdded(const QString&)),
                this, SLOT(deviceAdded(const QString&)));
        connect(notifier, SIGNAL(deviceRemoved(const QString&)),
                this, SLOT(deviceRemoved(const QString&)));
    }
}

bool SolidDeviceEngine::sourceRequested(const QString &name)
{
    /* This creates a list of all available devices.  This must be called first before any other sources
     * will be available.
     */
    if (name == "Devices") {
        fillDevices();
        if (devicelist.isEmpty() ) {
            return false;
        }
        setData(name, I18N_NOOP("Proccessor List"), processorlist);
        setData(name, I18N_NOOP("Block List"), blocklist);
        setData(name, I18N_NOOP("Storage Access List"), storageaccesslist);
        setData(name, I18N_NOOP("Storage Drive List"), storagedrivelist);
        setData(name, I18N_NOOP("Optical Drive List"), opticaldrivelist);
        setData(name, I18N_NOOP("Storage Volume List"), storagevolumelist);
        setData(name, I18N_NOOP("Optical Disc List"), opticaldisclist);
        setData(name, I18N_NOOP("Camera List"), cameralist);
        setData(name, I18N_NOOP("Portable Media Player List"), portablemediaplayerlist);
        setData(name, I18N_NOOP("Network Interface List"), networkinterfacelist);
        setData(name, I18N_NOOP("Ac Adapter List"), acadapterlist);
        setData(name, I18N_NOOP("Battery List"), batterylist);
        setData(name, I18N_NOOP("Button List"), buttonlist);
        setData(name, I18N_NOOP("Audio Interface List"), audiointerfacelist);
        setData(name, I18N_NOOP("DVB Interface List"), dvbinterfacelist);
        setData(name, I18N_NOOP("Unknown List"), unknownlist);
        setData(name, I18N_NOOP("Device List"), devicelist);
        return true;
    }
    else {
        if (devicelist.contains(name) ) {
            return populateDeviceData(name);
        }
        else {
            return false;
        }
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

    if (processorlist.contains(name)) {
        Solid::Processor *processor = device.as<Solid::Processor>();
        if (processor == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Processor");
        setData(name, I18N_NOOP("Number"), processor->number());
        setData(name, I18N_NOOP("Max Speed"), processor->maxSpeed());
        setData(name, I18N_NOOP("Can Change Frequency"), processor->canChangeFrequency());
    }
    if (blocklist.contains(name)) {
        Solid::Block *block = device.as<Solid::Block>();
        if (block == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Block");
        setData(name, I18N_NOOP("Major"), block->deviceMajor());
        setData(name, I18N_NOOP("Minor"), block->deviceMajor());
        setData(name, I18N_NOOP("Device"), block->device());
    }
    if (storageaccesslist.contains(name)) {
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
    if (storagedrivelist.contains(name)) {
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
    if (opticaldrivelist.contains(name)) {
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
    if (storagevolumelist.contains(name)) {
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
    if (opticaldisclist.contains(name)) {
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
    if (cameralist.contains(name)) {
        Solid::Camera *camera = device.as<Solid::Camera>();
        if (camera == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Camera");

        setData(name, I18N_NOOP("Supported Protocols"), camera->supportedProtocols());
        setData(name, I18N_NOOP("Supported Drivers"), camera->supportedDrivers());
    }
    if (portablemediaplayerlist.contains(name)) {
        Solid::PortableMediaPlayer *mediaplayer = device.as<Solid::PortableMediaPlayer>();
        if (mediaplayer == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("Portable Media Player");

        setData(name, I18N_NOOP("Supported Protocols"), mediaplayer->supportedProtocols());
        setData(name, I18N_NOOP("Supported Drivers"), mediaplayer->supportedDrivers());
    }
    if (networkinterfacelist.contains(name)) {
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
    if (acadapterlist.contains(name)) {
        Solid::AcAdapter *ac = device.as<Solid::AcAdapter>();
        if (ac == 0) {
            return false;
        }

        devicetypes << I18N_NOOP("AD Adapter");

        setData(name, I18N_NOOP("Plugged In"), ac->isPlugged());
        signalmanager->mapDevice(ac, device.udi());
    }
    if (batterylist.contains(name)) {
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
    if (buttonlist.contains(name)) {
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
    if (audiointerfacelist.contains(name)) {
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
    if (dvbinterfacelist.contains(name)) {
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
    devicelist << udi;
    QString name = I18N_NOOP("Devices");
    setData(name, I18N_NOOP("Device List"), devicelist);

    //add to device specific lists
    Solid::Device device(udi);
    if (device.is<Solid::Processor>()) {
        processorlist << udi;
        setData(name, I18N_NOOP("Proccessor List"), processorlist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::Block>()) {
        blocklist << udi;
        setData(name, I18N_NOOP("Block List"), blocklist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::StorageAccess>()) {
        storageaccesslist << udi;
        setData(name, I18N_NOOP("Storage Access List"), storageaccesslist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::StorageDrive>()) {
        storagedrivelist << udi;
        setData(name, I18N_NOOP("Storage Drive List"), storagedrivelist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::OpticalDrive>()) {
        opticaldrivelist << udi;
        setData(name, I18N_NOOP("Optical Drive List"), opticaldrivelist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::StorageVolume>()) {
        storagevolumelist << udi;
        setData(name, I18N_NOOP("Storage Volume List"), storagevolumelist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::OpticalDisc>()) {
        opticaldisclist << udi;
        setData(name, I18N_NOOP("Optical Disc List"), opticaldisclist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::Camera>()) {
        cameralist << udi;
        setData(name, I18N_NOOP("Camera List"), cameralist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::PortableMediaPlayer>()) {
        portablemediaplayerlist << udi;
        setData(name, I18N_NOOP("Portable Media Player List"), portablemediaplayerlist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::NetworkInterface>()) {
        networkinterfacelist << udi;
        setData(name, I18N_NOOP("Network Interface List"), networkinterfacelist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::AcAdapter>()) {
        acadapterlist << udi;
        setData(name, I18N_NOOP("AD Adapter List"), acadapterlist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::Battery>()) {
        batterylist << udi;
        setData(name, I18N_NOOP("Battery List"), batterylist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::Button>()) {
        buttonlist << udi;
        setData(name, I18N_NOOP("Button List"), buttonlist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::AudioInterface>()) {
        audiointerfacelist << udi;
        setData(name, I18N_NOOP("Audio Interface List"), audiointerfacelist);
        devicemap[udi] = device;
    }
    if (device.is<Solid::DvbInterface>()) {
        dvbinterfacelist << udi;
        setData(name, I18N_NOOP("DVB Interface List"), dvbinterfacelist);
        devicemap[udi] = device;
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
#elif defined(HAVE_STATFS) && !defined(Q_OS_SOLARIS)
    struct statfs fs_obj;
    if (statfs(path,&fs_obj) < 0){
        return -1;
    }
    else{
        return (qlonglong)fs_obj.f_bfree*(qlonglong)fs_obj.f_bsize;
    }
#else
#warning "This system does not support statfs or statvfs - freeDiskSpace() will return -1"
    return -1;
#endif
}

bool SolidDeviceEngine::updateFreeSpace(const QString &udi)
{
    if (storageaccesslist.contains(udi)){
        Solid::Device device = devicemap[udi];
        if (!device.isValid()) {
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
    return false;
}

bool SolidDeviceEngine::updateSource(const QString& source)
{
    if (storageaccesslist.contains(source)){
        updateFreeSpace(source);
        return true;
    }
    else{
        return false;
    }
}

void SolidDeviceEngine::deviceRemoved(const QString& udi)
{
    int pos = devicelist.indexOf(udi);
    if (pos > -1) {
        devicelist.removeAt(pos);
        devicemap.remove(udi);
        removeSource(udi);
        setData(I18N_NOOP("Devices"), I18N_NOOP("Device List"), devicelist);
    }
    checkForUpdates();
}

void SolidDeviceEngine::deviceChanged(const QString& udi, const QString &property, const QVariant &value)
{
    setData(udi, property, value);
    checkForUpdates();
}

#include "soliddeviceengine.moc"
