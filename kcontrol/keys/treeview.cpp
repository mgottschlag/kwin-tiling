/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2001-2002 Raffaele Sandrini <sandrini@kde.org)
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
#include <klineeditdlg.h>
#include <klocale.h>
#include <kservicegroup.h>
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

AppTreeItem::AppTreeItem(QListViewItem *parent, const QString& file)
    : KListViewItem(parent), m_init(false), m_file(file) {}

AppTreeItem::AppTreeItem(QListViewItem *parent, QListViewItem *after, const QString& file)
    : KListViewItem(parent, after), m_init(false), m_file(file) {}

AppTreeItem::AppTreeItem(QListView *parent, const QString& file)
    : KListViewItem(parent), m_init(false), m_file(file) {}

AppTreeItem::AppTreeItem(QListView *parent, QListViewItem *after, const QString& file)
    : KListViewItem(parent, after), m_init(false), m_file(file) {}

void AppTreeItem::setName(const QString &name)
{
    m_name = name;
    setText(0, m_name);
}

void AppTreeItem::setAccel(const QString &accel)
{
    m_accel = accel;
    int temp = accel.find(';');
    if (temp != -1)
    {
        setText(1, accel.left(temp));
        setText(2, accel.right(accel.length() - temp - 1));
    }
    else
    {
        setText(1, m_accel);
        setText(2, QString::null);
    }
}

void AppTreeItem::setOpen(bool o)
{
    if (o && !m_directoryPath.isEmpty() && !m_init)
    {
       m_init = true;
       AppTreeView *tv = static_cast<AppTreeView *>(listView());
       tv->fillBranch(m_directoryPath, this);
    }
    QListViewItem::setOpen(o);
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


AppTreeView::AppTreeView( QWidget *parent, const char *name )
    : KListView(parent, name)
{
    setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(true);
    setSorting(-1);
    setAcceptDrops(false);
    setDragEnabled(false);
    setMinimumWidth(240);
    setResizeMode(AllColumns);

    addColumn(i18n("Command"));
    addColumn(i18n("Shortcut"));
    addColumn(i18n("Alternate"));

    connect(this, SIGNAL(clicked( QListViewItem* )),
            SLOT(itemSelected( QListViewItem* )));

    connect(this,SIGNAL(selectionChanged ( QListViewItem * )),
            SLOT(itemSelected( QListViewItem* )));
}

AppTreeView::~AppTreeView()
{
}

void AppTreeView::fill()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    clear();
    fillBranch(QString::null, 0);
    QApplication::restoreOverrideCursor();
}

void AppTreeView::fillBranch(const QString& rPath, AppTreeItem *parent)
{
    // get rid of leading slash in the relative path
    QString relPath = rPath;
    if(relPath[0] == '/')
        relPath = relPath.mid(1, relPath.length());

    // We ask KSycoca to give us all services (sorted).
    KServiceGroup::Ptr root = KServiceGroup::group(relPath);

    if (!root || !root->isValid())
        return;

    KServiceGroup::List list = root->entries(true);

    QListViewItem *after = 0;

    for(KServiceGroup::List::ConstIterator it = list.begin();
        it != list.end(); ++it) 
    {
        KSycocaEntry * e = *it;

        if (e->isType(KST_KServiceGroup)) 
        {
            KServiceGroup::Ptr g(static_cast<KServiceGroup *>(e));
            QString groupCaption = g->caption();

            // Item names may contain ampersands. To avoid them being converted
            // to accelerators, replace them with two ampersands.
            groupCaption.replace("&", "&&");

            AppTreeItem *item;
            if (parent == 0)
                item = new AppTreeItem(this,  after, g->directoryEntryPath());
            else
                item = new AppTreeItem(parent, after, g->directoryEntryPath());

            item->setName(groupCaption);
            item->setPixmap(0, appIcon(g->icon()));
            item->setDirectoryPath(g->relPath());
            item->setExpandable(true);
            after = item;
        }
        else if (e->isType(KST_KService)) 
        {
            KService::Ptr s(static_cast<KService *>(e));
            QString serviceCaption = s->name();

            // Item names may contain ampersands. To avoid them being converted
            // to accelerators, replace them with two ampersands.
            serviceCaption.replace("&", "&&");

            AppTreeItem* item;
            if (parent == 0)
                item = new AppTreeItem(this, after, s->desktopEntryPath());
            else
                item = new AppTreeItem(parent, after, s->desktopEntryPath());

            item->setName(serviceCaption);
            item->setAccel(KHotKeys::getMenuEntryShortcut(s->desktopEntryPath()));
            item->setPixmap(0, appIcon(s->icon()));

            after = item;
        }
    }
}

void AppTreeView::itemSelected(QListViewItem *item)
{
    AppTreeItem *_item = static_cast<AppTreeItem*>(item);

    if(!item) return;

    emit entrySelected(_item->file(), _item->accel(), _item->isDirectory());
}

void AppTreeView::currentChanged(const QString& file)
{
    AppTreeItem *item = (AppTreeItem*)selectedItem();
    if (item == 0) return;

    item->setFile(file);

    KDesktopFile df(file);
    item->setName(df.readName());
    item->setPixmap(0, appIcon(df.readIcon()));
}

QStringList AppTreeView::fileList(const QString& rPath)
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

QStringList AppTreeView::dirList(const QString& rPath)
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

void AppTreeView::newitem()
{
    KLineEditDlg dlg(i18n("Item name:"), QString::null, this);
    dlg.setCaption(i18n("New Item"));

    if (!dlg.exec()) return;
    QString filename = dlg.text();
    if ( filename.contains('/'))
    {
        KMessageBox::error( this,  i18n("Item name cannot contain '/'"));
        return;
    }
    AppTreeItem *item = (AppTreeItem*)selectedItem();

    QListViewItem* parent = 0;
    QListViewItem* after = 0;

    QString sfile;

    if(item){
	if(item->isExpandable())
	    parent = item;
        else {
            parent = item->parent();
            after = item;
        }
	sfile = item->file();
    }

    if(parent)
        parent->setOpen(true);

    QString dir = sfile;

    // truncate ".directory" or "blah.desktop"

    int pos = dir.findRev('/');

    if (pos > 0)
	dir.truncate(pos);
    else
	dir = QString::null;

    if(!dir.isEmpty())
	dir += '/';
    dir += filename + ".desktop";

    QFile f(locate("apps", dir));
    if (f.exists()) {
    	KMessageBox::sorry(0, i18n("A file already exists with that name. Please provide another name."), i18n("File Exists"));
	return;
    }

    AppTreeItem* newitem;

    if (!parent)
	newitem = new AppTreeItem(this, after, dir);
    else
	newitem = new AppTreeItem(parent, after, dir);

    newitem->setName(filename);
    newitem->setPixmap(0, appIcon("unkown"));

    KConfig c(locateLocal("apps", dir));
    c.setDesktopGroup();
    c.writeEntry("Name", filename);
    c.writeEntry("Icon", filename);
    c.writeEntry("Type", "Application");
    c.sync();
    setSelected ( newitem, true);
    itemSelected( newitem);
}
