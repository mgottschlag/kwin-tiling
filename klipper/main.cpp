// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) Andrew Stanley-Jones

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>

#include <klocale.h>
#include <kcmdlineargs.h>
#include <kwin.h>
#include <kaboutdata.h>
#include <kuniqueapplication.h>

#include "toplevel.h"
#include "version.h"

#if defined Q_WS_X11
//#include <qxembed.h> // schroder
#include <QX11EmbedWidget>
#endif


extern "C" int KDE_EXPORT kdemain(int argc, char *argv[])
{
  Klipper::createAboutData();
  KCmdLineArgs::init( argc, argv, Klipper::aboutData());
  KUniqueApplication::addCmdLineOptions();

  if (!KUniqueApplication::start()) {
       fprintf(stderr, "Klipper is already running!\n");
       exit(0);
  }
  KUniqueApplication app;
  app.disableSessionManagement();

  Klipper *toplevel = new Klipper();

  // Make Klipper conform to freedesktop system tray standard, see
  // http://bugs.kde.org/show_bug.cgi?id=69119
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#warning "Qt4 port me to QX11EmbedWidget ";
  //QXEmbed::initialize();
#endif

  KWin::setSystemTrayWindowFor( toplevel->winId(), 0 );
  toplevel->setGeometry(-100, -100, 42, 42 );
  toplevel->show();

  int ret = app.exec();
  delete toplevel;
  Klipper::destroyAboutData();
  return ret;
}
