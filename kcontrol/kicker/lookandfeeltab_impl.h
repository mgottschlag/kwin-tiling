/*
 *  lookandfeeltab.h
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2000 Aaron J. Seigo <aseigo@olympusproject.org>
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


#ifndef __lookandfeeltab_h__
#define __lookandfeeltab_h__

#include "lookandfeeltab.h"

class QLabel;
class QStringList;

class LookAndFeelTab : public LookAndFeelTabBase
{
  Q_OBJECT

 public:
  LookAndFeelTab( QWidget *parent=0, const char* name=0 );

  void load();
  void save();
  void defaults();

  QString quickHelp() const;

 signals:
  void changed();

 protected:
  void fillTileCombos();
  int findComboEntry(QComboBox* combo, const QString& searchFor);
 
 protected slots:
  void browse_theme(const QString&);

 private:
  QString theme;
  QPixmap theme_preview;
};

#endif

