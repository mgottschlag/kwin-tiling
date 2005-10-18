/*
 * main.cpp
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <qlayout.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <kgenericfactory.h>
#include <kaboutdata.h>

#include "icons.h"
#include "iconthemes.h"
#include "main.h"

/**** DLL Interface ****/
typedef KGenericFactory<IconModule, QWidget> IconsFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_icons, IconsFactory("kcmicons") )

/**** IconModule ****/

IconModule::IconModule(QWidget *parent, const char *, const QStringList &)
  : KCModule(IconsFactory::instance(), parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  tab = new QTabWidget(this);
  layout->addWidget(tab);

  tab1 = new IconThemesConfig(IconsFactory::instance(), this);
  tab1->setObjectName( "themes" );
  tab->addTab(tab1, i18n("&Theme"));
  connect(tab1, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  tab2 = new KIconConfig(IconsFactory::instance(), this);
  tab2->setObjectName( "effects" );
  tab->addTab(tab2, i18n("Ad&vanced"));
  connect(tab2, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  KAboutData* about = new KAboutData("kcmicons", I18N_NOOP("Icons"), "3.0",
	      I18N_NOOP("Icons Control Panel Module"),
	      KAboutData::License_GPL,
 	      I18N_NOOP("(c) 2000-2003 Geert Jansen"), 0, 0);
  about->addAuthor("Geert Jansen", 0, "jansen@kde.org");
  about->addAuthor("Antonio Larrosa Jimenez", 0, "larrosa@kde.org");
  about->addCredit("Torsten Rahn", 0, "torsten@kde.org");
  setAboutData( about );
}


void IconModule::load()
{
  tab1->load();
  tab2->load();
}


void IconModule::save()
{
  tab1->save();
  tab2->save();
}


void IconModule::defaults()
{
  tab1->defaults();
  tab2->defaults();
}


void IconModule::moduleChanged(bool state)
{
  emit changed(state);
}

QString IconModule::quickHelp() const
{
  return i18n("<h1>Icons</h1>"
    "This module allows you to choose the icons for your desktop.<p>"
    "To choose an icon theme, click on its name and apply your choice by pressing the \"Apply\" button below. If you do not want to apply your choice you can press the \"Reset\" button to discard your changes.</p>"
    "<p>By pressing the \"Install New Theme\" button you can install your new icon theme by writing its location in the box or browsing to the location."
    " Press the \"OK\" button to finish the installation.</p>"
    "<p>The \"Remove Theme\" button will only be activated if you select a theme that you installed using this module."
    " You are not able to remove globally installed themes here.</p>"
    "<p>You can also specify effects that should be applied to the icons.</p>");
}



#include "main.moc"
