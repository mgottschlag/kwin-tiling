/*
 * localetime.cpp
 *
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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include <qcombobox.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QGridLayout>
//Should have been:
#include <QtAlgorithms>

#include <kdialog.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kcalendarsystem.h>

#include "toplevel.h"
#include "localetime.h"
#include "localetime.moc"

class StringPair
{
public:
  QChar storeName;
  QString userName;

  static StringPair find( const QList <StringPair> &list, const QChar &c)
  {
    for ( QList<StringPair>::ConstIterator it = list.begin();
	it != list.end();
	++it )
      if ((*it).storeName==c) return (*it);

    StringPair r;
    return r;
  }

};

/*  Sort the string pairs with qHeapSort in the order we want
    ( relative to the userName value and with "MESCORTO" before "MES" )
    */
bool operator< (const StringPair &p1, const StringPair &p2)
{
  return ! (p1.userName<p2.userName);
}

bool operator<= (const StringPair &p1, const StringPair &p2)
{
  return ! (p1.userName<=p2.userName);
}

bool operator> (const StringPair &p1, const StringPair &p2)
{
  return ! (p1.userName>p2.userName);
}

bool operator>= (const StringPair &p1, const StringPair &p2)
{
  return ! (p1.userName>=p2.userName);
}

StringPair KLocaleConfigTime::buildStringPair(const QChar &c, const QString &s) const
{
  StringPair pair;
  pair.storeName=c;
  pair.userName=s;
  return pair;
}

QList<StringPair> KLocaleConfigTime::timeMap() const
{
  QList < StringPair > list;
  list+=buildStringPair('H',ki18n("HH").toString(m_locale));
  list+=buildStringPair('k',ki18n("hH").toString(m_locale));
  list+=buildStringPair('I',ki18n("PH").toString(m_locale));
  list+=buildStringPair('l',ki18n("pH").toString(m_locale));
  list+=buildStringPair('M',ki18nc("Minute", "MM").toString(m_locale));
  list+=buildStringPair('S',ki18n("SS").toString(m_locale));
  list+=buildStringPair('p',ki18n("AMPM").toString(m_locale));

  qSort( list );

  return list;
}

QList <StringPair> KLocaleConfigTime::dateMap() const
{
  QList < StringPair > list;
  list+=buildStringPair('Y',ki18n("YYYY").toString(m_locale));
  list+=buildStringPair('y',ki18n("YY").toString(m_locale));
  list+=buildStringPair('n',ki18n("mM").toString(m_locale));
  list+=buildStringPair('m',ki18nc("Month", "MM").toString(m_locale));
  list+=buildStringPair('b',ki18n("SHORTMONTH").toString(m_locale));
  list+=buildStringPair('B',ki18n("MONTH").toString(m_locale));
  list+=buildStringPair('e',ki18n("dD").toString(m_locale));
  list+=buildStringPair('d',ki18n("DD").toString(m_locale));
  list+=buildStringPair('a',ki18n("SHORTWEEKDAY").toString(m_locale));
  list+=buildStringPair('A',ki18n("WEEKDAY").toString(m_locale));

  qSort( list );

  return list;
}

