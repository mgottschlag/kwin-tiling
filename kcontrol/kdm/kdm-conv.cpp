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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <unistd.h>
#include <sys/types.h>


#include <qlayout.h>
#include <qlabel.h>
#include <q3groupbox.h>
#include <q3buttongroup.h>

#include <q3header.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QHBoxLayout>
#include <QBoxLayout>

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

    alGroup = new Q3GroupBox( i18n("Enable Au&to-Login"), this );
    alGroup->setOrientation( Qt::Vertical );
    alGroup->setCheckable( true );
    alGroup->setSizePolicy( vpref );
    QVBoxLayout *laygroup2 = new QVBoxLayout(alGroup->layout(), KDialog::spacingHint() );

    alGroup->setWhatsThis( i18n("Turn on the auto-login feature."
	" This applies only to KDM's graphical login."
	" Think twice before enabling this!") );
    connect(alGroup, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    QWidget *hlpw1 = new QWidget( alGroup );
    userlb = new KComboBox( hlpw1 );
    u_label = new QLabel( userlb, i18n("Use&r:"), hlpw1 );
    QHBoxLayout *hlpl1 = new QHBoxLayout(alGroup->layout(), KDialog::spacingHint());
    hlpl1->addWidget(u_label);
    hlpl1->addWidget(userlb);
    hlpl1->addStretch( 1 );
    laygroup2->addWidget( hlpw1 );
    connect(userlb, SIGNAL(highlighted(int)), SLOT(slotChanged()));
    wtstr = i18n("Select the user to be logged in automatically.");
    u_label->setWhatsThis( wtstr );
    userlb->setWhatsThis( wtstr );
    autoLockCheck = new QCheckBox( i18n("Loc&k session"), alGroup );
    laygroup2->addWidget(autoLockCheck);
    connect( autoLockCheck, SIGNAL(toggled(bool)), SLOT(slotChanged()) );
    autoLockCheck->setWhatsThis( i18n("The automatically started session "
	"will be locked immediately (provided it is a KDE session). This can "
	"be used to obtain a super-fast login restricted to one user.") );







    puGroup = new Q3ButtonGroup(i18n("Preselect User"), this );
    puGroup->setOrientation( Qt::Vertical );
    QVBoxLayout *laygroup5 = new QVBoxLayout(puGroup->layout(), KDialog::spacingHint() );

    puGroup->setSizePolicy( vpref );

    connect(puGroup, SIGNAL(clicked(int)), SLOT(slotPresChanged()));
    connect(puGroup, SIGNAL(clicked(int)), SLOT(slotChanged()));
    npRadio = new QRadioButton(i18nc("preselected user", "&None"), puGroup);
    laygroup5->addWidget( npRadio );
    ppRadio = new QRadioButton(i18n("Prev&ious"), puGroup);
    ppRadio->setWhatsThis( i18n("Preselect the user that logged in previously. "
	"Use this if this computer is usually used several consecutive times by one user.") );
    laygroup5->addWidget( ppRadio );
    spRadio = new QRadioButton(i18n("Specif&y"), puGroup);
    spRadio->setWhatsThis( i18n("Preselect the user specified in the combo box below. "
	"Use this if this computer is predominantly used by a certain user.") );
    laygroup5->addWidget( spRadio );

    QWidget *hlpw = new QWidget(puGroup);
    laygroup5->addWidget( hlpw );
    puserlb = new KComboBox(true, hlpw);

    pu_label = new QLabel(puserlb, i18n("Us&er:"), hlpw);
    connect(puserlb, SIGNAL(textChanged(const QString &)), SLOT(slotChanged()));
    wtstr = i18n("Select the user to be preselected for login. "
	"This box is editable, so you can specify an arbitrary non-existent "
	"user to mislead possible attackers.");
    pu_label->setWhatsThis( wtstr );
    puserlb->setWhatsThis( wtstr );
    QBoxLayout *hlpl = new QHBoxLayout(hlpw, 0, KDialog::spacingHint());
    hlpl->addWidget(pu_label);
    hlpl->addWidget(puserlb);
    hlpl->addStretch( 1 );
    cbjumppw = new QCheckBox(i18n("Focus pass&word"), puGroup);
    laygroup5->addWidget( cbjumppw );
    cbjumppw->setWhatsThis( i18n("When this option is on, KDM will place the cursor "
	"in the password field instead of the user field after preselecting a user. "
	"Use this to save one key press per login, if the preselection usually does not need to "
	"be changed.") );
    connect(cbjumppw, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    npGroup = new Q3GroupBox(i18n("Enable Password-&Less Logins"), this );
    npGroup->setOrientation( Qt::Vertical );
    QVBoxLayout *laygroup3 = new QVBoxLayout(npGroup->layout(), KDialog::spacingHint() );

    npGroup->setCheckable( true );

    npGroup->setWhatsThis( i18n("When this option is checked, the checked users from"
	" the list below will be allowed to log in without entering their"
	" password. This applies only to KDM's graphical login."
	" Think twice before enabling this!") );

    connect(npGroup, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    pl_label = new QLabel(i18n("No password re&quired for:"), npGroup);
    laygroup3->addWidget( pl_label );
    npuserlv = new K3ListView(npGroup);
    laygroup3->addWidget( npuserlv );
    pl_label->setBuddy(npuserlv);
    npuserlv->addColumn(QString());
    npuserlv->header()->hide();
    npuserlv->setResizeMode(Q3ListView::LastColumn);
    npuserlv->setWhatsThis( i18n("Check all users you want to allow a password-less login for."
	" Entries denoted with '@' are user groups. Checking a group is like checking all users in that group."));
    connect( npuserlv, SIGNAL(clicked( Q3ListViewItem * )),
	     SLOT(slotChanged()) );

    btGroup = new Q3GroupBox( i18n("Miscellaneous"), this );
    btGroup->setOrientation( Qt::Vertical );
    QVBoxLayout *laygroup4 = new QVBoxLayout(btGroup->layout(), KDialog::spacingHint() );

    cbarlen = new QCheckBox(i18n("Automatically log in again after &X server crash"), btGroup);
    cbarlen->setWhatsThis( i18n("When this option is on, a user will be"
	" logged in again automatically when their session is interrupted by an"
	" X server crash; note that this can open a security hole: if you use"
	" a screen locker than KDE's integrated one, this will make"
	" circumventing a password-secured screen lock possible.") );
    laygroup4->addWidget( cbarlen );
    connect(cbarlen, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    QGridLayout *main = new QGridLayout(this, 5, 2, 10);
    main->addWidget(paranoia, 0, 0);
    main->addWidget(alGroup, 1, 0);
    main->addWidget(puGroup, 2, 0);
    main->addWidget(npGroup, 0, 1, 4, 1);
    main->addWidget(btGroup, 4, 0, 1, 2 );
    main->setColumnStretch(0, 1);
    main->setColumnStretch(1, 2);
    main->setRowStretch(3, 1);

    connect( userlb, SIGNAL(activated( const QString & )),
	     SLOT(slotSetAutoUser( const QString & )) );
    connect( puserlb, SIGNAL(textChanged( const QString & )),
	     SLOT(slotSetPreselUser( const QString & )) );
    connect( npuserlv, SIGNAL(clicked( Q3ListViewItem * )),
	     SLOT(slotUpdateNoPassUser( Q3ListViewItem * )) );

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
    config->writeEntry( "AutoLoginLocked", autoLockCheck->isChecked() );

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
    bool alenable = config->readEntry( "AutoLoginEnable", false );
    autoUser = config->readEntry( "AutoLoginUser" );
    autoLockCheck->setChecked( config->readEntry( "AutoLoginLocked", false ) );
    if (autoUser.isEmpty())
	alenable=false;
    alGroup->setChecked( alenable );

    config->setGroup("X-:*-Core");
    npGroup->setChecked(config->readEntry( "NoPassEnable", false ) );
    noPassUsers = config->readEntry( "NoPassUsers", QStringList() );

    config->setGroup("X-*-Core");
    cbarlen->setChecked(config->readEntry( "AutoReLogin", false ) );

    config->setGroup("X-:*-Greeter");
    QString presstr = config->readEntry( "PreselectUser", "None" );
    if (presstr == "Previous")
	ppRadio->setChecked(true);
    else if (presstr == "Default")
	spRadio->setChecked(true);
    else
	npRadio->setChecked(true);
    preselUser = config->readEntry( "DefaultUser" );
    cbjumppw->setChecked(config->readEntry( "FocusPasswd", false ) );

    slotPresChanged();
}


void KDMConvenienceWidget::defaults()
{
    alGroup->setChecked(false);
    autoLockCheck->setChecked(false);
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

void KDMConvenienceWidget::slotUpdateNoPassUser( Q3ListViewItem *item )
{
    if ( !item )
        return;
    Q3CheckListItem *itm = (Q3CheckListItem *)item;
    int ind = noPassUsers.indexOf( itm->text() );
    if (itm->isOn()) {
	if (ind < 0)
	    noPassUsers.append( itm->text() );
    } else {
	if (ind >= 0)
	    noPassUsers.removeAt( ind );
    }
}

void KDMConvenienceWidget::slotClearUsers()
{
    userlb->clear();
    puserlb->clear();
    npuserlv->clear();
    if (!autoUser.isEmpty())
	userlb->addItem(autoUser);
    if (!preselUser.isEmpty())
	puserlb->addItem(preselUser);
}

void KDMConvenienceWidget::slotAddUsers(const QMap<QString,int> &users)
{
    QMap<QString,int>::const_iterator it;
    for (it = users.begin(); it != users.end(); ++it) {
        if (it.value() > 0) {
            if (it.key() != autoUser)
                userlb->addItem(it.key());
            if (it.key() != preselUser)
                puserlb->addItem(it.key());
        }
        if (it.value() != 0)
            (new Q3CheckListItem(npuserlv, it.key(), Q3CheckListItem::CheckBox))->
    	        setOn(noPassUsers.contains(it.key()));
    }

    if (userlb->model())
        userlb->model()->sort(0);

    if (puserlb->model())
        puserlb->model()->sort(0);

    npuserlv->sort();
    userlb->setCurrentItem(autoUser);
    puserlb->setCurrentItem(preselUser);
}

void KDMConvenienceWidget::slotDelUsers(const QMap<QString,int> &users)
{
    QMap<QString,int>::const_iterator it;
    for (it = users.begin(); it != users.end(); ++it) {
	if (it.value() > 0) {
	    int idx = userlb->findText( it.key() );
	    if (it.key() != autoUser && idx != -1)
	        userlb->removeItem( idx );
	    idx     = puserlb->findText( it.key() );
	    if (it.key() != preselUser && idx != -1)
	        puserlb->removeItem( idx );
	}
	if (it.value() != 0)
	    delete npuserlv->findItem( it.key(), 0 );
    }
}

#include "kdm-conv.moc"
