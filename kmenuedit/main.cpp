/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2001-2002 Raffaele Sandrini <sandrini@kde.org)
 *   Copyright (C) 2008 Montel Laurent <montel@kde.org)
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

#include <KUniqueApplication>
#include <KLocale>
#include <KCmdLineArgs>
#include <KAboutData>

#include "kmenuedit.h"
#ifndef Q_WS_WIN
#include "khotkeys.h"
#endif

static const char description[] = I18N_NOOP("KDE menu editor");
static const char version[] = "0.8";

static KMenuEdit *menuEdit = 0;

class KMenuApplication : public KUniqueApplication
{
public:
   KMenuApplication() { }
#ifndef Q_WS_WIN
   virtual ~KMenuApplication() { KHotKeys::cleanup(); }
#endif
   virtual int newInstance()
   {
      KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
      if (args->count() > 0)
      {
          menuEdit->selectMenu(args->arg(0));
          if (args->count() > 1)
          {
              menuEdit->selectMenuEntry(args->arg(1));
          }
      }
      args->clear();
      return KUniqueApplication::newInstance();
   }
};


extern "C" int KDE_EXPORT kdemain( int argc, char **argv )
{
    KAboutData aboutData("kmenuedit", 0, ki18n("KDE Menu Editor"),
                         version, ki18n(description), KAboutData::License_GPL,
                         ki18n("(C) 2000-2003, Waldo Bastian, Raffaele Sandrini, Matthias Elter"));
    aboutData.addAuthor(ki18n("Waldo Bastian"), ki18n("Maintainer"), "bastian@kde.org");
    aboutData.addAuthor(ki18n("Raffaele Sandrini"), ki18n("Previous Maintainer"), "sandrini@kde.org");
    aboutData.addAuthor(ki18n("Matthias Elter"), ki18n("Original Author"), "elter@kde.org");
    aboutData.addAuthor(ki18n("Montel Laurent"), KLocalizedString(), "montel@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KUniqueApplication::addCmdLineOptions();

    KCmdLineOptions options;
    options.add("+[menu]", ki18n("Sub menu to pre-select"));
    options.add("+[menu-id]", ki18n("Menu entry to pre-select"));
    KCmdLineArgs::addCmdLineOptions( options );

    if (!KUniqueApplication::start())
        return 1;

    KMenuApplication app;

    menuEdit = new KMenuEdit();
    menuEdit->show();

    return  app.exec();
}
