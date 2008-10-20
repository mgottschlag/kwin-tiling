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

//plasma
#include <plasma/dialog.h>
//use for desktop view
#include <plasma/widgets/icon.h>
#include <plasma/theme.h>

//solid
#include <solid/device.h>
#include <solid/storagedrive.h>


using namespace Plasma;
using namespace Notifier;

K_EXPORT_PLASMA_APPLET(devicenotifier, DeviceNotifier)

DeviceNotifier::DeviceNotifier(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_solidEngine(0),
      m_icon(0),
      m_iconName(""),
      m_layout(0),
      m_proxy(0),
      m_displayTime(0),
      m_numberItems(0),
      m_itemsValidity(0),
      m_timer(0)
{
    setHasConfigurationInterface(true);
    int iconSize = IconSize(KIconLoader::Desktop);
    resize(iconSize, iconSize);
}

DeviceNotifier::~DeviceNotifier()
{
    delete m_dialog;
    delete m_timer;
}

void DeviceNotifier::init()
{
    KConfigGroup cg = config();
    m_timer = new QTimer(this);
    m_displayTime = cg.readEntry("TimeDisplayed", 8);
    m_numberItems = cg.readEntry("NumberItems", 4);
    m_itemsValidity = cg.readEntry("ItemsValidity", 5);

    m_solidEngine = dataEngine("hotplug");
    m_solidDeviceEngine = dataEngine("soliddevice");

    //main layout, used both in desktop and panel mode
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    //feed the list with what is already reported by the engine

    //connect to engine when a device is plug
    connect(m_solidEngine, SIGNAL(sourceAdded(const QString&)),
            this, SLOT(onSourceAdded(const QString&)));
    connect(m_solidEngine, SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(onSourceRemoved(const QString&)));

    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerExpired()));
}

void DeviceNotifier::constraintsEvent(Plasma::Constraints constraints)
{
    bool isSizeConstrained = formFactor() != Plasma::Planar && formFactor() != Plasma::MediaCenter;

    if (constraints & FormFactorConstraint) {
        if (isSizeConstrained) {
            m_dialog = new NotifierDialog(this, NotifierDialog::PanelArea);
            // on the panel we don't want a background, and our proxy widget in Planar has one
            setBackgroundHints(NoBackground);
            if (m_proxy) {
                m_layout->removeItem(m_proxy);
                delete m_proxy;
                m_proxy = 0;
            }
            fillPreviousDevices();
            initSysTray();
        } else {
            delete m_icon;
            m_icon = 0;
            // on the panel we don't want a background, and our proxy widget in Planar has one
            setBackgroundHints(StandardBackground);
            m_dialog = new NotifierDialog(this, NotifierDialog::DesktopArea);
            m_proxy = new QGraphicsProxyWidget(this);
            m_proxy->setWidget(m_dialog->dialog());
            m_layout->addItem(m_proxy);
            fillPreviousDevices();
            resize(m_dialog->dialog()->size() + QSize(60,60));
            setMinimumSize(m_dialog->dialog()->minimumSizeHint());
        }

        connect(m_dialog, SIGNAL(itemSelected()), this, SLOT(onItemDialogClicked()));
    }

    if (m_icon && constraints & Plasma::SizeConstraint) {
        m_icon->resize(geometry().size());
    }
}

void DeviceNotifier::initSysTray()
{
    if (m_icon) {
        return;
    }

    m_icon = new Plasma::Icon(KIcon("device-notifier",NULL), QString(), this);
    m_iconName = QString("device-notifier");

    connect(m_icon, SIGNAL(clicked()), this, SLOT(onClickNotifier()));

    m_layout->addItem(m_icon);
    setAspectRatioMode(Plasma::ConstrainedSquare);
}

void DeviceNotifier::fillPreviousDevices()
{
    isNotificationEnabled = false;
    foreach (const QString &source, m_solidEngine->sources()) {
            Solid::Device *device = new Solid::Device(source);
            Solid::Device parentDevice = device->parent();
            Solid::StorageDrive *drive = parentDevice.as<Solid::StorageDrive>();
            if(drive && (drive->isHotpluggable() || drive->isRemovable())) {
                onSourceAdded(source);
            }
    }
    isNotificationEnabled = true;
}

void DeviceNotifier::changeNotifierIcon(const QString& name)
{
    if (m_icon && name.isNull()) {
        m_icon->setIcon(m_iconName);
    } else if (m_icon) {
        m_icon->setIcon(name);
    }
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

            if (m_icon && isNotificationEnabled) {
                m_dialog->dialog()->move(popupPosition(m_dialog->dialog()->sizeHint()));
                m_dialog->show();
                m_timer->start(m_displayTime*1000);
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
    m_solidEngine->connectSource(name, this);
    m_solidDeviceEngine->connectSource(name, this);
}

void DeviceNotifier::onSourceRemoved(const QString &name)
{
    m_solidEngine->disconnectSource(name, this);
    m_solidDeviceEngine->disconnectSource(name, this);
    if (m_icon && m_dialog->countDevices() == 0) {
        m_dialog->hide();
    }
    m_dialog->removeDevice(name);
}

void DeviceNotifier::onClickNotifier()
{
    if (m_dialog->dialog()->isVisible()) {
        m_dialog->hide();
    } else {
        m_dialog->dialog()->move(popupPosition(m_dialog->dialog()->sizeHint()));
        m_dialog->show();
    }
}

void DeviceNotifier::onItemDialogClicked()
{
    if (m_icon) {
        m_dialog->hide();
        m_timer->stop();
    }
}

void DeviceNotifier::onTimerExpired()
{
    if (m_icon) {
        m_timer->stop();
        m_dialog->hide();
    }
}

#include "devicenotifier.moc"
