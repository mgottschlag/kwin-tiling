/****************************************************************************

 KHotKeys -  (C) 1999 Lubos Lunak <l.lunak@email.cz>

 khkglobalaccel.cpp  - Slightly modified KGlobalAccel from kdelibs
 
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
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.

 $Id$

****************************************************************************/

#include <qwidget.h>

#include "khkglobalaccel.h"


KHKGlobalAccel::KHKGlobalAccel()
    : KGlobalAccel( true ) // HACK this avoids creating a QWidget
                           // in KGlobalAccel, because
                           // KGlobalAccel::x11EventFilter() is not virtual
    {
    do_not_grab = false; // HACK but we want grabbing
    }

// from 20000711 snapshot
bool KHKGlobalAccel::x11EventFilter( const XEvent *event_ ) {

        if ( aKeyMap.isEmpty() ) return false;
        if ( event_->type != KeyPress ) return false;
	
        uint mod=event_->xkey.state & (ControlMask | ShiftMask | Mod1Mask | Mod4Mask);
        uint keysym= XKeycodeToKeysym(qt_xdisplay(), event_->xkey.keycode, 0);
	
        KKeyEntry entry;
#if 1
        QString action;
#endif

        for (KKeyEntryMap::ConstIterator it = aKeyMap.begin();
            it != aKeyMap.end(); ++it) {
            int kc = (*it).aCurrentKeyCode;
            if ( mod == keyToXMod( kc ) && keysym == keyToXSym( kc ) ) {
                entry = *it;
#if 1
                action = it.key();
#endif
            }
        }
	
        if ( !entry.receiver )
            return false;
	

	XAllowEvents(qt_xdisplay(), AsyncKeyboard, CurrentTime);
	XUngrabKeyboard(qt_xdisplay(), CurrentTime);
	XSync(qt_xdisplay(), false);
	if ( !QWidget::keyboardGrabber() ) {
#if 0
	    connect( this, SIGNAL( activated() ),
                     entry.receiver, entry.member);
	    emit activated();
	    disconnect( this, SIGNAL( activated() ), entry.receiver,
                        entry.member );
#else
// this is actually the only important change
	    connect( this, SIGNAL( activated( const QString&, const QString&,
	        int) ), entry.receiver, entry.member);
	    emit activated( action, entry.descr, entry.aCurrentKeyCode );
	    disconnect( this, SIGNAL( activated( const QString&,
                const QString&, int ) ), entry.receiver, entry.member );
#endif
	}

	return true;
}

#include "khkglobalaccel.moc"
