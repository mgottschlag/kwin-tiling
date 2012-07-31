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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "menufile.h"

#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QFileInfo>

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>


#define MF_MENU         "Menu"
#define MF_PUBLIC_ID    "-//freedesktop//DTD Menu 1.0//EN"
#define MF_SYSTEM_ID    "http://www.freedesktop.org/standards/menu-spec/1.0/menu.dtd"
#define MF_NAME         "Name"
#define MF_INCLUDE      "Include"
#define MF_EXCLUDE      "Exclude"
#define MF_FILENAME     "Filename"
#define MF_DELETED      "Deleted"
#define MF_NOTDELETED   "NotDeleted"
#define MF_MOVE         "Move"
#define MF_OLD          "Old"
#define MF_NEW          "New"
#define MF_DIRECTORY    "Directory"
#define MF_LAYOUT       "Layout"
#define MF_MENUNAME     "Menuname"
#define MF_SEPARATOR    "Separator"
#define MF_MERGE        "Merge"

MenuFile::MenuFile(const QString &file)
 : m_fileName(file), m_bDirty(false)
{
   load();
}

MenuFile::~MenuFile()
{
}

bool MenuFile::load()
{
   if (m_fileName.isEmpty())
      return false;

   QFile file( m_fileName );
   if (!file.open( QIODevice::ReadOnly ))
   {
       if ( file.exists() )
           kWarning() << "Could not read " << m_fileName ;
       create();
       return false;
   }

   QString errorMsg;
   int errorRow;
   int errorCol;
   if ( !m_doc.setContent( &file, &errorMsg, &errorRow, &errorCol ) ) {
      kWarning() << "Parse error in " << m_fileName << ", line " << errorRow << ", col " << errorCol << ": " << errorMsg ;
      file.close();
      create();
      return false;
   }
   file.close();

   return true;
}

void MenuFile::create()
{
   QDomImplementation impl;
   QDomDocumentType docType = impl.createDocumentType( MF_MENU, MF_PUBLIC_ID, MF_SYSTEM_ID );
   m_doc = impl.createDocument(QString(), MF_MENU, docType);
}

bool MenuFile::save()
{
   QFile file( m_fileName );

   if (!file.open( QIODevice::WriteOnly ))
   {
      kWarning() << "Could not write " << m_fileName ;
      m_error = i18n("Could not write to %1", m_fileName);
      return false;
   }
   QTextStream stream( &file );
   stream.setCodec( "UTF-8" );

   stream << m_doc.toString();

   file.close();

   if (file.error() != QFile::NoError)
   {
      kWarning() << "Could not close " << m_fileName ;
      m_error = i18n("Could not write to %1", m_fileName);
      return false;
   }

   m_bDirty = false;

   return true;
}

QDomElement MenuFile::findMenu(QDomElement elem, const QString &menuName, bool create)
{
   QString menuNodeName;
   QString subMenuName;
   int i = menuName.indexOf('/');
   if (i >= 0)
   {
      menuNodeName = menuName.left(i);
      subMenuName = menuName.mid(i+1);
   }
   else
   {
      menuNodeName = menuName;
   }
   if (i == 0)
      return findMenu(elem, subMenuName, create);

   if (menuNodeName.isEmpty())
      return elem;

   QDomNode n = elem.firstChild();
   while( !n.isNull() )
   {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if (e.tagName() == MF_MENU)
      {
         QString name;

         QDomNode n2 = e.firstChild();
         while ( !n2.isNull() )
         {
            QDomElement e2 = n2.toElement();
            if (!e2.isNull() && e2.tagName() == MF_NAME)
            {
               name = e2.text();
               break;
            }
            n2 = n2.nextSibling();
         }

         if (name == menuNodeName)
         {
            if (subMenuName.isEmpty())
               return e;
            else
               return findMenu(e, subMenuName, create);
         }
      }
      n = n.nextSibling();
   }

   if (!create)
      return QDomElement();

   // Create new node.
   QDomElement newElem = m_doc.createElement(MF_MENU);
   QDomElement newNameElem = m_doc.createElement(MF_NAME);
   newNameElem.appendChild(m_doc.createTextNode(menuNodeName));
   newElem.appendChild(newNameElem);
   elem.appendChild(newElem);

   if (subMenuName.isEmpty())
      return newElem;
   else
      return findMenu(newElem, subMenuName, create);
}

