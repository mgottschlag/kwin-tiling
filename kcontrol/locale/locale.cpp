/*
 * locale.cpp
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
 * Copyright (c) 1999 Hans Petter Bieker <bieker@pvv.ntnu.no>
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
#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qobjectlist.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>

#include "locale.h"
#include "locale.moc"

#define i18n(a) (a)

KLocaleConfig::KLocaleConfig(QWidget *parent, const char *name)
  : KConfigWidget (parent, name)
{
    locale =  KGlobal::locale();

    QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);
    QGroupBox *gbox = new QGroupBox(this, i18n("Locale"));
    tl->addWidget(gbox);

    QGridLayout *tl1 = new QGridLayout(gbox, 5, 4, 5);
    tl1->addRowSpacing(0, 15);
    tl1->addRowSpacing(4, 10);
    tl1->addColSpacing(0, 10);
    tl1->addColSpacing(3, 10);
    tl1->setColStretch(2, 1);

    changedFlag = FALSE;

    QLabel *label = new QLabel(gbox, i18n("&Country"));
    comboCountry = new KLanguageCombo(gbox);
    //comboCountry->setMinimumWidth(comboCountry->sizeHint().width());
    comboCountry->setFixedHeight(comboCountry->sizeHint().height());
    label->setBuddy(comboCountry);
    connect(comboCountry,SIGNAL(highlighted(int)),this,SLOT(changedCountry(int)));
    tl1->addWidget(label, 1, 1);
    tl1->addWidget(comboCountry, 1, 2);

    label = new QLabel(gbox, i18n("&Language"));
    comboLang = new KLanguageCombo(gbox);
    //comboLang->setMinimumWidth(comboLang->sizeHint().width());
    comboLang->setFixedHeight(comboLang->sizeHint().height());
    label->setBuddy(comboLang);
    connect(comboLang,SIGNAL(highlighted(int)),this,SLOT(changedLanguage(int)));
    tl1->addWidget(label, 2, 1);
    tl1->addWidget(comboLang, 2, 2);

    label = new QLabel(gbox, i18n("&Numbers"));
    comboNumber = new KLanguageCombo(gbox);
    //comboNumber->setMinimumWidth(comboNumber->sizeHint().width());
    comboNumber->setFixedHeight(comboNumber->sizeHint().height());
    label->setBuddy(comboNumber);
    connect(comboNumber,SIGNAL(highlighted(int)),this,SLOT(changedNumber(int)));
    tl1->addWidget(label, 3, 1);
    tl1->addWidget(comboNumber, 3, 2);

    label = new QLabel(gbox, i18n("&Money"));
    comboMoney = new KLanguageCombo(gbox);
    //comboMoney->setMinimumWidth(comboMoney->sizeHint().width());
    comboMoney->setFixedHeight(comboMoney->sizeHint().height());
    label->setBuddy(comboMoney);
    connect(comboMoney,SIGNAL(highlighted(int)),this,SLOT(changedMoney(int)));
    tl1->addWidget(label, 4, 1);
    tl1->addWidget(comboMoney, 4, 2);

    label = new QLabel(gbox, i18n("&Date and time"));
    comboDate = new KLanguageCombo(gbox);
    //comboDate->setMinimumWidth(comboDate->sizeHint().width());
    comboDate->setFixedHeight(comboDate->sizeHint().height());
    label->setBuddy(comboDate);
    connect(comboDate,SIGNAL(highlighted(int)),this,SLOT(changedTime(int)));
    tl1->addWidget(label, 5, 1);
    tl1->addWidget(comboDate, 5, 2);

    tl1->activate();

    gbox = new QGroupBox(this, i18n("Examples"));
    tl->addWidget(gbox);

    tl1 = new QGridLayout(gbox, 6, 4, 5);
    tl1->addRowSpacing(0, 15);
    tl1->addRowSpacing(5, 10);
    tl1->addColSpacing(0, 10);
    tl1->addColSpacing(3, 10);
    tl1->setColStretch(2, 1);

    label = new QLabel("1", gbox, i18n("Numbers:"));
    tl1->addWidget(label, 1, 1);

    numberSample = new QLabel(gbox);
    tl1->addWidget(numberSample, 1, 2);

    label = new QLabel("1", gbox, i18n("Money:"));
    tl1->addWidget(label, 2, 1);

    moneySample = new QLabel(gbox);
    tl1->addWidget(moneySample, 2, 2);

    label = new QLabel("1", gbox, i18n("Date:"));
    tl1->addWidget(label, 3, 1);

    dateSample = new QLabel(gbox);
    tl1->addWidget(dateSample, 3, 2);

    label = new QLabel("1", gbox, i18n("Time:"));
    tl1->addWidget(label, 4, 1);

    timeSample = new QLabel(gbox);
    tl1->addWidget(timeSample, 4, 2);

    tl->addStretch(1);
    tl->activate();

    updateSample();
    loadSettings();
}

KLocaleConfig::~KLocaleConfig ()
{
}

void KLocaleConfig::loadLanguageList(KLanguageCombo *combo, const QStringList &first)
{
  QString name;

  combo->clear();
  combo->tags.clear();

  QStringList filelist;
  for ( QStringList::ConstIterator it = first.begin(); it != first.end(); ++it )
    {
        QString str = locate("locale", *it + "/entry.desktop");
        if (!str.isNull())
          filelist << str;
        printf("%s\n", str.ascii());
    }
  if (filelist.isEmpty())
  {
    KConfigBase *config = KGlobal::config();
    config->setGroup("C");
    combo->insertLanguage("C", config->readEntry("Name"));

    filelist = KGlobal::dirs()->findAllResources("locale",
							   "*/entry.desktop");
  }

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
	combo->insertLanguage(path, name);
    }
}

