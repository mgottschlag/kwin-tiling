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

#include "devicenotifier.h"
#include "notifierview.h"

#include <QPainter>
#include <QColor>
#include <QTreeView>
#include <QApplication>
#include <QStandardItemModel>
#include <QGraphicsLinearLayout>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <KConfigDialog>
#include <KMessageBox>
#include <KRun>
#include <KStandardDirs>
#include <KDesktopFile>
#include <kdesktopfileactions.h>
#include <KGlobalSettings>
#include <KColorScheme>

#include <plasma/svg.h>
#include <plasma/containment.h>
#include <plasma/dialog.h>
#include <plasma/animator.h>
#include <plasma/delegate.h>

//use for desktop view
#include <plasma/widgets/icon.h>
#include <plasma/theme.h>

#include <solid/device.h>
#include <solid/opticaldisc.h>
#include <solid/storageaccess.h>
#include <solid/opticaldrive.h>

using namespace Plasma;
using namespace Notifier;

DeviceNotifier::DeviceNotifier(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_solidEngine(0),
      m_hotplugModel(0),
      m_widget(0),
      m_icon(0),
      m_label(0),
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

void DeviceNotifier::init()
{
    KConfigGroup cg = config();
    m_timer = new QTimer();
    m_displayTime = cg.readEntry("TimeDisplayed", 8);
    m_numberItems = cg.readEntry("NumberItems", 4);
    m_itemsValidity = cg.readEntry("ItemsValidity", 5);

    //main layout, used both in desktop and panel mode
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);
    
    m_solidEngine = dataEngine("hotplug");
    m_solidDeviceEngine = dataEngine("soliddevice");
    m_widget = new Dialog();
    m_widget->setFocusPolicy(Qt::NoFocus);
    m_widget->setWindowFlags(Qt::X11BypassWindowManagerHint);   

    QVBoxLayout *l_layout = new QVBoxLayout(m_widget);
    l_layout->setSpacing(0);
    l_layout->setMargin(0);

    m_hotplugModel = new QStandardItemModel(this);

    m_label = new QLabel(m_widget);
    updateColors();
    QLabel *icon = new QLabel(m_widget);
    icon->setPixmap(KIcon("emblem-mounted").pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    
    QHBoxLayout *l_layout2 = new QHBoxLayout(m_widget);
    l_layout2->setSpacing(0);
    l_layout2->setMargin(0);

    l_layout2->addWidget(icon);
    l_layout2->addWidget(m_label);

    m_notifierView= new NotifierView(m_widget);
    m_notifierView->setModel(m_hotplugModel);
    Plasma::Delegate *delegate = new Delegate(this);
    //map the roles of m_hotplugModel into the standard Plasma::Delegate roles
    delegate->setRoleMapping(Plasma::Delegate::SubTitleRole, ActionRole);
    delegate->setRoleMapping(Plasma::Delegate::ColumnTypeRole, ScopeRole);
    delegate->setRoleMapping(Plasma::Delegate::SubTitleMandatoryRole, SubTitleMandatoryRole);
    m_notifierView->setItemDelegate(delegate);

    l_layout->addLayout(l_layout2);
    l_layout->addWidget(m_notifierView);
    m_widget->setLayout(l_layout);

    m_widget->adjustSize();
    
    //feed the list with what is already reported by the engine
    isNotificationEnabled = false;
    foreach (const QString &source, m_solidEngine->sources()) {
        onSourceAdded(source);
    }
    isNotificationEnabled = true;

    //connect to engine when a device is plug
    connect(m_solidEngine, SIGNAL(sourceAdded(const QString&)),
            this, SLOT(onSourceAdded(const QString&)));
    connect(m_solidEngine, SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(onSourceRemoved(const QString&)));

    connect(m_notifierView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotOnItemClicked(const QModelIndex&)));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerExpired()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));    // allows updating of colors automatically
}


void DeviceNotifier::initSysTray()
{
    if (m_icon) {
        return;
    }

    m_widget->setWindowFlags(Qt::X11BypassWindowManagerHint);

    //we display the icon corresponding to the computer
    QList<Solid::Device> list = Solid::Device::allDevices();

    if (list.size() > 0) {
        Solid::Device device=list[0];

        while (device.parent().isValid()) {
            device = device.parent();
        }
        m_icon = new Plasma::Icon(KIcon(device.icon()), QString(), this);
    } else {
        //default icon if problem
        m_icon = new Plasma::Icon(KIcon("computer"), QString(), this);
    }
    connect(m_icon, SIGNAL(clicked()), this, SLOT(onClickNotifier()));

    setAspectRatioMode(Plasma::ConstrainedSquare);

    m_layout->addItem(m_icon);
}

DeviceNotifier::~DeviceNotifier()
{
    delete m_widget;
    delete m_hotplugModel;
    delete m_timer;
}

