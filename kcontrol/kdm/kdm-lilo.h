/* This file is part of the KDE Display Manager Configuration package

    Copyright (C) 1999 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#ifndef __KDMLILO_H__
#define __KDMLILO_H__


#include <qcheckbox.h>
#include <qlineedit.h>


#include <kcmodule.h>


class KDMLiloWidget : public KCModule
{
  Q_OBJECT

public:
	
  KDMLiloWidget(QWidget *parent=0, const char *name=0);

  void load();
  void save();
  void defaults();
  
private slots:
	
  void liloClicked();
  void changed();

 
private:

  QCheckBox *useLilo;
  QLineEdit *liloCmd, *liloMap;

};


#endif


