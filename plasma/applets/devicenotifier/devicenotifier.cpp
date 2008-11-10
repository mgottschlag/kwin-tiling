/***************************************************************************
 *   Copyright (C) 2007 by Alexis Ménard <darktears31@gmail.com>           *
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

//solid
#include <solid/device.h>
#include <solid/storagedrive.h>


using namespace Plasma;
using namespace Notifier;

K_EXPORT_PLASMA_APPLET(devicenotifier, DeviceNotifier)

DeviceNotifier::DeviceNotifier(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_solidEngine(0),
      m_icon(0),
      m_iconName(""),
      m_dialog(0),
      m_numberItems(0),
      m_itemsValidity(0)
{
    setHasConfigurationInterface(true);
    setBackgroundHints(StandardBackground);

    // let's initialize the widget
    (void)widget();

    resize(widget()->sizeHint());
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

    m_solidEngine = dataEngine("hotplug");
    m_solidDeviceEngine = dataEngine("soliddevice");

    m_icon = new Plasma::IconWidget(KIcon("device-notifier",NULL), QString());
    m_iconName = QString("device-notifier");

    Plasma::ToolTipManager::self()->registerWidget(this);

    setPopupIcon(m_icon->icon());

    //feed the list with what is already reported by the engine

    //connect to engine when a device is plug
    connect(m_solidEngine, SIGNAL(sourceAdded(const QString&)),
            this, SLOT(onSourceAdded(const QString&)));
    connect(m_solidEngine, SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(onSourceRemoved(const QString&)));
}

void DeviceNotifier::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & StartupCompletedConstraint) {
        fillPreviousDevices();
    }
}

QWidget *DeviceNotifier::widget()
{
    if (!m_dialog) {
        m_dialog = new NotifierDialog(this);
    }

    return m_dialog->dialog();
}

void DeviceNotifier::fillPreviousDevices()
{
    m_fillingPreviousDevices = true;
    foreach (const QString &source, m_solidEngine->sources()) {
            Solid::Device device = Solid::Device(source);
            Solid::Device parentDevice = device.parent();
            Solid::StorageDrive *drive = parentDevice.as<Solid::StorageDrive>();
            if(drive && (drive->isHotpluggable() || drive->isRemovable())) {
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

void DeviceNotifier::dataUpdated(const QString &source, Plasma::DataEngine::Data data)
{
    if (data.size() > 0) {
        //data from hotplug engine
        if (!data["predicateFiles"].isNull()) {
            int nb_actions = 0;
            QString last_action_label;
            foreach (const QString &desktop, data["predicateFiles"].toStringList()) {
                QString filePath = KStandardDirs::locate("data", "solid/actions/" + desktop);
                QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(filePath, true);
                nb_actions += services.size();
                if (services.size() > 0) {
                    last_action_label = QString(services[0].text());
                }
            }
            m_dialog->setDeviceData(source,data["predicateFiles"],NotifierDialog::PredicateFilesRole);
            m_dialog->setDeviceData(source,data["text"], Qt::DisplayRole);

            //icon name
            m_dialog->setDeviceData(source,data["icon"], NotifierDialog::IconNameRole);
            //icon data
            m_dialog->setDeviceData(source,KIcon(data["icon"].toString()), Qt::DecorationRole);

            if (nb_actions > 1) {
                QString s = i18np("1 action for this device",
                                  "%1 actions for this device",
                                  nb_actions);
                m_dialog->setDeviceData(source, s, NotifierDialog::ActionRole);
            } else {
                m_dialog->setDeviceData(source,last_action_label, NotifierDialog::ActionRole);
            }

        //data from soliddevice engine
        } else {
            kDebug() << "DeviceNotifier::solidDeviceEngine updated" << source;
            if (data["Device Types"].toStringList().contains("Storage Access")) {
                if (data["Accessible"].toBool() == true) {
                    m_dialog->setUnMount(true,source);

                    //set icon to mounted device
                    QStringList overlays;
                    overlays << "emblem-mounted";
                    m_dialog->setDeviceData(source, KIcon(m_dialog->getDeviceData(source,NotifierDialog::IconNameRole).toString(), NULL, overlays), Qt::DecorationRole);
                } else if (data["Device Types"].toStringList().contains("OpticalDisc")) {
                    //Unmounted optical drive
                    m_dialog->setDeviceData(source, KIcon("media-eject"), Qt::DecorationRole);
                    //set icon to unmounted device
                    m_dialog->setUnMount(true,source);
                    m_dialog->setDeviceData(source, KIcon(m_dialog->getDeviceData(source,NotifierDialog::IconNameRole).toString()), Qt::DecorationRole);
                } else {
                    m_dialog->setUnMount(false,source);

                    //set icon to unmounted device
                    m_dialog->setDeviceData(source, KIcon(m_dialog->getDeviceData(source,NotifierDialog::IconNameRole).toString()), Qt::DecorationRole);
                }
            }
            // actions specific for other types of devices will go here
        }
   }
}

void DeviceNotifier::notifyDevice(const QString &name)
{
    m_lastPlugged<<name;

    if (!m_fillingPreviousDevices) {
        showPopup();
    }
}

void DeviceNotifier::toolTipAboutToShow()
{
    Plasma::ToolTipContent toolTip;
    if (!m_lastPlugged.isEmpty()) {
        Solid::Device *device = new Solid::Device(m_lastPlugged.last());

        toolTip.setSubText(i18n("Last plugged in device: %1", device->product()));
        toolTip.setImage(KIcon(device->icon()));

        delete device;
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

    m_dialog->insertDevice(name);
    notifyDevice(name);

    m_solidEngine->connectSource(name, this);
    m_solidDeviceEngine->connectSource(name, this);
}

void DeviceNotifier::onSourceRemoved(const QString &name)
{
    m_solidEngine->disconnectSource(name, this);
    m_solidDeviceEngine->disconnectSource(name, this);

    m_dialog->removeDevice(name);
    removeLastDeviceNotification(name);
}

#include "devicenotifier.moc"
