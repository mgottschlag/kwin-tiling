/*
 * locale.cpp
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
 * Copyright (c) 1999-2001 Hans Petter Bieker <bieker@kde.org>
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
#include <qcombobox.h>

#include <kcharsets.h>
#include <kapp.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>
#include <kdialog.h>
#include <klocale.h>

#include "klanguagebutton.h"
#include "klocalesample.h"
#include "locale.h"
#include "locale.moc"
#include "toplevel.h"

KLocaleConfig::KLocaleConfig(KLocale *locale,
			     QWidget *parent, const char *name)
  : QWidget (parent, name),
    m_locale(locale)
{
  QGridLayout *lay = new QGridLayout(this, 7, 2,
				     KDialog::marginHint(),
				     KDialog::spacingHint());
  lay->setAutoAdd(TRUE);
  
  m_labCountry = new QLabel(this, I18N_NOOP("Country:"));
  m_comboCountry = new KLanguageButton( this );
  m_comboCountry->setFixedHeight(m_comboCountry->sizeHint().height());
  m_labCountry->setBuddy(m_comboCountry);
  connect( m_comboCountry, SIGNAL(activated(int)),
	   this, SLOT(changedCountry(int)) );

  m_labLang = new QLabel(this, I18N_NOOP("Language:"));
  m_comboLanguage = new KLanguageButton( this );
  m_comboLanguage->setFixedHeight(m_comboLanguage->sizeHint().height());
  connect( m_comboLanguage, SIGNAL(activated(int)),
	   this, SLOT(changedLanguage(int)) );

  new QLabel(this, I18N_NOOP("Encoding:"));
  QComboBox * cb = new QComboBox( this );
  cb->insertStringList( KGlobal::charsets()->descriptiveEncodingNames() );

  lay->setColStretch(1, 1);
}

KLocaleConfig::~KLocaleConfig()
{
}

void KLocaleConfig::loadLanguageList()
{
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = m_locale;

  // clear the list
  m_comboLanguage->clear();

  QStringList first = languageList();

  QStringList prilang;
  // add the primary languages for the country to the list
  for ( QStringList::ConstIterator it = first.begin(); it != first.end(); ++it )
    {
      QString str = locate("locale", *it + QString::fromLatin1("/entry.desktop"));
      if (!str.isNull())
	prilang << str;
    }

  // add all languages to the list
  QStringList alllang = KGlobal::dirs()->findAllResources("locale",
                               QString::fromLatin1("*/entry.desktop"));
  alllang.sort();
  QStringList langlist = prilang;
  if (langlist.count() > 0)
    langlist << QString::null; // separator
  langlist += alllang;

  int menu_index = -2;
  QString submenu; // we are working on this menu
  for ( QStringList::ConstIterator it = langlist.begin();
	it != langlist.end(); ++it )
    {
      if ((*it).isNull())
        {
	  m_comboLanguage->insertSeparator();
	  submenu = QString::fromLatin1("other");
	  m_comboLanguage->insertSubmenu(m_locale->translate("Other"),
					 submenu, QString::null, -2);
          menu_index = -1; // first entries should _not_ be sorted
          continue;
        }
      KSimpleConfig entry(*it);
      entry.setGroup(QString::fromLatin1("KCM Locale"));
      QString name = entry.readEntry(QString::fromLatin1("Name"),
				     m_locale->translate("without name"));
      
      QString path = *it;
      int index = path.findRev('/');
      path = path.left(index);
      index = path.findRev('/');
      path = path.mid(index+1);
      m_comboLanguage->insertLanguage(path, name,
				      QString::null, submenu, menu_index);
    }

  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfig::loadCountryList()
{
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = m_locale;

  QString sub = QString::fromLatin1("l10n/");

  // clear the list
  m_comboCountry->clear();

  QStringList regionlist = KGlobal::dirs()->findAllResources("locale",
                                 sub + QString::fromLatin1("*.desktop"));
  regionlist.sort();

  for ( QStringList::ConstIterator it = regionlist.begin();
    it != regionlist.end();
    ++it )
  {
    QString tag = *it;
    int index;

    index = tag.findRev('/');
    if (index != -1) tag = tag.mid(index + 1);

    index = tag.findRev('.');
    if (index != -1) tag.truncate(index);

    KSimpleConfig entry(*it);
    entry.setGroup(QString::fromLatin1("KCM Locale"));
    QString name = entry.readEntry(QString::fromLatin1("Name"),
				   m_locale->translate("without name"));

    m_comboCountry->insertSubmenu( name, '-' + tag, sub );
  }

  // add all languages to the list
  QStringList countrylist = KGlobal::dirs()->findAllResources
    ("locale", sub + QString::fromLatin1("*/entry.desktop"));
  countrylist.sort();

  for ( QStringList::ConstIterator it = countrylist.begin();
	it != countrylist.end(); ++it )
    {
      KSimpleConfig entry(*it);
      entry.setGroup(QString::fromLatin1("KCM Locale"));
      QString name = entry.readEntry(QString::fromLatin1("Name"),
				     m_locale->translate("without name"));
      QString submenu = '-' + entry.readEntry(QString::fromLatin1("Region"));
      
      QString tag = *it;
      int index = tag.findRev('/');
      tag.truncate(index);
      index = tag.findRev('/');
      tag = tag.mid(index+1);
      int menu_index = m_comboCountry->containsTag(tag) ? -1 : -2;
      m_comboCountry->insertLanguage(tag, name, sub, submenu, menu_index);
    }

  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfig::readLocale(const QString &path, QString &name,
			       const QString &sub) const
{
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = m_locale;

  // read the name
  QString filepath = sub;
  if (path.at(0) == '-')
    filepath += path.mid(1) + QString::fromLatin1(".desktop");
  else
    filepath += path + QString::fromLatin1("/entry.desktop");

  KSimpleConfig entry(locate("locale", filepath));
  entry.setGroup(QString::fromLatin1("KCM Locale"));
  name = entry.readEntry(QString::fromLatin1("Name"),
			 m_locale->translate("without name"));

  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfig::save()
{
  KConfigBase *config = KGlobal::config();

  config->setGroup(QString::fromLatin1("Locale"));

  config->writeEntry(QString::fromLatin1("Country"), m_locale->country());
  config->writeEntry(QString::fromLatin1("Language"), m_locale->language());

  config->sync();
}

void KLocaleConfig::slotLocaleChanged()
{
  loadLanguageList();
  loadCountryList();

  m_comboLanguage->setCurrentItem( m_locale->language() );
  m_comboCountry->setCurrentItem( m_locale->country() );
}

void KLocaleConfig::slotTranslate()
{
  QToolTip::add(m_comboCountry, m_locale->translate
        ( "This is were you live. KDE will use the defaults for "
          "this country.") );
  QToolTip::add(m_comboLanguage, m_locale->translate
        ( "All KDE programs will be displayed in this language (if "
          "available).") );

  QString str;

  str = m_locale->translate
    ( "Here you can choose your country. The settings "
      "for language, numbers etc. will automatically switch to the "
      "corresponding values." );
  QWhatsThis::add( m_labCountry, str );
  QWhatsThis::add( m_comboCountry, str );

  str = m_locale->translate
    ( "Here you can choose the language that will be used "
      "by KDE. If only US English is available, no translations have been "
      "installed. You can get translations packages for many languages from "
      "the place you got KDE from. <p> Note that some applications may not "
      "be translated to your language; in this case, they will automatically "
      "fall back to the default language, i.e. US English." );
  QWhatsThis::add( m_labLang, str );
  QWhatsThis::add( m_comboLanguage, str );
}

QStringList KLocaleConfig::languageList() const
{
  QString fileName = locate("locale",
			    QString::fromLatin1("l10n/%1/entry.desktop")
			    .arg(m_locale->country()));

  kdDebug() << "fileName: " << fileName << endl;
  
  KSimpleConfig entry(fileName);
  entry.setGroup(QString::fromLatin1("KCM Locale"));

  return entry.readListEntry(QString::fromLatin1("Languages"));
}

void KLocaleConfig::changedCountry(int i)
{
  m_locale->setCountry(m_comboCountry->tag(i));

  // change to the first language in the list
  QStringList langList = languageList();
  langList += QString::fromLatin1("C"); // always exists
  for ( QStringList::Iterator it = langList.begin();
	it != langList.end();
	++it)
    {
      if ( m_locale->setLanguage( *it ) )
	break;
    }

  emit languageChanged();
  emit localeChanged();
}

void KLocaleConfig::changedLanguage(int i)
{
  m_locale->setLanguage(m_comboLanguage->tag(i));

  emit languageChanged();
  emit localeChanged();
}
