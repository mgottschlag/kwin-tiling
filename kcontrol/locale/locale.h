/*
 * locale.h
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
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


#ifndef __KLOCALECONFIG_H__
#define __KLOCALECONFIG_H__


#include <kcontrol.h>

class QLabel;
class KLocale;
class KLanguageCombo;
class KLocaleSample;

class KLocaleConfig : public KConfigWidget
{
  Q_OBJECT

public:
  KLocaleConfig( QWidget *parent=0, const char *name=0);
  ~KLocaleConfig( );

  void loadLocaleList(KLanguageCombo *combo, const QString &sub, const QStringList &first);

public slots:
  void loadSettings();
  void applySettings();
  void defaultSettings();
  void updateSample();
  void reTranslateLists();
    
private:
  KLanguageCombo *comboCountry, *comboLang, *comboNumber, *comboMoney, *comboDate;

  // samples for how things will display w/selected locale
  QLabel *textSample;
  QLabel *numberSample;
  QLabel *moneySample;
  QLabel *timeSample;
  QLabel *dateSample;

  bool changedFlag;
  KLocale *locale;
  KLocaleSample *sample;

private slots:
  void changedCountry(int);
  void changedLanguage(int);
  void changedNumber(int);
  void changedMoney(int);
  void changedTime(int);
  void readLocale(const QString &path, QString &name, const QString &sub) const;
};

#endif
