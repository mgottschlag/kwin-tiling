/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2001-2002 Raffaele Sandrini <sandrini@kde.org)
 *   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
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
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <unistd.h>

#include <qdir.h>
#include <qheader.h>
#include <qstringlist.h>
#include <qdragobject.h>
#include <qdatastream.h>
#include <qcstring.h>
#include <qpopupmenu.h>
#include <qfileinfo.h>
#include <qcursor.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kdesktopfile.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kservice.h>
#include <kservicegroup.h>

#include "treeview.h"
#include "treeview.moc"
#include "khotkeys.h"
#include "menufile.h"

#define MOVE_FOLDER "M"
#define COPY_FOLDER "C"
#define MOVE_FILE   "m"
#define COPY_FILE   "c"

// Add sub menu
void FolderInfo::add(FolderInfo *info)
{
   subFolders.append(info);
}

// Remove sub menu (without deleting it)
void FolderInfo::take(FolderInfo *info)
{
   subFolders.take(subFolders.findRef(info));
}

// Remove sub menu (without deleting it)
bool FolderInfo::takeRecursive(FolderInfo *info)
{
   int i = subFolders.findRef(info);
   if (i >= 0)
   {
      subFolders.take(i);
      return true;
   }
   
   for(FolderInfo *subFolderInfo = subFolders.first();
       subFolderInfo; subFolderInfo = subFolders.next())
   {
      if (subFolderInfo->takeRecursive(info))
         return true;
   }
   return false;
}
    
// Add entry
void FolderInfo::add(KService *s)
{
   entries.append(s);
}
    
// Remove entry
void FolderInfo::take(const QString &file)
{
    for(KService::List::Iterator it = entries.begin();
        it != entries.end(); ++it) 
    {
       if ((*it)->desktopEntryPath() == file)
       {
          entries.remove(it);
          break;
       }
    }
}


// Return a unique sub-menu caption inspired by @p caption
QString FolderInfo::uniqueMenuCaption(const QString &caption)
{
   QString result = caption;
   for(int n = 1; ++n; )
   {
      bool ok = true;
      for(FolderInfo *subFolderInfo = subFolders.first();
          subFolderInfo; subFolderInfo = subFolders.next())
      {
         if (subFolderInfo->caption == result)
         {
            ok = false;
            break;
         }
      }
      if (ok)
         return result;
      
      result = caption + QString("-%1").arg(n);
   }
   return QString::null; // Never reached
}

// Return a unique item caption inspired by @p caption
QString FolderInfo::uniqueItemCaption(const QString &caption, const QString &exclude)
{
   QString result = caption;
   for(int n = 1; ++n; )
   {
      bool ok = true;
      if (result == exclude)
         ok = false;
      for(KService::List::Iterator it = entries.begin();
          ok && (it != entries.end()); ++it) 
      {
         if ((*it)->name() == result)
            ok = false;
      }
      if (ok)
         return result;
      
      result = caption + QString("-%1").arg(n);
   }
   return QString::null; // Never reached
}

// Return a list of existing submenu ids
QStringList FolderInfo::existingMenuIds()
{
   QStringList result;
   for(FolderInfo *subFolderInfo = subFolders.first();
       subFolderInfo; subFolderInfo = subFolders.next())
   {
       result.append(subFolderInfo->id);
qWarning("Existing Id: %s", subFolderInfo->id.latin1());
   }
   return result;
}


TreeItem::TreeItem(QListViewItem *parent, const QString& file)
    :QListViewItem(parent), _hidden(false), _init(false), _file(file), m_folderInfo(0) {}

TreeItem::TreeItem(QListViewItem *parent, QListViewItem *after, const QString& file)
    :QListViewItem(parent, after), _hidden(false), _init(false), _file(file), m_folderInfo(0) {}

TreeItem::TreeItem(QListView *parent, const QString& file)
    : QListViewItem(parent), _hidden(false), _init(false), _file(file), m_folderInfo(0) {}

TreeItem::TreeItem(QListView *parent, QListViewItem *after, const QString& file)
    : QListViewItem(parent, after), _hidden(false), _init(false), _file(file), m_folderInfo(0) {}

void TreeItem::setName(const QString &name)
{
    _name = name;
    update();
}

