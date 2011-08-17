/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2002 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#include "gestures.h"

#if 0
#include <stdlib.h>
#endif
#include <math.h>

#include "khotkeysglobal.h"


#include <kapplication.h>
#include <kdebug.h>
#include <kxerrorhandler.h>
#include <kkeyserver.h>
#include "input.h"
#include "windows_handler.h"
#include "action_data/action_data.h"

#include "windows_helper/window_selection_list.h"

// only necessary for circumventing bug #173606, see below
#include <QX11Info>
#include <X11/Xlib.h>
//#include <fixx11h.h>

// #include "voices.h"

namespace KHotKeys
{

QPointer<Gesture> gesture_handler = NULL;

Gesture::Gesture( bool enabled_P, QObject* parent_P )
        : QWidget(NULL)
        , _enabled( false )
        , recording( false )
        , button( 0 )
        , exclude( NULL )
    {
    kDebug() << enabled_P;
    (void) new DeleteObject( this, parent_P );
    nostroke_timer.setSingleShot( true );
    connect( &nostroke_timer, SIGNAL(timeout()), SLOT(stroke_timeout()));
    connect( windows_handler, SIGNAL(active_window_changed(WId)),
        SLOT(active_window_changed(WId)));
    }


Gesture::~Gesture()
    {
    enable( false );
    }


void Gesture::enable( bool enabled_P )
    {
    kDebug() << enabled_P;
    if( _enabled == enabled_P )
        return;
    _enabled = enabled_P;
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
    kDebug() << "Enabled:" << _enabled;
    kDebug() << "Handler:" << handlers.count();
    kDebug() << "Exclude:" << exclude << " Match? " << (exclude && exclude->match( Window_data( windows_handler->active_window())));

    if( _enabled && handlers.count() > 0
        && ( exclude == NULL || !exclude->match( Window_data( windows_handler->active_window()))))
        {
        kapp->removeX11EventFilter( this ); // avoid being installed twice
        kapp->installX11EventFilter( this );
        // CHECKME grab only when there's at least one gesture?
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

void Gesture::handleScore( ActionData* const data, const qreal score )
    {
    if(score > maxScore)
        {
        maxScore = score;
        bestFit = data;
        }
    }

void Gesture::register_handler( QObject* receiver_P, const char* slot_P )
    {
    if( handlers.contains( receiver_P ))
        return;
    handlers[ receiver_P ] = true;
    // connect directly because we want to be sure that all triggers submitted
    // their scores back to this object before executing the best match we
    // could find.
    connect( this, SIGNAL(handle_gesture(StrokePoints)),
            receiver_P, slot_P,
            Qt::DirectConnection
            );
    connect( receiver_P, SIGNAL(gotScore(ActionData*const,qreal)),
             this, SLOT(handleScore(ActionData*const,qreal)),
            Qt::DirectConnection
            );
    if( handlers.count() == 1 )
        update_grab();
    }

void Gesture::unregister_handler( QObject* receiver_P, const char* slot_P )
    {
    if( !handlers.contains( receiver_P ))
        return;
    handlers.remove( receiver_P );

    disconnect( this, SIGNAL(handle_gesture(StrokePoints)),
            receiver_P, slot_P
            );
    disconnect( receiver_P, SIGNAL(gotScore(ActionData*const,qreal)),
                this, SLOT(handleScore(ActionData*const,qreal))
            );
    if( handlers.count() == 0 )
        update_grab();
    }

bool Gesture::x11Event( XEvent* ev_P )
    {
/*      kDebug() << "   ( type = " << ev_P->type << " )" << KeyRelease << " " << KeyPress ;
        if( ev_P->type == XKeyPress || ev_P->type == XKeyRelease )
        {
            return voice_handler->x11Event( ev_P );
    }*/

    if( ev_P->type == ButtonPress && ev_P->xbutton.button == button )
        {
        kDebug() << "GESTURE: mouse press";
        stroke.reset();
        stroke.record( ev_P->xbutton.x, ev_P->xbutton.y );
        nostroke_timer.start( timeout );
        recording = true;
        start_x = ev_P->xbutton.x_root;
        start_y = ev_P->xbutton.y_root;
        return true;
        }
    // if stroke is finished... postprocess the data and send a signal.
    // then wait for incoming matching scores and execute the best fit.
    else if( ev_P->type == ButtonRelease && ev_P->xbutton.button == button
        && recording )
        {
        recording = false;
        nostroke_timer.stop();
        stroke.record( ev_P->xbutton.x, ev_P->xbutton.y );
        StrokePoints gesture( stroke.processData() );
        if( gesture.isEmpty() )
            {
            kDebug() << "GESTURE: replay";
            XAllowEvents( QX11Info::display(), AsyncPointer, CurrentTime );
            XUngrabPointer( QX11Info::display(), CurrentTime );
            mouse_replay( true );
            return true;
            }

        // prepare for the incoming scores from different triggers
        maxScore = 0.0;
        bestFit = NULL;

        emit handle_gesture( gesture );
        // the signal is emitted directly, so we get all trigger scores before
        // the next lines are executed. bestFit should now contain
        // a pointer to the ActionData with the best-matching gesture.

        if( bestFit != NULL )
            {
            // set up the windows_handler
            WId window = windows_handler->window_at_position( start_x, start_y );
            windows_handler->set_action_window( window );
            // then execute the action associated with the best match.
            bestFit->execute();
            }

        return true;
        }
    else if( ev_P->type == MotionNotify && recording )
        { // ignore small initial movement
        if( nostroke_timer.isActive()
            && abs( start_x - ev_P->xmotion.x_root ) < 10
            && abs( start_y - ev_P->xmotion.y_root ) < 10
        )
            return true;
        nostroke_timer.stop();

        stroke.record( ev_P->xmotion.x, ev_P->xmotion.y );
        }
    return false;
    }


void Gesture::stroke_timeout()
    {
    kDebug() << "GESTURE: timeout";
    XAllowEvents( QX11Info::display(), AsyncPointer, CurrentTime );
    XUngrabPointer( QX11Info::display(), CurrentTime );
    mouse_replay( false );
    
    // for xorg-server 1.7 to 1.9 RC4: disable drag'n'drop support to evade bug #173606
    if( VendorRelease( QX11Info::display() ) < 10899905 &&  VendorRelease( QX11Info::display() ) >= 10700000 )
        mouse_replay( true );
      
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
    kDebug() << grab_P;

    if( grab_P )
        {
        kDebug() << "gesture grab";
        Q_ASSERT( button != 0 );
        KXErrorHandler handler;
        static int mask[] = { 0, Button1MotionMask, Button2MotionMask, Button3MotionMask,
            Button4MotionMask, Button5MotionMask, ButtonMotionMask, ButtonMotionMask,
            ButtonMotionMask, ButtonMotionMask };
#define XCapL KKeyServer::modXLock()
#define XNumL KKeyServer::modXNumLock()
#define XScrL KKeyServer::modXScrollLock()
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
        kDebug() << "Gesture grab:" << err;
        }
    else
        {
        kDebug() << "Gesture ungrab";
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



// Definitions for Gesture end here, Definitions for Stroke following.


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
    if( point_count == -1 )
        {
        ++point_count;
        points[ point_count ].x = x;
        points[ point_count ].y = y;

        // start metrics
        min_x = max_x = x;
        min_y = max_y = y;
        }
    else
        {
        ++point_count;
        if( point_count >= MAX_POINTS )
            return false;
        points[ point_count ].x = x;
        points[ point_count ].y = y;

        // update metrics
        if( x < min_x )
            min_x = x;
        if( x > max_x )
            max_x = x;
        if( y < min_y )
            min_y = y;
        if( y > max_y )
            max_y = y;

        }
    return true;
    }


// Compute some additional data from the raw point coordinates and store
// it all in a new data structure to be passed on and saved.

StrokePoints Stroke::processData()
    {
    if(point_count < 2 )
        return StrokePoints(); // empty vector

    int n = point_count-1;

    StrokePoints results(n);

    // calculate s, where s is the length of a stroke up to the current point
    // (first loop) divided by the total stroke length (second loop)
    qreal strokelength = 0.0;
    results[0].s = 0.0;

    for (int i = 0; i < n-1; i++)
        {
        strokelength += hypot(points[i+1].x - points[i].x, points[i+1].y - points[i].y);
        results[i+1].s = strokelength;
        }

    for (int i = 0; i < n; i++)
        results[i].s /= strokelength;


    // check which axis is longer...
    int scaleX = max_x - min_x;
    int scaleY = max_y - min_y;
    qreal scale = (scaleX > scaleY) ? scaleX : scaleY;

    // ...and scale the stroke coordinates to a new size depending on this axis
    // (saving into the new data structure for higher precision)
    for (int i = 0; i < n; i++)
        {
        results[i].x = (points[i].x-(min_x+max_x)/2.0)/scale + 0.5;
        results[i].y = (points[i].y-(min_y+max_y)/2.0)/scale + 0.5;
        }


    // calculate values of delta_s and angle for the points	by simple comparison
    // with the respective successor.
    // delta_s is the distance to the successor in the same units as s.
    // angle is the angle to the successor in units of pi.
    for (int i = 0; i < n-1; i++)
        {
        results[i].delta_s = results[i+1].s - results[i].s;
        results[i].angle = atan2(results[i+1].y - results[i].y, results[i+1].x - results[i].x)/M_PI;
        }

    // last point of result would need special logic, so we simply discard it -
    // there's enough points anyway
    results.pop_back();

    return results;
    }

} // namespace KHotKeys

#include "moc_gestures.cpp"
