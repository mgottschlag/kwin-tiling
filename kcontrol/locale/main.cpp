/*
  main.cpp - A KControl Application

  Copyright 1999-2000 Hans Petter Bieker <bieker@kde.org>

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

#include "toplevel.h"
#include <klocale.h>

K_EXPORT_COMPONENT_FACTORY (kcm_locale, KLocaleFactory("kcmlocale") );
/*
extern "C" {
  KCModule *create_locale(QWidget *parent, const char* name) {
    KLocale::setMainCatalogue("kcmlocale");
    return new KLocaleApplication(parent, "kcmlocale");
  }
}
*/
