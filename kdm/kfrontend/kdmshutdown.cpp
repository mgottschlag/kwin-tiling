    /*

    Shutdown dialog
    $Id$

    Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000, 2001 Oswald Buddenhagen <ossi@kde.org>


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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */
 
#include <qfile.h>
#include <qcombobox.h>
#include <qvbuttongroup.h>
#include <qstyle.h>

#include <kapplication.h>
#include <klocale.h>
#include <kseparator.h>

#include "kdmshutdown.h"
#include "kdmconfig.h"
#include "liloinfo.h"
#include "kdm_greet.h"


KDMShutdown::KDMShutdown( QWidget* _parent)
    : FDialog( _parent, "Shutdown")
{
    QComboBox *targets = 0;
    QGridLayout* box = new QGridLayout( winFrame, 3, 2, 
			KDialog::spacingHint() * 2, KDialog::spacingHint());

    howGroup = new QVButtonGroup( i18n("Shutdown type"), winFrame);
    box->addWidget( howGroup, 0, 0, AlignTop);

    QRadioButton *rb;
    rb = new QRadioButton( i18n("&Halt"), howGroup);
    set_min( rb);
    // Default action
    rb->setChecked( true);
    rb->setFocus();

    restart_rb = new QRadioButton( i18n("&Reboot"), howGroup);
    set_min( restart_rb);

#if defined(__linux__) && defined(__i386__)
    if ( kdmcfg->_useLilo ) {
	QWidget *hlp = new QWidget (howGroup);
	QHBoxLayout *hb = new QHBoxLayout (hlp);
#if QT_VERSION >= 300
	int spc = kapp->style().pixelMetric(QStyle::PM_ExclusiveIndicatorWidth)
	    + howGroup->insideSpacing();
#else
	int spc = kapp->style().exclusiveIndicatorSize().width()
	    + KDialog::spacingHint();
#endif
	hb->addSpacing (spc);
	targets = new QComboBox( hlp);
	set_fixed (targets);
	hlp->setFixedSize (targets->width() + spc, targets->height());
	hb->addWidget( targets);

	// fill combo box with contents of lilo config
	LiloInfo info(kdmcfg->_liloCmd, kdmcfg->_liloMap);

	QStringList list;
	if (info.getBootOptions(&list) == 0) {
	    targets->insertStringList(list);
            liloTarget = info.getDefaultBootOptionIndex();
	    targets->setCurrentItem(liloTarget);
	    connect(targets, SIGNAL(activated(int)),
		    SLOT(target_changed(int)));
	}
    }
#endif

    set_fixed (howGroup);


    whenGroup = new QVButtonGroup( i18n("Shutdown mode"), winFrame);
    box->addWidget( whenGroup, 0, 1, AlignTop);

    rb = new QRadioButton( i18n("verb!", "&Schedule"), whenGroup);
    set_min( rb);
    if (kdmcfg->_defSdMode == SHUT_SCHEDULE)
	rb->setChecked( true);

    force_rb = new QRadioButton( i18n("&Force Now"), whenGroup);
    set_min( force_rb);
    // Default action
    if (kdmcfg->_defSdMode == SHUT_FORCENOW)
	force_rb->setChecked( true);

    try_rb = new QRadioButton( i18n("&Try Now"), whenGroup);
    set_min( try_rb);
    if (kdmcfg->_defSdMode == SHUT_TRYNOW)
	try_rb->setChecked( true);

    if (kdmcfg->_allowNuke == SHUT_NONE)
	force_rb->setEnabled (false);

    connect(whenGroup, SIGNAL(clicked(int)), SLOT(when_changed(int)));

    set_fixed (whenGroup);


    // Passwd line edit
    pswdEdit = new KPasswordEdit( winFrame, "edit", kdmcfg->_echoMode);
    pswdEdit->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));

    QLabel *plb = new QLabel (pswdEdit, i18n("Root &Password:"), winFrame);

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), SLOT(timerDone()) );
    
    QHBoxLayout *qhb = new QHBoxLayout ();
    qhb->addWidget( plb);
    qhb->addSpacing(KDialog::spacingHint());
    qhb->addWidget( pswdEdit);
    box->addMultiCellLayout( qhb, 1,1, 0,1);


    okButton = new QPushButton( i18n("&OK"), winFrame);
    set_fixed( okButton);
    okButton->setDefault( true);
    cancelButton = new QPushButton( i18n("&Cancel"), winFrame);
    set_fixed( cancelButton);

    box->addWidget( okButton, 2, 0, AlignHCenter);
    box->addWidget( cancelButton, 2, 1, AlignHCenter);

    connect( okButton, SIGNAL(clicked()), SLOT(bye_bye()));
    connect( cancelButton, SIGNAL(clicked()), SLOT(reject()));

    when_changed (0);
}

void
KDMShutdown::timerDone()
{
    okButton->setEnabled( true);
    cancelButton->setEnabled( true);
    when_changed (0);
}

void
KDMShutdown::target_changed(int id)
{
#if defined(__linux__) && defined(__i386__)
    restart_rb->setChecked(true);
    liloTarget = id;
#endif
}

void
KDMShutdown::when_changed(int)
{
    needRoot = kdmcfg->_allowShutdown == SHUT_ROOT ||
	       (kdmcfg->_allowNuke == SHUT_ROOT && force_rb->isChecked());
    bool can = needRoot && !timer->isActive();
    pswdEdit->setEnabled (can);
    if (can)
	pswdEdit->setFocus();
}

void
KDMShutdown::bye_bye()
{
    if( needRoot) {
	GSendInt (G_Verify);
	GSendStr ("root");
	GSendStr (pswdEdit->password());
	if (GRecvInt () < V_OK ) {
	    pswdEdit->erase();
	    pswdEdit->setEnabled( false);
	    okButton->setEnabled( false);
	    cancelButton->setEnabled( false);
	    timer->start( 1500 + kapp->random()/(RAND_MAX/1000), true );
	    return;
	}
    }
#if defined(__linux__) && defined(__i386__)
    if (kdmcfg->_useLilo && restart_rb->isChecked()) {
	LiloInfo info(kdmcfg->_liloCmd, kdmcfg->_liloMap);
	info.setNextBootOption(liloTarget);
    }
#endif
    GSendInt (G_Shutdown);
    GSendInt (restart_rb->isChecked() ? SHUT_REBOOT : SHUT_HALT);
    GSendInt (force_rb->isChecked() ? SHUT_FORCENOW :
	      try_rb->isChecked() ? SHUT_TRYNOW : SHUT_SCHEDULE);
    accept ();
}

#include "kdmshutdown.moc"

/*  
 * Local variables:  
 * mode: c++  
 * c-file-style: "k&r"  
 * End:  
*/
