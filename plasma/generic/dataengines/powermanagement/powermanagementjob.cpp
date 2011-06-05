/*
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
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

#include "powermanagementjob.h"
#include "powermanagementengine.h"

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>

#include <KDebug>

void PowermanagementJob::start()
{
    QString operation = operationName();

    if (operation == "suspend") {
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
            QDBusMessage call = QDBusMessage::createMethodCall ("org.kde.Solid.PowerManagement",
                                                                "/org/kde/Solid/PowerManagement",
                                                                "org.kde.Solid.PowerManagement",
                                                                "suspendToRam");
            QDBusConnection::sessionBus().asyncCall (call);
        } else {
            kDebug() << "DBus org.kde.Solid.PowerMangement not available.";
        }
    }
    else if (operation == "hibernate") {
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
            QDBusMessage call = QDBusMessage::createMethodCall ("org.kde.Solid.PowerManagement",
                                                                "/org/kde/Solid/PowerManagement",
                                                                "org.kde.Solid.PowerManagement",
                                                                "suspendToDisk");
            QDBusConnection::sessionBus().asyncCall (call);
        } else {
            kDebug() << "DBus org.kde.Solid.PowerMangement not available.";
        }
    }
    else if (operation == "setBrightness") {
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
            QDBusMessage call = QDBusMessage::createMethodCall ("org.kde.Solid.PowerManagement",
                                                                "/org/kde/Solid/PowerManagement",
                                                                "org.kde.Solid.PowerManagement",
                                                                "setBrightness");
            int brightness = parameters().value("brightness").toInt();
            call.setArguments(QList<QVariant>() << QVariant::fromValue(brightness));
            QDBusConnection::sessionBus().asyncCall (call);
        } else {
            kDebug() << "DBus org.kde.Solid.PowerMangement not available.";
        }
    }
    else if (operation == "changeProfile") {
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
            QDBusMessage call = QDBusMessage::createMethodCall ("org.kde.Solid.PowerManagement",
                                                                "/org/kde/Solid/PowerManagement",
                                                                "org.kde.Solid.PowerManagement",
                                                                "loadProfile");

            Plasma::DataEngine::Data data = m_engine->query("PowerDevil");
            StringStringMap availableProfiles = data["Available profiles"].value< StringStringMap >();
            QString profile = availableProfiles.key(parameters().value("profile").toString());

            call.setArguments(QList<QVariant>() << QVariant::fromValue(profile));
            QDBusConnection::sessionBus().asyncCall (call);
        } else {
            kDebug() << "DBus org.kde.Solid.PowerMangement not available.";
        }
    }

    emitResult();
}

#include "powermanagementjob.moc"
