/**
 * socks.h
 *
 * Copyright (c) 2001 George Staikos <staikos@kde.org>
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

#ifndef _SOCKS_H
#define _SOCKS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kcmodule.h>

class QVButtonGroup;

class KSocksConfig : public KCModule
{
  Q_OBJECT
public:
  KSocksConfig(QWidget *parent = 0L, const char *name = 0L);
  virtual ~KSocksConfig();

  void load();
  void save();
  void defaults();

  int buttons();
  QString quickHelp() const;

public slots:
  void configChanged();

private slots:
  void enableChanged();
  void methodChanged(int id);
  void testClicked();
  void chooseCustomLib();
  void customPathChanged(const QString&);

private:

  bool _socksEnabled;
  int _useWhat;

  QCheckBox *_c_enableSocks;
  QRadioButton *_c_detect, *_c_NEC, *_c_Dante, *_c_custom;
  QPushButton *_c_test;
  QLabel *_c_customLabel;
  KLineEdit *_c_customPath;
  QPushButton *_c_customChoose;
  QVButtonGroup *bg;

  KConfig *config;
};

#endif
