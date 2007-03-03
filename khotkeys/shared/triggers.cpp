/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _TRIGGERS_CPP_

#include "config-khotkeys.h"

#include "triggers.h"

#include <kglobalaccel.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kwinmodule.h>
#include <klocale.h>
#include <netwm_def.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "actions.h"
#include "action_data.h"
#include "input.h"
#include "gestures.h"
#include "windows.h"
#include <Q3PtrList>
#include "voices.h"

namespace KHotKeys
{

// Trigger

void Trigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    cfg_P.writeEntry( "Type", "ERROR" );
    }

Trigger* Trigger::create_cfg_read( KConfigGroup& cfg_P, Action_data* data_P )
    {
    QString type = cfg_P.readEntry( "Type" );
    if( type == "SHORTCUT" || type == "SINGLE_SHORTCUT" )
        return new Shortcut_trigger( cfg_P, data_P );
    if( type == "WINDOW" )
        return new Window_trigger( cfg_P, data_P );
    if( type == "GESTURE" )
        return new Gesture_trigger(cfg_P, data_P );
    if( type == "VOICE" )
        return new Voice_trigger (cfg_P, data_P );

    kWarning( 1217 ) << "Unknown Trigger type read from cfg file\n";
    return NULL;
    }

// Trigger_list

Trigger_list::Trigger_list( KConfigGroup& cfg_P, Action_data* data_P )
    : Q3PtrList< Trigger >()
    {
    setAutoDelete( true );
    _comment = cfg_P.readEntry( "Comment" );
    int cnt = cfg_P.readEntry( "TriggersCount", 0 );
    for( int i = 0;
         i < cnt;
         ++i )
        {
        KConfigGroup triggerConfig( cfg_P.config(), cfg_P.group() + QString::number( i ));
        Trigger* trigger = Trigger::create_cfg_read( triggerConfig, data_P );
        if( trigger )
            append( trigger );
        }
    }

void Trigger_list::cfg_write( KConfigGroup& cfg_P ) const
    {
    cfg_P.writeEntry( "Comment", comment());
    int i = 0;
    for( Iterator it( *this );
         it;
         ++it, ++i )
        {
        KConfigGroup triggerConfig( cfg_P.config(), cfg_P.group() + QString::number( i ));
        it.current()->cfg_write( triggerConfig );
        }
    cfg_P.writeEntry( "TriggersCount", i );
    }

Trigger_list* Trigger_list::copy( Action_data* data_P ) const
    {
    Trigger_list* ret = new Trigger_list( comment());
    for( Iterator it( *this );
         it;
         ++it )
        ret->append( it.current()->copy( data_P ));
    return ret;
    }

void Trigger_list::activate( bool activate_P )
    {
    for( Iterator it( *this );
         it;
         ++it )
        ( *it )->activate( activate_P );
    }

// Shortcut_trigger

Shortcut_trigger::Shortcut_trigger( Action_data* data_P, const KShortcut& shortcut_P )
    : Trigger( data_P ), _shortcut( shortcut_P )
    {
    keyboard_handler->insert_item( shortcut(), this );
    }

Shortcut_trigger::Shortcut_trigger( KConfigGroup& cfg_P, Action_data* data_P )
    : Trigger( cfg_P, data_P ), _shortcut( cfg_P.readEntry( "Key", 0 ))
    {
    keyboard_handler->insert_item( shortcut(), this );
    }

Shortcut_trigger::~Shortcut_trigger()
    {
    keyboard_handler->remove_item( shortcut(), this );
    }

void Shortcut_trigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Key", _shortcut.toString());
    cfg_P.writeEntry( "Type", "SHORTCUT" ); // overwrites value set in base::cfg_write()
    }

Shortcut_trigger* Shortcut_trigger::copy( Action_data* data_P ) const
    {
    kDebug( 1217 ) << "Shortcut_trigger::copy()" << endl;
    return new Shortcut_trigger( data_P ? data_P : data, shortcut());
    }

const QString Shortcut_trigger::description() const
    {
    // CHECKME vice mods
    return i18n( "Shortcut trigger: " ) + _shortcut.toString();
    // CHECKME i18n pro toString() ?
    }

bool Shortcut_trigger::handle_key( const KShortcut& shortcut_P )
    {
    if( shortcut() == shortcut_P )
        {
        windows_handler->set_action_window( 0 ); // use active window
        data->execute();
        return true;
        }
    return false;
    }

void Shortcut_trigger::activate( bool activate_P )
    {
    if( activate_P && khotkeys_active())
        keyboard_handler->activate_receiver( this );
    else
        keyboard_handler->deactivate_receiver( this );
    }

