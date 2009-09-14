/*
 * Copyright 2009 Aaron J. Seigo
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
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

#include <QCoreApplication>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KComponentData>
#include <KConfig>
#include <KDebug>

#include "../backgrounddialog.h"
#include "plasma-shell-desktop.h"

static const char description[] = "Background settings dialog";
static const char version[] = "1.0";

int main(int argc, char *argv[])
{
    KAboutData aboutData("plasmbackgrounddialog", 0, ki18n("Plasma"),
                         version, ki18n( description ), KAboutData::License_GPL,
                         ki18n("(C) 2008, Aaron Seigo"));
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    QSize res(1024, 768);
    QString id("foo");
    BackgroundDialog b(res, 0, 0, 0, id, AppSettings::self());
    b.show();

    return app.exec();
}

