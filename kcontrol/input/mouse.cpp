/*
 * mouse.cpp
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Layout management, enhancements:
 * Copyright (c) 1999 Dirk A. Mueller <dmuell@gmx.net>
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

#include <iostream.h> 

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <qfileinfo.h> 
#include <qstring.h>
#include <qhbuttongroup.h>
#include <qmessagebox.h>
#include <qlayout.h>

#include <klocale.h>
#include <kconfig.h>

#include "mouse.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "geom.h"

MouseConfig::~MouseConfig ()
{
    if (GUI) {
        delete accel;
        delete thresh;
        delete leftHanded;
        delete rightHanded;
        delete handedBox;
    }
}

MouseConfig::MouseConfig (QWidget * parent, const char *name, bool init)
  : KConfigWidget (parent, name)
{
    GUI = !init;

  if (GUI) {
      QBoxLayout* lay = new QVBoxLayout(this, 10);
      
      accel = new KIntNumInput(i18n("Acceleration"), 1,20,2,20, "x", 
                               10, true, this);
      accel->setSteps(1,20);
      lay->addWidget(accel);
      
      thresh = new KIntNumInput(i18n("Threshold"), 1,20,2,20,
                                i18n("pixels"), 10, true, this);
      thresh->setSteps(1,20);
      lay->addWidget(thresh);
      
      handedBox = new QHButtonGroup(i18n("Button mapping"), this, "handed");
      rightHanded = new QRadioButton(i18n("Right handed"), handedBox, "R");
      leftHanded = new QRadioButton(i18n("Left handed"), handedBox, "L");

      lay->addWidget(handedBox);
  }

  handedEnabled = true;
  config = kapp->config();

  GetSettings();
}

int MouseConfig::getAccel()
{
  return accel->value();
}

void MouseConfig::setAccel(int val)
{
  accel->setValue(val);
}

int MouseConfig::getThreshold()
{
  return thresh->value();
}

void MouseConfig::setThreshold(int val)
{
  thresh->setValue(val);
}


int MouseConfig::getHandedness()
{
  if (rightHanded->isChecked())
    return RIGHT_HANDED;
  else
    return LEFT_HANDED;
}

void MouseConfig::setHandedness(int val)
{
  rightHanded->setChecked(false);
  leftHanded->setChecked(false);
  if (val == RIGHT_HANDED)
    rightHanded->setChecked(true);
  else
    leftHanded->setChecked(true);
}

void MouseConfig::GetSettings( void )
{
  int accel_num, accel_den, threshold;
  XGetPointerControl( kapp->getDisplay(), 
		      &accel_num, &accel_den, &threshold );
  accel_num /= accel_den;   // integer acceleration only

  // get settings from X server
  int h = RIGHT_HANDED;
  unsigned char map[5];
  num_buttons = XGetPointerMapping(kapp->getDisplay(), map, 5);
      
  switch (num_buttons)
    {
    case 1:
      /* disable button remapping */
      if (GUI)
	{
	  rightHanded->setEnabled(false);
	  leftHanded->setEnabled(false);
	  handedEnabled = false;
	}
      break;
    case 2:
      if ( (int)map[0] == 1 && (int)map[1] == 2 )
	h = RIGHT_HANDED;
      else if ( (int)map[0] == 2 && (int)map[1] == 1 )
	h = LEFT_HANDED;
      else
	{
	  /* custom button setup: disable button remapping */
	  if (GUI)
	    {
	      rightHanded->setEnabled(false);
	      leftHanded->setEnabled(false);
	    }
	}
      break;
    case 3:
      middle_button = (int)map[1];
      if ( (int)map[0] == 1 && (int)map[2] == 3 )
	h = RIGHT_HANDED;
      else if ( (int)map[0] == 3 && (int)map[2] == 1 )
	h = LEFT_HANDED;
      else
	{
	  /* custom button setup: disable button remapping */
	  if (GUI)
	    {
	      rightHanded->setEnabled(false);
	      leftHanded->setEnabled(false);
	      handedEnabled = false;
	    }
	}
      break;
    default:
      /* custom setup with > 3 buttons: disable button remapping */
      if (GUI)
	{
	  rightHanded->setEnabled(false);
	  leftHanded->setEnabled(false);
	  handedEnabled = false;
	}
      break;
    }

  config->setGroup("Mouse");
  int a = config->readNumEntry("Acceleration",-1);
  if (a == -1)
    accelRate = accel_num;
  else
    accelRate = a;

  int t = config->readNumEntry("Threshold",-1);
  if (t == -1)
    thresholdMove = threshold;
  else
    thresholdMove = t;

  QString key = config->readEntry("MouseButtonMapping");
  if (key == "RightHanded")
    handed = RIGHT_HANDED;
  else if (key == "LeftHanded")
    handed = LEFT_HANDED;
  else if (key == NULL)
    handed = h;

  // the GUI should always show the real values
  if (GUI)
    {
      setAccel(accel_num);
      setThreshold(threshold);
      setHandedness(h);
    }
}

