/*
 * locale.cpp
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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
#include <qdir.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>

#include "locale.h"
#include "locale.moc"
#include <stdlib.h>
#include "config.h"	// needed for setenv() on HP-UX...

KLocaleConfig::KLocaleConfig(QWidget *parent, const char *name)
  : KConfigWidget (parent, name)
{
    locale = new KLocale("kdelibs");
    
    // allow localization of numbers and money
    locale->enableNumericLocale(true);
    
    QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);
    QGroupBox *gbox = new QGroupBox(i18n("Language"), this);
    tl->addWidget(gbox);
    
    QGridLayout *tl1 = new QGridLayout(gbox, 5, 4, 5);
    tl1->addRowSpacing(0, 15);
    tl1->addRowSpacing(4, 10);
    tl1->addColSpacing(0, 10);
    tl1->addColSpacing(3, 10);
    tl1->setColStretch(2, 1);
    
    changedFlag = FALSE;
    
    QLabel *label = new QLabel(i18n("&First"), gbox);
    label->setMinimumSize(label->sizeHint());
    combo1 = new KLanguageCombo(gbox);
    combo1->setMinimumWidth(combo1->sizeHint().width());
    combo1->setFixedHeight(combo1->sizeHint().height());
    label->setBuddy(combo1);
    connect(combo1,SIGNAL(highlighted(int)),this,SLOT(changed(int)));
    tl1->addWidget(label, 1, 1);
    tl1->addWidget(combo1, 1, 2);
    
    label = new QLabel(i18n("&Second"), gbox);
    label->setMinimumSize(label->sizeHint());
    combo2 = new KLanguageCombo(gbox);
    combo2->setMinimumWidth(combo2->sizeHint().width());
    combo2->setFixedHeight(combo2->sizeHint().height());
    label->setBuddy(combo2);
    connect(combo2,SIGNAL(highlighted(int)),this,SLOT(changed(int)));
    tl1->addWidget(label, 2, 1);
    tl1->addWidget(combo2, 2, 2);
    
    label = new QLabel(i18n("&Third"), gbox);
    label->setMinimumSize(label->sizeHint());
    combo3 = new KLanguageCombo(gbox);
    combo3->setMinimumWidth(combo3->sizeHint().width());
    combo3->setFixedHeight(combo3->sizeHint().height());
    label->setBuddy(combo3);
    connect(combo3,SIGNAL(highlighted(int)),this,SLOT(changed(int)));
    tl1->addWidget(label, 3, 1);
    tl1->addWidget(combo3, 3, 2);
    
    tl1->activate();
    
    gbox = new QGroupBox(i18n("Examples"), this);
    tl->addWidget(gbox);
    
    tl1 = new QGridLayout(gbox, 6, 4, 5);
    tl1->addRowSpacing(0, 15);
    tl1->addRowSpacing(5, 10);
    tl1->addColSpacing(0, 10);
    tl1->addColSpacing(3, 10);
    tl1->setColStretch(2, 1);
    
    label = new QLabel(i18n("Numbers:"), gbox);
    label->setMinimumSize(label->sizeHint());
    tl1->addWidget(label, 1, 1);
    
    numberSample = new QLabel(gbox);
    tl1->addWidget(numberSample, 1, 2);
    numberSample->setText(locale->formatNumber(1234567.89) +
			  " / " +
			  locale->formatNumber(-1234567.89));
    
    label = new QLabel(i18n("Money:"), gbox);
    label->setMinimumSize(label->sizeHint());
    tl1->addWidget(label, 2, 1);
    
    moneySample = new QLabel(gbox);
    tl1->addWidget(moneySample, 2, 2);
    moneySample->setText(locale->formatMoney(123456789.00) +
			 " / " + 
			 locale->formatMoney(-123456789.00));
    
    label = new QLabel(i18n("Date:"), gbox);
    label->setMinimumSize(label->sizeHint());
    tl1->addWidget(label, 3, 1);
    
    dateSample = new QLabel(gbox);
    tl1->addWidget(dateSample, 3, 2);
    dateSample->setText(locale->formatDate(QDate::currentDate()));
    
    label = new QLabel(i18n("Time:"), gbox);
    label->setMinimumSize(label->sizeHint());
    tl1->addWidget(label, 4, 1);
    
    timeSample = new QLabel(gbox);
    tl1->addWidget(timeSample, 4, 2);
    timeSample->setText(locale->formatTime(QTime::currentTime()));
    
    tl->addStretch(1);
    tl->activate();

    loadSettings();
}


KLocaleConfig::~KLocaleConfig ()
{
}


void KLocaleConfig::loadLanguageList(KLanguageCombo *combo)
{
  KConfigBase *config = KGlobal::config();
  QString name;

  combo->clear();
  tags.clear();

  config->setGroup("C");
  combo->insertLanguage("C;" + config->readEntry("Name"));
  tags.append("C");
 
  QStringList filelist = KGlobal::dirs()->findAllResources("locale",
							   "*/entry.desktop");
  for ( QStringList::ConstIterator it = filelist.begin();
	it != filelist.end(); ++it )
    {
	KSimpleConfig entry(*it);
	entry.setGroup("KCM Locale");
	name = entry.readEntry("Language");
	if (name.isEmpty())
	    name = "without name!";
	
	QString path = *it;
	int index = path.findRev('/');
	path = path.left(index);
	index = path.findRev('/');
	path = path.mid(index+1);
	combo->insertLanguage(path + ";" + name);
	tags.append(path);
    }
}


