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

#include "soliddeviceengine.h"

#include <KDebug>
#include <KLocale>

#include "plasma/datasource.h"

SolidDeviceEngine::SolidDeviceEngine(QObject* parent, const QStringList& args)
        : Plasma::DataEngine(parent)
{
    Q_UNUSED(args)
    signalmanager = new DeviceSignalMapManager(this);
}

SolidDeviceEngine::~SolidDeviceEngine()
{
    delete signalmanager;
}

bool SolidDeviceEngine::sourceRequested(const QString &name)
{
    /* This creates a list of all available devices.  This must be called first before any other sources
     * will be available.
     */
    if(name == "Devices") {
        //if the devicelist is already populated, return
        if(!devicelist.isEmpty()) {
            return true;
        }

        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Processor)) {
            processorlist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "ProccessorList", processorlist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Block)) {
            blocklist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "BlockList", blocklist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess)) {
            storageaccesslist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "StorageAccessList", storageaccesslist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive)) {
            storagedrivelist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "StorageDriveList", storagedrivelist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::OpticalDrive)) {
            opticaldrivelist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "OpticalDriveList", opticaldrivelist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume)) {
            storagevolumelist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "StorageVolumeList", storagevolumelist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc)) {
            opticaldisclist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "OpticalDiscList", opticaldisclist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Camera)) {
            cameralist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "CameraList", cameralist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::PortableMediaPlayer)) {
            portablemediaplayerlist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "PortableMediaPlayerList", portablemediaplayerlist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::NetworkInterface)) {
            networkinterfacelist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "NetworkInterfaceList", networkinterfacelist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::AcAdapter)) {
            acadapterlist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "AcAdapterList", acadapterlist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Battery)) {
            batterylist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "BatteryList", batterylist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Button)) {
            buttonlist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "ButtonList", buttonlist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::AudioInterface)) {
            audiointerfacelist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "AudioInterfaceList", audiointerfacelist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::DvbInterface)) {
            dvbinterfacelist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "DvbInterfaceList", dvbinterfacelist);
        foreach(Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Unknown)) {
            unknownlist << device.udi();
            if(!devicelist.contains(device.udi())) {
                devicelist << device.udi();
                devicemap[device.udi()] = device;
            }
        }
        setData(name, "UnknownList", unknownlist);

        if(devicelist.isEmpty() ) {
            return false;
        }
        setData(name, "DeviceList", devicelist);

        //detect when new devices are added
        Solid::DeviceNotifier *notifier = Solid::DeviceNotifier::instance();
        connect(notifier, SIGNAL(deviceAdded(const QString&)),
                this, SLOT(deviceAdded(const QString&)));
        connect(notifier, SIGNAL(deviceRemoved(const QString&)),
                this, SLOT(deviceRemoved(const QString&)));

        return true;
    }
    else {
        if(devicelist.contains(name) ) {
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
    if(!device.isValid()) {
        return false;
    }

    QStringList devicetypes;
    setData(name, "ParentUDI", device.parentUdi());
    setData(name, "Vendor", device.vendor());
    setData(name, "Product", device.product());
    setData(name, "Icon", device.icon());

    if(processorlist.contains(name)) {
        Solid::Processor *processor = device.as<Solid::Processor>();
        if(processor == 0) {
            return false;
        }

        devicetypes << "Processor";
        setData(name, "Number", processor->number());
        setData(name, "MaxSpeed", processor->maxSpeed());
        setData(name, "CanChangeFrequency", processor->canChangeFrequency());
    }
    if(blocklist.contains(name)) {
        Solid::Block *block = device.as<Solid::Block>();
        if(block == 0) {
            return false;
        }

        devicetypes << "Block";
        setData(name, "Major", block->deviceMajor());
        setData(name, "Minor", block->deviceMajor());
        setData(name, "Device", block->device());
    }
    if(storageaccesslist.contains(name)) {
        Solid::StorageAccess *storageaccess = device.as<Solid::StorageAccess>();
        if(storageaccess == 0) return false;

        devicetypes << "StorageAccess";
        setData(name, "Accessible", storageaccess->isAccessible());
        setData(name, "FilePath", storageaccess->filePath());

        //signalmanager->mapDevice(storageaccess, device.udi());
    }
    if(storagedrivelist.contains(name)) {
        Solid::StorageDrive *storagedrive = device.as<Solid::StorageDrive>();
        if(storagedrive == 0) {
            return false;
        }

        devicetypes << "StorageDrive";

        QStringList bus;
        bus << "Ide" <<    "Usb" << "Ieee1394" << "Scsi" << "Sata" << "Platform";
        QStringList drivetype;
        drivetype << "HardDisk" <<  "CdromDrive" <<  "Floppy" <<  "Tape" <<  "CompactFlash" <<  "MemoryStick" <<  "SmartMedia" <<  "SdMmc" <<  "Xd";

        setData(name, "Bus", bus.at((int)storagedrive->bus()));
        setData(name, "DriveType", drivetype.at((int)storagedrive->driveType()));
        setData(name, "Removable", storagedrive->isRemovable());
        setData(name, "Hotpluggable", storagedrive->isHotpluggable());
    }
    if(opticaldrivelist.contains(name)) {
        Solid::OpticalDrive *opticaldrive = device.as<Solid::OpticalDrive>();
        if(opticaldrive == 0) {
            return false;
        }

        devicetypes << "OpticalDrive";

        QStringList supportedtypes;
        Solid::OpticalDrive::MediumTypes mediatypes = opticaldrive->supportedMedia();
        if(mediatypes & Solid::OpticalDrive::Cdr) {
            supportedtypes << "Cdr";
        }
        if(mediatypes & Solid::OpticalDrive::Cdrw) {
            supportedtypes << "Cdrw";
        }
        if(mediatypes & Solid::OpticalDrive::Dvd) {
            supportedtypes << "Dvd";
        }
        if(mediatypes & Solid::OpticalDrive::Dvdr) {
            supportedtypes << "Dvdr";
        }
        if(mediatypes & Solid::OpticalDrive::Dvdrw) {
            supportedtypes << "Dvdrw";
        }
        if(mediatypes & Solid::OpticalDrive::Dvdram) {
            supportedtypes << "Dvdram";
        }
        if(mediatypes & Solid::OpticalDrive::Dvdplusr) {
            supportedtypes << "Dvdplusr";
        }
        if(mediatypes & Solid::OpticalDrive::Dvdplusrw) {
            supportedtypes << "Dvdplusrw";
        }
        if(mediatypes & Solid::OpticalDrive::Dvdplusdl) {
            supportedtypes << "Dvdplusdl";
        }
        if(mediatypes & Solid::OpticalDrive::Dvdplusdlrw) {
            supportedtypes << "Dvdplusdlrw";
        }
        if(mediatypes & Solid::OpticalDrive::Bd) {
            supportedtypes << "Bd";
        }
        if(mediatypes & Solid::OpticalDrive::Bdr) {
            supportedtypes << "Bdr";
        }
        if(mediatypes & Solid::OpticalDrive::Bdre) {
            supportedtypes << "Bdre";
        }
        if(mediatypes & Solid::OpticalDrive::HdDvd) {
            supportedtypes << "HdDvd";
        }
        if(mediatypes & Solid::OpticalDrive::HdDvdr) {
            supportedtypes << "HdDvdr";
        }
        if(mediatypes & Solid::OpticalDrive::HdDvdrw) {
            supportedtypes << "HdDvdrw";
        }
        setData(name, "SupportedMedia", supportedtypes);

        setData(name, "ReadSpeed", opticaldrive->readSpeed());
        setData(name, "WriteSpeed", opticaldrive->writeSpeed());

        //the following method return QList<int> so we need to convert it to QList<QVariant>
        QList<int> writespeeds = opticaldrive->writeSpeeds();
        QList<QVariant> variantlist = QList<QVariant>();
        foreach(int num, writespeeds) {
            variantlist << QVariant(num);
        }
        setData(name, "WriteSpeeds", variantlist);

    }
    if(storagevolumelist.contains(name)) {
        Solid::StorageVolume *storagevolume = device.as<Solid::StorageVolume>();
        if(storagevolume == 0) {
            return false;
        }

        devicetypes << "StorageVolume";

        QStringList usagetypes;
        usagetypes << "FileSystem" << "PartitionTable" << "Raid" << "Other" << "Unused";

        setData(name, "Ignored", storagevolume->isIgnored());
        setData(name, "Usage", usagetypes.at((int)storagevolume->usage()));
        setData(name, "FileSystemType", storagevolume->fsType());
        setData(name, "Label", storagevolume->label());
        setData(name, "Uuid", storagevolume->uuid());
        setData(name, "Size", storagevolume->size());
    }
    if(opticaldisclist.contains(name)) {
        Solid::OpticalDisc *opticaldisc = device.as<Solid::OpticalDisc>();
        if(opticaldisc == 0) {
            return false;
        }

        devicetypes << "OpticalDisc";

        //get the content types
        QStringList contenttypelist;
        Solid::OpticalDisc::ContentTypes contenttypes = opticaldisc->availableContent();
        if(contenttypes & Solid::OpticalDisc::Audio) {
            contenttypelist << "Audio";
        }
        if(contenttypes & Solid::OpticalDisc::Audio) {
            contenttypelist << "Data";
        }
        if(contenttypes & Solid::OpticalDisc::Audio) {
            contenttypelist << "VideoCd";
        }
        if(contenttypes & Solid::OpticalDisc::Audio) {
            contenttypelist << "SuperVideoCd";
        }
        if(contenttypes & Solid::OpticalDisc::Audio) {
            contenttypelist << "VideoDvd";
        }
        setData(name, "AvailableContent", contenttypelist);

        QStringList disctypes;
        disctypes << "UnknownDiscType" << "CdRom" << "CdRecordable"
        << "CdRewritable" << "DvdRom" << "DvdRam"
        << "DvdRecordable" << "DvdRewritable" << "DvdPlusRecordable"
        << "DvdPlusRewritable" << "DvdPlusRecordableDuallayer" << "DvdPlusRewritableDuallayer"
        << "BluRayRom" << "BluRayRecordable" << "BluRayRewritable"
        << "HdDvdRom" <<  "HdDvdRecordable" << "HdDvdRewritable";
        //+1 because the enum starts at -1
        setData(name, "DiscType", disctypes.at((int)opticaldisc->discType() + 1));
        setData(name, "Appendable", opticaldisc->isAppendable());
        setData(name, "Blank", opticaldisc->isBlank());
        setData(name, "Rewritable", opticaldisc->isRewritable());
        setData(name, "Capacity", opticaldisc->capacity());
    }
    if(cameralist.contains(name)) {
        Solid::Camera *camera = device.as<Solid::Camera>();
        if(camera == 0) {
            return false;
        }

        devicetypes << "Camera";

        setData(name, "SupportedProtocols", camera->supportedProtocols());
        setData(name, "SupportedDrivers", camera->supportedDrivers());
    }
    if(portablemediaplayerlist.contains(name)) {
        Solid::PortableMediaPlayer *mediaplayer = device.as<Solid::PortableMediaPlayer>();
        if(mediaplayer == 0) {
            return false;
        }

        devicetypes << "PortableMediaPlayer";

        setData(name, "SupportedProtocols", mediaplayer->supportedProtocols());
        setData(name, "SupportedDrivers", mediaplayer->supportedDrivers());
    }
    if(networkinterfacelist.contains(name)) {
        Solid::NetworkInterface *networkinterface = device.as<Solid::NetworkInterface>();
        if(networkinterface == 0) {
            return false;
        }

        devicetypes << "NetworkInterface";

        setData(name, "IfaceName", networkinterface->ifaceName());
        setData(name, "Wireless", networkinterface->isWireless());
        setData(name, "HwAddress", networkinterface->hwAddress());
        setData(name, "MacAddress", networkinterface->macAddress());
    }
    if(acadapterlist.contains(name)) {
        Solid::AcAdapter *ac = device.as<Solid::AcAdapter>();
        if(ac == 0) {
            return false;
        }

        devicetypes << "AcAdapter";

        setData(name, "Plugged In", ac->isPlugged());
        signalmanager->mapDevice(ac, device.udi());
    }
    if(batterylist.contains(name)) {
        Solid::Battery *battery = device.as<Solid::Battery>();
        if(battery == 0) {
            return false;
        }

        devicetypes << "Battery";

        QStringList batterytype;
        batterytype << "UnknownBattery" << "PdaBattery" << "UpsBattery"
        << "PrimaryBattery" << "MouseBattery" << "KeyboardBattery"
        << " KeyboardMouseBattery" << "CameraBattery";

        QStringList chargestate;
        chargestate << "Fully Charged" << "Charging" << "Discharging";

        setData(name, "Plugged In", battery->isPlugged());
        setData(name, "Type", batterytype.at((int)battery->type()));
        setData(name, "Charge Value Unit", battery->chargeValueUnit());
        setData(name, "Charge Value", battery->chargeValue());
        setData(name, "Charge Percent", battery->chargePercent());
        setData(name, "Voltage Unit", battery->voltageUnit());
        setData(name, "Voltage", battery->voltage());
        setData(name, "Rechargeable", battery->isRechargeable());
        setData(name, "Charge State", chargestate.at((int)battery->chargeState()));
        
        signalmanager->mapDevice(battery, device.udi());
    }
    if(buttonlist.contains(name)) {
        Solid::Button *button = device.as<Solid::Button>();
        if(button == 0) {
            return false;
        }

        devicetypes << "Button";

        QStringList buttontype;
        buttontype << "LidButton" << "PowerButton" << "SleepButton" << "UnknownButtonType";

        setData(name, "Type", buttontype.at((int)button->type()));
        setData(name, "HasState", button->hasState());
        setData(name, "StateValue", button->stateValue());
        setData(name, "Pressed", false);  //this is an extra value that is tracked by the button signals
        
        signalmanager->mapDevice(button, device.udi());
    }
    if(audiointerfacelist.contains(name)) {
        Solid::AudioInterface *audiointerface = device.as<Solid::AudioInterface>();
        if(audiointerface == 0) {
            return false;
        }

        devicetypes << "AudioInterface";

        QStringList audiodriver;
        audiodriver << "Alsa" << "OpenSoundSystem" << "UnknownAudioDriver";

        setData(name, "Driver", audiodriver.at((int)audiointerface->driver()));
        setData(name, "DriverHandle", audiointerface->driverHandle());
        setData(name, "Name", audiointerface->name());

        //get AudioInterfaceTypes
        QStringList audiointerfacetypes;
        Solid::AudioInterface::AudioInterfaceTypes devicetypes = audiointerface->deviceType();
        if(devicetypes & Solid::AudioInterface::UnknownAudioInterfaceType) {
            audiointerfacetypes << "UnknownAudioInterfaceType";
        }
        if(devicetypes & Solid::AudioInterface::AudioControl) {
            audiointerfacetypes << "AudioControl";
        }
        if(devicetypes & Solid::AudioInterface::AudioInput) {
            audiointerfacetypes << "AudioInput";
        }
        if(devicetypes & Solid::AudioInterface::AudioOutput) {
            audiointerfacetypes << "AudioOutput";
        }
        setData(name, "DeviceType", audiointerfacetypes);

        //get SoundCardTypes
        QStringList soundcardtype;
        soundcardtype << "InternalSoundcard" << "UsbSoundcard" << "FirewireSoundcard" << "Headset" << "Modem";
        setData(name, "SoundcardType", soundcardtype.at((int)audiointerface->soundcardType()));
    }
    if(dvbinterfacelist.contains(name)) {
        Solid::DvbInterface *dvbinterface = device.as<Solid::DvbInterface>();
        if(dvbinterface == 0) {
            return false;
        }

        devicetypes << "DvbInterface";

        setData(name, "Device", dvbinterface->device());
        setData(name, "DeviceAdapter", dvbinterface->deviceAdapter());

        //get devicetypes
        QStringList dvbdevicetypes;
        dvbdevicetypes << "DvbUnknown" << "DvbAudio" << "DvbCa"
        << "DvbDemux" << "DvbDvr" << "DvbFrontend"
        << "DvbNet" << "DvbOsd" << "DvbSec" << "DvbVideo";

        setData(name, "DvbDeviceType", dvbdevicetypes.at((int)dvbinterface->deviceType()));
        setData(name, "DeviceIndex", dvbinterface->deviceIndex());
    }
    setData(name, "DeviceTypes", devicetypes);
    return true;
}

void SolidDeviceEngine::deviceAdded(const QString& udi)
{
    devicelist << udi;
    QString name = I18N_NOOP("Devices");
    setData(name, "DeviceList", devicelist);

    //add to device specific lists
    Solid::Device device(udi);
    if(device.is<Solid::Processor>()) {
        processorlist << udi;
        setData(name, "ProccessorList", processorlist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::Block>()) {
        blocklist << udi;
        setData(name, "BlockList", blocklist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::StorageAccess>()) {
        storageaccesslist << udi;
        setData(name, "StorageAccessList", storageaccesslist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::StorageDrive>()) {
        storagedrivelist << udi;
        setData(name, "StorageDriveList", storagedrivelist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::OpticalDrive>()) {
        opticaldrivelist << udi;
        setData(name, "OpticalDriveList", opticaldrivelist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::StorageVolume>()) {
        storagevolumelist << udi;
        setData(name, "StorageVolumeList", storagevolumelist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::OpticalDisc>()) {
        opticaldisclist << udi;
        setData(name, "OpticalDiscList", opticaldisclist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::Camera>()) {
        cameralist << udi;
        setData(name, "CameraList", cameralist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::PortableMediaPlayer>()) {
        portablemediaplayerlist << udi;
        setData(name, "PortableMediaPlayerList", portablemediaplayerlist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::NetworkInterface>()) {
        networkinterfacelist << udi;
        setData(name, "NetworkInterfaceList", networkinterfacelist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::AcAdapter>()) {
        acadapterlist << udi;
        setData(name, "AcAdapterList", acadapterlist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::Battery>()) {
        batterylist << udi;
        setData(name, "BatteryList", batterylist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::Button>()) {
        buttonlist << udi;
        setData(name, "ButtonList", buttonlist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::AudioInterface>()) {
        audiointerfacelist << udi;
        setData(name, "AudioInterfaceList", audiointerfacelist);
        devicemap[udi] = device;
    }
    if(device.is<Solid::DvbInterface>()) {
        dvbinterfacelist << udi;
        setData(name, "DvbInterfaceList", dvbinterfacelist);
        devicemap[udi] = device;
    }

    checkForUpdates();
}

void SolidDeviceEngine::deviceRemoved(const QString& udi)
{
    int pos = devicelist.indexOf(udi);
    if(pos > -1) {
        devicelist.removeAt(pos);
        devicemap.remove(udi);
        removeSource(udi);
        setData(I18N_NOOP("Devices"), "DeviceList", devicelist);
    }
    checkForUpdates();
}

void SolidDeviceEngine::deviceChanged(const QString& udi, const QString &property, QVariant value)
{
    setData(udi, property, value);
    checkForUpdates();
}

#include "soliddeviceengine.moc"
