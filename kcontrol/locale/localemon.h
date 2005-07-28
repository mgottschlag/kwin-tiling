/*
 * localemon.h
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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef __KLOCALECONFIGMON_H__
#define __KLOCALECONFIGMON_H__

#include <qwidget.h>
//Added by qt3to4:
#include <QLabel>

class QCheckBox;
class QComboBox;
class QLineEdit;

class KIntNumInput;
class KLocale;
class KLanguageCombo;

class KLocaleConfigMoney : public QWidget
{
  Q_OBJECT

public:
  KLocaleConfigMoney(KLocale *locale, QWidget *parent = 0, const char *name = 0);
  virtual ~KLocaleConfigMoney();

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
  // Money
  void slotMonCurSymChanged(const QString &t);
  void slotMonDecSymChanged(const QString &t);
  void slotMonThoSepChanged(const QString &t);
  void slotMonFraDigChanged(int value);
  void slotMonPosPreCurSymChanged();
  void slotMonNegPreCurSymChanged();
  void slotMonPosMonSignPosChanged(int i);
  void slotMonNegMonSignPosChanged(int i);

private:
  KLocale *m_locale;

  // Money
  QLabel *m_labMonCurSym;
  QLineEdit *m_edMonCurSym;
  QLabel *m_labMonDecSym;
  QLineEdit *m_edMonDecSym;
  QLabel *m_labMonThoSep;
  QLineEdit *m_edMonThoSep;
  QLabel *m_labMonFraDig;
  KIntNumInput * m_inMonFraDig;

  QCheckBox *m_chMonPosPreCurSym;
  QCheckBox *m_chMonNegPreCurSym;
  QLabel *m_labMonPosMonSignPos;
  QComboBox *m_cmbMonPosMonSignPos;
  QLabel *m_labMonNegMonSignPos;
  QComboBox *m_cmbMonNegMonSignPos;
};

#endif
