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
#include <qspinbox.h>
#include <qlabel.h>

#include <kapp.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <dcopclient.h>

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
    new QCheckBox(i18n("Show an icon next to the mouse cursor"), groupBox);

  cb_taskbarButton_ =
    new QCheckBox
    (
     i18n("Show a button in the taskbar with an animated disk"),
     groupBox
    );

  QVBoxLayout * layout =
    new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

  layout->addWidget(groupBox);

  gb_cursor = new QGroupBox( 1, Vertical, i18n( "Busy cursor settings" ), this );
  ( void ) new QLabel( i18n( "Startup indication timeout (seconds) : " ), gb_cursor );
  sb_cursorTimeout = new QSpinBox( 0, 60, 1, gb_cursor );
  layout->addWidget(gb_cursor);

  gb_taskbar = new QGroupBox( 1, Vertical, i18n( "Taskbar settings" ), this );
  ( void )  new QLabel( i18n( "Startup indication timeout (seconds) : " ), gb_taskbar );
  sb_taskbarTimeout = new QSpinBox( 0, 60, 1, gb_taskbar );
  layout->addWidget(gb_taskbar);

  layout->addStretch(100);

  load();

  connect
    (
     cb_busyCursor_,
     SIGNAL(toggled(bool)),
     SLOT( checkChanged())
    );
  connect
    (
     cb_busyCursor_,
     SIGNAL(toggled(bool)),
     gb_cursor,
     SLOT( setEnabled(bool))
    );

  connect
    (
     cb_taskbarButton_,
     SIGNAL(toggled(bool)),
     SLOT( checkChanged())
    );
  connect
    (
     cb_taskbarButton_,
     SIGNAL(toggled(bool)),
     gb_taskbar,
     SLOT( setEnabled(bool))
    );

  connect
    (
     sb_cursorTimeout,
     SIGNAL(valueChanged(int)),
     SLOT( checkChanged())
    );

  connect
    (
     sb_taskbarTimeout,
     SIGNAL(valueChanged(int)),
     SLOT( checkChanged())
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
  
  gb_cursor->setEnabled( busyCursor );
  gb_taskbar->setEnabled( taskbarButton );

  c.setGroup( "BusyCursorSettings" );
  sb_cursorTimeout->setValue( c.readUnsignedNumEntry( "Timeout", 30 ));

  c.setGroup( "TaskbarButtonSettings" );
  sb_taskbarTimeout->setValue( c.readUnsignedNumEntry( "Timeout", 30 ));

  emit(changed(false));
}

  void
LaunchConfig::save()
{
  KConfig c("klaunchrc", false, false);

  c.setGroup("FeedbackStyle");

  c.writeEntry("BusyCursor",    cb_busyCursor_     ->isChecked());
  c.writeEntry("TaskbarButton", cb_taskbarButton_  ->isChecked());
  
  c.setGroup( "BusyCursorSettings" );
  c.writeEntry( "Timeout", sb_cursorTimeout->value());
 
  c.setGroup( "TaskbarButtonSettings" );
  c.writeEntry( "Timeout", sb_taskbarTimeout->value());

  c.sync();

  emit(changed(false));

  if (!kapp->dcopClient()->isAttached())
     kapp->dcopClient()->attach();
  QByteArray data;
  kapp->dcopClient()->send( "kicker", "Panel", "restart()", data );
  kapp->dcopClient()->send( "kdesktop", "", "configure()", data );
}

  void
LaunchConfig::defaults()
{
  cb_busyCursor_     ->setChecked(Default & BusyCursor);
  cb_taskbarButton_  ->setChecked(Default & TaskbarButton);

  sb_cursorTimeout->setValue( 30 );
  sb_taskbarTimeout->setValue( 30 );
  
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

  c.setGroup( "BusyCursorSettings" );
  unsigned int savedCursorTimeout = c.readUnsignedNumEntry( "Timeout", 30 );

  c.setGroup( "TaskbarButtonSettings" );
  unsigned int savedTaskbarTimeout = c.readUnsignedNumEntry( "Timeout", 30 );

  bool newBusyCursor =
    cb_busyCursor_->isChecked();

  bool newTaskbarButton =
    cb_taskbarButton_->isChecked();

  unsigned int newCursorTimeout = sb_cursorTimeout->value();
  
  unsigned int newTaskbarTimeout = sb_taskbarTimeout->value();
  
  emit
    (
     changed
     (
      savedBusyCursor     != newBusyCursor
      ||
      savedTaskbarButton  != newTaskbarButton
      ||
      savedCursorTimeout  != newCursorTimeout
      ||
      savedTaskbarTimeout != newTaskbarTimeout
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
