/*
  Copyright (c) 2002 Laurent Montel <lmontel@mandrakesoft.com>

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

#ifndef __fileshare_h__
#define __fileshare_h__

#include "kcmodule.h"
class QRadioButton;
class QLabel;
class KFileShareConfig  : public KCModule
{
  Q_OBJECT

 public:
  KFileShareConfig(QWidget *parent, const char *name, const QStringList &);

  void load();
  void save();
  void defaults();
  QString quickHelp() const;

 protected slots:
  void configChanged() { emit changed( true ); };

 protected:
    QRadioButton *noSharing;
    QRadioButton *sharing;
    QLabel * info;
};

#endif