static QString entryToDirId(const QString &path)
{
   // See also KDesktopFile::locateLocal
   QString local;
   if (QFileInfo(path).isAbsolute())
   {
      // XDG Desktop menu items come with absolute paths, we need to
      // extract their relative path and then build a local path.
      local = KGlobal::dirs()->relativeLocation("xdgdata-dirs", path);
   }

   if (local.isEmpty() || local.startsWith('/'))
   {
      // What now? Use filename only and hope for the best.
      local = path.mid(path.lastIndexOf('/')+1);
   }
   return local;
}

static void purgeIncludesExcludes(QDomElement elem, const QString &appId, QDomElement &excludeNode, QDomElement &includeNode)
{
   // Remove any previous includes/excludes of appId
   QDomNode n = elem.firstChild();
   while( !n.isNull() )
   {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      bool bIncludeNode = (e.tagName() == MF_INCLUDE);
      bool bExcludeNode = (e.tagName() == MF_EXCLUDE);
      if (bIncludeNode)
         includeNode = e;
      if (bExcludeNode)
         excludeNode = e;
      if (bIncludeNode || bExcludeNode)
      {
         QDomNode n2 = e.firstChild();
         while ( !n2.isNull() )
         {
            QDomNode next = n2.nextSibling();
            QDomElement e2 = n2.toElement();
            if (!e2.isNull() && e2.tagName() == MF_FILENAME)
            {
               if (e2.text() == appId)
               {
                  e.removeChild(e2);
                  break;
               }
            }
            n2 = next;
         }
      }
      n = n.nextSibling();
   }
}

static void purgeDeleted(QDomElement elem)
{
   // Remove any previous includes/excludes of appId
   QDomNode n = elem.firstChild();
   while( !n.isNull() )
   {
      QDomNode next = n.nextSibling();
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if ((e.tagName() == MF_DELETED) ||
          (e.tagName() == MF_NOTDELETED))
      {
         elem.removeChild(e);
      }
      n = next;
   }
}

static void purgeLayout(QDomElement elem)
{
   // Remove any previous includes/excludes of appId
   QDomNode n = elem.firstChild();
   while( !n.isNull() )
   {
      QDomNode next = n.nextSibling();
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if (e.tagName() == MF_LAYOUT)
      {
         elem.removeChild(e);
      }
      n = next;
   }
}

void MenuFile::addEntry(const QString &menuName, const QString &menuId)
{
   m_bDirty = true;

   m_removedEntries.removeAll(menuId);

   QDomElement elem = findMenu(m_doc.documentElement(), menuName, true);

   QDomElement excludeNode;
   QDomElement includeNode;

   purgeIncludesExcludes(elem, menuId, excludeNode, includeNode);

   if (includeNode.isNull())
   {
      includeNode = m_doc.createElement(MF_INCLUDE);
      elem.appendChild(includeNode);
   }

   QDomElement fileNode = m_doc.createElement(MF_FILENAME);
   fileNode.appendChild(m_doc.createTextNode(menuId));
   includeNode.appendChild(fileNode);
}

