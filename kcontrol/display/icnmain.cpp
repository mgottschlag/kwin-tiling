/**
 *  main.cpp
 *
 *  Copyright (c) 2000 Torsten Rahn <torsten@kde.org>
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


#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>

#include "icnmain.h"
#include "icnmain.moc"

KIconConfigMain::KIconConfigMain(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
//    KConfig *config = new KConfig("iconsrc",false,true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    tab = new QTabWidget(this);
    layout->addWidget(tab);

    general = new KIconConfigGeneral();
    tab->addTab(general, i18n("&General"));
    connect(general,SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

    label = new KIconConfigLabel();
    tab->addTab(label, i18n("&Label"));
    connect(label,SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

}

void KIconConfigMain::load()
{
    general->load();
    label->load();
}

void KIconConfigMain::save()
{
    general->save();
    label->save();
}

void KIconConfigMain::defaults()
{
    general->defaults();
    label->defaults();
}

void KIconConfigMain::moduleChanged(bool )
{
  emit changed(true);
}

QString KIconConfigMain::quickHelp()
{
    return i18n("<h1>Icons</h1> Here you can adjust the appearance"
                " of the icons and the label that is "
                " <h2>General</h2> Icons are being used in different"
		" places. You can set the size of an icon for each"
		" of these places here. In additon you can change "
		" the way an icon appears when your mouse-pointer"
		" is placed over an icon or when the feature that "
		" the icon symbolizes is not available."
                " <h2>Label</h2> Desktop-icons and Toolbar-icons"
		" can have tiny labels underneath. Here you can"
		" set the properties for these labels.");
}


extern "C"
{

  KCModule *create_icons(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmicons");
    return new KIconConfigMain(parent, name);
  }
}