void TreeItem::setHidden(bool b)
{
    if (_hidden == b) return;
    _hidden = b;
    update();
}

void TreeItem::update()
{
    QString s = _name;
    if (_hidden)
       s += i18n(" [Hidden]");
    setText(0, s);
}

void TreeItem::setOpen(bool o)
{
    if (o)
       load();

    QListViewItem::setOpen(o);
}

void TreeItem::load()
{
    if (m_folderInfo && !_init)
    {
       _init = true;
       TreeView *tv = static_cast<TreeView *>(listView());
       tv->fillBranch(m_folderInfo, this);
    }
}

static QPixmap appIcon(const QString &iconName)
{
    QPixmap normal = KGlobal::iconLoader()->loadIcon(iconName, KIcon::Small, 0, KIcon::DefaultState, 0L, true);
    // make sure they are not larger than 20x20
    if (normal.width() > 20 || normal.height() > 20)
    {
       QImage tmp = normal.convertToImage();
       tmp = tmp.smoothScale(20, 20);
       normal.convertFromImage(tmp);
    }
    return normal;
}


TreeView::TreeView( KActionCollection *ac, QWidget *parent, const char *name )
    : KListView(parent, name), m_ac(ac), m_rmb(0), m_clipboardInfo(0)
{
    setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(true);
    setSorting(-1);
    setAcceptDrops(true);
    setDropVisualizer(true);
    setDragEnabled(true);
    setMinimumWidth(240);

    addColumn("");
    header()->hide();

    connect(this, SIGNAL(dropped(QDropEvent*, QListViewItem*, QListViewItem*)),
	    SLOT(slotDropped(QDropEvent*, QListViewItem*, QListViewItem*)));

    connect(this, SIGNAL(clicked( QListViewItem* )),
	    SLOT(itemSelected( QListViewItem* )));

    connect(this,SIGNAL(selectionChanged ( QListViewItem * )),
            SLOT(itemSelected( QListViewItem* )));

    connect(this, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
	    SLOT(slotRMBPressed(QListViewItem*, const QPoint&)));

    // connect actions
    connect(m_ac->action("newitem"), SIGNAL(activated()), SLOT(newitem()));
    connect(m_ac->action("newsubmenu"), SIGNAL(activated()), SLOT(newsubmenu()));

    cleanupClipboard();

    m_menuFile = new MenuFile( locateLocal("xdgconf-menu", "applications-kmenuedit.menu"));
}

TreeView::~TreeView() {
    cleanupClipboard();
}

void TreeView::setViewMode(bool showHidden)
{
    delete m_rmb;

    // setup rmb menu
    m_rmb = new QPopupMenu(this);
    KAction *action;

    action = m_ac->action("edit_cut");
    if(action) {
        action->plug(m_rmb);
        action->setEnabled(false);
        connect(action, SIGNAL(activated()), SLOT(cut()));
    }

    action = m_ac->action("edit_copy");
    if(action) {
        action->plug(m_rmb);
        action->setEnabled(false);
        connect(action, SIGNAL(activated()), SLOT(copy()));
    }

    action = m_ac->action("edit_paste");
    if(action) {
        action->plug(m_rmb);
        action->setEnabled(false);
        connect(action, SIGNAL(activated()), SLOT(paste()));
    }

    m_rmb->insertSeparator();

    action = m_ac->action("delete");
    if(action) {
        action->plug(m_rmb);
        action->setEnabled(false);
        connect(action, SIGNAL(activated()), SLOT(del()));
    }

    action = m_ac->action("undelete");
    if(action) {
        action->plug(m_rmb);
        action->setEnabled(false);
        connect(action, SIGNAL(activated()), SLOT(undel()));
    }

    m_rmb->insertSeparator();

    if(m_ac->action("newitem"))
	m_ac->action("newitem")->plug(m_rmb);
    if(m_ac->action("newsubmenu"))
	m_ac->action("newsubmenu")->plug(m_rmb);

    m_showHidden = showHidden;
    readFolderInfo();
    fill();
}

