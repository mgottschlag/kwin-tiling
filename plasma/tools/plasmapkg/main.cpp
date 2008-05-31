/*
 * Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include <iostream>

#include <QDir>

#include <KApplication>
#include <KAboutData>
#include <KAction>
#include <KCmdLineArgs>
#include <KLocale>
#include <KService>
#include <KServiceTypeTrader>
#include <KShell>
#include <KStandardDirs>

#include <plasma/packagestructure.h>

static const char description[] = I18N_NOOP("Install, list, remove Plasma packages");
static const char version[] = "0.1";

void listPackages()
{
    //TODO: implement
    kDebug() << "My apologies, listing is not yet implemented.";
}

int main(int argc, char **argv)
{
    KAboutData aboutData("plasmapkg", 0, ki18n("Plasma Package Manager"),
                         version, ki18n(description), KAboutData::License_GPL,
                         ki18n("(C) 2008, Aaron Seigo"));
    aboutData.addAuthor( ki18n("Aaron Seigo"),
                         ki18n("Original author"),
                        "aseigo@kde.org" );

    KComponentData componentData(aboutData);

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("g");
    options.add("global", ki18n("For install or remove, operates on packages installed for all users."));
    options.add("t");
    options.add("type <type>",
                ki18n("The type of package, e.g. theme, wallpaper, plasmoid, DataEngine, Runner, etc."),
                "plasmoid");
    options.add("s");
    options.add("i");
    options.add("install <path>", ki18n("Install the package at <path>"));
    options.add("l");
    options.add("list", ki18n("List installed packages"));
    options.add("r");
    options.add("remove <name>", ki18n("Remove the package named <name>"));
    options.add("p");
    options.add("packageroot <path>", ki18n("Absolute path to the package root. If not supplied, then the standard data directories for this KDE session will be searched instead."));
    KCmdLineArgs::addCmdLineOptions( options );

    //KApplication app;
    //QCoreApplication app;

    //TODOs:
    //   implement list

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() == 0 || args->isSet("list")) {
        listPackages();
    } else {
        // install and remove
        const QString type = args->getOption("type").toLower();
        QString packageRoot = args->getOption("type").toLower();
        QString servicePrefix;

        Plasma::PackageStructure *installer = new Plasma::PackageStructure();
        if (type == i18n("plasmoid") || type == "plasmoid") {
            packageRoot = "plasma/plasmoids/";
            servicePrefix = "plasma-applet-";
        } else if (type == i18n("theme") || type == "theme") {
            packageRoot = "desktoptheme/";
        } else if (type == i18n("wallpaper") || type == "wallpaper") {
            packageRoot = "wallpapers/";
        } else if (type == i18n("dataengine") || type == "dataengine") {
            packageRoot = "plasma/dataengines/";
            servicePrefix = "plasma-dataengine-";
        } else if (type == i18n("runner") || type == "runner") {
            packageRoot = "plasma/runners/";
            servicePrefix = "plasma-abstractrunner-";
        } else {
            // support for non-native widget packages
            delete installer;
            installer = 0;

            QString constraint = QString("'%1' == [X-KDE-PluginInfo-Name]").arg(packageRoot);
            KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure", constraint);
            if (offers.isEmpty()) {
                kFatal() << i18n("Could not find a suitable installer for package of type") << type;
            }

            KService::Ptr offer = offers.first();
            QString error;
            installer = offer->createInstance<Plasma::PackageStructure>(0, QVariantList(), &error);

            if (!installer) {
                kFatal() << i18n("Could not load installer for package of type %1. Error reported was: %2",
                                 type, error);
            }

            packageRoot = "plasma/plasmoids";
        }

        if (args->isSet("packageroot")) {
            packageRoot = args->getOption("packageroot");
        } else if (args->isSet("global")) {
            packageRoot = KStandardDirs::locate("data", packageRoot);
        } else {
            packageRoot = KStandardDirs::locateLocal("data", packageRoot);
        }

        if (args->isSet("install")) {
            QString package = KShell::tildeExpand(args->getOption("install"));
            if (!QDir::isAbsolutePath(package)) {
                package = QDir::currentPath() + package;
            }

            if (installer->installPackage(package, packageRoot)) {
                kDebug() << "Successfully installed " << package;
            } else {
                kFatal() << "Installation of " << package << " failed!";
            }
        } else if (args->isSet("remove")) {
            QString package = args->getOption("remove");
            if (installer->uninstallPackage(package, packageRoot)) {
                kDebug() << "Successfully removed " << package;
            } else {
                kFatal() << "Removal of " << package << " failed!";
            }
        } else {
            KCmdLineArgs::usageError(i18n("One of install, remove or list is required."));
        }
    }

    return 0;
}

