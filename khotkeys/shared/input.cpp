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
#include <QWidget>
//Added by qt3to4:
#include <QList>

#include <kactioncollection.h>
#include <kaction.h>
#include <kkeyserver.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kdeversion.h>
#include <QTimer>

#include "khotkeysglobal.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <QX11Info>

#include "windows.h"

namespace KHotKeys
{

// Kbd

Kbd::Kbd( bool grabbing_enabled_P, QObject* parent_P )
    : QObject( parent_P )
    {
    assert( keyboard_handler == NULL );
    keyboard_handler = this;
    kga = new KActionCollection( this );
    kga->setEnabled( grabbing_enabled_P );
    connect(kga, SIGNAL(actionTriggered(KAction*)), SLOT(actionTriggered(KAction*)));
    }

Kbd::~Kbd()
    {
    keyboard_handler = NULL;
    }

void Kbd::insert_item( const KShortcut& shortcut_P, Kbd_receiver* receiver_P )
    {
    Receiver_data& rcv = receivers[ receiver_P ];
    rcv.shortcuts.append( shortcut_P );
    if( rcv.active )
        grab_shortcut( shortcut_P );
    }

void Kbd::remove_item( const KShortcut& shortcut_P, Kbd_receiver* receiver_P )
    {
    Receiver_data& rcv = receivers[ receiver_P ];
    rcv.shortcuts.removeAll( shortcut_P );
    if( rcv.active )
        ungrab_shortcut( shortcut_P );
    if( rcv.shortcuts.count() == 0 )
        receivers.remove( receiver_P );
    }

void Kbd::activate_receiver( Kbd_receiver* receiver_P )
    {
    Receiver_data& rcv = receivers[ receiver_P ];
    if( rcv.active )
        return;
    rcv.active = true;
    for( QList< KShortcut >::ConstIterator it( rcv.shortcuts.begin());
         it != rcv.shortcuts.end();
         ++it )
        grab_shortcut( *it );
    }

void Kbd::deactivate_receiver( Kbd_receiver* receiver_P )
    {
    Receiver_data& rcv = receivers[ receiver_P ];
    if( !rcv.active )
        return;
    rcv.active = false;
    for( QList< KShortcut >::ConstIterator it( rcv.shortcuts.begin());
         it != rcv.shortcuts.end();
         ++it )
        ungrab_shortcut( *it );
    }

void Kbd::grab_shortcut( const KShortcut& shortcut_P )
    {
    if( grabs.contains( shortcut_P ))
        ++grabs[ shortcut_P ];
    else
        {
        grabs[ shortcut_P ] = 1;
#if 0
        // CHECKME ugly ugly hack
        QString name = ' ' + QString::number( keycode_P );
        kga->insertItem( "", name, keycode_P );
        kga->connectItem( name, this, SLOT( key_slot( int )));
#endif
        QString name = ' ' + shortcut_P.toStringInternal();
        KAction* a = new KAction(name, kga, name.toLatin1().constData());
        a->setGlobalShortcut(shortcut_P);
        }
    }

void Kbd::ungrab_shortcut( const KShortcut& shortcut_P )
    {
    if( !grabs.contains( shortcut_P ))
        return;
    if( --grabs[ shortcut_P ] == 0 )
        {
#if 0
        // CHECKME workaround for KGlobalAccel::disconnectItem() not working
        kga->setItemEnabled( ' ' + QString::number( keycode_P ), false );
        // kga->disconnectItem( ' ' + QString::number( keycode_P ), NULL, NULL );
        kga->removeItem( ' ' + QString::number( keycode_P ));
#endif
        delete kga->action( ' ' + shortcut_P.toStringInternal());
        grabs.remove( shortcut_P );
        }
    }

void Kbd::actionTriggered(KAction* action)
    {
    KShortcut shortcut = action->globalShortcut();
    if( !grabs.contains( shortcut ))
        return;
    for( QHash< Kbd_receiver*, Receiver_data >::ConstIterator it = receivers.begin();
         it != receivers.end();
         ++it )
        if( ( *it ).shortcuts.contains( shortcut ) && ( *it ).active
            && it.key()->handle_key( shortcut ))
            return;
    }


#ifdef HAVE_XTEST

} // namespace KHotKeys
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
        ( XTestQueryExtension( QX11Info::display(), &dummy1, &dummy2, &dummy3, &dummy4 ) == True );
    return xtest_available;
    }