void DeviceNotifier::constraintsEvent(Plasma::Constraints constraints)
{
    // on the panel we don't want a background, and our proxy widget in Planar has one
    setBackgroundHints(NoBackground);

    bool isSizeConstrained = formFactor() != Plasma::Planar && formFactor() != Plasma::MediaCenter;

    if (constraints & FormFactorConstraint) {
        if (isSizeConstrained) {

            if (m_proxy) {
                m_proxy->setWidget(0);
                delete m_proxy;
                m_proxy = 0;
            }

            initSysTray();
        } else {
            delete m_icon;
            m_icon = 0;

            m_widget->setWindowFlags(Qt::X11BypassWindowManagerHint);

            m_proxy = new QGraphicsProxyWidget(this);
            m_proxy->setWidget(m_widget);
            m_proxy->show();
            m_layout->addItem(m_proxy);
        }
    }

 }

void DeviceNotifier::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect)
{
    Applet::paintInterface(p,option,rect);
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

            QModelIndex index = indexForUdi(source);

            if (index.isValid()) {
                m_hotplugModel->setData(index, data["predicateFiles"], PredicateFilesRole);
                m_hotplugModel->setData(index, data["text"], Qt::DisplayRole);

                //icon name
                m_hotplugModel->setData(index, data["icon"], IconNameRole);
                //icon data
                m_hotplugModel->setData(index, KIcon(data["icon"].toString()), Qt::DecorationRole);

                if (nb_actions > 1) {
                    QString s = i18np("1 action for this device",
                            "%1 actions for this device",
                            nb_actions);
                    m_hotplugModel->setData(index, s, ActionRole);
                } else {
                    m_hotplugModel->setData(index,last_action_label, ActionRole);
                }
                if (m_icon && isNotificationEnabled) {
                    m_widget->move(popupPosition(m_widget->sizeHint()));
                    m_widget->show();
                    m_timer->start(m_displayTime*1000);
                }
            }
            //data from soliddevice engine
        } else {
            kDebug() << "DeviceNotifier::solidDeviceEngine updated" << source;
            QModelIndex index = indexForUdi(source);
            if (index.isValid()) {
                QModelIndex actionIndex = m_hotplugModel->index(index.row(), 1, QModelIndex());

                if (data["Device Types"].toStringList().contains("Storage Access")) {
                    if (data["Accessible"].toBool() == true) {
                        m_hotplugModel->setData(actionIndex, KIcon("media-eject"), Qt::DecorationRole);

                        //set icon to mounted device
                        QStringList overlays;
                        overlays << "emblem-mounted";
                        m_hotplugModel->setData(index, KIcon(index.data(IconNameRole).toString(), NULL, overlays), Qt::DecorationRole);
                    } else {
                        m_hotplugModel->setData(actionIndex, KIcon(), Qt::DecorationRole);

                        //set icon to unmounted device
                        m_hotplugModel->setData(index, KIcon(index.data(IconNameRole).toString()), Qt::DecorationRole);
                    }
                }
            }
            // actions specific for other types of devices will go here
        }
   }
}

void DeviceNotifier::onSourceAdded(const QString &name)
{
    kDebug() << "DeviceNotifier:: source added" << name;
    if (m_hotplugModel->rowCount() == m_numberItems && m_numberItems != 0) {
        QModelIndex index = m_hotplugModel->index(m_hotplugModel->rowCount() - 1, 0);
        QString itemUdi = m_hotplugModel->data(index, SolidUdiRole).toString();

        //disconnect sources and after (important) remove the row
        m_solidDeviceEngine->disconnectSource(itemUdi, this);
        m_solidEngine->disconnectSource(itemUdi, this);
        m_hotplugModel->removeRow(m_hotplugModel->rowCount() - 1);
    }
    QStandardItem *item = new QStandardItem();
    item->setData(name, SolidUdiRole);
    item->setData(Plasma::Delegate::MainColumn, ScopeRole);
    item->setData(false, SubTitleMandatoryRole);

    m_hotplugModel->insertRow(0, item);
    m_solidEngine->connectSource(name, this);

    m_solidDeviceEngine->connectSource(name, this);

    //sets the "action" column
    QStandardItem *actionItem = new QStandardItem();
    actionItem->setData(name, SolidUdiRole);
    actionItem->setData(Plasma::Delegate::SecondaryActionColumn, ScopeRole);

    m_hotplugModel->setItem(0, 1, actionItem);

}

void DeviceNotifier::onSourceRemoved(const QString &name)
{
    m_solidEngine->disconnectSource(name, this);
    m_solidDeviceEngine->disconnectSource(name, this);

    QModelIndex index = indexForUdi(name);
    if (index.isValid()) {
        m_hotplugModel->removeRow(index.row());
    }

    if (m_icon && m_hotplugModel->rowCount() == 0) {
        m_widget->hide();
    }
}

QModelIndex DeviceNotifier::indexForUdi(const QString &udi) const
{
    int rowCount = m_hotplugModel->rowCount();
    for (int i=0; i < rowCount; ++i) {
        QModelIndex index = m_hotplugModel->index(i, 0);
        QString itemUdi = m_hotplugModel->data(index, SolidUdiRole).toString();
        if (itemUdi == udi) {
            return index;
        }
    }
    //Is it possible to go here?no...
    kDebug() << "We should not be here!";
    return QModelIndex();
}

