/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2001-2002 Raffaele Sandrini <sandrini@kde.org)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <unistd.h>

#include <kuniqueapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "kmenuedit.h"

static const char *description = I18N_NOOP("KDE Menu editor");
static const char *version = "0.4";

extern "C" int kdemain( int argc, char **argv )
{
    KAboutData aboutData("kmenuedit", I18N_NOOP("KDE Menu Editor"),
			 version, description, KAboutData::License_GPL,
			 "(C) 2002, Raffaele Sandrini");
    aboutData.addAuthor("Raffaele Sandrini", I18N_NOOP("Maintainer"), "sandrini@kde.org");
    aboutData.addAuthor("Matthias Elter", I18N_NOOP("Original Author"), "elter@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KUniqueApplication::addCmdLineOptions();

    if (!KUniqueApplication::start()) 
	return 1;

    KUniqueApplication app;

    KMenuEdit *menuEdit = new KMenuEdit;
    menuEdit->show();

    app.setMainWidget(menuEdit);
    return  app.exec();
}
