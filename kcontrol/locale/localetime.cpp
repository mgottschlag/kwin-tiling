/*
 * localetime.cpp
 *
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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qcombobox.h>

#include <klocale.h>
#include <kglobal.h>
#include <kdialog.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "toplevel.h"
#include "localetime.h"
#include "localetime.moc"

QMap<QChar, QString> KLocaleConfigTime::timeMap() const
{
  QMap<QChar, QString> map;

  map['H'] = m_locale->translate("HH");
  map['k'] = m_locale->translate("hH");
  map['I'] = m_locale->translate("PH");
  map['l'] = m_locale->translate("pH");
  map['M'] = m_locale->translate("MM");
  map['S'] = m_locale->translate("SS");
  map['p'] = m_locale->translate("AMPM");

  return map;
}

QMap<QChar, QString> KLocaleConfigTime::dateMap() const
{
  QMap <QChar, QString> map;

  map['Y'] = m_locale->translate("YYYY");
  map['y'] = m_locale->translate("YY");
  map['n'] = m_locale->translate("mM");
  map['m'] = m_locale->translate("MM");
  map['b'] = m_locale->translate("SHORTMONTH");
  map['B'] = m_locale->translate("MONTH");
  map['e'] = m_locale->translate("dD");
  map['d'] = m_locale->translate("DD");
  map['a'] = m_locale->translate("SHORTWEEKDAY");
  map['A'] = m_locale->translate("WEEKDAY");

  return map;
}

QString KLocaleConfigTime::userToStore(const QMap<QChar, QString> & map,
		    const QString & userFormat) const
{
  QString result;

  for ( uint pos = 0; pos < userFormat.length(); ++pos )
    {
      bool bFound = false;
      for ( QMap<QChar, QString>::ConstIterator it = map.begin();
	    it != map.end() && !bFound;
	    ++it )
	{
	  QString s = it.data();

	  if ( userFormat.mid( pos, s.length() ) == s )
	    {
	      result += '%';
	      result += it.key();

	      pos += s.length() - 1;

	      bFound = true;
	    }
	}

      if ( !bFound )
	{
	  QChar c = userFormat.at( pos );
	  if ( c == '%' )
	    result += c;

	  result += c;
	}
    }

  return result;
}

QString KLocaleConfigTime::storeToUser(const QMap<QChar, QString> & map,
				       const QString & storeFormat) const
{
  QString result;

  bool escaped = false;  
  for ( uint pos = 0; pos < storeFormat.length(); ++pos )
    {
      QChar c = storeFormat.at(pos);
      if ( escaped )
	{
	  QMap<QChar, QString>::ConstIterator it = map.find( c );
	  if ( it != map.end() )
	    result += it.data();
	  else
	    result += c;

	  escaped = false;
	}
      else if ( c == '%' )
	escaped = true;
      else
	result += c;
    }

  return result;
}


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
  m_comboTimeFmt = new QComboBox(true, this);
  //m_edTimeFmt = m_comboTimeFmt->lineEdit();
  //m_edTimeFmt = new QLineEdit(this);
  connect( m_comboTimeFmt, SIGNAL( textChanged(const QString &) ),
	   this, SLOT( slotTimeFmtChanged(const QString &) ) );

  m_labDateFmt = new QLabel(this, I18N_NOOP("Date format:"));
  m_comboDateFmt = new QComboBox(true, this);
  connect( m_comboDateFmt, SIGNAL( textChanged(const QString &) ),
	   this, SLOT( slotDateFmtChanged(const QString &) ) );

  m_labDateFmtShort = new QLabel(this, I18N_NOOP("Short date format:"));
  m_comboDateFmtShort = new QComboBox(true, this);
  connect( m_comboDateFmtShort, SIGNAL( textChanged(const QString &) ),
	   this, SLOT( slotDateFmtShortChanged(const QString &) ) );

  m_labWeekStartsMonday = new QLabel(this, I18N_NOOP("Start week on Monday:"));
  m_chWeekStartsMonday = new QCheckBox(this);
  connect( m_chWeekStartsMonday, SIGNAL( clicked() ),
	   this, SLOT( slotWeekStartsMondayChanged() ) );
  
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

  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver(config, "Locale");

  KSimpleConfig ent(locate("locale",
			   QString::fromLatin1("l10n/%1/entry.desktop")
			   .arg(m_locale->country())), true);
  ent.setGroup("KCM Locale");

  QString str;

  str = ent.readEntry("TimeFormat", QString::fromLatin1("%H:%M:%S"));
  config->deleteEntry("TimeFormat", false, true);
  if (str != m_locale->timeFormat())
    config->writeEntry("TimeFormat", m_locale->timeFormat(), true, true);

  str = ent.readEntry("DateFormat", QString::fromLatin1("%A %d %B %Y"));
  config->deleteEntry("DateFormat", false, true);
  if (str != m_locale->dateFormat())
    config->writeEntry("DateFormat", m_locale->dateFormat(), true, true);

  str = ent.readEntry("DateFormatShort", QString::fromLatin1("%Y-%m-%d"));
  config->deleteEntry("DateFormatShort", false, true);
  if (str != m_locale->dateFormatShort())
    config->writeEntry("DateFormatShort",
		       m_locale->dateFormatShort(), true, true);

  bool b;
  b = ent.readBoolEntry("WeekStartsMonday", true);
  config->deleteEntry("WeekStartsMonday", false, true);
  if (b != m_locale->weekStartsMonday())
    config->writeEntry("WeekStartsMonday",
		       m_locale->weekStartsMonday(), true, true);

  config->sync();

  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfigTime::slotLocaleChanged()
{
  //  m_edTimeFmt->setText( m_locale->timeFormat() );
  m_comboTimeFmt->setEditText( storeToUser( timeMap(),
					    m_locale->timeFormat() ) );
  // m_edDateFmt->setText( m_locale->dateFormat() );
  m_comboDateFmt->setEditText( storeToUser( dateMap(),
					    m_locale->dateFormat() ) );
  //m_edDateFmtShort->setText( m_locale->dateFormatShort() );
  m_comboDateFmtShort->setEditText( storeToUser( dateMap(),
					  m_locale->dateFormatShort() ) );
  m_chWeekStartsMonday->setChecked( m_locale->weekStartsMonday() );

  kdDebug() << "converting: " << m_locale->timeFormat() << endl;
  kdDebug() << storeToUser(timeMap(),
			   m_locale->timeFormat()) << endl;
  kdDebug() << userToStore(timeMap(),
			   QString::fromLatin1("HH:MM:SS AMPM test")) << endl;

}

void KLocaleConfigTime::slotTimeFmtChanged(const QString &t)
{
  //  m_locale->setTimeFormat(t);
  m_locale->setTimeFormat( userToStore( timeMap(), t ) );

  emit localeChanged();
}

void KLocaleConfigTime::slotDateFmtChanged(const QString &t)
{
  // m_locale->setDateFormat(t);
  m_locale->setDateFormat( userToStore( dateMap(), t ) );
  emit localeChanged();
}

void KLocaleConfigTime::slotDateFmtShortChanged(const QString &t)
{
  //m_locale->setDateFormatShort(t);
  m_locale->setDateFormatShort( userToStore( dateMap(), t ) );
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

  QString sep = QString::fromLatin1("\n");

  QString old;

  // clear() and insertStringList also changes the current item, so
  // we better use save and restore here..
  old = m_comboTimeFmt->currentText();
  m_comboTimeFmt->clear();
  str = i18n("some reasonable time formats for the language",
	     "HH:MM:SS\n"
	     "pH:MM:SS AMPM");
  m_comboTimeFmt->insertStringList(QStringList::split(sep, str));
  m_comboTimeFmt->setEditText(old);

  old = m_comboDateFmt->currentText();
  m_comboDateFmt->clear();
  str = i18n("some reasonable date formats for the language",
	     "WEEKDAY MONTH dD YYYY\n"
	     "SHORTWEEKDAY MONTH dD YYYY");
  m_comboDateFmt->insertStringList(QStringList::split(sep, str));
  m_comboDateFmt->setEditText(old);
  
  old = m_comboDateFmtShort->currentText();
  m_comboDateFmtShort->clear();
  str = i18n("some resasonable short date formats for the language",
	     "YYYY-MM-DD\n"
	     "dD.mM.YYYY\n"
	     "DD.MM.YYYY");
  m_comboDateFmtShort->insertStringList(QStringList::split(sep, str));
  m_comboDateFmtShort->setEditText(old);

  str = m_locale->translate
    ("<p>The text in this textbox will be used to format "
     "time strings. The sequences below will be replaced:</p>"
     "<table>"
     "<tr><td><b>HH</b></td><td>The hour as a decimal number using a 24-hour "
     "clock (00-23).</td></tr>"
     "<tr><td><b>hH</b></td><td>The hour (24-hour clock) as a decimal number "
     "(0-23).</td></tr>"
     "<tr><td><b>PH</b></td><td>The hour as a decimal number using a 12-hour "
     "clock (01-12).</td></tr>"
     "<tr><td><b>pH</b></td><td>The hour (12-hour clock) as a decimal number "
     "(1-12).</td></tr>"
     "<tr><td><b>MM</b></td><td>The minutes as a decimal number (00-59)."
     "</td><tr>"
     "<tr><td><b>SS</b></td><td>The seconds as a decimal number (00-59)."
     "</td></tr>"
     "<tr><td><b>AMPM</b></td><td>Either \"am\" or \"pm\" according to the "
     "given time value. Noon is treated as \"pm\" and midnight as \"am\"."
     "</td></tr>"
     "</table>");
  QWhatsThis::add( m_labTimeFmt, str );
  QWhatsThis::add( m_comboTimeFmt,  str );

  QString datecodes = m_locale->translate(
    "<table>"
    "<tr><td><b>YYYY</b></td><td>The year with century as a decimal number."
    "</td></tr>"
    "<tr><td><b>YY</b></td><td>The year without century as a decimal number "
    "(00-99).</td></tr>"
    "<tr><td><b>MM</b></td><td>The month as a decimal number (01-12)."
    "</td></tr>"
    "<tr><td><b>mM</b></td><td>The month as a decimal number (1-12).</td></tr>"
    "<tr><td><b>SHORTMONTH</b></td><td>The first three characters of the month name. "
    "</td></tr>"
    "<tr><td><b>MONTH</b></td><td>The full month name.</td></tr>"
    "<tr><td><b>DD</b></td><td>The day of month as a decimal number (01-31)."
    "</td></tr>"
    "<tr><td><b>dD</b></td><td>The day of month as a decimal number (1-31)."
    "</td></tr>"
    "<tr><td><b>SHORTWEEKDAY</b></td><td>The first three characters of the weekday name."
    "</td></tr>"
    "<tr><td><b>WEEKDAY</b></td><td>The full weekday name.</td></tr>"
    "</table>");

  str = m_locale->translate
    ( "<p>The text in this textbox will be used to format long "
      "dates. The sequences below will be replaced:</p>") + datecodes;
  QWhatsThis::add( m_labDateFmt, str );
  QWhatsThis::add( m_comboDateFmt,  str );
  
  str = m_locale->translate
    ( "<p>The text in this textbox will be used to format short "
      "dates. For instance, this is used when listing files. "
      "The sequences below will be replaced:</p>") + datecodes;
  QWhatsThis::add( m_labDateFmtShort, str );
  QWhatsThis::add( m_comboDateFmtShort,  str );

  str = m_locale->translate
    ("If this option is checked, calendars will be printed "
     "with Monday as the first day in the week. If not, "
     "Sunday will be used instead.");
  QWhatsThis::add( m_labWeekStartsMonday, str );
  QWhatsThis::add( m_chWeekStartsMonday,  str );
}

