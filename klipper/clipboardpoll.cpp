/* -------------------------------------------------------------

   clipboardpoll.cpp (part of Klipper - Cut & paste history for KDE)

   (C) 2003 by Lubos Lunak <l.lunak@kde.org>

   Licensed under the Artistic License

 ------------------------------------------------------------- */

#include "clipboardpoll.h"

#include <kapplication.h>
#include <qclipboard.h>
#include <kdebug.h>
#include <X11/Xatom.h>
#include <time.h>

#include "toplevel.h"

/*

 The polling magic:
 
 There's no way with X11 how to find out if the selection has changed (unless its ownership
 is taken away from the current client). In the future, there will be hopefully such notification,
 which will make this whole file more or less obsolete. But for now, Klipper has to poll.
 In order to avoid transferring all the data on every time pulse, this file implements two
 optimizations: The first one is checking whether the selection owner is Qt application (using
 the _QT_SELECTION/CLIPBOARD_SENTINEL atoms on the root window of screen 0), and if yes,
 Klipper can rely on QClipboard's signals. If the owner is not Qt app, first only the timestamp
 of the last selection change is requested using the TIMESTAMP selection target, and if it's
 the same, it's assumed the contents haven't changed. In both cases, if the owner changes,
 it indeed means the selection data has changed as well.
 
*/

extern Time qt_x_time;

ClipboardPoll::ClipboardPoll( QWidget* parent )
    :   QWidget( parent )
{
    hide();
    kapp->installX11EventFilter( this );
    connect( &timer, SIGNAL( timeout()), SLOT( timeout()));
    timer.start( 1000, false );
    const char* names[ 5 ] = { "_QT_SELECTION_SENTINEL", "_QT_CLIPBOARD_SENTINEL", "CLIPBOARD", "TIMESTAMP", "KLIPPER" };
    Atom atoms[ 5 ];
    XInternAtoms( qt_xdisplay(), const_cast< char** >( names ), 5, False, atoms );
    selection.sentinel_atom = atoms[ 0 ];
    clipboard.sentinel_atom = atoms[ 1 ];
    xa_clipboard = atoms[ 2 ];
    xa_timestamp = atoms[ 3 ];
    klipper_atom = atoms[ 4 ];
    selection.atom = XA_PRIMARY;
    clipboard.atom = xa_clipboard;
    selection.last_change = clipboard.last_change = qt_x_time; // don't trigger right after startup
    selection.last_owner = XGetSelectionOwner( qt_xdisplay(), XA_PRIMARY );
    clipboard.last_owner = XGetSelectionOwner( qt_xdisplay(), xa_clipboard );
    updateQtOwnership( selection );
    updateQtOwnership( clipboard );
}

bool ClipboardPoll::x11Event( XEvent* e )
{                                    // always root window on screen 0
    if( e->type != PropertyNotify || e->xproperty.window != qt_xrootwin( 0 ))
        return false;
    if( e->xproperty.atom != selection.sentinel_atom && e->xproperty.atom != clipboard.sentinel_atom )
        return false;
    updateQtOwnership( e->xproperty.atom == selection.sentinel_atom ? selection : clipboard );
    return false;
}

void ClipboardPoll::updateQtOwnership( SelectionData& data )
{
    Atom type;
    int format;
    unsigned long nitems;
    unsigned long after;
    unsigned char* prop = NULL;
    if( XGetWindowProperty( qt_xdisplay(), qt_xrootwin( 0 ), data.sentinel_atom, 0, 2, False,
        XA_WINDOW, &type, &format, &nitems, &after, &prop ) != Success
        || type != XA_WINDOW || format != 32 || nitems != 2 || prop == NULL )
    {
//        kdDebug() << "UPDATEQT BAD PROPERTY" << endl;
        data.owner_is_qt = false;
        if( prop != NULL )
            XFree( prop );
        return;
    }
    Window owner = reinterpret_cast< long* >( prop )[ 0 ]; // [0] is new owner, [1] is previous
    XFree( prop );
    data.owner_is_qt = ( owner == data.last_owner );
//    kdDebug() << "UPDATEQT:" << ( data.atom == XA_PRIMARY ) << ":" << data.owner_is_qt << endl;
}

void ClipboardPoll::timeout()
{
    KlipperWidget::updateXTime();
    bool signal = false;
    if( !kapp->clipboard()->ownsSelection())
        signal = signal || checkTimestamp( selection );
    if( !kapp->clipboard()->ownsClipboard())
        signal = signal || checkTimestamp( clipboard );
    if( signal )
    {
//        kdDebug() << "CLIPBOARD CHANGED" << endl;
        emit clipboardChanged();
    }
}

bool ClipboardPoll::checkTimestamp( SelectionData& data )
{
    Window current_owner = XGetSelectionOwner( qt_xdisplay(), data.atom );
    bool signal = false;
    if( data.owner_is_qt && data.last_owner == current_owner )
    {
        data.last_change = CurrentTime;
        return false;
    }
    if( current_owner != data.last_owner )
    {
        signal = true; // owner has changed
        data.last_owner = current_owner;
        data.owner_is_qt = false;
//        kdDebug() << "OWNER CHANGE:" << ( data.atom == XA_PRIMARY ) << ":" << current_owner << endl;
    }
    if( current_owner == None )
        return signal;
    XDeleteProperty( qt_xdisplay(), winId(), klipper_atom );
    XConvertSelection( qt_xdisplay(), data.atom, xa_timestamp, klipper_atom, winId(), qt_x_time );
    const int delay = 100; // ms
    int timeout = 1000; // ms
    for(;
         timeout > 0;
         timeout -= delay )
    {
        XEvent ev;
        while( XCheckTypedWindowEvent( qt_xdisplay(), winId(), SelectionNotify, &ev ))
        {
            if( ev.xselection.requestor != winId() || ev.xselection.selection != data.atom
                || ev.xselection.time != qt_x_time )
                continue;
            if( ev.xselection.property == None )
            {
//                kdDebug() << "REFUSED:" << ( data.atom == XA_PRIMARY ) << endl;
                signal = true;
                break;
            }
            Atom type;
            int format;
            unsigned long nitems;
            unsigned long after;
            unsigned char* prop = NULL;
            if( XGetWindowProperty( qt_xdisplay(), winId(), ev.xselection.property, 0, 1, False,
                XA_INTEGER, &type, &format, &nitems, &after, &prop ) != Success
//                || type != XA_INTEGER - I did such a lame mistake :( Qt sets xa_timestamp here
                || format != 32 || nitems != 1 || prop == NULL )
            {
//                kdDebug() << "BAD PROPERTY:" << ( data.atom == XA_PRIMARY ) << endl;
                if( prop != NULL )
                    XFree( prop );
                signal = true;
                break;
            }
            Time timestamp = reinterpret_cast< long* >( prop )[ 0 ];
            XFree( prop );
            if( timestamp != data.last_change || timestamp == CurrentTime )
            {
//                kdDebug() << "TIMESTAMP CHANGE:" << ( data.atom == XA_PRIMARY ) << endl;
                signal = true;
                data.last_change = timestamp;
                break;
            }
            break; // ok, same timestamp
        }
        struct timespec tm;
        tm.tv_sec = 0;
        tm.tv_nsec = delay * 1000 * 1000;
        nanosleep( &tm, NULL );
    }
    if( timeout == 0 )
        {
        signal = true;
//        kdDebug() << "TIMEOUT" << endl;
        }
    return signal;
}

#include "clipboardpoll.moc"
