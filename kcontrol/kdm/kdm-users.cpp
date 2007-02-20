/*
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdm-users.h"

#include <k3listview.h>
#include <k3urldrag.h>
#include <kcombobox.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kimagefilepreview.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kstandardguiitem.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QDir>
#include <QDragEnterEvent>
#include <QEvent>
#include <QFile>
#include <QGroupBox>
#include <QIntValidator>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QStyle>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

extern KConfig *config;

KDMUsersWidget::KDMUsersWidget( QWidget *parent )
	: QWidget( parent )
{
#ifdef __linux__
	struct stat st;
	if (!stat( "/etc/debian_version", &st )) { /* debian */
		defminuid = "1000";
		defmaxuid = "29999";
	} else if (!stat( "/usr/portage", &st )) { /* gentoo */
		defminuid = "1000";
		defmaxuid = "65000";
	} else if (!stat( "/etc/mandrake-release", &st )) { /* mandrake - check before redhat! */
		defminuid = "500";
		defmaxuid = "65000";
	} else if (!stat( "/etc/redhat-release", &st )) { /* redhat */
		defminuid = "100";
		defmaxuid = "65000";
	} else /* if (!stat( "/etc/SuSE-release", &st )) */ { /* suse */
		defminuid = "500";
		defmaxuid = "65000";
	}
#else
	defminuid = "1000";
	defmaxuid = "65000";