void TreeView::readFolderInfo(FolderInfo *folderInfo, KServiceGroup::Ptr folder, const QString &prefix)
{
    if (!folderInfo)
    {
       folderInfo = &m_rootFolder;
       folder = KServiceGroup::root();
    }

    if (!folder || !folder->isValid())
        return;

    folderInfo->caption = folder->caption();

    // Item names may contain ampersands. To avoid them being converted
    // to accelerators, replace them with two ampersands.
    folderInfo->caption.replace("&", "&&");
    folderInfo->hidden = folder->noDisplay();
    folderInfo->directoryFile = folder->directoryEntryPath();
    folderInfo->icon = folder->icon();
    QString id = folder->relPath();
    int i = id.findRev('/', -2);
    id = id.mid(i+1);
    folderInfo->id = id;
    folderInfo->fullId = prefix + id;

    KServiceGroup::List list = folder->entries(true, !m_showHidden);

    for(KServiceGroup::List::ConstIterator it = list.begin();
        it != list.end(); ++it) 
    {
        KSycocaEntry * e = *it;

        if (e->isType(KST_KServiceGroup)) 
        {
            KServiceGroup::Ptr g(static_cast<KServiceGroup *>(e));
            FolderInfo *subFolderInfo = new FolderInfo();
            readFolderInfo(subFolderInfo, g, folderInfo->fullId);
            folderInfo->add(subFolderInfo);
qWarning("Adding %s (%s)", subFolderInfo->caption.latin1(), subFolderInfo->fullId.latin1());
        }
        else if (e->isType(KST_KService)) 
        {
            folderInfo->add(static_cast<KService *>(e));
        }
    }
}

void TreeView::fill()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    clear();
    fillBranch(&m_rootFolder, 0);
    QApplication::restoreOverrideCursor();
}

QString TreeView::findName(KDesktopFile *df, bool deleted)
{
    QString name = df->readName();
    if (deleted)
    {
       if (name == "empty")
          name = QString::null;
       if (name.isEmpty())
       {
          QString file = df->fileName();
          QString res = df->resource();

          bool isLocal = true;
          QStringList files = KGlobal::dirs()->findAllResources(res.latin1(), file);
          for(QStringList::ConstIterator it = files.begin();
              it != files.end();
              ++it)
          {
             if (isLocal)
             {
                isLocal = false;
                continue;
             }

             KDesktopFile df2(*it);
             name = df2.readName();

             if (!name.isEmpty() && (name != "empty"))
                return name;
          }
       }
    }
    return name;
}

TreeItem *TreeView::createTreeItem(TreeItem *parent, QListViewItem *after, FolderInfo *folderInfo)
{
   TreeItem *item;		
   if (parent == 0)
      item = new TreeItem(this,  after, folderInfo->directoryFile);
   else
      item = new TreeItem(parent, after, folderInfo->directoryFile);

   item->setFolderInfo(folderInfo);
   item->setName(folderInfo->caption);
   item->setPixmap(0, appIcon(folderInfo->icon));
   item->setDirectoryPath(folderInfo->fullId);
   item->setHidden(folderInfo->hidden);
   item->setExpandable(true);
   return item;
}

TreeItem *TreeView::createTreeItem(TreeItem *parent, QListViewItem *after, KService *s)
{
   QString serviceCaption = s->name();

   // Item names may contain ampersands. To avoid them being converted
   // to accelerators, replace them with two ampersands.
   serviceCaption.replace("&", "&&");

   bool hidden = s->noDisplay();

   TreeItem* item;
   if (parent == 0)
      item = new TreeItem(this, after, s->desktopEntryPath());
   else
      item = new TreeItem(parent, after, s->desktopEntryPath());

   item->setName(serviceCaption);
   item->setPixmap(0, appIcon(s->icon()));

   item->setHidden(hidden);
   return item;
}

void TreeView::fillBranch(FolderInfo *folderInfo, TreeItem *parent)
{
    QString relPath = parent ? parent->directory() : QString::null;
    QPtrListIterator<FolderInfo> it( folderInfo->subFolders );
    TreeItem *after = 0;
    for (FolderInfo *subFolder; (subFolder = it.current()); ++it)
    {
       after = createTreeItem(parent, after, subFolder);
    }
    
    for(KService::List::ConstIterator it = folderInfo->entries.begin();
        it != folderInfo->entries.end(); ++it) 
    {
       after = createTreeItem(parent, after, *it);
    }
}

