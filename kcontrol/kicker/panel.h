/*
 *  panel.h
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


#ifndef __panel_h__
#define __panel_h__

#include <qwidget.h>

class QGridLayout;
class QButtonGroup;
class QRadioButton;

class PanelTab : public QWidget
{
  Q_OBJECT

 public:
  PanelTab( QWidget *parent=0, const char* name=0 );
  ~PanelTab( );

  void load();
  void save();
  void defaults();

 signals:
  void changed();
  
 protected slots:
  void position_clicked(int);
  void size_clicked(int);
  
 private:
  QGridLayout *layout;
  
  enum Position { Left = 0, Right, Top, Bottom } position; 
  QRadioButton *pos_buttons[4];
  QButtonGroup *pos_group;
  
  enum Size {Tiny=0, Normal, Large} size;
  QRadioButton *size_buttons[3];
  QButtonGroup *size_group;
};

#endif

