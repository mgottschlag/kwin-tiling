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

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>

#include "config.h"
#include "plasmaapp.h"

static const char description[] = I18N_NOOP( "The KDE desktop, panels and widgets workspace application." );
static const char version[] = "0.0";

int main(int argc, char **argv)
{
    KAboutData aboutData( "plasma-qgv", I18N_NOOP( "Plasma Workspace" ),
                          version, description, KAboutData::License_GPL,
                          "(c) 2006, The KDE Team" );
    aboutData.addAuthor( "Aaron J. Seigo",
                         I18N_NOOP( "Author and maintainer" ),
                         "aseigo@kde.org" );


    KCmdLineArgs::init(argc, argv, &aboutData);

    PlasmaApp app;
    return app.exec();
}
