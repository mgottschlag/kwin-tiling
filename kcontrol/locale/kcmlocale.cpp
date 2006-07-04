/*
 * locale.cpp
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
 * Copyright (c) 1999-2003 Hans Petter Bieker <bieker@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QLabel>
#include <QLayout>
#include <q3listbox.h>
#include <QPushButton>
#include <QToolTip>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>

#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klanguagebutton.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "kcmlocale.h"
#include "kcmlocale.moc"
#include "toplevel.h"

KLocaleConfig::KLocaleConfig(KLocale *locale,
                             QWidget *parent, const char *name)
  : QWidget (parent),
    m_locale(locale)
{
  QGridLayout *lay = new QGridLayout(this );
  lay->setMargin( KDialog::marginHint() );
  lay->setSpacing( KDialog::spacingHint());

  m_labCountry = new QLabel(this);
  m_labCountry->setObjectName( I18N_NOOP("Country or region:") );
  m_comboCountry = new KLanguageButton( this );
  m_labCountry->setBuddy(m_comboCountry);
  connect( m_comboCountry, SIGNAL(activated(const QString &)),
           this, SLOT(changedCountry(const QString &)) );

  m_labLang = new QLabel(this);
  m_labLang->setObjectName( I18N_NOOP("Languages:") );
  m_labLang->setAlignment( Qt::AlignTop );

  m_languages = new Q3ListBox(this);
  connect(m_languages, SIGNAL(selectionChanged()),
          SLOT(slotCheckButtons()));

  QWidget * vb = new QWidget(this);
  QVBoxLayout * boxlay = new QVBoxLayout(vb);
  boxlay->setMargin(0);
  boxlay->setSpacing(KDialog::spacingHint());
  m_addLanguage = new KLanguageButton(QString(), vb );
  m_addLanguage->setObjectName( I18N_NOOP("Add Language") );
  boxlay->addWidget(m_addLanguage);
  connect(m_addLanguage, SIGNAL(activated(const QString &)),
          SLOT(slotAddLanguage(const QString &)));
  m_removeLanguage = new QPushButton( vb );
  m_removeLanguage->setObjectName( I18N_NOOP("Remove Language") );
  m_upButton = new QPushButton( vb );
  m_upButton->setObjectName( I18N_NOOP("Move Up") );
  m_downButton = new QPushButton( vb );
  m_downButton->setObjectName( I18N_NOOP("Move Down") );
  boxlay->addWidget(m_removeLanguage);
  boxlay->addWidget(m_upButton);
  boxlay->addWidget(m_downButton);
  connect(m_removeLanguage, SIGNAL(clicked()),
          SLOT(slotRemoveLanguage()));
  connect(m_upButton, SIGNAL(clicked()),
          SLOT(slotLanguageUp()));
  connect(m_downButton, SIGNAL(clicked()),
          SLOT(slotLanguageDown()));
  boxlay->insertStretch(-1);

  // #### HPB: This should be implemented for KDE 3
  //  new QLabel(this, I18N_NOOP("Encoding:"));
  //QComboBox * cb = new QComboBox( this );
  //cb->insertStringList( KGlobal::charsets()->descriptiveEncodingNames() );

  lay->addWidget(m_labCountry, 0, 0, 1, 2 );
  lay->addWidget(m_comboCountry, 0, 2);
  lay->addWidget(m_labLang, 1, 0);
  lay->addWidget(m_languages, 1, 1);
  lay->addWidget(vb, 1, 2);

  lay->setRowStretch(2, 5);

  lay->setColumnStretch(1, 1);
  lay->setColumnStretch(2, 1);
}

void KLocaleConfig::slotAddLanguage(const QString & code)
{
  QStringList languageList = m_locale->languageList();

  int pos = m_languages->currentItem();
  if ( pos < 0 )
    pos = 0;

  // If it's already in list, just move it (delete the old, then insert a new)
  int oldPos = languageList.indexOf( code );
  if ( oldPos != -1 )
    languageList.removeAll( languageList.at(oldPos) );

  if ( oldPos != -1 && oldPos < pos )
    --pos;

  languageList.insert( pos, code );

  m_locale->setLanguage( languageList );

  emit localeChanged();
  if ( pos == 0 )
    emit languageChanged();
}

void KLocaleConfig::slotRemoveLanguage()
{
  QStringList languageList = m_locale->languageList();
  int pos = m_languages->currentItem();

  QStringList::iterator it = languageList.begin() + pos;

  if ( it != languageList.end() )
    {
      languageList.erase( it );

      m_locale->setLanguage( languageList );

      emit localeChanged();
      if ( pos == 0 )
        emit languageChanged();
    }
}

void KLocaleConfig::slotLanguageUp()
{
  QStringList languageList = m_locale->languageList();
  int pos = m_languages->currentItem();

  QStringList::Iterator it1 = languageList.begin() + pos - 1;
  QStringList::Iterator it2 = languageList.begin() + pos;

  if ( it1 != languageList.end() && it2 != languageList.end()  )
  {
    QString str = *it1;
    *it1 = *it2;
    *it2 = str;

    m_locale->setLanguage( languageList );

    emit localeChanged();
    if ( pos == 1 ) // at the lang before the top
      emit languageChanged();
  }
}

void KLocaleConfig::slotLanguageDown()
{
  QStringList languageList = m_locale->languageList();
  int pos = m_languages->currentItem();

  QStringList::Iterator it1 = languageList.begin() + pos;
  QStringList::Iterator it2 = languageList.begin() + pos + 1;

  if ( it1 != languageList.end() && it2 != languageList.end()  )
    {
      QString str = *it1;
      *it1 = *it2;
      *it2 = str;

      m_locale->setLanguage( languageList );

      emit localeChanged();
      if ( pos == 0 ) // at the top
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
    QString str = KStandardDirs::locate("locale", QString::fromLatin1("%1/entry.desktop")
                         .arg(*it));
    if (!str.isNull())
      prilang << str;
  }

  // add all languages to the list
  QStringList alllang = KGlobal::dirs()->findAllResources("locale",
                               QString::fromLatin1("*/entry.desktop"),
                               false, true);
  QStringList langlist = prilang;
  if (langlist.count() > 0)
    langlist << QString(); // separator
  langlist += alllang;

