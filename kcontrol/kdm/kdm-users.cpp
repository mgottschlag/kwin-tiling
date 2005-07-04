/* This file is part of the KDE Display Manager Configuration package
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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <unistd.h>
#include <sys/types.h>

#include <qstyle.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qwhatsthis.h>
#include <qvgroupbox.h>
#include <qpushbutton.h>

#include <kfiledialog.h>
#include <kimageio.h>
#include <kimagefilepreview.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kurldrag.h>

#include "kdm-users.h"

#include <sys/stat.h>


extern KSimpleConfig *config;

KDMUsersWidget::KDMUsersWidget(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
#ifdef __linux__
    struct stat st;
    if (!stat( "/etc/debian_version", &st )) {	/* debian */
	defminuid = "1000";
	defmaxuid = "29999";
    } else if (!stat( "/usr/portage", &st )) {	/* gentoo */
	defminuid = "1000";
	defmaxuid = "65000";
    } else if (!stat( "/etc/mandrake-release", &st )) {	/* mandrake - check before redhat! */
	defminuid = "500";
	defmaxuid = "65000";
    } else if (!stat( "/etc/redhat-release", &st )) {	/* redhat */
	defminuid = "100";
	defmaxuid = "65000";
    } else /* if (!stat( "/etc/SuSE-release", &st )) */ {	/* suse */
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
    m_userPixDir = config->readEntry( "FaceDir", KGlobal::dirs()->resourceDirs("data").last() + "kdm/faces" ) + '/';
    m_notFirst = false;
    QDir testDir( m_userPixDir );
    if ( !testDir.exists() && !testDir.mkdir( testDir.absPath() ) && !geteuid() )
        KMessageBox::sorry( this, i18n("Unable to create folder %1").arg( testDir.absPath() ) );

    m_defaultText = i18n("<default>");

    QString wtstr;

    minGroup = new QGroupBox( 2, Horizontal, i18n("System U&IDs"), this );
    QWhatsThis::add( minGroup, i18n("Users with a UID (numerical user identification) outside this range will not be listed by KDM and this setup dialog."
      " Note that users with the UID 0 (typically root) are not affected by this and must be"
      " explicitly hidden in \"Not hidden\" mode."));
    QSizePolicy sp_ign_fix( QSizePolicy::Ignored, QSizePolicy::Fixed );
    QValidator *valid = new QIntValidator( 0, 999999, minGroup );
    QLabel *minlab = new QLabel( i18n("Below:"), minGroup );
    leminuid = new KLineEdit( minGroup );
    minlab->setBuddy( leminuid );
    leminuid->setSizePolicy( sp_ign_fix );
    leminuid->setValidator( valid );
    connect( leminuid, SIGNAL(textChanged( const QString & )), SLOT(slotChanged()) );
    connect( leminuid, SIGNAL(textChanged( const QString & )), SLOT(slotMinMaxChanged()) );
    QLabel *maxlab = new QLabel( i18n("Above:"), minGroup );
    lemaxuid = new KLineEdit( minGroup );
    maxlab->setBuddy( lemaxuid );
    lemaxuid->setSizePolicy( sp_ign_fix );
    lemaxuid->setValidator( valid );
    connect(lemaxuid, SIGNAL(textChanged( const QString & )), SLOT(slotChanged()) );
    connect(lemaxuid, SIGNAL(textChanged( const QString & )), SLOT(slotMinMaxChanged()) );

    usrGroup = new QButtonGroup( 5, Qt::Vertical, i18n("Users"), this );
    connect( usrGroup, SIGNAL(clicked( int )), SLOT(slotShowOpts()) );
    connect( usrGroup, SIGNAL(clicked( int )), SLOT(slotChanged()) );
    cbshowlist = new QCheckBox( i18n("Show list"), usrGroup );
    QWhatsThis::add( cbshowlist, i18n("If this option is checked, KDM will show a list of users,"
      " so users can click on their name or image rather than typing in their login.") );
    cbcomplete = new QCheckBox( i18n("Autocompletion"), usrGroup );
    QWhatsThis::add( cbcomplete, i18n("If this option is checked, KDM will automatically complete"
      " user names while they are typed in the line edit.") );
    cbinverted = new QCheckBox( i18n("Inverse selection"), usrGroup );
    QWhatsThis::add( cbinverted, i18n("This option specifies how the users for \"Show list\" and \"Autocompletion\""
      " are selected in the \"Select users and groups\" list: "
      "If not checked, select only the checked users. "
      "If checked, select all non-system users, except the checked ones."));
    cbusrsrt = new QCheckBox( i18n("Sor&t users"), usrGroup );
    connect( cbusrsrt, SIGNAL(toggled( bool )), SLOT(slotChanged()) );
    QWhatsThis::add( cbusrsrt, i18n("If this is checked, KDM will alphabetically sort the user list."
      " Otherwise users are listed in the order they appear in the password file.") );

    wstack = new QWidgetStack( this );
    s_label = new QLabel( wstack, i18n("S&elect users and groups:"), this );
    optinlv = new KListView( this );
    optinlv->addColumn( i18n("Selected Users") );
    optinlv->setResizeMode( QListView::LastColumn );
    QWhatsThis::add( optinlv, i18n("KDM will show all checked users. Entries denoted with '@' are user groups. Checking a group is like checking all users in that group.") );
    wstack->addWidget( optinlv );
    connect( optinlv, SIGNAL(clicked( QListViewItem * )),
	     SLOT(slotUpdateOptIn( QListViewItem * )) );
    connect( optinlv, SIGNAL(clicked( QListViewItem * )),
	     SLOT(slotChanged()) );
    optoutlv = new KListView( this );
    optoutlv->addColumn( i18n("Hidden Users") );
    optoutlv->setResizeMode( QListView::LastColumn );
    QWhatsThis::add( optoutlv, i18n("KDM will show all non-checked non-system users. Entries denoted with '@' are user groups. Checking a group is like checking all users in that group.") );
    wstack->addWidget( optoutlv );
    connect( optoutlv, SIGNAL(clicked( QListViewItem * )),
	     SLOT(slotUpdateOptOut( QListViewItem * )) );
    connect( optoutlv, SIGNAL(clicked( QListViewItem * )),
	     SLOT(slotChanged()) );

    faceGroup = new QButtonGroup( 5, Qt::Vertical, i18n("User Image Source"), this );
    QWhatsThis::add( faceGroup, i18n("Here you can specify where KDM will obtain the images that represent users."
      " \"Admin\" represents the global folder; these are the pictures you can set below."
      " \"User\" means that KDM should read the user's $HOME/.face.icon file."
      " The two selections in the middle define the order of preference if both sources are available.") );
    connect( faceGroup, SIGNAL(clicked( int )), SLOT(slotFaceOpts()) );
    connect( faceGroup, SIGNAL(clicked( int )), SLOT(slotChanged()) );
    rbadmonly = new QRadioButton( i18n("Admin"), faceGroup );
    rbprefadm = new QRadioButton( i18n("Admin, user"), faceGroup );
    rbprefusr = new QRadioButton( i18n("User, admin"), faceGroup );
    rbusronly = new QRadioButton( i18n("User"), faceGroup );

    QGroupBox *picGroup = new QVGroupBox( i18n("User Images"), this );
    QWidget *hlpw = new QWidget( picGroup );
    usercombo = new KComboBox( hlpw );
    QWhatsThis::add( usercombo, i18n("The user the image below belongs to.") );
    connect( usercombo, SIGNAL(activated( int )),
	     SLOT(slotUserSelected()) );
    QLabel *userlabel = new QLabel( usercombo, i18n("User:"), hlpw );
    userbutton = new QPushButton( hlpw );
    userbutton->setAcceptDrops( true );
    userbutton->installEventFilter( this ); // for drag and drop
    uint sz = style().pixelMetric( QStyle::PM_ButtonMargin ) * 2 + 48;
    userbutton->setFixedSize( sz, sz );
    connect( userbutton, SIGNAL(clicked()),
	     SLOT(slotUserButtonClicked()) );
    QToolTip::add( userbutton, i18n("Click or drop an image here") );
    QWhatsThis::add( userbutton, i18n("Here you can see the image assigned to the user selected in the combo box above. Click on the image button to select from a list"
      " of images or drag and drop your own image on to the button (e.g. from Konqueror).") );
    rstuserbutton = new QPushButton( i18n("Unset"), hlpw );
    QWhatsThis::add( rstuserbutton, i18n("Click this button to make KDM use the default image for the selected user.") );
    connect( rstuserbutton, SIGNAL(clicked()),
	     SLOT(slotUnsetUserPix()) );
    QGridLayout *hlpl = new QGridLayout( hlpw, 3, 2, 0, KDialog::spacingHint() );
    hlpl->addWidget( userlabel, 0, 0 );
//    hlpl->addSpacing( KDialog::spacingHint() );
    hlpl->addWidget( usercombo, 0, 1 );
    hlpl->addMultiCellWidget( userbutton, 1,1, 0,1, AlignHCenter );
    hlpl->addMultiCellWidget( rstuserbutton, 2,2, 0,1, AlignHCenter );

    QHBoxLayout *main = new QHBoxLayout( this, 10 );

    QVBoxLayout *lLayout = new QVBoxLayout( main, 10 );
    lLayout->addWidget( minGroup );
    lLayout->addWidget( usrGroup );
    lLayout->addStretch( 1 );

    QVBoxLayout *mLayout = new QVBoxLayout( main, 10 );
    mLayout->addWidget( s_label );
    mLayout->addWidget( wstack );
    mLayout->setStretchFactor( wstack, 1 );
    main->setStretchFactor( mLayout, 1 );

    QVBoxLayout *rLayout = new QVBoxLayout( main, 10 );
    rLayout->addWidget( faceGroup );
    rLayout->addWidget( picGroup );
    rLayout->addStretch( 1 );

}

