/*
 *  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>
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

#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <kapp.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>

#include "kcmlaunch.h"

extern "C"
{
  KCModule * create_launch(QWidget * parent, const char * name)
  {
    KGlobal::locale()->insertCatalogue("kcmlaunch");

    return new LaunchConfig(parent, name);
  }
}

LaunchConfig::LaunchConfig(QWidget * parent, const char * name)
  : KCModule(parent, name)
{
  QGroupBox * groupBox = new QGroupBox
    (
     2,
     Vertical,
     i18n("When an application is starting"),
     this
    );

  cb_busyCursor_ =
    new QCheckBox(i18n("Change the pointer shape"), groupBox);

  cb_taskbarButton_ =
    new QCheckBox
    (
     i18n("Show a button in the taskbar with an animated disk"),
     groupBox
    );

  QVBoxLayout * layout =
    new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

  layout->addWidget(groupBox);
  layout->addStretch(100);

  load();

  connect
    (
     cb_busyCursor_,
     SIGNAL(toggled(bool)),
     SLOT(slotBusyCursorToggled(bool))
    );

  connect
    (
     cb_taskbarButton_,
     SIGNAL(toggled(bool)),
     SLOT(slotTaskbarButtonToggled(bool))
    );
}

LaunchConfig::~LaunchConfig()
{
}

  void
LaunchConfig::load()
{
  KConfig c("klaunchrc", false, false);

  c.setGroup("FeedbackStyle");

  bool busyCursor =
    c.readBoolEntry("BusyCursor", Default & BusyCursor);

  bool taskbarButton =
    c.readBoolEntry("TaskbarButton", Default & TaskbarButton);

  cb_busyCursor_     ->setChecked(busyCursor);
  cb_taskbarButton_  ->setChecked(taskbarButton);

  emit(changed(false));
}

  void
LaunchConfig::save()
{
  KConfig c("klaunchrc", false, false);

  c.setGroup("FeedbackStyle");

  c.writeEntry("BusyCursor",    cb_busyCursor_     ->isChecked());
  c.writeEntry("TaskbarButton", cb_taskbarButton_  ->isChecked());

  c.sync();

  emit(changed(false));
}

  void
LaunchConfig::defaults()
{
  cb_busyCursor_     ->setChecked(Default & BusyCursor);
  cb_taskbarButton_  ->setChecked(Default & TaskbarButton);

  checkChanged();
}

  void
LaunchConfig::slotBusyCursorToggled(bool)
{
  checkChanged();
}

  void
LaunchConfig::slotTaskbarButtonToggled(bool)
{
  checkChanged();
}

  void
LaunchConfig::checkChanged()
{
  KConfig c("klaunchrc", false, false);

  c.setGroup("FeedbackStyle");

  bool savedBusyCursor =
    c.readBoolEntry("BusyCursor", Default & BusyCursor);

  bool savedTaskbarButton =
    c.readBoolEntry("TaskbarButton", Default & TaskbarButton);

  bool newBusyCursor =
    cb_busyCursor_->isChecked();

  bool newTaskbarButton =
    cb_taskbarButton_->isChecked();

  emit
    (
     changed
     (
      savedBusyCursor     != newBusyCursor
      ||
      savedTaskbarButton  != newTaskbarButton
     )
    );
}

  QString
LaunchConfig::quickHelp() const
{
  return i18n
    (
     "<h1>Launch</h1>"
     " You can configure the application-launch feedback here."
    );
}

#include "kcmlaunch.moc"
