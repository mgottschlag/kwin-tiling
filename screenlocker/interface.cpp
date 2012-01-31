/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "interface.h"
#include "ksldapp.h"
#include "screensaveradaptor.h"
#include "kscreensaveradaptor.h"
// KDE
#include <KDE/KDebug>
#include <KDE/KIdleTime>
#include <KDE/KProcess>
#include <KDE/KRandom>
// Qt
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusServiceWatcher>

namespace ScreenLocker
{
Interface::Interface(KSldApp *parent)
    : QObject(parent)
    , m_daemon(parent)
    , m_serviceWatcher(new QDBusServiceWatcher(this))
    , m_next_cookie(0)
{
    (void) new ScreenSaverAdaptor( this );
    QDBusConnection::sessionBus().registerService(QLatin1String("org.freedesktop.ScreenSaver")) ;
    (void) new KScreenSaverAdaptor( this );
    QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.screensaver"));
    QDBusConnection::sessionBus().registerObject(QLatin1String("/ScreenSaver"), this);
    connect(m_daemon, SIGNAL(locked()), SLOT(slotLocked()));
    connect(m_daemon, SIGNAL(unlocked()), SLOT(slotUnlocked()));

    m_serviceWatcher->setConnection(QDBusConnection::sessionBus());
    m_serviceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(m_serviceWatcher, SIGNAL(serviceUnregistered(QString)), SLOT(serviceUnregistered(QString)));

    // Also receive updates triggered through the DBus (from powerdevil) see Bug #177123
    QStringList modules;
    QDBusInterface kdedInterface(QLatin1String( "org.kde.kded" ), QLatin1String( "/kded" ), QLatin1String( "org.kde.kded" ));
    QDBusReply<QStringList> reply = kdedInterface.call(QLatin1String( "loadedModules" ));

    if (!reply.isValid()) {
        return;
    }

    modules = reply.value();

    if (modules.contains(QLatin1String( "powerdevil" ))) {
      if (!QDBusConnection::sessionBus().connect(QLatin1String( "org.kde.kded" ), QLatin1String( "/modules/powerdevil" ), QLatin1String( "org.kde.PowerDevil" ),
                          QLatin1String( "DPMSconfigUpdated" ), this, SLOT(configure()))) {
            kDebug() << "error!";
        }
    }
    // I make it a really random number to avoid
    // some assumptions in clients, but just increase
    // while gnome-ss creates a random number every time
    m_next_cookie = KRandom::random() % 20000;
}

Interface::~Interface()
{
}

bool Interface::GetActive()
{
    return m_daemon->isLocked();
}

uint Interface::GetActiveTime()
{
    return m_daemon->activeTime();
}

uint Interface::GetSessionIdleTime()
{
    return KIdleTime::instance()->idleTime();
}

void Interface::Lock()
{
    m_daemon->lock();
}

bool Interface::SetActive (bool state)
{
    // TODO: what should the return value be?
    if (state) {
        Lock();
        return true;
    }
    // set inactive is ignored
    return false;
}

uint Interface::Inhibit(const QString &application_name, const QString &reason_for_inhibit)
{
    Q_UNUSED(application_name)
    Q_UNUSED(reason_for_inhibit)
    InhibitRequest sr;
    sr.cookie = m_next_cookie++;
    sr.dbusid = message().service();
    m_requests.append(sr);
    m_serviceWatcher->addWatchedService(sr.dbusid);
    KSldApp::self()->inhibit();
    return sr.cookie;
}

void Interface::UnInhibit(uint cookie)
{
    QMutableListIterator<InhibitRequest> it(m_requests);
    while (it.hasNext()) {
        if (it.next().cookie == cookie) {
            it.remove();
            KSldApp::self()->uninhibit();
            break;
        }
    }
}

void Interface::serviceUnregistered(const QString &name)
{
    m_serviceWatcher->removeWatchedService(name);
    QListIterator<InhibitRequest> it(m_requests);
    while (it.hasNext()) {
        const InhibitRequest &r = it.next();
        if (r.dbusid == name) {
            UnInhibit(r.cookie);
        }
    }
}

void Interface::SimulateUserActivity()
{
    // TODO: implement me when we support user activity interaction or the autolock
}

uint Interface::Throttle(const QString &application_name, const QString &reason_for_inhibit)
{
    Q_UNUSED(application_name)
    Q_UNUSED(reason_for_inhibit)
    // TODO: implement me
    return 0;
}

void Interface::UnThrottle(uint cookie)
{
    Q_UNUSED(cookie)
    // TODO: implement me
}

void Interface::slotLocked()
{
    emit ActiveChanged(true);
}

void Interface::slotUnlocked()
{
    emit ActiveChanged(false);
}

void Interface::configure()
{
    m_daemon->configure();
}

void Interface::setupPlasma()
{
    KProcess *plasmaProc = new KProcess;
    plasmaProc->setProgram(QLatin1String( "plasma-overlay" ));
    *plasmaProc << QLatin1String( "--setup" );

    //make sure it goes away when it's done (and not before)
    connect(plasmaProc, SIGNAL(finished(int,QProcess::ExitStatus)), plasmaProc, SLOT(deleteLater()));

    plasmaProc->start();
}

void Interface::saverLockReady()
{
    // unused
}

} // namespace

#include "interface.moc"
