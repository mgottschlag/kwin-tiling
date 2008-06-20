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
#include <plasma/widgets/icon.h>

// Qt
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QGraphicsLinearLayout>

// KDE
#include <KIcon>
#include <kworkspace/kworkspace.h>
#include <screensaver_interface.h>

#define MINSIZE 48

LockOut::LockOut(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    resize(MINSIZE*2,MINSIZE*4);
}

void LockOut::init()
{
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(0);

    setMinimumSize(0, 0);

    Plasma::Icon *icon_lock = new Plasma::Icon(KIcon("system-lock-screen"), "", this);
    m_layout->addItem(icon_lock);
    connect(icon_lock, SIGNAL(clicked()), this, SLOT(clickLock()));

    Plasma::Icon *icon_logout = new Plasma::Icon(KIcon("system-shutdown"), "", this);
    m_layout->addItem(icon_logout);
    connect(icon_logout, SIGNAL(clicked()), this, SLOT(clickLogout()));
}

LockOut::~LockOut()
{
}

void LockOut::checkLayout()
{
    Qt::Orientation direction;
    qreal ratioToKeep = 2;

    switch (formFactor()) {
        case Plasma::Vertical:
            if (geometry().width() >= MINSIZE) {
                direction = Qt::Horizontal;
                ratioToKeep = 2;
            } else {
                direction = Qt::Vertical;
                ratioToKeep = 0.5;
            }
            break;
        case Plasma::Horizontal:
            if (geometry().height() >= MINSIZE) {
                direction = Qt::Vertical;
                ratioToKeep = 0.5;
            } else {
                direction = Qt::Horizontal;
                ratioToKeep = 2;
            }
            break;
        default:
            direction = Qt::Vertical;
    }
    if (direction != m_layout->orientation()) {
        m_layout->setOrientation(direction);
    }

    if (formFactor() == Plasma::Horizontal) {
        //if we are on horizontal panel
        setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding));
        qreal wsize = size().height() * ratioToKeep;

        resize(QSizeF(wsize, size().height()));
        setMaximumSize(wsize, QWIDGETSIZE_MAX);
    } else if (formFactor() == Plasma::Vertical) {
        //if we are on vertical panel
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        qreal hsize = size().width() / ratioToKeep;
        resize(QSizeF(size().width(), hsize));
        setMaximumSize(QWIDGETSIZE_MAX, hsize);
    } else {
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX / ratioToKeep);
    }
}

void LockOut::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint ||
        constraints & Plasma::SizeConstraint) {
        checkLayout();
    }
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
    KWorkSpace::requestShutDown( KWorkSpace::ShutdownConfirmDefault,
                                 KWorkSpace::ShutdownTypeDefault,
                                 KWorkSpace::ShutdownModeDefault);
}


#include "lockout.moc"
