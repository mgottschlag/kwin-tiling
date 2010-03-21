/**************************************************************************
 *   Copyright 2009 by Jacopo De Simoi <wilderkde@gmail.com>               *
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
#include "devicewrapper.h"

//Qt
#include <QAction>
#include <QTimer>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

//Solid
#include <Solid/Device>
#include <Solid/StorageVolume>
#include <Solid/StorageAccess>
#include <Solid/OpticalDrive>
#include <Solid/OpticalDisc>

//KDE
#include <KIcon>
#include <KMessageBox>
#include <KStandardDirs>
#include <kdesktopfileactions.h>

//Plasma
#include <Plasma/DataEngine>

DeviceWrapper::DeviceWrapper(const QString& udi)
  : m_device(udi),
    m_isStorageAccess(false),
    m_isAccessible(false),
    m_isEncryptedContainer(false)
{
    m_udi = m_device.udi();
    return;
}

DeviceWrapper::~DeviceWrapper()
{
}

void DeviceWrapper::dataUpdated(const QString &source, Plasma::DataEngine::Data data)
{
    Q_UNUSED(source)

    if (data.isEmpty()) {
        return;
    }
    if (data["text"].isValid()) {
        m_actionIds.clear();
        foreach (const QString &desktop, data["predicateFiles"].toStringList()) {
            QString filePath = KStandardDirs::locate("data", "solid/actions/" + desktop);
            QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(filePath, true);

            foreach (KServiceAction serviceAction, services) {
                QString actionId = id()+'_'+desktop+'_'+serviceAction.name();
                m_actionIds << actionId;
                emit registerAction(actionId,  serviceAction.icon(), serviceAction.text(), desktop);
            }
        }
        m_isEncryptedContainer = data["isEncryptedContainer"].toBool();
    } else if (data["Device Types"].toStringList().contains("Storage Access")) {
        m_isStorageAccess = true;
        if (data["Accessible"].toBool() == true) {
            m_isAccessible = true;
        } else {
            m_isAccessible = false;
        }
    } else {
        m_isStorageAccess = false;
    }

    m_emblems = m_device.emblems();

    emit refreshMatch(m_udi);

    return;
}

QString DeviceWrapper::id() const {
     return m_udi;
}

Solid::Device DeviceWrapper::device() const {
    return m_device;
}

KIcon DeviceWrapper::icon() const {
    return KIcon(m_device.icon(), NULL, m_emblems);
}

bool DeviceWrapper::isStorageAccess() const {
    return m_isStorageAccess;
}

bool DeviceWrapper::isAccessible() const {
    return m_isAccessible;
}

QString DeviceWrapper::description() const {
    return m_device.description();
}

QString DeviceWrapper::defaultAction() const {

    QString actionString;

    if (m_isStorageAccess) {
        if (!m_isEncryptedContainer) {
            if (!m_isAccessible) {
                actionString = i18n("Mount the device");
            } else {
                actionString = i18n("Unmount the device");
            }
        } else {
            if (!m_isAccessible) {
                actionString = i18nc("Unlock the encrypted container; will ask for a password; partitions inside will appear as they had been plugged in","Unlock the container");
            } else {
                actionString = i18nc("Close the encrypted container; partitions inside will disappear as they had been unplugged", "Lock the container");
            }
        }
    } else {
            actionString = i18n("Eject medium");
    }
    return actionString;
}

void DeviceWrapper::runAction(QAction * action)
{
    if (action) {
        QString desktopAction = action->data().toString();
        if (!desktopAction.isEmpty()) {
            QStringList desktopFiles;
            desktopFiles.append(desktopAction);
            QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
            QDBusReply<void> reply = soliduiserver.call("showActionsDialog", id(), desktopFiles);
        }
    } else {
        if (m_device.is<Solid::StorageVolume>()) {
            Solid::StorageAccess *access = m_device.as<Solid::StorageAccess>();
            if (access) {
                if (access->isAccessible()) {
                    connect(access, SIGNAL(teardownDone(Solid::ErrorType, QVariant, const QString &)),this, SLOT(storageTeardownDone(Solid::ErrorType, QVariant)));
                    access->teardown();
                } else {
                    connect(access, SIGNAL(setupDone(Solid::ErrorType, QVariant, const QString &)),this, SLOT(storageSetupDone(Solid::ErrorType, QVariant)));
                    access->setup();
                }
                return;
            }
        }
        if (m_device.is<Solid::OpticalDisc>()) {
            Solid::OpticalDrive *drive = m_device.parent().as<Solid::OpticalDrive>();
            if (drive) {
                connect(drive, SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)), this, SLOT(storageEjectDone(Solid::ErrorType, QVariant)));
                drive->eject();
            }
        }
    }
}

QStringList DeviceWrapper::actionIds() const
{
    return m_actionIds;
}

void DeviceWrapper::storageTeardownDone(Solid::ErrorType error, QVariant errorData)
{
    if (error && errorData.isValid()) {
        QTimer::singleShot(0, this, SLOT(showTeardownError()));
    }

    //show the message only one time
    disconnect(sender(), SIGNAL(teardownDone(Solid::ErrorType, QVariant, const QString &)),
               this, SLOT(storageTeardownDone(Solid::ErrorType, QVariant)));
}

void DeviceWrapper::storageEjectDone(Solid::ErrorType error, QVariant errorData)
{
    if (error && errorData.isValid()) {
        QTimer::singleShot(0, this, SLOT(showEjectError()));
    }

    //show the message only one time
    disconnect(sender(), SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)),
               this, SLOT(storageEjectDone(Solid::ErrorType, QVariant)));
}

void DeviceWrapper::storageSetupDone(Solid::ErrorType error, QVariant errorData)
{
     if (error && errorData.isValid()) {
         QTimer::singleShot(0, this, SLOT(showSetupError()));
     }

    //show the message only one time
    disconnect(sender(), SIGNAL(setupDone(Solid::ErrorType, QVariant, const QString &)),
               this, SLOT(storageSetupDone(Solid::ErrorType, QVariant)));
}

void DeviceWrapper::showTeardownError()
{
    KMessageBox::error(0, i18n("Could not unmount the device.\nOne or more files on this device are open within an application."), QString());
}

void DeviceWrapper::showEjectError()
{
    KMessageBox::error(0, i18n("Could not eject the disc.\nOne or more files on this disc are open within an application."), QString());
}

void DeviceWrapper::showSetupError()
{
    KMessageBox::error(0, i18n("Could not mount the disc."), QString());
}


#include "devicewrapper.moc"
