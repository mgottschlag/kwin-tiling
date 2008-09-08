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

#include "PolicyKitMechanism.h"
#include <QtCore/QDebug>
#include <QtCore/QHash>
#include <QtCore/QSocketNotifier>
#include <QtCore/QCoreApplication>
#include <KDE/KGlobal>
#include <polkit-dbus/polkit-dbus.h>

K_GLOBAL_STATIC(PolicyKitMechanism, instancePtr)
//
// We have another 'theInstance' pointer, because during the
// init() of PolicyKitMechanismPrivate, the  polkitContextAddWatch
// callback is triggered. This function cant use
// PolicyKitMechanism::instance() - as this would cause another instance
// to be created, causing a recursion...
static PolicyKitMechanism *theInstance=0L;

static int polkitContextAddWatch(PolKitContext *, int fd)
{
    qDebug() << "PolicyKitMechanism - Adding watch " << fd;
    if(theInstance)
        theInstance->addWatch(fd);
    return 1;
}

static void polkitContextRemoveWatch(PolKitContext *, int fd)
{
    qDebug() << "PolicyKitMechanism - Removing watch " << fd;
    if(theInstance)
        theInstance->removeWatch(fd);
}

class PolicyKitMechanismPrivate
{
    public:

    PolicyKitMechanismPrivate()
    {
    }

    void init()
    {
        pkContext = polkit_context_new();

        if (0L==pkContext)
        {
            qDebug() << "PolicyKitMechanism - Could not get a new PolKitContext.";
            QCoreApplication::quit();
        }

        //TODO: handle name owner changed signal

        polkit_context_set_load_descriptions(pkContext);

        //TODO: polkit_context_set_config_changed

        polkit_context_set_io_watch_functions(pkContext, polkitContextAddWatch, polkitContextRemoveWatch);

        PolKitError * pkError;

        if (!polkit_context_init(pkContext, &pkError))
        {
            QString msg("PolicyKitMechanism -  Could not initialize PolKitContext");
            if (polkit_error_is_set(pkError))
                qDebug() << msg <<  ": " << polkit_error_get_error_message(pkError);
            else
                qDebug() << msg;
            QCoreApplication::quit();
        }

        polkit_context_ref(pkContext);

        DBusError      dbusError;
        DBusConnection *dbusCon;

        dbus_error_init(&dbusError);
        dbusCon = dbus_bus_get(DBUS_BUS_SYSTEM, &dbusError);

        pkTracker = polkit_tracker_new();
        polkit_tracker_set_system_bus_connection(pkTracker, dbusCon);
        polkit_tracker_init(pkTracker);
        polkit_tracker_ref(pkTracker);
    }

    virtual ~PolicyKitMechanismPrivate()
    {
        QHash<QString, PolKitAction *>::iterator it(pkActions.begin()),
                                                 end(pkActions.end());

        for(; it!=end; ++it)
            polkit_action_unref(it.value());

        QHash<int, QSocketNotifier *>::iterator wit(contextWatches.begin()),
                                                wend(contextWatches.end());

        for(; wit!=wend; ++wit)
            delete (*wit);

        polkit_context_unref(pkContext);
        polkit_tracker_unref(pkTracker);
    }

    PolKitContext                  *pkContext;
    PolKitTracker                  *pkTracker;
    QHash<QString, PolKitAction *> pkActions;
    QHash<int, QSocketNotifier *>  contextWatches;
};

PolicyKitMechanism * PolicyKitMechanism::instance()
{
    return instancePtr;
}

PolicyKitMechanism::PolicyKitMechanism()
                  : d_ptr(new PolicyKitMechanismPrivate())
{
    theInstance=this;
    d_ptr->init();
}

PolicyKitMechanism::~PolicyKitMechanism()
{
    delete d_ptr;
    theInstance=0L;
}

