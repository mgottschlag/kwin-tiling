/*
 * applettab.h
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


#ifndef __applettab_impl_h__
#define __applettab_impl_h__

#include <qwidget.h>
#include "applettab.h"

class QGroupBox;
class QButtonGroup;
class QRadioButton;
class QPushButton;
class K3ListView;
class QListViewItem;

class AppletTab : public AppletTabBase
{
  Q_OBJECT

 public:
  AppletTab( QWidget *parent=0, const char* name=0 );

  void load();
  void save();
  void defaults();

  QString quickHelp() const;

 Q_SIGNALS:
  void changed();

 protected Q_SLOTS:
  void level_changed(int level);
  void trusted_selection_changed(QListViewItem *);
  void available_selection_changed(QListViewItem *);
  void add_clicked();
  void remove_clicked();

 protected:
  void updateTrusted();
  void updateAvailable();
  void updateAddRemoveButton();
  
 private:
  QStringList   available, l_available, l_trusted;
};

#endif

