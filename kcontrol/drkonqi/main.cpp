/**
 *  main.cpp
 *
 *  Copyright (c) 2000 Timo Hummel <timo.hummel@sap.com>
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

#include "main.h"

KDrKonqiMain::KDrKonqiMain(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
    KConfig *config = new KConfig("drkonqirc",false,true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    tab = new QTabWidget(this);
    layout->addWidget(tab);

    general = new KDrKonqiGeneral(config, "General", this);
    tab->addTab(general, i18n("&General"));
    connect(general,SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

    customize = new KDrKonqiCustomize(config, "Customize", this);
    tab->addTab(customize, i18n("&Customize"));
    connect(customize,SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

}

void KDrKonqiMain::load()
{
    general->load();
    customize->load();
}

void KDrKonqiMain::save()
{
    general->save();
    customize->save();
}

void KDrKonqiMain::defaults()
{
    general->defaults();
    customize->defaults();
}

void KDrKonqiMain::moduleChanged(bool state)
{
  emit changed(true);
}

QString KDrKonqiMain::quickHelp()
{
    return i18n("<h1>Dr Konqi</h1>Here you can set the options if"
                " and how Dr. Konqi informs you about application"
                " crashes. Altough of course you should never see"
                " Dr. Konqi, in complex applications you can never"
                " eleminate all bugs."
                " <h2>General</h2>Here you can configure if and how"
                " Dr. Konqi informs you about an error. You can"
                " choose pre-defined message presets (end users and"
                " developers for example) to provide you only with"
                " the informations you need for your daily work."
                " <h2>Customize</h2>Here you can change the presets"
                " for Dr. Konqi's appearance.");
}


extern "C"
{

  KCModule *create_drkonqi(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmdrkonqi");
    return new KDrKonqiMain(parent, name);
  }
}
#include "main.moc"
