/*
  toplevel.cpp - A KControl Application

  Copyright 1998 Matthias Hoelzer
  Copyright 1999-2000 Hans Petter Bieker <bieker@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  */

#include <qlabel.h>
#include <qvgroupbox.h>
#include <qobjectlist.h>
#include <qtabwidget.h>
#include <qevent.h>
#include <qwidgetintdict.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kconfig.h>
#include <kcmodule.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kprocess.h>

#include <kdebug.h>

#include <stdio.h>

#include "locale.h"
#include "localenum.h"
#include "localemon.h"
#include "localetime.h"
#include "klocalesample.h"
#include "toplevel.h"
#include "toplevel.moc"

KLocaleApplication::KLocaleApplication(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  m_locale = new KLocale(QString::fromLatin1("kcmlocale"), false);
  QVBoxLayout *l = new QVBoxLayout(this, 5);
  l->setAutoAdd(TRUE);

  m_tab = new QTabWidget(this);

  m_localemain = new KLocaleConfig(m_locale, this);
  m_tab->addTab( m_localemain, QString::null);
  m_localenum = new KLocaleConfigNumber(m_locale, this);
  m_tab->addTab( m_localenum, QString::null );
  m_localemon = new KLocaleConfigMoney(m_locale, this);
  m_tab->addTab( m_localemon, QString::null );
  m_localetime = new KLocaleConfigTime(m_locale, this);
  m_tab->addTab( m_localetime, QString::null );

  // Examples
  m_gbox = new QVGroupBox(this);
  m_sample = new KLocaleSample(m_locale, m_gbox);

  // getting signals from childs
  connect(m_localemain, SIGNAL(localeChanged()),
	  this, SIGNAL(localeChanged()));
  connect(m_localemain, SIGNAL(languageChanged()),
	  this, SIGNAL(languageChanged()));

  // run the slots on the childs
  connect(this, SIGNAL(localeChanged()),
  	  m_localemain, SLOT(slotLocaleChanged()));
  connect(this, SIGNAL(localeChanged()),
	  m_localenum, SLOT(slotLocaleChanged()));
  connect(this, SIGNAL(localeChanged()),
  m_localemon, SLOT(slotLocaleChanged()));
  connect(this, SIGNAL(localeChanged()),
  	  m_localetime, SLOT(slotLocaleChanged()));

  // keep the example up to date
  // NOTE: this will make the sample be updated 6 times the first time
  // because combo boxes++ emits the change signal not only when the user changes
  // it, but also when it's changed by the program.
  connect(m_localenum, SIGNAL(localeChanged()),
	  m_sample, SLOT(slotLocaleChanged()));
  connect(m_localemon, SIGNAL(localeChanged()),
	  m_sample, SLOT(slotLocaleChanged()));
  connect(m_localetime, SIGNAL(localeChanged()),
	  m_sample, SLOT(slotLocaleChanged()));
  connect(this, SIGNAL(localeChanged()),
	  m_sample, SLOT(slotLocaleChanged()));

  // make sure we always have translated interface
  connect(this, SIGNAL(languageChanged()),
	  this, SLOT(slotTranslate()));
  connect(this, SIGNAL(languageChanged()),
	  m_localemain, SLOT(slotTranslate()));
  connect(this, SIGNAL(languageChanged()),
	  m_localenum, SLOT(slotTranslate()));
  connect(this, SIGNAL(languageChanged()),
	  m_localemon, SLOT(slotTranslate()));
  connect(this, SIGNAL(languageChanged()),
	  m_localetime, SLOT(slotTranslate()));

  // mark it as changed when we change it.
  connect(m_localemain, SIGNAL(localeChanged()),
	  SLOT(slotChanged()));
  connect(m_localenum, SIGNAL(localeChanged()),
	  SLOT(slotChanged()));
  connect(m_localemon, SIGNAL(localeChanged()),
	  SLOT(slotChanged()));
  connect(m_localetime, SIGNAL(localeChanged()),
	  SLOT(slotChanged()));

  load();
}

