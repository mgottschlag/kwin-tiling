/*
  main.cpp - A KControl Application

  written 1998 by Matthias Hoelzer
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */

#include <kcontrol.h>

class KLocaleConfigMoney;
class KLocaleConfigNumber;

class KLocaleApplication : public KControlApplication
{
public:
  KLocaleApplication(int &argc, char **arg, const char *name);

  void init();
  void apply();
  void defaultValues();

  static void reTranslate();
  static void reTranslate(QObjectListIt it);

  void reset();
  void resetNum();
  void resetMon();

private:
  KLocaleConfig *locale;
  KLocaleConfigNumber *localenum;
  KLocaleConfigMoney *localemon;
};
