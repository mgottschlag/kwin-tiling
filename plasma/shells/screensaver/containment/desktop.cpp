/*
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
*   Copyright 2008 by Chani Armitage <chanika@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
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

#include "desktop.h"

#include <QAction>
#include <QApplication>
//#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>

//#include <KAuthorized>
#include <KDebug>
//#include <KWindowSystem>
//#include <KActionCollection>
#include <KConfigDialog>

#include "plasma/corona.h"
#include "plasma/theme.h"
//#include "kworkspace/kworkspace.h"
//#include "knewstuff2/engine.h"

//TODO offer a way to change the theme and other such not-really-our-responsibility things

using namespace Plasma;

SaverDesktop::SaverDesktop(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_lockDesktopAction(0),
      m_appletBrowserAction(0)
{
}

SaverDesktop::~SaverDesktop()
{
}

void SaverDesktop::init()
{
    Containment::init();
    setHasConfigurationInterface(true);

    bool unlocked = immutability() == Mutable;
    //re-wire the lock action so we can check for a password
    QAction *lock = action("lock widgets");
    if (lock) {
        lock->disconnect(this);
        connect(lock, SIGNAL(triggered(bool)), this, SLOT(toggleLock()));
        lock->setText(unlocked ? i18n("Lock") : i18n("Unlock"));
    }

    //remove the desktop actions
    //FIXME do we really need to removeToolBoxTool?
    QAction *unwanted = action("zoom in");
    removeToolBoxTool(unwanted);
    delete unwanted;
    unwanted = action("zoom out");
    removeToolBoxTool(unwanted);
    delete unwanted;
    unwanted = action("add sibling containment");
    removeToolBoxTool(unwanted);
    delete unwanted;

    lock = new QAction(unlocked ? i18n("Quit") : i18n("Unlock and Quit"), this);
    lock->setIcon(KIcon("system-lock-screen"));
    //TODO kbd shortcut
    lock->setShortcutContext(Qt::WidgetShortcut);
    lock->setShortcut(QKeySequence("esc"));
    connect(lock, SIGNAL(triggered(bool)), this, SLOT(unlockDesktop()));
    addAction("unlock desktop", lock);
    addToolBoxTool(lock);

    QAction *a = action("configure");
    if (a) {
        a->setText(i18n("Settings"));
        addToolBoxTool(a);
    }

    //rearrange the toolboxtools
    a = action("add widgets");
    if (a) {
        removeToolBoxTool(a);
        addToolBoxTool(a);
    }
}

void SaverDesktop::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::ImmutableConstraint) {
        bool unlocked = immutability() == Mutable;
        QAction *a = action("lock widgets");
        if (a) {
            a->setText(unlocked ? i18n("Lock") : i18n("Unlock"));
        }
        a = action("unlock desktop");
        if (a) {
            a->setText(unlocked ? i18n("Quit") : i18n("Unlock and Quit"));
        }
    }
}

QList<QAction*> SaverDesktop::contextualActions()
{
    if (!m_appletBrowserAction) {
        m_appletBrowserAction = action("add widgets");
        m_lockDesktopAction = action("lock widgets");
    }
    QAction *config = action("configure");

    QList<QAction*> actions;
    actions.append(m_appletBrowserAction);
    if (config) {
        actions.append(config);
    }
    actions.append(m_lockDesktopAction);

    return actions;
}

void SaverDesktop::paintInterface(QPainter *painter,
                                    const QStyleOptionGraphicsItem *option,
                                    const QRect& contentsRect)
{
    //kDebug() << "paintInterface of background";

    //TODO learn how the new wallpaper plugins work
    painter->save();

    if (painter->worldMatrix() == QMatrix()) {
        // draw the background untransformed when possible;(saves lots of per-pixel-math)
        painter->resetTransform();
    }

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    QColor color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    //FIXME should I use the contents rect or the exposed rect?
    painter->fillRect(contentsRect, color);


    // for pixmaps we draw only the exposed part (untransformed since the
    // bitmapBackground already has the size of the viewport)
    //painter->drawPixmap(option->exposedRect, m_bitmapBackground, option->exposedRect);
    //kDebug() << "draw pixmap of background to" << option->exposedRect;

    // restore transformation and composition mode
    painter->restore();
}

void SaverDesktop::toggleLock()
{
    //requre a password to unlock
    if (!corona()) {
        return; //I'm lazy, I know this'll never happen
    }
    QDBusInterface lockprocess("org.kde.krunner_lock", "/LockProcess",
            "org.kde.krunner_lock.LockProcess", QDBusConnection::sessionBus(), this);
    if (corona()->immutability() == Mutable) {
        corona()->setImmutability(UserImmutable);
        lockprocess.call(QDBus::NoBlock, "startLock");
        kDebug() << "blaaaaaaaaaaaaaaaaa!!!!";
        emit locked();
    } else if (corona()->immutability() == UserImmutable) {
        QList<QVariant> args;
        args << i18n("Unlock Plasma Widgets");
        bool sent = lockprocess.callWithCallback("checkPass", args, this, SLOT(unlock(QDBusMessage)), SLOT(dbusError(QDBusError)));
        kDebug() << sent;
    }
}

void SaverDesktop::unlock(QDBusMessage reply)
{
    //assuming everything went as expected
    if (reply.arguments().isEmpty()) {
        kDebug() << "quit succeeded, I guess";
        return;
    }
    bool success = reply.arguments().first().toBool();
    kDebug() << success;
    if (success) {
        corona()->setImmutability(Mutable);
        emit unlocked(); //FIXME bad code
    }
}

void SaverDesktop::dbusError(QDBusError error)
{
    //Q_UNUSED(error)
    kDebug() << error.errorString(error.type());
    kDebug() << "bailing out";
    //ok, now i care. if it was the quit call and it failed, we shouldn't leave the user stuck in
    //plasma-overlay forever.
    qApp->quit();
}

void SaverDesktop::unlockDesktop()
{
    QDBusInterface lockprocess("org.kde.krunner_lock", "/LockProcess",
            "org.kde.krunner_lock.LockProcess", QDBusConnection::sessionBus(), this);
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

void SaverDesktop::createConfigurationInterface(KConfigDialog *parent)
{
    emit delegateConfigurationInterface(parent);
}

K_EXPORT_PLASMA_APPLET(saverdesktop, SaverDesktop)

#include "desktop.moc"