void MenuFile::setLayout(const QString &menuName, const QStringList &layout)
{
   m_bDirty = true;

   QDomElement elem = findMenu(m_doc.documentElement(), menuName, true);

   purgeLayout(elem);

   QDomElement layoutNode = m_doc.createElement(MF_LAYOUT);
   elem.appendChild(layoutNode);

   for(QStringList::ConstIterator it = layout.constBegin();
       it != layout.constEnd(); ++it)
   {
      QString li = *it;
      if (li == ":S")
      {
         layoutNode.appendChild(m_doc.createElement(MF_SEPARATOR));
      }
      else if (li == ":M")
      {
         QDomElement mergeNode = m_doc.createElement(MF_MERGE);
         mergeNode.setAttribute("type", "menus");
         layoutNode.appendChild(mergeNode);
      }
      else if (li == ":F")
      {
         QDomElement mergeNode = m_doc.createElement(MF_MERGE);
         mergeNode.setAttribute("type", "files");
         layoutNode.appendChild(mergeNode);
      }
      else if (li == ":A")
      {
         QDomElement mergeNode = m_doc.createElement(MF_MERGE);
         mergeNode.setAttribute("type", "all");
         layoutNode.appendChild(mergeNode);
      }
      else if (li.endsWith('/'))
      {
         li.truncate(li.length()-1);
         QDomElement menuNode = m_doc.createElement(MF_MENUNAME);
         menuNode.appendChild(m_doc.createTextNode(li));
         layoutNode.appendChild(menuNode);
      }
      else
      {
         QDomElement fileNode = m_doc.createElement(MF_FILENAME);
         fileNode.appendChild(m_doc.createTextNode(li));
         layoutNode.appendChild(fileNode);
      }
   }
}


void MenuFile::removeEntry(const QString &menuName, const QString &menuId)
{
   m_bDirty = true;
   m_removedEntries.append(menuId);

   QDomElement elem = findMenu(m_doc.documentElement(), menuName, true);

   QDomElement excludeNode;
   QDomElement includeNode;

   purgeIncludesExcludes(elem, menuId, excludeNode, includeNode);

   if (excludeNode.isNull())
   {
      excludeNode = m_doc.createElement(MF_EXCLUDE);
      elem.appendChild(excludeNode);
   }
   QDomElement fileNode = m_doc.createElement(MF_FILENAME);
   fileNode.appendChild(m_doc.createTextNode(menuId));
   excludeNode.appendChild(fileNode);
}

void MenuFile::addMenu(const QString &menuName, const QString &menuFile)
{
   m_bDirty = true;
   QDomElement elem = findMenu(m_doc.documentElement(), menuName, true);

   QDomElement dirElem = m_doc.createElement(MF_DIRECTORY);
   dirElem.appendChild(m_doc.createTextNode(entryToDirId(menuFile)));
   elem.appendChild(dirElem);
}

void MenuFile::moveMenu(const QString &oldMenu, const QString &newMenu)
{
   m_bDirty = true;

   // Undelete the new menu
   QDomElement elem = findMenu(m_doc.documentElement(), newMenu, true);
   purgeDeleted(elem);
   elem.appendChild(m_doc.createElement(MF_NOTDELETED));

// TODO: GET RID OF COMMON PART, IT BREAKS STUFF
   // Find common part
   QStringList oldMenuParts = oldMenu.split( '/');
   QStringList newMenuParts = newMenu.split( '/');
   QString commonMenuName;
   int max = qMin(oldMenuParts.count(), newMenuParts.count());
   int i = 0;
   for(; i < max; i++)
   {
      if (oldMenuParts[i] != newMenuParts[i])
         break;
      commonMenuName += '/' + oldMenuParts[i];
   }
   QString oldMenuName;
   for(int j = i; j < oldMenuParts.count()-1; j++)
   {
      if (i != j)
         oldMenuName += '/';
      oldMenuName += oldMenuParts[j];
   }
   QString newMenuName;
   for(int j = i; j < newMenuParts.count()-1; j++)
   {
      if (i != j)
         newMenuName += '/';
      newMenuName += newMenuParts[j];
   }

   if (oldMenuName == newMenuName) return; // Can happen

   elem = findMenu(m_doc.documentElement(), commonMenuName, true);

   // Add instructions for moving
   QDomElement moveNode = m_doc.createElement(MF_MOVE);
   QDomElement node = m_doc.createElement(MF_OLD);
   node.appendChild(m_doc.createTextNode(oldMenuName));
   moveNode.appendChild(node);
   node = m_doc.createElement(MF_NEW);
   node.appendChild(m_doc.createTextNode(newMenuName));
   moveNode.appendChild(node);
   elem.appendChild(moveNode);
}

