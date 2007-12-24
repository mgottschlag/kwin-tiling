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

#include "lockout.h"

// Plasma
#include <plasma/layouts/vboxlayout.h>
#include <plasma/widgets/icon.h>

// Qt
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

// KDE
#include <KIcon>
#include <kworkspace/kworkspace.h>
#include <ksmserver_interface.h>
#include <screensaver_interface.h>


LockOut::LockOut(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    Plasma::VBoxLayout *layout = new Plasma::VBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    Plasma::Icon *icon_lock = new Plasma::Icon(KIcon("system-lock-screen"), "", this);
    icon_lock->setAlignment(Qt::AlignCenter);
    layout->addItem(icon_lock);
    connect(icon_lock, SIGNAL(clicked()), this, SLOT(clickLock()));

    Plasma::Icon *icon_logout = new Plasma::Icon(KIcon("system-log-out"), "", this);
    icon_logout->setAlignment(Qt::AlignCenter);
    layout->addItem(icon_logout);
    connect(icon_logout, SIGNAL(clicked()), this, SLOT(clickLogout()));
}

LockOut::~LockOut()
{
}

QSizeF LockOut::contentSizeHint() const
{
    QSizeF sizeHint = contentSize();
    switch (formFactor()) {
        case Plasma::Vertical:
            sizeHint.setHeight(sizeHint.width() * 2);
            break;
        case Plasma::Horizontal:
            sizeHint.setWidth(sizeHint.height() / 2);
            break;
        default:
            sizeHint = layout()->sizeHint();
            break;
    }

    return sizeHint;
}

Qt::Orientations LockOut::expandingDirections() const
{
    return 0;
}

void LockOut::clickLock()
{
    kDebug()<<"LockOut:: lock clicked ";
	
    QString interface("org.freedesktop.ScreenSaver");
    org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",
                                              QDBusConnection::sessionBus());
    if (screensaver.isValid()) {
        screensaver.Lock();
    }
}

void LockOut::clickLogout()
{
    kDebug()<<"LockOut:: logout clicked ";
    QString interface("org.kde.ksmserver");
    org::kde::KSMServerInterface smserver(interface, "/KSMServer",
                                          QDBusConnection::sessionBus());
    if (smserver.isValid()) {
        smserver.logout(KWorkSpace::ShutdownConfirmDefault,
                        KWorkSpace::ShutdownTypeDefault,
                        KWorkSpace::ShutdownModeDefault);
    }
}


#include "lockout.moc"
