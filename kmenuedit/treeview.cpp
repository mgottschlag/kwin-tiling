/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <iostream.h>

#include <qdir.h>
#include <qheader.h>
#include <qstringlist.h>
#include <qfileinfo.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kdesktopfile.h>
#include <kdebug.h>

#include "treeview.h"
#include "treeview.moc"

TreeItem::TreeItem(QListViewItem *parent, const QString& file)
  :QListViewItem(parent)
{
  _file = file;
}

TreeItem::TreeItem(QListView *parent, const QString& file)
  : QListViewItem(parent)
{
  _file = file;
}

TreeView::TreeView( QWidget *parent, const char *name )
  : KListView(parent, name)
{
  setFrameStyle(QFrame::WinPanel | QFrame::Sunken);  
  setAllColumnsShowFocus(true);
  setRootIsDecorated(true);
  setSorting(-1);

  addColumn("");
  header()->hide();

  connect(this, SIGNAL(clicked( QListViewItem* )), SLOT(itemSelected( QListViewItem* )));

  fill();
}

void TreeView::fill()
{
  clear();
  fillBranch("", 0);
}

void TreeView::fillBranch(const QString& rPath, TreeItem *parent)
{
  // get rid of leading slash in the relative path
  QString relPath = rPath;
   if(relPath[0] == '/')
    relPath = relPath.mid(1, relPath.length());

   // I don't use findAllResources as subdirectories are not recognised as resources
   // and therefore I already have to iterate by hand to get the subdir list.
   QStringList dirlist = dirList(relPath);
   QStringList filelist = fileList(relPath);

  //for (QStringList::ConstIterator it = dirlist.begin(); it != dirlist.end(); ++it)
  //  cout << (*it).local8Bit() << endl;

  // first add tree items for the desktop files in this directory
  if (!filelist.isEmpty()) {
    QStringList::ConstIterator it = filelist.end();
    do{
      --it;
      
      KDesktopFile df(*it);
      if(df.readBoolEntry("Hidden"))
	continue;

      TreeItem* item;
      if (parent == 0) item = new TreeItem(this, *it);
      else item = new TreeItem(parent, *it);
    
      item->setText(0, df.readName());
      item->setPixmap(0, KGlobal::iconLoader()->
		      loadIcon(df.readIcon(),KIcon::Desktop, KIcon::SizeSmall));
    }
    while (it != filelist.begin());
  }

  // add directories and process sudirs
  if (!dirlist.isEmpty()) {
    QStringList::ConstIterator it = dirlist.end();
    do{
      --it;

      QString dirFile = KGlobal::dirs()->findResource("apps", *it + "/.directory");
      TreeItem* item;
    
      if (dirFile.isNull())
	{
	  if (parent == 0)
	    item = new TreeItem(this, *it + "/.directory");
	  else
	    item = new TreeItem(parent, *it + "/.directory");
	  item->setText(0, *it);
	  item->setPixmap(0, KGlobal::iconLoader()->
			  loadIcon("package",KIcon::Desktop, KIcon::SizeSmall));
	}
      else
	{
	  KDesktopFile df(dirFile);
	  if(df.readBoolEntry("Hidden"))
	    continue;

	  if (parent == 0)
	    item = new TreeItem(this,  *it + "/.directory");
	  else
	    item = new TreeItem(parent, *it + "/.directory");
	
	  item->setText(0, df.readName());
	  item->setPixmap(0, KGlobal::iconLoader()
			  ->loadIcon(df.readIcon(),KIcon::Desktop, KIcon::SizeSmall));
	}
      fillBranch(*it, item);
    }
    while (it != dirlist.begin());
  }
}

void TreeView::itemSelected(QListViewItem *item)
{
  if(!item) return;
  emit entrySelected(((TreeItem*)item)->file());
}

void TreeView::slotCurrentChanged()
{
  TreeItem *item = (TreeItem*)selectedItem();
  if (item == 0) return;

  KDesktopFile df(item->file());
  item->setText(0, df.readName());
  item->setPixmap(0, KGlobal::iconLoader()
		  ->loadIcon(df.readIcon(),KIcon::Desktop, KIcon::SizeSmall));
}

