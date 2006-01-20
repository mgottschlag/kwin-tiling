/*
 * localeother.h
 *
 * Copyright (c) 2001-2003 Hans Petter Bieker <bieker@kde.org>
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

#ifndef __KLOCALECONFIGOTHER_H__
#define __KLOCALECONFIGOTHER_H__

#include <qwidget.h>

class QLabel;
class QComboBox;

class KLocale;

class KLocaleConfigOther : public QWidget
{
  Q_OBJECT

public:
  KLocaleConfigOther(KLocale *locale, QWidget *parent = 0, const char *name = 0);
  virtual ~KLocaleConfigOther();

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

private Q_SLOTS:
  void slotPageSizeChanged(int i);
  void slotMeasureSystemChanged(int i);

private:
  KLocale *m_locale;

  QLabel *m_labMeasureSystem;
  QComboBox *m_combMeasureSystem;
  QLabel *m_labPageSize;
  QComboBox *m_combPageSize;
};

#endif
