/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _INPUT_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "input.h"

#include <assert.h>
#include <qwidget.h>

#include <kglobalaccel.h>
#include <kdebug.h>
#include <kapp.h>
#include <kdeversion.h>
#include <qtimer.h>
#include <kkeynative.h>

#include <khotkeysglobal.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "windows.h"

namespace KHotKeys
{

// Kbd

Kbd::Kbd( bool grabbing_enabled_P, QObject* parent_P )
    : QObject( parent_P )
    {
    assert( keyboard_handler == NULL );
    keyboard_handler = this;
    kga = new KGlobalAccel( NULL );
    kga->setEnabled( grabbing_enabled_P );
    }
    
Kbd::~Kbd()
    {
    keyboard_handler = NULL;
    delete kga;
    }
    
void Kbd::insert_item( unsigned int keycode_P, Kbd_receiver* receiver_P )
    {
    Receiver_data& rcv = receivers[ receiver_P ];
    rcv.keycodes.append( keycode_P );
    if( rcv.active )
        grab_key( keycode_P );
    }

void Kbd::remove_item( unsigned int keycode_P, Kbd_receiver* receiver_P )
    {
    Receiver_data& rcv = receivers[ receiver_P ];
    rcv.keycodes.remove( keycode_P );
    if( rcv.active )
        ungrab_key( keycode_P );
    if( rcv.keycodes.count() == 0 )
        receivers.remove( receiver_P );
    }
    
void Kbd::activate_receiver( Kbd_receiver* receiver_P )
    {
    Receiver_data& rcv = receivers[ receiver_P ];
    if( rcv.active )
        return;
    rcv.active = true;
    for( QValueList< unsigned int >::ConstIterator it( rcv.keycodes.begin());
         it != rcv.keycodes.end();
         ++it )
        grab_key( *it );
    }

void Kbd::deactivate_receiver( Kbd_receiver* receiver_P )
    {
    Receiver_data& rcv = receivers[ receiver_P ];
    if( !rcv.active )
        return;
    rcv.active = false;
    for( QValueList< unsigned int >::ConstIterator it( rcv.keycodes.begin());
         it != rcv.keycodes.end();
         ++it )
        ungrab_key( *it );
    }

void Kbd::grab_key( unsigned int keycode_P )
    {
    if( grabs.contains( keycode_P ))
        ++grabs[ keycode_P ];
    else
        {
        grabs[ keycode_P ] = 1;
#if 0
        // CHECKME ugly ugly hack
        QString name = ' ' + QString::number( keycode_P );
        kga->insertItem( "", name, keycode_P );
        kga->connectItem( name, this, SLOT( key_slot( int )));
#endif
        QString name = ' ' + QString::number( keycode_P );
        kga->insert( name, name, QString::null, keycode_P, keycode_P,
            this, SLOT( key_slot( int )));
        QTimer::singleShot( 0, this, SLOT( update_connections()));
        }
    }
    
void Kbd::ungrab_key( unsigned int keycode_P )
    {
    if( !grabs.contains( keycode_P ))
        return;
    if( --grabs[ keycode_P ] == 0 )
        {
#if 0
        // CHECKME workaround for KGlobalAccel::disconnectItem() not working
        kga->setItemEnabled( ' ' + QString::number( keycode_P ), false );
        // kga->disconnectItem( ' ' + QString::number( keycode_P ), NULL, NULL );
        kga->removeItem( ' ' + QString::number( keycode_P ));
#endif
        kga->remove( ' ' + QString::number( keycode_P ));
        grabs.remove( keycode_P );
        QTimer::singleShot( 0, this, SLOT( update_connections()));
        }
    }

void Kbd::update_connections()
    {
    kga->updateConnections();
    }
    
void Kbd::key_slot( int keycode_P )
    {
    kdDebug( 1217 ) << "Key pressed:" << keycode_P << endl;
    unsigned int keycode = static_cast< unsigned int >( keycode_P ); // CHECKME
    if( !grabs.contains( keycode ))
        return;
    for( QMap< Kbd_receiver*, Receiver_data >::ConstIterator it = receivers.begin();
         it != receivers.end();
         ++it )
        if( ( *it ).keycodes.contains( keycode ) && ( *it ).active
            && it.key()->handle_key( keycode ))
            return;
    }
    
} // namespace KHotKeys

    
#ifdef HAVE_XTEST
#include <X11/extensions/XTest.h>

