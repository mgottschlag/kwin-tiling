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

#include <kcontrol.h>

class QCheckBox;
class QComboBox;
class QLineEdit;

class KLanguageCombo;
class KLocaleSample;

class KLocaleConfigTime : public KConfigWidget
{
  Q_OBJECT

public:
  KLocaleConfigTime( QWidget *parent=0, const char *name=0);
  ~KLocaleConfigTime( );

public slots:
  void loadSettings();
  void applySettings();
  void defaultSettings();
  void reset();
  void updateSample();

private slots:
  // Time & dates
  void slotTimeFmtChanged(const QString &t);
  void slotDateFmtChanged(const QString &t);
  void slotDateFmtShortChanged(const QString &t);

private:
  KLocaleSample *sample;

  // Time & dates
  QLineEdit *edTimeFmt;
  QLineEdit *edDateFmt;
  QLineEdit *edDateFmtShort;
};

#endif
