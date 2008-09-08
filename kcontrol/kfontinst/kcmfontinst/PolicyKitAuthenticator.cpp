/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2008 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "PolicyKitAuthenticator.h"
#include <QtGui/QWidget>
#include <QtGui/QApplication>
#include <QtCore/QEventLoop>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <KDE/KGlobal>
#include <sys/types.h>
#include <unistd.h>
#include <set>

K_GLOBAL_STATIC(PolicyKitAuthenticator, theInstance)

struct PolicyKitAuthenticatorKey
{
    PolicyKitAuthenticatorKey(const QString &a=QString(), uint p=0)
        : action(a), pid(p) { }

    bool operator<(const PolicyKitAuthenticatorKey &o) const
    {
        return pid<o.pid || (pid==o.pid && action<o.action);
    }

    QString action;
    uint    pid;
};

class PolicyKitAuthenticatorPrivate
{
    public:

    PolicyKitAuthenticatorPrivate()
        : dbus("org.freedesktop.PolicyKit.AuthenticationAgent",
               "/",
               "org.freedesktop.PolicyKit.AuthenticationAgent",
               QDBusConnection::sessionBus())
    {
    }

    virtual ~PolicyKitAuthenticatorPrivate() { }

    std::set<PolicyKitAuthenticatorKey> sucesses;
    QDBusInterface                      dbus;
    bool                                replyStatus;
};

PolicyKitAuthenticator::PolicyKitAuthenticator()
                      : d_ptr(new PolicyKitAuthenticatorPrivate)
{
}

PolicyKitAuthenticator::~PolicyKitAuthenticator()
{
    delete d_ptr;
}

PolicyKitAuthenticator * PolicyKitAuthenticator::instance()
{
    return theInstance;
}

bool PolicyKitAuthenticator::authenticate(const QString &action, QWidget *widet, bool gui)
{
    return authenticate(action, widet ? widet->window()->winId() : 0, (uint)getpid(), gui);
}

bool PolicyKitAuthenticator::authenticate(const QString &action, uint winId, uint pid, bool gui)
{
    static const int constDBusTimeout=20*60*1000; // 20mins

    Q_D(PolicyKitAuthenticator);
    PolicyKitAuthenticatorKey key(action, pid);

    if(d->sucesses.end()!=d->sucesses.find(key))
        return true;

    QList<QVariant> args;

    args << action << winId << pid;

    QDBusMessage message=QDBusMessage::createMethodCall(d->dbus.service(), d->dbus.path(), d->dbus.interface(), "ObtainAuthorization");
    message.setArguments(args);

    if(gui)
    {
        QEventLoop eventLoop;

        d->dbus.connection().callWithCallback(message, this, SLOT(reply(QDBusMessage)), SLOT(error(QDBusError, QDBusMessage)), constDBusTimeout);
        connect(this, SIGNAL(quitLoop()), &eventLoop, SLOT(quit()));
        d->replyStatus=false;
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

        if(d->replyStatus)
        {
            d->sucesses.insert(key);
            return true;
        }
    }
    else if(isAuthenticated(d->dbus.connection().call(message, QDBus::Block, constDBusTimeout)))
    {
        d->sucesses.insert(key);
        return true;
    }

    return false;
}

void PolicyKitAuthenticator::reply(const QDBusMessage &msg)
{
    if(isAuthenticated(msg))
    {
        Q_D(PolicyKitAuthenticator);
        d->replyStatus=true;
    }
    emit quitLoop();
}

void PolicyKitAuthenticator::error(const QDBusError &, const QDBusMessage &)
{
    emit quitLoop();
}

bool PolicyKitAuthenticator::isAuthenticated(const QDBusMessage &msg)
{
    QList<QVariant> args=msg.arguments();

    return QDBusMessage::ReplyMessage==msg.type() && 1==args.count() &&
           QVariant::Bool==args[0].type() && args[0].toBool();
}

#include "PolicyKitAuthenticator.moc"
