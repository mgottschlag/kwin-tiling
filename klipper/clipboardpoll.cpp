// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project

   Copyright (C) 2003 by Lubos Lunak <l.lunak@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
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
 Klipper can rely on QClipboard's signals. If the owner is not Qt app, and the ownership has changed,
 it means the selection has changed as well. Otherwise, first only the timestamp
 of the last selection change is requested using the TIMESTAMP selection target, and if it's
 the same, it's assumed the contents haven't changed. Note that some applications (like XEmacs) does
 not provide this information, so Klipper has to assume that the clipboard might have changed in this
 case --- this is what is meant by REFUSED below.

*/

extern Time qt_x_time;

ClipboardPoll::ClipboardPoll( QWidget* parent )
    :   QWidget( parent )
{
    hide();
    kapp->installX11EventFilter( this );
    connect( &timer, SIGNAL( timeout()), SLOT( timeout()));
    timer.start( 1000, false );
    const char* names[ 6 ]
        = { "_QT_SELECTION_SENTINEL",
            "_QT_CLIPBOARD_SENTINEL",
            "CLIPBOARD",
            "TARGETS",
            "KLIPPER_SELECTION_TIMESTAMP",
            "KLIPPER_CLIPBOARD_TIMESTAMP" };
    Atom atoms[ 6 ];
    XInternAtoms( qt_xdisplay(), const_cast< char** >( names ), 6, False, atoms );
    selection.sentinel_atom = atoms[ 0 ];
    clipboard.sentinel_atom = atoms[ 1 ];
    xa_clipboard = atoms[ 2 ];
    xa_targets = atoms[ 3 ];
    selection.timestamp_atom = atoms[ 4 ];
    clipboard.timestamp_atom = atoms[ 5 ];
    selection.atom = XA_PRIMARY;
    clipboard.atom = xa_clipboard;
    selection.last_change = clipboard.last_change = qt_x_time; // don't trigger right after startup
    selection.last_owner = XGetSelectionOwner( qt_xdisplay(), XA_PRIMARY );
#ifdef NOISY_KLIPPER_
    kdDebug() << "(1) Setting last_owner for =" << "selection" << ":" << selection.last_owner << endl;
#endif
    clipboard.last_owner = XGetSelectionOwner( qt_xdisplay(), xa_clipboard );
#ifdef NOISY_KLIPPER_
    kdDebug() << "(2) Setting last_owner for =" << "clipboard" << ":" << clipboard.last_owner << endl;
#endif
    selection.waiting_for_timestamp = false;
    clipboard.waiting_for_timestamp = false;
    updateQtOwnership( selection );
    updateQtOwnership( clipboard );
}

bool ClipboardPoll::x11Event( XEvent* e )
{
// note that this is also installed as app-wide filter
    if( e->type == SelectionNotify && e->xselection.requestor == winId())
    {
        if( changedTimestamp( selection, *e ) ) {
#ifdef NOISY_KLIPPER_
            kdDebug() << "SELECTION CHANGED (GOT TIMESTAMP)" << endl;
#endif
            emit clipboardChanged( true );
        }

        if ( changedTimestamp( clipboard, *e ) )
        {
#ifdef NOISY_KLIPPER_
            kdDebug() << "CLIPBOARD CHANGED (GOT TIMESTAMP)" << endl;
#endif
            emit clipboardChanged( false );
        }
        return true; // filter out
    }
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
#ifdef REALLY_NOISY_KLIPPER_
        kdDebug() << "UPDATEQT BAD PROPERTY" << endl;
#endif
        data.owner_is_qt = false;
        if( prop != NULL )
            XFree( prop );
        return;
    }
    Window owner = reinterpret_cast< long* >( prop )[ 0 ]; // [0] is new owner, [1] is previous
    XFree( prop );
    Window current_owner = XGetSelectionOwner( qt_xdisplay(), data.atom );
    data.owner_is_qt = ( owner == current_owner );
#ifdef REALLY_NOISY_KLIPPER_
    kdDebug() << "owner=" << owner << "; current_owner=" << current_owner << endl;
    kdDebug() << "UPDATEQT:" << ( &data == &selection ? "selection" : "clipboard" )  << ":" << data.owner_is_qt << endl;
#endif
}

