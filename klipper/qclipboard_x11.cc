/****************************************************************************
** $Id$
**
** Implementation of QClipboard class for X11
**
** Created : 960430
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD

// #define QT_CLIPBOARD_DEBUG

#include <qapplication.h>
#include <qbitmap.h>
#include <qdatetime.h>
#include <qdragobject.h>
#include <qbuffer.h>
#include <qt_x11.h>

#include <config.h>

// REVISED: arnt

/* #####

  If we had two clipboards, one for automatic copy (ie. the normal
  X11 selection mechanism) and one for CTRL-C copying (ie. the
  Windows norm), then highlight-copy-highlight-paste could be
  implemented in QMultiLineEdit, etc., instead of having different
  behaviour on Windows and X11.
*/

/*****************************************************************************
  Internal QClipboard functions for X11.
 *****************************************************************************/

extern Time qt_x_time;			// def. in qapplication_x11.cpp
extern Time qt_x_incr;			// def. in qapplication_x11.cpp
extern Atom qt_selection_property;
extern Atom qt_selection_sentinel;
extern Atom* qt_xdnd_str_to_atom( const char *mimeType );
extern const char* qt_xdnd_atom_to_str( Atom );


static QWidget * owner = 0;

static void cleanup()
{
    delete owner;
    owner = 0;
}

static
void setupOwner()
{
    if ( owner )
	return;
    owner = new QWidget( 0, "internal clipboard owner" );
    qAddPostRoutine( cleanup );
}

class QClipboardWatcher : public QMimeSource {
public:
    QClipboardWatcher();
    bool empty() const;
    const char* format( int n ) const;
    QByteArray encodedData( const char* fmt ) const;
    QByteArray getDataInFormat(Atom fmtatom) const;
};



class QClipboardData
{
public:
    QClipboardData();
    ~QClipboardData();

    void setSource(QMimeSource* s)
    {
	QMimeSource *s2 = src;
	src = s;

	delete s2;
    }

    QMimeSource* source()
    { return src; }
    void addTransferredPixmap(QPixmap pm)
    { /* TODO: queue them */
	transferred[tindex] = pm;
	tindex=(tindex+1)%2;
    }
    void clearTransfers()
    {
	transferred[0] = QPixmap();
	transferred[1] = QPixmap();
    }

    void clear();

    // private:
    QMimeSource* src;

    QPixmap transferred[2];
    int tindex;
};

QClipboardData::QClipboardData()
{
    src = 0;
    tindex=0;
}

QClipboardData::~QClipboardData()
{
    QMimeSource *s2 = src;
    src = 0;
    delete s2;
}

void QClipboardData::clear()
{
    QMimeSource *s2 = src;
    src = 0;
    delete s2;
}


static QClipboardData *internalCbData = 0;

static void cleanupClipboardData()
{
    delete internalCbData;
    internalCbData = 0;
}

static QClipboardData *clipboardData()
{
    if ( internalCbData == 0 ) {
	internalCbData = new QClipboardData;
	CHECK_PTR( internalCbData );
	qAddPostRoutine( cleanupClipboardData );
    }
    return internalCbData;
}


/*****************************************************************************
  QClipboard member functions for X11.
 *****************************************************************************/

/*!
  Clears the clipboard contents.
*/

void QClipboard::clear()
{
    setText( QString::null );
}


void QClipboard::clobber()
{
    if ( !internalCbData ) return;

    qRemovePostRoutine(cleanupClipboardData);

    internalCbData->src = 0;
    delete internalCbData;
    internalCbData = 0;

    Window win =  XGetSelectionOwner(owner->x11Display(), XA_PRIMARY);

    if (win == owner->winId())
	XSetSelectionOwner(owner->x11Display(), XA_PRIMARY, None, qt_x_time);
}


bool qt_xclb_wait_for_event( Display *dpy, Window win, int type, XEvent *event,
			     int timeout )
{
    QTime started = QTime::currentTime();
    QTime now = started;
    do {
	if ( XCheckTypedWindowEvent(dpy,win,type,event) )
	    return TRUE;

	now = QTime::currentTime();
	if ( started > now )			// crossed midnight
	    started = now;
	
	// this loop can take quite a long time, and as klipper invokes this
	// once a second, we hack it, to make it not consume all the CPU.
	// XSync( dpy, FALSE );			// toss a ball while we wait
	usleep( 70000 ); // sleep 70ms for the event to arrive
    } while ( started.msecsTo(now) < timeout );
    return FALSE;
}


