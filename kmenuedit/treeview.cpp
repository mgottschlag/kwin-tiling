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

#include <qcstring.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <qdir.h>
#include <qdragobject.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qstringlist.h>

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
#include <kmultipledrag.h>
#include <kurldrag.h>

#include "treeview.h"
#include "treeview.moc"
#include "khotkeys.h"
#include "menufile.h"
#include "menuinfo.h"

#define MOVE_FOLDER 'M'
#define COPY_FOLDER 'C'
#define MOVE_FILE   'm'
#define COPY_FILE   'c'
#define COPY_SEPARATOR 'S'

TreeItem::TreeItem(QListViewItem *parent, QListViewItem *after, const QString& menuId, bool __init)
    :QListViewItem(parent, after), _hidden(false), _init(__init), _layoutDirty(false), _menuId(menuId),
     m_folderInfo(0), m_entryInfo(0) {}

TreeItem::TreeItem(QListView *parent, QListViewItem *after, const QString& menuId, bool __init)
    : QListViewItem(parent, after), _hidden(false), _init(__init), _layoutDirty(false), _menuId(menuId),
     m_folderInfo(0), m_entryInfo(0) {}

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

void TreeItem::paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
    QListViewItem::paintCell(p, cg, column, width, align);

    if (!m_folderInfo && !m_entryInfo)
    {
       // Draw Separator
       int h = (height() / 2) -1;
       if (isSelected())
          p->setPen( cg.highlightedText() );
       else
          p->setPen( cg.text() );
       p->drawLine(0,  h,
                   width, h);
    }
}

void TreeItem::setup()
{
    QListViewItem::setup();
    if (!m_folderInfo && !m_entryInfo)
       setHeight(8);
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


TreeView::TreeView( bool controlCenter, KActionCollection *ac, QWidget *parent, const char *name )
    : KListView(parent, name), m_ac(ac), m_rmb(0), m_clipboard(0),
      m_clipboardFolderInfo(0), m_clipboardEntryInfo(0),
      m_controlCenter(controlCenter), m_layoutDirty(false)
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
    if (m_ac->action("newsep"))
        connect(m_ac->action("newsep"), SIGNAL(activated()), SLOT(newsep()));

    m_menuFile = new MenuFile( locateLocal("xdgconf-menu", "applications-kmenuedit.menu"));
    m_rootFolder = new MenuFolderInfo;
    m_separator = new MenuSeparatorInfo;
    m_drag = 0;

    //	Read menu format configuration information
    KSharedConfig::Ptr		pConfig = KSharedConfig::openConfig("kickerrc");
	
    pConfig->setGroup("menus");
    m_detailedMenuEntries = pConfig->readBoolEntry("DetailedMenuEntries",true);
    if (m_detailedMenuEntries)
    {
        m_detailedEntriesNamesFirst = pConfig->readBoolEntry("DetailedEntriesNamesFirst",false);
    }
}

TreeView::~TreeView() {
    cleanupClipboard();
    delete m_rootFolder;
    delete m_separator;
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

    m_rmb->insertSeparator();

    if(m_ac->action("newitem"))
	m_ac->action("newitem")->plug(m_rmb);
    if(m_ac->action("newsubmenu"))
	m_ac->action("newsubmenu")->plug(m_rmb);
    if(m_ac->action("newsep"))
	m_ac->action("newsep")->plug(m_rmb);

    m_showHidden = showHidden;
    readMenuFolderInfo();
    fill();
}

