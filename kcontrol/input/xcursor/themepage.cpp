/*
 * $Id$
 *
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <klocale.h>
#include <kaboutdata.h>
#include <kstandarddirs.h>
#include <klistview.h>
#include <ksimpleconfig.h>
#include <kglobalsettings.h>
#include <kdialog.h>

#include <qlayout.h>
#include <qdir.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qpainter.h>
#include <qfileinfo.h>
#include <qpushbutton.h>

#include <cstdlib> // for getenv()

#include "themepage.h"
#include "themepage.moc"

#include "previewwidget.h"

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>


namespace {
	// Listview icon size
	const int iconSize = 24;

	// Listview columns
	enum Columns { NameColumn = 0, DescColumn, /* hidden */ DirColumn };
}

struct ThemeInfo {
    QString path;           // Path to the cursor theme
    bool writable;          // Theme directory is writable
};


ThemePage::ThemePage( QWidget* parent, const char* name )
	: QWidget( parent, name ), selectedTheme( NULL ), currentTheme( NULL )
{
	QBoxLayout *layout = new QVBoxLayout( this );
	layout->setAutoAdd( true );
	layout->setMargin( KDialog::marginHint() );
	layout->setSpacing( KDialog::spacingHint() );

	new QLabel( i18n("Select the cursor theme you want to use:"), this );

	// Create the preview widget
	preview = new PreviewWidget( new QHBox( this ) );

	// Create the theme list view
	listview = new KListView( this );
	listview->setFullWidth( true );
	listview->setAllColumnsShowFocus( true );
	listview->addColumn( i18n("Name") );
	listview->addColumn( i18n("Description") );

	connect( listview, SIGNAL(selectionChanged(QListViewItem*)),
			SLOT(selectionChanged(QListViewItem*)) );

	themeDirs = getThemeBaseDirs();
	insertThemes();

	QHBox *hbox = new QHBox( this );
	hbox->setSpacing( 6 );
	QPushButton *button1 = new QPushButton( i18n("Install New Theme..."), hbox );
	QPushButton *button2 = new QPushButton( i18n("Remove Theme"), hbox );
	button1->setEnabled( false );
	button2->setEnabled( false );
}


ThemePage::~ThemePage()
{
}


void ThemePage::save()
{
	KConfig *c = KGlobal::config();
	c->setGroup( "KDE" );
	c->writeEntry( "cursorTheme", selectedTheme, true, true );
	c->sync();
}


void ThemePage::load()
{
	// Get the name of the theme libXcursor currently uses
	const char *theme = XcursorGetTheme( x11Display() );
	currentTheme = theme ? theme : "default";

	// Get the name of the theme KDE is configured to use
	KConfig *c = KGlobal::config();
	c->setGroup( "KDE" );
	currentTheme = c->readEntry( "cursorTheme", currentTheme );

	// Find the theme in the listview and select it
	QListViewItem *item = listview->findItem( currentTheme, DirColumn );
	if ( item ) {
		selectedTheme = item->text( DirColumn );
		listview->setSelected( item, true );
		listview->ensureItemVisible( item );
	}

	// Update the preview widget as well
	if ( preview )
		preview->setTheme( selectedTheme );

	// Disable the listview if we're in kiosk mode
	if ( c->entryIsImmutable( "cursorTheme" ) )
		listview->setEnabled( false );
}


void ThemePage::defaults()
{
}


void ThemePage::selectionChanged( QListViewItem *item )
{
	selectedTheme = item->text( DirColumn );

	// Update the preview widget
	if ( preview )
		preview->setTheme( selectedTheme );

	emit changed( currentTheme != selectedTheme );
}


const QStringList ThemePage::getThemeBaseDirs() const
{
	// These are the default paths Xcursor will scan for cursor themes
	QString path( "~/.icons:/usr/share/icons:/usr/share/pixmaps:/usr/X11R6/lib/X11/icons" );

	// If XCURSOR_PATH is set, use that instead of the default path
	char *xcursorPath = std::getenv( "XCURSOR_PATH" );
	if ( xcursorPath )
		path = xcursorPath;

	// Expand all occurences of ~ to the home dir
	path.replace( "~/", QDir::homeDirPath() + '/' );
	return QStringList::split( ':', path );
}


