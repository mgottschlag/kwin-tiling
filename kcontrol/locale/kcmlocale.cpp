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

#include <qhbox.h>

#include <kdialog.h>
#include <kcharsets.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qlistbox.h>

#include "klanguagebutton.h"
#include "kmenubutton.h"
#include "klocalesample.h"
#include "kcmlocale.h"
#include "kcmlocale.moc"
#include "toplevel.h"

KLocaleConfig::KLocaleConfig(KLocale *locale,
			     QWidget *parent, const char *name)
  : QWidget (parent, name),
    m_locale(locale)
{
  QGridLayout *lay = new QGridLayout(this, 3, 2,
				     KDialog::marginHint(),
				     KDialog::spacingHint());
  lay->setAutoAdd(TRUE);
  
  m_labCountry = new QLabel(this, I18N_NOOP("Country:"));
  m_comboCountry = new KLanguageButton( this );
  m_comboCountry->setFixedHeight(m_comboCountry->sizeHint().height());
  m_labCountry->setBuddy(m_comboCountry);
  connect( m_comboCountry, SIGNAL(activated(int)),
	   this, SLOT(changedCountry(int)) );

  m_labLang = new QLabel(this, I18N_NOOP("Languages:"));
  m_labLang->setAlignment( AlignTop );
  QHBox *hb = new QHBox(this);
  hb->setSpacing(KDialog::spacingHint());
  m_languages = new QListBox(hb);
  connect(m_languages, SIGNAL(selectionChanged()),
	  SLOT(slotCheckButtons()));

  QWidget * vb = new QWidget(hb);
  QVBoxLayout * boxlay = new QVBoxLayout(vb, 0, KDialog::spacingHint());
  m_addLanguage = new KMenuButton(vb, I18N_NOOP("Add Language"));
  boxlay->add(m_addLanguage);
  connect(m_addLanguage, SIGNAL(activated(int)),
	  SLOT(slotAddLanguage(int)));
  m_removeLanguage = new QPushButton(vb, I18N_NOOP("Remove Language"));
  boxlay->add(m_removeLanguage);
  connect(m_removeLanguage, SIGNAL(clicked()),
	  SLOT(slotRemoveLanguage()));
  boxlay->insertStretch(-1);

  // #### HPB: This should be implemented for KDE 3
  //  new QLabel(this, I18N_NOOP("Encoding:"));
  //QComboBox * cb = new QComboBox( this );
  //cb->insertStringList( KGlobal::charsets()->descriptiveEncodingNames() );

  lay->setRowStretch(2, 5);

  lay->setColStretch(1, 1);
}

KLocaleConfig::~KLocaleConfig()
{
}

void KLocaleConfig::slotAddLanguage(int i)
{
  QString code = m_addLanguage->tag(i);
  QStringList languageList = m_locale->languageList();

  int pos = m_languages->currentItem();
  if ( pos < 0 ) pos = 0;

  // If it's already in list, just move it (delete the old, then insert a new)
  int oldPos = languageList.findIndex( code );
  if ( oldPos != -1 )
    languageList.remove( languageList.at(oldPos) );

  if ( oldPos != -1 && oldPos < pos )
    --pos;

  QStringList::Iterator it = languageList.at( pos );
 
  languageList.insert( it, code );

  m_locale->setLanguage( languageList );

  emit localeChanged();
  if ( pos == 0 )
    emit languageChanged();
}

void KLocaleConfig::slotRemoveLanguage()
{
  QStringList languageList = m_locale->languageList();
  int pos = m_languages->currentItem();

  QStringList::Iterator it = languageList.at( pos );

  if ( it != languageList.end() )
    {
      languageList.remove( it );

      m_locale->setLanguage( languageList );

      emit localeChanged();
      if ( pos == 0 )
	emit languageChanged();
    }
}


