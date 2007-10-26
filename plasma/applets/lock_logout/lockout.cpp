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

#include <QPainter>
#include <QColor>
#include <QApplication>

#include <plasma/widgets/vboxlayout.h>
#include <plasma/widgets/widget.h>
#include <plasma/widgets/icon.h>

#include <KDialog>
#include <KRun>
#include <KWindowSystem>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include "workspace/kworkspace.h"
#include "ksmserver_interface.h"
#include "screensaver_interface.h"


using namespace Plasma;


LockOut::LockOut(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),m_dialog(0)
{
    setDrawStandardBackground(true);
    //setHasConfigurationInterface(true);
    m_layout = new Plasma::VBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);
    m_font = QApplication::font();
    m_font.setWeight(QFont::Bold);
	k_icon_lock = new KIcon("system-lock-screen");
	m_icon_lock=new Plasma::Icon(*k_icon_lock,"",this);
	k_icon_logout = new KIcon("system-log-out");
	m_icon_logout=new Plasma::Icon(*k_icon_logout,"",this);
	//setGeometry(QRectF(geometry().x(),geometry().y(),geometry().width()/4,geometry().height()/4));
	connect(m_icon_lock,SIGNAL(clicked()),this,SLOT(clickLock()));
	connect(m_icon_logout,SIGNAL(clicked()),this,SLOT(clickLogout()));
}

LockOut::~LockOut()
{
}

void LockOut::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);
    Q_UNUSED(p);
    Q_UNUSED(contentsRect);
    //m_icon_lock->setIconSize(m_icon_lock->size()/2);
	//m_icon_logout->setIconSize(m_icon_logout->size()/2);
	//m_layout->setGeometry(QRectF(m_layout->geometry().x(),m_layout->geometry().y(),m_layout->geometry().width(),m_icon_logout->size().rheight ()*2));
	m_layout->addItem(m_icon_lock);
	m_layout->addItem(m_icon_logout);
    kDebug()<<"LockOut:: geometry "<<geometry().width();
}

void LockOut::showConfigurationInterface()
{
}

void LockOut::configAccepted()
{
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