void TreeView::readMenuFolderInfo(MenuFolderInfo *folderInfo, KServiceGroup::Ptr folder, const QString &prefix)
{
    if (!folderInfo)
    {
       folderInfo = m_rootFolder;
       if (m_controlCenter)
          folder = KServiceGroup::baseGroup("settings");
       else
          folder = KServiceGroup::root();
    }

    if (!folder || !folder->isValid())
        return;

    folderInfo->caption = folder->caption();
    folderInfo->comment = folder->comment();

    // Item names may contain ampersands. To avoid them being converted
    // to accelerators, replace them with two ampersands.
    folderInfo->hidden = folder->noDisplay();
    folderInfo->directoryFile = folder->directoryEntryPath();
    folderInfo->icon = folder->icon();
    QString id = folder->relPath();
    int i = id.findRev('/', -2);
    id = id.mid(i+1);
    folderInfo->id = id;
    folderInfo->fullId = prefix + id;

    KServiceGroup::List list = folder->entries(true, !m_showHidden, true, m_detailedMenuEntries && !m_detailedEntriesNamesFirst);

    for(KServiceGroup::List::ConstIterator it = list.begin();
        it != list.end(); ++it)
    {
        KSycocaEntry * e = *it;

        if (e->isType(KST_KServiceGroup))
        {
            KServiceGroup::Ptr g(static_cast<KServiceGroup *>(e));
            MenuFolderInfo *subFolderInfo = new MenuFolderInfo();
            readMenuFolderInfo(subFolderInfo, g, folderInfo->fullId);
            folderInfo->add(subFolderInfo, true);
        }
        else if (e->isType(KST_KService))
        {
            folderInfo->add(new MenuEntryInfo(static_cast<KService *>(e)), true);
        }
        else if (e->isType(KST_KServiceSeparator))
        {
            folderInfo->add(m_separator, true);
        }
    }
}

void TreeView::fill()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    clear();
    fillBranch(m_rootFolder, 0);
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

TreeItem *TreeView::createTreeItem(TreeItem *parent, QListViewItem *after, MenuFolderInfo *folderInfo, bool _init)
{
   TreeItem *item;
   if (parent == 0)
     item = new TreeItem(this, after, QString::null, _init);
   else
     item = new TreeItem(parent, after, QString::null, _init);

   item->setMenuFolderInfo(folderInfo);
   item->setName(folderInfo->caption);
   item->setPixmap(0, appIcon(folderInfo->icon));
   item->setDirectoryPath(folderInfo->fullId);
   item->setHidden(folderInfo->hidden);
   item->setExpandable(true);
   return item;
}

TreeItem *TreeView::createTreeItem(TreeItem *parent, QListViewItem *after, MenuEntryInfo *entryInfo, bool _init)
{
   bool hidden = entryInfo->hidden;

   TreeItem* item;
   if (parent == 0)
     item = new TreeItem(this, after, entryInfo->menuId(), _init);
   else
     item = new TreeItem(parent, after, entryInfo->menuId(),_init);

   QString	name;
	 
   if (m_detailedMenuEntries && entryInfo->description.length() != 0)
   {
      if (m_detailedEntriesNamesFirst)
      {
         name = entryInfo->caption + " (" + entryInfo->description + ")";
      }
      else
      {
         name = entryInfo->description + " (" + entryInfo->caption + ")";
      }
   }
   else
   {
      name = entryInfo->caption;
   }
   item->setMenuEntryInfo(entryInfo);
   item->setName(name);
   item->setPixmap(0, appIcon(entryInfo->icon));

   item->setHidden(hidden);
   return item;
}

TreeItem *TreeView::createTreeItem(TreeItem *parent, QListViewItem *after, MenuSeparatorInfo *, bool _init)
{
   TreeItem* item;
   if (parent == 0)
     item = new TreeItem(this, after, QString::null, _init);
   else
     item = new TreeItem(parent, after, QString::null,_init);

   return item;
}

void TreeView::fillBranch(MenuFolderInfo *folderInfo, TreeItem *parent)
{
    QString relPath = parent ? parent->directory() : QString::null;
    QPtrListIterator<MenuInfo> it( folderInfo->initialLayout );
    TreeItem *after = 0;
    for (MenuInfo *info; (info = it.current()); ++it)
    {
       MenuEntryInfo *entry = dynamic_cast<MenuEntryInfo*>(info);
       if (entry)
       {
          after = createTreeItem(parent, after, entry);
          continue;
       }
       
       MenuFolderInfo *subFolder = dynamic_cast<MenuFolderInfo*>(info);
       if (subFolder)
       {
          after = createTreeItem(parent, after, subFolder);
          continue;
       }
       MenuSeparatorInfo *separator = dynamic_cast<MenuSeparatorInfo*>(info);
       if (separator)
       {
          after = createTreeItem(parent, after, separator);
          continue;
       }
    }
}

