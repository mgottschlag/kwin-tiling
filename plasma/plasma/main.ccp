/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <kapplication.h>

#include "config.h"

#include "plasmaapp.h"

int main(int argc, const char* argv[])
{
    KAboutData aboutData( "plasma", I18N_NOOP("Plasma Workspace"),
                          "0.0",
                          I18N_NOOP("The KDE desktop, panels and widgets workspace application."),
                          KAboutData::License_GPL,
                          I18N_NOOP("(c) 2006, The KDE Team") );
    aboutData.addAuthor("Aaron J. Seigo", I18N_NOOP("Current maintainer"), "aseigo@kde.org");
    KCmdLineArgs::init(argc, argv, &aboutData);

    PlasmaApp app;
    return app.exec();
}
