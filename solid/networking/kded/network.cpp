/*  This file is part of kdebase/workspace/solid
    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library.  If not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this library
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "network.h"

#include <KDebug>

Network::Network( const QString & name, int status, const QString & serviceName )
	: m_name( name ), m_status( (Solid::Networking::Status)status ), m_service( serviceName )
{
}

void Network::setStatus( Solid::Networking::Status status )
{
	m_status = status;
}

Solid::Networking::Status Network::status()
{
	return m_status;
}

void Network::setName( const QString& name )
{
	m_name = name;
}

QString Network::name()
{
	return m_name;
}

QString Network::service()
{
	return m_service;
}

void Network::setService( const QString& service )
{
	m_service = service;
}

// vim: sw=4 ts=4