void TreeView::closeAllItems(QListViewItem *item)
{
    if (!item) return;
    while(item)
    {
       item->setOpen(false);
       closeAllItems(item->firstChild());
       item = item->nextSibling();
    }
}

void TreeView::selectMenu(const QString &menu)
{
   closeAllItems(firstChild());
   
   if (menu.length() <= 1)
   {
      setCurrentItem(firstChild());
      clearSelection();
      return; // Root menu
   }
      
   QString restMenu = menu.mid(1);
   if (!restMenu.endsWith("/"))
      restMenu += "/";

   TreeItem *item = 0;
   do
   {
      int i = restMenu.find("/");
      QString subMenu = restMenu.left(i+1);
      restMenu = restMenu.mid(i+1);
   
      item = (TreeItem*)(item ? item->firstChild() : firstChild());
      while(item)
      {
         MenuFolderInfo *folderInfo = item->folderInfo();
         if (folderInfo && (folderInfo->id == subMenu))
         {
            item->setOpen(true);
            break;
         }
         item = (TreeItem*) item->nextSibling();
      }
   }
   while( item && !restMenu.isEmpty());

   if (item)
   {
      setCurrentItem(item);
      ensureItemVisible(item);
   }
}

void TreeView::selectMenuEntry(const QString &menuEntry)
{
   TreeItem *item = (TreeItem *) selectedItem();
   if (!item)
   {
      item = (TreeItem *) currentItem();
      while (item && item->isDirectory())
          item = (TreeItem*) item->nextSibling();
   }
   else
       item = (TreeItem *) item->firstChild();

   while(item)
   {
      MenuEntryInfo *entry = item->entryInfo();
      if (entry && (entry->menuId() == menuEntry))
      {
         setCurrentItem(item);
         ensureItemVisible(item);
         return;
      }
      item = (TreeItem*) item->nextSibling();
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

    if(!item)
    {
        emit disableAction();
        return;
    }

    if (_item->isDirectory())
       emit entrySelected(_item->folderInfo());
    else
       emit entrySelected(_item->entryInfo());
}

void TreeView::currentChanged(MenuFolderInfo *folderInfo)
{
    TreeItem *item = (TreeItem*)selectedItem();
    if (item == 0) return;
    if (folderInfo == 0) return;

    item->setName(folderInfo->caption);
    item->setPixmap(0, appIcon(folderInfo->icon));
}

void TreeView::currentChanged(MenuEntryInfo *entryInfo)
{
    TreeItem *item = (TreeItem*)selectedItem();
    if (item == 0) return;
    if (entryInfo == 0) return;

    QString	name;
	 
    if (m_detailedMenuEntries && entryInfo->description.length() != 0)
    {
        if (m_detailedEntriesNamesFirst)
	 {
            name = entryInfo->caption + " (" + entryInfo->description + ")";
        }
	 else
        {
            name = entryInfo->description + " (" + entryInfo->caption + ")";
        }
    }
    else
    {
        name = entryInfo->caption;
    }
    item->setName(name);
    item->setPixmap(0, appIcon(entryInfo->icon));
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
    return e->provides("application/x-kmenuedit-internal") &&
           (e->source() == const_cast<TreeView *>(this));
}


static QString createDesktopFile(const QString &file, QString *menuId, QStringList *excludeList)
{
   QString base = file.mid(file.findRev('/')+1);
   base = base.left(base.findRev('.'));

   QRegExp r("(.*)(?=-\\d+)");
   base = (r.search(base) > -1) ? r.cap(1) : base;

   QString result = KService::newServicePath(true, base, menuId, excludeList);
   excludeList->append(*menuId);
   // Todo for Undo-support: Undo menuId allocation:

   return result;
}

static KDesktopFile *copyDesktopFile(MenuEntryInfo *entryInfo, QString *menuId, QStringList *excludeList)
{
   QString result = createDesktopFile(entryInfo->file(), menuId, excludeList);
   KDesktopFile *df = entryInfo->desktopFile()->copyTo(result);
   df->deleteEntry("Categories"); // Don't set any categories!
   
   return df;
}

static QString createDirectoryFile(const QString &file, QStringList *excludeList)
{
   QString base = file.mid(file.findRev('/')+1);
   base = base.left(base.findRev('.'));

   QString result;
   int i = 1;
   while(true)
   {
      if (i == 1)
         result = base + ".directory";
      else
         result = base + QString("-%1.directory").arg(i);

      if (!excludeList->contains(result))
      {
         if (locate("xdgdata-dirs", result).isEmpty())
            break;
      }
      i++;
   }
   excludeList->append(result);
   result = locateLocal("xdgdata-dirs", result);
   return result;
}


void TreeView::slotDropped (QDropEvent * e, QListViewItem *parent, QListViewItem*after)
{
   if(!e) return;

   if (e->source() != this) return; // Only internal drags are supported atm

   TreeItem *parentItem = static_cast<TreeItem*>(parent);

   // is there content in the clipboard?
   if (!m_drag) return;

   if (m_dragItem == after) return; // Nothing to do

   // get destination folder
   QString folder = parentItem ? parentItem->directory() : QString::null;
   MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
   int command = m_drag;
   if (command == MOVE_FOLDER)
   {
      MenuFolderInfo *folderInfo = m_dragInfo;
      if (e->action() == QDropEvent::Copy)
      {
         // Ugh.. this is hard :)
         // * Create new .directory file
         // Add
      }
      else
      {
          TreeItem *tmpItem = static_cast<TreeItem*>(parentItem);
          while (  tmpItem )
          {
              if (  tmpItem == m_dragItem )
              {
                  m_drag = 0;
                  return;
              }
              tmpItem = static_cast<TreeItem*>(tmpItem->parent() );
          }

         // Remove MenuFolderInfo
         TreeItem *oldParentItem = static_cast<TreeItem*>(m_dragItem->parent());
         MenuFolderInfo *oldParentFolderInfo = oldParentItem ? oldParentItem->folderInfo() : m_rootFolder;
         oldParentFolderInfo->take(folderInfo);

         // Move menu
         QString oldFolder = folderInfo->fullId;
         QString folderName = folderInfo->id;
         QString newFolder = m_menuFile->uniqueMenuName(folder, folderName, parentFolderInfo->existingMenuIds());
         folderInfo->id = newFolder;

         // Add file to menu
         //m_menuFile->moveMenu(oldFolder, folder + newFolder);
         m_menuFile->pushAction(MenuFile::MOVE_MENU, oldFolder, folder + newFolder);

         // Make sure caption is unique
         QString newCaption = parentFolderInfo->uniqueMenuCaption(folderInfo->caption);
         if (newCaption != folderInfo->caption)
         {
            folderInfo->setCaption(newCaption);
         }

	 // create the TreeItem
	 if(parentItem)
	   parentItem->setOpen(true);

         // update fileInfo data
         folderInfo->updateFullId(parentFolderInfo->fullId);
         folderInfo->setInUse(true);
         parentFolderInfo->add(folderInfo);

         if ((parentItem != oldParentItem) || !after)
         {
            if (oldParentItem)
               oldParentItem->takeItem(m_dragItem);
            else
               takeItem(m_dragItem);
            if (parentItem)
               parentItem->insertItem(m_dragItem);
            else
               insertItem(m_dragItem);
         }
         m_dragItem->moveItem(after);
         m_dragItem->setName(folderInfo->caption);
         m_dragItem->setDirectoryPath(folderInfo->fullId);
         setSelected(m_dragItem, true);
         itemSelected(m_dragItem);
      }
   }
   else if (command == MOVE_FILE)
   {
      MenuEntryInfo *entryInfo = m_dragItem->entryInfo();
      QString menuId = entryInfo->menuId();

      if (e->action() == QDropEvent::Copy)
      {

         // Need to copy file and then add it
         KDesktopFile *df = copyDesktopFile(entryInfo, &menuId, &m_newMenuIds); // Duplicate
//UNDO-ACTION: NEW_MENU_ID (menuId)

         KService *s = new KService(df);
         s->setMenuId(menuId);

         entryInfo = new MenuEntryInfo(s, df);

         QString oldCaption = entryInfo->caption;
         QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption, oldCaption);
         entryInfo->setCaption(newCaption);
      }
      else
      {
         del(m_dragItem, false);
         QString oldCaption = entryInfo->caption;
         QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption);
         entryInfo->setCaption(newCaption);
         entryInfo->setInUse(true);
      }
      // Add file to menu
      // m_menuFile->addEntry(folder, menuId);
      m_menuFile->pushAction(MenuFile::ADD_ENTRY, folder, menuId);

      // create the TreeItem
      if(parentItem)
         parentItem->setOpen(true);

      // update fileInfo data
      parentFolderInfo->add(entryInfo);

      TreeItem *newItem = createTreeItem(parentItem, after, entryInfo, true);

      setSelected ( newItem, true);
      itemSelected( newItem);
   }
   else if (command == COPY_SEPARATOR)
   {
      if (e->action() != QDropEvent::Copy)
         del(m_dragItem, false);

      TreeItem *newItem = createTreeItem(parentItem, after, m_separator, true);
            
      setSelected ( newItem, true);
      itemSelected( newItem);
   }
   else
   {
      // Error
   }
   m_drag = 0;
   setLayoutDirty(parentItem);
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
    m_dragPath = QString::null;
    TreeItem *item = (TreeItem*)selectedItem();
    if(item == 0) return 0;

    KMultipleDrag *drag = new KMultipleDrag( this );

    if (item->isDirectory())
    {
       m_drag = MOVE_FOLDER;
       m_dragInfo = item->folderInfo();
       m_dragItem = item;
    }
    else if (item->isEntry())
    {
       m_drag = MOVE_FILE;
       m_dragInfo = 0;
       m_dragItem = item;
       QString menuId = item->menuId();
       m_dragPath = item->entryInfo()->service->desktopEntryPath();
       if (!m_dragPath.isEmpty())
          m_dragPath = locate("apps", m_dragPath);
       if (!m_dragPath.isEmpty())
       {
          KURL url;
          url.setPath(m_dragPath);
          drag->addDragObject( new KURLDrag(url, 0));
       }
    }
    else
    {
       m_drag = COPY_SEPARATOR;
       m_dragInfo = 0;
       m_dragItem = item;
    }

    drag->addDragObject( new QStoredDrag("application/x-kmenuedit-internal", 0));
    if ( item->pixmap(0) )
        drag->setPixmap(*item->pixmap(0));
    return drag;
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

   bool ok;
   QString caption = KInputDialog::getText( i18n( "New Submenu" ),
        i18n( "Submenu name:" ), QString::null, &ok, this );

   if (!ok) return;

   QString file = caption;
   file.replace('/', '-');

   file = createDirectoryFile(file, &m_newDirectoryList); // Create

   // get destination folder
   QString folder;

   if(!item)
   {
      parentItem = 0;
      folder = QString::null;
   }
   else if(item->isDirectory())
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

   MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
   MenuFolderInfo *folderInfo = new MenuFolderInfo();
   folderInfo->caption = parentFolderInfo->uniqueMenuCaption(caption);
   folderInfo->id = m_menuFile->uniqueMenuName(folder, caption, parentFolderInfo->existingMenuIds());
   folderInfo->directoryFile = file;
   folderInfo->icon = "package";
   folderInfo->hidden = false;
   folderInfo->setDirty();

   KDesktopFile *df = new KDesktopFile(file);
   df->writeEntry("Name", folderInfo->caption);
   df->writeEntry("Icon", folderInfo->icon);
   df->sync();
   delete df;
   // Add file to menu
   // m_menuFile->addMenu(folder + folderInfo->id, file);
   m_menuFile->pushAction(MenuFile::ADD_MENU, folder + folderInfo->id, file);

   folderInfo->fullId = parentFolderInfo->fullId + folderInfo->id;

   // create the TreeItem
   if(parentItem)
      parentItem->setOpen(true);

   // update fileInfo data
   parentFolderInfo->add(folderInfo);

   TreeItem *newItem = createTreeItem(parentItem, item, folderInfo, true);

   setSelected ( newItem, true);
   itemSelected( newItem);

   setLayoutDirty(parentItem);
}

