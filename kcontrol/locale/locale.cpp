/*
 * locale.cpp
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
 * Copyright (c) 1999-2000 Hans Petter Bieker <bieker@kde.org>
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
#include <qtooltip.h>
#include <qiconset.h>
#include <qwhatsthis.h>

#include <kconfig.h>
#include <kglobal.h>

#include <kstddirs.h>
#include <ksimpleconfig.h>
#include <kcharsets.h>

#include "klocaleadv.h"
#include "klangcombo.h"
#include "klocalesample.h"
#include "locale.h"
#include "locale.moc"
#include "toplevel.h"

extern KLocaleAdvanced *locale;

KLocaleConfig::KLocaleConfig(QWidget *parent, const char *name)
  : QWidget (parent, name)
{
    QString wtstr;
    QGridLayout *tl1 = new QGridLayout(this, 1, 1, 10, 5);
    tl1->setColStretch( 2, 1);

    QLabel *label = new QLabel(this, I18N_NOOP("&Country"));
    comboCountry = new KLanguageCombo(this);
    comboCountry->setFixedHeight(comboCountry->sizeHint().height());
    label->setBuddy(comboCountry);
    connect( comboCountry, SIGNAL(activated(int)),
	     this, SLOT(changedCountry(int)) );
    tl1->addWidget(label, 1, 1);
    tl1->addWidget(comboCountry, 1, 2);
    wtstr = locale->translate("Here you can choose your country. The settings"
      " for language, numbers etc. will automatically switch to the"
      " corresponding values.");
    QWhatsThis::add( label, wtstr );
    QWhatsThis::add( comboCountry, wtstr );

    label = new QLabel(this, I18N_NOOP("&Language"));
    comboLang = new KLanguageCombo(this);
    comboLang->setFixedHeight(comboLang->sizeHint().height());
    label->setBuddy(comboLang);
    connect( comboLang, SIGNAL(activated(int)),
	     this, SLOT(changedLanguage(int)) );
    tl1->addWidget(label, 2, 1);
    tl1->addWidget(comboLang, 2, 2);
    wtstr = locale->translate("Here you can choose the language that will be used"
      " by KDE. If only US English is available, no translations have been"
      " installed. You can get translations packages for many languages from"
      " the place you got KDE from. <p> Note that some applications may not be translated to"
      " your language; in this case, they will automatically fall back"
      " to the default language, i.e. US English.");
    QWhatsThis::add( label, wtstr );
    QWhatsThis::add( comboLang, wtstr );

    label = new QLabel(this, I18N_NOOP("&Numbers"));
    comboNumber = new KLanguageCombo(this);
    comboNumber->setFixedHeight(comboNumber->sizeHint().height());
    label->setBuddy(comboNumber);
    connect( comboNumber, SIGNAL(activated(int)),
	     this, SLOT(changedNumber(int)) );
    tl1->addWidget(label, 3, 1);
    tl1->addWidget(comboNumber, 3, 2);
    wtstr = locale->translate( "Here you can choose a national setting to display"
      " numbers. You can also customize this using the 'Numbers' tab." );
    QWhatsThis::add( label, wtstr );
    QWhatsThis::add( comboNumber, wtstr );

    label = new QLabel(this, I18N_NOOP("&Money"));
    comboMoney = new KLanguageCombo(this);
    comboMoney->setFixedHeight(comboMoney->sizeHint().height());
    label->setBuddy(comboMoney);
    connect( comboMoney, SIGNAL(activated(int)),
	     this, SLOT(changedMoney(int)) );
    tl1->addWidget(label, 4, 1);
    tl1->addWidget(comboMoney, 4, 2);
    wtstr = locale->translate( "Here you can choose a national setting to display"
      " monetary values. You can also customize this using the 'Money' tab." );
    QWhatsThis::add( label, wtstr );
    QWhatsThis::add( comboMoney, wtstr );

    label = new QLabel(this, I18N_NOOP("&Date and time"));
    comboDate = new KLanguageCombo(this);
    comboDate->setFixedHeight(comboDate->sizeHint().height());
    label->setBuddy(comboDate);
    connect( comboDate, SIGNAL(activated(int)),
	     this, SLOT(changedTime(int)) );
    tl1->addWidget(label, 5, 1);
    tl1->addWidget(comboDate, 5, 2);
    wtstr = locale->translate( "Here you can choose a national setting to display"
      " date and time. You can also customize this using the 'Time & dates' tab." );
    QWhatsThis::add( label, wtstr );
    QWhatsThis::add( comboDate, wtstr );

    label = new QLabel(this, I18N_NOOP("C&harset"));
    comboChset = new KLanguageCombo(this);
    comboChset->setFixedHeight(comboChset->sizeHint().height());
    label->setBuddy(comboChset);
    connect( comboChset, SIGNAL(activated(int)),
	     this, SLOT(changedCharset(int)) );
    tl1->addWidget(label, 6, 1);
    tl1->addWidget(comboChset, 6, 2);
    wtstr = locale->translate( "Here you can choose the charset KDE uses to display"
      " text. ISO 8859-1 is default and should work for you if you use some"
      " Western European language. If not, you may have to choose a different"
      " charset." );
    QWhatsThis::add( label, wtstr );
    QWhatsThis::add( comboChset, wtstr );

    QStringList list = KGlobal::charsets()->availableCharsetNames();
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
       comboChset->insertItem(*it, *it);

    tl1->setRowStretch(7, 1);
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
        QString str = locate("locale", sub + *it + QString::fromLatin1("/entry.desktop"));
        if (!str.isNull())
          prilang << str;
    }

  // add all languages to the list
  QStringList alllang = KGlobal::dirs()->findAllResources("locale",
							   sub + QString::fromLatin1("*/entry.desktop"));
  alllang.sort();
  QStringList langlist = prilang;
  if (langlist.count() > 0)
    langlist << QString::null; // separator
  langlist += alllang;

  QString submenu; // we are working on this menu
  for ( QStringList::ConstIterator it = langlist.begin();
	it != langlist.end(); ++it )
    {
        if ((*it).isNull())
        {
	  combo->insertSeparator();
	  submenu = QString::fromLatin1("other");
	  combo->insertSubmenu(locale->translate("Other"), submenu);
          continue;
        }
	KSimpleConfig entry(*it);
	entry.setGroup(QString::fromLatin1("KCM Locale"));
	name = entry.readEntry(QString::fromLatin1("Name"), locale->translate("without name"));
	
	QString path = *it;
	int index = path.findRev('/');
	path = path.left(index);
	index = path.findRev('/');
	path = path.mid(index+1);
	combo->insertLanguage(path, name, sub, submenu);
    }
  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfig::load()
{
  KConfig *config = KGlobal::config();
  config->setGroup(QString::fromLatin1("Locale"));

  QString country = config->readEntry(QString::fromLatin1("Country"));

  QString lang = config->readEntry(QString::fromLatin1("Language"));
  lang = lang.left(lang.find(':')); // only use  the first lang
  locale->setLanguage(lang);

  QString number = config->readEntry(QString::fromLatin1("Numeric"));
  QString money = config->readEntry(QString::fromLatin1("Monetary"));
  QString time = config->readEntry(QString::fromLatin1("Time"));
  locale->setCountry(number, money, time);

  QString charset = config->readEntry(QString::fromLatin1("Charset"), QString::fromLatin1("iso-8859-1"));
  emit chsetChanged();

  KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/") + country + QString::fromLatin1("/entry.desktop")), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));
  QStringList langs = ent.readListEntry(QString::fromLatin1("Languages"));
  if (langs.isEmpty()) langs = QString::fromLatin1("C");

  // load lists into widgets
  loadLocaleList(comboLang, QString::null, langs);
  loadLocaleList(comboCountry, QString::fromLatin1("l10n/"), QStringList());
  loadLocaleList(comboNumber, QString::fromLatin1("l10n/"), QStringList());
  loadLocaleList(comboMoney, QString::fromLatin1("l10n/"), QStringList());
  loadLocaleList(comboDate, QString::fromLatin1("l10n/"), QStringList());  

  // update widgets
  comboLang->setCurrentItem(locale->language());
  comboNumber->setCurrentItem(number);
  comboMoney->setCurrentItem(money);
  comboDate->setCurrentItem(time);
  comboCountry->setCurrentItem(country);
  comboChset->setCurrentItem(charset);
}

