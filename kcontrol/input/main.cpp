/*
 * main.cpp
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
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


#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>


#include <X11/Xlib.h>


#include "keyboard.h"
#include "mouse.h"

// in keyboard.cpp
extern void numlockx_change_numlock_state( bool set_P );

extern "C"
{

  KCModule *create_keyboard(QWidget *parent, const char *name) 
  { 
    KGlobal::locale()->insertCatalogue("kcminput");
    return new KeyboardConfig(parent, name);
  }

  KCModule *create_mouse(QWidget *parent, const char *name) 
  { 
    KGlobal::locale()->insertCatalogue("kcminput");
    return new MouseConfig(parent, name);
  }

  void init_keyboard()
  {
    KConfig *config = new KConfig("kcminputrc", true); // Read-only, no globals
    config->setGroup("Keyboard");

    XKeyboardState   kbd;
    XKeyboardControl kbdc;

    XGetKeyboardControl(kapp->getDisplay(), &kbd);
    bool key = config->readBoolEntry("KeyboardRepeating", true);
    kbdc.key_click_percent = config->readNumEntry("ClickVolume", kbd.key_click_percent);
    kbdc.auto_repeat_mode = (key ? AutoRepeatModeOn : AutoRepeatModeOff);
    XChangeKeyboardControl(kapp->getDisplay(),
                           KBKeyClickPercent | KBAutoRepeatMode,
                           &kbdc);
    int numlockState = config->readNumEntry( "NumLock", 2 );
    if( numlockState != 2 )
        numlockx_change_numlock_state( numlockState == 0 );

    MouseSettings settings;
    settings.load(config);
    settings.apply();
    delete config;
  }

}


