/*
    Copyright 2008 by Alexis Ménard <darktears31@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "notifierdialog.h"

//Qt
#include <QStandardItemModel>
#include <QModelIndex>
#include <QLabel>
#include <QVBoxLayout>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QHeaderView>
#include <QTimer>

//KDE
#include <KDebug>
#include <KColorScheme>
#include <KIcon>
#include <KIconLoader>
#include <KGlobalSettings>
#include <KMessageBox>

//plasma
#include <plasma/dialog.h>
#include <plasma/delegate.h>
#include <plasma/theme.h>

//solid
#include <solid/device.h>
#include <solid/opticaldisc.h>
#include <solid/storageaccess.h>
#include <solid/opticaldrive.h>
#include <solid/deviceinterface.h>

//own
#include "notifierview.h"
#include "devicenotifier.h"

using namespace Notifier;
using namespace Plasma;

NotifierDialog::NotifierDialog(DeviceNotifier * notifier,DialogArea area,QObject *parent) :
QObject(parent),
m_hotplugModel(0),
m_widget(0),
m_notifierView(0),
m_label(0),
m_notifier(notifier),
m_rootItem(0)
{
    m_hotplugModel = new QStandardItemModel(this);
    buildDialog(area);
    //make the invisible root for tree device
    m_rootItem = m_hotplugModel->invisibleRootItem();
}

NotifierDialog::~NotifierDialog()
{

}

QWidget * NotifierDialog::dialog()
{
    return m_widget;
}

void NotifierDialog::hide()
{
    m_widget->hide();
}

void NotifierDialog::show()
{
    m_widget->show();
}

QStandardItem* NotifierDialog::searchOrCreateDeviceCategory(const QString &categoryName)
{
    int rowCount = m_hotplugModel->rowCount();
    if(rowCount > 0)
    {
        int i = 0;
        while (i<rowCount)
        {
            QModelIndex index = m_hotplugModel->index(i, 0);
            QString itemUdi = m_hotplugModel->data(index, SolidUdiRole).toString();
            QStandardItem *currentItem = m_hotplugModel->itemFromIndex(index);
            if(currentItem)
            {
                QString currentItemName = currentItem->text();
                if (currentItemName == categoryName)
                {
                    //the category is find... we have to return the pointer on this category
                    return m_hotplugModel->itemFromIndex(index);
                }
            }
            i++;
        }
    }
    //insert a new category for device if not find and return the pointer
    QStandardItem *newCategory = new QStandardItem(QString(categoryName));
    m_hotplugModel->setData(newCategory->index(),categoryName,Qt::DisplayRole);
    m_rootItem->insertRow(0,newCategory);
    m_hotplugModel->setItem(0, 1, NULL);
    m_hotplugModel->setHeaderData(0, Qt::Horizontal,QString(""),Qt::EditRole);
    m_hotplugModel->setHeaderData(1, Qt::Horizontal,QString(""),Qt::EditRole);
    return newCategory;
}

void NotifierDialog::insertDevice(const QString &name)
{
    QStandardItem *item = new QStandardItem();
    item->setData(name, SolidUdiRole);
    item->setData(Plasma::Delegate::MainColumn, ScopeRole);
    item->setData(false, SubTitleMandatoryRole);

    QStandardItem *actionItem = new QStandardItem();
    actionItem->setData(name, SolidUdiRole);
    actionItem->setData(Plasma::Delegate::SecondaryActionColumn, ScopeRole);

    //search or create the category for inserted device
    QString udi = item->data(SolidUdiRole).toString();
    if(!udi.isNull()) {
        Solid::Device *device = new Solid::Device(udi);
        QString categoryOfInsertedDevice = getCategoryNameOfDevice(device);
        QStandardItem *currentCategory = searchOrCreateDeviceCategory(categoryOfInsertedDevice);
        if(currentCategory)
        {
            currentCategory->insertRow(0,item);
            currentCategory->setChild(0, 1, actionItem);
        }
    }

    m_notifierView->calculateRects();
}

void NotifierDialog::setUnMount(bool unmount, const QString &name) 
{
    QModelIndex index = indexForUdi(name);
    if (!index.isValid()) {
        return;
    }
    QStandardItem *currentItem = m_hotplugModel->itemFromIndex(index);
    QStandardItem *childAction = currentItem->parent()->child(currentItem->row(), 1);
    QVariant icon;
    if (unmount) {
        icon = KIcon("media-eject");
    }
    else {
        icon = KIcon();
    }
    m_hotplugModel->setData(childAction->index(),icon,Qt::DecorationRole);
}

void NotifierDialog::setDeviceData(const QString &name, QVariant data, int role)
{
    QModelIndex index = indexForUdi(name);
    if (!index.isValid()) {
        return;
    }
    if (role == Qt::DecorationRole) {
        QStandardItem *device = m_hotplugModel->itemFromIndex(index);
        QStandardItem *category = device->parent();
        QModelIndex parentIndex = category->index();
        if (!parentIndex.data(Qt::DecorationRole).isValid()) {
           m_hotplugModel->setData(parentIndex,data,role);
        }
    }
    m_hotplugModel->setData(index,data,role);
}

QVariant NotifierDialog::getDeviceData(const QString &name, int role)
{
    QModelIndex index = indexForUdi(name);
    if (!index.isValid()) {
        return QVariant();
    }
    else {
        return index.data(role);
    }
}

void NotifierDialog::removeDevice(const QString &name)
{
    QModelIndex index = indexForUdi(name);
    if (!index.isValid()) {
        return;
    }
    QStandardItem *device = m_hotplugModel->itemFromIndex(index);
    QStandardItem *category = device->parent();
    //removing device
    category->removeRow(device->row());
    //remove category if there's no devices into it
    if(!category->hasChildren())
    {
        m_rootItem->removeRow(category->row());
    }

    m_notifierView->calculateRects();
}

void NotifierDialog::removeDevice(int index)
{
    m_hotplugModel->removeRow(index);
    m_notifierView->calculateRects();
}

int NotifierDialog::countDevices()
{
    return m_hotplugModel->rowCount();
}

QString NotifierDialog::getDeviceUdi(int index)
{
    QModelIndex modelIndex = m_hotplugModel->index(index, 0);
    return m_hotplugModel->data(modelIndex, SolidUdiRole).toString();
}

void NotifierDialog::buildDialog(DialogArea area)
{
    if (DesktopArea != area) {
        m_widget = new Dialog();
        m_widget->setWindowFlags(Qt::Popup);
    }
    else {
        m_widget = new QWidget();
        m_widget->setBackgroundRole(QPalette::Shadow);
    }
    QVBoxLayout *l_layout = new QVBoxLayout(m_widget);
    l_layout->setSpacing(0);
    l_layout->setMargin(0);
    
    m_label = new QLabel(m_widget);
    updateColors();
    
    QLabel *icon = new QLabel(m_widget);
    icon->setPixmap(KIcon("emblem-mounted").pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium));

    QHBoxLayout *l_layout2 = new QHBoxLayout(m_widget);
    l_layout2->setSpacing(0);
    l_layout2->setMargin(0);

    l_layout2->addWidget(icon);
    l_layout2->addWidget(m_label);

    l_layout2->setAlignment(Qt::AlignCenter);


    m_notifierView = new NotifierView(m_widget);
    m_notifierView->setModel(m_hotplugModel);
    m_notifierView->setMinimumSize(150,300);
    m_notifierView->setFocusPolicy(Qt::NoFocus);
    
    Plasma::Delegate *delegate = new Delegate(this);
    //map the roles of m_hotplugModel into the standard Plasma::Delegate roles
    delegate->setRoleMapping(Plasma::Delegate::SubTitleRole, ActionRole);
    delegate->setRoleMapping(Plasma::Delegate::ColumnTypeRole, ScopeRole);
    delegate->setRoleMapping(Plasma::Delegate::SubTitleMandatoryRole, SubTitleMandatoryRole);
    m_notifierView->setItemDelegate(delegate);

    l_layout->addLayout(l_layout2);
    l_layout->addWidget(m_notifierView);
    m_widget->setLayout(l_layout);

    connect(m_notifierView, SIGNAL(clicked(const QModelIndex&)),this,SLOT(slotOnItemClicked(const QModelIndex&)));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));    // allows updating of colors automatically
}

void NotifierDialog::storageTeardownDone(Solid::ErrorType error, QVariant errorData)
{
    if (error && errorData.isValid()) {
        KMessageBox::error(0, i18n("Cannot unmount the device.\nOne or more files on this device are open within an application."), QString());
    }
    else {
        m_notifier->changeNotifierIcon("dialog-ok");
        m_notifier->update();
        QTimer::singleShot(2000, this, SLOT(resetNotifierIcon()));
    }

    //show the message only one time
    disconnect(sender(), SIGNAL(teardownDone(Solid::ErrorType, QVariant, const QString &)),
               this, SLOT(storageTeardownDone(Solid::ErrorType, QVariant)));
}

void NotifierDialog::storageEjectDone(Solid::ErrorType error, QVariant errorData)
{
    if (error && errorData.isValid()) {
        KMessageBox::error(0, i18n("Cannot eject the disc.\nOne or more files on this disc are open within an application."), QString());
    } else {
        m_notifier->changeNotifierIcon("dialog-ok");
        m_notifier->update();
        QTimer::singleShot(2000, this, SLOT(resetNotifierIcon()));
    }
    //show the message only one time
    disconnect(sender(), SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)),
               this, SLOT(storageEjectDone(Solid::ErrorType, QVariant)));
}

QModelIndex NotifierDialog::indexForUdi(const QString &udi) const
{
    int rowCount = m_hotplugModel->rowCount();
    for (int i=0; i < rowCount; ++i) {
        QModelIndex index = m_hotplugModel->index(i, 0);
        QStandardItem *currentItem = m_hotplugModel->itemFromIndex(index);
        for (int j=0; j < currentItem->rowCount(); ++j) {
          QStandardItem *childItem = currentItem->child(j, 0);
          QString itemUdi = m_hotplugModel->data(childItem->index(), SolidUdiRole).toString();
          if (itemUdi == udi) {
              return childItem->index();
          }
        }
    }
    //Is it possible to go here?no...
    kDebug() << "We should not be here!";
    return QModelIndex();
}

void NotifierDialog::slotOnItemClicked(const QModelIndex &index)
{
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
             if (access->isAccessible()) {
                connect(access, SIGNAL(teardownDone(Solid::ErrorType, QVariant, const QString &)),this, SLOT(storageTeardownDone(Solid::ErrorType, QVariant)));
                access->teardown();
             }
        }
    //open  (index.data(ScopeRole).toInt() == OpenAction)
    } else {
        QStringList desktop_files = m_hotplugModel->data(index, PredicateFilesRole).toStringList();

        kDebug() << "DeviceNotifier:: call Solid Ui Server with params :" << udi \
                << "," << desktop_files;
        QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
        QDBusReply<void> reply = soliduiserver.call("showActionsDialog", udi, desktop_files);
    }
    emit itemSelected();
}

QString NotifierDialog::getCategoryNameOfDevice(Solid::Device * device)
{
    int index = Solid::DeviceInterface::staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum typeEnum = Solid::DeviceInterface::staticMetaObject.enumerator(index);
    for (int i = typeEnum.keyCount() - 1 ; i > 0; i--)
    {
        Solid::DeviceInterface::Type type = (Solid::DeviceInterface::Type)typeEnum.value(i);
        const Solid::DeviceInterface *interface = device->asDeviceInterface(type);
        if (interface)
        {
            return Solid::DeviceInterface::typeToString(type);
        }
    }
    return 0;
}

void NotifierDialog::resetNotifierIcon()
{
    m_notifier->changeNotifierIcon();
    m_notifier->update();
}

void NotifierDialog::updateColors()
{
    KColorScheme colorTheme = KColorScheme(QPalette::Active, KColorScheme::View,Plasma::Theme::defaultTheme()->colorScheme());
    m_label->setText(i18n("<font color=\"%1\">Devices recently plugged in:</font>",colorTheme.foreground(KColorScheme::NormalText).color().name()));
}

#include "notifierdialog.moc"
