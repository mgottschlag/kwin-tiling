/* Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "stand_alone.h"

#include <QtDBus/QtDBus>

#include "KDE/KDebug"
#include "KDE/KLocale"
#include "KDE/KToolInvocation"

namespace KHotKeys { namespace StandAloneDaemon {


bool isRunning()
    {
    sleep(1);
    return QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.khotkeys" );
    }


bool reload()
    {
    QDBusInterface iface( "org.kde.khotkeys", "/", QString() );

    if(!iface.isValid())
        {
        QDBusError err = iface.lastError();
        if (err.isValid())
            {
            kError() << err.name() << ":" << err.message();
            }
        return start();
        }
    else
        {
        QDBusMessage reply = iface.call("reread_configuration");
        QDBusError err = iface.lastError();
        if (err.isValid())
            {
            kError() << "Failed to reread configuration - " << err.name() << ":" << err.message();
            return false;
            }
        }

    return true;
    }


bool start()
    {
    int rc = KToolInvocation::kdeinitExec( "khotkeys" );
    if ( rc == 0 )
        {
        // Sleep a second so kdeinit can finish the starting
        sleep(1);
        return true;
        }
    else
        {
        kError() << "Unable to start server org.kde.khotkeys (standalone)";
        return false;
        }
    }


bool stop()
    {
    if (!isRunning())
        {
        return true;
        }

    QDBusInterface kdedInterface( "org.kde.khotkeys", "/MainApplication", QString() );
    QDBusReply<void> reply = kdedInterface.call( "quit" );
    QDBusError err = reply.error();

    if (err.isValid())
        {
        kError() << "Error when stopping khotkeys standalone daemon [" << err.name() << "]:" << err.message();
        return false;
        }

    Q_ASSERT( reply.isValid() );
    return true;
    }

}} // namespace KHotKeys::StandAloneDaemon