#endif

// CHECKME nevola XFlush(), musi se pak volat rucne
bool Kbd::send_macro_key( const QString& key, Window window_P )
    {
    return false;
    int keysym;
    uint x_mod;
#if 0
// TODO fix this, make sure it works even with stuff like "dead_acute"
        QKeySequence ks( key );
        if( key == "Enter" && ks.isEmpty() )
            key = "Return"; // CHECKE hack
	keyboard_handler->send_macro_key( ks.isEmpty() ? 0 : ks[0], w );

    bool ok = KKeyServer::keyQtToSymX(keycode, keysym) && KKeyServer::keyQtToModX(keycode, x_mod);
#endif
    KeyCode x_keycode = XKeysymToKeycode( QX11Info::display(), keysym );
    if( x_keycode == NoSymbol )
	return false;
#ifdef HAVE_XTEST
    if( xtest() && window_P == None )
        {
        // CHECKME tohle jeste potrebuje modifikatory
        bool ret = XTestFakeKeyEvent( QX11Info::display(), x_keycode, True, CurrentTime );
        ret = ret && XTestFakeKeyEvent( QX11Info::display(), x_keycode, False, CurrentTime );
        return ret;
        }
#endif
    if( window_P == None || window_P == InputFocus )
        window_P = windows_handler->active_window();
    if( window_P == None ) // CHECKME tohle cele je ponekud ...
        window_P = InputFocus;
    XKeyEvent ev;
    ev.type = KeyPress;
    ev.display = QX11Info::display();
    ev.window = window_P;
    ev.root = QX11Info::appRootWindow();   // I don't know whether these have to be set
    ev.subwindow = None;       // to these values, but it seems to work, hmm
    ev.time = CurrentTime;
    ev.x = 0;
    ev.y = 0;
    ev.x_root = 0;
    ev.y_root = 0;
    ev.keycode = x_keycode;
    ev.state = x_mod;
    ev.same_screen = True;
    bool ret = XSendEvent( QX11Info::display(), window_P, True, KeyPressMask, ( XEvent* )&ev );
#if 1
    ev.type = KeyRelease;  // is this actually really needed ??
    ev.display = QX11Info::display();
    ev.window = window_P;
    ev.root = QX11Info::appRootWindow();
    ev.subwindow = None;
    ev.time = CurrentTime;
    ev.x = 0;
    ev.y = 0;
    ev.x_root = 0;
    ev.y_root = 0;
    ev.state = x_mod;
    ev.keycode = x_keycode;
    ev.same_screen = True;
    ret = ret && XSendEvent( QX11Info::display(), window_P, True, KeyReleaseMask, ( XEvent* )&ev );
#endif
    // Qt's autorepeat compression is broken and can create "aab" from "aba"
    // XSync() should create delay longer than Qt's max autorepeat interval
    XSync( QX11Info::display(), False );
    return ret;
    }

bool Mouse::send_mouse_button( int button_P, bool release_P )
    {
#ifdef HAVE_XTEST
    if( xtest())
        {
        // CHECKME tohle jeste potrebuje modifikatory
        // a asi i spravnou timestamp misto CurrentTime
        bool ret = XTestFakeButtonEvent( QX11Info::display(), button_P, True, CurrentTime );
        if( release_P )
            ret = ret && XTestFakeButtonEvent( QX11Info::display(), button_P, False, CurrentTime );
        return ret;
        }
#endif
    return false;
    }

} // namespace KHotKeys

#include "input.moc"
