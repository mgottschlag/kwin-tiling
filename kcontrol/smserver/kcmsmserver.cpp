/*
 *  kcmsmserver.cpp
 *  Copyright (c) 2000 Oswald Buddenhagen <ob6@inf.tu-dresden.de>
 *
 *  based on kcmtaskbar.cpp
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
#include <qcheckbox.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <dcopclient.h>
#include <kapp.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>

#include "kcmsmserver.h"

SMServerConfig::SMServerConfig( QWidget *parent, const char* name )
  : KCModule (parent, name)
{
  confirmLogoutCheck = new QCheckBox(i18n("Confirm &logout"), this);
  connect(confirmLogoutCheck, SIGNAL(clicked()), SLOT(configChanged()));
  QWhatsThis::add(confirmLogoutCheck, i18n("Check this option if you want"
    " the session manager to display a logout confirmation dialog box."));

  saveSessionCheck = new QCheckBox(
    i18n("&Restore previous session when logging in"), this);
  connect(saveSessionCheck, SIGNAL(clicked()), SLOT(configChanged()));
  QWhatsThis::add(saveSessionCheck, i18n("Check this option if you want"
    " the session manager to save the running session when logging out"
    " and restore it on the next login."));

  QVBoxLayout *top_layout = new QVBoxLayout(this, KDialog::marginHint(),
                                                  KDialog::spacingHint());
  top_layout->addWidget(confirmLogoutCheck);
  top_layout->addWidget(saveSessionCheck);
  
  top_layout->addStretch(1);

  load();
}

SMServerConfig::~SMServerConfig()
{
}

void SMServerConfig::configChanged()
{
  emit changed(true);
}

void SMServerConfig::load()
{
  KConfig *c = new KConfig("ksmserverrc", false, false);
  c->setGroup("General");
  confirmLogoutCheck->setChecked(c->readBoolEntry("confirmLogout", true));
  saveSessionCheck->setChecked(c->readBoolEntry("saveSession", true));
  delete c;

  emit changed(false);
}

void SMServerConfig::save()
{
  KConfig *c = new KConfig("ksmserverrc", false, false);
  c->setGroup("General");
  c->writeEntry( "saveSession", saveSessionCheck->isChecked());
  c->writeEntry( "confirmLogout", confirmLogoutCheck->isChecked());
  c->sync();
  delete c;

  emit changed(false);
}

void SMServerConfig::defaults()
{
  saveSessionCheck->setChecked(true);
  confirmLogoutCheck->setChecked(true);

  emit changed(true);
}

QString SMServerConfig::quickHelp() const
{
  return i18n("<h1>Session Manager</h1>"
    " You can configure the session manager here."
    " This includes options such as whether or not the session exit (logout)"
    " should be confirmed and if the previous session should be restored"
    " when logging in.");
}

extern "C"
{
  KCModule *create_smserver(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmsmserver");
    return new SMServerConfig(parent, name);
  };
}
#include "kcmsmserver.moc"