void TreeView::itemSelected(QListViewItem *item)
{
    TreeItem *_item = (TreeItem*)item;
    bool selected = false;
    bool dselected = false;
    if (_item) {
        selected = true;
        dselected = _item->isHidden();
    }

    m_ac->action("edit_cut")->setEnabled(selected);
    m_ac->action("edit_copy")->setEnabled(selected);

    if (m_ac->action("delete"))
        m_ac->action("delete")->setEnabled(selected && !dselected);
    if (m_ac->action("undelete"))
        m_ac->action("undelete")->setEnabled(selected && dselected);

    if(!item) return;
qWarning("entrySelected: %s", _item->file().latin1());
    emit entrySelected(_item->file(), _item->name(), dselected);
}

void TreeView::currentChanged(const QString& file)
{
    TreeItem *item = (TreeItem*)selectedItem();
    if (item == 0) return;

    item->setFile(file);

    KDesktopFile df(file);
    item->setName(findName(&df, item->isHidden()));
    item->setPixmap(0, appIcon(df.readIcon()));
}

QStringList TreeView::fileList(const QString& rPath)
{
    QString relativePath = rPath;

    // truncate "/.directory"
    int pos = relativePath.findRev("/.directory");
    if (pos > 0) relativePath.truncate(pos);

    QStringList filelist;

    // loop through all resource dirs and build a file list
    QStringList resdirlist = KGlobal::dirs()->resourceDirs("apps");
    for (QStringList::ConstIterator it = resdirlist.begin(); it != resdirlist.end(); ++it)
    {
        QDir dir((*it) + "/" + relativePath);
        if(!dir.exists()) continue;

        dir.setFilter(QDir::Files);
        dir.setNameFilter("*.desktop;*.kdelnk");

        // build a list of files
        QStringList files = dir.entryList();
        for (QStringList::ConstIterator it = files.begin(); it != files.end(); ++it) {
            // does not work?!
            //if (filelist.contains(*it)) continue;

            if (relativePath.isEmpty()) {
                filelist.remove(*it); // hack
                filelist.append(*it);
            }
            else {
                filelist.remove(relativePath + "/" + *it); //hack
                filelist.append(relativePath + "/" + *it);
            }
        }
    }
    return filelist;
}

QStringList TreeView::dirList(const QString& rPath)
{
    QString relativePath = rPath;

    // truncate "/.directory"
    int pos = relativePath.findRev("/.directory");
    if (pos > 0) relativePath.truncate(pos);

    QStringList dirlist;

    // loop through all resource dirs and build a subdir list
    QStringList resdirlist = KGlobal::dirs()->resourceDirs("apps");
    for (QStringList::ConstIterator it = resdirlist.begin(); it != resdirlist.end(); ++it)
    {
        QDir dir((*it) + "/" + relativePath);
        if(!dir.exists()) continue;
        dir.setFilter(QDir::Dirs);

        // build a list of subdirs
        QStringList subdirs = dir.entryList();
        for (QStringList::ConstIterator it = subdirs.begin(); it != subdirs.end(); ++it) {
            if ((*it) == "." || (*it) == "..") continue;
            // does not work?!
            // if (dirlist.contains(*it)) continue;

            if (relativePath.isEmpty()) {
                dirlist.remove(*it); //hack
                dirlist.append(*it);
            }
            else {
                dirlist.remove(relativePath + "/" + *it); //hack
                dirlist.append(relativePath + "/" + *it);
            }
        }
    }
    return dirlist;
}

bool TreeView::acceptDrag(QDropEvent* e) const
{
    return QString(e->format()).contains("application/x-kmenuedit-internal") &&
           (e->source() == this);
}

static QString createDesktopFile(const QString &file, bool duplicate)
{
   QString base = file.mid(file.findRev('/')+1);
   base = base.left(base.findRev('.'));
   
   QString result;
   int i = duplicate ? 2 : 1;
   while(true)
   {
      if (i == 1)
         result = base + ".desktop";
      else
         result = base + QString("-%1.desktop").arg(i);

      if (locate("xdgdata-apps", result).isEmpty())
         break;
      i++;
   }
   result = locateLocal("xdgdata-apps", result);
   if (duplicate)
   {
      KDesktopFile df(file);
      KConfig *cfg = df.copyTo(result);
      cfg->sync();
      delete cfg;
   }
   return result;
}

