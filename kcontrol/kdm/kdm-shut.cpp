/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1997-1998 Thomas Tanghus (tanghus@earthling.net)

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


#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <ksimpleconfig.h>
#include <karrowbutton.h>
#include <klineedit.h>
#include <klocale.h>
#include <kdialog.h>
#include <kurlrequester.h>

#include "kdm-shut.h"
#include "kbackedcombobox.h"

extern KSimpleConfig *config;


KDMSessionsWidget::KDMSessionsWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
      QString wtstr;


      QGroupBox *group0 = new QGroupBox( i18n("Allow Shutdown"), this );

      sdlcombo = new QComboBox( false, group0 );
      sdllabel = new QLabel (sdlcombo, i18n ("&Local:"), group0);
      sdlcombo->insertItem(i18n("Everybody"), SdAll);
      sdlcombo->insertItem(i18n("Only Root"), SdRoot);
      sdlcombo->insertItem(i18n("Nobody"), SdNone);
      connect(sdlcombo, SIGNAL(activated(int)), SLOT(changed()));
      sdrcombo = new QComboBox( false, group0 );
      sdrlabel = new QLabel (sdrcombo, i18n ("&Remote:"), group0);
      sdrcombo->insertItem(i18n("Everybody"), SdAll);
      sdrcombo->insertItem(i18n("Only Root"), SdRoot);
      sdrcombo->insertItem(i18n("Nobody"), SdNone);
      connect(sdrcombo, SIGNAL(activated(int)), SLOT(changed()));
      QWhatsThis::add( group0, i18n("Here you can select who is allowed to shutdown"
        " the computer using KDM. You can specify different values for local (console) and remote displays. "
	"Possible values are:<ul>"
        " <li><em>Everybody:</em> everybody can shutdown the computer using KDM</li>"
        " <li><em>Only root:</em> KDM will only allow shutdown after the user has entered the root password</li>"
        " <li><em>Nobody:</em> nobody can shutdown the computer using KDM</li></ul>") );


      QGroupBox *group1 = new QGroupBox( i18n("Commands"), this );

      shutdown_lined = new KUrlRequester(group1);
      QLabel *shutdown_label = new QLabel(shutdown_lined, i18n("H&alt:"), group1);
      connect(shutdown_lined, SIGNAL(textChanged(const QString&)),
	      SLOT(changed()));
      wtstr = i18n("Command to initiate the system halt. Typical value: /sbin/halt");
      QWhatsThis::add( shutdown_label, wtstr );
      QWhatsThis::add( shutdown_lined, wtstr );

      restart_lined = new KUrlRequester(group1);
      QLabel *restart_label = new QLabel(restart_lined, i18n("Reb&oot:"), group1);
      connect(restart_lined, SIGNAL(textChanged(const QString&)),
	      SLOT(changed()));
      wtstr = i18n("Command to initiate the system reboot. Typical value: /sbin/reboot");
      QWhatsThis::add( restart_label, wtstr );
      QWhatsThis::add( restart_lined, wtstr );


      QGroupBox *group4 = new QGroupBox( i18n("Miscellaneous"), this );

      bm_combo = new KBackedComboBox( group4 );
      bm_combo->insertItem("None", i18nc("boot manager", "None"));
      bm_combo->insertItem("Grub", i18n("Grub"));
#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
      bm_combo->insertItem("Lilo", i18n("Lilo"));
