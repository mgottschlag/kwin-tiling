/*
  toplevel.h - A KControl Application

  written 1998 by Matthias Hoelzer

  Copyright 1998 Matthias Hoelzer.
  Copyright 1999-2000 Hans Petter Bieker <bieker@kde.org>.
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */

#ifndef __TOPLEVEL_H__
#define __TOPLEVEL_H__

#include <kcmodule.h>

class QTabWidget;
class QGroupBox;

class KLocale;
class KLocaleConfig;
class KLocaleConfigMoney;
class KLocaleConfigNumber;
class KLocaleConfigTime;
class KLocaleSample;

class KLocaleApplication : public KCModule
{
  Q_OBJECT

public:
  KLocaleApplication(QWidget *parent, const char *name);
  virtual ~KLocaleApplication();

  virtual void load();
  virtual void save();
  virtual void defaults();
  virtual QString quickHelp() const;

signals:
  void languageChanged();
  void localeChanged();

public slots:
  /**
   * Retranslates the current widget.
   */
  void slotTranslate();
  void slotChanged();

private:
  KLocale *m_locale;

  QTabWidget          *m_tab;
  KLocaleConfig       *m_localemain;
  KLocaleConfigNumber *m_localenum;
  KLocaleConfigMoney  *m_localemon;
  KLocaleConfigTime   *m_localetime;

  QGroupBox           *m_gbox;
  KLocaleSample       *m_sample;
};

#endif