// menu_index has to be removed for the country list, so perhaps there is the sae problem for the language list in KDE4 (see also kpersonalizer).
#ifdef __GNUC__
# warning "Check if menu_index is usable for the language list in KDE4"
#endif
  int menu_index = -2;
  QString submenu; // we are working on this menu
  for ( QStringList::ConstIterator it = langlist.begin();
        it != langlist.end(); ++it )
  {
    if ((*it).isNull())
    {
      m_addLanguage->insertSeparator();
      submenu = QString::fromLatin1("other");
      m_addLanguage->insertSubmenu(ki18n("Other").toString(m_locale),
                                   submenu, QString(), -1);
      menu_index = -2; // first entries should _not_ be sorted
      continue;
    }
    KSimpleConfig entry(*it);
    entry.setGroup("KCM Locale");
    QString name = entry.readEntry("Name",
                                   ki18n("without name").toString(m_locale));

    QString tag = *it;
    int index = tag.lastIndexOf('/');
    tag = tag.left(index);
    index = tag.lastIndexOf('/');
    tag = tag.mid(index + 1);
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
                                 sub + QString::fromLatin1("*.desktop"),
                                 false, true );

  for ( QStringList::ConstIterator it = regionlist.begin();
    it != regionlist.end();
    ++it )
  {
    QString tag = *it;
    int index;

    index = tag.lastIndexOf('/');
    if (index != -1)
      tag = tag.mid(index + 1);

    index = tag.lastIndexOf('.');
    if (index != -1)
      tag.truncate(index);

    KSimpleConfig entry(*it);
    entry.setGroup("KCM Locale");
    QString name = entry.readEntry("Name",
                                   ki18n("without name").toString(m_locale));

    QString map( KStandardDirs::locate( "locale",
                          QString::fromLatin1( "l10n/%1.png" )
                          .arg(tag) ) );
    //kDebug() << "REGION: " << (*it) << " Tag: " << tag << " Name: " << name << " Map: " << map << endl;
    QIcon icon;
    if ( !map.isNull() )
      icon = KGlobal::iconLoader()->loadIconSet(map, K3Icon::Small);
    m_comboCountry->insertSubmenu( icon, name, tag );
  }

  // add all languages to the list
  QStringList countrylist = KGlobal::dirs()->findAllResources
    ("locale", sub + QString::fromLatin1("*/entry.desktop"), false, true);

  for ( QStringList::ConstIterator it = countrylist.begin();
        it != countrylist.end(); ++it )
  {
    KSimpleConfig entry(*it);
    entry.setGroup("KCM Locale");
    QString name = entry.readEntry("Name",
                                   ki18n("without name").toString(m_locale));
    QString submenu = entry.readEntry("Region");

    QString tag = *it;
    int index = tag.lastIndexOf('/');
    tag.truncate(index);
    index = tag.lastIndexOf('/');
    tag = tag.mid(index + 1);

    QString flag( KStandardDirs::locate( "locale",
                          QString::fromLatin1( "l10n/%1/flag.png" )
                          .arg(tag) ) );
    //kDebug() << "COUNTRY: " << (*it) << " Tag: " << tag << " Submenu: " << submenu << " Flag: " << flag << endl;
    QIcon icon( KGlobal::iconLoader()->loadIconSet(flag, K3Icon::Small) );

    m_comboCountry->insertItem( icon, name, tag, submenu );
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

  KSimpleConfig entry(KStandardDirs::locate("locale", filepath));
  entry.setGroup("KCM Locale");
  name = entry.readEntry("Name");

  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfig::save()
{
  KConfigBase *config = KGlobal::config();

  config->setGroup("Locale");

  config->writeEntry("Country", m_locale->country(), KConfigBase::Persistent|KConfigBase::Global);
  if ( m_locale->languageList().isEmpty() )
    config->writeEntry("Language", QString::fromLatin1(""), KConfigBase::Persistent|KConfigBase::Global);
  else
    config->writeEntry("Language",
                       m_locale->languageList(), ':', KConfigBase::Persistent|KConfigBase::Global);

  config->sync();
}

void KLocaleConfig::slotCheckButtons()
{
  m_removeLanguage->setEnabled( m_languages->currentItem() != -1 );
  m_upButton->setEnabled( m_languages->currentItem() > 0 );
  m_downButton->setEnabled( m_languages->currentItem() != -1 &&
                            m_languages->currentItem() < (signed)(m_languages->count() - 1) );
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
    readLocale(*it, name, QString());

    m_languages->insertItem(name);
  }
  slotCheckButtons();

  m_comboCountry->setCurrentItem( m_locale->country() );
}

