/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Chani Armitage <chanika@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "savercorona.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsLayout>
#include <QAction>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>

#include <KDebug>
#include <KDialog>
#include <KStandardDirs>
#include <KIcon>

#include <Plasma/Containment>

SaverCorona::SaverCorona(QObject *parent)
    : Plasma::Corona(parent)
{
    init();
}

void SaverCorona::init()
{
    QDesktopWidget *desktop = QApplication::desktop();
    m_numScreens = desktop->numScreens();
    if (m_numScreens > 1) {
        kDebug() << "maybe someone should implement multiple screen support";
    }

    bool unlocked = immutability() == Plasma::Mutable;

    //TODO delete the lock action and make our own
    //so that Corona won't fuck with it
    //remember to update our containment too

    //re-wire the lock action so we can check for a password
    QAction *lock = action("lock widgets");
    if (lock) {
        lock->disconnect(SIGNAL(triggered(bool)));
        connect(lock, SIGNAL(triggered(bool)), this, SLOT(toggleLock()));
        lock->setText(unlocked ? i18n("Lock Screen") : i18n("Configure Widgets"));
    }

    //the most important action ;)
    QAction *leave = new QAction(unlocked ? i18n("Leave Screensaver") : i18n("Unlock"), this);
    leave->setIcon(KIcon("system-lock-screen"));
    leave->setShortcut(QKeySequence("esc"));
    connect(leave, SIGNAL(triggered()), this, SLOT(unlockDesktop()));
    addAction("unlock desktop", leave);

    connect(this, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)), SLOT(updateActions(Plasma::ImmutabilityType)));
}

void SaverCorona::loadDefaultLayout()
{
    kDebug();
    QString defaultConfig = KStandardDirs::locate("appdata", "plasma-overlay-default-layoutrc");
    if (!defaultConfig.isEmpty()) {
        kDebug() << "attempting to load the default layout from:" << defaultConfig;
        loadLayout(defaultConfig);
        return;
    }

    QDesktopWidget *desktop = QApplication::desktop();

    // create a containment for the screen
    QRect g = desktop->screenGeometry(0);
    kDebug() << "     screen geometry is" << g;
    Plasma::Containment *c = addContainment("saverdesktop");
    c->setScreen(0);
    c->setFormFactor(Plasma::Planar);
    c->flushPendingConstraintsEvents();

    // a default clock
    Plasma::Applet *clock =  Plasma::Applet::load("clock", c->id() + 1);
    c->addApplet(clock, QPointF(KDialog::spacingHint(), KDialog::spacingHint()), true);
    clock->init();
    clock->flushPendingConstraintsEvents();

    emit containmentAdded(c);

}

int SaverCorona::numScreens() const
{
    return QApplication::desktop()->numScreens();
}

QRect SaverCorona::screenGeometry(int id) const
{
    return QApplication::desktop()->screenGeometry(id);
}

void SaverCorona::updateActions(Plasma::ImmutabilityType immutability)
{
    bool unlocked = immutability == Plasma::Mutable;
    QAction *a = action("lock widgets");
    if (a) {
        a->setText(unlocked ? i18n("Lock Screen") : i18n("Configure Widgets"));
    }
    a = action("unlock desktop");
    if (a) {
        a->setText(unlocked ? i18n("Leave Screensaver") : i18n("Unlock"));
    }
}

void SaverCorona::toggleLock()
{
    //require a password to unlock
    QDBusInterface lockprocess("org.kde.screenlocker", "/LockProcess",
            "org.kde.screenlocker.LockProcess", QDBusConnection::sessionBus(), this);
    if (immutability() == Plasma::Mutable) {
        setImmutability(Plasma::UserImmutable);
        lockprocess.call(QDBus::NoBlock, "startLock");
        kDebug() << "locking up!";
    } else if (immutability() == Plasma::UserImmutable) {
        QList<QVariant> args;
        args << i18n("Unlock widgets to configure them");
        bool sent = lockprocess.callWithCallback("checkPass", args, this, SLOT(unlock(QDBusMessage)), SLOT(dbusError(QDBusError)));
        kDebug() << sent;
    }
}

void SaverCorona::unlock(QDBusMessage reply)
{
    //assuming everything went as expected
    if (reply.arguments().isEmpty()) {
        kDebug() << "quit succeeded, I guess";
        return;
    }
    //else we were trying to unlock just the widgets
    bool success = reply.arguments().first().toBool();
    kDebug() << success;
    if (success) {
        setImmutability(Plasma::Mutable);
    }
}

void SaverCorona::dbusError(QDBusError error)
{
    kDebug() << error.errorString(error.type());
    kDebug() << "bailing out";
    //if it was the quit call and it failed, we shouldn't leave the user stuck in
    //plasma-overlay forever.
    qApp->quit();
}

void SaverCorona::unlockDesktop()
{
    QDBusInterface lockprocess("org.kde.screenlocker", "/LockProcess",
            "org.kde.screenlocker.LockProcess", QDBusConnection::sessionBus(), this);
    bool sent = (lockprocess.isValid() &&
            lockprocess.callWithCallback("quit", QList<QVariant>(), this, SLOT(unlock(QDBusMessage)), SLOT(dbusError(QDBusError))));
    //the unlock slot above is a dummy that should never be called.
    //somehow I need a valid reply slot or the error slot is never ever used.
    if (!sent) {
        //ah crud.
        kDebug() << "bailing out!";
        qApp->quit();
    }
}


#include "savercorona.moc"

