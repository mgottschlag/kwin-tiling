/*
 * Copyright (C) 2003 Fredrik H�lund <fredrik@kde.org>
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <klocale.h>
#include <kaboutdata.h>
#include <kstandarddirs.h>
#include <k3listview.h>
#include <ksimpleconfig.h>
#include <kglobalsettings.h>
#include <kdialog.h>
#include <kmessagebox.h>
#include <kurlrequesterdlg.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <ktar.h>

#include <qlayout.h>
#include <qdir.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlabel.h>

#include <qpainter.h>
#include <qfileinfo.h>
#include <qpushbutton.h>

#include <cstdlib> // for getenv()

#include "themepage.h"
#include "themepage.moc"

#include "previewwidget.h"

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>
#include <kvbox.h>

// Check for older version
#if !defined(XCURSOR_LIB_MAJOR) && defined(XCURSOR_MAJOR)
# define XCURSOR_LIB_MAJOR	XCURSOR_MAJOR
# define XCURSOR_LIB_MINOR	XCURSOR_MINOR
#endif

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

	new QLabel( i18n("Select the cursor theme you want to use (hover preview to test cursor):"), this );

	// Create the preview widget
	preview = new PreviewWidget( new KHBox( this ) );

	// Create the theme list view
	listview = new K3ListView( this );
	listview->setFullWidth( true );
	listview->setAllColumnsShowFocus( true );
	listview->addColumn( i18n("Name") );
	listview->addColumn( i18n("Description") );

	connect( listview, SIGNAL(selectionChanged(Q3ListViewItem*)),
			SLOT(selectionChanged(Q3ListViewItem*)) );

	themeDirs = getThemeBaseDirs();
	insertThemes();

	KHBox *hbox = new KHBox( this );
	hbox->setSpacing( KDialog::spacingHint() );
	installButton = new QPushButton( i18n("Install New Theme..."), hbox );
	removeButton = new QPushButton( i18n("Remove Theme"), hbox );

	connect( installButton, SIGNAL( clicked() ), SLOT( installClicked() ) );
	connect( removeButton, SIGNAL( clicked() ), SLOT( removeClicked() ) );

	// Disable the install button if ~/.icons isn't writable
	QString path = QDir::homePath() + "/.icons";
	QFileInfo icons = QFileInfo( path );

	if ( ( icons.exists() && !icons.isWritable() ) ||
		( !icons.exists() && !QFileInfo( QDir::homePath() ).isWritable() ) )
		installButton->setEnabled( false );

	if ( !themeDirs.contains( path ) )
		installButton->setEnabled( false );

	selectionChanged( listview->currentItem() );
}


ThemePage::~ThemePage()
{
}


void ThemePage::save()
{
	if ( currentTheme == selectedTheme )
		return;

	KConfig c( "kcminputrc" );
	c.setGroup( "Mouse" );
	c.writeEntry( "cursorTheme", selectedTheme != "system" ? selectedTheme : QString() );

	KMessageBox::information( this, i18n("You have to restart KDE for these "
				"changes to take effect."), i18n("Cursor Settings Changed"),
				"CursorSettingsChanged" );

	currentTheme = selectedTheme;
}


void ThemePage::load()
{
	// Get the name of the theme libXcursor currently uses
	const char *theme = XcursorGetTheme( x11Display() );
	currentTheme = theme;

	// Get the name of the theme KDE is configured to use
	KConfig c( "kcminputrc" );
	c.setGroup( "Mouse" );
	currentTheme = c.readEntry( "cursorTheme", currentTheme );
        if( currentTheme.isEmpty())
            currentTheme = "system";

	// Find the theme in the listview and select it
	Q3ListViewItem *item = listview->findItem( currentTheme, DirColumn );
        if( !item )
                item = listview->findItem( "system", DirColumn );
	selectedTheme = item->text( DirColumn );
	listview->setSelected( item, true );
	listview->ensureItemVisible( item );

	// Update the preview widget as well
	if ( preview )
		preview->setTheme( selectedTheme );

	// Disable the listview if we're in kiosk mode
	if ( c.entryIsImmutable( "cursorTheme" ) )
		listview->setEnabled( false );
}


void ThemePage::defaults()
{
}


void ThemePage::selectionChanged( Q3ListViewItem *item )
{
	if ( !item )
	{
		removeButton->setEnabled( false );
		return;
	}

	selectedTheme = item->text( DirColumn );

	// Update the preview widget
	if ( preview )
		preview->setTheme( selectedTheme );

	removeButton->setEnabled( themeInfo[ selectedTheme ] && themeInfo[ selectedTheme ]->writable );

	emit changed( currentTheme != selectedTheme );
}


void ThemePage::installClicked()
{
	// Get the URL for the theme we're going to install
	KUrl url = KUrlRequesterDlg::getURL( QString(), this, i18n( "Drag or Type Theme URL" ) );
	if ( url.isEmpty() )
		return;

	QString tmpFile;
	if ( !KIO::NetAccess::download( url, tmpFile, this ) ) {
		QString text;

		if ( url.isLocalFile() )
			text = i18n( "Unable to find the cursor theme archive %1." );
		else
			text = i18n( "Unable to download the cursor theme archive; "
			             "please check that the address %1 is correct." );

		KMessageBox::sorry( this, text.arg( url.prettyURL() ) );
		return;
	}

	if ( !installThemes( tmpFile ) )
		KMessageBox::error( this, i18n( "The file %1 does not appear to be a valid "
				"cursor theme archive.").arg( url.fileName() ) );

	KIO::NetAccess::removeTempFile( tmpFile );
}


void ThemePage::removeClicked()
{
	QString question = i18n( "<qt>Are you sure you want to remove the "
		"<strong>%1</strong> cursor theme?<br>"
		"This will delete all the files installed by this theme.</qt>")
		.arg( listview->currentItem()->text( NameColumn ) );

	// Get confirmation from the user
	int answer = KMessageBox::warningContinueCancel( this, question, i18n( "Confirmation" ), KStdGuiItem::del() );
	if ( answer != KMessageBox::Continue )
		return;

	// Delete the theme from the harddrive
        KUrl u;
        u.setPath( themeInfo[  selectedTheme ]->path );
	KIO::del( u );

	// Remove the theme from the listview and from the themeinfo dict
	delete listview->findItem( selectedTheme, DirColumn );
	themeInfo.remove( selectedTheme );
	listview->setSelected( listview->currentItem(), true );

	// TODO:
	//  Since it's possible to substitute cursors in a system theme by adding a local
	//  theme with the same name, we shouldn't remove the theme from the list if it's
	//  still available elsewhere. This could be solved by calling insertThemes() here,
	//  but since KIO::del() is an asynchronos operation, the theme we're deleting will
	//  be readded to the list again before KIO has removed it.
}


bool ThemePage::installThemes( const QString &file )
{
	KTar archive( file );

	if ( !archive.open( QIODevice::ReadOnly ) )
		return false;

	const KArchiveDirectory *archiveDir = archive.directory();
	QStringList themeDirs;

	const QStringList entries = archiveDir->entries();
	for ( QStringList::ConstIterator it = entries.begin(); it != entries.end(); ++it )
	{
		const KArchiveEntry *entry = archiveDir->entry( *it );
		if ( entry->isDirectory() && entry->name().toLower() != "default" ) {
			const KArchiveDirectory *dir = static_cast< const KArchiveDirectory* >( entry );
			if ( dir->entry( "index.theme" ) && dir->entry( "cursors" ) )
				themeDirs << dir->name();
		}
	}

	if ( themeDirs.count() < 1 )
		return false;

	const QString destDir = QDir::homePath() + "/.icons/";
	KStandardDirs::makeDir( destDir ); // Make sure the directory exists

	for ( QStringList::ConstIterator it = themeDirs.begin(); it != themeDirs.end(); ++it )
	{
		// Check if a theme with that name already exists
		if ( QDir( destDir ).exists( *it ) ) {
			const QString question = i18n( "A theme named %1 already exists in your icon "
					"theme folder. Do you want replace it with this one?" ).arg( *it );
			int answer = KMessageBox::warningContinueCancel( this, question, i18n( "Overwrite Theme?"), i18n("Replace") );
			if ( answer != KMessageBox::Continue )
				continue;

			// ### If the theme that's being replaced is the current theme, it
			//     will cause cursor inconsistencies in newly started apps.
		}

		// ### Should we check if a theme with the same name exists in a global theme dir?
		//     If that's the case it will effectively replace it, even though the global theme
		//     won't be deleted. Checking for this situation is easy, since the global theme
		//     will be in the listview. Maybe this should never be allowed since it might
		//     result in strange side effects (from the average users point of view). OTOH
		//     a user might want to do this 'upgrade' a global theme.

		const QString dest = destDir + *it;
		const KArchiveDirectory *dir = static_cast< const KArchiveDirectory* >( archiveDir->entry( *it ) );
		dir->copyTo( dest );
		insertTheme( dest );
	}

	listview->sort();

	archive.close();
	return true;
}


void ThemePage::insertTheme( const QString &path )
{
	QString dirName = QDir( path ).dirName();

	// Defaults in case there's no name or comment field.
	QString name   = dirName;
	QString desc   = i18n( "No description available" );
	QString sample = "left_ptr";

	KSimpleConfig c( path + "/index.theme", true ); // Open read-only
	c.setGroup( "Icon Theme" );

	// Don't insert the theme if it's hidden.
	if ( c.readEntry( "Hidden", false) )
		return;

	// ### If the theme is hidden, the user will probably find it strange that it
	//     doesn't appear in the list view. There also won't be a way for the user
	//     to delete the theme using the KCM. Perhaps a warning about this should
	//     be issued, and the user given a chance to undo the installation.

	// Read the name, description and sample cursor
	name   = c.readEntry( "Name", name );
	desc   = c.readEntry( "Comment", desc );
	sample = c.readEntry( "Example", sample );

	// Create a ThemeInfo object if one doesn't already exist, and fill in the members
	ThemeInfo *info = themeInfo[ dirName ];
	if ( !info ) {
		info = new ThemeInfo;
		themeInfo.insert( dirName, info );
	}

	info->path     = path;
	info->writable = true;

	// If an item with the same name already exists, remove it
	delete listview->findItem( dirName, DirColumn );

	// Create the listview item and insert it into the list.
	KListViewItem *item = new KListViewItem( listview, name, desc, /*hidden*/ dirName );
	item->setPixmap( NameColumn, createIcon( dirName, sample ) );
	listview->insertItem( item );
}