void KLocaleConfig::loadCountryList(KLanguageCombo *combo, const QStringList &first)
{
  KConfigBase *config = KGlobal::config();
  QString name;

  combo->clear();
  combo->tags.clear();

  config->setGroup("C");
  combo->insertLanguage("C", config->readEntry("Name"), "l10n/");

  QStringList filelist = KGlobal::dirs()->findAllResources("locale",
							   "l10n/*/entry.desktop");
  for ( QStringList::ConstIterator it = first.fromLast(); it != first.end(); --it )
    {
        int i;
        if ((i = filelist.findIndex(locate("locale", "l10n/" + *it + "/entry.desktop"))) > 0)
          {
              filelist.prepend(*filelist.at(i));
              filelist.remove(filelist.at(i + 1));
          }
    }

  for ( QStringList::ConstIterator it = filelist.begin();
	it != filelist.end(); ++it )
    {
	KSimpleConfig entry(*it);
	entry.setGroup("KCM Locale");
	name = entry.readEntry("Name");
	if (name.isEmpty())
	    name = "without name!";
	
	QString path = *it;
	int index = path.findRev('/');
	path = path.left(index);
	index = path.findRev('/');
	path = path.mid(index+1);
	combo->insertLanguage(path, name, "l10n/");
    }
}

void KLocaleConfig::loadSettings()
{
  loadCountryList(comboCountry, 0);
  loadLanguageList(comboLang, 0);
  loadCountryList(comboNumber, 0);
  loadCountryList(comboMoney, 0);
  loadCountryList(comboDate, 0);

  KConfig *config = KGlobal::config();
  config->setGroup("Locale");

  QString str = config->readEntry("Country");
  comboCountry->setItem(str);

  // Language
  str = config->readEntry("Language");
  str = str.left(str.find(':')); // for compatible -- FIXME in KDE3
  comboLang->setItem(str);

  // Numeric
  str = config->readEntry("Numeric");
  comboNumber->setItem(str);

  // Money
  str = config->readEntry("Monetary");
  comboMoney->setItem(str);

  // Date and time
  str = config->readEntry("Time");
  comboDate->setItem(str);
}

