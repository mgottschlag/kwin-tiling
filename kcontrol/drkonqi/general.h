/**
 * general.h
 *
 * Copyright (c) 2000 Timo Hummel <timo.hummel@sap.com>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __general_h__
#define __general_h__

#include <kcmodule.h>
#include <kconfig.h>
#include <qcombobox.h>
#include <qcheckbox.h>

class KDrKonqiGeneral : public KCModule
{
  Q_OBJECT

public:
  KDrKonqiGeneral (KConfig *config, QString group, QWidget *parent=0, const char *name=0);

  virtual void load();
  virtual void save();
  virtual void defaults();
  
protected slots:
  void changed();
  
      
private:
  KConfig *g_pConfig;
  QString groupname;

  QCheckBox *cbEnable;
  QCheckBox *cbAllowBugReporting;
  QComboBox *cmPresets;
};

#endif