void KLocaleConfig::readLocale(const QString &path, QString &name, const QString &sub) const
{
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = locale;

  // read the name
  KSimpleConfig entry(locate("locale", sub + path + QString::fromLatin1("/entry.desktop")));
  entry.setGroup(QString::fromLatin1("KCM Locale"));
  name = entry.readEntry(QString::fromLatin1("Name"), locale->translate("without name"));

  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfig::save()
{
  KConfigBase *config = KGlobal::config();

  config->setGroup(QString::fromLatin1("Locale"));

  config->writeEntry(QString::fromLatin1("Country"), comboCountry->currentTag(), true, true);
  config->writeEntry(QString::fromLatin1("Language"), comboLang->currentTag(), true, true);
  config->writeEntry(QString::fromLatin1("Numeric"), comboNumber->currentTag(), true, true);
  config->writeEntry(QString::fromLatin1("Monetary"), comboMoney->currentTag(), true, true);
  config->writeEntry(QString::fromLatin1("Time"), comboDate->currentTag(), true, true);
  config->writeEntry(QString::fromLatin1("Charset"), comboChset->currentTag(), true, true);

  config->sync();
}

void KLocaleConfig::defaults()
{
  QString C = QString::fromLatin1("C");
  locale->setLanguage(C);
  locale->setCountry(C);

  reTranslateLists();
  loadLocaleList(comboLang, QString::null, QStringList());

  comboCountry->setCurrentItem(C);
  comboLang->setCurrentItem(C);
  comboNumber->setCurrentItem(C);
  comboMoney->setCurrentItem(C);
  comboDate->setCurrentItem(C);
  comboChset->setCurrentItem(QString::fromLatin1("iso-8859-1"));

  emit resample();
  emit countryChanged();
}

QString KLocaleConfig::quickHelp()
{
  return locale->translate("<h1>Locale</h1> Here you can select from several predefined"
    " national settings, i.e. your country, the language that will be used by the"
    " KDE desktop, the way numbers and dates are displayed etc. In most cases it will be"
    " sufficient to choose the country you live in. For instance KDE"
    " will automatically choose \"German\" as language if you choose"
    " \"Germany\" from the list. It will also change the time format"
    " to use 24 hours and and use comma as decimal separator.<p>"
    " Please note that you can still customize all this to suit your taste"
    " using the other tabs. However, the national settings for your country"
    " should be a good start. <p> If your country is not listed here, you"
    " have to adjust all settings to your needs manually.");
}

void KLocaleConfig::changedCountry(int i)
{
  QString country = comboCountry->tag(i);

  KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/") + country + QString::fromLatin1("/entry.desktop")), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));
  QStringList langs = ent.readListEntry(QString::fromLatin1("Languages"));

  QString lang = QString::fromLatin1("C");
  // use the first INSTALLED langauge in the list, or default to C
  for ( QStringList::Iterator it = langs.begin(); it != langs.end(); ++it )
    if (comboLang->containsTag(*it))
      {
	lang = *it;
	break;
      }

  locale->setLanguage(lang);
  locale->setCountry(country);

  reTranslateLists();
  loadLocaleList(comboLang, QString::null, langs);

  comboLang->setCurrentItem(lang);
  comboNumber->setCurrentItem(country);
  comboMoney->setCurrentItem(country);
  comboDate->setCurrentItem(country);

  emit countryChanged();
  emit resample();
}

void KLocaleConfig::changedLanguage(int i)
{
  locale->setLanguage(comboLang->tag(i));

  reTranslateLists();

  emit resample();
}

void KLocaleConfig::changedNumber(int i)
{
  locale->setCountry(comboNumber->tag(i),
                     QString::null,
                     QString::null);

  emit numberChanged();
  emit resample();
}

void KLocaleConfig::changedMoney(int i)
{
  locale->setCountry(QString::null,
                     comboMoney->tag(i),
                     QString::null);

  emit moneyChanged();
  emit resample();
}

void KLocaleConfig::changedTime(int i)
{
  locale->setCountry(QString::null,
                      QString::null,
                      comboDate->tag(i));

  emit timeChanged();
  emit resample();
}

void KLocaleConfig::changedCharset(int)
{
  locale->setChset(comboChset->currentTag());

  emit chsetChanged();
}

void KLocaleConfig::reTranslateLists()
{
  int j;
  QString name;
  for (j = 0; j < comboCountry->count(); j++)
  {
    readLocale(comboCountry->tag(j), name, QString::fromLatin1("l10n/"));
    comboCountry->changeLanguage(name, j);
    comboNumber->changeLanguage(name, j);
    comboMoney->changeLanguage(name, j);
    comboDate->changeLanguage(name, j);
  }

  for (j = 0; j < comboLang->count(); j++)
  {
    if (comboLang->tag(j) == QString::fromLatin1("other"))
      comboLang->changeItem(locale->translate("Other"), j);
    else
    {
      readLocale(comboLang->tag(j), name, QString::null);
      comboLang->changeLanguage(name, j);
    }
  }
}

void KLocaleConfig::reTranslate()
{
    QToolTip::add(comboCountry, locale->translate("This is were you live. KDE will use the defaults for this country."));
    QToolTip::add(comboLang, locale->translate("All KDE programs will be displayed in this language (if available)."));
    QToolTip::add(comboNumber, locale->translate("The rules of this country will be used to localize numbers."));
    QToolTip::add(comboMoney, locale->translate("The rules of this country will be used to localize money."));
    QToolTip::add(comboDate, locale->translate("The rules of this country will be used to display time and dates."));
    QToolTip::add(comboChset, locale->translate("The prefered charset for fonts."));
}








