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
#include <qpixmap.h>

class QGridLayout;
class QButtonGroup;
class QGroupBox;
class QRadioButton;
class QCheckBox;
class QLabel;
class KLineEdit;
class KIntNumInput;

class HBPreview : public QWidget
{
  Q_OBJECT;
  
 public:
  HBPreview(QWidget *parent=0, const char* name=0);

  void setEnabled(bool v) { _enabled = v; repaint(); }
  void setHighlight(bool v) { _highlight = v; repaint(); }
  void setWidth(int v) { _width = v; repaint(); }

 protected:
  void paintEvent(QPaintEvent*);

 private:
  bool       _enabled, _highlight;
  QPixmap    _icon;
  int        _width;
};

class PanelTab : public QWidget
{
  Q_OBJECT;

 public:
  PanelTab( QWidget *parent=0, const char* name=0 );

  void load();
  void save();
  void defaults();

 signals:
  void changed();

 protected slots:
  void position_clicked(int);
  void size_clicked(int);
  void show_hbs_clicked();
  void hbs_input_changed(int);
  void ah_clicked();
  void ah_input_changed(int);
  void ta_input_changed(const QString&);

 private:
  QGridLayout *layout;

  // position group
  QButtonGroup *pos_group;
  QRadioButton *pos_buttons[4];
  enum Position { Left = 0, Right, Top, Bottom } position; 
  
  // size group
  QButtonGroup *size_group;
  QRadioButton *size_buttons[3];
  enum Size { Tiny=0, Normal, Large } size;

  // hide button group
  QGroupBox    *hb_group;
  KIntNumInput *hb_input;
  QCheckBox    *show_hbs, *highlight_hbs;
  HBPreview    *hb_preview;

  // auto-hide group
  QGroupBox    *ah_group;
  QCheckBox    *ah_cb;
  KIntNumInput *ah_input;

  // misc group
  QGroupBox    *misc_group;
  KLineEdit    *ta_input;
  QLabel       *ta_label;
};

#endif

