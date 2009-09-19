/***************************************************************************
 *   Copyright (C) 2007 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>           *
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

//own
#include "devicenotifier.h"
#include "notifierview.h"
#include "notifierdialog.h"

//Qt
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QTimer>

//KDE
#include <KIcon>
#include <KConfigDialog>
#include <KStandardDirs>
#include <KDesktopFile>
#include <kdesktopfileactions.h>
#include <KIconLoader>

//plasma
#include <Plasma/Dialog>
//use for desktop view
#include <Plasma/IconWidget>
#include <Plasma/Theme>
#include <Plasma/Plasma>

//solid
#include <solid/device.h>
#include <solid/storagedrive.h>
#include <solid/opticaldisc.h>
#include <solid/opticaldrive.h>

using namespace Plasma;
using namespace Notifier;

K_EXPORT_PLASMA_APPLET(devicenotifier, DeviceNotifier)

DeviceNotifier::DeviceNotifier(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_solidEngine(0),
      m_solidDeviceEngine(0),
      m_icon(0),
      m_iconName(""),
      m_dialog(0),
      m_numberItems(0),
      m_itemsValidity(0),
      m_showAll(false),
      m_checkHiddenDevices(true)
{
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(IgnoreAspectRatio);

    // let's initialize the widget
    resize(widget()->sizeHint());
}

DeviceNotifier::~DeviceNotifier()
{
    delete m_icon;
    delete m_dialog;
    delete m_popupTimer;
}

void DeviceNotifier::init()
{
    KConfigGroup cg = config();
    m_numberItems = cg.readEntry("NumberItems", 4);
    m_itemsValidity = cg.readEntry("ItemsValidity", 5);
    m_hidePopupAfter = cg.readEntry("hidePopupAfter", 5);
    m_showOnlyRemovable = cg.readEntry("showOnlyRemovable", false);
    m_showPopupOnInsert = cg.readEntry("showPopupOnInsert", true);

    m_solidEngine = dataEngine("hotplug");
    m_solidDeviceEngine = dataEngine("soliddevice");

    m_icon = new Plasma::IconWidget(KIcon("device-notifier",NULL), QString());
    m_iconName = QString("device-notifier");
    m_popupTimer = new QTimer();

    m_popupTimer->setSingleShot(true);
    connect(m_popupTimer, SIGNAL(timeout()), this, SLOT(hidePopup()));
    connect(m_dialog, SIGNAL(deviceSelected()), m_popupTimer, SLOT(stop()));

    Plasma::ToolTipManager::self()->registerWidget(this);

    setPopupIcon(m_icon->icon());

    //feed the list with what is already reported by the engine

    //connect to engine when a device is plug
    connect(m_solidEngine, SIGNAL(sourceAdded(const QString&)),
            this, SLOT(onSourceAdded(const QString&)));
    connect(m_solidEngine, SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(onSourceRemoved(const QString&)));

    fillPreviousDevices();

    setStatus(Plasma::PassiveStatus);
}

QWidget *DeviceNotifier::widget()
{
    if (!m_dialog) {
        m_dialog = new NotifierDialog(this);
        connect(m_dialog, SIGNAL(actionSelected()), this, SLOT(hidePopup()));
    }

    return m_dialog->dialog();
}

void DeviceNotifier::fillPreviousDevices()
{
    m_fillingPreviousDevices = true;
    foreach (const QString &source, m_solidEngine->sources()) {
        if (m_showOnlyRemovable) {
            Solid::Device device = Solid::Device(source);
            Solid::Device parentDevice = device.parent();
            Solid::StorageDrive *drive = parentDevice.as<Solid::StorageDrive>();
            if (drive && (drive->isHotpluggable() || drive->isRemovable())) {
                onSourceAdded(source);
            }
        } else {
            onSourceAdded(source);
        }
    }
    m_fillingPreviousDevices = false;
}

void DeviceNotifier::changeNotifierIcon(const QString& name)
{
    if (m_icon && name.isNull()) {
        m_icon->setIcon(m_iconName);
    } else if (m_icon) {
        m_icon->setIcon(name);
    }

    setPopupIcon(m_icon->icon());
}

void DeviceNotifier::popupEvent(bool show)
{
    if (show) {
        Plasma::ToolTipManager::self()->clearContent(this);
    } else if (status() == Plasma::NeedsAttentionStatus) {
        setStatus(Plasma::ActiveStatus);
    } else {
        m_dialog->removeActions();
    }
}

void DeviceNotifier::dataUpdated(const QString &source, Plasma::DataEngine::Data data)
{
    if (data.isEmpty()) {
        return;
    }

    //kDebug() << data.keys();
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
            if (services.size() > 0) {
                lastActionLabel = QString(services[0].text());
            }
        }
        m_dialog->setDeviceData(source,data["predicateFiles"],NotifierDialog::PredicateFilesRole);
        m_dialog->setDeviceData(source,data["text"], Qt::DisplayRole);

        //icon name
        m_dialog->setDeviceData(source,data["icon"], NotifierDialog::IconNameRole);
        //icon data
        m_dialog->setDeviceData(source,KIcon(data["icon"].toString(), NULL, data["emblems"].toStringList()), Qt::DecorationRole);

        if (nb_actions > 1) {
            QString s = i18np("1 action for this device",
                    "%1 actions for this device",
                    nb_actions);
            m_dialog->setDeviceData(source, s, NotifierDialog::ActionRole);
        } else {
            m_dialog->setDeviceData(source, lastActionLabel, NotifierDialog::ActionRole);
        }

        //data from soliddevice engine
    } else if (data["Device Types"].toStringList().contains("Storage Access")) {
        //kDebug() << "DeviceNotifier::solidDeviceEngine updated" << source;
        m_dialog->setDeviceData(source, KIcon(m_dialog->getDeviceData(source,NotifierDialog::IconNameRole).toString(), NULL, data["Emblems"].toStringList()), Qt::DecorationRole);
        QList<QVariant> freeSpaceData;
        freeSpaceData << QVariant(0) << QVariant(0);
        m_dialog->setDeviceData(source, QVariant(freeSpaceData), NotifierDialog::DeviceFreeSpaceRole);

        //kDebug() << "DeviceNotifier::solidDeviceEngine updated" << source;
        if (data["Accessible"].toBool() == true) {
            m_dialog->setUnMount(true, source);

            if (data["Free Space"].isValid()) {
                QList<QVariant> freeSpaceData;
                freeSpaceData << data["Size"] << data["Free Space"];
                m_dialog->setDeviceData(source, QVariant(freeSpaceData), NotifierDialog::DeviceFreeSpaceRole);
            }
        } else if (data["Device Types"].toStringList().contains("OpticalDisc")) {
            //Unmounted optical drive
            m_dialog->setDeviceData(source, KIcon("media-eject"), Qt::DecorationRole);
            //set icon to unmounted device
            m_dialog->setUnMount(true, source);
        } else {
            m_dialog->setUnMount(false,source);
        }
    }
}

void DeviceNotifier::notifyDevice(const QString &name)
{
    m_lastPlugged << name;

    setStatus(Plasma::NeedsAttentionStatus);

    if (!m_fillingPreviousDevices) {
        if (m_showPopupOnInsert) {
            showPopup(m_hidePopupAfter * 1000);
        }
        changeNotifierIcon("preferences-desktop-notification");
        update();
        QTimer::singleShot(5000, this, SLOT(changeNotifierIcon()));
    } else {
        setStatus(Plasma::ActiveStatus);
    }
}

void DeviceNotifier::toolTipAboutToShow()
{
    Plasma::ToolTipContent toolTip;
    if (!m_lastPlugged.isEmpty()) {
        Solid::Device device(m_lastPlugged.last());
        toolTip.setSubText(i18n("Last plugged in device: %1", device.description()));
        toolTip.setImage(KIcon(device.icon()));
    } else {
        toolTip.setSubText(i18n("No devices plugged in"));
        toolTip.setImage(KIcon("device-notifier"));
    }

    Plasma::ToolTipManager::self()->setContent(this, toolTip);
}

void DeviceNotifier::toolTipHidden()
{
    Plasma::ToolTipManager::self()->clearContent(this);
}

void DeviceNotifier::removeLastDeviceNotification(const QString &name)
{
    m_lastPlugged.removeAll(name);
}

void DeviceNotifier::onSourceAdded(const QString &name)
{
    kDebug() << "DeviceNotifier:: source added" << name;
    if (m_dialog->countDevices() == m_numberItems && m_numberItems != 0) {
        QString itemUdi = m_dialog->getDeviceUdi(m_dialog->countDevices() - 1);
        //disconnect sources and after (important) remove the row
        m_solidDeviceEngine->disconnectSource(itemUdi, this);
        m_solidEngine->disconnectSource(itemUdi, this);

        m_dialog->removeDevice(m_dialog->countDevices() - 1);
    }

    KConfigGroup cg = config();

    bool visibility = cg.readEntry(name, true);

    if (visibility || m_showAll) {
        m_dialog->insertDevice(name);
        notifyDevice(name);
        m_dialog->setDeviceData(name, visibility, NotifierDialog::VisibilityRole);

        m_solidEngine->connectSource(name, this);
        m_solidDeviceEngine->connectSource(name, this);
    }
    if (!visibility) {
        m_hiddenDevices << name;
        m_dialog->addShowAllAction(true);
    }
}

void DeviceNotifier::onSourceRemoved(const QString &name)
{
    m_solidEngine->disconnectSource(name, this);
    m_solidDeviceEngine->disconnectSource(name, this);

    m_dialog->removeDevice(name);
    removeLastDeviceNotification(name);
    if (m_checkHiddenDevices) {
        m_hiddenDevices.removeAll(name);
        m_dialog->addShowAllAction(m_hiddenDevices.count() > 0);
    } else {
        m_checkHiddenDevices = true;
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
        QString name = m_lastPlugged.takeAt(0);
        onSourceRemoved(name);
    }

    fillPreviousDevices();
}

void DeviceNotifier::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *configurationWidget = new QWidget();
    m_configurationUi.setupUi(configurationWidget);
    parent->addPage(configurationWidget, i18n("General"), icon());
    parent->setButtons( KDialog::Ok | KDialog::Cancel);
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    m_configurationUi.hidePopupAfter->setValue(m_hidePopupAfter);
    m_configurationUi.hidePopupAfter->setSuffix(ki18np(" second", " seconds"));
    m_configurationUi.showPopupOnInsert->setChecked(m_showPopupOnInsert);
    m_configurationUi.showOnlyRemovable->setChecked(m_showOnlyRemovable);
}

void DeviceNotifier::configAccepted()
{
    KConfigGroup cg = config();

    m_hidePopupAfter = m_configurationUi.hidePopupAfter->value();
    m_showPopupOnInsert = m_configurationUi.showPopupOnInsert->isChecked();
    m_showOnlyRemovable = m_configurationUi.showOnlyRemovable->isChecked();

    cg.writeEntry("hidePopupAfter", m_hidePopupAfter);
    cg.writeEntry("showPopupOnInsert", m_showPopupOnInsert);
    cg.writeEntry("showOnlyRemovable", m_showOnlyRemovable);

    emit configNeedsSaving();

    resetDevices();
}

void DeviceNotifier::setItemShown(const QString &name, bool shown)
{
    m_dialog->setDeviceData(name, shown, NotifierDialog::VisibilityRole);
    m_checkHiddenDevices = false;
    if (!shown) {
        m_hiddenDevices << name;
        m_dialog->addShowAllAction(true);
    } else {
        m_hiddenDevices.removeAll(name);
        m_dialog->addShowAllAction(m_hiddenDevices.count() > 0);
    }
    if (!shown && !m_showAll) {
        onSourceRemoved(name);
    }


    KConfigGroup cg = config();
    cg.writeEntry(name, shown);
}

void DeviceNotifier::setAllItemsShown(bool shown)
{
    m_showAll = shown;
    resetDevices();
}

void DeviceNotifier::showErrorMessage(const QString &message)
{
    showMessage(KIcon("dialog-error"), message, ButtonOk);
}

#include "devicenotifier.moc"