void MenuFile::removeMenu(const QString &menuName)
{
   m_bDirty = true;

   QDomElement elem = findMenu(m_doc.documentElement(), menuName, true);

   purgeDeleted(elem);
   elem.appendChild(m_doc.createElement(MF_DELETED));
}

   /**
    * Returns a unique menu-name for a new menu under @p menuName
    * inspired by @p newMenu
    */
QString MenuFile::uniqueMenuName(const QString &menuName, const QString &newMenu, const QStringList & excludeList)
{
   QDomElement elem = findMenu(m_doc.documentElement(), menuName, false);

   QString result = newMenu;
   if (result.endsWith('/'))
       result.truncate(result.length()-1);

   QRegExp r("(.*)(?=-\\d+)");
   result = (r.indexIn(result) > -1) ? r.cap(1) : result;

   int trunc = result.length(); // Position of trailing '/'

   result.append("/");

   for(int n = 1; ++n; )
   {
      if (findMenu(elem, result, false).isNull() && !excludeList.contains(result))
         return result;

      result.truncate(trunc);
      result.append(QString("-%1/").arg(n));
   }
   return QString(); // Never reached
}

void MenuFile::performAction(const ActionAtom *atom)
{
   switch(atom->action)
   {
     case ADD_ENTRY:
        addEntry(atom->arg1, atom->arg2);
        return;
     case REMOVE_ENTRY:
        removeEntry(atom->arg1, atom->arg2);
        return;
     case ADD_MENU:
        addMenu(atom->arg1, atom->arg2);
        return;
     case REMOVE_MENU:
        removeMenu(atom->arg1);
        return;
     case MOVE_MENU:
        moveMenu(atom->arg1, atom->arg2);
        return;
   }
}

MenuFile::ActionAtom *MenuFile::pushAction(MenuFile::ActionType action, const QString &arg1, const QString &arg2)
{
   ActionAtom *atom = new ActionAtom;
   atom->action = action;
   atom->arg1 = arg1;
   atom->arg2 = arg2;
   m_actionList.append(atom);
   return atom;
}

void MenuFile::popAction(ActionAtom *atom)
{
   if (m_actionList.last() != atom)
   {
      qWarning("MenuFile::popAction Error, action not last in list.");
      return;
   }
   m_actionList.removeLast();
   delete atom;
}

bool MenuFile::performAllActions()
{
    Q_FOREACH(ActionAtom *atom, m_actionList) {
        performAction( atom );
        delete atom;
    }
    m_actionList.clear();

    // Entries that have been removed from the menu are added to .hidden
   // so that they don't re-appear in Lost & Found
   QStringList removed = m_removedEntries;
   m_removedEntries.clear();
   for(QStringList::ConstIterator it = removed.constBegin();
       it != removed.constEnd(); ++it)
   {
      addEntry("/.hidden/", *it);
   }

   m_removedEntries.clear();

   if (!m_bDirty)
      return true;

   return save();
}

bool MenuFile::dirty() const
{
   return (m_actionList.count() != 0) || m_bDirty;
}

void MenuFile::restoreMenuSystem( const QString &filename)
{
    m_error.clear();

    m_fileName = filename;
    m_doc.clear();
    m_bDirty = false;
    Q_FOREACH(ActionAtom *atom, m_actionList) {
        delete atom;
    }
    m_actionList.clear();

    m_removedEntries.clear();
    create();
}