void KLocaleConfig::loadSettings()
{
  KConfig *config = KGlobal::config();

  loadLanguageList(combo1);
  loadLanguageList(combo2);
  loadLanguageList(combo3);


  // This code is adopted from klocale.cpp

  QString languages, lang;
  int i=0, pos;

  config->setGroup("Locale");
  languages = config->readEntry("Language");

  while (1) {
    lang = languages.left(languages.find(':'));
    languages = languages.right(languages.length() - lang.length() - 1);
    if (lang.isEmpty() || lang == "C")
        break;
    i++;
    switch (i) {
      case 1: pos = tags.findIndex(lang); 
              if (pos >= 0)
                combo1->setCurrentItem(pos);
              break;
      case 2: pos = tags.findIndex(lang); 
              if (pos >= 0)
                combo2->setCurrentItem(pos);
              break;
      case 3: pos = tags.findIndex(lang); 
              if (pos >= 0)
                combo3->setCurrentItem(pos);
              break;        
      default: return;
    }
  } 
  changed(combo1->currentItem());
}


void KLocaleConfig::applySettings()
{
  KConfigBase *config = KGlobal::config();

  QString value;

  value = QString("%1:%2:%3")
                            .arg(*tags.at(combo1->currentItem()))
                            .arg(*tags.at(combo2->currentItem()))
                            .arg(*tags.at(combo3->currentItem()));

  config->setGroup("Locale");
  config->writeEntry("Language", value);  
  config->sync();

  if (changedFlag)
    QMessageBox::information(this,i18n("Applying language settings"),
			     i18n("Changed language settings apply only to newly started "
				  "applications.\nTo change the language of all "
				  "programs, you will have to logout first."),
                             i18n("OK"));

  changedFlag = FALSE;
}


void KLocaleConfig::defaultSettings()
{
  combo1->setCurrentItem(0);
  combo2->setCurrentItem(0);
  combo3->setCurrentItem(0);
}


void KLocaleConfig::changed(int i)
{
  changedFlag = TRUE;

  delete locale;
  setenv("KDE_LANG", (*tags.at(i)).ascii(), 1);
  locale = new KLocale("kdelibs");

  debug("changed %s", locale->language().ascii());
  // should update the locale here before redisplaying samples.
  numberSample->setText(locale->formatNumber(1234567.89) +
			" / " +
			locale->formatNumber(-1234567.89));

  moneySample->setText(locale->formatMoney(123456789.00) +
		       " / " + 
		       locale->formatMoney(-123456789.00));
  dateSample->setText(locale->formatDate(QDate::currentDate()));
  timeSample->setText(locale->formatTime(QTime::currentTime()));
}