static QString createDirectoryFile(const QString &file, bool duplicate)
{
   QString base = file.mid(file.findRev('/')+1);
   base = base.left(base.findRev('.'));
   
   QString result;
   int i = duplicate ? 2 : 1;
   while(true)
   {
      if (i == 1)
         result = base + ".directory";
      else
         result = base + QString("-%1.directory").arg(i);

      if (locate("xdgdata-dirs", result).isEmpty())
         break;
      i++;
   }
   result = locateLocal("xdgdata-dirs", result);
   if (duplicate)
   {
      KDesktopFile df(file);
      KConfig *cfg = df.copyTo(result);
      cfg->sync();
      delete cfg;
   }
   return result;
}


void TreeView::slotDropped (QDropEvent * e, QListViewItem *parent, QListViewItem*after)
{
   if(!e) return;

   if (e->source() != this) return; // Only internal drags are supported atm
    
   TreeItem *parentItem = static_cast<TreeItem*>(parent);

   // is there content in the clipboard?
   if (m_drag.isEmpty()) return;

   // get destination folder
   QString folder = parentItem ? parentItem->directory() : QString::null;
   FolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : &m_rootFolder;
   QString command = m_drag.left(1);
   if (command == MOVE_FOLDER)
   {
      FolderInfo *folderInfo = m_dragInfo;
      if (e->action() == QDropEvent::Copy)
      {
         // Ugh.. this is hard :)
         // * Create new .directory file
         // Add 
      }
      else
      {
         del(m_dragItem, false);
         // Move menu
         QString oldFolder = folderInfo->fullId;
         QString folderName = folderInfo->id;
         QString newFolder = m_menuFile->uniqueMenuName(folder, folderName, parentFolderInfo->existingMenuIds());
         folderInfo->id = newFolder;

         // Add file to menu
         m_menuFile->moveMenu(oldFolder, folder + newFolder);

         // Make sure caption is unique
         QString newCaption = parentFolderInfo->uniqueMenuCaption(folderInfo->caption);
         if (newCaption != folderInfo->caption)
         {
            folderInfo->caption = newCaption;
            // TODO: Update caption in .directory file.
         }

         folderInfo->fullId = parentFolderInfo->fullId + folderInfo->id;
         parentFolderInfo->add(folderInfo);
          
         // create the TreeItem
         TreeItem *newItem = createTreeItem(parentItem, after, folderInfo);

         setSelected ( newItem, true);
         itemSelected( newItem);
      }
   }
   else if (command == MOVE_FILE)
   {
      QString file = m_dragItem->file();
      if (file.isEmpty()) return; // Error

      KDesktopFile *df = 0;
      if (e->action() == QDropEvent::Copy)
      {
         // Need to copy file and then add it
         file = createDesktopFile(file, true); // Duplicate

         df = new KDesktopFile(file);
         QString oldCaption = df->readName();
         QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption, oldCaption);
         if (oldCaption != newCaption)
            df->writeEntry("Name", newCaption);
      }
      else
      {
         del(m_dragItem, false);
         df = new KDesktopFile(file);
         QString oldCaption = df->readName();
         QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption);
         if (oldCaption != newCaption)
            df->writeEntry("Name", newCaption);
      }
      // Add file to menu
      m_menuFile->addEntry(folder, file);

      KService *s = new KService(df);

      // update fileInfo data
      parentFolderInfo->add(s);

      // create the TreeItem
      if(parentItem)
         parentItem->setOpen(true);

      TreeItem *newItem = createTreeItem(parentItem, after, s);
      delete df;

      setSelected ( newItem, true);
      itemSelected( newItem);
   }
   else
   {
      // Error
   }
   m_drag = QString::null;
}


void TreeView::startDrag()
{
  QDragObject *drag = dragObject();
  
  if (!drag)
     return;
            
  drag->dragMove();
}                  

QDragObject *TreeView::dragObject()
{
    TreeItem *item = (TreeItem*)selectedItem();
    if(item == 0) return 0;

    if (item->isDirectory())
    {
       m_drag = MOVE_FOLDER;
       m_dragInfo = item->folderInfo();
       m_dragItem = item;
    }
    else
    {
       m_drag = MOVE_FILE;
       m_dragInfo = 0;
       m_dragItem = item;
    }

    QStoredDrag *d = new QStoredDrag("application/x-kmenuedit-internal", this);
    if ( item->pixmap(0) )
        d->setPixmap(*item->pixmap(0));
    return d;
}

