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


#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

#include <kdialog.h>
#include <ksimpleconfig.h>
#include <kfontrequester.h>
#include <klocale.h>

#include "kdm-font.h"


extern KSimpleConfig *config;

KDMFontWidget::KDMFontWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  QGridLayout *ml = new QGridLayout(this, 5, 2, KDialog::marginHint(), KDialog::spacingHint());
  QLabel *label = new QLabel(i18n("&General:"), this);
  stdFontChooser = new KFontRequester(this);
  label->setBuddy(stdFontChooser);
  QWhatsThis::add( stdFontChooser, i18n("This changes the font which is used for all the text in the login manager except for the greeting and failure messages.") );
  connect(stdFontChooser, SIGNAL(fontSelected(const QFont&)),this,SLOT(configChanged()));
  ml->addWidget(label, 1, 0);
  ml->addWidget(stdFontChooser, 1, 1);

  label = new QLabel(i18n("&Failures:"), this);
  failFontChooser = new KFontRequester(this);
  label->setBuddy(failFontChooser);
  QWhatsThis::add( failFontChooser, i18n("This changes the font which is used for failure messages in the login manager.") );
  connect(failFontChooser, SIGNAL(fontSelected(const QFont&)),this,SLOT(configChanged()));
  ml->addWidget(label, 2, 0);
  ml->addWidget(failFontChooser, 2, 1);

  label = new QLabel(i18n("Gree&ting:"), this);
  greetingFontChooser = new KFontRequester(this);
  label->setBuddy(greetingFontChooser);
  QWhatsThis::add( greetingFontChooser, i18n("This changes the font which is used for the login manager's greeting.") );
  connect(greetingFontChooser, SIGNAL(fontSelected(const QFont&)),this,SLOT(configChanged()));
  ml->addWidget(label, 3, 0);
  ml->addWidget(greetingFontChooser, 3, 1);

  aacb = new QCheckBox (i18n("Use anti-aliasing for fonts"), this);
  QWhatsThis::add( aacb, i18n("If you check this box and your X-Server has the Xft extension, "
	"fonts will be antialiased (smoothed) in the login dialog.") );
  connect(aacb, SIGNAL(toggled ( bool )),this,SLOT(configChanged()));
  ml->addMultiCellWidget(aacb, 4, 4, 0, 1);
  ml->setRowStretch(5, 10);
}

void KDMFontWidget::makeReadOnly()
{
  stdFontChooser->button()->setEnabled(false);
  failFontChooser->button()->setEnabled(false);
  greetingFontChooser->button()->setEnabled(false);
  aacb->setEnabled(false);
}

void KDMFontWidget::configChanged()
{
    emit changed(true);
}

void KDMFontWidget::set_def()
{
  stdFontChooser->setFont(QFont("Sans Serif", 12));
  failFontChooser->setFont(QFont("Sans Serif", 12, QFont::Bold));
  greetingFontChooser->setFont(QFont("Serif", 24));
}

void KDMFontWidget::save()
{
  config->setGroup("X-*-Greeter");

  // write font
  config->writeEntry("StdFont", stdFontChooser->font());
  config->writeEntry("GreetFont", greetingFontChooser->font());
  config->writeEntry("FailFont", failFontChooser->font());
  config->writeEntry("AntiAliasing", aacb->isChecked());
}


void KDMFontWidget::load()
{
  set_def();

  config->setGroup("X-*-Greeter");

  // Read the fonts
  QFont font = stdFontChooser->font();
  stdFontChooser->setFont(config->readFontEntry("StdFont", &font));
  font = failFontChooser->font();
  failFontChooser->setFont(config->readFontEntry("FailFont", &font));
  font = greetingFontChooser->font();
  greetingFontChooser->setFont(config->readFontEntry("GreetFont",  &font));

  aacb->setChecked(config->readBoolEntry("AntiAliasing"));
}


void KDMFontWidget::defaults()
{
  set_def();
  aacb->setChecked(false);
}

#include "kdm-font.moc"