void TreeView::newitem()
{
   TreeItem *parentItem = 0;
   TreeItem *item = (TreeItem*)selectedItem();

   bool ok;
   QString caption = KInputDialog::getText( i18n( "New Item" ),
        i18n( "Item name:" ), QString::null, &ok, this );

   if (!ok) return;

   QString menuId;
   QString file = caption;
   file.replace('/', '-');

   file = createDesktopFile(file, &menuId, &m_newMenuIds); // Create

   KDesktopFile *df = new KDesktopFile(file);
   df->writeEntry("Name", caption);
   df->writeEntry("Type", "Application");

   // get destination folder
   QString folder;

   if(!item)
   {
      parentItem = 0;
      folder = QString::null;
   }
   else if(item->isDirectory())
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

   MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;

   // Add file to menu
   // m_menuFile->addEntry(folder, menuId);
   m_menuFile->pushAction(MenuFile::ADD_ENTRY, folder, menuId);

   KService *s = new KService(df);
   s->setMenuId(menuId);

   MenuEntryInfo *entryInfo = new MenuEntryInfo(s, df);

   // create the TreeItem
   if(parentItem)
      parentItem->setOpen(true);

   // update fileInfo data
   parentFolderInfo->add(entryInfo);

   TreeItem *newItem = createTreeItem(parentItem, item, entryInfo, true);

   setSelected ( newItem, true);
   itemSelected( newItem);

   setLayoutDirty(parentItem);
}

