/*

  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 
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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
*/                                                                            



#include <kiconloader.h>
#include <kdesktopfile.h>


#include "moduleinfo.h"
#include "moduleinfo.moc"
#include "utils.h"


ModuleInfo::ModuleInfo(QString desktopFile)
  : _fileName(desktopFile)
{
  KDesktopFile desktop(desktopFile);

  // find out about the type of the module
  ModuleType t = Executable;
  if (desktop.readEntry("ModuleType", "Executable") == "Library")
    t = Library;
  setType(t);

  if (t == Library)
    {
      setLibrary(desktop.readEntry("LibraryName"));
      setHandle(desktop.readEntry("FactoryName", library()));
    }
  else
    setExecutable(desktop.readEntry("Exec"));

  // executable only with root access?
  setOnlyRoot(desktop.readBoolEntry("RootOnly", false));

  // user or system settings?
  setLocalUser(desktop.readBoolEntry("LocalUser", true));

  // set the modules' simple attributes
  setName(desktop.readName());
  QString icon = desktop.readMiniIcon();
  if (icon.isEmpty())
    icon = desktop.readIcon();
  setIcon(icon);
  
  // try to find out the modules' groups
  QString group = desktop.readEntry("Group");
  if (group.isEmpty())
    {	  
      group = desktopFile; 
      int pos = group.find("Settings/");
      if (pos >= 0)
	group = group.mid(pos+9);
      pos = group.findRev('/');
      if (pos >= 0)
	group = group.left(pos);
    }
  QStringList groups;
  splitString(group, '/', groups);
  setGroups(groups);
}


QPixmap ModuleInfo::icon()
{
  QPixmap icon = KGlobal::iconLoader()->loadApplicationIcon(_icon, KIconLoader::Small);
  if (icon.isNull())
    icon = BarIcon("unknown");
  return icon;
}


QCString ModuleInfo::moduleId() const
{
  QString res;
  
  QStringList::ConstIterator it;
  for (it = _groups.begin(); it != _groups.end(); ++it)
    res.append(*it+"-");
  res.append(name());

  return res.ascii();
}
