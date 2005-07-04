/*
 * localenum.h
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


#ifndef __KLOCALECONFIGNUM_H__
#define __KLOCALECONFIGNUM_H__

#include <qwidget.h>

class QCheckBox;
class QComboBox;
class QLineEdit;

class KLocale;
class KLanguageCombo;

class KLocaleConfigNumber : public QWidget
{
  Q_OBJECT

public:
  KLocaleConfigNumber( KLocale *_locale,
		       QWidget *parent=0, const char *name=0);
  virtual ~KLocaleConfigNumber( );

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
  // Numbers
  void slotMonPosSignChanged(const QString &t);
  void slotMonNegSignChanged(const QString &t);
  void slotDecSymChanged(const QString &t);
  void slotThoSepChanged(const QString &t);

private:
  KLocale *m_locale;

  // Numbers
  QLabel *m_labDecSym;
  QLineEdit *m_edDecSym;
  QLabel *m_labThoSep;
  QLineEdit *m_edThoSep;
  QLabel *m_labMonPosSign;
  QLineEdit *m_edMonPosSign;
  QLabel *m_labMonNegSign;
  QLineEdit *m_edMonNegSign;
};

#endif
