/*
 * main.cpp
 *
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <kmessagebox.h>

#include "themecreator.h"
#include "installer.h"
#include "global.h"
#include "options.h"
#include "version.h"

#include <qlayout.h>
#include <qtabwidget.h>

#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kgenericfactory.h>
#include <kaboutdata.h>

ThemeCreator* theme = NULL;

//-----------------------------------------------------------------------------
class KThemeMgr : public KCModule
{
public:

  KThemeMgr(QWidget *parent, const char *name, const QStringList &);
  ~KThemeMgr();

  virtual void init();
  virtual void save();
  virtual void load();
  virtual void defaults();
  const KAboutData* aboutData() const;

private:
  Installer* mInstaller;
  Options* mOptions;
};

typedef KGenericFactory<KThemeMgr, QWidget > ThemeTimeFactory;
K_EXPORT_COMPONENT_FACTORY (libkcm_themes, ThemeTimeFactory("kcmthemes") );

//-----------------------------------------------------------------------------
KThemeMgr::KThemeMgr(QWidget *parent, const char *name, const QStringList &)
  : KCModule(parent, name)
{
  init();

  mInstaller = NULL;
  theme = new ThemeCreator;

  QVBoxLayout *layout = new QVBoxLayout(this);
  QTabWidget *tab = new QTabWidget(this);
  layout->addWidget(tab);

  mInstaller = new Installer(this);
  tab->addTab(mInstaller, i18n("&Installer"));
  connect(mInstaller, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));

  mOptions = new Options(this);
  tab->addTab(mOptions, i18n("&Contents"));
  connect(mOptions, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
}


//-----------------------------------------------------------------------------
KThemeMgr::~KThemeMgr()
{
  delete theme;
}

//-----------------------------------------------------------------------------
void KThemeMgr::init()
{
  //kdDebug() << "No init necessary" << endl;
    KGlobal::dirs()->addResourceType("themes", KStandardDirs::kde_default("data") + "kthememgr/Themes/");
}


//-----------------------------------------------------------------------------
void KThemeMgr::defaults()
{
  mInstaller->defaults();
}


//-----------------------------------------------------------------------------
void KThemeMgr::save()
{
  mOptions->save();
  mInstaller->save();
  theme->install();
}

//-----------------------------------------------------------------------------
void KThemeMgr::load()
{
  mInstaller->load();
  mOptions->load();
}

//-----------------------------------------------------------------------------
const KAboutData* KThemeMgr::aboutData() const
{

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmthememgr"), I18N_NOOP("KDE Theme Manager"),
                  KTHEME_VERSION, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 1998 - 2001 The KDE Theme Manager authors"));

    about->addAuthor("Stefan Taferner", 0, "taferner@kde.org");
    about->addAuthor("Waldo Bastian", 0, "bastian@kde.org");
    about->addCredit("Divide by Zero", 
                     I18N_NOOP("Support for MS Windows' Themes"), 
						   "divide@priv.onet.pl" );

    return about;
}
