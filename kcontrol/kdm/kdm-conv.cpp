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

    QSizePolicy vpref( QSizePolicy::Minimum, QSizePolicy::Fixed );

    alGroup = new QVGroupBox(i18n("Automatic Login"), this );
    alGroup->setSizePolicy( vpref );

    cbalen = new QCheckBox(i18n("Enable au&to-login"), alGroup);
    QWhatsThis::add( cbalen, i18n("Turn on the auto-login feature."
	" This applies only to KDM's graphical login."
	" Think twice before enabling this!") );
    connect(cbalen, SIGNAL(toggled(bool)), SLOT(slotALChanged()));
    connect(cbalen, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    QWidget *hlpw = new QWidget(alGroup);
    userlb = new KComboBox(hlpw);
    u_label = new QLabel(userlb, i18n("Use&r:"), hlpw);
    connect(userlb, SIGNAL(highlighted(int)), SLOT(slotChanged()));
    wtstr = i18n("Select the user to be logged in automatically.");
    QWhatsThis::add( u_label, wtstr );
    QWhatsThis::add( userlb, wtstr );
    QBoxLayout *hlpl = new QHBoxLayout(hlpw);
    hlpl->addWidget(u_label, 0);
    hlpl->addSpacing(KDialog::spacingHint());
    hlpl->addWidget(userlb, 1);


    puGroup = new QVButtonGroup(i18n("Preselect User"), this );
    puGroup->setSizePolicy( vpref );

    connect(puGroup, SIGNAL(clicked(int)), SLOT(slotPresChanged()));
    connect(puGroup, SIGNAL(clicked(int)), SLOT(slotChanged()));
    npRadio = new QRadioButton(i18n("&None"), puGroup);
    ppRadio = new QRadioButton(i18n("Prev&ious"), puGroup);
    QWhatsThis::add( ppRadio, i18n("Preselect the user that logged in previously. "
	"Use this if this computer is usually used several consecutive times by one user.") );
    spRadio = new QRadioButton(i18n("Specif&y"), puGroup);
    QWhatsThis::add( spRadio, i18n("Preselect the user specified in the combo box below. "
	"Use this if this computer is predominantly used by a certain user.") );
    hlpw = new QWidget(puGroup);
    puserlb = new KComboBox(true, hlpw);
    pu_label = new QLabel(puserlb, i18n("Us&er:"), hlpw);
    connect(puserlb, SIGNAL(textChanged(const QString &)), SLOT(slotChanged()));
    wtstr = i18n("Select the user to be preselected for login. "
	"This box is editable, so you can specify an arbitrary non-existent "
	"user to mislead possible attackers.");
    QWhatsThis::add( pu_label, wtstr );
    QWhatsThis::add( puserlb, wtstr );
    hlpl = new QHBoxLayout(hlpw);
    hlpl->addWidget(pu_label, 0);
    hlpl->addSpacing(KDialog::spacingHint());
    hlpl->addWidget(puserlb, 1);
    cbjumppw = new QCheckBox(i18n("Focus pass&word"), puGroup);
    QWhatsThis::add( cbjumppw, i18n("When this option is on, KDM will place the cursor "
	"in the password field instead of the user field after preselecting a user. "
	"Use this to save one key press per login, if the preselection usually does not need to "
	"be changed.") );
    connect(cbjumppw, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    npGroup = new QVGroupBox(i18n("Password-less Login"), this );

    cbplen = new QCheckBox(i18n("Enable password-&less logins"), npGroup);
    QWhatsThis::add( cbplen, i18n("When this option is checked, the checked users from"
	" the list below will be allowed to log in without entering their"
	" password. This applies only to KDM's graphical login."
	" Think twice before enabling this!") );

    connect(cbplen, SIGNAL(toggled(bool)), SLOT(slotNPChanged()));
    connect(cbplen, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    pl_label = new QLabel(i18n("No password re&quired for:"), npGroup);
    npuserlv = new KListView(npGroup);
    pl_label->setBuddy(npuserlv);
    npuserlv->addColumn(QString::null);
    npuserlv->header()->hide();
    npuserlv->setResizeMode(QListView::LastColumn);
    QWhatsThis::add(npuserlv, i18n("Check all users you want to allow a password-less login for."));
	
    btGroup = new QVGroupBox( i18n("Miscellaneous"), this );

    cbarlen = new QCheckBox(i18n("Automatically log in again after &X server crash"), btGroup);
    QWhatsThis::add( cbarlen, i18n("When this option is on, a user will be"
	" logged in again automatically, when his session is interrupted by an"
	" X server crash. Note, that this can open a security hole: if you use"
	" another screen locker than KDE's integrated one, this will make"
	" circumventing a password-secured screen lock possible.") );
    connect(cbarlen, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    QGridLayout *main = new QGridLayout(this, 5, 2, 10);
    main->addWidget(alGroup, 0, 0);
    main->addWidget(puGroup, 1, 0);
    main->addMultiCellWidget(npGroup, 0,2, 1,1);
    main->addMultiCellWidget(btGroup, 3,3, 0,1);
    main->setColStretch(0, 1);
    main->setColStretch(1, 2);
    main->setRowStretch(2, 1);

    connect( userlb, SIGNAL(activated( const QString & )),
	     SLOT(slotSetAutoUser( const QString & )) );
    connect( puserlb, SIGNAL(textChanged( const QString & )),
	     SLOT(slotSetPreselUser( const QString & )) );
    connect( npuserlv, SIGNAL(clicked( QListViewItem * )),
	     SLOT(slotUpdateNoPassUser( QListViewItem * )) );

}

void KDMConvenienceWidget::makeReadOnly()
{
    cbalen->setEnabled(false);
    userlb->setEnabled(false);
    cbplen->setEnabled(false);
    npuserlv->setEnabled(false);
    pl_label->setEnabled(false);
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

void KDMConvenienceWidget::slotALChanged()
{
    userlb->setEnabled(cbalen->isChecked());
}

void KDMConvenienceWidget::slotNPChanged()
{
    bool en = cbplen->isChecked();
    npuserlv->setEnabled(en);
    pl_label->setEnabled(en);
}

void KDMConvenienceWidget::updateEnables()
{
    slotALChanged();
    slotPresChanged();
    slotNPChanged();
}

void KDMConvenienceWidget::save()
{
    config->setGroup("X-:0-Core");
    config->writeEntry( "AutoLoginEnable", cbalen->isChecked() );
    config->writeEntry( "AutoLoginUser", userlb->currentText() );

    config->setGroup("X-:*-Core");
    config->writeEntry( "NoPassEnable", cbplen->isChecked() );
    config->writeEntry( "NoPassUsers", noPassUsers );

    config->setGroup("X-*-Core");
    config->writeEntry( "AutoReLogin", cbarlen->isChecked() );

    config->setGroup("X-*-Greeter");
    config->writeEntry( "PreselectUser", npRadio->isChecked() ? "None" :
				    ppRadio->isChecked() ? "Previous" :
							   "Default" );
    config->writeEntry( "DefaultUser", puserlb->currentText() );
    config->writeEntry( "FocusPasswd", cbjumppw->isChecked() );
}


void KDMConvenienceWidget::load()
{
    config->setGroup("X-:0-Core");
    cbalen->setChecked(config->readBoolEntry( "AutoLoginEnable", false) );
    autoUser = config->readEntry( "AutoLoginUser" );

    config->setGroup("X-:*-Core");
    cbplen->setChecked(config->readBoolEntry( "NoPassEnable", false) );
    noPassUsers = config->readListEntry( "NoPassUsers");

    config->setGroup("X-*-Core");
    cbarlen->setChecked(config->readBoolEntry( "AutoReLogin", false) );

    config->setGroup("X-*-Greeter");
    QString presstr = config->readEntry( "PreselectUser", "None" );
    if (presstr == "Previous")
	ppRadio->setChecked(true);
    else if (presstr == "Default")
	spRadio->setChecked(true);
    else
	npRadio->setChecked(true);
    preselUser = config->readEntry( "DefaultUser" );
    cbjumppw->setChecked(config->readBoolEntry( "FocusPasswd", false) );

    updateEnables();
}


void KDMConvenienceWidget::defaults()
{
    cbalen->setChecked(false);
    npRadio->setChecked(true);
    cbplen->setChecked(false);
    cbarlen->setChecked(false);
    cbjumppw->setChecked(false);
    autoUser = "";
    preselUser = "";
    noPassUsers.clear();

    updateEnables();
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
	if (it.data() != 0) {
	    const QString *name = &it.key();
	    if (*name != autoUser)
		userlb->insertItem(*name);
	    if (*name != preselUser)
		puserlb->insertItem(*name);
	    (new QCheckListItem(npuserlv, *name, QCheckListItem::CheckBox))->
	      setOn(noPassUsers.find(*name) != noPassUsers.end());
	}
    }
    userlb->listBox()->sort();
    puserlb->listBox()->sort();
    npuserlv->sort();
    userlb->setCurrentItem(autoUser);
    puserlb->setCurrentItem(preselUser);
}

void KDMConvenienceWidget::slotDelUsers(const QMap<QString,int> &users)
{
    QMapConstIterator<QString,int> it;
    for (it = users.begin(); it != users.end(); ++it) {
	if (it.data() != 0) {
	    const QString *name = &it.key();
	    if (*name != autoUser)
	        delete userlb->listBox()->
		  findItem( *name, ExactMatch | CaseSensitive );
	    if (*name != preselUser)
	        delete puserlb->listBox()->
		  findItem( *name, ExactMatch | CaseSensitive );
	    delete npuserlv->findItem( *name, 0 );
	}
    }
}

#include "kdm-conv.moc"
