/**
 *  sample.cpp
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>

#include "sample.h"

KSampleConfig::KSampleConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  QVBoxLayout *topLayout = new QVBoxLayout(this, 5);

  bGrp = new QButtonGroup(1, Qt::Vertical,
						  i18n("Sample optionss"), this);
  connect(bGrp, SIGNAL(clicked(int)), this, SLOT(configChanged()));

  topLayout->addWidget(bGrp);

  button1 = new QRadioButton(i18n("&Some option"), bGrp);
  button2 = new QRadioButton(i18n("Some &other option"), bGrp);
  button3 = new QRadioButton(i18n("&Yet another option"), bGrp);

  load();
}

KSampleConfig::~KSampleConfig() {}

void KSampleConfig::configChanged()
{
  emit changed(true);
}

void KSampleConfig::load()
{
  KConfig *config = new KConfig("kcmsamplerc", true);
  
  config->setGroup("blahgroup");
  bGrp->setButton(config->readNumEntry("blahoption", 0));
  delete config;

  emit changed(false);
}

void KSampleConfig::save()
{

  KConfig *config= new KConfig("kcmsamplerc", false);

  config->setGroup("blahgroup");

  if (button1->isChecked())
	config->writeEntry("blahoption", 0);
  else if (button2->isChecked())
    config->writeEntry("blahoption", 1);
  else
	config->writeEntry("blahoption", 2);
  
  config->sync();
  delete config;

  emit changed(false);
}

void KSampleConfig::defaults()
{
  bGrp->setButton(0);
  
  emit changed(true);
}

extern "C"
{
  KCModule *create_email(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmsample");
    return new KSampleConfig(parent, name);
  };
}
