    /*

    Shutdown dialog

    Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000-2003 Oswald Buddenhagen <ossi@kde.org>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */

#include "kdmshutdown.h"
#include "kdmconfig.h"
#include "liloinfo.h"
#include "kdm_greet.h"

#include <kapplication.h>
#include <kseparator.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include <qcombobox.h>
#include <qvbuttongroup.h>
#include <qstyle.h>
#include <qlayout.h>
#include <qaccel.h>
#include <qpopupmenu.h>

int KDMShutdown::curPlugin = -1;
PluginList KDMShutdown::pluginList;

KDMShutdown::KDMShutdown( QWidget *_parent )
    : inherited( _parent )
    , whenGroup( 0 )
    , verify( 0 )
    , needRoot( -1 )
#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
    , liloInfo( 0 )
#endif
{
    QSizePolicy fp( QSizePolicy::Fixed, QSizePolicy::Fixed );

    QVBoxLayout *box = new QVBoxLayout( winFrame, 10 );

    QHBoxLayout *hlay = new QHBoxLayout( box );

    howGroup = new QVButtonGroup( i18n("Shutdown Type"), winFrame );
    hlay->addWidget( howGroup, 0, AlignTop );

    QRadioButton *rb;
    rb = new KDMRadioButton( i18n("&Turn off computer"), howGroup );
    // Default action
    rb->setChecked( true );
    rb->setFocus();

    restart_rb = new KDMRadioButton( i18n("&Restart computer"), howGroup );

    connect( rb, SIGNAL(doubleClicked()), SLOT(accept()) );
    connect( restart_rb, SIGNAL(doubleClicked()), SLOT(accept()) );

#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
    if (kdmcfg->_useLilo) {
	QWidget *hlp = new QWidget( howGroup );
	QHBoxLayout *hb = new QHBoxLayout( hlp );
	int spc = kapp->style().pixelMetric(QStyle::PM_ExclusiveIndicatorWidth)
	    + howGroup->insideSpacing();
	hb->addSpacing( spc );
	targets = new QComboBox( hlp );
	targets->setSizePolicy( fp );
	hlp->setFixedSize( targets->width() + spc, targets->height() );
	hb->addWidget( targets );

	// fill combo box with contents of lilo config
	liloInfo = new LiloInfo( kdmcfg->_liloCmd, kdmcfg->_liloMap );

	QStringList list;
	if (!liloInfo->getBootOptions( list, defaultLiloTarget )) {
	    oldLiloTarget = defaultLiloTarget;
	    targets->insertStringList( list );
	    QString nextOption;
	    liloInfo->getNextBootOption( nextOption );
	    if (!nextOption.isEmpty()) {
		int idx = list.findIndex( nextOption );
		if (idx < 0) {
		    targets->insertItem( nextOption );
		    oldLiloTarget = list.count();
		} else
		    oldLiloTarget = idx;
	    }
	    targets->setCurrentItem( oldLiloTarget );
	    connect( targets, SIGNAL(activated(int)),
		     SLOT(slotTargetChanged()) );
	}
    }
#endif

    howGroup->setSizePolicy( fp );

    if (!kdmcfg->_interactiveSd) {
	whenGroup = new QVButtonGroup( i18n("Shutdown Mode"), winFrame );
	hlay->addWidget( whenGroup, 0, AlignTop );

	rb = new QRadioButton( i18n("verb!", "&Schedule"), whenGroup );
	if (kdmcfg->_defSdMode == SHUT_SCHEDULE)
	    rb->setChecked( true );

	force_rb = new QRadioButton( i18n("&Force now"), whenGroup );
	// Default action
	if (kdmcfg->_defSdMode == SHUT_FORCENOW)
	    force_rb->setChecked( true );

	try_rb = new QRadioButton( i18n("Try &now"), whenGroup );
	if (kdmcfg->_defSdMode == SHUT_TRYNOW)
	    try_rb->setChecked( true );

	if (kdmcfg->_allowNuke == SHUT_NONE)
	    force_rb->setEnabled( false );

	connect( whenGroup, SIGNAL(clicked(int)), SLOT(slotWhenChanged()) );

	whenGroup->setSizePolicy( fp );
    }

    if (kdmcfg->_allowShutdown == SHUT_ROOT ||
	(!kdmcfg->_interactiveSd && kdmcfg->_allowNuke == SHUT_ROOT))
    {
	if (curPlugin < 0) {
	    curPlugin = 0;
	    pluginList = KGVerify::init( kdmcfg->_pluginsShutdown );
	}
	verify = new KGVerify( this, winFrame,
			       whenGroup ? whenGroup : howGroup, "root",
			       pluginList, KGreeterPlugin::Authenticate,
			       KGreeterPlugin::Shutdown );
	verify->selectPlugin( curPlugin );
	box->addLayout( verify->getLayout() );
	QAccel *accel = new QAccel( winFrame );
	accel->insertItem( ALT+Key_A, 0 );
	connect( accel, SIGNAL(activated(int)), SLOT(slotActivatePlugMenu()) );
    }

    box->addWidget( new KSeparator( KSeparator::HLine, winFrame ) );

    okButton = new KPushButton( KStdGuiItem::ok(), winFrame );
    okButton->setSizePolicy( fp );
    okButton->setDefault( true );
    cancelButton = new KPushButton( KStdGuiItem::cancel(), winFrame );
    cancelButton->setSizePolicy( fp );

    hlay = new QHBoxLayout( box );
    hlay->addStretch( 1 );
    hlay->addWidget( okButton );
    hlay->addStretch( 1 );
    hlay->addWidget( cancelButton );
    hlay->addStretch( 1 );

    connect( okButton, SIGNAL(clicked()), SLOT(accept()) );
    connect( cancelButton, SIGNAL(clicked()), SLOT(reject()) );

    slotWhenChanged();
}

