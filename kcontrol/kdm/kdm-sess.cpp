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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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

#include "kdm-sess.h"


extern KSimpleConfig *config;

KDMSessionsWidget::KDMSessionsWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
      QString wtstr;


      QGroupBox *group0 = new QGroupBox( i18n("Allow Shutdown"), this );

      sdlcombo = new QComboBox( FALSE, group0 );
      sdllabel = new QLabel (sdlcombo, i18n ("&Local:"), group0);
      sdlcombo->insertItem(i18n("Everybody"), SdAll);
      sdlcombo->insertItem(i18n("Only Root"), SdRoot);
      sdlcombo->insertItem(i18n("Nobody"), SdNone);
      connect(sdlcombo, SIGNAL(activated(int)), SLOT(changed()));
      sdrcombo = new QComboBox( FALSE, group0 );
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


#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
      QGroupBox *group4 = new QGroupBox( i18n("LILO"), this );

      lilo_check = new QCheckBox(i18n("Show boot opt&ions"), group4);
      connect(lilo_check, SIGNAL(toggled(bool)),
	      SLOT(changed()));
      wtstr = i18n("Enable LILO boot options in the \"Shutdown...\" dialog.");
      QWhatsThis::add( lilo_check, wtstr );
#endif

      QBoxLayout *main = new QVBoxLayout( this, 10 );
      QGridLayout *lgroup0 = new QGridLayout( group0, 3, 5, 10);
#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
      QGridLayout *lgroup4 = new QGridLayout( group4, 3, 4, 10);
#endif

      main->addWidget(group0);
#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
      main->addWidget(group4);
#endif
      main->addStretch();

      lgroup0->addRowSpacing(0, group0->fontMetrics().height()/2);
      lgroup0->addColSpacing(2, KDialog::spacingHint() * 2);
      lgroup0->setColStretch(1, 1);
      lgroup0->setColStretch(4, 1);
      lgroup0->addWidget(sdllabel, 1, 0);
      lgroup0->addWidget(sdlcombo, 1, 1);
      lgroup0->addWidget(sdrlabel, 1, 3);
      lgroup0->addWidget(sdrcombo, 1, 4);

#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
      lgroup4->addRowSpacing(0, group4->fontMetrics().height()/2);
      lgroup4->addWidget(lilo_check, 1, 0);
#endif

      main->activate();

}

void KDMSessionsWidget::makeReadOnly()
{
    sdlcombo->setEnabled(false);
    sdrcombo->setEnabled(false);
#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
    lilo_check->setEnabled(false);
#endif
}

void KDMSessionsWidget::writeSD(QComboBox *combo)
{
    QString what;
    switch (combo->currentItem()) {
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

#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
    config->setGroup("Shutdown");
    config->writeEntry("UseLilo", lilo_check->isChecked());
#endif
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
  combo->setCurrentItem(sdMode);
}

void KDMSessionsWidget::load()
{
  config->setGroup("X-:*-Core");
  readSD(sdlcombo, "All");

  config->setGroup("X-*-Core");
  readSD(sdrcombo, "Root");

#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
  config->setGroup("Shutdown");
  lilo_check->setChecked(config->readBoolEntry("UseLilo", false));
#endif
}



void KDMSessionsWidget::defaults()
{
  sdlcombo->setCurrentItem(SdAll);
  sdrcombo->setCurrentItem(SdRoot);

#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
  lilo_check->setChecked(false);
#endif
}


void KDMSessionsWidget::changed()
{
  emit changed(true);
}

#include "kdm-sess.moc"
