/*
 * localetime.h
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

#ifndef __KLOCALECONFIGTIME_H__
#define __KLOCALECONFIGTIME_H__

#include <QWidget>

#include <QMap>

class QCheckBox;
class QComboBox;

class KLocale;
class KLanguageCombo;

class StringPair;

class KLocaleConfigTime : public QWidget
{
  Q_OBJECT

public:
  KLocaleConfigTime( KLocale *_locale, QWidget *parent=0);
  virtual ~KLocaleConfigTime( );

  void save();

protected:
  void showEvent( QShowEvent *e );

public Q_SLOTS:
  /**
   * Loads all settings from the current locale into the current widget.
   */
  void slotLocaleChanged();
  /**
   * Retranslate all objects owned by this object using the current locale.
   */
  void slotTranslate();

Q_SIGNALS:
  void localeChanged();

private Q_SLOTS:
  // Time & dates
  void slotTimeFmtChanged(const QString &t);
  void slotDateFmtChanged(const QString &t);
  void slotDateFmtShortChanged(const QString &t);
  void slotWeekStartDayChanged(int firstDay);
  void slotDateMonthNamePossChanged();
  void slotCalendarSystemChanged(int calendarSystem);

private:
  void updateWeekDayNames();

  QList<StringPair> timeMap() const;
  QList<StringPair> dateMap() const;

  QString storeToUser(const QList<StringPair> & map,
		      const QString & storeFormat) const;
  QString userToStore(const QList<StringPair> & map,
		      const QString & userFormat) const;
  StringPair buildStringPair(const QChar &storeName, const QString &userName) const;

  KLocale *m_locale;

  // Time & dates
  QLabel *m_labTimeFmt;
  QComboBox *m_comboTimeFmt;
  QLabel *m_labDateFmt;
  QComboBox * m_comboDateFmt;
  QLabel *m_labDateFmtShort;
  QComboBox * m_comboDateFmtShort;
  QLabel * m_labWeekStartDay;
  QComboBox * m_comboWeekStartDay;
  QCheckBox *m_chDateMonthNamePossessive;
  QLabel * m_labCalendarSystem;
  QComboBox * m_comboCalendarSystem;
};

#endif
