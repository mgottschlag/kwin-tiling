/*
   Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "actions.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KDE/KUrl>
#include <KDE/KRun>

namespace KHotKeys {

Menuentry_action::Menuentry_action( Action_data* data_P, const QString& menuentry_P )
    : Command_url_action( data_P, menuentry_P )
    {
    }


Menuentry_action::Menuentry_action( KConfigGroup& cfg_P, Action_data* data_P )
    : Command_url_action( cfg_P, data_P )
    {
    }


void Menuentry_action::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "MENUENTRY" ); // overwrites value set in base::cfg_write()
    }


KService::Ptr Menuentry_action::service() const
    {
    if (!_service)
    {
        const_cast<Menuentry_action *>(this)->_service = KService::serviceByStorageId(command_url());
    }
    return _service;
    }


void Menuentry_action::set_service( KService::Ptr service )
    {
    Q_ASSERT( service );
    if (!service) return;
    _service = service;
    set_command_url(service->name());
    }


void Menuentry_action::execute()
    {
    (void) service();
    if (!_service)
        return;
    kDebug() << "Starting service " << _service->desktopEntryName();
    KRun::run( *_service, KUrl::List(), 0 );
    timeout.setSingleShot( true );
    timeout.start( 1000 ); // 1sec timeout
    }


Action* Menuentry_action::copy( Action_data* data_P ) const
    {
    return new Menuentry_action( data_P, command_url());
    }


const QString Menuentry_action::description() const
    {
    (void) service();
    return i18n( "Menuentry : " ) + (_service ? _service->name() : QString());
    }


} // namespace KHotKeys

