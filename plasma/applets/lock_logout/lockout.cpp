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
#include <Plasma/IconWidget>

// Qt
#include <QtGui/QWidget> // QWIDGETSIZE_MAX
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QGraphicsLinearLayout>

// KDE
#include <KIcon>
#ifndef Q_OS_WIN
#include <kworkspace/kworkspace.h>
#include <screensaver_interface.h>
#endif

// Windows
#ifdef Q_OS_WIN
#define _WIN32_WINNT 0x0500 // require NT 5.0 (win 2k pro)
#include <windows.h>
#endif // Q_OS_WIN

static const int MINBUTTONSIZE = 8;
static const int MARGINSIZE = 2;

LockOut::LockOut(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    resize(MINBUTTONSIZE, MINBUTTONSIZE * 2 + MARGINSIZE);
}

void LockOut::init()
{
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(0);

    Plasma::IconWidget *icon_lock = new Plasma::IconWidget(KIcon("system-lock-screen"), "", this);
    m_layout->addItem(icon_lock);
    connect(icon_lock, SIGNAL(clicked()), this, SLOT(clickLock()));
#ifndef Q_OS_WIN
    Plasma::IconWidget *icon_logout = new Plasma::IconWidget(KIcon("system-shutdown"), "", this);
    m_layout->addItem(icon_logout);
    connect(icon_logout, SIGNAL(clicked()), this, SLOT(clickLogout()));
#endif
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
            if (geometry().width() >= MINBUTTONSIZE * 2 + MARGINSIZE) {
                direction = Qt::Horizontal;
                ratioToKeep = 2;
            } else {
                direction = Qt::Vertical;
                ratioToKeep = 0.5;
            }
            break;
        case Plasma::Horizontal:
            if (geometry().height() >= MINBUTTONSIZE * 2 + MARGINSIZE) {
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

    if (direction == Qt::Horizontal) {
        setMinimumSize(MINBUTTONSIZE * 2 + MARGINSIZE, MINBUTTONSIZE);
    } else {
        setMinimumSize(MINBUTTONSIZE, MINBUTTONSIZE * 2 + MARGINSIZE);
    }

    if (direction != m_layout->orientation()) {
        m_layout->setOrientation(direction);
    }

    if (formFactor() == Plasma::Horizontal) {
        setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding));
        qreal wsize = size().height() * ratioToKeep;
        setMaximumSize(wsize, QWIDGETSIZE_MAX);
    } else if (formFactor() == Plasma::Vertical) {
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));
        qreal hsize = size().width() / ratioToKeep;
        setMaximumSize(QWIDGETSIZE_MAX, hsize);
    } else {
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
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

#ifndef Q_OS_WIN
    QString interface("org.freedesktop.ScreenSaver");
    org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",
                                              QDBusConnection::sessionBus());
    if (screensaver.isValid()) {
        screensaver.Lock();
    }
#else
    LockWorkStation();
#endif // !Q_OS_WIN
}

void LockOut::clickLogout()
{
    kDebug()<<"LockOut:: logout clicked ";
#ifndef Q_OS_WIN
    KWorkSpace::requestShutDown( KWorkSpace::ShutdownConfirmDefault,
                                 KWorkSpace::ShutdownTypeDefault,
                                 KWorkSpace::ShutdownModeDefault);
#endif
}


#include "lockout.moc"