void KLocaleConfig::slotTranslate()
{
  kDebug() << "slotTranslate()" << endl;

  m_comboCountry->setToolTip( ki18n
        ( "This is where you live. KDE will use the defaults for "
          "this country or region.").toString(m_locale) );
  m_addLanguage->setToolTip( ki18n
        ( "This will add a language to the list. If the language is already "
          "in the list, the old one will be moved instead." ).toString(m_locale) );

  m_removeLanguage->setToolTip( ki18n
        ( "This will remove the highlighted language from the list." ).toString(m_locale) );

  m_languages->setToolTip( ki18n
        ( "KDE programs will be displayed in the first available language in "
          "this list.\nIf none of the languages are available, US English "
          "will be used.").toString(m_locale) );

  QString str;

  str = ki18n
    ( "Here you can choose your country or region. The settings "
      "for languages, numbers etc. will automatically switch to the "
      "corresponding values." ).toString(m_locale);
  m_labCountry->setWhatsThis( str );
  m_comboCountry->setWhatsThis( str );

  str = ki18n
    ( "Here you can choose the languages that will be used by KDE. If the "
      "first language in the list is not available, the second will be used, "
      "etc. If only US English is available, no translations "
      "have been installed. You can get translation packages for many "
      "languages from the place you got KDE from.<p>"
      "Note that some applications may not be translated to your languages; "
      "in this case, they will automatically fall back to US English." ).toString(m_locale);
  m_labLang->setWhatsThis( str );
  m_languages->setWhatsThis( str );
  m_addLanguage->setWhatsThis( str );
  m_removeLanguage->setWhatsThis( str );
}

QStringList KLocaleConfig::languageList() const
{
  QString fileName = KStandardDirs::locate("locale",
                            QString::fromLatin1("l10n/%1/entry.desktop")
                            .arg(m_locale->country()));

  KSimpleConfig entry(fileName);
  entry.setGroup("KCM Locale");

  return entry.readEntry("Languages", QStringList());
}

void KLocaleConfig::changedCountry(const QString & code)
{
  m_locale->setCountry(code);

  // change to the preferred languages in that country, installed only
  QStringList languages = languageList();
  QStringList newLanguageList;
  for ( QStringList::Iterator it = languages.begin();
        it != languages.end();
        ++it )
  {
    QString name;
    readLocale(*it, name, QString());

    if (!name.isEmpty())
      newLanguageList += *it;
  }
  m_locale->setLanguage( newLanguageList );

  emit localeChanged();
  emit languageChanged();
}