static inline int maxSelectionIncr( Display *dpy )
{
    return XMaxRequestSize(dpy) > 65536 ?
	4*65536 : XMaxRequestSize(dpy)*4 - 100;
}


// uglehack: externed into qt_xdnd.cpp.  qt is really not designed for
// single-platform, multi-purpose blocks of code...
bool qt_xclb_read_property( Display *dpy, Window win, Atom property,
			   bool deleteProperty,
			   QByteArray *buffer, int *size, Atom *type,
			   int *format, bool nullterm )
{
    int	   maxsize = maxSelectionIncr(dpy);
    ulong  bytes_left;
    ulong  length;
    uchar *data;
    Atom   dummy_type;
    int    dummy_format;
    int    r;

    if ( !type )				// allow null args
	type = &dummy_type;
    if ( !format )
	format = &dummy_format;

    // Don't read anything, just get the size of the property data
    r = XGetWindowProperty( dpy, win, property, 0, 0, FALSE,
			    AnyPropertyType, type, format,
			    &length, &bytes_left, &data );
    if ( r != Success ) {
	buffer->resize( 0 );
	return FALSE;
    }
    XFree( (char*)data );

    int  offset = 0;
    bool ok = buffer->resize( (int)bytes_left+ (nullterm?1:0) );

    if ( ok ) {					// could allocate buffer
	while ( bytes_left ) {			// more to read...
	    r = XGetWindowProperty( dpy, win, property, offset/4, maxsize/4,
				    FALSE, AnyPropertyType, type, format,
				    &length, &bytes_left, &data );
	    if ( r != Success )
		break;
	    length *= *format/8;		// length in bytes
	    // Here we check if we get a buffer overflow and tries to
	    // recover -- this shouldn't normally happen, but it doesn't
	    // hurt to be defensive
	    if ( offset+length > buffer->size() ) {
		length = buffer->size() - offset;
		bytes_left = 0;			// escape loop
	    }
	    memcpy( buffer->data()+offset, data, (unsigned int)length );
	    offset += (unsigned int)length;
	    XFree( (char*)data );
	}
	if (nullterm)
	    buffer->at(offset) = '\0';		// zero-terminate (for text)
    }
    if ( size )
	*size = offset;				// correct size, not 0-term.
    XFlush( dpy );
    if ( deleteProperty ) {
	XDeleteProperty( dpy, win, property );
	XFlush( dpy );
    }
    return ok;
}


// this is externed into qt_xdnd.cpp too.
QByteArray qt_xclb_read_incremental_property( Display *dpy, Window win,
					      Atom property, int nbytes,
					      bool nullterm )
{
    XEvent event;

    QByteArray buf;
    QByteArray tmp_buf;
    bool alloc_error = FALSE;
    int  length;
    int  offset = 0;

    XWindowAttributes wa;
    XGetWindowAttributes( dpy, win, &wa );
    // Change the event mask for the window, it will be restored before
    // this function ends
    XSelectInput( dpy, win, PropertyChangeMask);

    if ( nbytes > 0 ) {
	// Reserve buffer + zero-terminator (for text data)
	// We want to complete the INCR transfer even if we cannot
	// allocate more memory
	alloc_error = !buf.resize(nbytes+1);
    }

    while ( TRUE ) {
	if ( !qt_xclb_wait_for_event(dpy,win,PropertyNotify,
				     (XEvent*)&event,5000) )
	    break;
	XFlush( dpy );
	if ( event.xproperty.atom != property ||
	     event.xproperty.state != PropertyNewValue )
	    continue;
	if ( qt_xclb_read_property(dpy, win, property, TRUE, &tmp_buf,
					&length,0, 0, nullterm) ) {
	    if ( length == 0 ) {		// no more data, we're done
		buf.at( offset ) = '\0';
		buf.resize( offset+1 );
		break;
	    } else if ( !alloc_error ) {
		if ( offset+length > (int)buf.size() ) {
		    if ( !buf.resize(offset+length+65535) ) {
			alloc_error = TRUE;
			length = buf.size() - offset;
		    }
		}
		memcpy( buf.data()+offset, tmp_buf.data(), length );
		tmp_buf.resize( 0 );
		offset += length;
	    }
	} else {
	    break;
	}
    }
    // Restore the event mask
    XSelectInput( dpy, win, wa.your_event_mask & ~PropertyChangeMask );
    return buf;
}


/*!
  \internal
  Internal cleanup for Windows.
*/

void QClipboard::ownerDestroyed()
{
    owner = 0;
}


/*!
  \internal
  Internal optimization for Windows.
*/

