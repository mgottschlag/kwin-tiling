/*
 * keyboard.h
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

#ifndef __KKEYBOARDCONFIG_H__
#define __KKEYBOARDCONFIG_H__

#include <kapp.h>
#include <kcontrol.h>

class KIntNumInput;
class KConfig;
class QCheckBox;

class KeyboardConfig : public KConfigWidget
{
  Q_OBJECT
public:
  KeyboardConfig( QWidget *parent=0, const char* name=0, bool init=FALSE );
  ~KeyboardConfig( );
  void saveParams( void );

  void loadSettings();
  void applySettings();
  void defaultSettings();
      
private:
  void GetSettings( void );

  void setClick( int );
  void setRepeat( int );
  void setRepeatRate( int );

  int getClick();
  int getRepeatRate();

  QCheckBox *repeatBox;
  KIntNumInput *click;

  KConfig *config;
  int clickVolume, keyboardRepeat;

  bool GUI;
};

#endif

