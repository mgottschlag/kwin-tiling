/* This file is part of the KDE project
   Copyright (C) 2002 Waldo Bastian <bastian@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include <qradiobutton.h>
#include <qspinbox.h>
#include <kconfig.h>

#include "kxmlrpcdlg.h"
#include "kxmlrpcdlg.moc"

#include "kxmlrpcdlgbase.h"

KXmlRpcDialog::KXmlRpcDialog(QWidget* parent, const char* name)
 : KDialogBase(parent, name, true, QString::null,
                 KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, false)
{
  mWidget = new KXmlRpcDialogBase(this);
  setMainWidget(mWidget);

  mConfig = new KConfig("kxmlrpcdrc", false, false);
  mConfig->setGroup("General");
  int port = mConfig->readUnsignedNumEntry("Port", 0);

  if (port)
  {
     mWidget->manualPort->setChecked(true);
     mWidget->port->setValue(port);
  }
  else
  {
     mWidget->port->setValue(18300);
     mWidget->autoPort->setChecked(true);
  }
}

KXmlRpcDialog::~KXmlRpcDialog()
{
  delete mConfig;
}

void KXmlRpcDialog::slotOk()
{
  int port = 0;
  if (mWidget->manualPort->isChecked())
     port = mWidget->port->value();
  mConfig->setGroup("General");
  mConfig->writeEntry("Port", port);

  accept();
}