void QClipboard::connectNotify( const char * )
{
}


/*!\reimp
*/

bool QClipboard::event( QEvent *e )
{
    if ( e->type() != QEvent::Clipboard )
	return QObject::event( e );

    XEvent *xevent = (XEvent *)(((QCustomEvent *)e)->data());
    Display *dpy = qt_xdisplay();
    QClipboardData *d = clipboardData();

    switch ( xevent->type ) {

    case SelectionClear:			// new selection owner
	clipboardData()->clear();
	emit dataChanged();
	break;

    case SelectionNotify:
	clipboardData()->clear();
	break;

    case SelectionRequest:
	{		// someone wants our data
	    XSelectionRequestEvent *req = &xevent->xselectionrequest;

	    if (req->requestor == owner->winId())
		break;

	    XEvent evt;
	    evt.xselection.type = SelectionNotify;
	    evt.xselection.display	= req->display;
	    evt.xselection.requestor	= req->requestor;
	    evt.xselection.selection	= req->selection;
	    evt.xselection.target	= req->target;
	    evt.xselection.property	= None;
	    evt.xselection.time = req->time;
	    // ### Should we check that we own the clipboard?
	    // ### We do above

	    if ( !d->source() )
		d->setSource(new QClipboardWatcher());

	    const char* fmt;
	    QByteArray data;
	    static Atom xa_targets = *qt_xdnd_str_to_atom( "TARGETS" );
	    static Atom xa_multiple = *qt_xdnd_str_to_atom( "MULTIPLE" );
	    struct AtomPair { Atom target; Atom property; } *multi = 0;
	    int nmulti = 0;
	    int imulti = -1;
	    if ( req->target == xa_multiple ) {
		if ( qt_xclb_read_property( dpy,
					    req->requestor, req->property,
					    FALSE, &data, 0, 0, 0, 0 ) )
		    {
			nmulti = data.size()/sizeof(*multi);
			multi = new AtomPair[nmulti];
			memcpy(multi,data.data(),data.size());
		    }
		imulti = 0;
	    }

	    while ( imulti < nmulti ) {
		Window target;
		Atom property;

		evt.xselection.property = None;

		if ( multi ) {
		    target = multi[imulti].target;
		    property = multi[imulti].property;
		    imulti++;
		} else {
		    target = req->target;
		    property = req->property;
		}

		if ( target == xa_targets ) {
		    int atoms = 0; // 3 standard ones
		    while (d->source()->format(atoms))
			atoms++;
		    atoms += 3;

		    data = QByteArray(atoms * sizeof(Atom));
		    Atom* atarget = (Atom*)data.data();

		    int n = 0;
		    while ((fmt=d->source()->format(n)) && n < atoms) {
			Atom *dnd = qt_xdnd_str_to_atom(fmt);

#ifdef QT_CLIPBOARD_DEBUG
			qDebug("qclipboard_x11.cpp:%d: atom* for '%s' = %p %d",
			       __LINE__, fmt, dnd, n);
#endif

			atarget[n++] = *dnd;
		    }

		    if (n < atoms - 3) {
#ifdef QT_CLIPBOARD_DEBUG
			qDebug("qclipboard_x11.cpp:%d: n(%d) < n(%d) - 3",
			       __LINE__, n, atoms);
#endif

			if ( d->source()->provides("image/ppm") )
			    atarget[n++] = XA_PIXMAP;
			if ( d->source()->provides("image/pbm") )
			    atarget[n++] = XA_BITMAP;
			if ( d->source()->provides("text/plain") )
			    atarget[n++] = XA_STRING;
		    }

		    XChangeProperty ( dpy, req->requestor, property,
				      xa_targets, 32,
				      PropModeReplace,
				      (uchar *)data.data(),
				      data.size()/4 );
		    evt.xselection.property = property;
		} else {
		    bool already_done = FALSE;
		    if ( target == XA_STRING ) {
			fmt = "text/plain";
		    } else if ( target == XA_PIXMAP ) {
			fmt = "image/ppm";
			data = d->source()->encodedData(fmt);
			QPixmap pm;
			pm.loadFromData(data);
			Pixmap ph = pm.handle();
			XChangeProperty ( dpy, req->requestor, property,
					  target, 8,
					  PropModeReplace,
					  (uchar *)&ph,
					  sizeof(Pixmap));
			evt.xselection.property = property;
			d->addTransferredPixmap(pm);
			already_done = TRUE;
		    } else if ( target == XA_BITMAP ) {
			fmt = "image/pbm";
			data = d->source()->encodedData(fmt);
			QPixmap pm;
			QImage img;
			img.loadFromData(data);
			if ( img.depth() == 1 ) {
			    pm.convertFromImage(img);
			    Pixmap ph = pm.handle();
			    XChangeProperty ( dpy, req->requestor, property,
					      target, 8,
					      PropModeReplace,
					      (uchar *)&ph,
					      sizeof(Pixmap));
			    evt.xselection.property = property;
			    d->addTransferredPixmap(pm);
			} else {
			    pm.convertFromImage(img.convertDepth(1));
			    Pixmap ph = pm.handle();
			    XChangeProperty ( dpy, req->requestor, property,
					      target, 8,
					      PropModeReplace,
					      (uchar *)&ph,
					      sizeof(Pixmap));
			    evt.xselection.property = property;
			    d->addTransferredPixmap(pm);
			}
			already_done = TRUE;
		    } else {
			fmt = qt_xdnd_atom_to_str(target);
			if ( fmt && !d->source()->provides(fmt) ) {
			    fmt = 0; // Not a MIME type we can produce
			}
		    }
		    if ( fmt ) {
			if ( !already_done ) {
			    data = d->source()->encodedData(fmt);
			    XChangeProperty ( dpy, req->requestor, property,
					      target, 8,
					      PropModeReplace,
					      (uchar *)data.data(),
					      data.size() );
			    evt.xselection.property = property;
			}
		    }
		}

		XSendEvent( dpy, req->requestor, False, 0, &evt );
		if ( !nmulti )
		    break;
	    }
	}
	break;
    }

    return TRUE;
}






