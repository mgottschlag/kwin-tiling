/*
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>

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
  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <qheader.h>
#include <qcursor.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kservicegroup.h>
#include <kiconloader.h>

#include <kdebug.h>

#include "moduleiconview.h"
#include "moduleiconview.moc"
#include "modules.h"
#include "global.h"


ModuleIconView::ModuleIconView(ConfigModuleList *list, QWidget * parent, const char * name)
  : KListView(parent, name)
  , _path(KCGlobal::baseGroup())
  , _modules(list)
{
  setSorting(1, true);
  addColumn(QString::null);

  // Needed to enforce a cut of the items label rather than
  // showing a horizontal scrollbar
  setResizeMode(LastColumn);

  header()->hide();

  // This is intentionally _not_ connected with executed(), since
  // honoring doubleclick doesn't make any sense here (changed by
  // large user demand)
  connect(this, SIGNAL(clicked(QListViewItem*)),
          this, SLOT(slotItemSelected(QListViewItem*)));
}

void ModuleIconView::makeSelected(ConfigModule *m)
{
  if (!m) return;

  for (QListViewItem *i = firstChild(); i; i = i->nextSibling())
  {
     if(static_cast<ModuleIconItem*>(i)->module() == m)
     {
        setSelected(i, true);
        break;
     }
  }
}

void ModuleIconView::makeVisible(ConfigModule *m)
{
  if (!m) return;
  QString tmp = _modules->findModule(m);
  if (tmp.isEmpty())
     return;

  _path = tmp;
  fill();
}

void ModuleIconView::fill()
{
  clear();

  QPixmap icon;
  // add our "up" icon if we aren't top level
  if (_path != KCGlobal::baseGroup())
  {
     icon = loadIcon( "back" );
     // go-back node
     ModuleIconItem *i = new ModuleIconItem(this, i18n("Back"), icon);
     i->setOrderNo(0);
     int last_slash = _path.findRev('/', -2);
     if (last_slash == -1)
        i->setTag(QString::null);
     else
        i->setTag(_path.left(last_slash+1));
  }

  int c = 0;
  QStringList submenus = _modules->submenus(_path);
  for (QStringList::Iterator it = submenus.begin(); it != submenus.end(); ++it )
  {
     QString path = (*it);

     KServiceGroup::Ptr group = KServiceGroup::group(path);
     if (!group || !group->isValid())
        continue;

     icon = loadIcon( group->icon() );

     ModuleIconItem *i = new ModuleIconItem(this, group->caption(), icon);
     i->setTag(path);
     i->setOrderNo(++c);
  }

  c = 0;
  QPtrList<ConfigModule> moduleList = _modules->modules(_path);
  for (ConfigModule *module=moduleList.first(); module != 0; module=moduleList.next())
  {
     icon = loadIcon( module->icon() );

     ModuleIconItem *i = new ModuleIconItem(this, module->moduleName(), icon, module);
     i->setOrderNo(++c);
  }
}

void ModuleIconView::slotItemSelected(QListViewItem* item)
{
  QApplication::restoreOverrideCursor();
  if (!item) return;

  if (static_cast<ModuleIconItem*>(item)->module())
  {
     emit moduleSelected(static_cast<ModuleIconItem*>(item)->module());
  }
  else
  {
     _path = static_cast<ModuleIconItem*>(item)->tag();
     fill();
     setCurrentItem(firstChild());
  }
}

void ModuleIconView::keyPressEvent(QKeyEvent *e)
{
  if(   e->key() == Key_Return
     || e->key() == Key_Enter
     || e->key() == Key_Space)
  {
     if (currentItem())
        slotItemSelected(currentItem());
  }
  else
  {
     KListView::keyPressEvent(e);
  }
}

QPixmap ModuleIconView::loadIcon( const QString &name )
{
  QPixmap icon = DesktopIcon( name, KCGlobal::iconSize() );

  if(icon.isNull())
     icon = DesktopIcon( "folder", KCGlobal::iconSize() );

  return icon;
}