void TreeView::slotRMBPressed(QListViewItem*, const QPoint& p)
{
    TreeItem *item = (TreeItem*)selectedItem();
    if(item == 0) return;

    if(m_rmb) m_rmb->exec(p);
}

void TreeView::newsubmenu()
{
   TreeItem *parentItem = 0;
   TreeItem *item = (TreeItem*)selectedItem();

   // nil selected? -> nil to paste to
   if (item == 0) return;

   bool ok;
   QString caption = KInputDialog::getText( i18n( "New Submenu" ),
        i18n( "Submenu name:" ), QString::null, &ok, this );

   if (!ok) return;

   QString file = caption;
   file.replace('/', '-');

   file = createDirectoryFile(file, false); // Create

qWarning("CreateDirectoryFile: %s", file.latin1());
    
   // get destination folder
   QString folder;

   if(item->isDirectory())
   {
      parentItem = item;
      item = 0;
      folder = parentItem->directory();
   }
   else
   {
      parentItem = static_cast<TreeItem*>(item->parent());
      folder = parentItem ? parentItem->directory() : QString::null;
   }

   FolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : &m_rootFolder;
   FolderInfo *folderInfo = new FolderInfo();
   folderInfo->caption = parentFolderInfo->uniqueMenuCaption(caption);
   folderInfo->id = m_menuFile->uniqueMenuName(folder, caption, parentFolderInfo->existingMenuIds());
   folderInfo->directoryFile = file;
   folderInfo->icon = "package";
   folderInfo->hidden = false;

   KDesktopFile *df = new KDesktopFile(file);
   df->writeEntry("Name", folderInfo->caption);
   df->writeEntry("Icon", folderInfo->icon);
   df->sync();
   
   // Add file to menu
   m_menuFile->addMenu(folder + folderInfo->id, file);

   folderInfo->fullId = parentFolderInfo->fullId + folderInfo->id;

   // update fileInfo data
   parentFolderInfo->add(folderInfo);
         
   // create the TreeItem
   if(parentItem)
      parentItem->setOpen(true);

   TreeItem *newItem = createTreeItem(parentItem, item, folderInfo);

   setSelected ( newItem, true);
   itemSelected( newItem);
}

void TreeView::newitem()
{
   TreeItem *parentItem = 0;
   TreeItem *item = (TreeItem*)selectedItem();

   // nil selected? -> nil to paste to
   if (item == 0) return;

   bool ok;
   QString caption = KInputDialog::getText( i18n( "New Item" ),
        i18n( "Item name:" ), QString::null, &ok, this );

   if (!ok) return;

   QString file = caption;
   file.replace('/', '-');

   file = createDesktopFile(file, false); // Create
    
   KDesktopFile *df = new KDesktopFile(file);
   df->writeEntry("Name", caption);
   df->writeEntry("Type", "Application");
   df->sync();

   // get destination folder
   QString folder;

   if(item->isDirectory())
   {
      parentItem = item;
      item = 0;
      folder = parentItem->directory();
   }
   else
   {
      parentItem = static_cast<TreeItem*>(item->parent());
      folder = parentItem ? parentItem->directory() : QString::null;
   }

   FolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : &m_rootFolder;

   // Add file to menu
   m_menuFile->addEntry(folder, file);

   KService *s = new KService(df);

   // update fileInfo data
   parentFolderInfo->add(s);

   // create the TreeItem
   if(parentItem)
      parentItem->setOpen(true);

   TreeItem *newItem = createTreeItem(parentItem, item, s);
   delete df;

   setSelected ( newItem, true);
   itemSelected( newItem);
}

void TreeView::cut()
{
    copy( true );

    m_ac->action("edit_cut")->setEnabled(false);
    m_ac->action("edit_copy")->setEnabled(false);
    m_ac->action("delete")->setEnabled(false);

    // Select new current item
    setSelected( currentItem(), true );
    // Switch the UI to show that item
    itemSelected( selectedItem() );
}

void TreeView::copy()
{
    copy( false );
}