#endif

	// We assume that $kde_datadir/kdm exists, but better check for pics/ and pics/users,
	// and create them if necessary.
	config->setGroup( "X-*-Greeter" );
	m_userPixDir = config->readEntry( "FaceDir", KGlobal::dirs()->resourceDirs( "data" ).last() + "kdm/faces" ) + '/';
	m_notFirst = false;
	QDir testDir( m_userPixDir );
	if (!testDir.exists() && !testDir.mkdir( testDir.absolutePath() ) && !geteuid())
		KMessageBox::sorry( this, i18n("Unable to create folder %1", testDir.absolutePath() ) );

	m_defaultText = i18n("<default>");

	QString wtstr;

	minGroup = new QGroupBox( i18n("System U&IDs"), this );
	minGroup->setWhatsThis( i18n("Users with a UID (numerical user identification) outside this range will not be listed by KDM and this setup dialog."
	                             " Note that users with the UID 0 (typically root) are not affected by this and must be"
	                             " explicitly hidden in \"Not hidden\" mode.") );
	QSizePolicy sp_ign_fix( QSizePolicy::Ignored, QSizePolicy::Fixed );
	QValidator *valid = new QIntValidator( 0, 999999, minGroup );
	QLabel *minlab = new QLabel( i18n("Below:"), minGroup );
	leminuid = new KLineEdit( minGroup );
	minlab->setBuddy( leminuid );
	leminuid->setSizePolicy( sp_ign_fix );
	leminuid->setValidator( valid );
	connect( leminuid, SIGNAL(textChanged( const QString & )), SIGNAL(changed()) );
	connect( leminuid, SIGNAL(textChanged( const QString & )), SLOT(slotMinMaxChanged()) );
	QLabel *maxlab = new QLabel( i18n("Above:"), minGroup );
	lemaxuid = new KLineEdit( minGroup );
	maxlab->setBuddy( lemaxuid );
	lemaxuid->setSizePolicy( sp_ign_fix );
	lemaxuid->setValidator( valid );
	connect( lemaxuid, SIGNAL(textChanged( const QString & )), SIGNAL(changed()) );
	connect( lemaxuid, SIGNAL(textChanged( const QString & )), SLOT(slotMinMaxChanged()) );
	QGridLayout *grid = new QGridLayout( minGroup );
	grid->addWidget( minlab, 0, 0 );
	grid->addWidget( leminuid, 0, 1 );
	grid->addWidget( maxlab, 1, 0 );
	grid->addWidget( lemaxuid, 1, 1 );

	usrGroup = new QGroupBox( i18n("Users"), this );
	cbshowlist = new QCheckBox( i18n("Show list"), usrGroup );
	cbshowlist->setWhatsThis( i18n("If this option is checked, KDM will show a list of users,"
	                               " so users can click on their name or image rather than typing in their login.") );
	cbcomplete = new QCheckBox( i18n("Autocompletion"), usrGroup );
	cbcomplete->setWhatsThis( i18n("If this option is checked, KDM will automatically complete"
	                               " user names while they are typed in the line edit.") );
	cbinverted = new QCheckBox( i18n("Inverse selection"), usrGroup );
	cbinverted->setWhatsThis( i18n("This option specifies how the users for \"Show list\" and \"Autocompletion\""
	                               " are selected in the \"Select users and groups\" list: "
	                               "If not checked, select only the checked users. "
	                               "If checked, select all non-system users, except the checked ones.") );
	cbusrsrt = new QCheckBox( i18n("Sor&t users"), usrGroup );
	cbusrsrt->setWhatsThis( i18n("If this is checked, KDM will alphabetically sort the user list."
	                             " Otherwise users are listed in the order they appear in the password file.") );
	QButtonGroup *buttonGroup = new QButtonGroup( usrGroup );
	buttonGroup->setExclusive( false );
	connect( buttonGroup, SIGNAL(buttonClicked( int )), SLOT(slotShowOpts()) );
	connect( buttonGroup, SIGNAL(buttonClicked( int )), SIGNAL(changed()) );
	buttonGroup->addButton( cbshowlist );
	buttonGroup->addButton( cbcomplete );
	buttonGroup->addButton( cbinverted );
	buttonGroup->addButton( cbusrsrt );
	QBoxLayout *box = new QVBoxLayout( usrGroup );
	box->addWidget( cbshowlist );
	box->addWidget( cbcomplete );
	box->addWidget( cbinverted );
	box->addWidget( cbusrsrt );

	wstack = new QStackedWidget( this );
	s_label = new QLabel( i18n("S&elect users and groups:"), this );
	s_label->setBuddy( wstack );
	optinlv = new K3ListView( this );
	optinlv->addColumn( i18n("Selected Users") );
	optinlv->setResizeMode( Q3ListView::LastColumn );
	optinlv->setWhatsThis( i18n("KDM will show all checked users. Entries denoted with '@' are user groups. Checking a group is like checking all users in that group.") );
	wstack->addWidget( optinlv );
	connect( optinlv, SIGNAL(clicked( Q3ListViewItem * )),
	         SLOT(slotUpdateOptIn( Q3ListViewItem * )) );
	connect( optinlv, SIGNAL(clicked( Q3ListViewItem * )),
	         SIGNAL(changed()) );
	optoutlv = new K3ListView( this );
	optoutlv->addColumn( i18n("Hidden Users") );
	optoutlv->setResizeMode( Q3ListView::LastColumn );
	optoutlv->setWhatsThis( i18n("KDM will show all non-checked non-system users. Entries denoted with '@' are user groups. Checking a group is like checking all users in that group.") );
	wstack->addWidget( optoutlv );
	connect( optoutlv, SIGNAL(clicked( Q3ListViewItem * )),
	         SLOT(slotUpdateOptOut( Q3ListViewItem * )) );
	connect( optoutlv, SIGNAL(clicked( Q3ListViewItem * )),
	         SIGNAL(changed()) );

	faceGroup = new QGroupBox( i18n("User Image Source"), this );
	faceGroup->setWhatsThis( i18n("Here you can specify where KDM will obtain the images that represent users."
	                              " \"Admin\" represents the global folder; these are the pictures you can set below."
	                              " \"User\" means that KDM should read the user's $HOME/.face.icon file."
	                              " The two selections in the middle define the order of preference if both sources are available.") );
	rbadmonly = new QRadioButton( i18n("Admin"), faceGroup );
	rbprefadm = new QRadioButton( i18n("Admin, user"), faceGroup );
	rbprefusr = new QRadioButton( i18n("User, admin"), faceGroup );
	rbusronly = new QRadioButton( i18n("User"), faceGroup );
	buttonGroup = new QButtonGroup( faceGroup );
	connect( buttonGroup, SIGNAL(buttonClicked( int )), SLOT(slotFaceOpts()) );
	connect( buttonGroup, SIGNAL(buttonClicked( int )), SIGNAL(changed()) );
	buttonGroup->addButton( rbadmonly );
	buttonGroup->addButton( rbprefadm );
	buttonGroup->addButton( rbprefusr );
	buttonGroup->addButton( rbusronly );
	box = new QVBoxLayout( faceGroup );
	box->addWidget( rbadmonly );
	box->addWidget( rbprefadm );
	box->addWidget( rbprefusr );
	box->addWidget( rbusronly );

	QGroupBox *picGroup = new QGroupBox( i18n("User Images"), this );
	usercombo = new KComboBox( picGroup );
	usercombo->setWhatsThis( i18n("The user the image below belongs to.") );
	connect( usercombo, SIGNAL(activated( int )),
	         SLOT(slotUserSelected()) );
	QLabel *userlabel = new QLabel( i18n("User:"), picGroup );
	userlabel->setBuddy( usercombo );
	userbutton = new QPushButton( picGroup );
	userbutton->setAcceptDrops( true );
	userbutton->installEventFilter( this ); // for drag and drop
	uint sz = style()->pixelMetric( QStyle::PM_ButtonMargin ) * 2 + 48;
	userbutton->setFixedSize( sz, sz );
	connect( userbutton, SIGNAL(clicked()),
	         SLOT(slotUserButtonClicked()) );
	userbutton->setToolTip( i18n("Click or drop an image here") );
	userbutton->setWhatsThis( i18n("Here you can see the image assigned to the user selected in the combo box above. Click on the image button to select from a list"
	                               " of images or drag and drop your own image on to the button (e.g. from Konqueror).") );
	rstuserbutton = new QPushButton( i18n("Unset"), picGroup );
	rstuserbutton->setWhatsThis( i18n("Click this button to make KDM use the default image for the selected user.") );
	connect( rstuserbutton, SIGNAL(clicked()),
	         SLOT(slotUnsetUserPix()) );
	QGridLayout *hlpl = new QGridLayout( picGroup );
	hlpl->setSpacing( KDialog::spacingHint() );
	hlpl->addWidget( userlabel, 0, 0 );
	hlpl->addWidget( usercombo, 0, 1 ); // XXX this makes the layout too wide
	hlpl->addWidget( userbutton, 1, 0, 1, 2, Qt::AlignHCenter );
	hlpl->addWidget( rstuserbutton, 2, 0, 1, 2, Qt::AlignHCenter );

	QHBoxLayout *main = new QHBoxLayout( this );
	main->setSpacing( 10 );

	QVBoxLayout *lLayout = new QVBoxLayout();
	main->addItem( lLayout );
	lLayout->setSpacing( 10 );
	lLayout->addWidget( minGroup );
	lLayout->addWidget( usrGroup );
	lLayout->addStretch( 1 );

	QVBoxLayout *mLayout = new QVBoxLayout();
	main->addItem( mLayout );
	mLayout->setSpacing( 10 );
	mLayout->addWidget( s_label );
	mLayout->addWidget( wstack );
	mLayout->setStretchFactor( wstack, 1 );
	main->setStretchFactor( mLayout, 1 );

	QVBoxLayout *rLayout = new QVBoxLayout();
	main->addItem( rLayout );
	rLayout->setSpacing( 10 );
	rLayout->addWidget( faceGroup );
	rLayout->addWidget( picGroup );
	rLayout->addStretch( 1 );

}