QClipboardWatcher::QClipboardWatcher()
{
    setupOwner();
}

bool QClipboardWatcher::empty() const
{
    Display *dpy   = owner->x11Display();
    return XGetSelectionOwner(dpy,XA_PRIMARY) == None;
}

const char* QClipboardWatcher::format( int n ) const
{
    if ( empty() )
	return 0;

    // TODO: record these once
    static Atom xa_targets = *qt_xdnd_str_to_atom( "TARGETS" );
    QByteArray targets = getDataInFormat(xa_targets);
    if ( targets.size()/sizeof(Atom) > (uint)n ) {
	Atom* target = (Atom*)targets.data();
	if ( *target == XA_PIXMAP )
	    return "image/ppm";
	const char* fmt = qt_xdnd_atom_to_str(target[n]);
	return fmt;
    } else {
	if ( n == 0 )
	    return "text/plain";
    }
    return 0;
}

QByteArray QClipboardWatcher::encodedData( const char* fmt ) const
{
    if ( !fmt || empty() )
	return QByteArray( 0 );

    Atom fmtatom = 0;

    if ( 0==qstrcmp(fmt,"text/plain") ) {
	fmtatom = XA_STRING;
    } else if ( 0==qstrcmp(fmt,"image/ppm") ) {
	fmtatom = XA_PIXMAP;
	QByteArray pmd = getDataInFormat(fmtatom);
	if ( pmd.size() == sizeof(Pixmap) ) {
	    Pixmap xpm = *((Pixmap*)pmd.data());
	    Display *dpy   = owner->x11Display();
	    Window r;
	    int x,y;
	    uint w,h,bw,d;
	    XGetGeometry(dpy,xpm, &r,&x,&y,&w,&h,&bw,&d);
	    QImageIO iio;
	    GC gc = XCreateGC( dpy, xpm, 0, 0 );
	    if ( d == 1 ) {
		QBitmap qbm(w,h);
		XCopyArea(dpy,xpm,qbm.handle(),gc,0,0,w,h,0,0);
		iio.setFormat("PBMRAW");
		iio.setImage(qbm.convertToImage());
	    } else {
		QPixmap qpm(w,h);
		XCopyArea(dpy,xpm,qpm.handle(),gc,0,0,w,h,0,0);
		iio.setFormat("PPMRAW");
		iio.setImage(qpm.convertToImage());
	    }
	    XFreeGC(dpy,gc);
	    QBuffer buf;
	    buf.open(IO_WriteOnly);
	    iio.setIODevice(&buf);
	    iio.write();
	    return buf.buffer();
	} else {
	    fmtatom = *qt_xdnd_str_to_atom(fmt);
	}
    } else {
	fmtatom = *qt_xdnd_str_to_atom(fmt);
    }
    return getDataInFormat(fmtatom);
}