const bool ThemePage::isCursorTheme( const QString &theme, const int depth ) const
{
	// Prevent infinate recursion
	if ( depth > 10 )
		return false;

	// Search each icon theme directory for 'theme'
	for ( QStringList::ConstIterator it = themeDirs.begin(); it != themeDirs.end(); ++it )
	{
		QDir dir( *it  );
		if ( !dir.exists() )
			continue;

		const QStringList subdirs( dir.entryList( QDir::Dirs ) );
		if ( subdirs.contains( theme ) )
		{
			const QString path       = *it + '/' + theme;
			const QString indexfile  = path + "/index.theme";
			const bool haveIndexFile = dir.exists( indexfile );
			const bool haveCursors   = dir.exists( path + "/cursors" );
			QStringList inherit;

			// Return true if we have a cursors subdirectory
			if ( haveCursors )
				return true;

			// Parse the index.theme file if one exists
			if ( haveIndexFile )
			{
				KSimpleConfig c( indexfile, true ); // Open read-only
				c.setGroup( "Icon Theme" );
				inherit = c.readListEntry( "Inherits" );
			}

			// Recurse through the list of inherited themes
			for ( QStringList::ConstIterator it2 = inherit.begin(); it2 != inherit.end(); ++it2 )
			{
				if ( *it2 == theme ) // Avoid possible DoS
					continue;

				if ( isCursorTheme( *it2, depth + 1 ) );
					return true;
			}
		}
	}

	return false;
}


void ThemePage::insertThemes()
{
	// Scan each base dir for cursor themes and add them to the listview.
	// An icon theme is considered to be a cursor theme if it contains
	// a cursors subdirectory or if it inherits a cursor theme.
	for ( QStringList::ConstIterator it = themeDirs.begin(); it != themeDirs.end(); ++it )
	{
		QDir dir( *it );
		if ( !dir.exists() )
			continue;

		QStringList subdirs( dir.entryList( QDir::Dirs ) );
		subdirs.remove( "." );
		subdirs.remove( ".." );

		for ( QStringList::ConstIterator it = subdirs.begin(); it != subdirs.end(); ++it )
		{
			// Only add the theme if we don't already have a theme with that name
			// in the list. Xcursor will use the first theme it finds in that
			// case, and since we use the same search order that should also be
			// the theme we end up adding to the list.
			if ( listview->findItem( *it, DirColumn ) )
				continue;

			const QString path       = dir.path() + '/' + *it;
			const QString indexfile  = path + "/index.theme";
			const bool haveIndexFile = dir.exists( *it + "/index.theme" );
			const bool haveCursors   = dir.exists( *it + "/cursors" );

			// If the directory doesn't have a cursors subdir and lack an
			// index.theme file it's definately not a cursor theme
			if ( !haveIndexFile && !haveCursors )
				continue;

			// Defaults in case there's no index.theme file or it lacks
			// a name and a comment field.
			QString name   = *it;
			QString desc   = i18n( "No description available" );
			QString sample = "left_ptr";

			// Parse the index.theme file if the theme has one.
			if ( haveIndexFile )
			{
				KSimpleConfig c( indexfile, true );
				c.setGroup( "Icon Theme" );

				// Skip this theme if it's hidden.
				if ( c.readBoolEntry( "Hidden", false ) )
					continue;

				// If there's no cursors subdirectory we'll do a recursive scan
				// to check if the theme inherits a theme with one.
				if ( !haveCursors )
				{
					bool result = false;
					QStringList inherit = c.readListEntry( "Inherits" );
					for ( QStringList::ConstIterator it2 = inherit.begin(); it2 != inherit.end(); ++it2 )
						if ( result = isCursorTheme( *it2 ) )
							break;

					// If this theme doesn't inherit a cursor theme, proceed
					// to the next theme in the list.
					if ( !result )
						continue;
				}

				// Read the name, description and sample cursor
				name   = c.readEntry( "Name", name );
				desc   = c.readEntry( "Comment", desc );
				sample = c.readEntry( "Example", sample );
			}

			// Create a ThemeInfo object, and fill in the members
			ThemeInfo *info = new ThemeInfo;
			info->path     = path;
			info->writable = QFileInfo( path ).isWritable();
			themeInfo.insert( *it, info );

			// Create the listview item and insert it into the list.
			KListViewItem *item = new KListViewItem( listview, name, desc, /*hidden*/ *it );
			item->setPixmap( NameColumn, createIcon( *it, sample ) );
			listview->insertItem( item );
		}
	}

	// Note: If a default theme dir wasn't found in the above search, Xcursor will
	//       default to using the cursor font.

	// Sort the theme list
	listview->sort();
}