namespace KHotKeys
{

static bool xtest_available = false;
static bool xtest_inited = false;
static bool xtest()
    {
    if( xtest_inited )
        return xtest_available;
    xtest_inited = true;
    int dummy1, dummy2, dummy3, dummy4;
    xtest_available =
        ( XTestQueryExtension( qt_xdisplay(), &dummy1, &dummy2, &dummy3, &dummy4 ) == True );
    return xtest_available;
    }
#endif

// CHECKME nevola XFlush(), musi se pak volat rucne
bool Kbd::send_macro_key( unsigned int keycode, Window window_P )
    {
    unsigned int keysym = KKeyNative( KKey( keycode )).sym();
    KeyCode x_keycode = XKeysymToKeycode( qt_xdisplay(), keysym );
    if( x_keycode == NoSymbol )
	return false;
    unsigned int x_mod = KKeyNative( KKey( keycode )).mod();
#ifdef HAVE_XTEST
    if( xtest() && window_P == None )
        {
        // CHECKME tohle jeste potrebuje modifikatory
        bool ret = XTestFakeKeyEvent( qt_xdisplay(), x_keycode, True, CurrentTime );
        ret = ret && XTestFakeKeyEvent( qt_xdisplay(), x_keycode, False, CurrentTime );
        return ret;
        }
#endif
    if( window_P == None || window_P == InputFocus )
        window_P = windows_handler->active_window();
    if( window_P == None ) // CHECKME tohle cele je ponekud ...
        window_P = InputFocus;
    XKeyEvent ev;
    ev.type = KeyPress;
    ev.display = qt_xdisplay();
    ev.window = window_P;
    ev.root = qt_xrootwin();   // I don't know whether these have to be set
    ev.subwindow = None;       // to these values, but it seems to work, hmm
    ev.time = CurrentTime;
    ev.x = 0;
    ev.y = 0;
    ev.x_root = 0;
    ev.y_root = 0;
    ev.keycode = x_keycode;
    ev.state = x_mod;
    ev.same_screen = True;
    bool ret = XSendEvent( qt_xdisplay(), window_P, True, KeyPressMask, ( XEvent* )&ev );
#if 1
    ev.type = KeyRelease;  // is this actually really needed ??
    ev.display = qt_xdisplay();
    ev.window = window_P;
    ev.root = qt_xrootwin();
    ev.subwindow = None;
    ev.time = CurrentTime;
    ev.x = 0;
    ev.y = 0;
    ev.x_root = 0;
    ev.y_root = 0;
    ev.state = x_mod;
    ev.keycode = x_keycode;
    ev.same_screen = True;
    ret = ret && XSendEvent( qt_xdisplay(), window_P, True, KeyReleaseMask, ( XEvent* )&ev );
#endif
    return ret;
    }

bool Mouse::send_mouse_button( int button_P, bool release_P )
    {
#ifdef HAVE_XTEST
    if( xtest())
        {
        // CHECKME tohle jeste potrebuje modifikatory
        // a asi i spravnou timestamp misto CurrentTime
        bool ret = XTestFakeButtonEvent( qt_xdisplay(), button_P, True, CurrentTime );
        if( release_P )
            ret = ret && XTestFakeButtonEvent( qt_xdisplay(), button_P, False, CurrentTime );
        return ret;
        }
#endif
    return false;
    }
    
} // namespace KHotKeys

#include "input.moc"
