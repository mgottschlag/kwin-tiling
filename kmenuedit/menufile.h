/*
 *   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as 
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef __menufile_h__
#define __menufile_h__

#include <qdom.h>
#include <qstring.h>

class MenuFile
{
public:
   MenuFile(const QString &file);
   ~MenuFile();
   
   bool load();
   bool save();
   void create();
   
   void addEntry(const QString &menuName, const QString &entry);
   void removeEntry(const QString &menuName, const QString &entry);
   
   void addMenu(const QString &menuName, const QString &menuFile);
   void moveMenu(const QString &oldMenu, const QString &newMenu);
   void removeMenu(const QString &menuName);

   /**
    * Returns a unique menu-name for a new menu under @p menuName 
    * inspired by @p newMenu and not part of @p excludeList
    */
   QString uniqueMenuName(const QString &menuName, const QString &newMenu, const QStringList &excludeList);

protected:
   /**
    * Finds menu @p menuName in @p elem. 
    * If @p create is true, the menu is created if it doesn't exist yet.
    * @return The menu dom-node of @p menuName
    */
   QDomElement findMenu(QDomElement elem, const QString &menuName, bool create);
   
private:
   QString m_fileName;

   QDomDocument m_doc;
   bool m_bDirty;
};


#endif
