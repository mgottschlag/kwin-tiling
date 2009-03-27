/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


#include <KCmdLineArgs>
#include <KAboutData>
#include <KLocale>

#include "PkKAuthorizationDialog.h"

using namespace PolkitKde;

int main(int argc, char *argv[])
{
    KAboutData aboutData("polkit-kde-authorization", "", ki18n("PolicyKit KDE Authorization"),
                         "0.1", ki18n("KDE interface for managing PolicyKit Authorizations"),
                         KAboutData::License_GPL, ki18n("(C) 2008 Daniel Nicoletti"));
    aboutData.addAuthor(ki18n("Daniel Nicoletti"), ki18n("Author"), "dantti85-pk@yahoo.com.br");
    aboutData.addAuthor(ki18n("Dario Freddi"), ki18n("Developer"), "drf54321@gmail.com", "http://drfav.wordpress.com");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[action]", ki18n("Action to be displayed"));
    KCmdLineArgs::addCmdLineOptions(options);

    PkKAuthorizationDialog::addCmdLineOptions();

    if (!PkKAuthorizationDialog::start()) {
        qWarning("PolKit Kde Authorization is already running!\n");
        return 0;
    }

    PkKAuthorizationDialog app;

    return app.exec();
}
