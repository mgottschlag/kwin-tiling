/**
 *  general.cpp
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
#include <kdialog.h>

#include "main.h"
#include "general.h"

KDrKonqiGeneral::KDrKonqiGeneral(KConfig *config, QString group, QWidget *parent, const char *name)
  : KCModule(parent, name), g_pConfig(config), groupname(group)
{
  QGridLayout *layout = new QGridLayout(this, 2,2,
                                        KDialog::marginHint(),
					KDialog::spacingHint());



   lb_Presets = new QListBox(this, "Preset list",0);
   layout->addWidget(lb_Presets, 0, 0);


QStringList qldd = KGlobal::dirs()->findAllResources("data","drkonqi/presets/*rc",false, true);


for (QStringList::Iterator it = qldd.begin();  it != qldd.end(); ++it)
{
// Read the configuration titles
KConfig config ((*it).latin1(), true, true, "data");

config.setGroup("General");

QString ConfigNames = config.readEntry(QString::fromLatin1("Name"),QString::fromLatin1("unknown"));
lb_Presets->insertItem(ConfigNames.latin1());
}
   connect(lb_Presets,SIGNAL(selectionChanged()), SLOT(showDetails()));
  load();
}

void KDrKonqiGeneral::showDetails (void)
{
QStringList qldd = KGlobal::dirs()->findAllResources("data","drkonqi/presets/*rc",false, true);

for (QStringList::Iterator it = qldd.begin();  it != qldd.end(); ++it)
{
// Read the configuration titles
KConfig config ((*it).latin1(), true, true, "data");

config.setGroup("General");

if ( lb_Presets->currentText() == config.readEntry(QString::fromLatin1("Name"),QString::fromLatin1("unknown")))
{

}

}
}


void KDrKonqiGeneral::load()
{
 // Set all options to the config file settings here
}

void KDrKonqiGeneral::defaults()
{
 // Load the defaults here
}

void KDrKonqiGeneral::save()
{
 // Save the settings here
}

void KDrKonqiGeneral::changed()
{
  emit KCModule::changed(true);
}