const QStringList ThemePage::getThemeBaseDirs() const
{
#if XCURSOR_LIB_MAJOR == 1 && XCURSOR_LIB_MINOR < 1
	// These are the default paths Xcursor will scan for cursor themes
	QString path( "~/.icons:/usr/share/icons:/usr/share/pixmaps:/usr/X11R6/lib/X11/icons" );

	// If XCURSOR_PATH is set, use that instead of the default path
	char *xcursorPath = std::getenv( "XCURSOR_PATH" );
	if ( xcursorPath )
		path = xcursorPath;
#else
	// Get the search patch from Xcursor
	QString path = XcursorLibraryPath();
#endif
	// Expand all occurences of ~ to the home dir
	path.replace( "~/", QDir::homePath() + '/' );
	return path.split( ':');
}


bool ThemePage::isCursorTheme( const QString &theme, const int depth ) const
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
				inherit = c.readEntry( "Inherits", QStringList() );
			}

			// Recurse through the list of inherited themes
			for ( QStringList::ConstIterator it2 = inherit.begin(); it2 != inherit.end(); ++it2 )
			{
				if ( *it2 == theme ) // Avoid possible DoS
					continue;

				if ( isCursorTheme( *it2, depth + 1 ) )
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
				if ( c.readEntry( "Hidden", false) )
					continue;

				// If there's no cursors subdirectory we'll do a recursive scan
				// to check if the theme inherits a theme with one.
				if ( !haveCursors )
				{
					bool result = false;
					QStringList inherit = c.readEntry( "Inherits", QStringList() );
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

	KListViewItem *item = new KListViewItem( listview, ' ' + i18n( "No theme" ), i18n( "The old classic X cursors") , /*hidden*/ "none" );
	listview->insertItem( item );
	item = new KListViewItem( listview, ' ' + i18n( "System theme" ), i18n( "Do not change cursor theme") , /*hidden*/ "system" );
	listview->insertItem( item );
        // no ThemeInfo object for this one
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
		r = r.normalized();

		// Calculate the image size
		int size = qMax( iconSize, qMax( r.width(), r.height() ) );

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
			image = image.scaled( iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation );

		pix.convertFromImage( image );
		XcursorImageDestroy( xcur );
	} else {

		QImage image( iconSize, iconSize, 32 );
		image.setAlphaBuffer( true );

		Q_UINT32 *data = reinterpret_cast< Q_UINT32* >( image.bits() );
		for ( int i = 0; i < image.width() * image.height(); i++ )
			data[ i ] = 0;

		pix.convertFromImage( image );
	}

	return pix;
}


// vim: set noet ts=4 sw=4:
