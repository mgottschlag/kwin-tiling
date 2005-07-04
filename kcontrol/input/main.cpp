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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <dcopref.h>
#include <qfile.h>

#include <X11/Xlib.h>

#ifdef HAVE_XCURSOR
#  include <X11/Xcursor/Xcursor.h>
#endif

#include "mouse.h"

extern "C"
{
  KDE_EXPORT KCModule *create_mouse(QWidget *parent, const char *)
  {
    return new MouseConfig(parent, "kcminput");
  }

  KDE_EXPORT void init_mouse()
  {
    KConfig *config = new KConfig("kcminputrc", true, false); // Read-only, no globals
    MouseSettings settings;
    settings.load(config);
    settings.apply(true); // force

#ifdef HAVE_XCURSOR
    config->setGroup("Mouse");
    QCString theme = QFile::encodeName(config->readEntry("cursorTheme", QString()));
    QCString size = config->readEntry("cursorSize", QString()).local8Bit();

    // Note: If you update this code, update kapplymousetheme as well.

    // use a default value for theme only if it's not configured at all, not even in X resources
    if( theme.isEmpty()
        && QCString( XGetDefault( qt_xdisplay(), "Xcursor", "theme" )).isEmpty()
        && QCString( XcursorGetTheme( qt_xdisplay())).isEmpty())
    {
        theme = "default";
    }

     // Apply the KDE cursor theme to ourselves
    if( !theme.isEmpty())
        XcursorSetTheme(qt_xdisplay(), theme.data());

    if (!size.isEmpty())
    	XcursorSetDefaultSize(qt_xdisplay(), size.toUInt());

    // Load the default cursor from the theme and apply it to the root window.
    Cursor handle = XcursorLibraryLoadCursor(qt_xdisplay(), "left_ptr");
    XDefineCursor(qt_xdisplay(), qt_xrootwin(), handle);
    XFreeCursor(qt_xdisplay(), handle); // Don't leak the cursor

    // Tell klauncher to set the XCURSOR_THEME and XCURSOR_SIZE environment
    // variables when launching applications.
    DCOPRef klauncher("klauncher");
    if( !theme.isEmpty())
        klauncher.send("setLaunchEnv", QCString("XCURSOR_THEME"), theme);
    if( !size.isEmpty())
        klauncher.send("setLaunchEnv", QCString("XCURSOR_SIZE"), size);
#endif

    delete config;
  }
}