void KDMUsersWidget::makeReadOnly()
{
    leminuid->setReadOnly(true);
    lemaxuid->setReadOnly(true);
    cbshowlist->setEnabled(false);
    cbcomplete->setEnabled(false);
    cbinverted->setEnabled(false);
    cbusrsrt->setEnabled(false);
    rbadmonly->setEnabled(false);
    rbprefadm->setEnabled(false);
    rbprefusr->setEnabled(false);
    rbusronly->setEnabled(false);
    wstack->setEnabled(false);
    disconnect( userbutton, SIGNAL(clicked()), this, SLOT(slotUserButtonClicked()) );
    userbutton->setAcceptDrops(false);
    rstuserbutton->setEnabled(false);
}

void KDMUsersWidget::slotShowOpts()
{
    bool en = cbshowlist->isChecked() || cbcomplete->isChecked();
    cbinverted->setEnabled( en );
    cbusrsrt->setEnabled( en );
    wstack->setEnabled( en );
    wstack->raiseWidget( cbinverted->isChecked() ? optoutlv : optinlv );
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
    if (user != m_defaultText &&
	p.load( m_userPixDir + user + ".face.icon" )) {
	rstuserbutton->setEnabled( !getuid() );
    } else {
	p.load( m_userPixDir + ".default.face.icon" );
	rstuserbutton->setEnabled( false );
    }
    userbutton->setPixmap( p.smoothScale( 48, 48, QImage::ScaleMin ) );
}


