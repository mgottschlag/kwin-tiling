/* 
 * Copyright (c) 2002 Hamish Rodda <meddie@yoyo.its.monash.edu.au>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdlib.h>
#include <kdebug.h>

#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "krandrapp.h"

static const char* krandrtrayVersion = "0.1";
static const KCmdLineOptions options[] =
{
	{ "login", I18N_NOOP("Application is being auto-started at KDE session start"), 0L },
	{ 0L, 0L, 0L }
};

int main(int argc, char **argv)
{
	KAboutData aboutData("krandrtray", I18N_NOOP("Resize and Rotate"), krandrtrayVersion, I18N_NOOP("Resize and Rotate System Tray App"), KAboutData::License_GPL, "(c) 2002 Hamish Rodda", 0L, "");
	aboutData.addAuthor("Hamish Rodda",I18N_NOOP("Maintainer"),
						"meddie@yoyo.its.monash.edu.au");

	KCmdLineArgs::init(argc,argv,&aboutData);
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication::addCmdLineOptions();

	KRandRApp app;

	return app.exec();
}
