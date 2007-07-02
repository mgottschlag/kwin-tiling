/**************************************************************************

    main.cpp  - The main function for KPager
    Copyright (C) 1998-2000  Antonio Larrosa Jimenez <larrosa@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    Send comments and bug fixes to larrosa@kde.org

***************************************************************************/

#include <kuniqueapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <QColor>
#include <kdebug.h>
#include <stdlib.h>
#include <QtGui/QSessionManager>

#include "kpager.h"

bool closed_by_sm = false;

class KPagerApplication : public KUniqueApplication
{
public:
  KPagerApplication() : KUniqueApplication() {}

  void commitData(QSessionManager& sm) {
    if (activeWindow() && activeWindow()->isHidden()) {
      sm.setRestartHint( QSessionManager::RestartNever );
      return;
    }
    closed_by_sm = true;
    KUniqueApplication::commitData( sm );
    closed_by_sm = false;
  }

  int newInstance() {
    if ( activeWindow() )
      activeWindow()->show();
    return 0;
  }

};

int main(int argc, char **argv)
{
    KAboutData *aboutdata = new KAboutData("kpager", 0, ki18n("KPager"), "1.5",
					   ki18n("Desktop Overview"), KAboutData::License_GPL,
					   ki18n("(C) 1998-2002, Antonio Larrosa Jimenez"),KLocalizedString(),
					   "http://developer.kde.org/~larrosa/kpager.html");

    aboutdata->addAuthor(ki18n("Antonio Larrosa Jimenez"),
			 ki18n("Original Developer/Maintainer"),"larrosa@kde.org",
			 "http://developer.kde.org/~larrosa/index.html");
    aboutdata->addAuthor(ki18n("Matthias Elter"),
			 ki18n("Developer"),"elter@kde.org");
    aboutdata->addAuthor(ki18n("Matthias Ettrich"),
			 ki18n("Developer"),"ettrich@kde.org");

    KCmdLineArgs::init(argc, argv, aboutdata);

    KCmdLineOptions pagerOpts;
    pagerOpts.add("hidden", ki18n("Create pager but keep the window hidden"));
    KCmdLineArgs::addCmdLineOptions(pagerOpts);
    KUniqueApplication::addCmdLineOptions();

    if (!KUniqueApplication::start())
    {
      kError() << "kpager is already running!" << endl;
      return 0;
    }


    KApplication * app = new KPagerApplication;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KPagerMainWindow *kpager = new KPagerMainWindow(0,"KPager");
    kpager->setPlainCaption( i18n("Desktop Pager") );


    if (!args->isSet("hidden"))
      kpager->show();
    else
      kpager->hide();

    int ret = app->exec();

    delete app;
    return ret;
}

