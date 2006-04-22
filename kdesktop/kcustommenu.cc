/* This file is part of the KDE project
   Copyright (C) 2001 Waldo Bastian <bastian@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <qimage.h>
#include <qregexp.h>

#include <kconfig.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <ktoolinvocation.h>

#include "kcustommenu.h"

class KCustomMenu::KCustomMenuPrivate
{
public:
   QMap<int,KService::Ptr> entryMap;
};

KCustomMenu::KCustomMenu(const QString &configfile, QWidget *parent)
   : QMenu(parent)
{
  setObjectName("kcustom_menu");
  d = new KCustomMenuPrivate; 
  
  KConfig cfg(configfile, true, false);
  int count = cfg.readEntry("NrOfItems", 0);
  for(int i = 0; i < count; i++)
  {
     QString entry = cfg.readEntry(QString("Item%1").arg(i+1), QString());
     if (entry.isEmpty())
        continue;

     // Try KSycoca first.
     KService::Ptr menuItem = KService::serviceByDesktopPath( entry );
     if (!menuItem)
        menuItem = KService::serviceByDesktopName( entry );
     if (!menuItem)
        menuItem = new KService( entry );

     if (!menuItem->isValid())
        continue;
 
     insertMenuItem( menuItem, -1 );
  }
  connect(this, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
}

KCustomMenu::~KCustomMenu()
{
  delete d;
}

void
KCustomMenu::slotActivated(int id)
{
  KService::Ptr s = d->entryMap[id];
  if (!s)
     return;
  KToolInvocation::startServiceByDesktopPath(s->desktopEntryPath());
}

// The following is copied from kicker's PanelServiceMenu
void 
KCustomMenu::insertMenuItem(KService::Ptr & s, int nId, int nIndex/*= -1*/)
{
    QString serviceName = s->name();

    // item names may contain ampersands. To avoid them being converted
    // to accelators, replace them with two ampersands.
    serviceName.replace("&", "&&");

    QPixmap normal = KGlobal::instance()->iconLoader()->loadIcon(s->icon(), K3Icon::Small,
                                                                 0, K3Icon::DefaultState, 0L, true);
    QPixmap active = KGlobal::instance()->iconLoader()->loadIcon(s->icon(), K3Icon::Small,
                                                                 0, K3Icon::ActiveState, 0L, true);
    // make sure they are not larger than 16x16
    if (normal.width() > 16 || normal.height() > 16) {
        QImage tmp = normal.toImage();
        tmp = tmp.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        normal.convertFromImage(tmp);
    }
    if (active.width() > 16 || active.height() > 16) {
        QImage tmp = active.toImage();
        tmp = tmp.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        active.convertFromImage(tmp);
    }

    QIcon iconset;
    iconset.setPixmap(normal, QIcon::Small, QIcon::Normal);
    iconset.setPixmap(active, QIcon::Small, QIcon::Active);

    int newId = insertItem(iconset, serviceName, nId, nIndex);
    d->entryMap.insert(newId, s);
}

#include "kcustommenu.moc"
