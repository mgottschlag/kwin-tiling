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
#include <KStandardDirs>
#include <kdesktopfileactions.h>

//Plasma
#include <Plasma/ToolTipManager>

//solid
#include <solid/device.h>
#include <solid/storagedrive.h>
#include <solid/opticaldisc.h>
#include <solid/opticaldrive.h>

//Own
#include "notifierdialog.h"
#include "deviceitem.h"

using namespace Plasma;
using namespace Notifier;

static const char *DEFAULT_ICON_NAME = "device-notifier";

K_EXPORT_PLASMA_APPLET(devicenotifier, DeviceNotifier)

DeviceNotifier::DeviceNotifier(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_solidEngine(0),
      m_solidDeviceEngine(0),
      m_dialog(0),
      m_numberItems(0),
      m_itemsValidity(0),
      m_globalVisibility(false),
      m_checkHiddenDevices(true)
{
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(IgnoreAspectRatio);

    // let's initialize the widget
    setMinimumSize(graphicsWidget()->minimumSize());
}

DeviceNotifier::~DeviceNotifier()
{
    delete m_dialog;
}

void DeviceNotifier::init()
{
    KConfigGroup cg = config();
    m_numberItems = cg.readEntry("NumberItems", 4);
    m_itemsValidity = cg.readEntry("ItemsValidity", 5);
    m_showDevices = cg.readEntry("ShowDevices", (int)RemovableOnly);

    m_solidEngine = dataEngine("hotplug");
    m_solidDeviceEngine = dataEngine("soliddevice");

    connect(m_dialog, SIGNAL(deviceSelected()), this, SLOT(showPopup()));

    Plasma::ToolTipManager::self()->registerWidget(this);

    setPopupIcon(DEFAULT_ICON_NAME);

    //connect to engine when a device is plug
    connect(m_solidEngine, SIGNAL(sourceAdded(const QString&)),
            this, SLOT(onSourceAdded(const QString&)));
    connect(m_solidEngine, SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(onSourceRemoved(const QString&)));

    //feed the list with what is already reported by the engine
    fillPreviousDevices();

    setStatus(Plasma::PassiveStatus);
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
        if (device.as<Solid::StorageVolume>()->isIgnored()) {
            onSourceAdded(device.udi());
        }
    }
    foreach (const QString &udi, m_solidEngine->sources()) {
        onSourceAdded(udi);
    }

    m_fillingPreviousDevices = false;
}

void DeviceNotifier::changeNotifierIcon(const QString& name)
{
    setPopupIcon(name.isNull() ? DEFAULT_ICON_NAME : name);
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
}

void DeviceNotifier::dataUpdated(const QString &udi, Plasma::DataEngine::Data data)
{
    if (data.isEmpty()) {
        return;
    }

    //data from hotplug engine
    //kDebug() << data["udi"] << data["predicateFiles"].toStringList() << data["Device Types"].toStringList();
    QStringList predicateFiles = data["predicateFiles"].toStringList();
    if (!predicateFiles.isEmpty()) {
        //kDebug() << "adding" << data["udi"];
        int nb_actions = 0;
        QString lastActionLabel;
        foreach (const QString &desktop, data["predicateFiles"].toStringList()) {
            QString filePath = KStandardDirs::locate("data", "solid/actions/" + desktop);
            QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(filePath, true);
            nb_actions += services.size();
            m_dialog->insertAction(udi, desktop);
            if (services.size() > 0) {
                lastActionLabel = QString(services[0].text());
            }
        }

        m_dialog->setDeviceData(udi, data["text"], Qt::DisplayRole);

        if (nb_actions > 1) {
            QString s = i18np("1 action for this device",
                    "%1 actions for this device",
                    nb_actions);
            m_dialog->setDeviceData(udi, s, NotifierDialog::DescriptionRole);
        } else {
            m_dialog->setDeviceData(udi, lastActionLabel, NotifierDialog::DescriptionRole);
        }

    //data from soliddevice engine
    } else if (data["Device Types"].toStringList().contains("Storage Access")) {
        //kDebug() << "DeviceNotifier::solidDeviceEngine updated" << udi;

        //icon name
        m_dialog->setDeviceData(udi, data["Icon"], NotifierDialog::IconNameRole);
        m_dialog->setDeviceData(udi, KIcon(data["Icon"].toString(), NULL, data["Emblems"].toStringList()), Qt::DecorationRole);

        //kDebug() << "DeviceNotifier::solidDeviceEngine updated" << udi;
        if (data["Accessible"].toBool()) {
            m_dialog->setMounted(true, udi);
            m_dialog->setDeviceLeftAction(udi, DeviceItem::Umount);
        } else if (data["Device Types"].toStringList().contains("OpticalDisc")) {
            //set icon to unmounted device
            m_dialog->setMounted(false, udi);
            m_dialog->setDeviceLeftAction(udi, DeviceItem::Mount);
        } else {
            m_dialog->setMounted(false, udi);
            m_dialog->setDeviceLeftAction(udi, DeviceItem::Mount);
        }

        if (data["Ignored"].toBool()) {
            m_dialog->setDeviceData(udi, data["File Path"], Qt::DisplayRole);

            const QString desktop("test-predicate-openinwindow.desktop");
            QString filePath = KStandardDirs::locate("data", "solid/actions/" + desktop);
            QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(filePath, true);
            m_dialog->insertAction(udi, desktop);
            m_dialog->setDeviceData(udi, services[0].text(), NotifierDialog::DescriptionRole);

            m_dialog->setDeviceLeftAction(udi, DeviceItem::Nothing);
        }
    } else if (data["Device Types"].toStringList().contains("Storage Volume")) {
        if (data["Device Types"].toStringList().contains("OpticalDisc")) {
            m_dialog->setMounted(true, udi);
        }
    }
}

