/**
 *  customize.cpp
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
#include <qgroupbox.h>

#include "customize.h"

KDrKonqiCustomize::KDrKonqiCustomize(KConfig *config, QString group, QWidget *parent, const char *name)
  : KCModule(parent, name), g_pConfig(config), groupname(group)
{
  QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
                                              KDialog::spacingHint());     

  gb_Presets = new QGroupBox(i18n("Presets"), this);

  layout->addWidget(gb_Presets);

  QGridLayout *grid = new QGridLayout(gb_Presets,2,3,
					KDialog::marginHint(),
					KDialog::spacingHint());

  grid->addRowSpacing(0, gb_Presets->fontMetrics().height());
  grid->setColStretch(1, 1);
  grid->setColStretch(2, 1);
  grid->setColStretch(0, 1);
    
  lb_PresetList = new QListBox(gb_Presets, "Preset list",0);

  grid->addMultiCellWidget(lb_PresetList, 1, 1, 0, 2, 0);

  bt_Add = new QPushButton(i18n("Add..."), gb_Presets);
  grid->addWidget(bt_Add,2,0);

  bt_Remove = new QPushButton(i18n("Remove..."), gb_Presets);
  grid->addWidget(bt_Remove,2,1);

  bt_Save = new QPushButton(i18n("Save"), gb_Presets);
  grid->addWidget(bt_Save,2,2);

  
  gb_Show = new QGroupBox(i18n("Show"), this);
  layout->addWidget(gb_Show);

  QGridLayout *grid2 = new QGridLayout(gb_Show,5,2,
                                       KDialog::marginHint(),
                                       KDialog::spacingHint());

  grid2->addRowSpacing(0, gb_Show->fontMetrics().height());
  grid2->setColStretch(0, 1);
  grid2->setColStretch(1, 1);

  cb_Technical = new QCheckBox(i18n("Technical details as list"),gb_Show, "Technical");
  grid2->addMultiCellWidget(cb_Technical, 1, 1, 0, 1, 0);

  cb_Signal = new QCheckBox(i18n("Signal number"), gb_Show, "Signalnum");
  grid2->addMultiCellWidget(cb_Signal, 2, 2, 0, 1, 0);

  cb_SignalDetail = new QCheckBox(i18n("Detailed signal description"), gb_Show, "DetailSig");
  grid2->addWidget(cb_SignalDetail,3,1);

  cb_WhatToDo = new QCheckBox(i18n("What-to-do hint"), gb_Show, "Whattodo");
  grid2->addMultiCellWidget(cb_WhatToDo, 4, 4, 0, 1, 0);

  cb_ShowBugReport = new QCheckBox(i18n("Show bug report button"), gb_Show, "BugReport");
  grid2->addMultiCellWidget(cb_ShowBugReport, 5, 5, 0, 1, 0);

  grid2->setColStretch(0,1);
  grid2->setColStretch(1,10);
  layout->addStretch(0);
  layout->activate();         
/*  bGrp = new QButtonGroup(1, Qt::Vertical,
						  i18n("Sample optionss"), this);
  connect(bGrp, SIGNAL(clicked(int)), this, SLOT(configChanged()));

  topLayout->addWidget(bGrp);

  button1 = new QRadioButton(i18n("&Some option"), bGrp);
  button2 = new QRadioButton(i18n("Some &other option"), bGrp);
  button3 = new QRadioButton(i18n("&Yet another option"), bGrp);
*/
  load();
}

void KDrKonqiCustomize::load()
{
 // Set all options to the config file settings here
}

void KDrKonqiCustomize::defaults()
{
 // Load the defaults here
}

void KDrKonqiCustomize::save()
{
 // Save the settings here
}

void KDrKonqiCustomize::changed()
{
  emit KCModule::changed(true);
}