void ClipboardPoll::timeout()
{
    KlipperWidget::updateTimestamp();
    if( !kapp->clipboard()->ownsSelection() && checkTimestamp( selection ) ) {
#ifdef NOISY_KLIPPER_
        kdDebug() << "SELECTION CHANGED" << endl;
#endif
        emit clipboardChanged( true );
    }
    if( !kapp->clipboard()->ownsClipboard() && checkTimestamp( clipboard ) ) {
#ifdef NOISY_KLIPPER_
        kdDebug() << "CLIPBOARD CHANGED" << endl;
#endif
        emit clipboardChanged( false );
    }

}

bool ClipboardPoll::checkTimestamp( SelectionData& data )
{
    Window current_owner = XGetSelectionOwner( qt_xdisplay(), data.atom );
    bool signal = false;
    updateQtOwnership( data );
    if( data.owner_is_qt )
    {
        data.last_change = CurrentTime;
#ifdef REALLY_NOISY_KLIPPER_
        kdDebug() << "(3) Setting last_owner for =" << ( &data==&selection ?"selection":"clipboard" ) << ":" << current_owner << endl;
#endif
        data.last_owner = current_owner;
        data.waiting_for_timestamp = false;
        return false;
    }
    if( current_owner != data.last_owner )
    {
        signal = true; // owner has changed
        data.last_owner = current_owner;
#ifdef REALLY_NOISY_KLIPPER_
        kdDebug() << "(4) Setting last_owner for =" << ( &data==&selection ?"selection":"clipboard" ) << ":" << current_owner << endl;
#endif
        data.waiting_for_timestamp = false;
        data.last_change = CurrentTime;
#ifdef REALLY_NOISY_KLIPPER_
        kdDebug() << "OWNER CHANGE:" << ( data.atom == XA_PRIMARY ) << ":" << current_owner << endl;
#endif
        return true;
    }
    if( current_owner == None ) {
        return false; // None also last_owner...
    }
    if( data.waiting_for_timestamp ) {
        // We're already waiting for the timestamp of the last check
        return false;
    }
    XDeleteProperty( qt_xdisplay(), winId(), data.timestamp_atom );
    XConvertSelection( qt_xdisplay(), data.atom, xa_targets, data.timestamp_atom, winId(), qt_x_time );
    data.waiting_for_timestamp = true;
    data.waiting_x_time = qt_x_time;
#ifdef REALLY_NOISY_KLIPPER_
    kdDebug() << "WAITING TIMESTAMP:" << ( data.atom == XA_PRIMARY ) << endl;
#endif
    return false;
}

bool ClipboardPoll::changedTimestamp( SelectionData& data, const XEvent& ev )
{
    if( ev.xselection.requestor != winId()
        || ev.xselection.selection != data.atom
        || ev.xselection.time != data.waiting_x_time )
    {
        return false;
    }
    data.waiting_for_timestamp = false;
    if( ev.xselection.property == None )
    {
#ifdef NOISY_KLIPPER_
        kdDebug() << "REFUSED:" << ( data.atom == XA_PRIMARY ) << endl;
#endif
        return true;
    }
    Atom type;
    int format;
    unsigned long nitems;
    unsigned long after;
    unsigned char* prop = NULL;
    if( XGetWindowProperty( qt_xdisplay(), winId(), ev.xselection.property, 0, 1, False,
        AnyPropertyType, &type, &format, &nitems, &after, &prop ) != Success
        || format != 32 || nitems != 1 || prop == NULL )
    {
#ifdef NOISY_KLIPPER_
        kdDebug() << "BAD PROPERTY:" << ( data.atom == XA_PRIMARY ) << endl;
#endif
        if( prop != NULL )
            XFree( prop );
        return true;
    }
    Time timestamp = reinterpret_cast< long* >( prop )[ 0 ];
    XFree( prop );
#ifdef NOISY_KLIPPER_
    kdDebug() << "GOT TIMESTAMP:" << ( data.atom == XA_PRIMARY ) << endl;
    kdDebug() <<   "timestamp=" << timestamp
              << "; CurrentTime=" << CurrentTime
              << "; last_change=" << data.last_change
              << endl;
#endif
    if( timestamp != data.last_change || timestamp == CurrentTime )
    {
#ifdef NOISY_KLIPPER_
        kdDebug() << "TIMESTAMP CHANGE:" << ( data.atom == XA_PRIMARY ) << endl;
#endif
        data.last_change = timestamp;
        return true;
    }
    return false; // ok, same timestamp
}

#include "clipboardpoll.moc"
