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
#include "action_data/action_data.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KDE/KMessageBox>
#include <KDE/KUrl>
#include <KDE/KRun>

namespace KHotKeys {

MenuEntryActionVisitor::~MenuEntryActionVisitor()
    {}


MenuEntryAction::MenuEntryAction( ActionData* data_P, const QString& menuentry_P )
    : CommandUrlAction( data_P, menuentry_P )
    {
    }


void MenuEntryAction::accept(ActionVisitor& visitor)
    {
    if (MenuEntryActionVisitor *v = dynamic_cast<MenuEntryActionVisitor*>(&visitor))
        {
        v->visit(*this);
        }
    else
        {
        kDebug() << "Visitor error";
        }
    }


void MenuEntryAction::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "MENUENTRY" ); // overwrites value set in base::cfg_write()
    }


KService::Ptr MenuEntryAction::service() const
    {
    if (!_service)
        {
        const_cast<MenuEntryAction *>(this)->_service = KService::serviceByStorageId(command_url());
        }
    return _service;
    }


void MenuEntryAction::set_service( KService::Ptr service )
    {
    _service = service;
    if (service)
        {
        set_command_url(service->storageId());
        }
    else
        {
        set_command_url(QString());
        }
    }


void MenuEntryAction::execute()
    {
    if (!service())
        {
        KMessageBox::sorry(
                NULL,
                i18n("No service configured."),
                i18n("Input Action: %1", data->comment()));
        return;
        }

    if (!KRun::run( *service(), KUrl::List(), 0 ))
        {
        KMessageBox::sorry(
                NULL,
                i18n("Failed to start service '%1'.", service()->name()),
                i18n("Input Action: %1", data->comment()));
        return;
        }
    }


Action* MenuEntryAction::copy( ActionData* data_P ) const
    {
    return new MenuEntryAction( data_P, command_url());
    }


const QString MenuEntryAction::description() const
    {
    return i18n( "Menu entry: " ) + (service() ? service()->comment() : QString());
    }


} // namespace KHotKeys