// Window_trigger

Window_trigger::Window_trigger( KConfigGroup& cfg_P, Action_data* data_P )
    : Trigger( cfg_P, data_P ), active( false )
    {
//    kDebug( 1217 ) << "Window_trigger" << endl;
    KConfigGroup windowsConfig( cfg_P.config(), cfg_P.group() + "Windows" );
    _windows = new Windowdef_list( windowsConfig );
    window_actions = cfg_P.readEntry( "WindowActions",0 );
    init();
    }

Window_trigger::~Window_trigger()
    {
//    kDebug( 1217 ) << "~Window_trigger :" << this << endl;
    disconnect( windows_handler, NULL, this, NULL );
    delete _windows;
    }

void Window_trigger::init()
    {
    kDebug( 1217 ) << "Window_trigger::init()" << endl;
    connect( windows_handler, SIGNAL( window_added( WId )), this, SLOT( window_added( WId )));
    connect( windows_handler, SIGNAL( window_removed( WId )), this, SLOT( window_removed( WId )));
    if( window_actions & ( WINDOW_ACTIVATES | WINDOW_DEACTIVATES /*| WINDOW_DISAPPEARS*/ ))
        connect( windows_handler, SIGNAL( active_window_changed( WId )),
            this, SLOT( active_window_changed( WId )));
    connect( windows_handler, SIGNAL( window_changed( WId, unsigned int )),
        this, SLOT( window_changed( WId, unsigned int )));
    }

void Window_trigger::activate( bool activate_P )
    {
    active = activate_P && khotkeys_active();
    }

void Window_trigger::window_added( WId window_P )
    {
    bool matches = windows()->match( Window_data( window_P ));
    existing_windows[ window_P ] = matches;
    kDebug( 1217 ) << "Window_trigger::w_added() : " << matches << endl;
    if( active && matches && ( window_actions & WINDOW_APPEARS ))
        {
        windows_handler->set_action_window( window_P );
        data->execute();
        }
    }

void Window_trigger::window_removed( WId window_P )
    {
    if( existing_windows.contains( window_P ))
        {
        bool matches = existing_windows[ window_P ];
        kDebug( 1217 ) << "Window_trigger::w_removed() : " << matches << endl;
        if( active && matches && ( window_actions & WINDOW_DISAPPEARS ))
            {
            windows_handler->set_action_window( window_P );
            data->execute();
            }
        existing_windows.remove( window_P );
        // CHECKME jenze co kdyz se window_removed zavola pred active_window_changed ?
        }
    else
        kDebug( 1217 ) << "Window_trigger::w_removed()" << endl;
    }

void Window_trigger::active_window_changed( WId window_P )
    {
    bool was_match = false;
    if( existing_windows.contains( last_active_window ))
        was_match = existing_windows[ last_active_window ];
    if( active && was_match && ( window_actions & WINDOW_DEACTIVATES ))
        {
        windows_handler->set_action_window( window_P );
        data->execute();
        }
/*    bool matches = windows()->match( Window_data( window_P ));
    existing_windows[ window_P ] = matches;*/
    bool matches = existing_windows.contains( window_P )
        ? existing_windows[ window_P ] : false;
    if( active && matches && ( window_actions & WINDOW_ACTIVATES ))
        {
        windows_handler->set_action_window( window_P );
        data->execute();
        }
    kDebug( 1217 ) << "Window_trigger::a_w_changed() : " << was_match << "|" << matches << endl;
    last_active_window = window_P;
    }

void Window_trigger::window_changed( WId window_P, unsigned int dirty_P )
    { // CHECKME snad nebude mit vliv, kdyz budu kaslat na properties_P a zkratka
      // kontrolovat kazdou zmenu
      // CHECKME kdyz se zmeni okno z match na non-match, asi to nebrat jako DISAPPEAR
    if( ! ( dirty_P & ( NET::WMName | NET::WMWindowType )))
        return;
    kDebug( 1217 ) << "Window_trigger::w_changed()" << endl;
    bool was_match = false;
    if( existing_windows.contains( window_P ))
        was_match = existing_windows[ window_P ];
    bool matches = windows()->match( Window_data( window_P ));
    existing_windows[ window_P ] = matches;
    if( active && matches && !was_match )
        if( window_actions & WINDOW_APPEARS )
            {
            windows_handler->set_action_window( window_P );
            data->execute();
            }
        else if( window_actions & WINDOW_ACTIVATES && window_P == windows_handler->active_window())
            {
            windows_handler->set_action_window( window_P );
            data->execute();
            }
    kDebug( 1217 ) << "Window_trigger::w_changed() : " << was_match << "|" << matches << endl;
    }