void TreeView::newsep()
{
   TreeItem *parentItem = 0;
   TreeItem *item = (TreeItem*)selectedItem();

   if(!item)
   {
      parentItem = 0;
   }
   else if(item->isDirectory())
   {
      parentItem = item;
      item = 0;
   }
   else
   {
      parentItem = static_cast<TreeItem*>(item->parent());
   }

   // create the TreeItem
   if(parentItem)
      parentItem->setOpen(true);

   TreeItem *newItem = createTreeItem(parentItem, item, m_separator, true);

   setSelected ( newItem, true);
   itemSelected( newItem);

   setLayoutDirty(parentItem);
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

    if (cutting)
       setLayoutDirty((TreeItem*)item->parent());

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
           m_clipboardFolderInfo = item->folderInfo();

           del(item, false);
        }
        else
        {
           // Place in clipboard
           m_clipboard = COPY_FOLDER;
           m_clipboardFolderInfo = item->folderInfo();
        }
    }
    else if (item->isEntry())
    {
        if (cutting)
        {
           // Place in clipboard
           m_clipboard = MOVE_FILE;
           m_clipboardEntryInfo = item->entryInfo();

           del(item, false);
        }
        else
        {
           // Place in clipboard
           m_clipboard = COPY_FILE;
           m_clipboardEntryInfo = item->entryInfo();
        }
    }
    else
    {
        // Place in clipboard
        m_clipboard = COPY_SEPARATOR;
        if (cutting)
           del(item, false);
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
   if (!m_clipboard) return;

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

   MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
   int command = m_clipboard;
   if ((command == COPY_FOLDER) || (command == MOVE_FOLDER))
   {
      MenuFolderInfo *folderInfo = m_clipboardFolderInfo;
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
         // m_menuFile->moveMenu(oldFolder, folder + newFolder);
         m_menuFile->pushAction(MenuFile::MOVE_MENU, oldFolder, folder + newFolder);

         // Make sure caption is unique
         QString newCaption = parentFolderInfo->uniqueMenuCaption(folderInfo->caption);
         if (newCaption != folderInfo->caption)
         {
            folderInfo->setCaption(newCaption);
         }
         // create the TreeItem
         if(parentItem)
             parentItem->setOpen(true);

         // update fileInfo data
         folderInfo->fullId = parentFolderInfo->fullId + folderInfo->id;
         folderInfo->setInUse(true);
         parentFolderInfo->add(folderInfo);

         TreeItem *newItem = createTreeItem(parentItem, item, folderInfo);

         setSelected ( newItem, true);
         itemSelected( newItem);
      }

      m_clipboard = COPY_FOLDER; // Next one copies.
   }
   else if ((command == COPY_FILE) || (command == MOVE_FILE))
   {
      MenuEntryInfo *entryInfo = m_clipboardEntryInfo;
      QString menuId;

      if (command == COPY_FILE)
      {
         // Need to copy file and then add it
         KDesktopFile *df = copyDesktopFile(entryInfo, &menuId, &m_newMenuIds); // Duplicate

         KService *s = new KService(df);
         s->setMenuId(menuId);
         entryInfo = new MenuEntryInfo(s, df);

         QString oldCaption = entryInfo->caption;
         QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption, oldCaption);
         entryInfo->setCaption(newCaption);
      }
      else if (command == MOVE_FILE)
      {
         menuId = entryInfo->menuId();
         m_clipboard = COPY_FILE; // Next one copies.

         QString oldCaption = entryInfo->caption;
         QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption);
         entryInfo->setCaption(newCaption);
         entryInfo->setInUse(true);
      }
      // Add file to menu
      // m_menuFile->addEntry(folder, menuId);
      m_menuFile->pushAction(MenuFile::ADD_ENTRY, folder, menuId);

      // create the TreeItem
      if(parentItem)
         parentItem->setOpen(true);

      // update fileInfo data
      parentFolderInfo->add(entryInfo);

      TreeItem *newItem = createTreeItem(parentItem, item, entryInfo, true);

      setSelected ( newItem, true);
      itemSelected( newItem);
   }
   else
   {
      // create separator
      if(parentItem)
         parentItem->setOpen(true);

      TreeItem *newItem = createTreeItem(parentItem, item, m_separator, true);

      setSelected ( newItem, true);
      itemSelected( newItem);
   }
   setLayoutDirty(parentItem);
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
    TreeItem *parentItem = static_cast<TreeItem*>(item->parent());
    // is file a .directory or a .desktop file
    if(item->isDirectory())
    {
        MenuFolderInfo *folderInfo = item->folderInfo();

        // Remove MenuFolderInfo
        MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
        parentFolderInfo->take(folderInfo);
        folderInfo->setInUse(false);

        if (m_clipboard == COPY_FOLDER && (m_clipboardFolderInfo == folderInfo))
        {
           // Copy + Del == Cut
           m_clipboard = MOVE_FOLDER; // Clipboard now owns folderInfo

        }
        else
        {
           if (folderInfo->takeRecursive(m_clipboardFolderInfo))
              m_clipboard = MOVE_FOLDER; // Clipboard now owns m_clipboardFolderInfo

           if (deleteInfo)
              delete folderInfo; // Delete folderInfo
        }

        // Remove from menu
        // m_menuFile->removeMenu(item->directory());
        m_menuFile->pushAction(MenuFile::REMOVE_MENU, item->directory(), QString::null);

        // Remove tree item
        delete item;
    }
    else if (item->isEntry())
    {
        MenuEntryInfo *entryInfo = item->entryInfo();
        QString menuId = entryInfo->menuId();

        // Remove MenuFolderInfo
        MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
        parentFolderInfo->take(entryInfo);
        entryInfo->setInUse(false);

        if (m_clipboard == COPY_FILE && (m_clipboardEntryInfo == entryInfo))
        {
           // Copy + Del == Cut
           m_clipboard = MOVE_FILE; // Clipboard now owns entryInfo
        }
        else
        {
           if (deleteInfo)
              delete entryInfo; // Delete entryInfo
        }

        // Remove from menu
        QString folder = parentItem ? parentItem->directory() : QString::null;
        // m_menuFile->removeEntry(folder, menuId);
        m_menuFile->pushAction(MenuFile::REMOVE_ENTRY, folder, menuId);

        // Remove tree item
        delete item;
    }
    else
    {
        // Remove separator
        delete item;
    }
    setLayoutDirty(parentItem);
}