void TreeView::copy( bool cutting )
{
    TreeItem *item = (TreeItem*)selectedItem();

    // nil selected? -> nil to copy
    if (item == 0) return;

    // clean up old stuff
    cleanupClipboard();

    // is item a folder or a file?
    if(item->isDirectory())
    {
        QString folder = item->directory();
        if (cutting)
        {
           // Place in clipboard
           m_clipboard = MOVE_FOLDER;
           m_clipboardInfo = item->folderInfo();

           del(item, false);
#if 0
           // Remove FolderInfo
           TreeItem *parentItem = static_cast<TreeItem*>(item->parent());
           FolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : &m_rootFolder;
           parentFolderInfo->take(m_clipboardInfo);

           // Remove from menu
           m_menuFile->removeMenu(folder);
           
           // Remove tree item
           delete item;
#endif
        }
        else
        {
           // Place in clipboard
           m_clipboard = COPY_FOLDER;
           m_clipboardInfo = item->folderInfo();
        }
    }
    else
    {
        QString file = item->file();

        if (cutting)
        {
           // Place in clipboard
           m_clipboard = MOVE_FILE + file;

           del(item, false);
#if 0
           // Remove FolderInfo
           TreeItem *parentItem = static_cast<TreeItem*>(item->parent());
           FolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : &m_rootFolder;
           parentFolderInfo->take(file);

           // Remove from menu
           QString folder = parentItem ? parentItem->directory() : QString::null;
           m_menuFile->removeEntry(folder, file);
           
           // Remove tree item
           delete item;
#endif
        }
        else
        {
           // Place in clipboard
           m_clipboard = COPY_FILE + file;
        }
    }

    m_ac->action("edit_paste")->setEnabled(true);
}


void TreeView::paste()
{
   TreeItem *parentItem = 0;
   TreeItem *item = (TreeItem*)selectedItem();

   // nil selected? -> nil to paste to
   if (item == 0) return;

   // is there content in the clipboard?
   if (m_clipboard.isEmpty()) return;

   // get destination folder
   QString folder;

   if(item->isDirectory())
   {
      parentItem = item;
      item = 0;
      folder = parentItem->directory();
   }
   else
   {
      parentItem = static_cast<TreeItem*>(item->parent());
      folder = parentItem ? parentItem->directory() : QString::null;
   }

   FolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : &m_rootFolder;
   QString command = m_clipboard.left(1);
   if ((command == COPY_FOLDER) || (command == MOVE_FOLDER))
   {
      FolderInfo *folderInfo = m_clipboardInfo;
      if (command == COPY_FOLDER)
      {
         // Ugh.. this is hard :)
         // * Create new .directory file
         // Add 
      }
      else if (command == MOVE_FOLDER)
      {
         // Move menu
         QString oldFolder = folderInfo->fullId;
         QString folderName = folderInfo->id;
         QString newFolder = m_menuFile->uniqueMenuName(folder, folderName, parentFolderInfo->existingMenuIds());
         folderInfo->id = newFolder;

         // Add file to menu
         m_menuFile->moveMenu(oldFolder, folder + newFolder);

         // Make sure caption is unique
         QString newCaption = parentFolderInfo->uniqueMenuCaption(folderInfo->caption);
         if (newCaption != folderInfo->caption)
         {
            folderInfo->caption = newCaption;
            // TODO: Update caption in .directory file.
         }

         folderInfo->fullId = parentFolderInfo->fullId + folderInfo->id;
         parentFolderInfo->add(folderInfo);
          
         // create the TreeItem
         TreeItem *newItem = createTreeItem(parentItem, item, folderInfo);

         setSelected ( newItem, true);
         itemSelected( newItem);
      }

      m_clipboard = COPY_FOLDER; // Next one copies.
   }
   else if ((command == COPY_FILE) || (command == MOVE_FILE))
   {
      QString file = m_clipboard.mid(1);
      if (file.isEmpty()) return; // Error

      KDesktopFile *df = 0;
      if (command == COPY_FILE)
      {
         // Need to copy file and then add it
         file = createDesktopFile(file, true); // Duplicate

         df = new KDesktopFile(file);
         QString oldCaption = df->readName();
         QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption, oldCaption);
         if (oldCaption != newCaption)
            df->writeEntry("Name", newCaption);
      }
      else if (command == MOVE_FILE)
      {
         m_clipboard = COPY_FILE + file; // Next one copies.

         df = new KDesktopFile(file);
         QString oldCaption = df->readName();
         QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption);
         if (oldCaption != newCaption)
            df->writeEntry("Name", newCaption);
      }
      // Add file to menu
      m_menuFile->addEntry(folder, file);

      KService *s = new KService(df);

      // update fileInfo data
      parentFolderInfo->add(s);

      // create the TreeItem
      if(parentItem)
         parentItem->setOpen(true);

      TreeItem *newItem = createTreeItem(parentItem, item, s);
      delete df;

      setSelected ( newItem, true);
      itemSelected( newItem);
   }
   else
   {
      return; // Error
   }
}

