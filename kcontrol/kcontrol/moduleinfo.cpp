/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>

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

#include <qimage.h>

#include <kiconloader.h>
#include <kdesktopfile.h>

#include "moduleinfo.h"
#include "moduleinfo.moc"
#include "utils.h"


ModuleInfo::ModuleInfo(QString desktopFile)
  : _fileName(desktopFile)
{
  KDesktopFile desktop(desktopFile);

  // set the modules simple attributes
  setName(desktop.readName());
  setComment(desktop.readComment());
  setIcon(desktop.readIcon());

  // library and factory
  setLibrary(desktop.readEntry("X-KDE-LibraryName"));
  setHandle(desktop.readEntry("X-KDE-FactoryName", library()));

  // does the module need super user privileges?
  setNeedsRootPrivileges(desktop.readBoolEntry("X-KDE-NeedsRootPrivileges", false));

  // does the module implement a read-only mode?
  setHasReadOnlyMode(desktop.readBoolEntry("X-KDE-HasReadOnlyMode", false));

  // get the documentation path
  setDocPath(desktop.readEntry("DocPath"));

  // get the keyword list
  QStringList kw = desktop.readListEntry("Keywords");
  setKeywords(kw);
 
  // try to find out the modules groups
  QString group = desktop.readEntry("X-KDE-Group");
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

QCString ModuleInfo::moduleId() const
{
  QString res;
  
  QStringList::ConstIterator it;
  for (it = _groups.begin(); it != _groups.end(); ++it)
    res.append(*it+"-");
  res.append(name());

  return res.ascii();
}