void KDMUsersWidget::changeUserPix(const QString &pix)
{
    QString user( usercombo->currentText() );
    if (user == m_defaultText)
    {
       user = ".default";
       if (KMessageBox::questionYesNo(this, i18n("Save image as default image?"))
            != KMessageBox::Yes)
          return;
    }

    QImage p( pix );
    if (p.isNull()) {
	KMessageBox::sorry( this,
			    i18n("There was an error loading the image\n"
				 "%1").arg( pix ) );
	return;
    }

    p = p.smoothScale( 48, 48, QImage::ScaleMin );
    QString userpix = m_userPixDir + user + ".face.icon";
    if (!p.save( userpix, "PNG" ))
        KMessageBox::sorry(this,
	    i18n("There was an error saving the image:\n%1")
		.arg( userpix ) );

    slotUserSelected();
}

void KDMUsersWidget::slotUserButtonClicked()
{
    KFileDialog dlg(m_notFirst ? QString::null :
	KGlobal::dirs()->resourceDirs("data").last() + "kdm/pics/users",
                    KImageIO::pattern( KImageIO::Reading ),
                    this, 0, true);
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

bool KDMUsersWidget::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::DragEnter) {
	QDragEnterEvent *ee = (QDragEnterEvent *) e;
	ee->accept( KURLDrag::canDecode(ee) );
        return true;
    }

    if (e->type() == QEvent::Drop) {
        userButtonDropEvent((QDropEvent *) e);
        return true;
    }

    return false;
}

