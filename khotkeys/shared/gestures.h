/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2002 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
 $Id$

****************************************************************************/

#ifndef _GESTURES_H_
#define _GESTURES_H_

#include <qwidget.h>
#include <qtimer.h>

#include <X11/Xlib.h>
#include <fixx11h.h>

namespace KHotKeys
{

class Gesture;
extern Gesture* gesture_handler;

class Stroke
    {
    public:
    // maximum number of numbers in stroke
        static const int MAX_SEQUENCE = 25;
    // largest number of points allowed to be sampled
        static const int MAX_POINTS = 5000;
    // default number of sample points in a bin to be valid
        static const int MIN_BIN_POINTS = 20;
    // default threshold of size of smaller axis needed for it to define its own bin size
        static const int SCALE_RATIO = 4;
    // default number of sample points required to have a valid stroke
        static const int MIN_POINTS = 50;
	Stroke();
	~Stroke();
	bool record( int x, int y );
	char* translate( int min_bin_points_P = MIN_BIN_POINTS, int scale_ratio_P = SCALE_RATIO,
	    int min_points_P = MIN_POINTS ); // CHECKME returns ret_val ( see below )
	void reset();
    protected:
	int bin( int x, int y );
	// metrics for input stroke
	int min_x, min_y;
	int max_x, max_y;
	int point_count;
	int delta_x, delta_y;
	int bound_x_1, bound_x_2;
	int bound_y_1, bound_y_2;
	struct point
	    {
	    int x;
	    int y;
	    };
	point* points;
	char ret_val[ MAX_SEQUENCE ];
    };

class Gesture
    : public QWidget // not QObject because of x11EventFilter()
    {
    Q_OBJECT
    public:
        Gesture( bool enabled_P, QObject* parent_P );
        virtual ~Gesture();
        void enable( bool enable_P );
        void set_mouse_button( unsigned int button_P );
        void set_timeout( int time_P );
    signals:
        void handle_gesture( const QString &gesture );
    protected:
	virtual bool x11Event( XEvent* ev_P );
    private slots:
        void stroke_timeout();
    private:
        void grab_mouse( bool grab_P );
        void mouse_replay( bool release_P );
        bool _enabled;
        Stroke stroke;
        int start_x, start_y;
        QTimer nostroke_timer;
        bool recording;
        unsigned int button;
        int timeout;
    };

// Gesture class must be QWidget derived because of x11Event()
// but it should be QObject owned -> use a QObject proxy that will delete it
class DeleteObject
    : public QObject
    {
    Q_OBJECT
    public:
        DeleteObject( QWidget* widget_P, QObject* parent_P )
            : QObject( parent_P ), widget( widget_P ) {};
        virtual ~DeleteObject() { delete widget; }
    private:
        QWidget* widget;
    };
    

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
