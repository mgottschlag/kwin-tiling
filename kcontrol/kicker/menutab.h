/*
 *  menutab.h
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


#ifndef __menutab_h__
#define __menutab_h__

#include <qwidget.h>

class QGridLayout;
class QGroupBox;
class QCheckBox;
class KIntNumInput;

class MenuTab : public QWidget
{
  Q_OBJECT

 public:
  MenuTab( QWidget *parent=0, const char* name=0 );

  void load();
  void save();
  void defaults();

  QString quickHelp() const;

 signals:
  void changed();

 protected slots:
  void cache_time_changed(int);
  void max_entries_changed(int);
  void clear_cache_clicked();

 private:
  QGridLayout *layout;

  // general group
  QGroupBox    *general_group;
  QCheckBox    *clear_cache_cb;
  KIntNumInput *cache_time_input;

  //browser menu group
  QGroupBox    *browser_group;
  QCheckBox    *show_hidden_cb;
  KIntNumInput *max_entries_input;

  // kmenu group
  QGroupBox    *kmenu_group;
  QCheckBox    *merge_cb, *show_bookmarks_cb, *show_recent_cb, *show_qb_cb;
};

#endif