KDMShutdown::~KDMShutdown()
{
#if defined(__linux__) && ( defined(__i386__)  || defined(__amd64__) )
    delete liloInfo;
#endif
    hide();
    delete verify;
}

void
KDMShutdown::slotActivatePlugMenu()
{
    if (needRoot) {
	QPopupMenu *cmnu = verify->getPlugMenu();
	QSize sh( cmnu->sizeHint() / 2 );
	cmnu->exec( geometry().center() - QPoint( sh.width(), sh.height() ) );
    }
}

void
KDMShutdown::accept()
{
    if (needRoot == 1)
	verify->accept();
    else
	bye_bye();
}

void
KDMShutdown::slotTargetChanged()
{
#if defined(__linux__) && ( defined(__i386__)  || defined(__amd64__) )
    restart_rb->setChecked( true );
#endif
}

void
KDMShutdown::slotWhenChanged()
{
    int nNeedRoot = kdmcfg->_allowShutdown == SHUT_ROOT ||
		    (kdmcfg->_allowNuke == SHUT_ROOT && force_rb->isChecked());
    if (verify && nNeedRoot != needRoot) {
	if (needRoot == 1)
	    verify->abort();
	needRoot = nNeedRoot;
	verify->setEnabled( needRoot );
	if (needRoot)
	    verify->start();
    }
}

void
KDMShutdown::bye_bye()
{
#if defined(__linux__) && ( defined(__i386__)  || defined(__amd64__) )
    if (kdmcfg->_useLilo && restart_rb->isChecked()) {
	if (targets->currentItem() != oldLiloTarget) {
	    if (targets->currentItem() == defaultLiloTarget)
		liloInfo->setNextBootOption( "" );
	    else
		liloInfo->setNextBootOption( targets->currentText() );
	}
    }
#endif
    GSendInt( G_Shutdown );
    GSendInt( restart_rb->isChecked() ? SHUT_REBOOT : SHUT_HALT );
    GSendInt( kdmcfg->_interactiveSd ? SHUT_INTERACT :
	      force_rb->isChecked() ? SHUT_FORCENOW :
	      try_rb->isChecked() ? SHUT_TRYNOW : SHUT_SCHEDULE );
    inherited::accept();
}

void
KDMShutdown::verifyPluginChanged( int id )
{
    curPlugin = id;
    adjustSize();
}

void
KDMShutdown::verifyOk()
{
    bye_bye();
}

void
KDMShutdown::verifyFailed()
{
    okButton->setEnabled( false );
    cancelButton->setEnabled( false );
}

void
KDMShutdown::verifyRetry()
{
    okButton->setEnabled( true );
    cancelButton->setEnabled( true );
}

void
KDMShutdown::verifySetUser( const QString & )
{
}


KDMRadioButton::KDMRadioButton( const QString &label, QWidget *parent )
    : inherited( label, parent )
{
}

void
KDMRadioButton::mouseDoubleClickEvent( QMouseEvent * )
{
    emit doubleClicked();
}

#include "kdmshutdown.moc"