void KDMUsersWidget::makeReadOnly()
{
	leminuid->setReadOnly( true );
	lemaxuid->setReadOnly( true );
	cbshowlist->setEnabled( false );
	cbcomplete->setEnabled( false );
	cbinverted->setEnabled( false );
	cbusrsrt->setEnabled( false );
	rbadmonly->setEnabled( false );
	rbprefadm->setEnabled( false );
	rbprefusr->setEnabled( false );
	rbusronly->setEnabled( false );
	wstack->setEnabled( false );
	disconnect( userbutton, SIGNAL(clicked()), this, SLOT(slotUserButtonClicked()) );
	userbutton->setAcceptDrops( false );
	rstuserbutton->setEnabled( false );
}

void KDMUsersWidget::slotShowOpts()
{
	bool en = cbshowlist->isChecked() || cbcomplete->isChecked();
	cbinverted->setEnabled( en );
	cbusrsrt->setEnabled( en );
	wstack->setEnabled( en );
	wstack->setCurrentWidget( cbinverted->isChecked() ? optoutlv : optinlv );
	en = cbshowlist->isChecked();
	faceGroup->setEnabled( en );
	if (!en) {
		usercombo->setEnabled( false );
		userbutton->setEnabled( false );
		rstuserbutton->setEnabled( false );
	} else
		slotFaceOpts();
}

