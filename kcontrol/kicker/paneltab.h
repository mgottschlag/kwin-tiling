/*
 *  paneltab.h
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2000 Preston Brown <pbrown@kde.org>
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


#ifndef __paneltab_h__
#define __paneltab_h__

#include <qwidget.h>

class QGridLayout;
class QButtonGroup;
class QGroupBox;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QLabel;
class QString;
class QSlider;
class KLineEdit;

class PanelTab : public QWidget
{
  Q_OBJECT

 public:
  PanelTab( QWidget *parent=0, const char* name=0 );
  ~PanelTab( );

  void load();
  void save();
  void defaults();

  QString quickHelp();

 signals:
  void changed();

 protected slots:
  void position_clicked(int);
  void size_clicked(int);
  void show_hbs_clicked();
  void highlight_hbs_clicked();
  void hbs_size_changed(int);
  void use_theme_clicked();
  void browse_theme();

 private:
  QGridLayout *layout;

  // position group
  QButtonGroup *pos_group;
  QRadioButton *pos_buttons[4];
  enum Position { Left = 0, Right, Top, Bottom } position; 
  
  // size group
  QButtonGroup *size_group;
  QCheckBox *mergeCB;
  QRadioButton *size_buttons[3];
  enum Size {Tiny=0, Normal, Large} size;

  // hide button group
  QGroupBox    *hb_group;
  QSlider      *hb_size;
  QLabel       *hb_size_label;
  QCheckBox    *show_hbs, *highlight_hbs;
  bool showHBs, highlightHBs;
  int HBwidth;

  // theme group
  QGroupBox    *theme_group;
  QCheckBox    *use_theme_cb;
  QLabel       *theme_label;
  KLineEdit    *theme_input;
  QPushButton  *browse_button;
  bool         use_theme;
  QString      theme;
  QPixmap      theme_preview;
};

#endif