QString KLocaleConfigTime::userToStore(const QList<StringPair> & list,
		    const QString & userFormat) const
{
  QString result;

  for ( int pos = 0; pos < userFormat.length(); ++pos )
    {
      bool bFound = false;
      for ( QList<StringPair>::ConstIterator it = list.begin();
	    it != list.end() && !bFound;
	    ++it )
	{
	  QString s = (*it).userName;

	  if ( userFormat.mid( pos, s.length() ) == s )
	    {
	      result += '%';
	      result += (*it).storeName;

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

QString KLocaleConfigTime::storeToUser(const QList<StringPair> & list,
				       const QString & storeFormat) const
{
  QString result;

  bool escaped = false;
  for ( int pos = 0; pos < storeFormat.length(); ++pos )
    {
      QChar c = storeFormat.at(pos);
      if ( escaped )
	{
	  StringPair it = StringPair::find( list, c );
	  if ( !it.userName.isEmpty() )
	    result += it.userName;
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
  QGridLayout *lay = new QGridLayout(this, 7, 2,
				     KDialog::marginHint(),
				     KDialog::spacingHint());
  lay->setAutoAdd(true);

  m_labCalendarSystem = new QLabel(this, I18N_NOOP("Calendar system:"));
  m_comboCalendarSystem = new QComboBox(false, this);
  connect(m_comboCalendarSystem, SIGNAL(activated(int)),
	  this, SLOT(slotCalendarSystemChanged(int)));
  QStringList tmpCalendars;
  tmpCalendars << QString() << QString();
  m_comboCalendarSystem->insertStringList(tmpCalendars);

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

  m_labWeekStartDay = new QLabel(this, I18N_NOOP("First day of the week:"));
  m_comboWeekStartDay = new QComboBox(false, this);
  connect (m_comboWeekStartDay, SIGNAL(activated(int)),
           this, SLOT(slotWeekStartDayChanged(int)));

  updateWeekDayNames();

  m_chDateMonthNamePossessive = new QCheckBox(this, I18N_NOOP("Use declined form of month name"));
  connect( m_chDateMonthNamePossessive, SIGNAL( clicked() ),
	     SLOT( slotDateMonthNamePossChanged() ) );

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
  KConfigGroup group(config, "Locale");

  KSimpleConfig ent(locate("locale",
			   QString::fromLatin1("l10n/%1/entry.desktop")
			   .arg(m_locale->country())), true);
  ent.setGroup("KCM Locale");

  QString str;

  str = ent.readEntry("CalendarSystem", QString::fromLatin1("gregorian"));
  group.deleteEntry("CalendarSystem", KConfigBase::Global);
  if (str != m_locale->calendarType())
    group.writeEntry("CalendarSystem", m_locale->calendarType(), KConfigBase::Persistent|KConfigBase::Global);

  str = ent.readEntry("TimeFormat", QString::fromLatin1("%H:%M:%S"));
  group.deleteEntry("TimeFormat", KConfigBase::Global);
  if (str != m_locale->timeFormat())
    group.writeEntry("TimeFormat", m_locale->timeFormat(), KConfigBase::Persistent|KConfigBase::Global);

  str = ent.readEntry("DateFormat", QString::fromLatin1("%A %d %B %Y"));
  group.deleteEntry("DateFormat", KConfigBase::Global);
  if (str != m_locale->dateFormat())
    group.writeEntry("DateFormat", m_locale->dateFormat(), KConfigBase::Persistent|KConfigBase::Global);

  str = ent.readEntry("DateFormatShort", QString::fromLatin1("%Y-%m-%d"));
  group.deleteEntry("DateFormatShort", KConfigBase::Global);
  if (str != m_locale->dateFormatShort())
    group.writeEntry("DateFormatShort",
		       m_locale->dateFormatShort(), KConfigBase::Persistent|KConfigBase::Global);

  int firstDay;
  firstDay = ent.readEntry("WeekStartDay", 1);
  group.deleteEntry("WeekStartDay",KConfigBase::Global);
  if (firstDay != m_locale->weekStartDay())
      group.writeEntry("WeekStartDay", m_locale->weekStartDay(), KConfigBase::Persistent|KConfigBase::Global);

  if ( m_locale->nounDeclension() )
  {
    bool b;
    b = ent.readEntry("DateMonthNamePossessive", false);
    group.deleteEntry("DateMonthNamePossessive", KConfigBase::Global);
    if (b != m_locale->dateMonthNamePossessive())
      group.writeEntry("DateMonthNamePossessive",
		         m_locale->dateMonthNamePossessive(), KConfigBase::Persistent|KConfigBase::Global);
  }

  group.sync();

  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfigTime::showEvent( QShowEvent *e )
{
  // This option makes sense only for languages where nouns are declined
   if ( !m_locale->nounDeclension() )
    m_chDateMonthNamePossessive->hide();
   QWidget::showEvent( e );
}

void KLocaleConfigTime::slotCalendarSystemChanged(int calendarSystem)
{
  kDebug() << "CalendarSystem: " << calendarSystem << endl;

  typedef QVector<QString> CalendarVector;
  CalendarVector calendars(4);
  calendars[0] = "gregorian";
  calendars[1] = "hijri";
  calendars[2] = "hebrew";
  calendars[3] = "jalali";

  QString calendarType;
  if( calendarSystem >= calendars.size())
	  calendarType = calendars.first();
  else
  	calendarType = calendars.at(calendarSystem);

  m_locale->setCalendar(calendarType);

  updateWeekDayNames();
  emit localeChanged();
}

void KLocaleConfigTime::slotLocaleChanged()
{
  typedef QVector<QString> CalendarVector;
  CalendarVector calendars(4);
  calendars[0] = "gregorian";
  calendars[1] = "hijri";
  calendars[2] = "hebrew";
  calendars[3] = "jalali";

  QString calendarType = m_locale->calendarType();
  int calendarSystem = 0;

  CalendarVector::iterator it = qFind(calendars.begin(), calendars.end(),
calendarType);
  if ( it != calendars.end() )
    calendarSystem = it - calendars.begin();

  kDebug() << "calSys: " << calendarSystem << ": " << calendarType << endl;
  m_comboCalendarSystem->setCurrentIndex( calendarSystem );

  //  m_edTimeFmt->setText( m_locale->timeFormat() );
  m_comboTimeFmt->setEditText( storeToUser( timeMap(),
					    m_locale->timeFormat() ) );
  // m_edDateFmt->setText( m_locale->dateFormat() );
  m_comboDateFmt->setEditText( storeToUser( dateMap(),
					    m_locale->dateFormat() ) );
  //m_edDateFmtShort->setText( m_locale->dateFormatShort() );
  m_comboDateFmtShort->setEditText( storeToUser( dateMap(),
					  m_locale->dateFormatShort() ) );
  m_comboWeekStartDay->setCurrentIndex( m_locale->weekStartDay() - 1 );

  if ( m_locale->nounDeclension() )
    m_chDateMonthNamePossessive->setChecked( m_locale->dateMonthNamePossessive() );

  kDebug(173) << "converting: " << m_locale->timeFormat() << endl;
  kDebug(173) << storeToUser(timeMap(),
			   m_locale->timeFormat()) << endl;
  kDebug(173) << userToStore(timeMap(),
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

void KLocaleConfigTime::slotWeekStartDayChanged(int firstDay) {
    kDebug(173) << k_funcinfo << "first day is now: " << firstDay << endl;
    m_locale->setWeekStartDay(m_comboWeekStartDay->currentIndex() + 1);
    emit localeChanged();
}

void KLocaleConfigTime::slotDateMonthNamePossChanged()
{
  if (m_locale->nounDeclension())
  {
    m_locale->setDateMonthNamePossessive(m_chDateMonthNamePossessive->isChecked());
    emit localeChanged();
  }
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
  str = i18nc("some reasonable time formats for the language",
	     "HH:MM:SS\n"
	     "pH:MM:SS AMPM");
  m_comboTimeFmt->insertStringList(str.split( sep));
  m_comboTimeFmt->setEditText(old);

  old = m_comboDateFmt->currentText();
  m_comboDateFmt->clear();
  str = i18nc("some reasonable date formats for the language",
	     "WEEKDAY MONTH dD YYYY\n"
	     "SHORTWEEKDAY MONTH dD YYYY");
  m_comboDateFmt->insertStringList(str.split( sep));
  m_comboDateFmt->setEditText(old);

  old = m_comboDateFmtShort->currentText();
  m_comboDateFmtShort->clear();
  str = i18nc("some reasonable short date formats for the language",
	     "YYYY-MM-DD\n"
	     "dD.mM.YYYY\n"
	     "DD.MM.YYYY");
  m_comboDateFmtShort->insertStringList(str.split( sep));
  m_comboDateFmtShort->setEditText(old);

  updateWeekDayNames();

  while ( m_comboCalendarSystem->count() < 4 )
    m_comboCalendarSystem->insertItem(QString());
  m_comboCalendarSystem->changeItem
    (ki18nc("Calendar System Gregorian", "Gregorian").toString(m_locale), 0);
  m_comboCalendarSystem->changeItem
    (ki18nc("Calendar System Hijri", "Hijri").toString(m_locale), 1);
  m_comboCalendarSystem->changeItem
    (ki18nc("Calendar System Hebrew", "Hebrew").toString(m_locale), 2);
  m_comboCalendarSystem->changeItem
    (ki18nc("Calendar System Jalali", "Jalali").toString(m_locale), 3);

  str = ki18n
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
     "</table>").toString(m_locale);
  m_labTimeFmt->setWhatsThis( str );
  m_comboTimeFmt->setWhatsThis(  str );

  QString datecodes = ki18n(
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
    "</table>").toString(m_locale);

  str = ki18n
    ( "<p>The text in this textbox will be used to format long "
      "dates. The sequences below will be replaced:</p>").toString(m_locale) + datecodes;
  m_labDateFmt->setWhatsThis( str );
  m_comboDateFmt->setWhatsThis(  str );

  str = ki18n
    ( "<p>The text in this textbox will be used to format short "
      "dates. For instance, this is used when listing files. "
      "The sequences below will be replaced:</p>").toString(m_locale) + datecodes;
  m_labDateFmtShort->setWhatsThis( str );
  m_comboDateFmtShort->setWhatsThis(  str );

  str = ki18n
    ("<p>This option determines which day will be considered as "
     "the first one of the week.</p>").toString(m_locale);
  m_comboWeekStartDay->setWhatsThis(  str );

  if ( m_locale->nounDeclension() )
  {
    str = ki18n
      ("<p>This option determines whether possessive form of month "
       "names should be used in dates.</p>").toString(m_locale);
    m_chDateMonthNamePossessive->setWhatsThis(  str );
  }
}

void KLocaleConfigTime::updateWeekDayNames()
{
  const KCalendarSystem * calendar = m_locale->calendar();

  for ( int i = 1; ; ++i )
  {
    QString str = calendar->weekDayName(i);
    bool outsideComboList = m_comboWeekStartDay->count() < i;

    if ( str.isNull() )
    {
      if ( outsideComboList )
        break;
      else
        m_comboWeekStartDay->removeItem(i - 1);
    }
        
    if ( outsideComboList )
      m_comboWeekStartDay->insertItem(str, i - 1);
    else
      m_comboWeekStartDay->changeItem(str, i - 1);
  }
}
