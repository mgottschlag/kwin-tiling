/*
 * mouse.cpp
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
#include <qcheckbox.h>
#undef Below
#undef Above
#include <qslider.h>

#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kipc.h>
#include <kapp.h>

#include "mouse.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#undef Below

MouseConfig::MouseConfig (QWidget * parent, const char *name)
  : KCModule(parent, name)
{
  QBoxLayout* lay = new QVBoxLayout(this, 10);

  accel = new KIntNumInput(20, this);
  accel->setLabel(i18n("Acceleration"));
  accel->setRange(1,20,2);
  accel->setSuffix("x");
  accel->setSteps(1,20);
  lay->addWidget(accel);
  connect(accel, SIGNAL(valueChanged(int)), this, SLOT(changed()));

  thresh = new KIntNumInput(accel, 20, this);
  thresh->setLabel(i18n("Threshold"));
  thresh->setRange(1,20,2);
  thresh->setSuffix(i18n("pixels"));
  thresh->setSteps(1,20);
  lay->addWidget(thresh);
  connect(thresh, SIGNAL(valueChanged(int)), this, SLOT(changed()));

  handedBox = new QHButtonGroup(i18n("Button mapping"), this, "handed");
  rightHanded = new QRadioButton(i18n("Right handed"), handedBox, "R");
  leftHanded = new QRadioButton(i18n("Left handed"), handedBox, "L");
  connect(handedBox, SIGNAL(clicked(int)), this, SLOT(changed()));

  lay->addWidget(handedBox);

  handedEnabled = true;

  // SC/DC/AutoSelect/ChangeCursor

    singleClick = new QCheckBox(i18n("Single &click to activate"), this);
    connect(singleClick, SIGNAL(clicked()), SLOT(changed()));
    lay->addWidget(singleClick);

    cbAutoSelect = new QCheckBox(i18n("&Automatically select icons"), this);
    lay->addWidget(cbAutoSelect);
    connect(cbAutoSelect, SIGNAL(clicked()), this, SLOT(changed()));

    //----------
    QGridLayout* grid = new QGridLayout(lay, 2 /*rows*/, 3 /*cols*/ );

    int row = 0;
    slAutoSelect = new QSlider(0, 2000, 10, 0, QSlider::Horizontal, this);
    slAutoSelect->setSteps( 125, 125 );
    slAutoSelect->setTickmarks( QSlider::Below );
    slAutoSelect->setTickInterval( 250 );
    slAutoSelect->setTracking( true );
    grid->addMultiCellWidget(slAutoSelect,row,row,1,2);
    connect(slAutoSelect, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    lDelay = new QLabel(slAutoSelect, i18n("De&lay:"), this);
    lDelay->adjustSize();
    grid->addWidget(lDelay, row, 0);

    row++;
    QLabel * label = new QLabel(i18n("Small"), this);
    grid->addWidget(label,row,1);

    label = new QLabel(i18n("Large"), this);
    grid->addWidget(label,row,2, Qt::AlignRight);

    //lay->addLayout( grid );
    //----------

    cbCursor = new QCheckBox(i18n("&Change cursor shape when over an icon"), this);
    lay->addWidget(cbCursor,Qt::AlignLeft);
    connect(cbCursor, SIGNAL(clicked()), this, SLOT(changed()));

    connect( singleClick, SIGNAL( clicked() ), this, SLOT( slotClick() ) );
    connect( cbAutoSelect, SIGNAL( clicked() ), this, SLOT( slotClick() ) );

  lay->addStretch(10);
  load();
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

void MouseConfig::load()
{
  KConfig *config = new KConfig("kcminputrc");

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
      rightHanded->setEnabled(false);
      leftHanded->setEnabled(false);
      handedEnabled = false;
      break;
    case 2:
      if ( (int)map[0] == 1 && (int)map[1] == 2 )
	h = RIGHT_HANDED;
      else if ( (int)map[0] == 2 && (int)map[1] == 1 )
	h = LEFT_HANDED;
      else
	{
	  /* custom button setup: disable button remapping */
	  rightHanded->setEnabled(false);
	  leftHanded->setEnabled(false);
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
	  rightHanded->setEnabled(false);
	  leftHanded->setEnabled(false);
	  handedEnabled = false;
	}
      break;
    default:
      /* custom setup with > 3 buttons: disable button remapping */
      rightHanded->setEnabled(false);
      leftHanded->setEnabled(false);
      handedEnabled = false;
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

  setAccel(accelRate);
  setThreshold(thresholdMove);
  setHandedness(handed);

  // SC/DC/AutoSelect/ChangeCursor
  config->setGroup(QString::fromLatin1("KDE"));
  bool b = config->readBoolEntry(QString::fromLatin1("SingleClick"), DEFAULT_SINGLECLICK);
  singleClick->setChecked(b);
  int  autoSelect = config->readNumEntry("AutoSelectDelay", DEFAULT_AUTOSELECTDELAY);
  if ( autoSelect < 0 ) autoSelect = 0;
  bool changeCursor = config->readBoolEntry("ChangeCursor", DEFAULT_CHANGECURSOR);

  cbAutoSelect->setChecked( autoSelect > 0 );
  slAutoSelect->setValue( autoSelect );
  cbCursor->setChecked( changeCursor );
  slotClick();

  delete config;
}

void MouseConfig::save()
{
  KConfig *config = new KConfig("kcminputrc");

  accelRate = getAccel();
  thresholdMove = getThreshold();
  handed = getHandedness();

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

  config->setGroup(QString::fromLatin1("KDE"));
  config->writeEntry(QString::fromLatin1("SingleClick"), singleClick->isChecked(), true, true);
  config->writeEntry( "AutoSelectDelay", cbAutoSelect->isChecked()?slAutoSelect->value():-1, true, true );
  config->writeEntry( "ChangeCursor", cbCursor->isChecked(), true, true );

  config->sync();

  KIPC::sendMessageAll(KIPC::SettingsChanged, KApplication::SETTINGS_MOUSE);

  delete config;
}


void MouseConfig::defaults()
{
    setThreshold(2);
    setAccel(2);
    setHandedness(RIGHT_HANDED);
    singleClick->setChecked( DEFAULT_SINGLECLICK );
    cbAutoSelect->setChecked( DEFAULT_AUTOSELECTDELAY != -1 );
    slAutoSelect->setValue( DEFAULT_AUTOSELECTDELAY == -1 ? 50 : DEFAULT_AUTOSELECTDELAY );
    cbCursor->setChecked( DEFAULT_CHANGECURSOR );
    slotClick();
}

void MouseConfig::slotClick()
{
  // Autoselect has a meaning only in single-click mode
  cbAutoSelect->setEnabled(singleClick->isChecked());
  // Delay has a meaning only for autoselect
  bool bDelay = cbAutoSelect->isChecked() && singleClick->isChecked();
  slAutoSelect->setEnabled( bDelay );
  lDelay->setEnabled( bDelay );
}

void MouseConfig::changed()
{
  emit KCModule::changed(true);
}


#include "mouse.moc"
