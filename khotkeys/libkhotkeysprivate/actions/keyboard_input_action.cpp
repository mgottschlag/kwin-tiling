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

#include "input.h"
#include "windows_handler.h"
#include "shortcuts_handler.h"
#include "windows_helper/window_selection_list.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>

// #include <X11/X.h>
#include <X11/Xlib.h>
#include <QtGui/QX11Info>

/*
*/

namespace KHotKeys {

KeyboardInputActionVisitor::~KeyboardInputActionVisitor()
    {}


KeyboardInputAction::KeyboardInputAction(
        ActionData* data_P,
        const QString& input_P,
        Windowdef_list* dest_window_P,
        bool active_window_P)
    :   Action( data_P ),
        _input( input_P ),
        _dest_window( dest_window_P )
    {
    if (dest_window_P) _destination = SpecificWindow;
    else if (active_window_P) _destination = ActiveWindow;
    else _destination = ActionWindow;

    if (!_dest_window) _dest_window = new Windowdef_list;
    }


KeyboardInputAction::~KeyboardInputAction()
    {
    delete _dest_window;
    }


void KeyboardInputAction::accept(ActionVisitor& visitor)
    {
    if (KeyboardInputActionVisitor *v = dynamic_cast<KeyboardInputActionVisitor*>(&visitor))
        {
        v->visit(*this);
        }
    else
        {
        kDebug() << "Visitor error";
        }
    }


const QString& KeyboardInputAction::input() const
    {
    return _input;
    }


void KeyboardInputAction::setDestination(const DestinationWindow & dest)
    {
    _destination = dest;
    }


void KeyboardInputAction::setDestinationWindowRules(Windowdef_list *list)
    {
    if (_dest_window)
        delete _dest_window;

    _dest_window = list;
    }


void KeyboardInputAction::setInput(const QString &input)
    {
    _input = input;
    }


const Windowdef_list* KeyboardInputAction::dest_window() const
    {
    return _dest_window;
    }


Windowdef_list* KeyboardInputAction::dest_window()
    {
    return _dest_window;
    }


KeyboardInputAction::DestinationWindow KeyboardInputAction::destination() const
    {
    return _destination;
    }


void KeyboardInputAction::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "KEYBOARD_INPUT" ); // overwrites value set in base::cfg_write()
    cfg_P.writeEntry( "Input", input());

    cfg_P.writeEntry( "DestinationWindow", int(_destination) );

    if( _destination == SpecificWindow && dest_window() != NULL )
        {
        KConfigGroup windowGroup( cfg_P.config(), cfg_P.name() + "DestinationWindow" );
        dest_window()->cfg_write( windowGroup );
        }
    }


void KeyboardInputAction::execute()
    {
    kDebug();

    if( input().isEmpty())
        {
        kDebug() << "Input is empty";
        return;
        }

    Window w = InputFocus;
    switch (_destination)
        {
        case SpecificWindow:
            Q_ASSERT(dest_window());
            w = windows_handler->find_window(dest_window());
            if (w == None) w = InputFocus;
            break;

        case ActionWindow:
            w = windows_handler->action_window();
            if (w == None) w = InputFocus;
            break;

        case ActiveWindow:
            // Nothing to do because w is InputFocus already
            break;

        default:
            Q_ASSERT(false);
        }

    int last_index = -1, start = 0;
    while(( last_index = input().indexOf( ':', last_index + 1 )) != -1 ) // find next ';'
        {
        QString key = input().mid( start, last_index - start ).trimmed();
        keyboard_handler->send_macro_key( key, w );
        start = last_index + 1;
        }
    // and the last one
    QString key = input().mid( start, input().length()).trimmed();
    keyboard_handler->send_macro_key( key, w ); // the rest
    XFlush( QX11Info::display());
    }


const QString KeyboardInputAction::description() const
    {
    QString tmp = input();
    tmp.replace( '\n', ' ' );
    tmp.truncate( 30 );
    return i18n( "Keyboard input: " ) + tmp;
    }


Action* KeyboardInputAction::copy( ActionData* data_P ) const
    {
    return new KeyboardInputAction(
            data_P,
            input(),
            dest_window() ? dest_window()->copy() : NULL,
            _destination == ActiveWindow);
    }

} // namespace KHotKeys