void KDMUsersWidget::slotFaceOpts()
{
	bool en = !rbusronly->isChecked();
	usercombo->setEnabled( en );
	userbutton->setEnabled( en );
	if (en)
		slotUserSelected();
	else
		rstuserbutton->setEnabled( false );
}

void KDMUsersWidget::slotUserSelected()
{
	QString user = usercombo->currentText();
	QImage p;
	if (user != m_defaultText && p.load( m_userPixDir + user + ".face.icon" ))
		rstuserbutton->setEnabled( !getuid() );
	else {
		p.load( m_userPixDir + ".default.face.icon" );
		rstuserbutton->setEnabled( false );
	}
	userbutton->setPixmap( QPixmap::fromImage( p.scaled( 48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation ) ) );
}


void KDMUsersWidget::changeUserPix( const QString &pix )
{
	QString user( usercombo->currentText() );
	if (user == m_defaultText) {
		user = ".default";
		if (KMessageBox::questionYesNo( this, i18n("Save image as default image?"),
		                                QString(), KStandardGuiItem::save(),
		                                KStandardGuiItem::cancel() ) != KMessageBox::Yes)
			return;
	}

	QImage p( pix );
	if (p.isNull()) {
		KMessageBox::sorry( this,
			i18n("There was an error loading the image\n%1", pix ) );
		return;
	}

	p = p.scaled( 48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation );
	QString userpix = m_userPixDir + user + ".face.icon";
	if (!p.save( userpix, "PNG" ))
		KMessageBox::sorry( this,
			i18n("There was an error saving the image:\n%1", userpix ) );

	slotUserSelected();
}

void KDMUsersWidget::slotUserButtonClicked()
{
	KFileDialog dlg( m_notFirst ? QString() :
	                 KGlobal::dirs()->resourceDirs( "data" ).last() + "kdm/pics/users",
	                 KImageIO::pattern( KImageIO::Reading ),
	                 this );
	dlg.setOperationMode( KFileDialog::Opening );
	dlg.setCaption( i18n("Choose Image") );
	dlg.setMode( KFile::File | KFile::LocalOnly );

	KImageFilePreview *ip = new KImageFilePreview( &dlg );
	dlg.setPreviewWidget( ip );
	if (dlg.exec() != QDialog::Accepted)
		return;
	m_notFirst = true;

	changeUserPix( dlg.selectedFile() );
}

void KDMUsersWidget::slotUnsetUserPix()
{
	QFile::remove( m_userPixDir + usercombo->currentText() + ".face.icon" );
	slotUserSelected();
}

bool KDMUsersWidget::eventFilter( QObject *, QEvent *e )
{
	if (e->type() == QEvent::DragEnter) {
		QDragEnterEvent *ee = (QDragEnterEvent *)e;
		ee->setAccepted( K3URLDrag::canDecode( ee ) );
		return true;
	}

	if (e->type() == QEvent::Drop) {
		userButtonDropEvent( (QDropEvent *)e );
		return true;
	}

	return false;
}

KUrl *decodeImgDrop( QDropEvent *e, QWidget *wdg );

void KDMUsersWidget::userButtonDropEvent( QDropEvent *e )
{
	KUrl *url = decodeImgDrop( e, this );
	if (url) {
		QString pixpath;
		KIO::NetAccess::download( *url, pixpath, parentWidget() );
		changeUserPix( pixpath );
		KIO::NetAccess::removeTempFile( pixpath );
		delete url;
	}
}

void KDMUsersWidget::save()
{
	config->setGroup( "X-*-Greeter" );

	config->writeEntry( "MinShowUID", leminuid->text() );
	config->writeEntry( "MaxShowUID", lemaxuid->text() );

	config->writeEntry( "UserList", cbshowlist->isChecked() );
	config->writeEntry( "UserCompletion", cbcomplete->isChecked() );
	config->writeEntry( "ShowUsers",
	                    cbinverted->isChecked() ? "NotHidden" : "Selected" );
	config->writeEntry( "SortUsers", cbusrsrt->isChecked() );

	config->writeEntry( "HiddenUsers", hiddenUsers );
	config->writeEntry( "SelectedUsers", selectedUsers );

	config->writeEntry( "FaceSource",
	                    rbadmonly->isChecked() ? "AdminOnly" :
	                    rbprefadm->isChecked() ? "PreferAdmin" :
	                    rbprefusr->isChecked() ? "PreferUser" : "UserOnly" );
}