void TreeView::cleanupClipboard() {
    if (m_clipboard == MOVE_FOLDER)
       delete m_clipboardFolderInfo;
    m_clipboardFolderInfo = 0;

    if (m_clipboard == MOVE_FILE)
       delete m_clipboardEntryInfo;
    m_clipboardEntryInfo = 0;

    m_clipboard = 0;
}

static QStringList extractLayout(TreeItem *item)
{
    bool firstFolder = true;
    bool firstEntry = true;
    QStringList layout;
    for(;item; item = static_cast<TreeItem*>(item->nextSibling()))
    {
       if (item->isDirectory())
       {
          if (firstFolder)
          {
             firstFolder = false;
             layout << ":M"; // Add new folders here...
          }
          layout << (item->folderInfo()->id);
       }
       else if (item->isEntry())
       {
          if (firstEntry)
          {
             firstEntry = false;
             layout << ":F"; // Add new entries here...
          }
          layout << (item->entryInfo()->menuId());
       }
       else
       {
          layout << ":S";
       }
    }
    return layout;
}

QStringList TreeItem::layout()
{
    QStringList layout = extractLayout(static_cast<TreeItem*>(firstChild()));
    _layoutDirty = false;
    return layout;
}

void TreeView::saveLayout()
{
    if (m_layoutDirty)
    {
       QStringList layout = extractLayout(static_cast<TreeItem*>(firstChild()));
       m_menuFile->setLayout(m_rootFolder->fullId, layout);
       m_layoutDirty = false;
    }

    QPtrList<QListViewItem> lst;
    QListViewItemIterator it( this );
    while ( it.current() ) {
       TreeItem *item = static_cast<TreeItem*>(it.current());
       if ( item->isLayoutDirty() )
       {
          m_menuFile->setLayout(item->folderInfo()->fullId, item->layout());
       }
       ++it;
    }
}