void TreeView::del()
{
    TreeItem *item = (TreeItem*)selectedItem();

    // nil selected? -> nil to delete
    if (item == 0) return;
    
    del(item, true);

    m_ac->action("edit_cut")->setEnabled(false);
    m_ac->action("edit_copy")->setEnabled(false);
    m_ac->action("delete")->setEnabled(false);

    // Select new current item
    setSelected( currentItem(), true );
    // Switch the UI to show that item
    itemSelected( selectedItem() );
}

void TreeView::del(TreeItem *item, bool deleteInfo)
{
    // is file a .directory or a .desktop file
    if(item->isDirectory())
    {
        FolderInfo *folderInfo = item->folderInfo();
         
        // Remove FolderInfo
        TreeItem *parentItem = static_cast<TreeItem*>(item->parent());
        FolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : &m_rootFolder;
        parentFolderInfo->take(folderInfo);

        if (m_clipboard == COPY_FOLDER && (m_clipboardInfo == folderInfo))
        {
           // Copy + Del == Cut
           m_clipboard = MOVE_FOLDER; // Clipboard now owns folderInfo
           
        }
        else
        {
           if (folderInfo->takeRecursive(m_clipboardInfo))
              m_clipboard = MOVE_FOLDER; // Clipboard now owns m_clipboardInfo
              
           if (deleteInfo)
              delete folderInfo; // Delete folderInfo
        }

        // Remove from menu
        m_menuFile->removeMenu(item->directory());

        // Remove tree item
        delete item;
qWarning("Deleting folder: SubFolder count = %d Entries count = %d TreeItem count = %d", parentFolderInfo->subFolders.count(), parentFolderInfo->entries.count(), parentItem ? parentItem->childCount() : childCount());
    }
    else
    {
        QString file = item->file();

        // Remove FolderInfo
        TreeItem *parentItem = static_cast<TreeItem*>(item->parent());
        FolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : &m_rootFolder;
        parentFolderInfo->take(file);

        // Remove from menu
        QString folder = parentItem ? parentItem->directory() : QString::null;
        m_menuFile->removeEntry(folder, file);
           
        // Remove tree item
        delete item;
qWarning("Deleting file: SubFolder count = %d Entries count = %d TreeItem count = %d", parentFolderInfo->subFolders.count(), parentFolderInfo->entries.count(), parentItem ? parentItem->childCount() : childCount());
    }
}

void TreeView::undel()
{
    TreeItem *item = (TreeItem*)selectedItem();

    // nil selected? -> nil to delete
    if (!item || !item->isHidden()) return;
    
    undel(item);
    
    m_ac->action("edit_cut")->setEnabled(false);
    m_ac->action("edit_copy")->setEnabled(false);
    m_ac->action("delete")->setEnabled(false);

    // Select new current item
    setSelected( currentItem(), true );
    // Switch the UI to show that item
    itemSelected( selectedItem() );
}

void TreeView::undel(TreeItem *item)
{
    KDesktopFile df(item->file());
    df.writeEntry("Name", item->name());
    df.deleteEntry("Hidden");
    df.deleteEntry("NoDisplay");
    df.sync();

    item->setHidden(false);

    item->load();
    
    QListViewItem * myChild = item->firstChild();
    while( myChild ) {
        TreeItem *childItem = static_cast<TreeItem*>(myChild);
        undel(childItem);
        myChild = myChild->nextSibling();
    }
}

void TreeView::cleanupClipboard() {
    if (m_clipboard == MOVE_FOLDER)
       delete m_clipboardInfo;
    m_clipboardInfo = 0;

    m_clipboard = QString::null;
}