void KLocaleConfig::loadLanguageList()
{
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = m_locale;

  // clear the list
  m_addLanguage->clear();

  QStringList first = languageList();

  QStringList prilang;
  // add the primary languages for the country to the list
  for ( QStringList::ConstIterator it = first.begin();
	it != first.end();
	++it )
    {
      QString str = locate("locale", QString::fromLatin1("%1/entry.desktop")
			   .arg(*it));
      if (!str.isNull())
	prilang << str;
    }

  // add all languages to the list
  QStringList alllang = KGlobal::dirs()->findAllResources("locale",
                               QString::fromLatin1("*/entry.desktop"));
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
	  m_addLanguage->insertSeparator();
	  submenu = QString::fromLatin1("other");
	  m_addLanguage->insertSubmenu(m_locale->translate("Other"),
					 submenu, QString::null, -1);
          menu_index = -2; // first entries should _not_ be sorted
          continue;
        }
      KSimpleConfig entry(*it);
      entry.setGroup("KCM Locale");
      QString name = entry.readEntry("Name",
				     m_locale->translate("without name"));
      
      QString tag = *it;
      int index = tag.findRev('/');
      tag = tag.left(index);
      index = tag.findRev('/');
      tag = tag.mid(index+1);
      m_addLanguage->insertItem(name, tag, submenu, menu_index);
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
    entry.setGroup("KCM Locale");
    QString name = entry.readEntry("Name",
				   m_locale->translate("without name"));

    m_comboCountry->insertSubmenu( name, tag, sub, -2 );
  }

  // add all languages to the list
  QStringList countrylist = KGlobal::dirs()->findAllResources
    ("locale", sub + QString::fromLatin1("*/entry.desktop"));

  for ( QStringList::ConstIterator it = countrylist.begin();
	it != countrylist.end(); ++it )
    {
      KSimpleConfig entry(*it);
      entry.setGroup("KCM Locale");
      QString name = entry.readEntry("Name",
				     m_locale->translate("without name"));
      QString submenu = entry.readEntry("Region");
      
      QString tag = *it;
      int index = tag.findRev('/');
      tag.truncate(index);
      index = tag.findRev('/');
      tag = tag.mid(index+1);
      int menu_index = submenu.isEmpty() ? -1 : -2;

      QPixmap flag( locate( "locale",
			    QString::fromLatin1( "l10n/%1/flag.png" )
			    .arg(tag) ) );
      QIconSet icon( flag );
      m_comboCountry->insertItem( icon, name, tag, submenu, menu_index );
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
  QString filepath = QString::fromLatin1("%1%2/entry.desktop")
    .arg(sub)
    .arg(path);

  KSimpleConfig entry(locate("locale", filepath));
  entry.setGroup("KCM Locale");
  name = entry.readEntry("Name");

  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfig::save()
{
  KConfigBase *config = KGlobal::config();

  config->setGroup("Locale");

  config->writeEntry("Country", m_locale->country(), true, true);
  if ( m_locale->languageList().isEmpty() )
    config->writeEntry("Language", QString::fromLatin1(""), true, true);
  else
    config->writeEntry("Language",
		       m_locale->languageList(), ':', true, true);

  config->sync();
}

void KLocaleConfig::slotCheckButtons()
{
  m_removeLanguage->setEnabled( m_languages->currentItem() != -1 );
}

void KLocaleConfig::slotLocaleChanged()
{
  loadLanguageList();
  loadCountryList();

  // update language widget
  m_languages->clear();
  QStringList languageList = m_locale->languageList();
  for ( QStringList::Iterator it = languageList.begin();
	it != languageList.end();
	++it )
    {
      QString name;
      readLocale(*it, name, QString::null);

      m_languages->insertItem(name);
    }
  slotCheckButtons();

  m_comboCountry->setCurrentItem( m_locale->country() );
}

void KLocaleConfig::slotTranslate()
{
  kdDebug() << "slotTranslate()" << endl;

  QToolTip::add(m_comboCountry, m_locale->translate
        ( "This is where you live. KDE will use the defaults for "
          "this country.") );
  QToolTip::add(m_addLanguage, m_locale->translate
	( "This will add a language to the list. If the language is already "
	  "in the list, the old one will be moved instead." ) );

  QToolTip::add(m_removeLanguage, m_locale->translate
	( "This will remove the highlighted language from the list." ) );

  QToolTip::add(m_languages, m_locale->translate
        ( "KDE program will be displayed in the first available language in "
	  "this list. If none of the languages are available, US English "
	  "will be used.") );

  QString str;

  str = m_locale->translate
    ( "Here you can choose your country. The settings "
      "for languages, numbers etc. will automatically switch to the "
      "corresponding values." );
  QWhatsThis::add( m_labCountry, str );
  QWhatsThis::add( m_comboCountry, str );

  str = m_locale->translate
    ( "Here you can choose the languages that will be used by KDE. If the "
      "first language in the list is not available, the second will be used "
      "etc. If only US English is available, no translations "
      "have been installed. You can get translations packages for many "
      "languages from the place you got KDE from.<p>"
      "Note that some applications may not be translated to your languages; "
      "in this case, they will automatically fall back to English US." );
  QWhatsThis::add( m_labLang, str );
  QWhatsThis::add( m_languages, str );
  QWhatsThis::add( m_addLanguage, str );
  QWhatsThis::add( m_removeLanguage, str );
}

QStringList KLocaleConfig::languageList() const
{
  QString fileName = locate("locale",
			    QString::fromLatin1("l10n/%1/entry.desktop")
			    .arg(m_locale->country()));

  KSimpleConfig entry(fileName);
  entry.setGroup("KCM Locale");

  return entry.readListEntry("Languages");
}

void KLocaleConfig::changedCountry(int i)
{
  m_locale->setCountry(m_comboCountry->tag(i));

  // change to the prefered languages in that country, installed only
  QStringList languages = languageList();
  QStringList newLanguageList;
  for ( QStringList::Iterator it = languages.begin();
	it != languages.end();
	++it )
    {
      QString name;
      readLocale(*it, name, QString::null);

      if (!name.isEmpty())
	newLanguageList += *it;
    }
  m_locale->setLanguage( newLanguageList );

  emit localeChanged();
  emit languageChanged();
}
