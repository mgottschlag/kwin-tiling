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

#include <klocale.h>
#include <kconfig.h>

#include "bell.h"

#include <X11/Xlib.h>

#include "geom.h"


static bool GUI;

KBellConfig::~KBellConfig ()
{
    if (GUI) {
        delete volume;
        delete pitch;
        delete duration;
    }
}

KBellConfig::KBellConfig (QWidget * parent, const char *name, bool init)
    : KConfigWidget (parent, name)
{
  GUI = !init;
  
  if (GUI) {
    QVBoxLayout *layout = new QVBoxLayout(this, 10);

    // args: label, min, max, step, initial, units
    volume = new KSliderControl(i18n("Volume:"), 0, 100, 5, 50, "%", this);
    volume->setSteps(5,25);
    layout->addWidget(volume);

    pitch = new KSliderControl(i18n("Pitch:"),  0, 2000, 20, 800, 
			       i18n("Hz"), this);
    pitch->setSteps(40,200);
    layout->addWidget(pitch);

    duration = new KSliderControl(i18n("Duration:"), 0, 1000, 50, 100,
				  i18n("ms"), this);
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
  }
  
  config = kapp->getConfig();
  
  GetSettings();
  
  if (init)
    saveParams();
}

// set the slider and the LCD to 'val'
void KBellConfig::setBellVolume(int val)
{
    volume->setValue(val);
}

void KBellConfig::setBellPitch(int val)
{
    pitch->setValue(val);
}

void KBellConfig::setBellDuration(int val)
{
    duration->setValue(val);
}

// return the current LCD setting
int  KBellConfig::getBellVolume()
{
    return volume->value();
}

int  KBellConfig::getBellPitch()
{
    return pitch->value();
}

int  KBellConfig::getBellDuration()
{
    return duration->value();
}

void KBellConfig::GetSettings( void )
{
    XKeyboardState kbd;
    
    config->setGroup("Bell");
    bellVolume = config->readNumEntry("Volume",-1);
    bellPitch = config->readNumEntry("Pitch",-1);
    bellDuration = config->readNumEntry("Duration",-1);
    
    XGetKeyboardControl(kapp->getDisplay(), &kbd);
    
    // if the config file didn't have anything, use the X server settings
    if (bellVolume == -1)
        bellVolume = kbd.bell_percent;
    if (bellPitch == -1)
        bellPitch = kbd.bell_pitch;
    if (bellDuration == -1)
        bellDuration = kbd.bell_duration;
    
    // the GUI should reflect the real values
    if (GUI) {
        setBellVolume(kbd.bell_percent);
        setBellPitch(kbd.bell_pitch);
        setBellDuration(kbd.bell_duration);
    }
}

void KBellConfig::saveParams( void )
{
    XKeyboardControl kbd;
    
    if (GUI) {
        bellVolume = getBellVolume();
        bellPitch = getBellPitch();
        bellDuration = getBellDuration();
    }
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

void KBellConfig::ringBell()
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

void KBellConfig::loadSettings()
{
    GetSettings();
}

void KBellConfig::applySettings()
{
    saveParams();
}

void KBellConfig::defaultSettings()
{
    setBellVolume(50);
    setBellPitch(800);
    setBellDuration(100);
}

#include "bell.moc"
