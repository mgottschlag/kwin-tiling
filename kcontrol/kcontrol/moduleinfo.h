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



#ifndef _MODULEINFO_H_
#define _MODULEINFO_H_


#include <qobject.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qstringlist.h>


class ModuleInfo : public QObject
{
  Q_OBJECT 

public:

  ModuleInfo(QString desktopFile);

  const QString fileName() const { return _fileName; };
  const QStringList &groups() const { return _groups; };
  const QStringList &keywords() const { return _keywords; };
  QString name() const { return _name; };
  QPixmap icon();
  bool isDirectory() const { return _directory; };
  QString library() const { return _lib; };
  QString handle() const { return _handle; };
  bool onlyRoot() const { return _root; };
  QString docPath() const { return _doc; };
  QCString moduleId() const;

protected:

  void setGroups(QStringList &groups) { _groups = groups; };
  void setKeywords(QStringList &k) { _keywords = k; };
  void setName(QString name) { _name = name; };
  void setIcon(QString icon) { _icon = icon; };
  void setDirectory(bool dir) { _directory = dir; };
  void setLibrary(QString lib) { _lib = lib; };
  void setHandle(QString handle) { _handle = handle; };
  void setOnlyRoot(bool only) { _root = only; };
  void setDocPath(QString p) { _doc = p; };

private:
  
  QStringList _groups, _keywords;
  QString     _name, _icon, _lib, _handle, _fileName, _doc;
  bool        _directory, _root;
};


#endif