void MouseConfig::saveParams( void )
{
  if (GUI) {
      accelRate = getAccel();
      thresholdMove = getThreshold();
      handed = getHandedness();
  }
  
  XChangePointerControl( kapp->getDisplay(),
                         true, true, accelRate, 1, thresholdMove);

  
  unsigned char map[5];
  int remap=1;
  if (handedEnabled) {
      switch (num_buttons) {
      case 1:
          map[0] = (unsigned char) 1;
          break;
      case 2:
          if (handed == RIGHT_HANDED) {
              map[0] = (unsigned char) 1;
              map[1] = (unsigned char) 3;
          }
          else {
              map[0] = (unsigned char) 3;
              map[1] = (unsigned char) 1;
          }
          break;
      case 3:
          if (handed == RIGHT_HANDED) {
              map[0] = (unsigned char) 1;
              map[1] = (unsigned char) middle_button;
              map[2] = (unsigned char) 3;
          }
          else {
              map[0] = (unsigned char) 3;
              map[1] = (unsigned char) middle_button;
              map[2] = (unsigned char) 1;
          }
          break;
      case 5:  
          // Intellimouse case, where buttons 1-3 are left, middle, and
          // right, and 4-5 are up/down
          if (handed == RIGHT_HANDED) {
              map[0] = (unsigned char) 1;
              map[1] = (unsigned char) 2;
              map[2] = (unsigned char) 3;
              map[3] = (unsigned char) 4;
              map[4] = (unsigned char) 5;
          }
          else {
              map[0] = (unsigned char) 3;
              map[1] = (unsigned char) 2;
              map[2] = (unsigned char) 1;
              map[3] = (unsigned char) 4;
              map[4] = (unsigned char) 5;
          }
          break;
      default: {
              //catch-all for mice with four or more than five buttons
              //Without this, XSetPointerMapping is called with a undefined value
              //for map
              remap=0;  //don't remap
          }
          break;
      }
      int retval;
      if (remap)
          while ((retval=XSetPointerMapping(kapp->getDisplay(), map,
                                            num_buttons)) == MappingBusy)
              /* keep trying until the pointer is free */
          { };
  }
  
  config->setGroup("Mouse");
  config->writeEntry("Acceleration",accelRate);
  config->writeEntry("Threshold",thresholdMove);
  if (handed == RIGHT_HANDED)
      config->writeEntry("MouseButtonMapping",QString("RightHanded"));
  else
      config->writeEntry("MouseButtonMapping",QString("LeftHanded"));
  
  config->sync();
}

void MouseConfig::loadSettings()
{
    GetSettings();
}

void MouseConfig::applySettings()
{
    saveParams();
}

void MouseConfig::defaultSettings()
{
    setThreshold(2);
    setAccel(2);
    setHandedness(RIGHT_HANDED);
}


#include "mouse.moc"
