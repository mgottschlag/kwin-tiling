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

    emitResult();
}

#include "powermanagementjob.moc"
