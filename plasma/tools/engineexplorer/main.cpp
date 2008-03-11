/*
 *   Copyright 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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

#include "engineexplorer.h"

static const char description[] = I18N_NOOP("Explore the data published by Plasma DataEngines");
static const char version[] = "0.0";

int main(int argc, char **argv)
{
    KAboutData aboutData("plasmaengineexplorer", 0, ki18n("Plasma Engine Explorer"),
                         version, ki18n(description), KAboutData::License_GPL,
                         ki18n("(c) 2006, The KDE Team"));
    aboutData.addAuthor(ki18n("Aaron J. Seigo"),
                        ki18n( "Author and maintainer" ),
                        "aseigo@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("height <pixels>", ki18n("The desired height in pixels"));
    options.add("width <pixels>", ki18n("The desired width in pixels"));
    options.add("x <pixels>", ki18n("The desired x position in pixels"));
    options.add("y <pixels>", ki18n("The desired y position in pixels"));
    options.add("engine <data engine>", ki18n("The data engine to use"));
    options.add("interval <ms>", ki18n("Update Interval in milliseconds.  Default: 50ms"), "50");
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication app;
    EngineExplorer* w = new EngineExplorer;

    bool ok1,ok2 = false;
    //get pos if available
    int x = args->getOption("height").toInt(&ok1);
    int y = args->getOption("width").toInt(&ok2);
    if (ok1 & ok2) {
        w->resize(x,y);
    }

    //get size
    x = args->getOption("x").toInt(&ok1);
    y = args->getOption("y").toInt(&ok2);
    if (ok1 & ok2) {
        w->move(x,y);
    }

    //set interval
    int interval = args->getOption("interval").toInt(&ok1);
    if (ok1) {
        w->setInterval(interval);
    }

    //set engine
    QString engine = args->getOption("engine");
    if (!engine.isEmpty()) {
        w->setEngine(engine);
    }

    args->clear();

    w->show();
    return app.exec();
}
