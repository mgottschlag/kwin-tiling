/*
  main.cpp - A KControl Application

  written 1998 by Matthias Hoelzer
  
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

#include <stdio.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qobjectlist.h>
#include <qlayout.h>

#include <kcmodule.h>
#include <klocale.h>
#include <kglobal.h>

#include "locale.h"
#include "localenum.h"
#include "localemon.h"
#include "localetime.h"
#include "klocalesample.h"
#include "main.h"
#include "main.moc"


KLocaleApplication::KLocaleApplication(QWidget *parent, const char *name)
  : KCModule(parent, name)
{

  QVBoxLayout *l = new QVBoxLayout(this, 5);

  tab = new QTabWidget( this, "section" );
  l->addWidget(tab);

#define i18n(a) (a)
  locale = new KLocaleConfig( this, i18n("&Locale") );
  tab->addTab( locale, "1");
  localenum = new KLocaleConfigNumber( this, i18n("&Numbers") );
  tab->addTab( localenum, "1" );
  localemon = new KLocaleConfigMoney( this, i18n("&Money") );
  tab->addTab( localemon, "1" ); 
  localetime = new KLocaleConfigTime( this, i18n("&Time && dates") );
  tab->addTab( localetime, "1" ); 
#undef i18n

  connect(locale,     SIGNAL(resample()), this, SLOT(update()));
  connect(localenum,  SIGNAL(resample()), this, SLOT(update()));
  connect(localemon,  SIGNAL(resample()), this, SLOT(update()));
  connect(localetime, SIGNAL(resample()), this, SLOT(update()));

  // Examples
  gbox = new QGroupBox("1", this, i18n("Examples"));
  l->addWidget(gbox);
  sample = new KLocaleSample(gbox);

  reTranslate();
  updateSample();
}


void KLocaleApplication::load()
{
    locale->load();
    localenum->load();
    localemon->load();
    localetime->load();
}

void KLocaleApplication::save()
{
    locale->save();
    localenum->save();
    localemon->save();
    localetime->save();
}

void KLocaleApplication::defaults()
{
    locale->defaults();
    localenum->defaults();
    localemon->defaults();
    localetime->defaults();
}

void KLocaleApplication::updateSample()
{
    sample->update();
}

void KLocaleApplication::reTranslate()
{
  QObjectList it;
  it.append( this );
  reTranslate(it);
  //CT ?  setTitle(i18n("Locale settings"));
}

void KLocaleApplication::reTranslate(QObjectListIt it)
{
    QObject *wc;
    while( (wc = it.current()) != 0 ) {
      ++it;
      if (wc->children())
        reTranslate(QObjectListIt(*wc->children()));

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
      else if ( !qstrcmp(wc->className(), "QPushButton"))
      {
	const char *s = 0; 
	if (!qstrcmp(wc->name(), "ok"))
	  s = "&OK";
	else if (!qstrcmp(wc->name(), "apply settings"))
	  s = "&Apply";
	else if (!qstrcmp(wc->name(), "cancel dialog"))
	  s = "&Cancel";
	else if (!qstrcmp(wc->name(), "back to default"))
	  s = "&Defaults";
	else if (!qstrcmp(wc->name(), "give help"))
	  s = "&Help";

	//if (s)
	  // ((QPushButton *)wc)->setText(i18n(s));
      }
      else if ( !qstrcmp(wc->className(), "QComboBox"))
      {
	if (!qstrcmp(wc->name(), "signpos"))
	{
	  ((QComboBox*)wc)->changeItem(i18n("Parens around"), 0);
	  ((QComboBox*)wc)->changeItem(i18n("Before quantity money"), 1);
	  ((QComboBox*)wc)->changeItem(i18n("After quantity money"), 2);
	  ((QComboBox*)wc)->changeItem(i18n("Before money"), 3);
	  ((QComboBox*)wc)->changeItem(i18n("After money"), 4);
	}
      }
      else if ( !qstrcmp(wc->className(), "QTabWidget"))
      {
          ((QTabWidget *)wc)->changeTab(locale, i18n(locale->name()));
          ((QTabWidget *)wc)->changeTab(localenum, i18n(localenum->name()));
          ((QTabWidget *)wc)->changeTab(localemon, i18n(localemon->name()));
          ((QTabWidget *)wc)->changeTab(localetime, i18n(localetime->name()));
      }
    }
}

void KLocaleApplication::reset()
{
  localenum->reset();
  localemon->reset();
  localetime->reset();
}

extern "C" {
  KCModule *create_locale(QWidget *parent, const char* name) {
    KGlobal::locale()->insertCatalogue("kcmlocale");
    return new KLocaleApplication(parent, name);
  }
}
