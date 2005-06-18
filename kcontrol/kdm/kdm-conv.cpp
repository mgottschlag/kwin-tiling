/* This file is part of the KDE Display Manager Configuration package

    Copyright (C) 2000 Oswald Buddenhagen <ossi@kde.org>
    Based on several other files.

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <unistd.h>
#include <sys/types.h>


#include <qlayout.h>
#include <qlabel.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qvbuttongroup.h>
#include <qwhatsthis.h>
#include <qheader.h>

#include <kdialog.h>
#include <ksimpleconfig.h>
#include <klocale.h>

#include "kdm-conv.h"

extern KSimpleConfig *config;

KDMConvenienceWidget::KDMConvenienceWidget(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QString wtstr;

    QLabel *paranoia = new QLabel( i18n("<qt><center><font color=red><big><b>Attention!<br>Read help!</b></big></font></center></qt>"), this );

    QSizePolicy vpref( QSizePolicy::Minimum, QSizePolicy::Fixed );

    alGroup = new QHGroupBox( i18n("Enable Au&to-Login"), this );
    alGroup->setInsideSpacing( KDialog::spacingHint() );
    alGroup->setCheckable( true );
    alGroup->setSizePolicy( vpref );

    QWhatsThis::add( alGroup, i18n("Turn on the auto-login feature."
	" This applies only to KDM's graphical login."
	" Think twice before enabling this!") );
    connect(alGroup, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    u_label = new QLabel(i18n("Use&r:"), alGroup);
    userlb = new KComboBox(alGroup);
    u_label->setBuddy( userlb );
    QWidget *filler = new QWidget( alGroup );
    filler->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed, 1, 0 ) );
    connect(userlb, SIGNAL(highlighted(int)), SLOT(slotChanged()));
    wtstr = i18n("Select the user to be logged in automatically.");
    QWhatsThis::add( u_label, wtstr );
    QWhatsThis::add( userlb, wtstr );


    puGroup = new QVButtonGroup(i18n("Preselect User"), this );
    puGroup->setSizePolicy( vpref );

    connect(puGroup, SIGNAL(clicked(int)), SLOT(slotPresChanged()));
    connect(puGroup, SIGNAL(clicked(int)), SLOT(slotChanged()));
    npRadio = new QRadioButton(i18n("preselected user", "&None"), puGroup);
    ppRadio = new QRadioButton(i18n("Prev&ious"), puGroup);
    QWhatsThis::add( ppRadio, i18n("Preselect the user that logged in previously. "
	"Use this if this computer is usually used several consecutive times by one user.") );
    spRadio = new QRadioButton(i18n("Specif&y"), puGroup);
    QWhatsThis::add( spRadio, i18n("Preselect the user specified in the combo box below. "
	"Use this if this computer is predominantly used by a certain user.") );
    QWidget *hlpw = new QWidget(puGroup);
    puserlb = new KComboBox(true, hlpw);
    pu_label = new QLabel(puserlb, i18n("Us&er:"), hlpw);
    connect(puserlb, SIGNAL(textChanged(const QString &)), SLOT(slotChanged()));
    wtstr = i18n("Select the user to be preselected for login. "
	"This box is editable, so you can specify an arbitrary non-existent "
	"user to mislead possible attackers.");
    QWhatsThis::add( pu_label, wtstr );
    QWhatsThis::add( puserlb, wtstr );
    QBoxLayout *hlpl = new QHBoxLayout(hlpw, 0, KDialog::spacingHint());
    hlpl->addWidget(pu_label);
    hlpl->addWidget(puserlb);
    hlpl->addStretch( 1 );
    cbjumppw = new QCheckBox(i18n("Focus pass&word"), puGroup);
    QWhatsThis::add( cbjumppw, i18n("When this option is on, KDM will place the cursor "
	"in the password field instead of the user field after preselecting a user. "
	"Use this to save one key press per login, if the preselection usually does not need to "
	"be changed.") );
    connect(cbjumppw, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    npGroup = new QVGroupBox(i18n("Enable Password-&Less Logins"), this );
    npGroup->setCheckable( true );

    QWhatsThis::add( npGroup, i18n("When this option is checked, the checked users from"
	" the list below will be allowed to log in without entering their"
	" password. This applies only to KDM's graphical login."
	" Think twice before enabling this!") );

    connect(npGroup, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    pl_label = new QLabel(i18n("No password re&quired for:"), npGroup);
    npuserlv = new KListView(npGroup);
    pl_label->setBuddy(npuserlv);
    npuserlv->addColumn(QString::null);
    npuserlv->header()->hide();
    npuserlv->setResizeMode(QListView::LastColumn);
    QWhatsThis::add(npuserlv, i18n("Check all users you want to allow a password-less login for."
	" Entries denoted with '@' are user groups. Checking a group is like checking all users in that group."));
    connect( npuserlv, SIGNAL(clicked( QListViewItem * )),
	     SLOT(slotChanged()) );

    btGroup = new QVGroupBox( i18n("Miscellaneous"), this );

    cbarlen = new QCheckBox(i18n("Automatically log in again after &X server crash"), btGroup);
    QWhatsThis::add( cbarlen, i18n("When this option is on, a user will be"
	" logged in again automatically when their session is interrupted by an"
	" X server crash; note that this can open a security hole: if you use"
	" a screen locker than KDE's integrated one, this will make"
	" circumventing a password-secured screen lock possible.") );
    connect(cbarlen, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    QGridLayout *main = new QGridLayout(this, 5, 2, 10);
    main->addWidget(paranoia, 0, 0);
    main->addWidget(alGroup, 1, 0);
    main->addWidget(puGroup, 2, 0);
    main->addMultiCellWidget(npGroup, 0,3, 1,1);
    main->addMultiCellWidget(btGroup, 4,4, 0,1);
    main->setColStretch(0, 1);
    main->setColStretch(1, 2);
    main->setRowStretch(3, 1);

    connect( userlb, SIGNAL(activated( const QString & )),
	     SLOT(slotSetAutoUser( const QString & )) );
    connect( puserlb, SIGNAL(textChanged( const QString & )),
	     SLOT(slotSetPreselUser( const QString & )) );
    connect( npuserlv, SIGNAL(clicked( QListViewItem * )),
	     SLOT(slotUpdateNoPassUser( QListViewItem * )) );

}

void KDMConvenienceWidget::makeReadOnly()
{
    ((QWidget*)alGroup->child("qt_groupbox_checkbox"))->setEnabled(false);
    userlb->setEnabled(false);
    ((QWidget*)npGroup->child("qt_groupbox_checkbox"))->setEnabled(false);
    npuserlv->setEnabled(false);
    cbarlen->setEnabled(false);
    npRadio->setEnabled(false);
    ppRadio->setEnabled(false);
    spRadio->setEnabled(false);
    puserlb->setEnabled(false);
    cbjumppw->setEnabled(false);
}

void KDMConvenienceWidget::slotPresChanged()
{
    bool en = spRadio->isChecked();
    pu_label->setEnabled(en);
    puserlb->setEnabled(en);
    cbjumppw->setEnabled(!npRadio->isChecked());
}

void KDMConvenienceWidget::save()
{
    config->setGroup("X-:0-Core");
    config->writeEntry( "AutoLoginEnable", alGroup->isChecked() );
    config->writeEntry( "AutoLoginUser", userlb->currentText() );

    config->setGroup("X-:*-Core");
    config->writeEntry( "NoPassEnable", npGroup->isChecked() );
    config->writeEntry( "NoPassUsers", noPassUsers );

    config->setGroup("X-*-Core");
    config->writeEntry( "AutoReLogin", cbarlen->isChecked() );

    config->setGroup("X-:*-Greeter");
    config->writeEntry( "PreselectUser", npRadio->isChecked() ? "None" :
				    ppRadio->isChecked() ? "Previous" :
							   "Default" );
    config->writeEntry( "DefaultUser", puserlb->currentText() );
    config->writeEntry( "FocusPasswd", cbjumppw->isChecked() );
}


void KDMConvenienceWidget::load()
{
    config->setGroup("X-:0-Core");
    bool alenable = config->readBoolEntry( "AutoLoginEnable", false);
    autoUser = config->readEntry( "AutoLoginUser" );
    if (autoUser.isEmpty())
	alenable=false;
    alGroup->setChecked( alenable );

    config->setGroup("X-:*-Core");
    npGroup->setChecked(config->readBoolEntry( "NoPassEnable", false) );
    noPassUsers = config->readListEntry( "NoPassUsers");

    config->setGroup("X-*-Core");
    cbarlen->setChecked(config->readBoolEntry( "AutoReLogin", false) );

    config->setGroup("X-:*-Greeter");
    QString presstr = config->readEntry( "PreselectUser", "None" );
    if (presstr == "Previous")
	ppRadio->setChecked(true);
    else if (presstr == "Default")
	spRadio->setChecked(true);
    else
	npRadio->setChecked(true);
    preselUser = config->readEntry( "DefaultUser" );
    cbjumppw->setChecked(config->readBoolEntry( "FocusPasswd", false) );

    slotPresChanged();
}


void KDMConvenienceWidget::defaults()
{
    alGroup->setChecked(false);
    npRadio->setChecked(true);
    npGroup->setChecked(false);
    cbarlen->setChecked(false);
    cbjumppw->setChecked(false);
    autoUser = "";
    preselUser = "";
    noPassUsers.clear();

    slotPresChanged();
}


void KDMConvenienceWidget::slotChanged()
{
  emit changed(true);
}

void KDMConvenienceWidget::slotSetAutoUser( const QString &user )
{
    autoUser = user;
}

void KDMConvenienceWidget::slotSetPreselUser( const QString &user )
{
    preselUser = user;
}

void KDMConvenienceWidget::slotUpdateNoPassUser( QListViewItem *item )
{
    if ( !item )
        return;
    QCheckListItem *itm = (QCheckListItem *)item;
    QStringList::iterator it = noPassUsers.find( itm->text() );
    if (itm->isOn()) {
	if (it == noPassUsers.end())
	    noPassUsers.append( itm->text() );
    } else {
	if (it != noPassUsers.end())
	    noPassUsers.remove( it );
    }
}

void KDMConvenienceWidget::slotClearUsers()
{
    userlb->clear();
    puserlb->clear();
    npuserlv->clear();
    if (!autoUser.isEmpty())
	userlb->insertItem(autoUser);
    if (!preselUser.isEmpty())
	puserlb->insertItem(preselUser);
}

void KDMConvenienceWidget::slotAddUsers(const QMap<QString,int> &users)
{
    QMapConstIterator<QString,int> it;
    for (it = users.begin(); it != users.end(); ++it) {
        if (it.data() > 0) {
            if (it.key() != autoUser)
                userlb->insertItem(it.key());
            if (it.key() != preselUser)
                puserlb->insertItem(it.key());
        }
        if (it.data() != 0)
            (new QCheckListItem(npuserlv, it.key(), QCheckListItem::CheckBox))->
    	        setOn(noPassUsers.find(it.key()) != noPassUsers.end());
    }

    if (userlb->listBox())
        userlb->listBox()->sort();

    if (puserlb->listBox())
        puserlb->listBox()->sort();

    npuserlv->sort();
    userlb->setCurrentItem(autoUser);
    puserlb->setCurrentItem(preselUser);
}

void KDMConvenienceWidget::slotDelUsers(const QMap<QString,int> &users)
{
    QMapConstIterator<QString,int> it;
    for (it = users.begin(); it != users.end(); ++it) {
	if (it.data() > 0) {
	    if (it.key() != autoUser && userlb->listBox())
	        delete userlb->listBox()->
		  findItem( it.key(), ExactMatch | CaseSensitive );
	    if (it.key() != preselUser && puserlb->listBox())
	        delete puserlb->listBox()->
		  findItem( it.key(), ExactMatch | CaseSensitive );
	}
	if (it.data() != 0)
	    delete npuserlv->findItem( it.key(), 0 );
    }
}

#include "kdm-conv.moc"