QByteArray QClipboardWatcher::getDataInFormat(Atom fmtatom) const
{
    QByteArray buf;

    Window   win   = owner->winId();
    Display *dpy   = owner->x11Display();

    XConvertSelection( dpy, XA_PRIMARY, fmtatom,
		       qt_selection_property, win, CurrentTime );
    XFlush( dpy );

    XEvent xevent;
    if ( !qt_xclb_wait_for_event(dpy,win,SelectionNotify,&xevent,5000) )
	return buf;

    Atom   type;

    if ( qt_xclb_read_property(dpy,win,qt_selection_property,TRUE,
			       &buf,0,&type,0,FALSE) ) {
	if ( type == qt_x_incr ) {
	    int nbytes = buf.size() >= 4 ? *((int*)buf.data()) : 0;
	    buf = qt_xclb_read_incremental_property( dpy, win,
						     qt_selection_property,
						     nbytes, FALSE );
	}
    }

    return buf;
}


/*!
  Returns a reference to a QMimeSource representation of the current
  clipboard data.
*/
QMimeSource* QClipboard::data() const
{
    QClipboardData *d = clipboardData();

    if ( !d->source() )
	d->setSource(new QClipboardWatcher());

    return d->source();
}

/*!
  Sets the clipboard data.  Ownership of the data is transferred to
  the clipboard - the only ways to remove this data is to set
  something else, or to call clear().  The QDragObject subclasses are
  reasonable things to put on the clipboard (but do not try to call
  QDragObject::drag() on the same object).  Any QDragObject placed in
  the clipboard should have a parent of 0.  Do not put QDragMoveEvent
  or QDropEvent subclasses on the clipboard, as they do not belong to
  the event handler which receives them.

  The setText() and setPixmap() functions are simpler wrappers for this.
*/
void QClipboard::setData( QMimeSource* src )
{
    QClipboardData *d = clipboardData();
    setupOwner();
    Window   win   = owner->winId();
    Display *dpy   = owner->x11Display();

    d->setSource( src );
    emit dataChanged();

    // ### 3 round trips per selection change?  this should be optimized...
    Window prevOwner = XGetSelectionOwner( dpy, XA_PRIMARY );
    XSetSelectionOwner( dpy, XA_PRIMARY, win, qt_x_time );

    // ### perhaps this should be CHECK_RANGE ?
#if defined(DEBUG)
    if ( XGetSelectionOwner(dpy,XA_PRIMARY) != win ) {
	qWarning( "QClipboard::setData: Cannot set X11 selection owner" );
	return;
    }
#endif

    // Signal to other Qt processes that the selection has changed
    Window owners[2];
    owners[0] = win;
    owners[1] = prevOwner;
    XChangeProperty( dpy, QApplication::desktop()->winId(),
		     qt_selection_sentinel, XA_WINDOW, 32, PropModeReplace,
		     (unsigned char*)&owners, 2 );
}


/*
  Called by the main event loop in qapplication_x11.cpp when the
  _QT_SELECTION_SENTINEL property has been changed (i.e. when some Qt
  process has performed QClipboard::setData(). If it returns TRUE, the
  QClipBoard dataChanged() signal should be emitted.
*/

bool qt_check_selection_sentinel( XEvent* )
{
    bool doIt = TRUE;
    if ( owner ) {
	/*
	  Since the X selection mechanism cannot give any signal when
	  the selection has changed, we emulate it (for Qt processes) here.
	  The notification should be ignored in case of either
	  a) This is the process that did setData (because setData()
	  then has already emitted dataChanged())
	  b) This is the process that owned the selection when dataChanged()
	  was called (because we have then received a SelectionClear event,
	  and have already emitted dataChanged() as a result of that)
	*/

	//# Could optimize away the X server roundtrip of XGetWindowProperty
	// by checking if dataChanged() is connected to anything
	// ### This is done in the main event loop in QApplication
	Window* owners;
	Atom actualType;
	int actualFormat;
	ulong nitems;
	ulong bytesLeft;
	XGetWindowProperty( owner->x11Display(),
			    QApplication::desktop()->winId(),
			    qt_selection_sentinel, 0, 2, FALSE, XA_WINDOW,
			    &actualType, &actualFormat, &nitems,
			    &bytesLeft, (unsigned char**)&owners );
	if ( actualType == XA_WINDOW && actualFormat == 32 && nitems == 2 ) {
	    Window win = owner->winId();
	    if ( owners[0] == win || owners[1] == win )
		doIt = FALSE;
	}
	XFree( owners );
    }
    return doIt;
}


#endif // QT_NO_CLIPBOARD
