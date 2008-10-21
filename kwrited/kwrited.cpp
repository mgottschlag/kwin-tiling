/*
    kwrited is a write(1) receiver for KDE.
    Copyright 1997,1998 by Lars Doelle <lars.doelle@on-line.de>
    Copyright 2008 by George Kiagiadakis <gkiagia@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Own
#include "kwrited.h"

#include <kdebug.h>
#include <kptydevice.h>
#include <kuser.h>
#include <knotification.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>

K_PLUGIN_FACTORY(KWritedFactory,
                 registerPlugin<KWritedModule>();
    )
K_EXPORT_PLUGIN(KWritedFactory("kwrited"))


KWrited::KWrited() : QObject()
{
  pty = new KPtyDevice();
  pty->open();
  pty->login(KUser().loginName().toLocal8Bit().data(), qgetenv("DISPLAY"));
  connect(pty, SIGNAL(readyRead()), this, SLOT(block_in()));

  kDebug() << "listening on device" << pty->ttyName();
}

KWrited::~KWrited()
{
    pty->logout();
    delete pty;
}

void KWrited::block_in()
{
  QByteArray buf = pty->readAll();
  QString msg = QString::fromLocal8Bit( buf.constData(), buf.size() );
  msg.remove('\r');
  msg.remove('\a');

  KNotification *notification = new KNotification("NewMessage", 0, KNotification::Persistent);
  notification->setComponentData( KWritedFactory::componentData() );
  notification->setText( msg );
  connect(notification, SIGNAL(closed()), notification, SLOT(deleteLater()) );
  notification->sendEvent();
}

KWritedModule::KWritedModule(QObject* parent, const QList<QVariant>&)
    : KDEDModule(parent)
{
    pro = new KWrited;
}

KWritedModule::~KWritedModule()
{
    delete pro;
}

#include "kwrited.moc"
