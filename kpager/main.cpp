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
#include <qsessionmanager.h>

#include "kpager.h"

static KCmdLineOptions pagerOpts[] =
{
    { "hidden", I18N_NOOP("Create pager but keep the window hidden"), 0 },
    KCmdLineLastOption
};

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
    KAboutData *aboutdata = new KAboutData("kpager", "KPager", "1.5",
					   I18N_NOOP("Desktop Overview"), KAboutData::License_GPL,
					   "(C) 1998-2002, Antonio Larrosa Jimenez","",
					   "http://developer.kde.org/~larrosa/kpager.html");

    aboutdata->addAuthor("Antonio Larrosa Jimenez",
			 I18N_NOOP("Original Developer/Maintainer"),"larrosa@kde.org",
			 "http://developer.kde.org/~larrosa/index.html");
    aboutdata->addAuthor("Matthias Elter",
			 I18N_NOOP("Developer"),"elter@kde.org", "");
    aboutdata->addAuthor("Matthias Ettrich",
			 I18N_NOOP("Developer"),"ettrich@kde.org", "");

    KCmdLineArgs::init(argc, argv, aboutdata);
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