QPixmap ThemePage::createIcon( const QString &theme, const QString &sample ) const
{
	XcursorImage *xcur;
	QPixmap pix;

	xcur = XcursorLibraryLoadImage( sample.latin1(), theme.latin1(), iconSize );
	if ( !xcur ) xcur = XcursorLibraryLoadImage( "left_ptr", theme.latin1(), iconSize );

	if ( xcur ) {
		// Calculate an auto-crop rectangle for the cursor image
		// (helps with cursors converted from windows .cur or .ani files)
		QRect r( QPoint( xcur->width, xcur->height ), QPoint() );
		XcursorPixel *src = xcur->pixels;

		for ( int y = 0; y < int( xcur->height ); y++ ) {
			for ( int x = 0; x < int( xcur->width ); x++ ) {
				if ( *(src++) >> 24 ) {
					if ( x < r.left() )   r.setLeft( x );
					if ( x > r.right() )  r.setRight( x );
					if ( y < r.top() )    r.setTop( y );
					if ( y > r.bottom() ) r.setBottom( y );
				}
			}
		}

		// Normalize the rectangle
		r = r.normalize();

		// Calculate the image size
		int size = kMax( iconSize, kMax( r.width(), r.height() ) );

		// Create the intermediate QImage
		QImage image( size, size, 32 );
		image.setAlphaBuffer( true );

		// Clear the image
		Q_UINT32 *dst = reinterpret_cast<Q_UINT32*>( image.bits() );
		for ( int i = 0; i < image.width() * image.height(); i++ )
			dst[i] = 0;

		// Compute the source and destination offsets
		QPoint dstOffset( (image.width() - r.width()) / 2, (image.height() - r.height()) / 2 );
		QPoint srcOffset( r.topLeft() );

		dst = reinterpret_cast<Q_UINT32*>( image.scanLine(dstOffset.y()) ) + dstOffset.x();
		src = reinterpret_cast<Q_UINT32*>( xcur->pixels ) + srcOffset.y() * xcur->width + srcOffset.x();

		// Copy the XcursorImage into the QImage, converting it from premultiplied
		// to non-premultiplied alpha and cropping it if needed.
		for ( int y = 0; y < r.height(); y++ )
		{
			for ( int x = 0; x < r.width(); x++, dst++, src++ ) {
				const Q_UINT32 pixel = *src;

				const Q_UINT8 a = qAlpha( pixel );
				const Q_UINT8 r = qRed( pixel );
				const Q_UINT8 g = qGreen( pixel );
				const Q_UINT8 b = qBlue( pixel );

				if ( !a || a == 255 ) {
					*dst = pixel;
				} else {
					float alpha = a / 255.0;
					*dst = qRgba( int(r / alpha), int(g / alpha), int(b / alpha), a );
				}
			}
			dst += ( image.width() - r.width() );
			src += ( xcur->width - r.width() );
		}

		// Scale down the image if we need to
		if ( image.width() > iconSize || image.height() > iconSize )
			image = image.smoothScale( iconSize, iconSize, QImage::ScaleMin );

		pix.convertFromImage( image );
		XcursorImageDestroy( xcur );
	}

	return pix;
}


// vim: set noet ts=4 sw=4:
