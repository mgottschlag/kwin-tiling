/*
 * kcmlocale.h
 *
 * Copyright (c) 1998 Matthias Hoelzer <hoelzer@physik.uni-wuerzburg.de>
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

#ifndef KCMLOCALE_H
#define KCMLOCALE_H

#include <qwidget.h>
#include <qstringlist.h>

class KAddButton;
class KLanguageButton;
class KLocale;
class KLocaleSample;

class QLabel;
class Q3ListBox;
class QPushButton;

class KLocaleConfig : public QWidget
{
  Q_OBJECT

public:
  KLocaleConfig( KLocale *_locale,
                 QWidget *parent = 0, const char *name = 0);

  void save();

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
  void languageChanged();

private Q_SLOTS:
  void loadLanguageList();
  void loadCountryList();

  void changedCountry(const QString & code);
  void readLocale(const QString &path, QString &name,
                  const QString &sub) const;

  void slotAddLanguage(const QString & id);
  void slotRemoveLanguage();
  void slotLanguageUp();
  void slotLanguageDown();
  void slotCheckButtons();

private:
  QStringList languageList() const;

  KLocale *m_locale;

  KLanguageButton *m_comboCountry;

  QLabel *m_labCountry;
  QLabel *m_labLang;

  Q3ListBox * m_languages;
  KLanguageButton * m_addLanguage;
  QPushButton * m_removeLanguage;
  QPushButton * m_upButton;
  QPushButton * m_downButton;
};

#endif
