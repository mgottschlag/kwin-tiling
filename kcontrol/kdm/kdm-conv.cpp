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

#include <kdialog.h>
#include <ksimpleconfig.h>
#include <klocale.h>

#include "kdm-conv.h"


extern KSimpleConfig *config;

KDMConvenienceWidget::KDMConvenienceWidget(QWidget *parent, const char *name, QStringList *show_users)
    : QWidget(parent, name)
{
    QString wtstr;


    alGroup = new QVGroupBox(i18n("Automatic Login"), this );

    cbalen = new QCheckBox(i18n("Enable au&to-login"), alGroup);
    QWhatsThis::add( cbalen, i18n("Turn on the auto-login feature."
	" This applies only to KDM's graphical login."
	" Think twice before enabling this!") );
    connect(cbalen, SIGNAL(toggled(bool)), SLOT(slotEnALChanged()));
    connect(cbalen, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    cbal1st = new QCheckBox(i18n("Truly automatic lo&gin"), alGroup);
    QWhatsThis::add( cbal1st, i18n("When this option is on, the auto-login"
	" will be carried out immediately when KDM starts (i.e., when your computer"
	" comes up). When this is off, you will need to initiate the auto-login"
	" by crashing the X server (by pressing alt-ctrl-backspace).") );
    connect(cbal1st, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    QWidget *hlpw = new QWidget(alGroup);
    userlb = new KComboBox(hlpw);
    u_label = new QLabel(userlb, i18n("Use&r:"), hlpw);
    connect(userlb, SIGNAL(highlighted(int)), SLOT(slotChanged()));
    wtstr = i18n("Select the user to be logged in automatically from this list.");
    QWhatsThis::add( u_label, wtstr );
    QWhatsThis::add( userlb, wtstr );
    QBoxLayout *hlpl = new QHBoxLayout(hlpw);
    hlpl->addWidget(u_label, 0);
    hlpl->addSpacing(KDialog::spacingHint());
    hlpl->addWidget(userlb, 1);


    puGroup = new QVButtonGroup(i18n("Preselect User"), this );
    connect(puGroup, SIGNAL(clicked(int)), SLOT(slotPresChanged()));
    connect(puGroup, SIGNAL(clicked(int)), SLOT(slotChanged()));
    npRadio = new QRadioButton(i18n("&None"), puGroup);
    ppRadio = new QRadioButton(i18n("Prev&ious"), puGroup);
    spRadio = new QRadioButton(i18n("Specif&y"), puGroup);
    hlpw = new QWidget(puGroup);
    puserlb = new KComboBox(hlpw);
    pu_label = new QLabel(puserlb, i18n("Us&er:"), hlpw);
    connect(puserlb, SIGNAL(highlighted(int)), SLOT(slotChanged()));
    wtstr = i18n("Select the user to be preselected for login from this list.");
    QWhatsThis::add( pu_label, wtstr );
    QWhatsThis::add( puserlb, wtstr );
    hlpl = new QHBoxLayout(hlpw);
    hlpl->addWidget(pu_label, 0);
    hlpl->addSpacing(KDialog::spacingHint());
    hlpl->addWidget(puserlb, 1);
    cbjumppw = new QCheckBox(i18n("Focus pass&word"), puGroup);
    QWhatsThis::add( cbjumppw, i18n("When this option is on, KDM will place the cursor "
	"in the password field instead of the login field after preselecting a user. "
	"This will save one key press per login, if the user name is very seldom changed.") );
    connect(cbjumppw, SIGNAL(toggled(bool)), SLOT(slotChanged()));


    npGroup = new QGroupBox(i18n("Password-less Login"), this );
    QGridLayout *rLayout = new QGridLayout(npGroup, 6, 3, 10);
    rLayout->addRowSpacing(0, 10);

    cbplen = new QCheckBox(i18n("Enable password-&less logins"), npGroup);
    QWhatsThis::add( cbplen, i18n("When this option is checked, the users from"
	" the right list will be allowed to log in without entering their"
	" password. This applies only to KDM's graphical login."
	" Think twice before enabling this!") );
    rLayout->addMultiCellWidget(cbplen, 1, 1, 0, 2);
    connect(cbplen, SIGNAL(toggled(bool)), SLOT(slotEnPLChanged()));
    connect(cbplen, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    w_label = new QLabel(i18n("Password re&quired:"), npGroup);
    rLayout->addWidget(w_label, 2, 0);
    wpuserlb = new KListBox(npGroup);
    w_label->setBuddy(wpuserlb);
    rLayout->addMultiCellWidget(wpuserlb, 3, 5, 0, 0);
    wtstr = i18n("This is the list of users which need to type their password to log in.");
    QWhatsThis::add( w_label, wtstr );
    QWhatsThis::add( wpuserlb, wtstr );

    n_label = new QLabel(i18n("S&kip password check:"), npGroup);
    rLayout->addWidget(n_label, 2, 2);
    npuserlb = new KListBox(npGroup);
    n_label->setBuddy(npuserlb);
    rLayout->addMultiCellWidget(npuserlb, 3, 5, 2, 2);
    wtstr = i18n("This is the list of users which are allowed in without typing their password.");
    QWhatsThis::add( n_label, wtstr );
    QWhatsThis::add( npuserlb, wtstr );

    QSize sz(40, 20);

    wp_to_np = new QPushButton( "&>>", npGroup );
    wp_to_np->setFixedSize( sz );
    rLayout->addWidget(wp_to_np, 3, 1);
    connect( wp_to_np, SIGNAL( clicked() ), SLOT( slotWpToNp() ) );
    QWhatsThis::add( wp_to_np, i18n("Click here to add the highlighted user"
	" on the left to the list of selected users on the right, i.e. users"
	" that are allowed in without entering their password.") );

    np_to_wp = new QPushButton( "&<<", npGroup );
    np_to_wp->setFixedSize( sz );
    rLayout->addWidget(np_to_wp, 4, 1);
    connect( np_to_wp, SIGNAL( clicked() ), SLOT( slotNpToWp() ) );
    QWhatsThis::add( np_to_wp, i18n("Click here to remove the highlighted"
	" user from the list of selected users."));

    rLayout->setRowStretch(5, 1);


    btGroup = new QVGroupBox( i18n("Miscellaneous"), this );

    cbarlen = new QCheckBox(i18n("Automatically log in again after &X server crash"), btGroup);
    QWhatsThis::add( cbarlen, i18n("When this option is on, a user will be"
	" logged in again automatically, when his session is interrupted by an"
	" X server crash. Note, that this can open a security hole: if you use"
	" another screen locker than kdesktop's integrated one, this will make"
	" circumventing a password-secured screen lock possible.") );
    connect(cbarlen, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    QGridLayout *main = new QGridLayout(this, 4, 2, 10);
    main->addWidget(alGroup, 0, 0);
    main->addWidget(puGroup, 1, 0);
    main->addMultiCellWidget(npGroup, 0,2, 1,1);
    main->addMultiCellWidget(btGroup, 3,3, 0,1);
    main->setColStretch(0, 1);
    main->setColStretch(1, 2);
    main->setRowStretch(2, 1);

    updateButton();
    load(show_users);

    // read only mode
    if (getuid() != 0)
      {
	cbalen->setEnabled(false);
	cbal1st->setEnabled(false);
	userlb->setEnabled(false);
	cbplen->setEnabled(false);
	wpuserlb->setEnabled(false);
	npuserlb->setEnabled(false);
	wp_to_np->setEnabled(false);
	np_to_wp->setEnabled(false);
	cbarlen->setEnabled(false);
	npRadio->setEnabled(false);
	ppRadio->setEnabled(false);
	spRadio->setEnabled(false);
	puserlb->setEnabled(false);
	cbjumppw->setEnabled(false);
      }
}

void KDMConvenienceWidget::updateButton()
{
    wp_to_np->setEnabled( wpuserlb->count()>0);
    np_to_wp->setEnabled( npuserlb->count()>0);
}

void KDMConvenienceWidget::removeText(QListBox *lb, const QString &user)
{
    for (uint i = 0, j = lb->count(); i < j; i++)
	if (lb->text(i) == user)
	    lb->removeItem(i);
}


void KDMConvenienceWidget::addShowUser(const QString &user)
{
    if (user != QString::fromLatin1("root")) {
	userlb->insertItem(user);
	userlb->listBox()->sort();
	wpuserlb->insertItem(user);
	wpuserlb->sort();
    }
}

void KDMConvenienceWidget::removeShowUser(const QString &user)
{
    removeText(userlb->listBox(), user);
    removeText(wpuserlb, user);
    removeText(npuserlb, user);
}


void KDMConvenienceWidget::slotWpToNp()
{
    int id = wpuserlb->currentItem();
    if (id < 0)
       return;
    QString user = wpuserlb->currentText();
    npuserlb->insertItem(user);
    npuserlb->sort();
    wpuserlb->removeItem(id);
    slotChanged();
    updateButton();
}

void KDMConvenienceWidget::slotNpToWp()
{
    int id = npuserlb->currentItem();
    if (id < 0)
       return;
    QString user = npuserlb->currentText();
    wpuserlb->insertItem(user);
    wpuserlb->sort();
    npuserlb->removeItem(id);
    slotChanged();
    updateButton();
}


void KDMConvenienceWidget::slotEnALChanged()
{
    bool en = cbalen->isChecked();
    cbal1st->setEnabled(en);
    u_label->setEnabled(en);
    userlb->setEnabled(en);
}


void KDMConvenienceWidget::slotPresChanged()
{
    bool en = spRadio->isChecked();
    pu_label->setEnabled(en);
    puserlb->setEnabled(en);
    cbjumppw->setEnabled(!npRadio->isChecked());
}


void KDMConvenienceWidget::slotEnPLChanged()
{
    bool en = cbplen->isChecked();
    w_label->setEnabled(en);
    wpuserlb->setEnabled(en);
    n_label->setEnabled(en);
    npuserlb->setEnabled(en);
    wp_to_np->setEnabled(en && wpuserlb->count()>0 );
    np_to_wp->setEnabled(en && npuserlb->count()>0 );
}


void KDMConvenienceWidget::save()
{
    config->setGroup("X-:0-Core");
    config->writeEntry( "AutoLoginEnable", cbalen->isChecked() );
    config->writeEntry( "AutoLoginUser", userlb->currentText() );
    config->writeEntry( "AutoLogin1st", cbal1st->isChecked() );

    config->setGroup("X-:*-Core");
    config->writeEntry( "NoPassEnable", cbplen->isChecked() );
    QString npusrstr;
    for(uint i = 0, j = npuserlb->count(); i < j; i++) {
        npusrstr.append(npuserlb->text(i));
        npusrstr.append(",");
    }
    config->writeEntry( "NoPassUsers", npusrstr );

    config->setGroup("X-*-Core");
    config->writeEntry( "AutoReLogin", cbarlen->isChecked() );

    config->setGroup("X-*-Greeter");
    config->writeEntry( "PreselectUser", npRadio->isChecked() ? "None" :
				    ppRadio->isChecked() ? "Previous" :
							   "Default" );
    config->writeEntry( "DefaultUser", puserlb->currentText() );
    config->writeEntry( "FocusPasswd", cbjumppw->isChecked() );
}


void KDMConvenienceWidget::load(QStringList *show_users)
{
    show_users->remove("root");
    show_users->sort();

    userlb->clear();
    userlb->insertStringList(*show_users);
    config->setGroup("X-:0-Core");
    cbalen->setChecked(config->readBoolEntry( "AutoLoginEnable", false) );
    QString user = config->readEntry( "AutoLoginUser" );
    for (uint i = 0, j = userlb->count(); i < j; i++)
	if (userlb->text(i) == user)
	    userlb->setCurrentItem(i);
    cbal1st->setChecked(config->readBoolEntry( "AutoLogin1st", true) );

    config->setGroup("X-:*-Core");
    cbplen->setChecked(config->readBoolEntry( "NoPassEnable", false) );
    QStringList npusers = config->readListEntry( "NoPassUsers");
    wpuserlb->clear();
    for (QStringList::ConstIterator it = show_users->begin(),
	    et = show_users->end(); it != et; ++it)
	if (!npusers.contains(*it))
	    wpuserlb->insertItem(*it);
    npuserlb->clear();
    for (QStringList::ConstIterator it = npusers.begin(), et = npusers.end();
	 it != et; ++it)
	if (show_users->contains(*it))
	    npuserlb->insertItem(*it);

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
    puserlb->clear();
    puserlb->insertStringList(*show_users);
    user = config->readEntry( "DefaultUser" );
    for (uint i = 0, j = puserlb->count(); i < j; i++)
	if (puserlb->text(i) == user)
	    puserlb->setCurrentItem(i);
    cbjumppw->setChecked(config->readBoolEntry( "FocusPasswd", false) );

    slotEnALChanged();
    slotPresChanged();
    slotEnPLChanged();
}


void KDMConvenienceWidget::defaults()
{
    cbalen->setChecked(false);
    cbal1st->setChecked(true);
    npRadio->setChecked(true);
    cbplen->setChecked(false);
    cbarlen->setChecked(false);
    cbjumppw->setChecked(false);

    slotEnALChanged();
    slotPresChanged();
    slotEnPLChanged();
}


void KDMConvenienceWidget::slotChanged()
{
  emit changed(true);
}

#include "kdm-conv.moc"
