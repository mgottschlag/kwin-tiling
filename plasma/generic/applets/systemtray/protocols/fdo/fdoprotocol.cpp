/***************************************************************************
 *   fdoprotocol.cpp                                                       *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#include "fdoprotocol.h"

#include "fdoselectionmanager.h"


namespace SystemTray
{

FdoProtocol::FdoProtocol(QObject *parent)
    : Protocol(parent),
      m_selectionManager(0)
{
}

FdoProtocol::~FdoProtocol()
{
    delete m_selectionManager;
}

void FdoProtocol::init()
{
    m_selectionManager = new FdoSelectionManager;
    connect(m_selectionManager, SIGNAL(taskCreated(SystemTray::Task*)),
            this, SIGNAL(taskCreated(SystemTray::Task*)));
    connect(m_selectionManager, SIGNAL(notificationCreated(SystemTray::Notification*)),
            this, SIGNAL(notificationCreated(SystemTray::Notification*)));
}

}

#include "fdoprotocol.moc"

