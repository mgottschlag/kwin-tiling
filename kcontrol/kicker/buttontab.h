/*
 *  buttontab.h
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


#ifndef __buttontab_h__
#define __buttontab_h__

#include <qwidget.h>
#include <qstringlist.h>

class QGridLayout;
class QGroupBox;
class KComboBox;
class QLabel;
class QCheckBox;

class ButtonTab : public QWidget
{
  Q_OBJECT

 public:
  ButtonTab( QWidget *parent=0, const char* name=0 );

  void load();
  void save();
  void defaults();

  QString quickHelp();

 signals:
  void changed();

 protected:
  void fill_tile_input();
  QStringList queryAvailableTiles();

 protected slots:
  void tiles_clicked();
  void kmenu_clicked();
  void kmenu_changed(const QString&);
  void url_clicked();
  void url_changed(const QString&);
  void browser_clicked();
  void browser_changed(const QString&);
  void exe_clicked();
  void exe_changed(const QString&);
  void drawer_clicked();
  void drawer_changed(const QString&);

 private:
  QGridLayout *layout;
  QStringList tiles;

  // general group
  QGroupBox    *general_group;
  QCheckBox    *tiles_cb;

  // kmenu button-tiles group
  QGroupBox    *kmenu_group;
  QCheckBox    *kmenu_cb;
  QLabel       *kmenu_label;
  KComboBox    *kmenu_input;

  // url button-tiles group
  QGroupBox    *url_group;
  QCheckBox    *url_cb;
  QLabel       *url_label;
  KComboBox    *url_input;

  // browser button-tiles group
  QGroupBox    *browser_group;
  QCheckBox    *browser_cb;
  QLabel       *browser_label;
  KComboBox    *browser_input;

  // exe button-tiles group
  QGroupBox    *exe_group;
  QCheckBox    *exe_cb;
  QLabel       *exe_label;
  KComboBox    *exe_input;

  // drawer button-tiles group
  QGroupBox    *drawer_group;
  QCheckBox    *drawer_cb;
  QLabel       *drawer_label;
  KComboBox    *drawer_input;
};

#endif

