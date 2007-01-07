/***************************************************************************
 *   Copyright (C) 2005-2006 by Stephen Leaf <smileaf@smileaf.org>         *
 *   Copyright (C) 2006 by Oswald Buddenhagen <ossi@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "kdm-theme.h"

#include <kdialog.h>
#include <kglobal.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprogressdialog.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <kurlrequester.h>
#include <kurlrequesterdialog.h>

#include <QDir>
#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QPixmap>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWhatsThis>
#include <QWidget>

#include <unistd.h>

extern KSimpleConfig *config;

class ThemeData : public QTreeWidgetItem {
  public:
	ThemeData( QTreeWidget *parent = 0 ) : QTreeWidgetItem( parent ) {}

	QString path;
	QString screenShot;
	QString copyright;
	QString description;
};

KDMThemeWidget::KDMThemeWidget( QWidget *parent )
	: QWidget( parent )
{
	QGridLayout *ml = new QGridLayout( this );
	ml->setSpacing( KDialog::spacingHint() );
	ml->setMargin( KDialog::marginHint() );

	themeWidget = new QTreeWidget( this );
	themeWidget->setHeaderLabels( QStringList() << i18n("Theme") << i18n("Author") );
	themeWidget->setSortingEnabled( true );
	themeWidget->sortItems( 0, Qt::AscendingOrder );
	themeWidget->setRootIsDecorated( false );
	themeWidget->setWhatsThis( i18n("This is a list of installed themes.\n"
	                                "Click the one to be used.") );

	ml->addWidget( themeWidget, 0, 0, 2, 4 );

	preview = new QLabel( this );
	preview->setFixedSize( QSize( 200, 150 ) );
	preview->setScaledContents( true );
	preview->setWhatsThis( i18n("This is a screen shot of what KDM will look like.") );

	ml->addWidget( preview, 0, 4 );

	info = new QLabel( this );
	info->setMaximumWidth( 200 );
	info->setAlignment( Qt::AlignTop );
	info->setWordWrap( true );
	info->setWhatsThis( i18n("This contains information about the selected theme.") );

	ml->addWidget( info, 1, 4 );

	bInstallTheme = new QPushButton( i18n("Install &new theme"), this );
	bInstallTheme->setWhatsThis( i18n("This will install a theme into the theme directory.") );

	ml->addWidget( bInstallTheme, 2, 0 );

	bRemoveTheme = new QPushButton( i18n("&Remove theme"), this );
	bRemoveTheme->setWhatsThis( i18n("This will remove the selected theme.") );

	ml->addWidget( bRemoveTheme, 2, 1 );

	connect( themeWidget, SIGNAL(itemSelectionChanged()), SLOT(themeSelected()) );
	connect( bInstallTheme, SIGNAL(clicked()), SLOT(installNewTheme()) );
	connect( bRemoveTheme, SIGNAL(clicked()), SLOT(removeSelectedThemes()) );

	themeDir = KGlobal::dirs()->resourceDirs( "data" ).last() + "kdm/themes/";
	QDir testDir( themeDir );
	if (!testDir.exists() && !testDir.mkdir( testDir.absolutePath() ) && !geteuid())
		KMessageBox::sorry( this, i18n("Unable to create folder %1", testDir.absolutePath() ) );

	foreach (QString ent,
	         QDir( themeDir ).entryList( QDir::Dirs | QDir::NoDotAndDotDot,
	                                     QDir::Unsorted ))
		insertTheme( themeDir + ent );
}

void KDMThemeWidget::selectTheme( const QString &path )
{
	for (int i = 0; i < themeWidget->topLevelItemCount(); i++) {
		ThemeData *td = (ThemeData *)themeWidget->topLevelItem( i );
		if (td->path == path) {
			themeWidget->clearSelection();
			td->setSelected( true );
			updateInfoView( td );
		}
	}
}

void KDMThemeWidget::load()
{
	config->setGroup( "X-*-Greeter" );

	selectTheme( config->readEntry( "Theme", themeDir + "circles" ) );
}

void KDMThemeWidget::save()
{
	config->setGroup( "X-*-Greeter" );

	config->writeEntry( "Theme", defaultTheme ? defaultTheme->path : "" );
}

void KDMThemeWidget::defaults()
{
	selectTheme( themeDir + "circles" );

	emit changed();
}

void KDMThemeWidget::makeReadOnly()
{
	themeWidget->setEnabled( false );
	bInstallTheme->setEnabled( false );
	bRemoveTheme->setEnabled( false );
}

void KDMThemeWidget::insertTheme( const QString &_theme )
{
	KSimpleConfig themeConfig( _theme + "/KdmGreeterTheme.desktop" );

	themeConfig.setGroup( "KdmGreeterTheme" );
	QString name = themeConfig.readEntry( "Name" );
	if (name.isEmpty())
		return;

	ThemeData *child = new ThemeData( themeWidget );
	child->setText( 0, name );
	child->setText( 1, themeConfig.readEntry( "Author" ) );
	child->path = _theme;
	child->screenShot = themeConfig.readEntry( "Screenshot" );
	child->copyright = themeConfig.readEntry( "Copyright" );
	child->description = themeConfig.readEntry( "Description" );
}

void KDMThemeWidget::updateInfoView( ThemeData *theme )
{
	if (!(defaultTheme = theme)) {
		info->setText( QString() );
		preview->setPixmap( QPixmap() );
		preview->setText( QString() );
	} else {
		info->setText(
			((theme->copyright.length() > 0) ?
				i18n("<qt><strong>Copyright:</strong> %1<br/></qt>",
					theme->copyright) : "") +
			((theme->description.length() > 0) ?
				i18n("<qt><strong>Description:</strong> %1</qt>",
					theme->description) : "") );
		preview->setPixmap( theme->path + '/' + theme->screenShot );
		preview->setText( theme->screenShot.isEmpty() ?
			"Screenshot not available" : QString() );
	}
}

// Theme installation code inspired by kcm_icon
void KDMThemeWidget::installNewTheme()
{
	KUrlRequesterDialog fileRequester( QString::null, i18n("Drag or Type Theme URL"), this );
	fileRequester.urlRequester()->setMode( KFile::File | KFile::Directory | KFile::ExistingOnly );
	KUrl themeURL = fileRequester.getUrl();
	if (themeURL.isEmpty())
		return;

#if 0
	if (themeURL.isLocalFile() && QDir( themeURL.path() ).exists()) {
		insertTheme( themeURL.path() );
		emit changed();
		return;
	}
#endif

	QString themeTmpFile;

	if (!KIO::NetAccess::download( themeURL, themeTmpFile, this )) {
		QString sorryText;
		if (themeURL.isLocalFile())
			sorryText = i18n("Unable to find the KDM theme archive %1.",themeURL.prettyUrl());
		else
			sorryText = i18n("Unable to download the KDM theme archive;\n"
			                 "please check that address %1 is correct.",themeURL.prettyUrl());
		KMessageBox::sorry( this, sorryText );
		return;
	}

	QList<const KArchiveDirectory *> foundThemes;

	KTar archive( themeTmpFile );
	archive.open( IO_ReadOnly );

	const KArchiveDirectory *archDir = archive.directory();
	foreach (QString ent, archDir->entries()) {
		const KArchiveEntry *possibleDir = archDir->entry( ent );
		if (possibleDir->isDirectory()) {
			const KArchiveDirectory *subDir =
				static_cast<const KArchiveDirectory *>( possibleDir );
			if (subDir->entry( "KdmGreeterTheme.desktop" ))
				foundThemes.append( subDir );
		}
	}

	if (foundThemes.isEmpty())
		KMessageBox::error( this, i18n("The file is not a valid KDM theme archive.") );
	else {
		KProgressDialog progressDiag( this,
			i18n("Installing KDM themes"), QString(), true );
		progressDiag.setAutoClose( true );
		progressDiag.progressBar()->setMaximum( foundThemes.size() );
		progressDiag.show();

		foreach (const KArchiveDirectory *ard, foundThemes) {
			progressDiag.setLabel(
				i18n("<qt>Installing <strong>%1</strong> theme</qt>", ard->name() ) );

			QString path = themeDir + ard->name();
			ard->copyTo( path, true );
			if (QDir( path ).exists())
				insertTheme( path );

			progressDiag.progressBar()->setValue( progressDiag.progressBar()->value() + 1 );
			if (progressDiag.wasCancelled())
				break;
		}
	}

	archive.close();

	KIO::NetAccess::removeTempFile( themeTmpFile );
	emit changed();
}

void KDMThemeWidget::themeSelected()
{
	if (themeWidget->selectedItems().size() > 0)
		updateInfoView( (ThemeData *)(themeWidget->selectedItems().first()) );
	else
		updateInfoView( 0 );
	emit changed();
}

void KDMThemeWidget::removeSelectedThemes()
{
	QStringList delList, nameList;
	QList<QTreeWidgetItem *> themes = themeWidget->selectedItems();
	if (themes.isEmpty())
		return;
	foreach (QTreeWidgetItem *itm, themes) {
		nameList.append( itm->text( 0 ) );
		delList.append( ((ThemeData *)itm)->path );
	}
	if (KMessageBox::questionYesNoList( this,
			i18n("Are you sure you want to remove the following themes?"),
			nameList, i18n("Remove themes?") ) != KMessageBox::Yes)
		return;
	KIO::del( KUrl::List( delList ) ); // XXX error check

	foreach (QTreeWidgetItem *itm, themes)
		themeWidget->takeTopLevelItem( themeWidget->indexOfTopLevelItem( itm ) );
}

#include "kdm-theme.moc"