void Window_trigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    QString save_cfg_group = cfg_P.group();
    KConfigGroup windowsConfig( cfg_P.config(), cfg_P.group() + "Windows" );
    windows()->cfg_write( windowsConfig );
    cfg_P.writeEntry( "WindowActions", window_actions );
    cfg_P.writeEntry( "Type", "WINDOW" ); // overwrites value set in base::cfg_write()
    }

#ifdef HAVE_COVARIANT_RETURN    // stupid gcc, it doesn't even warn it can't do this
Window_trigger* Window_trigger::copy( Action_data* data_P ) const
#else
Trigger* Window_trigger::copy( Action_data* data_P ) const
#endif
    {
    Window_trigger* ret = new Window_trigger( data_P ? data_P : data, windows()->copy(),
        window_actions );
    ret->existing_windows = existing_windows; // CHECKME je tohle vazne treba ?
    return ret;
    }

const QString Window_trigger::description() const
    {
    return i18n( "Window trigger: " ) + windows()->comment();
    }

// Gesture_trigger

Gesture_trigger::Gesture_trigger( Action_data* data_P, const QString &gesturecode_P )
    : Trigger( data_P ), _gesturecode( gesturecode_P )
    {
    }

Gesture_trigger::Gesture_trigger( KConfigGroup& cfg_P, Action_data* data_P )
    : Trigger( cfg_P, data_P )
    {
    _gesturecode = cfg_P.readEntry( "Gesture" );
    }

Gesture_trigger::~Gesture_trigger()
    {
    gesture_handler->unregister_handler( this, SLOT( handle_gesture( const QString&, WId )));
    }

void Gesture_trigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Gesture", gesturecode());
    cfg_P.writeEntry( "Type", "GESTURE" ); // overwrites value set in base::cfg_write()
    }

Trigger* Gesture_trigger::copy( Action_data* data_P ) const
    {
    kDebug( 1217 ) << "Gesture_trigger::copy()" << endl;
    return new Gesture_trigger( data_P ? data_P : data, gesturecode());
    }

const QString Gesture_trigger::description() const
    {
    return i18n( "Gesture trigger: " ) + gesturecode();
    }

void Gesture_trigger::handle_gesture( const QString &gesture_P, WId window_P )
    {
    if( gesturecode() == gesture_P )
        {
        windows_handler->set_action_window( window_P );
        data->execute();
        }
    }

void Gesture_trigger::activate( bool activate_P )
    {
    if( activate_P )
        gesture_handler->register_handler( this, SLOT( handle_gesture( const QString&, WId )));
    else
        gesture_handler->unregister_handler( this, SLOT( handle_gesture( const QString&, WId )));
    }


// Voice_trigger

	Voice_trigger::Voice_trigger( Action_data* data_P, const QString &Voicecode_P, const VoiceSignature& signature1_P, const VoiceSignature& signature2_P )
	: Trigger( data_P ), _voicecode( Voicecode_P )
    {
		_voicesignature[0]=signature1_P;
		_voicesignature[1]=signature2_P;
    }

Voice_trigger::Voice_trigger( KConfigGroup& cfg_P, Action_data* data_P )
	: Trigger( cfg_P, data_P )
    {
    _voicecode = cfg_P.readEntry( "Name" );
	_voicesignature[0].read( cfg_P , "Signature1" );
	_voicesignature[1].read( cfg_P , "Signature2" );
    }

Voice_trigger::~Voice_trigger()
    {
    voice_handler->unregister_handler( this );
    }

void Voice_trigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Name", voicecode());
    cfg_P.writeEntry( "Type", "VOICE" ); // overwrites value set in base::cfg_write()
    _voicesignature[0].write( cfg_P , "Signature1" );
    _voicesignature[1].write( cfg_P , "Signature2" );
    }

Trigger* Voice_trigger::copy( Action_data* data_P ) const
    {
    kDebug( 1217 ) << "Voice_trigger::copy()" << endl;
	return new Voice_trigger( data_P ? data_P : data, voicecode(), voicesignature(1) , voicesignature(2) );
    }

const QString Voice_trigger::description() const
    {
    return i18n( "Voice trigger: " ) + voicecode();
    }

void Voice_trigger::handle_Voice(  )
    {
        windows_handler->set_action_window( 0 ); // use active window
        data->execute();

    }

void Voice_trigger::activate( bool activate_P )
    {
    if( activate_P && khotkeys_active())
		voice_handler->register_handler( this );
    else
        voice_handler->unregister_handler( this );
    }


} // namespace KHotKeys

#include "triggers.moc"
