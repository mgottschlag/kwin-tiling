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

#ifndef NETWORKSTATUS_NETWORK_H
#define NETWORKSTATUS_NETWORK_H

#include <solid/networking.h>

class Network
{
public:
	Network( const QString & name, int status, const QString & serviceName );
	/**
	 * Update the status of this network
	 */
	void setStatus( Solid::Networking::Status status );
	/**
	 * The connection status of this network
	 */
	Solid::Networking::Status status() const;
	/**
	 * The name of this network
	 */
	QString name() const;
	void setName( const QString& name );
	/**
	 * Returns the service owning this network
	 */
	QString service() const;
	void setService( const QString& service );

private:
	Network( const Network & );
	QString m_name;
	Solid::Networking::Status m_status;
	QString m_service;
};

#endif
// vim: sw=4 ts=4
