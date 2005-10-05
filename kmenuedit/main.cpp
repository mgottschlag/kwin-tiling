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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <unistd.h>

#include <kuniqueapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "kmenuedit.h"
#include "khotkeys.h"

static const char description[] = I18N_NOOP("KDE menu editor");
static const char version[] = "0.7";

static const KCmdLineOptions options[] =
{
	{ "+[menu]", I18N_NOOP("Sub menu to pre-select"), 0 },
	{ "+[menu-id]", I18N_NOOP("Menu entry to pre-select"), 0 },
	KCmdLineLastOption
};

static KMenuEdit *menuEdit = 0;

class KMenuApplication : public KUniqueApplication
{
public:
   KMenuApplication() { }
   virtual ~KMenuApplication() { KHotKeys::cleanup(); }
   
   virtual int newInstance()
   {
      KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
      if (args->count() > 0)
      {
          menuEdit->selectMenu(QString::fromLocal8Bit(args->arg(0)));
          if (args->count() > 1)
          {
              menuEdit->selectMenuEntry(QString::fromLocal8Bit(args->arg(1)));
          }
      }
      return KUniqueApplication::newInstance();
   }
};


extern "C" int KDE_EXPORT kdemain( int argc, char **argv )
{
    KAboutData aboutData("kmenuedit", I18N_NOOP("KDE Menu Editor"),
			 version, description, KAboutData::License_GPL,
			 "(C) 2000-2003, Waldo Bastian, Raffaele Sandrini, Matthias Elter");
    aboutData.addAuthor("Waldo Bastian", I18N_NOOP("Maintainer"), "bastian@kde.org");
    aboutData.addAuthor("Raffaele Sandrini", I18N_NOOP("Previous Maintainer"), "sandrini@kde.org");
    aboutData.addAuthor("Matthias Elter", I18N_NOOP("Original Author"), "elter@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KUniqueApplication::addCmdLineOptions();
    KCmdLineArgs::addCmdLineOptions( options );

    if (!KUniqueApplication::start()) 
	return 1;

    KMenuApplication app;

    menuEdit = new KMenuEdit(false);
    menuEdit->show();

    app.setMainWidget(menuEdit);
    return  app.exec();
}
