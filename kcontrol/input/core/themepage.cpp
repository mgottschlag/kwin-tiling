/*
 * Copyright (C) 2003 Fredrik Höglund <fredrik@kde.org>
 *
 * Based on the large cursor code written by Rik Hemsley,
 * Copyright (c) 2000 Rik Hemsley <rik@kde.org>
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
#include <kstandarddirs.h>
#include <kurl.h>
#include <kprocess.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kconfig.h>
#include <klistview.h>
#include <kdialog.h>

#include <qlayout.h>
#include <qdir.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlabel.h>

#include "themepage.h"
#include "themepage.moc"

#include "bitmaps.h"


namespace {
	// Listview columns
	enum Columns { NameColumn = 0, DescColumn, /* hidden */ DirColumn };
}


ThemePage::ThemePage( QWidget* parent, const char* name )
	: QWidget( parent, name )
{
	QBoxLayout *layout = new QVBoxLayout( this );
	layout->setAutoAdd( true );
	layout->setMargin( KDialog::marginHint() );
	layout->setSpacing( KDialog::spacingHint() );

	new QLabel( i18n("Select the cursor theme you want to use:"), this );

	// Create the theme list view
	listview = new KListView( this );
	listview->setFullWidth( true );
	listview->setAllColumnsShowFocus( true );
	listview->addColumn( i18n("Name") );
	listview->addColumn( i18n("Description") );

	connect( listview, SIGNAL(selectionChanged(Q3ListViewItem*)),
			SLOT(selectionChanged(Q3ListViewItem*)) );

	insertThemes();
}


ThemePage::~ThemePage()
{
}


void ThemePage::selectionChanged( Q3ListViewItem *item )
{
	selectedTheme = item->text( DirColumn );
	emit changed( selectedTheme != currentTheme );
}


void ThemePage::save()
{
	if ( currentTheme == selectedTheme )
		return;

	bool whiteCursor = selectedTheme.right( 5 ) == "White";
	bool largeCursor = selectedTheme.left( 5 ) == "Large";

	KConfig c( "kcminputrc" );
	c.setGroup( "Mouse" );
	c.writeEntry( "LargeCursor", largeCursor );
	c.writeEntry( "WhiteCursor", whiteCursor );

	currentTheme = selectedTheme;

	fixCursorFile();

	KMessageBox::information( this, i18n("You have to restart KDE for these "
				"changes to take effect."), i18n("Cursor Settings Changed"),
				"CursorSettingsChanged" );
}


void ThemePage::load()
{
	bool largeCursor, whiteCursor;

	KConfig c( "kcminputrc" );
	c.setGroup( "Mouse" );
	largeCursor = c.readEntry( "LargeCursor", false);
	whiteCursor = c.readEntry( "WhiteCursor", false);

	if ( largeCursor )
		currentTheme = whiteCursor ? "LargeWhite" : "LargeBlack";
	else
		currentTheme = whiteCursor ? "SmallWhite" : "SmallBlack";

	selectedTheme = currentTheme;
	Q3ListViewItem *item = listview->findItem( currentTheme, DirColumn );
	item->setSelected( true );
}


void ThemePage::defaults()
{
	currentTheme = selectedTheme = "SmallBlack";
	Q3ListViewItem *item = listview->findItem( currentTheme, DirColumn );
	item->setSelected( true );
}


void ThemePage::insertThemes()
{
	KListViewItem *item;

	item = new KListViewItem( listview, i18n("Small black"),
			i18n("Small black cursors"), "SmallBlack" );
	item->setPixmap( 0, QPixmap( arrow_small_black_xpm ) );
	listview->insertItem( item );

	item = new KListViewItem( listview, i18n("Large black"),
			i18n("Large black cursors"), "LargeBlack" );
	item->setPixmap( 0, QPixmap( arrow_large_black_xpm ) );
	listview->insertItem( item );

	item = new KListViewItem( listview, i18n("Small white"),
			i18n("Small white cursors"), "SmallWhite" );
	item->setPixmap( 0, QPixmap( arrow_small_white_xpm ) );
	listview->insertItem( item );

	item = new KListViewItem( listview, i18n("Large white"),
			i18n("Large white cursors"), "LargeWhite" );
	item->setPixmap( 0, QPixmap( arrow_large_white_xpm ) );
	listview->insertItem( item );
}


void ThemePage::fixCursorFile()
{
	// Make sure we have the 'font' resource dir registered and can find the
	// override dir.
	//
	// Next, if the user wants large cursors, copy the font
	// cursor_large.pcf.gz to (localkdedir)/share/fonts/override/cursor.pcf.gz.
	// Else remove the font cursor.pcf.gz from (localkdedir)/share/fonts/override.
	//
	// Run mkfontdir to update fonts.dir in that dir.

	KGlobal::dirs()->addResourceType( "font", "share/fonts/" );
	KIO::mkdir( KUrl::fromPathOrURL(QDir::homePath() + "/.fonts/kde-override") );
	QString overrideDir = QDir::homePath() + "/.fonts/kde-override/";

	KUrl installedFont;
	installedFont.setPath( overrideDir + "cursor.pcf.gz" );

	if ( currentTheme == "SmallBlack" )
		KIO::NetAccess::del( installedFont, this );
	else {
		KUrl source;

		if ( currentTheme == "LargeBlack" )
			source.setPath( locate("data", "kcminput/cursor_large_black.pcf.gz") );
		else if ( currentTheme == "LargeWhite" )
			source.setPath( locate("data", "kcminput/cursor_large_white.pcf.gz") );
		else if ( currentTheme == "SmallWhite" )
			source.setPath( locate("data", "kcminput/cursor_small_white.pcf.gz") );

		KIO::NetAccess::file_copy( source, installedFont, -1, true );
	}

	QString cmd = KGlobal::dirs()->findExe( "mkfontdir" );
	if ( !cmd.isEmpty() )
	{
		KProcess p;
		p << cmd << overrideDir;
		p.start(KProcess::Block);
	}
}

// vim: set noet ts=4 sw=4:
