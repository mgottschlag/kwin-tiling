/*
 *  kcmtaskbar.cpp
 *
 *  Copyright (c) 2000 Kurt Granroth <granroth@kde.org>
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
 */
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>

#include <dcopclient.h>
#include <kapp.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>

#include "kcmtaskbar.h"

TaskbarConfig::TaskbarConfig( QWidget *parent, const char* name )
  : KCModule (parent, name)
{
  showAllCheck = new QCheckBox(i18n("Show &all windows"), this);
  connect(showAllCheck, SIGNAL(clicked()), SLOT(configChanged()));
  QWhatsThis::add(showAllCheck, i18n("Check this option if you want"
    " the taskbar to display all of the existing windows at once.  By"
    " default, the taskbar will only show those windows that are on"
    " the current desktop."));

  externalCheck = new QCheckBox(i18n("Run &outside of panel"), this);
  connect(externalCheck, SIGNAL(clicked()), SLOT(configChanged()));
  connect(externalCheck, SIGNAL(clicked()), SLOT(slotExternal()));
  QWhatsThis::add(externalCheck, i18n("Check this option if you want"
    " the taskbar to exist outside of the panel.  By default, the"
    " taskbar will be embeded into the panel."));

  // position group
  positionGroup = new QButtonGroup(2, Qt::Horizontal, i18n("Position"), this);
  connect(positionGroup, SIGNAL(clicked(int)),
          this,          SLOT(slotPosChanged(int)));

  QRadioButton *rb;
  QRadioButton *left = new QRadioButton(i18n("&Left"), positionGroup);
  QWhatsThis::add(left, i18n("Dock the taskbar on the left side of the screen"));
  QRadioButton *right = new QRadioButton(i18n("&Right"), positionGroup);
  QWhatsThis::add(right, i18n("Dock the taskbar on the right side of the screen"));
  rb = new QRadioButton(i18n("&Top"), positionGroup);
  QWhatsThis::add(rb, i18n("Dock the taskbar on the top of the screen"));
  rb = new QRadioButton(i18n("&Bottom"), positionGroup);
  QWhatsThis::add(rb, i18n("Dock the taskbar on the bottom of the screen"));

  // disable Left and Right until after KDE 2.0
  left->setEnabled(false);
  right->setEnabled(false);

  QVBoxLayout *top_layout = new QVBoxLayout(this, KDialog::marginHint(),
                                                  KDialog::spacingHint());
  top_layout->addWidget(showAllCheck);
  top_layout->addWidget(externalCheck);
  top_layout->addWidget(positionGroup);
  top_layout->addStretch(1);

  load();
}

TaskbarConfig::~TaskbarConfig()
{
}

void TaskbarConfig::configChanged()
{
  emit changed(true);
}

void TaskbarConfig::load()
{
  KConfig *c = new KConfig("ktaskbarappletrc", false, false);
  { // group for the benefit of the group saver
  KConfigGroupSaver saver(c, "General");

  showAllCheck->setChecked(c->readBoolEntry("ShowAllWindows", false));
  bool is_top_level = c->readBoolEntry("TopLevel", false);
  externalCheck->setChecked(is_top_level);
  positionGroup->setEnabled(is_top_level);
  positionGroup->setButton(c->readNumEntry("Position", 3));

  }

  delete c;
  emit changed(false);
}

void TaskbarConfig::save()
{
  KConfig *c = new KConfig("ktaskbarappletrc", false, false);
  { // group for the benefit of the group saver
  KConfigGroupSaver saver(c, "General");

  c->writeEntry("ShowAllWindows", showAllCheck->isChecked());
  c->writeEntry("TopLevel", externalCheck->isChecked());
  if (externalCheck->isChecked())
    c->writeEntry("Position", positionGroup->id(positionGroup->selected()));
  c->sync();
  }

  delete c;

  emit changed(false);

  // Tell kicker about the new config file.
  if (!kapp->dcopClient()->isAttached())
    kapp->dcopClient()->attach();
  QByteArray data;
  kapp->dcopClient()->send( "kicker", "Panel", "restart()", data );
}

void TaskbarConfig::defaults()
{
  showAllCheck->setChecked(false);
  externalCheck->setChecked(false);
  positionGroup->setEnabled(false);

  emit changed(true);
}

QString TaskbarConfig::quickHelp() const
{
  return i18n("<h1>Taskbar</h1> You can configure the taskbar here."
    " This includes options such as whether or not the taskbar should be"
    " embedded in the panel or float outside (default: embedded).  You"
    " can also configure whether or not the taskbar should show all"
    " windows at once or only those on the current desktop");
}

void TaskbarConfig::slotExternal()
{
  positionGroup->setEnabled(externalCheck->isChecked());
}

void TaskbarConfig::slotPosChanged(int)
{
  emit changed(true);
}

extern "C"
{
  KCModule *create_taskbar(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("taskbar");
    return new TaskbarConfig(parent, name);
  };
}
#include "kcmtaskbar.moc"