void DeviceNotifier::notifyDevice(const QString &udi)
{
    m_lastPlugged << udi;

    setStatus(Plasma::NeedsAttentionStatus);

    if (!m_fillingPreviousDevices) {
        showPopup(5000);
        changeNotifierIcon("preferences-desktop-notification");
        update();
        QTimer::singleShot(5000, m_dialog, SLOT(resetNotifierIcon()));
    } else {
        setStatus(Plasma::ActiveStatus);
    }
}

void DeviceNotifier::toolTipAboutToShow()
{
    Plasma::ToolTipContent toolTip;
    if (m_lastPlugged.isEmpty()) {
        toolTip.setSubText(i18n("No devices plugged in"));
        toolTip.setImage(KIcon("device-notifier"));
    } else {
        Solid::Device device(m_lastPlugged.last());
        toolTip.setSubText(i18n("Last plugged in device: %1", device.description()));
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
     if (m_showDevices == NonRemovableOnly) {
        Solid::Device device = Solid::Device(udi);
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
        m_dialog->insertDevice(udi);
        notifyDevice(udi);
        m_dialog->setDeviceData(udi, visibility, NotifierDialog::VisibilityRole);

        m_solidEngine->connectSource(udi, this);
        m_solidDeviceEngine->connectSource(udi, this);
    }

    if (!visibility) {
        m_hiddenDevices << udi;
    }
}

void DeviceNotifier::onSourceRemoved(const QString &udi)
{
    m_solidEngine->disconnectSource(udi, this);
    m_solidDeviceEngine->disconnectSource(udi, this);

    m_dialog->removeDevice(udi);
    removeLastDeviceNotification(udi);
    if (m_checkHiddenDevices) {
        m_hiddenDevices.removeAll(udi);
    } else {
        m_checkHiddenDevices = false;
    }
    if (m_numberItems == 0) {
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
    parent->addPage(configurationWidget, i18nc("General options page", "General"), icon());
    parent->setButtons( KDialog::Ok | KDialog::Cancel);
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    m_configurationUi.showDevices->setCurrentIndex(m_showDevices);
}

void DeviceNotifier::configAccepted()
{
    KConfigGroup cg = config();

    m_showDevices = m_configurationUi.showDevices->currentIndex();

    cg.writeEntry("ShowDevices", m_showDevices);

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

void DeviceNotifier::showErrorMessage(const QString &message)
{
    showMessage(KIcon("dialog-error"), message, ButtonOk);
}

bool DeviceNotifier::areThereHiddenDevices()
{
    return (m_hiddenDevices.count() > 0);
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

#include "devicenotifier.moc"
