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
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kapp.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include "kdm-conv.moc"


extern KSimpleConfig *c;

KDMConvenienceWidget::KDMConvenienceWidget(QWidget *parent, const char *name, QStringList *show_users)
    : KCModule(parent, name)
{
    QString wtstr;
    QVBoxLayout *main = new QVBoxLayout(this, 10);


    QHBoxLayout *ulay = new QHBoxLayout(main, 10);

    alGroup = new QGroupBox(i18n("Automatic login"), this );
    QVBoxLayout *alGLayout = new QVBoxLayout( alGroup, 10, 10 );
    alGLayout->addSpacing(10);

    cbalen = new QCheckBox(i18n("&Enable auto-login"), alGroup);
    QWhatsThis::add( cbalen, i18n("Turn on the auto-login feature."
	" This applies only to KDM's graphical login."
	" Think twice before enabling this!") );
    alGLayout->addWidget( cbalen );
    connect(cbalen, SIGNAL(toggled(bool)), this, SLOT(slotEnALChanged()));
    connect(cbalen, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));

    cbal1st = new QCheckBox(i18n("&Truly automatic login"), alGroup);
    QWhatsThis::add( cbal1st, i18n("When this option is on, the auto-login"
	" will be carried out immediately when KDM starts (i.e., when your computer"
	" comes up). When this is off, you will need to initiate the auto-login"
	" by crashing the X server (by pressing alt-ctrl-backspace).") );
    alGLayout->addWidget( cbal1st );
    connect(cbal1st, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));

    wtstr = i18n("Select the user to be logged in automatically from this list.");
    u_label = new QLabel(i18n("Use&r to log in"), alGroup);
    QWhatsThis::add( u_label, wtstr );
    alGLayout->addWidget(u_label);
    userlb = new KListBox(alGroup);
    u_label->setBuddy(userlb);
    QWhatsThis::add( userlb, wtstr );
    alGLayout->addWidget(userlb);
    connect(userlb, SIGNAL(highlighted(QListBoxItem *)), this, SLOT(slotChanged()));

    ulay->addWidget(alGroup);
    ulay->setStretchFactor(alGroup, 1);


    npGroup = new QGroupBox(i18n("Password-less login"), this );
    QGridLayout *rLayout = new QGridLayout(npGroup, 6, 3, 10);
    rLayout->addRowSpacing(0, 10);

    cbplen = new QCheckBox(i18n("Enable password-&less logins"), npGroup);
    QWhatsThis::add( cbplen, i18n("When this option is checked, the users from"
	" the right list will be allowed to log in without entering their"
	" password. This applies only to KDM's graphical login."
	" Think twice before enabling this!") );
    rLayout->addMultiCellWidget(cbplen, 1, 1, 0, 2);
    connect(cbplen, SIGNAL(toggled(bool)), this, SLOT(slotEnPLChanged()));
    connect(cbplen, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));

    wtstr = i18n("This is the list of users which need to type their password to log in.");
    w_label = new QLabel(i18n("Password requ&ired"), npGroup);
    QWhatsThis::add( w_label, wtstr );
    rLayout->addWidget(w_label, 2, 0);
    wpuserlb = new KListBox(npGroup);
    w_label->setBuddy(wpuserlb);
    QWhatsThis::add( wpuserlb, wtstr );
    rLayout->addMultiCellWidget(wpuserlb, 3, 5, 0, 0);

    wtstr = i18n("This is the list of users which are allowed in without typing their password.");
    n_label = new QLabel(i18n("&No password required"), npGroup);
    QWhatsThis::add( n_label, wtstr );
    rLayout->addWidget(n_label, 2, 2);
    npuserlb = new KListBox(npGroup);
    n_label->setBuddy(npuserlb);
    QWhatsThis::add( npuserlb, wtstr );
    rLayout->addMultiCellWidget(npuserlb, 3, 5, 2, 2);

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

    ulay->addWidget(npGroup);
    ulay->setStretchFactor(npGroup, 2);


    btGroup = new QButtonGroup( i18n("Miscellaneous"), this );
    QVBoxLayout *llay = new QVBoxLayout(btGroup, 10);
    llay->addSpacing(10);

    cbarlen = new QCheckBox(i18n("Automatically log in a&gain after X server crash"), btGroup);
    QWhatsThis::add( cbarlen, i18n("When this option is on, a user will be"
	" logged in again automatically, when his session is interrupted by an"
	" X server crash.") );
    llay->addWidget(cbarlen);
    connect(cbarlen, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));

    cbshwen = new QCheckBox(i18n("Sho&w previous user"), btGroup);
    QWhatsThis::add( cbshwen, i18n("When this option is on, KDM will display"
	" and preselect the user that logged in previously.") );
    llay->addWidget(cbshwen);
    connect(cbshwen, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));

    main->addWidget(btGroup);
