/*
 * mouse.h
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Layout management, enhancements:
 * Copyright (c) 1999 Dirk A. Mueller <dmuell@gmx.net>
 *
 * SC/DC/AutoSelect/ChangeCursor:
 * Copyright (c) 2000 David Faure <faure@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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


#ifndef __MOUSECONFIG_H__
#define __MOUSECONFIG_H__

#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

#include <kapp.h>
#include <knuminput.h>
#include <kglobalsettings.h>

#include <kcmodule.h>

#define RIGHT_HANDED 0
#define LEFT_HANDED  1

class QCheckBox;
class QSlider;

class MouseConfig : public KCModule
{
  Q_OBJECT
public:
  MouseConfig(QWidget *parent=0, const char* name=0);

  void save( void );
  void load();
  void defaults();

private slots:

  void slotClick();
  void changed();

private:

  int getAccel();
  int getThreshold();
  int getHandedness();

  void setAccel(int);
  void setThreshold(int);
  void setHandedness(int);

  KIntNumInput *accel;
  KIntNumInput *thresh;

  QButtonGroup *handedBox;
  QRadioButton *leftHanded, *rightHanded;
  int num_buttons;
  int middle_button;
  bool handedEnabled;

  int accelRate, thresholdMove, handed;

  QCheckBox *singleClick;
  QCheckBox *cbAutoSelect;
  QLabel *lDelay;
  QSlider *slAutoSelect;
  QCheckBox *cbCursor;
};

#endif

