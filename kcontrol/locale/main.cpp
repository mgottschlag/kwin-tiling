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
#include <qgroupbox.h>
#include <qlabel.h>
#include <qobjectlist.h>
#include <qtabwidget.h>

#include <kcmodule.h>
#include <klocale.h>
#include <kglobal.h>

#include "locale.h"
#include "localenum.h"
#include "localemon.h"
#include "localetime.h"
#include "main.h"


KLocaleApplication::KLocaleApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  locale = 0;
  localenum = 0;
  localemon = 0;
  localetime = 0;

  if (runGUI())
    {
#define i18n(a) (a)
      if (!pages || pages->contains("language"))
        addPage(locale = new KLocaleConfig(dialog, i18n("&Locale")), 
		0, "locale-1.html");
      if (!pages || pages->contains("number"))
        addPage(localenum = new KLocaleConfigNumber(dialog, i18n("&Numbers")), 
		0, "locale-2.html");
      if (!pages || pages->contains("money"))
        addPage(localemon = new KLocaleConfigMoney(dialog, i18n("&Money")), 
		0, "locale-3.html");
      if (!pages || pages->contains("time"))
        addPage(localetime = new KLocaleConfigTime(dialog, i18n("&Time && dates")), 
		0, "locale-4.html");
#undef i18n

      reTranslate();
      updateSample();

      if (locale || localenum || localemon || localetime)
        dialog->show();
      else
        {
          fprintf(stderr, i18n("usage: kcmlocale [-init | language | number | money | time]\n").ascii());
	  justInit = TRUE;
        }
    }
}


void KLocaleApplication::init()
{
}

void KLocaleApplication::apply()
{
  if (locale)
    locale->save();
  if (localenum)
    localenum->save();
  if (localemon)
    localemon->save();
  if (localetime)
    localetime->save();
}

void KLocaleApplication::defaultValues()
{
  if (locale)
    locale->defaults();
  if (localenum)
    localenum->defaults();
  if (localemon)
    localemon->defaults();
  if (localetime)
    localetime->defaults();
}

void KLocaleApplication::updateSample()
{
  if (locale)
    locale->updateSample();
  if (localenum)
    localenum->updateSample();
  if (localemon)
    localemon->updateSample();
  if (localetime)
    localetime->updateSample();
}

void KLocaleApplication::reTranslate()
{
  QObjectList it;
  it.append(kapp-> mainWidget());
  reTranslate(it);
  setTitle(i18n("Locale settings"));
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

	if (s)
          ((QPushButton *)wc)->setText(i18n(s));
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
	if (locale)
          ((QTabWidget *)wc)->changeTab(locale, i18n(locale->name()));
        if (localenum)
          ((QTabWidget *)wc)->changeTab(localenum, i18n(localenum->name()));
        if (localemon)
          ((QTabWidget *)wc)->changeTab(localemon, i18n(localemon->name()));
        if (localetime)
          ((QTabWidget *)wc)->changeTab(localetime, i18n(localetime->name()));
      }
    }
}

void KLocaleApplication::reset()
{
  resetNum();
  resetMon();
  resetTime();
}

void KLocaleApplication::resetMon()
{
  if (localemon)
    localemon->reset();
}

void KLocaleApplication::resetNum()
{
  if (localenum)
    localenum->reset();
}

void KLocaleApplication::resetTime()
{
  if (localetime)
    localetime->reset();
}

int main(int argc, char **argv)
{
  KLocaleApplication app(argc, argv, "kcmlocale");
  
  if (app.runGUI())
    return app.exec();
  else
    {
      app.init();
      return 0;
    }
}
