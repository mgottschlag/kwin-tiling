/*
 * keyboard.cpp
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Layout management, cleanups:
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

//#include <iostream.h>
//#include <sys/types.h>
//#include <unistd.h>
//#include <sys/stat.h>
//#include <stdlib.h>

#include <qfileinfo.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <qlayout.h>
#include <klocale.h>
#include <kconfig.h>
#include <knuminput.h>

#include "keyboard.h"
#include <X11/Xlib.h>


KeyboardConfig::KeyboardConfig (QWidget * parent, const char *name)
    : KCModule (parent, name)
{
  QBoxLayout* lay = new QVBoxLayout(this, 10);

  repeatBox = new QCheckBox(i18n("Keyboard repeat"), this);
  lay->addWidget(repeatBox);
  connect(repeatBox, SIGNAL(clicked()), this, SLOT(changed()));

  lay->addSpacing(10);
  click = new KIntNumInput(100, this);
  click->setLabel(i18n("Key click volume"));
  click->setRange(0, 100, 10);
  click->setSuffix("%");
  click->setSteps(5,25);
  connect(click, SIGNAL(valueChanged(int)), this, SLOT(changed()));

  lay->addWidget(click);

  lay->addStretch(10);
  load();
}

int  KeyboardConfig::getClick()
{
    return click->value();
}

// set the slider and LCD values
void KeyboardConfig::setRepeat(int r)
{
    repeatBox->setChecked(r == AutoRepeatModeOn);
}

void KeyboardConfig::setClick(int v)
{
    click->setValue(v);
}

void KeyboardConfig::load()
{
  KConfig *config = new KConfig("kcminput");

    XKeyboardState kbd;

    XGetKeyboardControl(kapp->getDisplay(), &kbd);

    config->setGroup("Keyboard");
    bool key = config->readBoolEntry("KeyboardRepeat", true);
    keyboardRepeat = (key ? AutoRepeatModeOn : AutoRepeatModeOff);
    clickVolume = config->readNumEntry("ClickVolume", kbd.key_click_percent);

    setClick(kbd.key_click_percent);
    setRepeat(kbd.global_auto_repeat);

  delete config;
}

void KeyboardConfig::save()
{
  KConfig *config = new KConfig("kcminput");

    XKeyboardControl kbd;

    clickVolume = getClick();
    keyboardRepeat = repeatBox->isChecked() ? AutoRepeatModeOn : AutoRepeatModeOff;

    kbd.key_click_percent = clickVolume;
    kbd.auto_repeat_mode = keyboardRepeat;
    XChangeKeyboardControl(kapp->getDisplay(),
                           KBKeyClickPercent | KBAutoRepeatMode,
                           &kbd);

    config->setGroup("Keyboard");
    config->writeEntry("ClickVolume",clickVolume);
    config->writeEntry("KeyboardRepeat", (keyboardRepeat == AutoRepeatModeOn));
    config->sync();

  delete config;
}


void KeyboardConfig::defaults()
{
    setClick(50);
    setRepeat(true);
}


void KeyboardConfig::changed()
{
  emit KCModule::changed(true);
}


#include "keyboard.moc"