KURL *decodeImgDrop(QDropEvent *e, QWidget *wdg);

void KDMUsersWidget::userButtonDropEvent(QDropEvent *e)
{
    KURL *url = decodeImgDrop(e, this);
    if (url) {
	QString pixpath;
	KIO::NetAccess::download(*url, pixpath, parentWidget());
	changeUserPix( pixpath );
	KIO::NetAccess::removeTempFile(pixpath);
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


void KDMUsersWidget::updateOptList( QListViewItem *item, QStringList &list )
{
    if ( !item )
        return;
    QCheckListItem *itm = (QCheckListItem *)item;
    QStringList::iterator it = list.find( itm->text() );
    if (itm->isOn()) {
	if (it == list.end())
	    list.append( itm->text() );
    } else {
	if (it != list.end())
	    list.remove( it );
    }
}

void KDMUsersWidget::slotUpdateOptIn( QListViewItem *item )
{
    updateOptList( item, selectedUsers );
}

void KDMUsersWidget::slotUpdateOptOut( QListViewItem *item )
{
    updateOptList( item, hiddenUsers );
}

void KDMUsersWidget::slotClearUsers()
{
    optinlv->clear();
    optoutlv->clear();
    usercombo->clear();
    usercombo->insertItem( m_defaultText );
}

void KDMUsersWidget::slotAddUsers(const QMap<QString,int> &users)
{
    QMapConstIterator<QString,int> it;
    for (it = users.begin(); it != users.end(); ++it) {
      const QString *name = &it.key();
      (new QCheckListItem(optinlv, *name, QCheckListItem::CheckBox))->
	      setOn(selectedUsers.find(*name) != selectedUsers.end());
      (new QCheckListItem(optoutlv, *name, QCheckListItem::CheckBox))->
	      setOn(hiddenUsers.find(*name) != hiddenUsers.end());
      usercombo->insertItem(*name);
    }
    optinlv->sort();
    optoutlv->sort();
    if (usercombo->listBox())
        usercombo->listBox()->sort();
}

void KDMUsersWidget::slotDelUsers(const QMap<QString,int> &users)
{
    QMapConstIterator<QString,int> it;
    for (it = users.begin(); it != users.end(); ++it) {
        const QString *name = &it.key();
        if (usercombo->listBox())
            delete usercombo->listBox()->findItem( *name, ExactMatch | CaseSensitive );
        delete optinlv->findItem( *name, 0 );
        delete optoutlv->findItem( *name, 0 );
    }
}

void KDMUsersWidget::load()
{
    QString str;

    config->setGroup("X-*-Greeter");

    selectedUsers = config->readListEntry( "SelectedUsers");
    hiddenUsers = config->readListEntry( "HiddenUsers");

    leminuid->setText(config->readEntry("MinShowUID", defminuid));
    lemaxuid->setText(config->readEntry("MaxShowUID", defmaxuid));

    cbshowlist->setChecked( config->readBoolEntry( "UserList", true ) );
    cbcomplete->setChecked( config->readBoolEntry( "UserCompletion", false ) );
    cbinverted->setChecked( config->readEntry( "ShowUsers" ) != "Selected" );
    cbusrsrt->setChecked(config->readBoolEntry("SortUsers", true));

    QString ps = config->readEntry( "FaceSource" );
    if (ps == QString::fromLatin1("UserOnly"))
	rbusronly->setChecked(true);
    else if (ps == QString::fromLatin1("PreferUser"))
	rbprefusr->setChecked(true);
    else if (ps == QString::fromLatin1("PreferAdmin"))
	rbprefadm->setChecked(true);
    else
	rbadmonly->setChecked(true);

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

void KDMUsersWidget::slotChanged()
{
  emit changed(true);
}

#include "kdm-users.moc"