KLocaleApplication::~KLocaleApplication()
{
  delete m_locale;
}

void KLocaleApplication::load()
{
  //delete locale;
  // #### HPB: Do not use environment variables here!
  //  locale = new KLocale(QString::fromLatin1("kcmlocale"), false);
  // *m_locale = KLocale(QString::fromLatin1("kcmlocale"), false);
  // #### HPB: KLocale has to be fixed so that it's possible to ask for a complete
  // reload
  //m_locale->setDefaultsOnly( false );
  kdDebug() << "KLocaleApplication::load() not implemented" << endl;

  emit changed(false);
  emit languageChanged();
  emit localeChanged();
}

void KLocaleApplication::save()
{
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = m_locale;
  KMessageBox::information(this, m_locale->translate
			   ("Changed language settings apply only to "
			    "newly started applications.\nTo change the "
			    "language of all programs, you will have to "
			    "logout first."),
			   m_locale->translate("Applying language settings"));
  // restore the old global locale
  KGlobal::_locale = lsave;

  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver(config, QString::fromLatin1("Locale"));

  bool langChanged = config->readEntry("Language")
    != m_locale->language();

  m_localemain->save();
  m_localenum->save();
  m_localemon->save();
  m_localetime->save();

  // rebuild the date base if language was changed
  if (langChanged)
    {
      KProcess proc;
      proc << QString::fromLatin1("kbuildsycoca");
      proc.start(KProcess::DontCare);
    }

  emit changed(false);
}

void KLocaleApplication::defaults()
{
  // #### HPB: Do not use user config here..
  QStringList languageList(KLocale::defaultLanguage());
  m_locale->setLanguage(languageList);
  m_locale->setCountry(KLocale::defaultCountry());
  m_locale->setDefaultsOnly( true );

  emit localeChanged();
}

QString KLocaleApplication::quickHelp() const
{
  return m_locale->translate("<h1>Locale</h1>\n"
          "<p>From here you can configure language, numeric, and time \n"
          "settings for your particular region. In most cases it will be \n"
          "sufficient to choose the country you live in. For instance KDE \n"
          "will automatically choose \"German\" as language if you choose \n"
          "\"Germany\" from the list. It will also change the time format \n"
          "to use 24 hours and and use comma as decimal separator.</p>\n");
}

void KLocaleApplication::slotTranslate()
{
  // The untranslated string for QLabel are stored in
  // the name() so we use that when retranslating
  QObject *wc;
  QObjectList *list = queryList("QWidget");
  QObjectListIt it(*list);
  while ( (wc = it.current()) != 0 ) {
    ++it;

    // unnamed labels will cause errors and should not be
    // retranslated. E.g. the example box should not be
    // retranslated from here.
    if (wc->name() == 0) continue;
    if (strcmp(wc->name(), "") == 0) continue;
    if (strcmp(wc->name(), "unnamed") == 0) continue;

    if (strcmp(wc->className(), "QLabel") == 0)
      ((QLabel *)wc)->setText( m_locale->translate( wc->name() ) );
    else if (strcmp(wc->className(), "QGroupBox") == 0)
      ((QGroupBox *)wc)->setTitle( m_locale->translate( wc->name() ) );
    else if (strcmp(wc->className(), "QPushButton") == 0 ||
	     strcmp(wc->className(), "KMenuButton") == 0)
      ((QPushButton *)wc)->setText( m_locale->translate( wc->name() ) );
  }
  delete list;

  // Here we have the pointer
  m_gbox->setCaption(m_locale->translate("Examples"));
  m_tab->changeTab(m_localemain, m_locale->translate("&Locale"));
  m_tab->changeTab(m_localenum, m_locale->translate("&Numbers"));
  m_tab->changeTab(m_localemon, m_locale->translate("&Money"));
  m_tab->changeTab(m_localetime, m_locale->translate("&Time && dates"));

  // FIXME: All widgets are done now. However, there are
  // still some problems. Popup menus from the QLabels are
  // not retranslated.
}

void KLocaleApplication::slotChanged()
{
  emit changed(true);
}
