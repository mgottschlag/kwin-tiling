/*
 *  lnftab.h
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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
 */


#ifndef __lnftab_h__
#define __lnftab_h__

#include <qwidget.h>

class QGridLayout;
class QGroupBox;
class QPushButton;
class QCheckBox;
class QLabel;
class QString;
class KLineEdit;
class KIntNumInput;

class LnFTab : public QWidget
{
  Q_OBJECT

 public:
  LnFTab( QWidget *parent=0, const char* name=0 );

  void load();
  void save();
  void defaults();

  QString quickHelp() const;

 signals:
  void changed();

 protected slots:
  void use_theme_clicked();
  void browse_theme();
  void hide_clicked();
  void autohide_clicked();
  void hide_changed(int);
  void autohide_changed(int);

 private:
  QGridLayout *layout;

  // hide animation group
  QGroupBox    *hide_group;
  QCheckBox    *hide_cb;
  KIntNumInput *hide_input;

  // auto-hide animation group
  QGroupBox    *autohide_group;
  QCheckBox    *autohide_cb;
  KIntNumInput *autohide_input;

  // theme group
  QGroupBox    *theme_group;
  QCheckBox    *use_theme_cb;
  QLabel       *theme_label;
  KLineEdit    *theme_input;
  QPushButton  *browse_button;
  bool         use_theme;
  QString      theme;
  QPixmap      theme_preview;

  // misc group
  QGroupBox    *misc_group;
  QCheckBox    *fade_out_cb;
};

#endif

