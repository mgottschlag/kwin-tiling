/*
 * locale.cpp
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
 * Copyright (c) 1999 Hans Petter Bieker <bieker@kde.org>
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
#include <qtooltip.h>

#include <kconfig.h>
#include <kglobal.h>

#define KLocaleConfigAdvanced KLocaleConfig
#include <klocale.h>
#undef KLocaleConfigAdvanced

#include <kmessagebox.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>
#include <kcharsets.h>

#include "klangcombo.h"
#include "klocalesample.h"
#include "locale.h"
#include "locale.moc"
#include "main.h"

extern KLocale *locale;

KLocaleConfig::KLocaleConfig(QWidget *parent, const char *name)
  : QWidget (parent, name)
{
    QGridLayout *tl1 = new QGridLayout(this, 1, 1, 10, 5);
    tl1->setRowStretch( 0, 1);
    tl1->setColStretch( 2, 1);

    changedFlag = FALSE;

    QLabel *label = new QLabel(this, I18N_NOOP("&Country"));
    comboCountry = new KLanguageCombo(this);
    comboCountry->setFixedHeight(comboCountry->sizeHint().height());
    label->setBuddy(comboCountry);
    connect( comboCountry, SIGNAL(activated(int)),
	     this, SLOT(changedCountry(int)) );
    tl1->addWidget(label, 1, 1);
    tl1->addWidget(comboCountry, 1, 2);

    label = new QLabel(this, I18N_NOOP("&Language"));
    comboLang = new KLanguageCombo(this);
    comboLang->setFixedHeight(comboLang->sizeHint().height());
    label->setBuddy(comboLang);
    connect( comboLang, SIGNAL(activated(int)),
	     this, SLOT(changedLanguage(int)) );
    tl1->addWidget(label, 2, 1);
    tl1->addWidget(comboLang, 2, 2);

    label = new QLabel(this, I18N_NOOP("&Numbers"));
    comboNumber = new KLanguageCombo(this);
    comboNumber->setFixedHeight(comboNumber->sizeHint().height());
    label->setBuddy(comboNumber);
    connect( comboNumber, SIGNAL(activated(int)),
	     this, SLOT(changedNumber(int)) );
    tl1->addWidget(label, 3, 1);
    tl1->addWidget(comboNumber, 3, 2);

    label = new QLabel(this, I18N_NOOP("&Money"));
    comboMoney = new KLanguageCombo(this);
    comboMoney->setFixedHeight(comboMoney->sizeHint().height());
    label->setBuddy(comboMoney);
    connect( comboMoney, SIGNAL(activated(int)),
	     this, SLOT(changedMoney(int)) );
    tl1->addWidget(label, 4, 1);
    tl1->addWidget(comboMoney, 4, 2);

    label = new QLabel(this, I18N_NOOP("&Date and time"));
    comboDate = new KLanguageCombo(this);
    comboDate->setFixedHeight(comboDate->sizeHint().height());
    label->setBuddy(comboDate);
    connect( comboDate, SIGNAL(activated(int)),
	     this, SLOT(changedTime(int)) );
    tl1->addWidget(label, 5, 1);
    tl1->addWidget(comboDate, 5, 2);

    label = new QLabel(this, I18N_NOOP("&Charset"));
    comboChset = new KLanguageCombo(this);
    comboChset->setFixedHeight(comboChset->sizeHint().height());
    label->setBuddy(comboChset);
    tl1->addWidget(label, 6, 1);
    tl1->addWidget(comboChset, 6, 2);

    QToolTip::add(comboCountry, I18N_NOOP("This is were you live. KDE will use the defaults for this country."));
    QToolTip::add(comboLang, I18N_NOOP("All KDE programs will be displayed in this language (if available)."));
    QToolTip::add(comboNumber, I18N_NOOP("The rules of this country will be used to localize numbers"));
    QToolTip::add(comboMoney, I18N_NOOP("The rules of this country will be used to localize money"));
    QToolTip::add(comboDate, I18N_NOOP("The rules of this country will be used to display time and dates"));
    QToolTip::add(comboChset, I18N_NOOP("The prefered charset for fonts"));

    QStringList list = KGlobal::charsets()->availableCharsetNames();
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
       comboChset->insertItem(QIconSet(), *it, *it);


    tl1->setRowStretch(6,1);
    load();
}

KLocaleConfig::~KLocaleConfig ()
{
}

void KLocaleConfig::loadLocaleList(KLanguageCombo *combo, const QString &sub, const QStringList &first)
{
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = locale;

  QString name;

  // clear the list
  combo->clear();

  QStringList prilang;
  // add the primary languages for the country to the list
  for ( QStringList::ConstIterator it = first.begin(); it != first.end(); ++it )
    {
        QString str = locate("locale", sub + *it + "/entry.desktop");
        if (!str.isNull())
          prilang << str;
    }

  // add all languages to the list
  QStringList alllang = KGlobal::dirs()->findAllResources("locale",
							   sub + "*/entry.desktop");
  alllang.sort();
  QStringList langlist = prilang;
  if (langlist.count() > 0)
    langlist << QString::null; // separator
  langlist += alllang;

  for ( QStringList::ConstIterator it = langlist.begin();
	it != langlist.end(); ++it )
    {
        if ((*it).isNull())
        {
	  combo->insertSeparator();
	  combo->insertOther();
          continue;
        }
	KSimpleConfig entry(*it);
	entry.setGroup("KCM Locale");
	name = entry.readEntry("Name", locale->translate("without name"));
	
	QString path = *it;
	int index = path.findRev('/');
	path = path.left(index);
	index = path.findRev('/');
	path = path.mid(index+1);
	combo->insertLanguage(path, name, sub);
    }
  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfig::load()
{
  loadLocaleList(comboCountry, "l10n/", QStringList());
  loadLocaleList(comboNumber, "l10n/", QStringList());
  loadLocaleList(comboMoney, "l10n/", QStringList());
  loadLocaleList(comboDate, "l10n/", QStringList());

  KConfig *config = KGlobal::config();
  config->setGroup("Locale");

  QString str = config->readEntry("Country");
  comboCountry->setCurrentItem(str);

  KSimpleConfig ent(locate("locale", "l10n/" + str + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");
  QStringList langs = ent.readListEntry("Languages");
  if (langs.isEmpty()) langs = QString("C");

  loadLocaleList(comboLang, 0, langs);

  // Language
  str = config->readEntry("Language");
  str = str.left(str.find(':')); // for compatible -- FIXME in KDE3
  comboLang->setCurrentItem(str);

  // Numeric
  str = config->readEntry("Numeric");
  comboNumber->setCurrentItem(str);

  // Money
  str = config->readEntry("Monetary");
  comboMoney->setCurrentItem(str);

  // Date and time
  str = config->readEntry("Time");
  comboDate->setCurrentItem(str);

  // Charset
  str = config->readEntry(QString::fromLatin1("Charset"), QString::fromLatin1("unicode"));
  comboChset->setCurrentItem(str);
}

void KLocaleConfig::readLocale(const QString &path, QString &name, const QString &sub) const
{
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = locale;

  // read the name
  KSimpleConfig entry(locate("locale", sub + path + "/entry.desktop"));
  entry.setGroup("KCM Locale");
  name = entry.readEntry("Name", locale->translate("without name"));

  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfig::save()
{
  KConfigBase *config = KGlobal::config();

  config->setGroup("Locale");

  config->writeEntry("Country", comboCountry->currentTag(), true, true);
  config->writeEntry("Language", comboLang->currentTag(), true, true);
  config->writeEntry("Numeric", comboNumber->currentTag(), true, true);
  config->writeEntry("Monetary", comboMoney->currentTag(), true, true);
  config->writeEntry("Time", comboDate->currentTag(), true, true);
  config->writeEntry("Charset", comboChset->currentTag(), true, true);

  config->sync();

  if (changedFlag)
    KMessageBox::information(this,
			     locale->translate("Changed language settings apply only to newly started "
				  "applications.\nTo change the language of all "
				  "programs, you will have to logout first."),
                             locale->translate("Applying language settings"));

  changedFlag = FALSE;
}

void KLocaleConfig::defaults()
{
  changedFlag = FALSE;

  locale->setLanguage("C");
  locale->setCountry("C");

  reTranslateLists();
  loadLocaleList(comboLang, 0, QStringList());

  comboCountry->setCurrentItem("C");
  comboLang->setCurrentItem("C");
  comboNumber->setCurrentItem("C");
  comboMoney->setCurrentItem("C");
  comboDate->setCurrentItem("C");
  
  emit resample();
}

void KLocaleConfig::changedCountry(int i)
{
  changedFlag = TRUE;

  QString country = comboCountry->tag(i);

  KSimpleConfig ent(locate("locale", "l10n/" + country + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");
  QStringList langs = ent.readListEntry("Languages");
  if (langs.isEmpty()) langs = QString("C");

  locale->setLanguage(*langs.at(0));
  locale->setCountry(country);

  reTranslateLists();
  loadLocaleList(comboLang, 0, langs);

  comboLang->setCurrentItem(*langs.at(0));
  comboNumber->setCurrentItem(country);
  comboMoney->setCurrentItem(country);
  comboDate->setCurrentItem(country);

  emit resample();
  emit countryChanged();
}

void KLocaleConfig::changedLanguage(int i)
{
  changedFlag = TRUE;

  locale->setLanguage(comboLang->tag(i));

  reTranslateLists();

  emit resample();
}

void KLocaleConfig::changedNumber(int i)
{
  changedFlag = TRUE;

  locale->setCountry(comboMoney->tag(i),
                     QString::null,
                     QString::null);
  emit resample();
}

void KLocaleConfig::changedMoney(int i)
{
  changedFlag = TRUE;

  locale->setCountry(QString::null,
                     comboDate->tag(i),
                     QString::null);
  emit resample();
}

void KLocaleConfig::changedTime(int i)
{
  changedFlag = TRUE;

  locale->setCountry(QString::null,
                      QString::null,
                      comboDate->tag(i));
  emit resample();
}

void KLocaleConfig::reTranslateLists()
{
  int j;
  QString name;
  for (j = 0; j < comboCountry->count(); j++)
  {
    readLocale(comboCountry->tag(j), name, "l10n/");
    comboCountry->changeLanguage(name, j);
    comboNumber->changeLanguage(name, j);
    comboMoney->changeLanguage(name, j);
    comboDate->changeLanguage(name, j);
  }

  for (j = 0; j < comboLang->count(); j++)
  {
    if (comboLang->tag(j) == "other")
      name = locale->translate("Other");
    else
      readLocale(comboLang->tag(j), name, 0);
    comboLang->changeLanguage(name, j);
  }
}