void KDMUsersWidget::updateOptList( Q3ListViewItem *item, QStringList &list )
{
	if (!item)
		return;
	Q3CheckListItem *itm = (Q3CheckListItem *)item;
	int ind = list.indexOf( itm->text() );
	if (itm->isOn()) {
		if (ind < 0)
			list.append( itm->text() );
	} else {
		if (ind >= 0)
			list.removeAt( ind );
	}
}

void KDMUsersWidget::slotUpdateOptIn( Q3ListViewItem *item )
{
	updateOptList( item, selectedUsers );
}

void KDMUsersWidget::slotUpdateOptOut( Q3ListViewItem *item )
{
	updateOptList( item, hiddenUsers );
}

void KDMUsersWidget::slotClearUsers()
{
	optinlv->clear();
	optoutlv->clear();
	usercombo->clear();
	usercombo->addItem( m_defaultText );
}

void KDMUsersWidget::slotAddUsers( const QMap<QString,int> &users )
{
	QMap<QString,int>::const_iterator it;
	for (it = users.begin(); it != users.end(); ++it) {
		const QString *name = &it.key();
		(new Q3CheckListItem( optinlv, *name, Q3CheckListItem::CheckBox ))->
			setOn( selectedUsers.contains( *name ) );
		(new Q3CheckListItem( optoutlv, *name, Q3CheckListItem::CheckBox ))->
			setOn( hiddenUsers.contains( *name ) );
		usercombo->addItem( *name );
	}
	optinlv->sort();
	optoutlv->sort();
	if (usercombo->model())
		usercombo->model()->sort( 0 );
}

void KDMUsersWidget::slotDelUsers( const QMap<QString,int> &users )
{
	QMap<QString,int>::const_iterator it;
	for (it = users.begin(); it != users.end(); ++it) {
		const QString *name = &it.key();
		int idx = usercombo->findText( *name );
		if (idx != -1)
			usercombo->removeItem( idx );
		delete optinlv->findItem( *name, 0 );
		delete optoutlv->findItem( *name, 0 );
	}
}

void KDMUsersWidget::load()
{
	QString str;

	config->setGroup( "X-*-Greeter" );

	selectedUsers = config->readEntry( "SelectedUsers", QStringList() );
	hiddenUsers = config->readEntry( "HiddenUsers", QStringList() );

	leminuid->setText( config->readEntry( "MinShowUID", defminuid ) );
	lemaxuid->setText( config->readEntry( "MaxShowUID", defmaxuid ) );

	cbshowlist->setChecked( config->readEntry( "UserList", true ) );
	cbcomplete->setChecked( config->readEntry( "UserCompletion", false ) );
	cbinverted->setChecked( config->readEntry( "ShowUsers" ) != "Selected" );
	cbusrsrt->setChecked( config->readEntry( "SortUsers", true ) );

	QString ps = config->readEntry( "FaceSource" );
	if (ps == QLatin1String("UserOnly"))
		rbusronly->setChecked( true );
	else if (ps == QLatin1String("PreferUser"))
		rbprefusr->setChecked( true );
	else if (ps == QLatin1String("PreferAdmin"))
		rbprefadm->setChecked( true );
	else
		rbadmonly->setChecked( true );

	slotUserSelected();

	slotShowOpts();
	slotFaceOpts();
}

void KDMUsersWidget::defaults()
{
	leminuid->setText( defminuid );
	lemaxuid->setText( defmaxuid );
	cbshowlist->setChecked( true );
	cbcomplete->setChecked( false );
	cbinverted->setChecked( true );
	cbusrsrt->setChecked( true );
	rbadmonly->setChecked( true );
	hiddenUsers.clear();
	selectedUsers.clear();
	slotShowOpts();
	slotFaceOpts();
}

void KDMUsersWidget::slotMinMaxChanged()
{
	emit setMinMaxUID( leminuid->text().toInt(), lemaxuid->text().toInt() );
}

#include "kdm-users.moc"
