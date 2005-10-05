/*
  toplevel.cpp - A KControl Application

  Copyright 1998 Matthias Hoelzer
  Copyright 1999-2003 Hans Petter Bieker <bieker@kde.org>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

  */

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <q3groupbox.h>

#include <kaboutdata.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kservice.h>

#include "localenum.h"
#include "localemon.h"
#include "localetime.h"
#include "localeother.h"
#include "klocalesample.h"
#include "toplevel.h"
#include "kcmlocale.h"
#include "toplevel.moc"

KLocaleApplication::KLocaleApplication(QWidget *parent, const char* /*name*/,
                                       const QStringList &args)
  : KCModule( KLocaleFactory::instance(), parent, args)
{
  KAboutData* aboutData = new KAboutData("kcmlocale",
        I18N_NOOP("KCMLocale"),
        "3.0",
        I18N_NOOP("Regional settings"),
        KAboutData::License_GPL,
        "(C) 1998 Matthias Hoelzer, "
        "(C) 1999-2003 Hans Petter Bieker",
        0, 0, "bieker@kde.org");
  setAboutData( aboutData );

  m_nullConfig = new KConfig(QString::null, false, false);
  m_globalConfig = new KConfig(QString::null, false, true);

  m_locale = new KLocale(QLatin1String("kcmlocale"), m_nullConfig);
  QVBoxLayout *l = new QVBoxLayout(this, 0, KDialog::spacingHint());
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
  m_localeother = new KLocaleConfigOther(m_locale, this);
  m_tab->addTab( m_localeother, QString::null );

  // Examples
  m_gbox = new Q3GroupBox(this);
  m_gbox->setOrientation(Qt::Vertical);
  QVBoxLayout *laygroup = new QVBoxLayout(m_gbox->layout(), KDialog::spacingHint() );
  m_sample = new KLocaleSample(m_locale, m_gbox);
  laygroup->addWidget( m_sample );


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
  connect(this, SIGNAL(localeChanged()),
          m_localeother, SLOT(slotLocaleChanged()));

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
  // No examples for this yet
  //connect(m_localeother, SIGNAL(slotLocaleChanged()),
  //m_sample, SLOT(slotLocaleChanged()));
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
  connect(this, SIGNAL(languageChanged()),
          m_localeother, SLOT(slotTranslate()));

  // mark it as changed when we change it.
  connect(m_localemain, SIGNAL(localeChanged()),
          SLOT(slotChanged()));
  connect(m_localenum, SIGNAL(localeChanged()),
          SLOT(slotChanged()));
  connect(m_localemon, SIGNAL(localeChanged()),
          SLOT(slotChanged()));
  connect(m_localetime, SIGNAL(localeChanged()),
          SLOT(slotChanged()));
  connect(m_localeother, SIGNAL(localeChanged()),
          SLOT(slotChanged()));

  load();
}

KLocaleApplication::~KLocaleApplication()
{
  delete m_locale;
  delete m_globalConfig;
  delete m_nullConfig;
}

void KLocaleApplication::load()
{
  m_globalConfig->reparseConfiguration();
  *m_locale = KLocale(QLatin1String("kcmlocale"), m_globalConfig);

  emit localeChanged();
  emit languageChanged();
  emit changed(false);
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
                           m_locale->translate("Applying Language Settings"),
                           QLatin1String("LanguageChangesApplyOnlyToNewlyStartedPrograms"));
  // restore the old global locale
  KGlobal::_locale = lsave;

  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver(config, "Locale");

  // ##### this doesn't make sense
  bool langChanged = config->readEntry("Language")
    != m_locale->language();

  m_localemain->save();
  m_localenum->save();
  m_localemon->save();
  m_localetime->save();
  m_localeother->save();

  // rebuild the date base if language was changed
  if (langChanged)
  {
    KService::rebuildKSycoca(this);
  }

  emit changed(false);
}

void KLocaleApplication::defaults()
{
  *m_locale = KLocale(QLatin1String("kcmlocale"), m_nullConfig);

  kdDebug() << "defaults: " << m_locale->languageList() << endl;

  emit localeChanged();
  emit languageChanged();
}

QString KLocaleApplication::quickHelp() const
{
  return m_locale->translate("<h1>Country/Region & Language</h1>\n"
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
  QObjectList list = queryList("QWidget");
  QObjectList::iterator it = list.begin();
  while ( (wc = *it) != 0 )
  {
    ++it;

    // unnamed labels will cause errors and should not be
    // retranslated. E.g. the example box should not be
    // retranslated from here.
    if (wc->name() == 0)
      continue;
    if (::qstrcmp(wc->name(), "") == 0)
      continue;
    if (::qstrcmp(wc->name(), "unnamed") == 0)
      continue;

    if (::qstrcmp(wc->className(), "QLabel") == 0)
      ((QLabel *)wc)->setText( m_locale->translate( wc->name() ) );
    else if (::qstrcmp(wc->className(), "QGroupBox") == 0 ||
             ::qstrcmp(wc->className(), "QVGroupBox") == 0)
      ((QGroupBox *)wc)->setTitle( m_locale->translate( wc->name() ) );
    else if (::qstrcmp(wc->className(), "QPushButton") == 0 ||
             ::qstrcmp(wc->className(), "KMenuButton") == 0)
      ((QPushButton *)wc)->setText( m_locale->translate( wc->name() ) );
    else if (::qstrcmp(wc->className(), "QCheckBox") == 0)
      ((QCheckBox *)wc)->setText( m_locale->translate( wc->name() ) );
  }

  // Here we have the pointer
  m_gbox->setCaption(m_locale->translate("Examples"));
  m_tab->changeTab(m_localemain, m_locale->translate("&Locale"));
  m_tab->changeTab(m_localenum, m_locale->translate("&Numbers"));
  m_tab->changeTab(m_localemon, m_locale->translate("&Money"));
  m_tab->changeTab(m_localetime, m_locale->translate("&Time && Dates"));
  m_tab->changeTab(m_localeother, m_locale->translate("&Other"));

  // FIXME: All widgets are done now. However, there are
  // still some problems. Popup menus from the QLabels are
  // not retranslated.
}

void KLocaleApplication::slotChanged()
{
  emit changed(true);
}

