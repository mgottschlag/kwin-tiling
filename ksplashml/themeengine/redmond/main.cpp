/***************************************************************************
 *   Copyright Brian Ledbetter 2001-2003 <brian@shadowcom.net>             *
 *   Copyright Ravikiran Rajagopal 2003                                    *
 *   ravi@ee.eng.ohio-state.edu                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

#include <kstandarddirs.h>

#include <qlabel.h>
#include <qpixmap.h>
#include <qwidget.h>

#include "themeredmond.h"

extern "C"
{
  ThemeEngineConfig *KsThemeConfig( QWidget *parent, KConfig *config )
  {
    return new CfgRedmond( parent, config );
  }

  QStringList KsThemeSupports()
  {
    return ThemeRedmond::names();
  }

  void* KsThemeInit( QWidget *parent, ObjKsTheme *theme )
  {
    return new ThemeRedmond( parent, theme );
  }
}