bool TreeView::save()
{
    saveLayout();
    m_rootFolder->save(m_menuFile);

    bool success = m_menuFile->performAllActions();

    m_newMenuIds.clear();
    m_newDirectoryList.clear();
    
    if (success)
    {
       KService::rebuildKSycoca(this);
    }
    else
    {
       KMessageBox::sorry(this, "<qt>"+i18n("Menu changes could not be saved because of the following problem:")+"<br><br>"+
                                m_menuFile->error()+"</qt>");
    }
    return success;
}

void TreeView::setLayoutDirty(TreeItem *parentItem)
{
    if (parentItem)
       parentItem->setLayoutDirty();
    else
       m_layoutDirty = true;
}

bool TreeView::isLayoutDirty()
{
    QPtrList<QListViewItem> lst;
    QListViewItemIterator it( this );
    while ( it.current() ) {
       if ( static_cast<TreeItem*>(it.current())->isLayoutDirty() )
          return true;
       ++it;
    }
    return false;
}

bool TreeView::dirty()
{
    return m_layoutDirty || m_rootFolder->hasDirt() || m_menuFile->dirty() || isLayoutDirty();
}

void TreeView::findServiceShortcut(const KShortcut&cut, KService::Ptr &service)
{
    service = m_rootFolder->findServiceShortcut(cut);
}

