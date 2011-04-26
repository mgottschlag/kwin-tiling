/*
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
#include "windows_handler.h"
#include "windows_helper/window_selection_list.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>

#ifdef Q_WS_X11
// Has to be behind all qt related stuff. Else the build fails miserably
#include <X11/X.h>
#endif

namespace KHotKeys {


ActivateWindowActionVisitor::~ActivateWindowActionVisitor()
    {}


ActivateWindowAction::ActivateWindowAction( ActionData* data_P,
    const Windowdef_list* window_P )
    : Action( data_P ), _window( window_P )
    {
    }


void ActivateWindowAction::accept(ActionVisitor& visitor)
    {
    if (ActivateWindowActionVisitor *v = dynamic_cast<ActivateWindowActionVisitor*>(&visitor))
        {
        v->visit(*this);
        }
    else
        {
        kDebug() << "Visitor error";
        }
    }


const Windowdef_list* ActivateWindowAction::window() const
    {
    return _window;
    }


ActivateWindowAction::~ActivateWindowAction()
    {
    delete _window;
    }


void ActivateWindowAction::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "ACTIVATE_WINDOW" ); // overwrites value set in base::cfg_write()
    KConfigGroup windowGroup( cfg_P.config(), cfg_P.name() + "Window" );
    window()->cfg_write( windowGroup );
    }


void ActivateWindowAction::execute()
    {
    if( window()->match( windows_handler->active_window()))
        return; // is already active
    WId win_id = windows_handler->find_window( window());
    if( win_id != None )
        windows_handler->activate_window( win_id );
    }


const QString ActivateWindowAction::description() const
    {
    return i18n( "Activate window: " ) + window()->comment();
    }


Action* ActivateWindowAction::copy( ActionData* data_P ) const
    {
    return new ActivateWindowAction( data_P, window()->copy());
    }


void ActivateWindowAction::set_window_list(Windowdef_list *list)
    {
    delete _window;
    _window = list;
    }

} // namespace KHotKeys

