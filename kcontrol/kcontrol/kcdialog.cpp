/*
   Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (c) 2000 Matthias Elter <elter@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

#include <klocale.h>
#include <kapp.h>

#include "kcdialog.h"
#include "kcdialog.moc"
#include "kcmodule.h"

KCDialog::KCDialog(KCModule *client, int b, const QString &docpath, QWidget *parent, const char *name, bool modal)
  : KDialogBase(parent, name, modal, QString::null, 
	(b & KCModule::Help ? Help : 0) |
	(b & KCModule::Default ? Default : 0) |
	(b & KCModule::Reset ? User1 : 0) |
	(b & KCModule::Cancel ? Cancel : 0) |
	(b & KCModule::Apply ? Apply : 0) |
	(b & KCModule::Ok ? Ok : 0),
	Ok, false, i18n("&Reset")),
    _client(client)
{
  client->reparent(this,0,QPoint(0,0),true);
  setMainWidget(client);
  connect(client, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));

  setHelp( docpath, QString::null );

  // disable initial buttons
  enableButton(User1, false);
  enableButton(Apply, false);
}

void KCDialog::slotDefault()
{
  _client->defaults();
  clientChanged(true);
}


void KCDialog::slotUser1()
{
  _client->load();
  clientChanged(false);
}

void KCDialog::slotApply()
{
  _client->save();
  clientChanged(false);
}


void KCDialog::slotOk()
{
  _client->save();
  accept();
}

void KCDialog::clientChanged(bool state)
{
  // enable/disable buttons
  enableButton(User1, state);
  enableButton(Apply, state);
}