void DeviceNotifier::onClickNotifier()
{
    if (m_widget->isVisible()) {
        m_widget->hide();
    } else {
        m_widget->move(popupPosition(m_widget->sizeHint()));
        m_widget->show();
    }    
}

void DeviceNotifier::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget;
    ui.setupUi(widget);
    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(widget, parent->windowTitle(), icon());

    ui.spinTime->setValue(m_displayTime);
    ui.spinItems->setValue(m_numberItems);
    ui.spinTimeItems->setValue(m_itemsValidity);
}

void DeviceNotifier::configAccepted()
{
    kDebug() << "DeviceNotifier:: Config Accepted with params" << ui.spinTime->value() \
             << "," << ui.spinItems->value() \
             << "," << ui.spinTimeItems->value();
    m_displayTime = ui.spinTime->value();
    m_numberItems = ui.spinItems->value();
    m_itemsValidity = ui.spinTimeItems->value();
    KConfigGroup cg = config();
    cg.writeEntry("TimeDisplayed", m_displayTime);
    cg.writeEntry("NumberItems", m_numberItems);
    cg.writeEntry("ItemsValidity", m_itemsValidity);
    emit configNeedsSaving();
}

void DeviceNotifier::slotOnItemClicked(const QModelIndex &index)
{
    kDebug() << index;
    if (m_icon) {
        m_widget->hide();
        m_timer->stop();
    }

    QString udi = QString(m_hotplugModel->data(index, SolidUdiRole).toString());

    //unmount (probably in the future different action types for different device types)
    if (index.data(ScopeRole).toInt() == Plasma::Delegate::SecondaryActionColumn) {
        Solid::Device device(udi);

        if (device.is<Solid::OpticalDisc>()) {
            Solid::OpticalDrive *drive = device.parent().as<Solid::OpticalDrive>();
            connect(drive, SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)),
                    this, SLOT(storageEjectDone(Solid::ErrorType, QVariant)));
            drive->eject();
        } else if (device.is<Solid::StorageVolume>()) {
            Solid::StorageAccess *access = device.as<Solid::StorageAccess>();

            connect(access, SIGNAL(teardownDone(Solid::ErrorType, QVariant, const QString &)),
                this, SLOT(storageTeardownDone(Solid::ErrorType, QVariant)));
            access->teardown();
        }
    //open  (index.data(ScopeRole).toInt() == OpenAction)
    } else {
        QStringList desktop_files = m_hotplugModel->data(index, PredicateFilesRole).toStringList();

        kDebug() << "DeviceNotifier:: call Solid Ui Server with params :" << udi \
                << "," << desktop_files;
        QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
        QDBusReply<void> reply = soliduiserver.call("showActionsDialog", udi, desktop_files);
    }
}

void DeviceNotifier::onTimerExpired()
{
    if (m_icon) {
        m_timer->stop();
        m_widget->hide();
    }
}

void DeviceNotifier::storageTeardownDone(Solid::ErrorType error, QVariant errorData)
{
    if (error && errorData.isValid()) {
        KMessageBox::error(0, i18n("Cannot unmount the device.\nOne or more files on this device are open within an application."), QString());
    } else if (m_icon) {
        m_icon->setIcon(KIcon("dialog-ok"));
        QTimer::singleShot(2000, this, SLOT(resetIcon()));
        update();
    }

    //show the message only one time
    disconnect(sender(), SIGNAL(teardownDone(Solid::ErrorType, QVariant, const QString &)),
               this, SLOT(storageTeardownDone(Solid::ErrorType, QVariant)));
}

void DeviceNotifier::storageEjectDone(Solid::ErrorType error, QVariant errorData)
{
    if (error && errorData.isValid()) {
        KMessageBox::error(0, i18n("Cannot eject the disc.\nOne or more files on this disc are open within an application."), QString());
    } else if (m_icon) {
        m_icon->setIcon(KIcon("dialog-ok"));
        QTimer::singleShot(2000, this, SLOT(resetIcon()));
        update();
    }

    //show the message only one time
    disconnect(sender(), SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)),
               this, SLOT(storageEjectDone(Solid::ErrorType, QVariant)));
}

void DeviceNotifier::resetIcon()
{
    if (m_icon) {
        //we display the icon corresponding to the computer
        QList<Solid::Device> list = Solid::Device::allDevices();

        if (list.size() > 0) {
            Solid::Device device=list[0];

            while (device.parent().isValid()) {
                device = device.parent();
            }
            m_icon->setIcon(KIcon(device.icon()));
        } else {
            //default icon if problem
            m_icon->setIcon(KIcon("computer"));
        }
        update();
    }
}

void DeviceNotifier::updateColors()
{ 
    KColorScheme colorTheme = KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme());
    m_label->setText(i18n("<font color=\"%1\">Devices recently plugged in:</font>",
                            colorTheme.foreground(KColorScheme::NormalText).color().name()));
}

#include "devicenotifier.moc"