void KLocaleConfig::applySettings()
{
  KConfigBase *config = KGlobal::config();

  config->setGroup("Locale");

  config->writeEntry("Country", comboCountry->currentTag());
  config->writeEntry("Language", comboLang->currentTag());
  config->writeEntry("Numeric", comboNumber->currentTag());
  config->writeEntry("Monetary", comboMoney->currentTag());
  config->writeEntry("Time", comboDate->currentTag());

  config->sync();

  if (changedFlag)
    KMessageBox::information(this,
			     i18n("Changed language settings apply only to newly started "
				  "applications.\nTo change the language of all "
				  "programs, you will have to logout first."),
                             i18n("Applying language settings"));

  changedFlag = FALSE;
}

void KLocaleConfig::defaultSettings()
{
  comboCountry->setCurrentItem(0);
  comboLang->setCurrentItem(0);
  comboNumber->setCurrentItem(0);
  comboMoney->setCurrentItem(0);
  comboDate->setCurrentItem(0);
}

void KLocaleConfig::changedCountry(int i)
{
  changedFlag = TRUE;

  QString country = *comboCountry->tags.at(i);

  KSimpleConfig ent(locate("locale", "l10n/" + country + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");
  QStringList langs = ent.readListEntry("Languages");
  if (langs.isEmpty()) langs = QString("C");

  locale->setLanguage(*langs.at(0),
                      *langs.at(0), // FIXME -- this is obsoleted
                      country,
                      country,
                      country);

//  loadCountryList(comboCountry, country);
  loadLanguageList(comboLang, langs);
//  loadCountryList(comboNumber, country);
//  loadCountryList(comboMoney, country);
//  loadCountryList(comboDate, country);


  comboLang->setItem((*langs.at(0)).isNull()?QString::fromLatin1("C"):*langs.at(0));
  comboNumber->setItem(country);
  comboMoney->setItem(country);
  comboDate->setItem(country);

  updateSample();
}

void KLocaleConfig::changedLanguage(int i)
{
  changedFlag = TRUE;

  locale->setLanguage(*comboLang->tags.at(i),
                      *comboLang->tags.at(i),
                      QString::null,
                      QString::null,
                      QString::null);
  updateSample();
}

void KLocaleConfig::changedNumber(int i)
{
  changedFlag = TRUE;

  locale->setLanguage(QString::null,
                      QString::null,
                      *comboMoney->tags.at(i),
                      QString::null,
                      QString::null);
  updateSample();
}

void KLocaleConfig::changedMoney(int i)
{
  changedFlag = TRUE;

  locale->setLanguage(QString::null,
                      QString::null,
                      QString::null,
                      *comboDate->tags.at(i),
                      QString::null);
  updateSample();
}

void KLocaleConfig::changedTime(int i)
{
  changedFlag = TRUE;

  locale->setLanguage(QString::null,
                      QString::null,
                      QString::null,
                      QString::null,
                      *comboDate->tags.at(i));
  updateSample();
}

#undef i18n
void scani18n(QObjectListIt it)
{
    QObject *wc;
    while( wc = it.current() ) {
      ++it;
      if (wc->children())
        scani18n(QObjectListIt(*wc->children()));

      if ( !qstrcmp(wc->name(), "unnamed") )
         continue;
      if ( !wc->isWidgetType() )
         continue;

      if ( !qstrcmp(wc->className(), "QGroupBox"))
      {
        ((QGroupBox *)wc)->setTitle(i18n(wc->name()));
        ((QGroupBox *)wc)->setMinimumSize(((QGroupBox *)wc)->sizeHint());
      }
      else if ( !qstrcmp(wc->className(), "QLabel"))
      {
        ((QLabel *)wc)->setText(i18n(wc->name()));
        ((QLabel *)wc)->setMinimumSize(((QLabel *)wc)->sizeHint());
      }
    }
}
#define i18n(a) (a)

void KLocaleConfig::updateSample()
{
  numberSample->setText(locale->formatNumber(1234567.89) +
			" / " +
			locale->formatNumber(-1234567.89));

  moneySample->setText(locale->formatMoney(123456789.00) +
		       " / " +
		       locale->formatMoney(-123456789.00));
  dateSample->setText(locale->formatDate(QDate::currentDate()));
  timeSample->setText(locale->formatTime(QTime::currentTime()));

  QObject *w = this;
  while (w->parent())
    w = w->parent();
  scani18n(QObjectListIt(*w->children() ));
}
