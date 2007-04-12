/*
 * Copyright (C) 2003 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <kglobal.h>

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QCursor>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPaintEvent>


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xcursor/Xcursor.h>
#include <QX11Info>

#include "previewwidget.h"


extern bool qt_has_xft;
extern bool qt_use_xrender;


namespace {

	// Preview cursors
	const char *cursor_names[] =
    {
		"left_ptr",
		"left_ptr_watch",
		"watch",
		"hand2",
		"question_arrow",
		"sb_h_double_arrow",
		"sb_v_double_arrow",
		"bottom_left_corner",
		"bottom_right_corner",
		"fleur",
		"pirate",
		"cross",
		"X_cursor",
		"right_ptr",
		"right_side",
		"right_tee",
		"sb_right_arrow",
		"sb_right_tee",
		"base_arrow_down",
		"base_arrow_up",
		"bottom_side",
		"bottom_tee",
		"center_ptr",
		"circle",
		"dot",
		"dot_box_mask",
		"dot_box_mask",
		"double_arrow",
		"draped_box",
		"left_side",
		"left_tee",
		"ll_angle",
		"top_side",
		"top_tee",
	};

	const int numCursors = 6;     // The number of cursors in the above list to be previewed
	const int previewSize = 24;   // The cursor size to be used in the preview widget
	const int cursorSpacing = 20;
}


class PreviewCursor
{
	public:
		PreviewCursor();
		~PreviewCursor();

		void load( const QString &, const QString & );
		const Picture picture() const { return m_pict; }
		const Cursor handle() const   { return m_handle; }
		const int width() const  { return m_width; }
		const int height() const { return m_height; }

	private:
		Picture createPicture( const XcursorImage* ) const;
		void cropCursorImage( XcursorImage*& ) const;
		Picture m_pict;
		Cursor m_handle;
		int m_width;
		int m_height;
}; // class PreviewCursor


PreviewCursor::PreviewCursor() :
	m_pict( 0 ), m_handle( 0 ), m_width( 0 ), m_height( 0 )
{
}


void PreviewCursor::load( const QString &name, const QString &theme )
{
        Display *dpy = QX11Info::display();

	if ( m_pict ) XRenderFreePicture( dpy, m_pict );
	if ( m_handle ) XFreeCursor( dpy, m_handle );
        m_pict = 0;
        m_handle = 0;
        m_width = m_height = 0;

	// Load the preview cursor image
	XcursorImage *image =
		XcursorLibraryLoadImage( name.toLatin1(), theme.toLatin1(), previewSize );

	// If the theme doesn't have this cursor, load the default cursor for now
	if ( !image )
		image = XcursorLibraryLoadImage( "left_ptr", theme.toLatin1(), previewSize );

	// TODO The old classic X cursors
	if ( !image )
		return;

	// Auto-crop the image (some cursor themes use a fixed image size
	//   for all cursors, and doing this results in correctly centered images)
	cropCursorImage( image );

	m_pict = createPicture( image );
	m_width = image->width;
	m_height = image->height;

	// Scale the image if its height is greater than 2x the requested size
	if ( m_height > previewSize * 2.0 ) {
		double factor = double( previewSize * 2.0 / m_height );
		XTransform xform = {
			{{ XDoubleToFixed(1.0), XDoubleToFixed(0),  XDoubleToFixed(0) },
			 { XDoubleToFixed(0),  XDoubleToFixed(1.0), XDoubleToFixed(0) },
			 { XDoubleToFixed(0),  XDoubleToFixed(0),  XDoubleToFixed(factor) }}
		};
		XRenderSetPictureTransform( dpy, m_pict, &xform );
		m_width = int( m_width * factor );
		m_height = int( m_height * factor );
	}

	// We don't need this image anymore
	XcursorImageDestroy( image );

	// Load the actual cursor we will use
	int size = XcursorGetDefaultSize( dpy );
	XcursorImages *images = XcursorLibraryLoadImages( name.toLatin1(), theme.toLatin1(), size );

	if ( images ) {
		m_handle = XcursorImagesLoadCursor( dpy, images );
		XcursorImagesDestroy( images );
	} else {
		images = XcursorLibraryLoadImages( "left_ptr", theme.toLatin1(), size );
		m_handle = XcursorImagesLoadCursor( dpy, images );
		XcursorImagesDestroy( images );
	}
}


PreviewCursor::~PreviewCursor()
{
    if ( m_handle ) XFreeCursor( QX11Info::display() , m_handle );
    if ( m_pict ) XRenderFreePicture( QX11Info::display(), m_pict );
}


Picture PreviewCursor::createPicture( const XcursorImage* image ) const
{
        Display *dpy = QX11Info::display();

	XImage ximage;
	ximage.width            = image->width;
	ximage.height           = image->height;
	ximage.xoffset          = 0;
	ximage.format           = ZPixmap;
	ximage.data             = reinterpret_cast<char*>( image->pixels );
	ximage.byte_order       = ImageByteOrder( dpy );
	ximage.bitmap_unit      = 32;
	ximage.bitmap_bit_order = ximage.byte_order;
	ximage.bitmap_pad       = 32;
	ximage.depth            = 32;
	ximage.bits_per_pixel   = 32;
	ximage.bytes_per_line   = image->width * 4;
	ximage.red_mask         = 0x00ff0000;
	ximage.green_mask       = 0x0000ff00;
	ximage.blue_mask        = 0x000000ff;
	ximage.obdata           = 0;

	XInitImage( &ximage );

	Pixmap pix = XCreatePixmap( dpy, DefaultRootWindow(dpy), ximage.width, ximage.height, 32 );
	GC gc = XCreateGC( dpy, pix, 0, NULL );
	XPutImage( dpy, pix, gc, &ximage, 0, 0, 0, 0, ximage.width, ximage.height );
	XFreeGC( dpy, gc );

	XRenderPictFormat *fmt = XRenderFindStandardFormat( dpy, PictStandardARGB32 );
	Picture pict = XRenderCreatePicture( dpy, pix, fmt, 0, NULL );
	XFreePixmap( dpy, pix );

	return pict;
}


void PreviewCursor::cropCursorImage( XcursorImage *&image ) const
{
	// Calculate the auto-crop rectangle
	QRect r( QPoint( image->width, image->height ), QPoint() );
	XcursorPixel *pixels = image->pixels;
	for ( int y = 0; y < int(image->height); y++ ) {
		for ( int x = 0; x < int(image->width); x++ ) {
			if ( *(pixels++) >> 24 ) {
				if ( x < r.left() )   r.setLeft( x );
				if ( x > r.right() )  r.setRight( x );
				if ( y < r.top() )    r.setTop( y );
				if ( y > r.bottom() ) r.setBottom( y );
			}
		}
	}

	// Normalize the rectangle
	r = r.normalized();

	// Don't crop the image if the size isn't going to change
	if ( r.width() == int( image->width ) && r.height() == int( image->height ) )
		return;

	// Create the new image
	XcursorImage *cropped = XcursorImageCreate( r.width(), r.height() );
	XcursorPixel *src = image->pixels + r.top() * image->width + r.left();
	XcursorPixel *dst = cropped->pixels;
	for ( int y = 0; y < r.height(); y++, src += (image->width - r.width()) ) {
		for ( int x = 0; x < r.width(); x++ ) {
			*(dst++) = *(src++);
		}
	}

	// Destroy the original
	XcursorImageDestroy( image );
	image = cropped;
}



// ------------------------------------------------------------------------------------------------



PreviewWidget::PreviewWidget( QWidget *parent )
		: QWidget( parent )
{
	cursors = new PreviewCursor* [ numCursors ];
	for ( int i = 0; i < numCursors; i++ )
		cursors[i] = new PreviewCursor;

	current = -1;
	setMouseTracking( true );
	setFixedHeight( previewSize + 20 );
}


PreviewWidget::~PreviewWidget()
{
	for ( int i = 0; i < numCursors; i++ )
		delete cursors[i];

	delete [] cursors;
}


void PreviewWidget::setTheme( const QString &theme )
{
	setUpdatesEnabled( false );

	int minHeight = previewSize + 20; // Minimum height of the preview widget
	int maxHeight = height();         // Tallest cursor height
	int maxWidth  = previewSize;      // Widest cursor width

	for ( int i = 0; i < numCursors; i++ ) {
		cursors[i]->load( cursor_names[i], theme.toLatin1() );
		if ( cursors[i]->width() > maxWidth )
			maxWidth = cursors[i]->width();
		if ( cursors[i]->height() > maxHeight )
			maxHeight = cursors[i]->height();
	}

	current = -1;
	setFixedSize( ( maxWidth + cursorSpacing ) * numCursors, qMax( maxHeight, minHeight ) );
	setUpdatesEnabled( true );
	repaint( );
}


void PreviewWidget::paintEvent( QPaintEvent * )
{
	QPixmap buffer( size() );
	QPainter p( &buffer );
	p.fillRect( rect(), palette().brush( QPalette::Background ) );
	Picture dest;

	if ( buffer.x11PictureHandle()==0 ) {
            XRenderPictFormat *fmt = XRenderFindVisualFormat( QX11Info::display(), (Visual*)buffer.x11Info().visual() );
            dest = XRenderCreatePicture( QX11Info::display(), buffer.handle(), fmt, 0, NULL );
	} else
#ifdef __GNUC__
#warning make sure x11PictureHandle is the substitute of x11RenderHandle
#endif		
		dest = buffer.x11PictureHandle();

	int rwidth = width() / numCursors;

	for ( int i = 0; i < numCursors; i++ ) {
		if ( cursors[i]->picture() ) {
                    XRenderComposite( QX11Info::display(), PictOpOver,
                                      cursors[i]->picture(), 0, dest, 0, 0, 0, 0,
                                      rwidth * i + (rwidth - cursors[i]->width()) / 2,
                                      (height() - cursors[i]->height()) / 2,
                                      cursors[i]->width(), cursors[i]->height() );
		}
	}
	p.end();

	QPainter p2( this );
	p2.drawPixmap( 0, 0, buffer );
	p2.end();

	if ( buffer.x11PictureHandle()==0 )
            XRenderFreePicture( QX11Info::display(), dest );
}


void PreviewWidget::mouseMoveEvent( QMouseEvent *e )
{
	int pos = e->x() / ( width() / numCursors );

	if ( pos != current && pos < numCursors ) {
                XDefineCursor( QX11Info::display(), winId(), cursors[pos]->handle() );
		current = pos;
	}
}


// vim: set noet ts=4 sw=4:
