/*
 * bell.cpp
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
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
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <qfileinfo.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qtabwidget.h>
#include <qtabbar.h>

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>

#include "bell.h"

#include <X11/Xlib.h>

#include "geom.h"


KBellModule::KBellModule(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  QVBoxLayout *layout = new QVBoxLayout(this, 10);

  // args: label, min, max, step, initial, units
  volume = new KIntNumInput(50, this);
  volume->setLabel(i18n("Volume:"));
  volume->setRange(0, 100, 5);
  volume->setSuffix("%");
  volume->setSteps(5,25);
  layout->addWidget(volume);

  pitch = new KIntNumInput(volume, 800, this);
  pitch->setLabel(i18n("Pitch:"));
  pitch->setRange(0, 2000, 20);
  pitch->setSuffix(i18n("Hz"));
  pitch->setSteps(40,200);
  layout->addWidget(pitch);

  duration = new KIntNumInput(pitch, 100, this);
  duration->setLabel(i18n("Duration:"));
  duration->setRange(0, 1000, 50);
  duration->setSuffix(i18n("ms"));
  duration->setSteps(20,100);
  layout->addWidget(duration);

  QFrame *hLine = new QFrame(this);
  hLine->setFrameStyle(QFrame::Sunken|QFrame::HLine);

  layout->addWidget(hLine);

  test = new QPushButton(i18n("&Test"), this, "test");
  layout->addWidget(test, 0, AlignLeft);
  connect( test, SIGNAL(clicked()), SLOT(ringBell()));

  layout->addStretch(5);

  layout->activate();

  config = new KConfig("kcmbellrc");

  GetSettings();

  // watch for changes
  connect(volume, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));
  connect(pitch, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));
  connect(duration, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));
}


// set the slider and the LCD to 'val'
void KBellModule::setBellVolume(int val)
{
    volume->setValue(val);
}

void KBellModule::setBellPitch(int val)
{
    pitch->setValue(val);
}

void KBellModule::setBellDuration(int val)
{
    duration->setValue(val);
}

// return the current LCD setting
int  KBellModule::getBellVolume()
{
    return volume->value();
}

int  KBellModule::getBellPitch()
{
    return pitch->value();
}

int  KBellModule::getBellDuration()
{
    return duration->value();
}

void KBellModule::GetSettings( void )
{
    XKeyboardState kbd;
    XGetKeyboardControl(kapp->getDisplay(), &kbd);

    config->setGroup("Bell");
    bellVolume = config->readNumEntry("Volume",kbd.bell_percent);
    bellPitch = config->readNumEntry("Pitch",kbd.bell_pitch);
    bellDuration = config->readNumEntry("Duration",kbd.bell_duration);

    // the GUI should reflect the real values
    setBellVolume(kbd.bell_percent);
    setBellPitch(kbd.bell_pitch);
    setBellDuration(kbd.bell_duration);
}

void KBellModule::saveParams( void )
{
    XKeyboardControl kbd;

    bellVolume = getBellVolume();
    bellPitch = getBellPitch();
    bellDuration = getBellDuration();

    kbd.bell_percent = bellVolume;
    kbd.bell_pitch = bellPitch;
    kbd.bell_duration = bellDuration;
    XChangeKeyboardControl(kapp->getDisplay(),
                           KBBellPercent | KBBellPitch | KBBellDuration,
                           &kbd);

    config->setGroup("Bell");
    config->writeEntry("Volume",bellVolume);
    config->writeEntry("Pitch",bellPitch);
    config->writeEntry("Duration",bellDuration);
    config->sync();
}

void KBellModule::ringBell()
{
    // store the old state
    XKeyboardState old_state;
    XGetKeyboardControl(kapp->getDisplay(), &old_state);

    // switch to the test state
    XKeyboardControl kbd;
    kbd.bell_percent = getBellVolume();
    kbd.bell_pitch = getBellPitch();
    kbd.bell_duration = getBellDuration();
    XChangeKeyboardControl(kapp->getDisplay(),
                           KBBellPercent | KBBellPitch | KBBellDuration,
                           &kbd);
    // ring bell
    XBell(kapp->getDisplay(),100);

    // restore old state
    kbd.bell_percent = old_state.bell_percent;
    kbd.bell_pitch = old_state.bell_pitch;
    kbd.bell_duration = old_state.bell_duration;
    XChangeKeyboardControl(kapp->getDisplay(),
                           KBBellPercent | KBBellPitch | KBBellDuration,
                           &kbd);
}

void KBellModule::load()
{
    GetSettings();
}

void KBellModule::save()
{
    saveParams();
}

void KBellModule::defaults()
{
    setBellVolume(100);
    setBellPitch(800);
    setBellDuration(100);
}


void KBellModule::slotChanged()
{
  emit changed(true);
}


extern "C"
{
  KCModule *create_bell(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmbell");
    return new KBellModule(parent, name);
  }

  void init_bell()
  {
    KConfig *config = new KConfig("kcmbellrc");

    XKeyboardState kbd;
    XGetKeyboardControl(kapp->getDisplay(), &kbd);

    config->setGroup("Bell");
    int bellVolume = config->readNumEntry("Volume", kbd.bell_percent);
    int bellPitch = config->readNumEntry("Pitch",kbd.bell_pitch);
    int bellDuration = config->readNumEntry("Duration",kbd.bell_duration);

    XKeyboardControl kbd2;

    kbd2.bell_percent = bellVolume;
    kbd2.bell_pitch = bellPitch;
    kbd2.bell_duration = bellDuration;

    XChangeKeyboardControl(kapp->getDisplay(),
                           KBBellPercent | KBBellPitch | KBBellDuration,
                           &kbd2);
  }
}

