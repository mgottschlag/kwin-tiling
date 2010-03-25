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

#ifndef KDED_NETWORKSTATUS_H
#define KDED_NETWORKSTATUS_H

#include <QStringList>

#include <KDEDModule>

#include "network.h"

class NetworkStatusModule : public KDEDModule
{
Q_OBJECT
Q_CLASSINFO( "D-Bus Interface", "org.kde.Solid.Networking" )
public:
    NetworkStatusModule(QObject* parent, const QList<QVariant>&);
    ~NetworkStatusModule();
    // Client interface
public Q_SLOTS:
    Q_SCRIPTABLE int status();
    // Service interface
    Q_SCRIPTABLE QStringList networks();
    Q_SCRIPTABLE void setNetworkStatus( const QString & networkName, int status );
    Q_SCRIPTABLE void registerNetwork( const QString & networkName, int status, const QString & serviceName );
    Q_SCRIPTABLE void unregisterNetwork( const QString & networkName );
Q_SIGNALS:
    // Client interface
    /**
     * A status change occurred affecting the overall connectivity
     * @param status The new status
     */
    void statusChanged( uint status );
protected Q_SLOTS:
    void serviceUnregistered( const QString & name );
    void solidNetworkingStatusChanged( Solid::Networking::Status status );
protected:
    // set up embedded backend
    void init();
    // recalculate cached status
    void updateStatus();

private:
    class Private;
    Private *d;
};

#endif
// vim: sw=4 ts=4
