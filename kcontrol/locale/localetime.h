/*
 * localetime.h
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


#ifndef __KLOCALECONFIGTIME_H__
#define __KLOCALECONFIGTIME_H__

#include <qwidget.h>

#include <qmap.h>

class QCheckBox;
class QComboBox;
//class QLineEdit;

class KLocale;
class KLanguageCombo;

class StringPair;

class KLocaleConfigTime : public QWidget
{
  Q_OBJECT

public:
  KLocaleConfigTime( KLocale *_locale, QWidget *parent=0, const char *name=0);
  virtual ~KLocaleConfigTime( );

  void save();

public slots:
  /**
   * Loads all settings from the current locale into the current widget.
   */
  void slotLocaleChanged();
  /**
   * Retranslate all objects owned by this object using the current locale.
   */
  void slotTranslate();

signals:
  void localeChanged();

private slots:
  // Time & dates
  void slotTimeFmtChanged(const QString &t);
  void slotDateFmtChanged(const QString &t);
  void slotDateFmtShortChanged(const QString &t);
  void slotWeekStartsMondayChanged();

private:
  QValueList<StringPair> timeMap() const;
  QValueList<StringPair> dateMap() const;

  QString storeToUser(const QValueList<StringPair> & map,
		      const QString & storeFormat) const;
  QString userToStore(const QValueList<StringPair> & map,
		      const QString & userFormat) const;
  StringPair buildStringPair(const QChar &storeName, const QString &userName) const;
		      

  KLocale *m_locale;

  // Time & dates
  QLabel *m_labTimeFmt;
  //QLineEdit *m_edTimeFmt;
  QComboBox *m_comboTimeFmt;
  QLabel *m_labDateFmt;
  //QLineEdit *m_edDateFmt;
  QComboBox * m_comboDateFmt;
  QLabel *m_labDateFmtShort;
  //QLineEdit *m_edDateFmtShort;
  QComboBox * m_comboDateFmtShort;
  QCheckBox *m_chWeekStartsMonday;
};

#endif
