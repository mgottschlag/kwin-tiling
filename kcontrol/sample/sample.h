/**
 * sample.h
 *
 * Copyright (c) 2000 Matthias Elter <elter@kde.org>
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

#ifndef __sample_h__
#define __sample_h__

#include <kcmodule.h>

class QRadioButton;
class QButtonGroup;

class KSampleConfig : public KCModule
{
  Q_OBJECT

public:
  KSampleConfig(QWidget *parent = 0L, const char *name = 0L);
  virtual ~KSampleConfig();
  
  void load();
  void save();
  void defaults();
  
  int buttons();
  
protected slots:
  void configChanged();
      
private:
 QButtonGroup *bGrp;
 QRadioButton *button1, *button2, *button3;

};

#endif
