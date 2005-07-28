/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2002 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

 Based on LibStroke :
  ( libstroke - an X11 stroke interface library
  Copyright (c) 1996,1997,1998,1999  Mark F. Willey, ETLA Technical
  There is a reference application available on the LibStroke Home Page:
  http://www.etla.net/~willey/projects/libstroke/ )
 
****************************************************************************/

#define _GESTURES_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gestures.h"

#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <X11/Xlib.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kxerrorhandler.h>
#include <kkeynative.h>

#include "input.h"
#include "windows.h"
#include <QX11Info>

namespace KHotKeys
{

Gesture* gesture_handler;

Gesture::Gesture( bool /*enabled_P*/, QObject* parent_P )
    : _enabled( false ), recording( false ), button( 0 ), exclude( NULL )
    {
    (void) new DeleteObject( this, parent_P );
    assert( gesture_handler == NULL );
    gesture_handler = this;
    connect( &nostroke_timer, SIGNAL( timeout()), SLOT( stroke_timeout()));
    connect( windows_handler, SIGNAL( active_window_changed( WId )),
        SLOT( active_window_changed( WId )));
    }
      
Gesture::~Gesture()
    {
    enable( false );
    gesture_handler = NULL;
    }

void Gesture::enable( bool enabled_P )
    {
    if( _enabled == enabled_P )
        return;
    _enabled = enabled_P;
    assert( button != 0 );
    update_grab();
    }

void Gesture::set_exclude( Windowdef_list* windows_P )
    {
    delete exclude;
    // check for count() > 0 - empty exclude list means no window is excluded,
    // but empty Windowdef_list matches everything
    if( windows_P != NULL && windows_P->count() > 0 )
        exclude = windows_P->copy();
    else
        exclude = NULL;
    update_grab();
    }

void Gesture::update_grab()
    {
    if( _enabled && handlers.count() > 0
        && ( exclude == NULL || !exclude->match( Window_data( windows_handler->active_window()))))
        {
        kapp->removeX11EventFilter( this ); // avoid being installed twice
        kapp->installX11EventFilter( this );
        // CHECKME at se grabuje jen kdyz je alespon jedno gesto?
        grab_mouse( true );
        }
    else
        {
        grab_mouse( false );
        kapp->removeX11EventFilter( this );
        }
    }

void Gesture::active_window_changed( WId )
    {
    update_grab();
    }

void Gesture::register_handler( QObject* receiver_P, const char* slot_P )
    {
    if( handlers.contains( receiver_P ))
        return;
    handlers[ receiver_P ] = true;
    connect( this, SIGNAL( handle_gesture( const QString&, WId )),
        receiver_P, slot_P );
    if( handlers.count() == 1 )
        update_grab();
    }

void Gesture::unregister_handler( QObject* receiver_P, const char* slot_P )
    {
    if( !handlers.contains( receiver_P ))
        return;
    handlers.remove( receiver_P );
    disconnect( this, SIGNAL( handle_gesture( const QString&, WId )),
        receiver_P, slot_P );
    if( handlers.count() == 0 )
        update_grab();
    }

bool Gesture::x11Event( XEvent* ev_P )
    {
    if( ev_P->type == ButtonPress && ev_P->xbutton.button == button )
        {
        kdDebug( 1217 ) << "GESTURE: mouse press" << endl;
        stroke.reset();
        stroke.record( ev_P->xbutton.x, ev_P->xbutton.y );
        nostroke_timer.start( timeout, true );
        recording = true;
        start_x = ev_P->xbutton.x_root;
        start_y = ev_P->xbutton.y_root;
        return true;
        }
    else if( ev_P->type == ButtonRelease && ev_P->xbutton.button == button
        && recording )
        {
        recording = false;
        nostroke_timer.stop();
        stroke.record( ev_P->xbutton.x, ev_P->xbutton.y );
        QString gesture( stroke.translate());
        if( gesture.isEmpty())
            {
            kdDebug( 1217 ) << "GESTURE: replay" << endl;
            XAllowEvents( QX11Info::display(), AsyncPointer, CurrentTime );
            XUngrabPointer( QX11Info::display(), CurrentTime );
            mouse_replay( true );
            return true;
            }
        kdDebug( 1217 ) << "GESTURE: got: " << gesture << endl;
        emit handle_gesture( gesture, windows_handler->window_at_position( start_x, start_y ));
        return true;
        }
    else if( ev_P->type == MotionNotify && recording )
        { // ignore small initial movement
        if( nostroke_timer.isActive()
            && abs( start_x - ev_P->xmotion.x_root ) < 10
            && abs( start_y - ev_P->xmotion.y_root ) < 10 )
            return true;
        nostroke_timer.stop();
        stroke.record( ev_P->xmotion.x, ev_P->xmotion.y );
        }
    return false;
    }

void Gesture::stroke_timeout()
    {
    kdDebug( 1217 ) << "GESTURE: timeout" << endl;
    XAllowEvents( QX11Info::display(), AsyncPointer, CurrentTime );
    XUngrabPointer( QX11Info::display(), CurrentTime );
    mouse_replay( false );
    recording = false;
    }

void Gesture::mouse_replay( bool release_P )
    {
    bool was_enabled = _enabled;
    enable( false );
    Mouse::send_mouse_button( button, release_P );
    enable( was_enabled );
    }

void Gesture::grab_mouse( bool grab_P )
    {
    if( grab_P )
        {
        KXErrorHandler handler;
        static int mask[] = { 0, Button1MotionMask, Button2MotionMask, Button3MotionMask,
            Button4MotionMask, Button5MotionMask, ButtonMotionMask, ButtonMotionMask,
            ButtonMotionMask, ButtonMotionMask };
#define XCapL KKeyNative::modXLock()
#define XNumL KKeyNative::modXNumLock()
#define XScrL KKeyNative::modXScrollLock()
        unsigned int mods[ 8 ] = 
            {
            0, XCapL, XNumL, XNumL | XCapL,
            XScrL, XScrL | XCapL,
            XScrL | XNumL, XScrL | XNumL | XCapL
            };
#undef XCapL
#undef XNumL
#undef XScrL
        for( int i = 0;
             i < 8;
             ++i )
            XGrabButton( QX11Info::display(), button, mods[ i ], QX11Info::appRootWindow(), False,
                ButtonPressMask | ButtonReleaseMask | mask[ button ], GrabModeAsync, GrabModeAsync,
                None, None );
        bool err = handler.error( true );
        kdDebug( 1217 ) << "Gesture grab:" << err << endl;
        }
    else
        {
        kdDebug( 1217 ) << "Gesture ungrab" << endl;
        XUngrabButton( QX11Info::display(), button, AnyModifier, QX11Info::appRootWindow());
        }
    }

void Gesture::set_mouse_button( unsigned int button_P )
    {
    if( button == button_P )
        return;
    if( !_enabled )
        {
        button = button_P;
        return;
        }
    grab_mouse( false );
    button = button_P;
    grab_mouse( true );
    }

void Gesture::set_timeout( int timeout_P )
    {
    timeout = timeout_P;
    }
    
Stroke::Stroke()
    {
    reset();
    points = new point[ MAX_POINTS ]; // CHECKME
    }
    
Stroke::~Stroke()
    {
    delete[] points;
    }

void Stroke::reset()
    {
    min_x = 10000;
    min_y = 10000;
    max_x = -1;
    max_y = -1;
    point_count = -1;
    }
        
bool Stroke::record( int x, int y )
    {
    if( point_count >= MAX_POINTS )
	return false;
    if( point_count == -1 )
	{
	++point_count;
	points[ point_count ].x = x;
	points[ point_count ].y = y;
	min_x = max_x = x;
	min_y = max_y = y;
	}
    else
	{
      // interpolate between last and current point
	int delx = x - points[ point_count ].x;
	int dely = y - points[ point_count ].y;
	if( abs( delx ) > abs( dely )) // step by the greatest delta direction
	    { 
    	    float iy = points[ point_count ].y;
             // go from the last point to the current, whatever direction it may be
    	    for( int ix = points[ point_count ].x;
		 ( delx > 0 ) ? ( ix < x ) : ( ix > x );
		 ( delx > 0 ) ? ++ix : --ix )
		{
		// step the other axis by the correct increment
		if( dely < 0 )
		    iy -= fabs( dely / ( float ) delx );
		else
		    iy += fabs( dely / ( float ) delx );
		// add the interpolated point
		++point_count;
		if( point_count >= MAX_POINTS )
		    return false;
		points[ point_count ].x = ix;
		points[ point_count ].y = ( int )iy;
		}
	    // add the last point
	    ++point_count;
	    if( point_count >= MAX_POINTS )
		return false;
	    points[ point_count ].x = x;
	    points[ point_count ].y = y;
	    // update metrics, it's ok to do it only for the last point
            if( x < min_x )
		min_x = x;
    	    if( x > max_x )
		max_x = x;
    	    if( y < min_y )
		min_y = y;
    	    if( y > max_y )
		max_y = y;
    	    }
	else
	    { // same thing, but for dely larger than delx case...
    	    float ix = points[ point_count ].x;
             // go from the last point to the current, whatever direction it may be
    	    for( int iy = points[ point_count ].y;
		 ( dely > 0 ) ? ( iy < y ) : ( iy > y );
		 ( dely > 0 ) ? ++iy : --iy )
		{
		// step the other axis by the correct increment
		if( delx < 0 )
		    ix -= fabs( delx / ( float ) dely );
		else
		    ix += fabs( delx / ( float ) dely );
		// add the interpolated point
		++point_count;
		if( point_count >= MAX_POINTS )
		    return false;
		points[ point_count ].x = ( int )ix;
		points[ point_count ].y = iy;
		}
	    // add the last point
	    ++point_count;
	    if( point_count >= MAX_POINTS )
		return false;
	    points[ point_count ].x = x;
	    points[ point_count ].y = y;
	    // update metrics, ts's ok to do it only for the last point
            if( x < min_x )
		min_x = x;
    	    if( x > max_x )
		max_x = x;
    	    if( y < min_y )
		min_y = y;
    	    if( y > max_y )
		max_y = y;
    	    }
	}
    return true;
    }
    
char* Stroke::translate( int min_bin_points_percentage_P, int scale_ratio_P, int min_points_P )
    {
    if( point_count < min_points_P )
	return NULL;
    // determine size of grid
    delta_x = max_x - min_x;
    delta_y = max_y - min_y;
    if( delta_x > scale_ratio_P * delta_y )
	{
	int avg_y = ( max_y + min_y ) / 2;
	min_y = avg_y - delta_x / 2;
	max_y = avg_y + delta_x / 2;
        delta_y = max_y - min_y;
	}
    else if( delta_y > scale_ratio_P * delta_x )
	{
	int avg_x = ( max_x + min_x ) / 2;
	min_x = avg_x - delta_y / 2;
	max_x = avg_x + delta_y / 2;
        delta_x = max_x - min_x;
	}
    // calculate bin boundary positions
    bound_x_1 = min_x + delta_x / 3;
    bound_x_2 = min_x + 2 * delta_x / 3;
    bound_y_1 = min_y + delta_y / 3;
    bound_y_2 = min_y + 2 * delta_y / 3;

    int sequence_count = 0;
    // points-->sequence translation scratch variables
    int prev_bin = 0;
    int current_bin = 0;
    int bin_count = 0;
// build string by placing points in bins, collapsing bins and discarding
// those with too few points...
    for( int pos = 0;
	 pos <= point_count;
	 ++pos )
	{
        // figure out which bin the point falls in
	current_bin = bin( points[ pos ].x, points[ pos ].y );
        // if this is the first point, consider it the previous bin, too.
	if( prev_bin == 0 )
	    prev_bin = current_bin;
        if( prev_bin == current_bin )
    	    bin_count++;
	else
	    {  // we are moving to a new bin -- consider adding to the sequence
	                                        // CHECKME tohle taky konfigurovatelne ?
	    if( bin_count >= ( min_bin_points_percentage_P * point_count / 100 )
		|| sequence_count == 0 )
		{
		if( sequence_count >= MAX_SEQUENCE )
		    return NULL;
	        ret_val[ sequence_count++ ] = prev_bin + '0';
		}
	    // restart counting points in the new bin
	    bin_count=0;
	    prev_bin = current_bin;
	    }
	}

    // add the last run of points to the sequence
    if( sequence_count >= MAX_SEQUENCE - 1 )
        return NULL;
    ret_val[ sequence_count++ ] = current_bin + '0';
    ret_val[ sequence_count ] = 0; // endmark
    return ret_val;
    }

/* figure out which bin the point falls in */
int Stroke::bin( int x, int y )
    {
    int bin_num = 1;
    if( x > bound_x_1 )
        ++bin_num;
    if( x > bound_x_2 )
        ++bin_num;
    if( y < bound_y_1 )
        bin_num += 3;
    if( y < bound_y_2 )
        bin_num += 3;
    return bin_num;
    }

} // namespace KHotKeys

#include "gestures.moc"