PolKitResult PolicyKitMechanism::canDoAction(const QString &action, unsigned int pid)
{
    Q_D(PolicyKitMechanism);

    qDebug() << "PolicyKitMechanism - canDoAction " << action << ' ' << pid;

    PolKitAction *pkAction=getAction(action);

    if(!pkAction)
    {
        qWarning() << "PolicyKitMechanism - Could not create action.";
        return POLKIT_RESULT_NO;
    }

    DBusError    dbusError;
    dbus_error_init(&dbusError);
    PolKitCaller *pkCaller=polkit_tracker_get_caller_from_pid(d->pkTracker, pid, &dbusError);
    if (!pkCaller)
    {
        qWarning() << "PolicyKitMechanism - Could not define caller from pid";
        return POLKIT_RESULT_NO;
    }
    PolKitResult pkResult;
    PolKitError *pkError = NULL;

    pkResult = polkit_context_is_caller_authorized(d->pkContext, pkAction, pkCaller, true, &pkError);
    polkit_caller_unref(pkCaller);

    if (polkit_error_is_set(pkError))
    {
        qWarning() << "PolicyKitMechanism - Could not determine if caller is authorized for this action.";
        return POLKIT_RESULT_NO;
    }

    return pkResult;
}

PolKitResult PolicyKitMechanism::canDoAction(const QString &action, const QString &dbusName)
{
    Q_D(PolicyKitMechanism);

    qDebug() << "PolicyKitMechanism - canDoAction " << action << ' ' << dbusName;

    PolKitAction *pkAction=getAction(action);

    if(!pkAction)
    {
        qWarning() << "PolicyKitMechanism - Could not create action.";
        return POLKIT_RESULT_NO;
    }

    DBusError    dbusError;
    dbus_error_init(&dbusError);
    PolKitCaller *pkCaller=polkit_tracker_get_caller_from_dbus_name(d->pkTracker, dbusName.toLatin1().constData(), &dbusError);
    if (!pkCaller)
    {
        qWarning() << "PolicyKitMechanism - Could not define caller from pid";
        return POLKIT_RESULT_NO;
    }
    PolKitResult pkResult;
    PolKitError *pkError = NULL;

    pkResult = polkit_context_is_caller_authorized(d->pkContext, pkAction, pkCaller, true, &pkError);
    polkit_caller_unref(pkCaller);

    if (polkit_error_is_set(pkError))
    {
        qWarning() << "PolicyKitMechanism - Could not determine if caller is authorized for this action.";
        return POLKIT_RESULT_NO;
    }

    return pkResult;
}

void PolicyKitMechanism::removeAction(const QString &action)
{
    Q_D(PolicyKitMechanism);

    QHash<QString, PolKitAction *>::iterator it(d->pkActions.find(action));

    if(d->pkActions.end()!=it)
    {
        polkit_action_unref(it.value());
        d->pkActions.erase(it);
    }
}

PolKitAction * PolicyKitMechanism::getAction(const QString &action)
{
    Q_D(PolicyKitMechanism);

    QHash<QString, PolKitAction *>::iterator it(d->pkActions.find(action));
    PolKitAction                             *pkAction=0L;

    if(d->pkActions.end()==it)
    {
        pkAction=polkit_action_new();
        if(!pkAction || !polkit_action_set_action_id(pkAction, action.toLatin1().constData()))
            pkAction=0L;
        else
            d->pkActions[action]=pkAction;
    }
    else
        pkAction=it.value();

    return pkAction;
}

void PolicyKitMechanism::contextWatchActivated(int fd)
{
    Q_D(PolicyKitMechanism);
    Q_ASSERT(d->contextWatches.contains(fd));
    qDebug() << "PolicyKitMechanism - Context watch activated";
    polkit_context_io_func(d->pkContext, fd);
}

void PolicyKitMechanism::addWatch(int fd)
{
    Q_D(PolicyKitMechanism);

    QSocketNotifier *notify = new QSocketNotifier(fd, QSocketNotifier::Read);
    d->contextWatches[fd] = notify;

    notify->connect(notify, SIGNAL(activated(int)), theInstance, SLOT(contextWatchActivated(int)));
}

void PolicyKitMechanism::removeWatch(int fd)
{
    Q_D(PolicyKitMechanism);

    Q_ASSERT(d->contextWatches.contains(fd));
    delete d->contextWatches[fd];
    d->contextWatches.remove(fd);
}

#include "PolicyKitMechanism.moc"