#endif
      QLabel *bm_label = new QLabel( bm_combo, i18n("Boot manager:"), group4 );
      connect(bm_combo, SIGNAL(activated(int)), SLOT(changed()));
      wtstr = i18n("Enable boot options in the \"Shutdown...\" dialog.");
      QWhatsThis::add( bm_label, wtstr );
      QWhatsThis::add( bm_combo, wtstr );

      QBoxLayout *main = new QVBoxLayout( this, 10 );
      QGridLayout *lgroup0 = new QGridLayout( group0, 1, 1, 10);
      QGridLayout *lgroup1 = new QGridLayout( group1, 1, 1, 10);
      QGridLayout *lgroup4 = new QGridLayout( group4, 1, 1, 10);

      main->addWidget(group0);
      main->addWidget(group1);
      main->addWidget(group4);
      main->addStretch();

      lgroup0->addRowSpacing(0, group0->fontMetrics().height()/2);
      lgroup0->addColSpacing(2, KDialog::spacingHint() * 2);
      lgroup0->setColStretch(1, 1);
      lgroup0->setColStretch(4, 1);
      lgroup0->addWidget(sdllabel, 1, 0);
      lgroup0->addWidget(sdlcombo, 1, 1);
      lgroup0->addWidget(sdrlabel, 1, 3);
      lgroup0->addWidget(sdrcombo, 1, 4);

      lgroup1->addRowSpacing(0, group1->fontMetrics().height()/2);
      lgroup1->addColSpacing(2, KDialog::spacingHint() * 2);
      lgroup1->setColStretch(1, 1);
      lgroup1->setColStretch(4, 1);
      lgroup1->addWidget(shutdown_label, 1, 0);
      lgroup1->addWidget(shutdown_lined, 1, 1);
      lgroup1->addWidget(restart_label, 1, 3);
      lgroup1->addWidget(restart_lined, 1, 4);

      lgroup4->addRowSpacing(0, group4->fontMetrics().height()/2);
      lgroup4->addWidget(bm_label, 1, 0);
      lgroup4->addWidget(bm_combo, 1, 1);
      lgroup4->setColStretch(2, 1);

      main->activate();

}

void KDMSessionsWidget::makeReadOnly()
{
    sdlcombo->setEnabled(false);
    sdrcombo->setEnabled(false);

    restart_lined->lineEdit()->setReadOnly(true);
    restart_lined->button()->setEnabled(false);
    shutdown_lined->lineEdit()->setReadOnly(true);
    shutdown_lined->button()->setEnabled(false);

    bm_combo->setEnabled(false);
}

void KDMSessionsWidget::writeSD(QComboBox *combo)
{
    QString what;
    switch (combo->currentIndex()) {
    case SdAll: what = "All"; break;
    case SdRoot: what = "Root"; break;
    default: what = "None"; break;
    }
    config->writeEntry( "AllowShutdown", what);
}

void KDMSessionsWidget::save()
{
    config->setGroup("X-:*-Core");
    writeSD(sdlcombo);

    config->setGroup("X-*-Core");
    writeSD(sdrcombo);

    config->setGroup("Shutdown");
    config->writeEntry("HaltCmd", shutdown_lined->url(), KConfigBase::Persistent);
    config->writeEntry("RebootCmd", restart_lined->url(), KConfigBase::Persistent);

    config->writeEntry("BootManager", bm_combo->currentId());
}

void KDMSessionsWidget::readSD(QComboBox *combo, QString def)
{
  QString str = config->readEntry("AllowShutdown", def);
  SdModes sdMode;
  if(str == "All")
    sdMode = SdAll;
  else if(str == "Root")
    sdMode = SdRoot;
  else
    sdMode = SdNone;
  combo->setCurrentIndex(sdMode);
}

void KDMSessionsWidget::load()
{
  config->setGroup("X-:*-Core");
  readSD(sdlcombo, "All");

  config->setGroup("X-*-Core");
  readSD(sdrcombo, "Root");

  config->setGroup("Shutdown");
  restart_lined->setURL(config->readEntry("RebootCmd", "/sbin/reboot"));
  shutdown_lined->setURL(config->readEntry("HaltCmd", "/sbin/halt"));

  bm_combo->setCurrentId(config->readEntry("BootManager", "None"));
}



void KDMSessionsWidget::defaults()
{
  restart_lined->setURL("/sbin/reboot");
  shutdown_lined->setURL("/sbin/halt");

  sdlcombo->setCurrentIndex(SdAll);
  sdrcombo->setCurrentIndex(SdRoot);

  bm_combo->setCurrentId("None");
}


void KDMSessionsWidget::changed()
{
  emit changed(true);
}

#include "kdm-shut.moc"
