/*
 * localetime.cpp
 *
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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>

#include <kglobal.h>

#define KLocaleConfigAdvanced KLocaleConfigTime
#include <klocale.h>
#undef KLocaleConfigAdvanced

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "main.h"
#include "localetime.h"
#include "localetime.moc"

extern KLocale *locale;

KLocaleConfigTime::KLocaleConfigTime(QWidget *parent, const char*name)
 : QWidget(parent, name)
{
  QLabel *label;

  // Time
  QGridLayout *tl1 = new QGridLayout(this, 1, 1, 10, 5);
  tl1->setColStretch(2, 1); 

  label = new QLabel("1", this, I18N_NOOP("Time format"));
  edTimeFmt = new QLineEdit(this);
  connect( edTimeFmt, SIGNAL( textChanged(const QString &) ), this, SLOT( slotTimeFmtChanged(const QString &) ) );
  tl1->addWidget(label, 0, 1);
  tl1->addWidget(edTimeFmt, 0, 2);

  label = new QLabel("1", this, I18N_NOOP("Date format"));
  edDateFmt = new QLineEdit(this);
  connect( edDateFmt, SIGNAL( textChanged(const QString &) ), this, SLOT( slotDateFmtChanged(const QString &) ) );
  tl1->addWidget(label, 1, 1);
  tl1->addWidget(edDateFmt, 1, 2);

  label = new QLabel("1", this, I18N_NOOP("Short date format"));
  edDateFmtShort = new QLineEdit(this);
  connect( edDateFmtShort, SIGNAL( textChanged(const QString &) ), this, SLOT( slotDateFmtShortChanged(const QString &) ) );
  tl1->addWidget(label, 2, 1);
  tl1->addWidget(edDateFmtShort, 2, 2);
  
  tl1->setRowStretch(3, 1);

  QToolTip::add(edTimeFmt, I18N_NOOP(
"The text in this textbox will be used to format\n"  
"time strings. The sequences below will be replaced:\n"
"\n"
"%H The hour as a decimal number using a 24-hour clock\n"
"   (range 00 to 23).\n"
"%k The hour (24-hour clock) as a decimal number (range\n"
"   0 to 23); single digits are preceded by a blank.\n"
"%I The  hour as a decimal number using a 12-hour clock\n"
"   (range 01 to 12).\n"
"%l The hour (12-hour clock) as a decimal number (range\n"
"   1 to 12); single digits are preceded by a blank.\n"
"%M The minute as a decimal number (range 00 to 59).\n"
"%S The second as a decimal number (range 00 to 61).\n"
"%p Either AM or PM according to the given time\n"
"   value. Noon is treated as Pm and midnight as Am."));

  QToolTip::add(edDateFmt, I18N_NOOP(
"The text in this textbox will be used to format long\n"
"dates. The sequences below will be replaced:\n"
"\n"
"%Y The year with century as a decimal number.\n"
"%y The year without century as a decimal number (00-99).\n"
"%C (year / 100) as decimal number; single digits are\n"
"   preceded by a zero.\n"
"%m The month as a decimal number (01-12).\n"
"%n The month as a decimal number (1-12); single digits are\n"
"   preceded by a blank.\n"
"%b The national representation of the abbreviated month\n"
"   name, where the abbreviation is the first three characters.\n"
"%B The national representation of the full month name.\n"
"%d The day of month as a decimal number (01-31).\n"
"%e The day of month as a decimal number (1-31); single\n"
"   digits are preceded by a blank.\n"
"%j The day of the year as a decimal number (001-366).\n"
"%a The national representation of the abbreviated weekday\n"
"   name, where the abbreviation is the first three characters.\n"
"%A The national representation of the full weekday name.\n"
"%u The weekday (Monday as the first day of the week) as\n"
"   a decimal number (1-7).\n"
"%w The weekday (Sunday as the first day of the week) as\n"
"   a decimal number (0-6)."));

  QToolTip::add(edDateFmtShort, I18N_NOOP(
"The text in this textbox will be used to format short\n"
"dates. Short dates can not be longer than 12 charaters.\n"
"The sequences below will be replaced:\n"
"\n"
"%Y The year with century as a decimal number.\n"
"%y The year without century as a decimal number (00-99).\n"
"%C (year / 100) as decimal number; single digits are\n"
"   preceded by a zero.\n"
"%m The month as a decimal number (01-12).\n"
"%n The month as a decimal number (1-12); single digits are\n"
"   preceded by a blank.\n"
"%b The national representation of the abbreviated month\n"
"   name, where the abbreviation is the first three characters.\n"
"%B The national representation of the full month name.\n"
"%d The day of month as a decimal number (01-31).\n"
"%e The day of month as a decimal number (1-31); single\n"
"   digits are preceded by a blank.\n"
"%j The day of the year as a decimal number (001-366).\n"
"%a The national representation of the abbreviated weekday\n"
"   name, where the abbreviation is the first three characters.\n"
"%A The national representation of the full weekday name.\n"
"%u The weekday (Monday as the first day of the week) as\n"
"   a decimal number (1-7).\n"
"%w The weekday (Sunday as the first day of the week) as\n"
"   a decimal number (0-6)."));

  load();
}

KLocaleConfigTime::~KLocaleConfigTime()
{
}

void KLocaleConfigTime::load()
{
  edTimeFmt->setText(locale->_timefmt);
  edDateFmt->setText(locale->_datefmt);
  edDateFmtShort->setText(locale->_datefmtshort);
}

void KLocaleConfigTime::save()
{
  KSimpleConfig *c = new KSimpleConfig("kdeglobals", false);
  c->setGroup("Locale");
  // Write something to the file to make it dirty
  c->writeEntry("TimeFormat", QString::null);

  c->deleteEntry("TimeFormat", false);
  c->deleteEntry("DateFormat", false);
  c->deleteEntry("DateFormatShort", false);
  delete c;

  KConfigBase *config = new KConfig;
  config->setGroup("Locale");

  KSimpleConfig ent(locate("locale", "l10n/" + locale->time + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  QString str;

  str = ent.readEntry("TimeFormat", "%I:%M:%S %p");
  str = config->readEntry("TimeFormat", str);
  if (str != locale->_timefmt)
    config->writeEntry("TimeFormat", locale->_timefmt, true, true);

  str = ent.readEntry("DateFormat", "%A %d %B %Y");
  str = config->readEntry("DateFormat", str);
  if (str != locale->_datefmt)
    config->writeEntry("DateFormat", locale->_datefmt, true, true);

  str = ent.readEntry("DateFormatShort", "%m/%d/%y");
  str = config->readEntry("DateFormatShort", str);
  if (str != locale->_datefmtshort)
    config->writeEntry("DateFormatShort", locale->_datefmtshort, true, true);

  delete config;
}

void KLocaleConfigTime::defaults()
{
  reset();
}

void KLocaleConfigTime::slotTimeFmtChanged(const QString &t)
{
  locale->_timefmt = t;
  emit resample();
}

void KLocaleConfigTime::slotDateFmtChanged(const QString &t)
{
  locale->_datefmt = t;
  emit resample();
}

void KLocaleConfigTime::slotDateFmtShortChanged(const QString &t)
{
  locale->_datefmtshort = t;
  emit resample();
}

void KLocaleConfigTime::reset()
{
  KSimpleConfig ent(locate("locale", "l10n/" + locale->time + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  locale->_timefmt = ent.readEntry("TimeFormat", "%I:%M:%S %p");
  locale->_datefmt = ent.readEntry("DateFormat", "%A %d %B %Y");
  locale->_datefmtshort = ent.readEntry("DateFormatShort", "%m/%d/%y");

  load();
}
