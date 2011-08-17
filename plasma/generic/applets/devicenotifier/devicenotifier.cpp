/***************************************************************************
 *   Copyright (C) 2007 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>            *
 *   Copyright 2009 by Jacopo De Simoi <wilderkde@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "devicenotifier.h"

//Qt
#include <QGraphicsSceneContextMenuEvent>

//KDE
#include <KConfigDialog>
#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>
#include <KCModuleProxy>
#include <KCModuleInfo>
#include <kdesktopfileactions.h>

//Plasma
#include <Plasma/ToolTipManager>

//solid
#include <Solid/Device>
#include <Solid/StorageDrive>
#include <Solid/OpticalDisc>
#include <Solid/OpticalDrive>

//Own
#include "notifierdialog.h"
#include "deviceitem.h"
#include <Plasma/Containment>
using namespace Plasma;

static const char DEFAULT_ICON_NAME[] = "device-notifier";

K_EXPORT_PLASMA_APPLET(devicenotifier, Notifier::DeviceNotifier)

namespace Notifier
{

HotplugDataConsumer::HotplugDataConsumer(NotifierDialog *parent)
    : QObject(parent),
      m_dialog(parent)
{
    Q_ASSERT(parent);
}

void HotplugDataConsumer::dataUpdated(const QString &udi, Plasma::DataEngine::Data data)
{
    if (data.isEmpty()) {
        return;
    }

    //data from hotplug engine
    //kDebug() << "adding" << data["udi"];
    int numActions = 0;
    QString lastActionLabel;
    QStringList currentActions = m_dialog->deviceActions(udi);
    QStringList newActions = data["predicateFiles"].toStringList();

    foreach (const QString &desktop, newActions) {
        QString filePath = KStandardDirs::locate("data", "solid/actions/" + desktop);
        QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(filePath, true);
        numActions += services.size();

        if (!currentActions.contains(desktop)) {
            m_dialog->insertAction(udi, desktop);
        }

        if (services.size() > 0) {
            lastActionLabel = QString(services[0].text());
        }
    }

    foreach (const QString &action, currentActions) {
        if (!newActions.contains(action)) {
            m_dialog->removeAction(udi, action);
        }
    }

    m_dialog->setDeviceData(udi, data["text"], Qt::DisplayRole);
    m_dialog->setDeviceData(udi, data["isEncryptedContainer"], NotifierDialog::IsEncryptedContainer);

    if (numActions > 1) {
        QString s = i18np("1 action for this device",
                          "%1 actions for this device",
                          numActions);
        m_dialog->setDeviceData(udi, s, NotifierDialog::DescriptionRole);
    } else {
        m_dialog->setDeviceData(udi, lastActionLabel, NotifierDialog::DescriptionRole);
    }
}

DeviceDataConsumer::DeviceDataConsumer(NotifierDialog *parent)
    : QObject(parent),
      m_dialog(parent)
{
    Q_ASSERT(parent);
}

void DeviceDataConsumer::dataUpdated(const QString &udi, Plasma::DataEngine::Data data)
{
    m_dialog->setDeviceData(udi, data["Icon"], NotifierDialog::IconNameRole);
    m_dialog->setDeviceData(udi, KIcon(data["Icon"].toString(), NULL, data["Emblems"].toStringList()), Qt::DecorationRole);

    const bool isOpticalMedia = data["Device Types"].toStringList().contains("OpticalDisc");

    m_dialog->setDeviceData(udi, isOpticalMedia, NotifierDialog::IsOpticalMedia);

    if (data["Device Types"].toStringList().contains("Storage Access")) {
        //kDebug() << "DeviceNotifier::solidDeviceEngine updated" << udi;
        if (data["Accessible"].toBool()) {
            m_dialog->setMounted(true, udi);
        } else {
            m_dialog->setMounted(false, udi);
        }

        if (data["Ignored"].toBool()) {
            m_dialog->setDeviceData(udi, data["File Path"], Qt::DisplayRole);

            const QString desktop("test-predicate-openinwindow.desktop");
            QString filePath = KStandardDirs::locate("data", "solid/actions/" + desktop);
            QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(filePath, true);
            if (services.size() > 0) { //in case there is no action at all
                m_dialog->insertAction(udi, desktop);
                m_dialog->setDeviceData(udi, services[0].text(), NotifierDialog::DescriptionRole);
            }

            m_dialog->setDeviceLeftAction(udi, DeviceItem::Nothing);
        }
    } else if (data["Device Types"].toStringList().contains("Storage Volume")) {
        if (isOpticalMedia) {
            m_dialog->setMounted(true, udi);
        }
    }
}

DeviceNotifier::DeviceNotifier(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_hotplugEngine(0),
      m_solidDeviceEngine(0),
      m_deviceNotificationsEngine(0),
      m_dialog(0),
      m_numberItems(0),
      m_itemsValidity(0),
      m_globalVisibility(false),
      m_checkHiddenDevices(true),
      m_triggeringPopupInternally(false),
      m_poppedUpInternally(false),
      m_autoMountingWidget(0),
      m_deviceActionsWidget(0)
{
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(IgnoreAspectRatio);
    KGlobal::locale()->insertCatalog("solid_qt");

    // let's initialize the widget
    resize(graphicsWidget()->minimumSize());
    m_hotplugDataConsumer = new HotplugDataConsumer(m_dialog);
    m_deviceDataConsumer = new DeviceDataConsumer(m_dialog);
}

DeviceNotifier::~DeviceNotifier()
{
    delete m_dialog;
}

void DeviceNotifier::init()
{
    configChanged();

    m_hotplugEngine = dataEngine("hotplug");
    m_solidDeviceEngine = dataEngine("soliddevice");
    m_deviceNotificationsEngine = dataEngine("devicenotifications");

    //don't close the dialog when the user is using it
    connect(m_dialog, SIGNAL(activated()), this, SLOT(showPopup()));

    Plasma::ToolTipManager::self()->registerWidget(this);

    setPopupIcon(DEFAULT_ICON_NAME);

    //connect to engine when a device is plugged in
    connect(m_hotplugEngine, SIGNAL(sourceAdded(QString)),
            this, SLOT(onSourceAdded(QString)));
    connect(m_hotplugEngine, SIGNAL(sourceRemoved(QString)),
            this, SLOT(onSourceRemoved(QString)));
    connect(m_deviceNotificationsEngine, SIGNAL(sourceAdded(QString)),
            this, SLOT(newNotification(QString)));

    setStatus(Plasma::PassiveStatus);
    //feed the list with what is already reported by the engine
    fillPreviousDevices();

    m_iconTimer = new QTimer(this);
    m_iconTimer->setSingleShot(true);
    connect(m_iconTimer, SIGNAL(timeout()), this, SLOT(resetNotifierIcon()));
}

void DeviceNotifier::newNotification(const QString &source)
{
    DataEngine::Data data = m_deviceNotificationsEngine->query(source);
    if (m_lastPlugged.contains(data["udi"].toString()) && !m_hiddenDevices.contains(data["udi"].toString())) {
        showNotification(data["error"].toString(), data["errorDetails"].toString(), data["udi"].toString());
    }
}

void DeviceNotifier::configChanged()
{
    KConfigGroup cg = config();
    m_numberItems = cg.readEntry("NumberItems", 4);
    m_itemsValidity = cg.readEntry("ItemsValidity", 5);
    m_showDevices = cg.readEntry("ShowDevices", (int)RemovableOnly);
}

QGraphicsWidget *DeviceNotifier::graphicsWidget()
{
    if (!m_dialog) {
        m_dialog = new NotifierDialog(this);
        connect(m_dialog, SIGNAL(actionSelected()), this, SLOT(hidePopup()));
        connect(m_dialog, SIGNAL(globalVisibilityChanged(bool)), this, SLOT(setGlobalVisibility(bool)));
    }

    return m_dialog->dialog();
}

void DeviceNotifier::fillPreviousDevices()
{
    m_fillingPreviousDevices = true;

    QList<Solid::Device> list = Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume);
    foreach (const Solid::Device &device, list) {
        // We manually add non-removable devices that are a priori ignored
        // discard swap and partition tables
        Solid::Device parentDevice = device.parent();
        Solid::StorageDrive *drive = parentDevice.as<Solid::StorageDrive>();
        const Solid::StorageVolume *volume = device.as<Solid::StorageVolume>();
        if (drive && (!drive->isHotpluggable() && !drive->isRemovable()) &&
            (volume->usage() == Solid::StorageVolume::FileSystem)) {
            deviceAdded(device, false);
        }
    }

    foreach (const QString &udi, m_hotplugEngine->sources()) {
        onSourceAdded(udi);
    }

    m_fillingPreviousDevices = false;
}

void DeviceNotifier::changeNotifierIcon(const QString& name, uint timeout)
{
    m_iconTimer->stop();
    setPopupIcon(name.isNull() ? DEFAULT_ICON_NAME : name);
    if (timeout) {
        m_iconTimer->setInterval(timeout);
        m_iconTimer->start();
    }
}

void DeviceNotifier::resetNotifierIcon()
{
    changeNotifierIcon();
    update();
}

void DeviceNotifier::popupEvent(bool show)
{
    if (show) {
        Plasma::ToolTipManager::self()->clearContent(this);
    } else if (status() == Plasma::NeedsAttentionStatus) {
        setStatus(Plasma::ActiveStatus);
    } else {
        m_dialog->collapseDevices();
    }

    if (m_triggeringPopupInternally && show) {
        m_poppedUpInternally = true;
    } else if (!show) {
        m_poppedUpInternally = false;
    }

    if (!m_triggeringPopupInternally) {
        changeNotifierIcon();
    }
    m_triggeringPopupInternally = false;
}

void DeviceNotifier::keepPopupOpen()
{
    if (!m_poppedUpInternally) {
        m_poppedUpInternally = false;
        showPopup();
    }
}

void DeviceNotifier::notifyDevice(const QString &udi)
{
    m_lastPlugged << udi;

    if (!m_fillingPreviousDevices) {
        emit activate();
        changeNotifierIcon("preferences-desktop-notification", LONG_NOTIFICATION_TIMEOUT);
        m_triggeringPopupInternally = true;
        showPopup(LONG_NOTIFICATION_TIMEOUT);
        update();
        setStatus(Plasma::NeedsAttentionStatus);
    } else {
        setStatus(Plasma::ActiveStatus);
    }
}

void DeviceNotifier::toolTipAboutToShow()
{
    Plasma::ToolTipContent toolTip;
    if (m_lastPlugged.isEmpty()) {
        toolTip.setMainText(i18n("No devices available."));
        toolTip.setImage(KIcon("device-notifier"));
    } else {
        Solid::Device device(m_lastPlugged.last());
        toolTip.setMainText(i18n("Most recent device"));
        toolTip.setSubText(device.description());
        toolTip.setImage(KIcon(device.icon()));
    }

    Plasma::ToolTipManager::self()->setContent(this, toolTip);
}

void DeviceNotifier::toolTipHidden()
{
    Plasma::ToolTipManager::self()->clearContent(this);
}

void DeviceNotifier::removeLastDeviceNotification(const QString &udi)
{
    m_lastPlugged.removeAll(udi);
}

void DeviceNotifier::onSourceAdded(const QString &udi)
{
    DataEngine::Data data = m_hotplugEngine->query(udi);
    Solid::Device device = Solid::Device(udi);
    deviceAdded(device, data["added"].toBool());
}

void DeviceNotifier::deviceAdded(const Solid::Device &device, bool hotplugged)
{
    const QString udi = device.udi();
    if (m_showDevices == NonRemovableOnly) {
        Solid::Device parentDevice = device.parent();
        Solid::StorageDrive *drive = parentDevice.as<Solid::StorageDrive>();
        if (drive && (drive->isHotpluggable() || drive->isRemovable())) {
            return;
        }
    } else if (m_showDevices == RemovableOnly) {
        Solid::Device device = Solid::Device(udi);
        Solid::Device parentDevice = device.parent();
        Solid::StorageDrive *drive = parentDevice.as<Solid::StorageDrive>();
        if (drive && (!drive->isHotpluggable() && !drive->isRemovable())) {
            return;
        }
    }

    kDebug() << "DeviceNotifier:: source added" << udi;
    KConfigGroup cg = config();
    bool visibility = cg.readEntry(udi, true);

    if (visibility || m_globalVisibility) {
        // WORKAROUND: Some distributions do not set the HAL flag volume.ignore = true
        // on partitions belonging to fixed devices; this causes issues since so far
        // the code assumed this behavior. In particular some fixed devices might
        // be added to the notifier twice: once as fixed devices (by fillPreviousDevices)
        // and once as hotplugged devices (by onSourceAdded). The first time, however
        // the dataengine might not be ready with the required data, so we need to
        // make sure that the second time we reconnect the notifier to both engines in order to
        // receive data through dataUpdated.
        //
        // Please note this is by no means a correct fix; it just avoids
        // showing duplicate entries; mount/unmount actions typically will not
        // work for such devices. A real fix seems to require a good amount of
        // new code and would need to be throughoutly tested.
        //
        // TODO: Handling of fixed devices needs to be rethought for 4.6

        if (m_lastPlugged.contains(udi)) {
            // Reconnect to both engines since now we can possibly
            // correctly connect to the updates.
            m_hotplugEngine->disconnectSource(udi, m_hotplugDataConsumer);
            m_solidDeviceEngine->disconnectSource(udi, m_deviceNotificationsEngine);
        } else {
            m_dialog->insertDevice(udi);

            if (hotplugged) {
                notifyDevice(udi);
                m_dialog->expandDevice(udi);
            }

            m_dialog->setDeviceData(udi, visibility, NotifierDialog::VisibilityRole);
            m_lastPlugged << udi;
        }

        m_hotplugEngine->connectSource(udi, m_hotplugDataConsumer);
        m_solidDeviceEngine->connectSource(udi, m_deviceDataConsumer);
    }

    if (!visibility && !m_hiddenDevices.contains(udi)) {
        m_hiddenDevices << udi;
    }

    if (visibility) {
        setStatus(Plasma::ActiveStatus);
    }
}

void DeviceNotifier::onSourceRemoved(const QString &udi)
{
    m_hotplugEngine->disconnectSource(udi, m_hotplugDataConsumer);
    m_solidDeviceEngine->disconnectSource(udi, m_deviceDataConsumer);

    m_dialog->removeDevice(udi);
    removeLastDeviceNotification(udi);
    if (m_checkHiddenDevices) {
        m_hiddenDevices.removeAll(udi);
    } else {
        m_checkHiddenDevices = false;
    }
    if (m_lastPlugged.count() == 0) {
        setStatus(Plasma::PassiveStatus);
    } else {
        setStatus(Plasma::ActiveStatus);
    }
}

void DeviceNotifier::resetDevices()
{
    while (m_lastPlugged.count() > 0) {
        QString udi = m_lastPlugged.takeAt(0);
        onSourceRemoved(udi);
    }

    fillPreviousDevices();
}

void DeviceNotifier::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *configurationWidget = new QWidget();
    m_configurationUi.setupUi(configurationWidget);
    m_deviceActionsWidget = new KCModuleProxy("solid-actions");
    m_autoMountingWidget = new KCModuleProxy("device_automounter_kcm");

    parent->addPage(configurationWidget, i18n("Display"), icon());
    parent->addPage(m_deviceActionsWidget, m_deviceActionsWidget->moduleInfo().moduleName(),
                    m_deviceActionsWidget->moduleInfo().icon());
    parent->addPage(m_autoMountingWidget, i18n("Automounting"),
                    m_autoMountingWidget->moduleInfo().icon());

    parent->setButtons( KDialog::Ok | KDialog::Cancel);
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    switch (m_showDevices) {
        case RemovableOnly:
            m_configurationUi.removableDevices->setChecked(true);
            break;
        case NonRemovableOnly:
            m_configurationUi.nonRemovableDevices->setChecked(true);
            break;
        case AllDevices:
            m_configurationUi.allDevices->setChecked(true);
            break;
    }
}

void DeviceNotifier::configAccepted()
{
    KConfigGroup cg = config();

    if (m_configurationUi.allDevices->isChecked()) {
        m_showDevices = AllDevices;
    } else if (m_configurationUi.nonRemovableDevices->isChecked()) {
        m_showDevices = NonRemovableOnly;
    } else {
        m_showDevices = RemovableOnly;
    }

    cg.writeEntry("ShowDevices", m_showDevices);

    //Save the configurations of the embedded KCMs
    m_deviceActionsWidget->save();
    m_autoMountingWidget->save();

    emit configNeedsSaving();

    resetDevices();
}

void DeviceNotifier::setDeviceVisibility(const QString &udi, bool visibility)
{
    m_dialog->setDeviceData(udi, visibility, NotifierDialog::VisibilityRole);
    m_checkHiddenDevices = false;
    if (visibility) {
        m_hiddenDevices.removeAll(udi);
    } else {
        m_hiddenDevices << udi;
    }

    if (!visibility && !m_globalVisibility) {
        onSourceRemoved(udi);
    }

    KConfigGroup cg = config();
    cg.writeEntry(udi, visibility);
}

void DeviceNotifier::setGlobalVisibility(bool visibility)
{
    m_globalVisibility = visibility;
    resetDevices();
}

void DeviceNotifier::showNotification(const QString &message, const QString &details, const QString &udi)
{
    if (!isPopupShowing()) {
        m_triggeringPopupInternally = true;
        showPopup(LONG_NOTIFICATION_TIMEOUT);
    }

    m_dialog->showStatusBarMessage(message, details, udi);

    update();
}

bool DeviceNotifier::areThereHiddenDevices()
{
    return (m_hiddenDevices.count() > 0);
}

bool DeviceNotifier::poppedUpInternally()
{
    return m_poppedUpInternally;
}

void DeviceNotifier::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    m_dialog->setMenuActionsAt(event->scenePos());

    PopupApplet::contextMenuEvent(event);
}

QList<QAction *> DeviceNotifier::contextualActions()
{
    return m_dialog->contextualActions();
}

}

#include "devicenotifier.moc"
