/*
 * localetime.cpp
 *
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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <klocale.h>
#include <kglobal.h>
#include <kdialog.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "toplevel.h"
#include "localetime.h"
#include "localetime.moc"

KLocaleConfigTime::KLocaleConfigTime(KLocale *_locale,
				     QWidget *parent, const char*name)
 : QWidget(parent, name),
   m_locale(_locale)
{
  // Time
  QGridLayout *lay = new QGridLayout(this, 5, 2, 
				     KDialog::marginHint(),
				     KDialog::spacingHint());
  lay->setAutoAdd(TRUE);

  m_labTimeFmt = new QLabel(this, I18N_NOOP("Time format:"));
  m_edTimeFmt = new QLineEdit(this);
  connect( m_edTimeFmt, SIGNAL( textChanged(const QString &) ), this, SLOT( slotTimeFmtChanged(const QString &) ) );

  m_labDateFmt = new QLabel(this, I18N_NOOP("Date format:"));
  m_edDateFmt = new QLineEdit(this);
  connect( m_edDateFmt, SIGNAL( textChanged(const QString &) ), this, SLOT( slotDateFmtChanged(const QString &) ) );

  m_labDateFmtShort = new QLabel(this, I18N_NOOP("Short date format:"));
  m_edDateFmtShort = new QLineEdit(this);
  connect( m_edDateFmtShort, SIGNAL( textChanged(const QString &) ), this, SLOT( slotDateFmtShortChanged(const QString &) ) );

  m_labWeekStartsMonday = new QLabel(this, I18N_NOOP("Start week on Monday:"));
  m_chWeekStartsMonday = new QCheckBox(this);
  connect( m_chWeekStartsMonday, SIGNAL( clicked() ), this,
	   SLOT( slotWeekStartsMondayChanged() ) );
  
  lay->setColStretch(1, 1);
}

KLocaleConfigTime::~KLocaleConfigTime()
{
}

void KLocaleConfigTime::save()
{
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = m_locale;

  KSimpleConfig *c = new KSimpleConfig(QString::fromLatin1("kdeglobals"), false);
  c->setGroup(QString::fromLatin1("Locale"));
  // Write something to the file to make it dirty
  c->writeEntry(QString::fromLatin1("TimeFormat"), QString::null);

  c->deleteEntry(QString::fromLatin1("TimeFormat"), false);
  c->deleteEntry(QString::fromLatin1("DateFormat"), false);
  c->deleteEntry(QString::fromLatin1("DateFormatShort"), false);
  c->deleteEntry(QString::fromLatin1("WeekStartsMonday"), false);
  delete c;

  KConfig *config = KGlobal::config();
  config->reparseConfiguration();
  KConfigGroupSaver saver(config, QString::fromLatin1("Locale"));

  KSimpleConfig ent(locate("locale",
			   QString::fromLatin1("l10n/%1/entry.desktop")
			   .arg(m_locale->country())), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));

  QString str;

  str = ent.readEntry(QString::fromLatin1("TimeFormat"), QString::fromLatin1("%H:%M:%S"));
  str = config->readEntry(QString::fromLatin1("TimeFormat"), str);
  if (str != m_locale->timeFormat())
    config->writeEntry(QString::fromLatin1("TimeFormat"), m_locale->timeFormat(), true, true);

  str = ent.readEntry(QString::fromLatin1("DateFormat"), QString::fromLatin1("%A %d %B %Y"));
  str = config->readEntry(QString::fromLatin1("DateFormat"), str);
  if (str != m_locale->dateFormat())
    config->writeEntry(QString::fromLatin1("DateFormat"), m_locale->dateFormat(), true, true);

  str = ent.readEntry(QString::fromLatin1("DateFormatShort"), QString::fromLatin1("%Y-%m-%d"));
  str = config->readEntry(QString::fromLatin1("DateFormatShort"), str);
  if (str != m_locale->dateFormatShort())
    config->writeEntry(QString::fromLatin1("DateFormatShort"), m_locale->dateFormatShort(), true, true);

  bool b;
  b = ent.readBoolEntry(QString::fromLatin1("WeekStartsMonday"), true);
  b = config->readBoolEntry(QString::fromLatin1("WeekStartsMonday"), b);
  if (b != m_locale->weekStartsMonday())
    config->writeEntry(QString::fromLatin1("WeekStartsMonday"),
		       m_locale->weekStartsMonday(), true, true);

  config->sync();

  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfigTime::slotLocaleChanged()
{
  m_edTimeFmt->setText( m_locale->timeFormat() );
  m_edDateFmt->setText( m_locale->dateFormat() );
  m_edDateFmtShort->setText( m_locale->dateFormatShort() );
  m_chWeekStartsMonday->setChecked( m_locale->weekStartsMonday() );
}

void KLocaleConfigTime::slotTimeFmtChanged(const QString &t)
{
  m_locale->setTimeFormat(t);
  emit localeChanged();
}

void KLocaleConfigTime::slotDateFmtChanged(const QString &t)
{
  m_locale->setDateFormat(t);
  emit localeChanged();
}

void KLocaleConfigTime::slotDateFmtShortChanged(const QString &t)
{
  m_locale->setDateFormatShort(t);
  emit localeChanged();
}

void KLocaleConfigTime::slotWeekStartsMondayChanged()
{
  m_locale->setWeekStartsMonday(m_chWeekStartsMonday->isChecked());
  emit localeChanged();
}

void KLocaleConfigTime::slotTranslate()
{
  QString str;

  str = m_locale->translate
    ("<p>The text in this textbox will be used to format "
     "time strings. The sequences below will be replaced:</p>"
     "<table>"
     "<tr><td><b>%H</b></td><td>The hour as a decimal number using a 24-hour "
     "clock (00-23).</td></tr>"
     "<tr><td><b>%k</b></td><td>The hour (24-hour clock) as a decimal number "
     "(0-23).</td></tr>"
     "<tr><td><b>%I</b></td><td>The hour as a decimal number using a 12-hour "
     "clock (01-12).</td></tr>"
     "<tr><td><b>%l</b></td><td>The hour (12-hour clock) as a decimal number "
     "(1-12).</td></tr>"
     "<tr><td><b>%M</b></td><td>The minutes as a decimal number (00-59)."
     "</td><tr>"
     "<tr><td><b>%S</b></td><td>The seconds as a decimal number (00-59)."
     "</td></tr>"
     "<tr><td><b>%p</b></td><td>Either \"am\" or \"pm\" according to the "
     "given time value. Noon is treated as \"pm\" and midnight as \"am\"."
     "</td></tr>"
     "</table>");
  QWhatsThis::add( m_labTimeFmt, str );
  QWhatsThis::add( m_edTimeFmt,  str );

  QString datecodes = m_locale->translate(
    "<table>"
    "<tr><td><b>%Y</b></td><td>The year with century as a decimal number."
    "</td></tr>"
    "<tr><td><b>%y</b></td><td>The year without century as a decimal number "
    "(00-99).</td></tr>"
    "<tr><td><b>%m</b></td><td>The month as a decimal number (01-12)."
    "</td></tr>"
    "<tr><td><b>%n</b></td><td>The month as a decimal number (1-12).</td></tr>"
    "<tr><td><b>%b</b></td><td>The first three characters of the month name. "
    "</td></tr>"
    "<tr><td><b>%B</b></td><td>The full month name.</td></tr>"
    "<tr><td><b>%d</b></td><td>The day of month as a decimal number (01-31)."
    "</td></tr>"
    "<tr><td><b>%e</b></td><td>The day of month as a decimal number (1-31)."
    "</td></tr>"
    "<tr><td><b>%a</b></td><td>The first three characters of the weekday name."
    "</td></tr>"
    "<tr><td><b>%A</b></td><td>The full weekday name.</td></tr>"
    "</table>");

  str = m_locale->translate
    ( "<p>The text in this textbox will be used to format long "
      "dates. The sequences below will be replaced:</p>") + datecodes;
  QWhatsThis::add( m_labDateFmt, str );
  QWhatsThis::add( m_edDateFmt,  str );
  
  str = m_locale->translate
    ( "<p>The text in this textbox will be used to format short "
      "dates. For instance, this is used when listing files. "
      "The sequences below will be replaced:</p>") + datecodes;
  QWhatsThis::add( m_labDateFmtShort, str );
  QWhatsThis::add( m_edDateFmtShort,  str );

  str = m_locale->translate
    ("If this option is checked, calendars will be printed "
     "with Monday as the first day in the week. If not, "
     "Sunday will be used instead.");
  QWhatsThis::add( m_labWeekStartsMonday, str );
  QWhatsThis::add( m_chWeekStartsMonday,  str );
}
