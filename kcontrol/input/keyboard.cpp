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
#include "geom.h"


KeyboardConfig::~KeyboardConfig ()
{
    if (GUI) {
        delete click;
    }
}

KeyboardConfig::KeyboardConfig (QWidget * parent, const char *name, bool init)
    : KConfigWidget (parent, name)
{
    GUI = !init;
    
    if (GUI) {
        QBoxLayout* lay = new QVBoxLayout(this, 10);

        repeatBox = new QCheckBox(i18n("Keyboard repeat"), this);
        lay->addWidget(repeatBox);
        
        click = new KIntNumInput(i18n("Key click volume"), 0, 100, 10, 100,
                                 "%", 10, true, this);
        click->setSteps(5,25);
        lay->addWidget(click);
    }
    config = kapp->getConfig();
    
    GetSettings();
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

void KeyboardConfig::GetSettings( void )
{
    XKeyboardState kbd;
    
    XGetKeyboardControl(kapp->getDisplay(), &kbd);
    
    config->setGroup("Keyboard");
    bool key = config->readBoolEntry("KeyboardRepeat", true);
    keyboardRepeat = (key ? AutoRepeatModeOn : AutoRepeatModeOff);
    clickVolume = config->readNumEntry("ClickVolume", kbd.key_click_percent);
    
    // the GUI should reflect the real values
    if (GUI) {
        setClick(kbd.key_click_percent);
        setRepeat(kbd.global_auto_repeat);
    }
}

void KeyboardConfig::saveParams( void )
{
    XKeyboardControl kbd;
    
    if (GUI) {
        clickVolume = getClick();
        keyboardRepeat = repeatBox->isChecked() ? AutoRepeatModeOn : AutoRepeatModeOff;
    }
    
    kbd.key_click_percent = clickVolume;
    kbd.auto_repeat_mode = keyboardRepeat;
    XChangeKeyboardControl(kapp->getDisplay(), 
                           KBKeyClickPercent | KBAutoRepeatMode,
                           &kbd);
    
    config->setGroup("Keyboard");
    config->writeEntry("ClickVolume",clickVolume);
    config->writeEntry("KeyboardRepeat", (keyboardRepeat == AutoRepeatModeOn));
    config->sync();
}

void KeyboardConfig::loadSettings()
{
    GetSettings();
}

void KeyboardConfig::applySettings()
{
    saveParams();
}

void KeyboardConfig::defaultSettings()
{
    setClick(50);
    setRepeat(true);
}


#include "keyboard.moc"