void TreeView::slotDeleteCurrent()
{
  TreeItem *item = (TreeItem*)selectedItem();
  
  // nothing selected? -> nothing to delete
  if (item == 0) return;

  QString file = item->file();
  
  // is file a .directory or a .desktop file
  if(file.find(".directory") > 0)
    deleteDir(file.mid(0, file.find("/.directory")));
  else if (file.find(".desktop"))
    deleteFile(file);
}

void TreeView::deleteFile(const QString& deskfile)
{
  bool allremoved = true;
 
  // search the selected item in all resource dirs
  QStringList resdirs = KGlobal::dirs()->resourceDirs("apps");
  for (QStringList::ConstIterator it = resdirs.begin(); it != resdirs.end(); ++it)
    {
      QFile f((*it) + "/" + deskfile);
      
      // continue if it does not exist in this resource dir
      if(!f.exists()) continue;
      
      // remove all writeable files
      if(!f.remove())
	allremoved = false;
    }
  
  // if we did not have the permissions to remove all files we set a hidden flag
  // in the local one.
  if(!allremoved)
    {
      KSimpleConfig c(locateLocal("apps", deskfile));
      c.writeEntry("Hidden", true);
      c.sync();
    }
}

void TreeView::deleteDir(const QString& directory)
{
  bool allremoved = true;

  cout << "deletedir: " << directory.local8Bit() << endl;

  QStringList dirlist = dirList(directory);
  QStringList filelist = fileList(directory);

  // delete files
  for (QStringList::ConstIterator it = filelist.begin(); it != filelist.end(); ++it)
    deleteFile(*it);

  // process subdirs
  for (QStringList::ConstIterator it = dirlist.begin(); it != dirlist.end(); ++it)
    deleteDir(*it);

  // delete .directory file in all prefixes
  deleteFile(directory + "/.directory");
  
  // try to rmdir the directory in all prefixes
  QDir dir;
  QStringList dirs = KGlobal::dirs()->findDirs("apps", directory);
  for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it)
    {
      // remove all writeable files
      if(!dir.rmdir(*it))
	allremoved = false;
    }
      
  // set a local hidden flag if not all dirs could be deleted
  if(!allremoved)
    {
      KSimpleConfig c(locateLocal("apps", directory + "/.directory"));
      c.writeEntry("Hidden", true);
      c.sync();
    }
}

QStringList TreeView::fileList(const QString& relativePath)
{
  QStringList filelist;

  // loop through all resource dirs and build a file list
  QStringList resdirlist = KGlobal::dirs()->resourceDirs("apps");
  for (QStringList::ConstIterator it = resdirlist.begin(); it != resdirlist.end(); ++it)
    {
      QDir dir((*it) + "/" + relativePath);
      if(!dir.exists()) continue;
     
      dir.setFilter(QDir::Files);
      dir.setNameFilter("*.desktop");

      // build a list of files
      QStringList files = dir.entryList();
      for (QStringList::ConstIterator it = files.begin(); it != files.end(); ++it)
	{
	  // does not work?!
	  //if (filelist.contains(*it)) continue;

	  if (relativePath == "")
	    {
	      filelist.remove(*it); // hack
	      filelist.append(*it);
	    }
	  else
	    {
	      filelist.remove(relativePath + "/" + *it); //hack
	      filelist.append(relativePath + "/" + *it);
	    }
	}
    }
  return filelist;
}

QStringList TreeView::dirList(const QString& relativePath)
{
  QStringList dirlist;

  // loop through all resource dirs and build a file and subdir list
  QStringList resdirlist = KGlobal::dirs()->resourceDirs("apps");
  for (QStringList::ConstIterator it = resdirlist.begin(); it != resdirlist.end(); ++it)
    {
      QDir dir((*it) + "/" + relativePath);
      if(!dir.exists()) continue;
      dir.setFilter(QDir::Dirs);

      // build a list of subdirs
      QStringList subdirs = dir.entryList();
      for (QStringList::ConstIterator it = subdirs.begin(); it != subdirs.end(); ++it)
	{
	  if ((*it) == "." || (*it) == "..") continue;
	  // does not work?!
	  // if (dirlist.contains(*it)) continue;
	  
	  if (relativePath == "")
	    {
	      dirlist.remove(*it); //hack
	      dirlist.append(*it);
	    }
	  else
	    {
	      dirlist.remove(relativePath + "/" + *it); //hack
	      dirlist.append(relativePath + "/" + *it);
	    }
	}
    }
  return dirlist;
}
