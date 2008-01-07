/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdlib.h>
#include <kdebug.h>

#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobal.h>

#include "krandrapp.h"
#include <KIcon>

static const char krandrtrayVersion[] = "0.5";
int main(int argc, char **argv)
{
	KAboutData aboutData("display-randr", "krandr", ki18n("Resize and Rotate"), krandrtrayVersion, 
			ki18n("X Resize and Rotate System Tray App"), KAboutData::License_GPL, 
			ki18n("(c) 2007 Gustavo Pichorim Boiko, 2002-2003 Hamish Rodda"), ki18n(0L));
	aboutData.addAuthor(ki18n("Gustavo Pichorim Boiko"),ki18n("Maintainer"), "gustavo.boiko@kdemail.net");
	aboutData.addAuthor(ki18n("Hamish Rodda"),ki18n("Original Author"), "rodda@kde.org");
	aboutData.addCredit(ki18n("Lubos Lunak"),ki18n("Many fixes"), "l.lunak@suse.cz");
	aboutData.addCredit(ki18n("Harry Bock"),ki18n("Many fixes, multi-head support"), "hbock@providence.edu");
	aboutData.setProductName("krandr/krandrtray");

	KCmdLineArgs::init(argc,argv,&aboutData);

	KCmdLineOptions options;
	options.add("login", ki18n("Application is being auto-started at KDE session start"), 0L);
	KCmdLineArgs::addCmdLineOptions(options);
	KCmdLineArgs::addStdCmdLineOptions();

	KRandRApp app;
	QApplication::setWindowIcon(KIcon("preferences-desktop-display-randr"));
	return app.exec();
}