//    main->addStretch(1);


    load(show_users);

    // read only mode
    if (getuid() != 0)
      {
	alGroup->setEnabled(false);
	npGroup->setEnabled(false);
	btGroup->setEnabled(false);
      }
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
	userlb->sort();
	wpuserlb->insertItem(user);
	wpuserlb->sort();
    }
}

void KDMConvenienceWidget::removeShowUser(const QString &user)
{
    removeText(userlb, user);
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
}

void KDMConvenienceWidget::slotEnALChanged()
{
    bool en = cbalen->isChecked();
    cbal1st->setEnabled(en);
    u_label->setEnabled(en);
    userlb->setEnabled(en);
}

void KDMConvenienceWidget::slotEnPLChanged()
{
    bool en = cbplen->isChecked();
    w_label->setEnabled(en);
    wpuserlb->setEnabled(en);
    n_label->setEnabled(en);
    npuserlb->setEnabled(en);
    wp_to_np->setEnabled(en);
    np_to_wp->setEnabled(en);
}

void KDMConvenienceWidget::save()
{
    c->setGroup("KDM");

    c->writeEntry( "AutoLoginEnable", cbalen->isChecked() );
    c->writeEntry( "AutoLoginUser", userlb->text(userlb->currentItem()) );
    c->writeEntry( "AutoLogin1st", cbal1st->isChecked() );

    c->writeEntry( "NoPassEnable", cbplen->isChecked() );
    QString npusrstr;
    for(uint i = 0, j = npuserlb->count(); i < j; i++) {
        npusrstr.append(npuserlb->text(i));
        npusrstr.append(",");
    }
    c->writeEntry( "NoPassUsers", npusrstr );

    c->writeEntry( "AutoReLogin", cbarlen->isChecked() );
    c->writeEntry( "ShowPrevious", cbshwen->isChecked() );
}

#define CHECK_STRING( x) (x != 0 && x[0] != 0)

void KDMConvenienceWidget::load(QStringList *show_users)
{
    c->setGroup("KDM");

    show_users->remove("root");
    show_users->sort();
    userlb->clear();
    userlb->insertStringList(*show_users);

    cbalen->setChecked(c->readBoolEntry( "AutoLoginEnable", false) );
    QString user = c->readEntry( "AutoLoginUser" );
    for (uint i = 0, j = userlb->count(); i < j; i++)
	if (userlb->text(i) == user)
	    userlb->setCurrentItem(i);
    userlb->ensureCurrentVisible();
    cbal1st->setChecked(c->readBoolEntry( "AutoLogin1st", true) );

    cbplen->setChecked(c->readBoolEntry( "NoPassEnable", false) );
    QStringList npusers = c->readListEntry( "NoPassUsers");
    QStringList wpusers;
    for (QStringList::ConstIterator it = show_users->begin(), 
	    et = show_users->end(); it != et; ++it)
	if (npusers.contains(*it) == 0)
	    wpusers.append(*it);
    npuserlb->clear();
    npuserlb->insertStringList(npusers);
    wpuserlb->clear();
    wpuserlb->insertStringList(wpusers);

    cbarlen->setChecked(c->readBoolEntry( "AutoReLogin", false) );
    cbshwen->setChecked(c->readBoolEntry( "ShowPrevious", false) );

    slotEnALChanged();
    slotEnPLChanged();
}

#undef CHECK_STRING

void KDMConvenienceWidget::defaults()
{
    cbalen->setChecked(false);
    cbal1st->setChecked(true);
    cbplen->setChecked(false);
    cbarlen->setChecked(false);
    cbshwen->setChecked(false);
}


void KDMConvenienceWidget::slotChanged()
{
  emit KCModule::changed(true);
}
