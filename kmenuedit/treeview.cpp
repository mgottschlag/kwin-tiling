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
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kdesktopfile.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kapplication.h>

#include "treeview.h"
#include "treeview.moc"
#include "khotkeys.h"

const char* clipboard_prefix = ".kmenuedit_clipboard/";

TreeItem::TreeItem(QListViewItem *parent, const QString& file)
    :QListViewItem(parent), _file(file), _hidden(false), _init(false), _directory(false) {}

TreeItem::TreeItem(QListViewItem *parent, QListViewItem *after, const QString& file)
    :QListViewItem(parent, after), _file(file), _hidden(false), _init(false), _directory(false) {}

TreeItem::TreeItem(QListView *parent, const QString& file)
    : QListViewItem(parent), _file(file), _hidden(false), _init(false), _directory(false) {}

TreeItem::TreeItem(QListView *parent, QListViewItem *after, const QString& file)
    : QListViewItem(parent, after), _file(file), _hidden(false), _init(false), _directory(false) {}

void TreeItem::setName(const QString &name)
{
    _name = name;
    update();
}

QString TreeItem::file() const
{
    return _directory ? _file + "/.directory" : _file;
}

void TreeItem::setDirectory(bool b)
{
    _directory = b;
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
    if (o && _directory && !_init)
    {
       _init = true;
       TreeView *tv = static_cast<TreeView *>(listView());
       tv->fillBranch(_file, this);
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


TreeView::TreeView( KActionCollection *ac, QWidget *parent, const char *name )
    : KListView(parent, name), _ac(ac), _rmb(0)
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
    connect(_ac->action("newitem"), SIGNAL(activated()), SLOT(newitem()));
    connect(_ac->action("newsubmenu"), SIGNAL(activated()), SLOT(newsubmenu()));

    cleanupClipboard();
}

TreeView::~TreeView() {
    cleanupClipboard();
}

void TreeView::setViewMode(bool showHidden)
{
    delete _rmb;

    // setup rmb menu
    _rmb = new QPopupMenu(this);
    KAction *action;

    action = _ac->action("edit_cut");
    if(action) {
        action->plug(_rmb);
        action->setEnabled(false);
        connect(action, SIGNAL(activated()), SLOT(cut()));
    }

    action = _ac->action("edit_copy");
    if(action) {
        action->plug(_rmb);
        action->setEnabled(false);
        connect(action, SIGNAL(activated()), SLOT(copy()));
    }

    action = _ac->action("edit_paste");
    if(action) {
        action->plug(_rmb);
        action->setEnabled(false);
        connect(action, SIGNAL(activated()), SLOT(paste()));
    }

    _rmb->insertSeparator();

    action = _ac->action("delete");
    if(action) {
        action->plug(_rmb);
        action->setEnabled(false);
        connect(action, SIGNAL(activated()), SLOT(del()));
    }

    action = _ac->action("undelete");
    if(action) {
        action->plug(_rmb);
        action->setEnabled(false);
        connect(action, SIGNAL(activated()), SLOT(undel()));
    }

    _rmb->insertSeparator();

    if(_ac->action("newitem"))
	_ac->action("newitem")->plug(_rmb);
    if(_ac->action("newsubmenu"))
	_ac->action("newsubmenu")->plug(_rmb);

    _showHidden = showHidden;
    fill();
}

void TreeView::fill()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    clear();
    fillBranch("", 0);
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

    // first add tree items for the desktop files in this directory
    if (!filelist.isEmpty()) {
	QStringList::ConstIterator it = filelist.end();
	do{
	    --it;

	    KDesktopFile df(*it);
	    bool hidden = df.readBoolEntry("NoDisplay") || df.readBoolEntry("Hidden");

	    if (hidden && !_showHidden)
	        continue;

            QString name = findName(&df, hidden);
            if (name.isEmpty() && df.readEntry("Exec").isEmpty())
                continue;

            TreeItem* item;
            if (parent == 0)
                item = new TreeItem(this, *it);
            else
                item = new TreeItem(parent, *it);

            item->setName(name);
            item->setPixmap(0, appIcon(df.readIcon()));

            item->setHidden(hidden);
	}
	while (it != filelist.begin());
    }

    // add directories and process sudirs
    if (!dirlist.isEmpty()) {
	QStringList::ConstIterator it = dirlist.end();
	do {
	    --it;

	    QString dirFile = *it + "/.directory";
	    TreeItem* item;

	    bool hidden = false;
	    QString name;
	    QString icon;

            KDesktopFile df(dirFile, true);
            hidden = df.readBoolEntry("NoDisplay") || df.readBoolEntry("Hidden");
            if (hidden && !_showHidden)
                continue;

            name = findName(&df, hidden);
            if (name.isEmpty())
                name = *it;

            icon = df.readIcon();
            if (icon.isEmpty())
                icon = "package";

            if (parent == 0)
                item = new TreeItem(this,  *it);
            else
                item = new TreeItem(parent, *it);

            item->setName(name);
            item->setPixmap(0, appIcon(icon));

            item->setHidden(hidden);
            item->setExpandable(true);
            item->setDirectory(true);
	}
	while (it != dirlist.begin());
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

#if 0
        // Check if the file is Writeable for us
        QFileInfo finfo((KGlobal::dirs()->findResourceDir("apps", _item->file())) + _item->file());

        if (finfo.isWritable() && (_item->name() != i18n("Settings")))
            _ac->action("delete")->setEnabled(true);
        else
            _ac->action("delete")->setEnabled(false);
#endif
    }

    _ac->action("edit_cut")->setEnabled(selected);
    _ac->action("edit_copy")->setEnabled(selected);

    if (_ac->action("delete"))
        _ac->action("delete")->setEnabled(selected && !dselected);
    if (_ac->action("undelete"))
        _ac->action("undelete")->setEnabled(selected && dselected);

    if(!item) return;

    emit entrySelected(_item->file(), _item->name(), dselected);
}

void TreeView::currentChanged()
{
    TreeItem *item = (TreeItem*)selectedItem();
    if (item == 0) return;

    KDesktopFile df(item->file());
    item->setName(findName(&df, item->isHidden()));
    item->setPixmap(0, appIcon(df.readIcon()));
}

// moving = src will be removed later
void TreeView::copyFile(const QString& src, const QString& dest, bool moving)
{
    // We can't simply copy a .desktop file as several prefixes might
    // contain a version of that file. To make sure to copy all groups
    // and keys we read all groups and keys via KConfig which handles
    // the merging. We then write out the destination .desktop file
    // in a writeable prefix we get using locateLocal().

    if (src == dest) return;

    kdDebug() << "copyFile: " << src.local8Bit() << " to " << dest.local8Bit() << endl;

    // read-only + don't merge in kdeglobals
    KConfig s(src, true, false, "apps");

    KSimpleConfig d(locateLocal("apps", dest));

    // don't copy hidden files
    if(s.readBoolEntry("Hidden", false) == true)
	return;

    // loop through all groups
    QStringList groups = s.groupList();
    for (QStringList::ConstIterator it = groups.begin(); it != groups.end(); ++it) {
        if(*it == "<default>")
            continue;

        if((*it).contains("Desktop Entry"))
            d.setDesktopGroup();
        else
            d.setGroup(*it);

        // get a map of keys/value pairs
        QMap<QString, QString> map = s.entryMap(*it);

        // iterate through the map and write out key/value pairs to the dest file
        QMap<QString, QString>::ConstIterator iter;
        for (iter = map.begin(); iter != map.end(); ++iter)
            d.writeEntry(iter.key(), iter.data());
    }

    // unset "Hidden"
    d.setDesktopGroup();
    d.writeEntry("Hidden", false);

    d.sync();

    if( moving && KHotKeys::present()) // tell khotkeys this menu entry has moved
        KHotKeys::menuEntryMoved( dest, src );
}

// moving = src will be removed later
void TreeView::copyDir(const QString& s, const QString& d, bool moving )
{
    // We create the destination directory in a writeable prefix returned
    // by locateLocal(), copy the .directory and the .desktop files over.
    // Then process the subdirs.

    QString src = s;
    QString dest = d;

    // truncate "/.directory"
    int pos = src.findRev("/.directory");
    if (pos > 0) src.truncate(pos);
    pos = dest.findRev("/.directory");
    if (pos > 0) dest.truncate(pos);

    if (src == dest) return;

    kdDebug() << "copyDir: " << src.local8Bit() << " to " << dest.local8Bit() << endl;

    QStringList dirlist = dirList(src);
    QStringList filelist = fileList(src);

    // copy .directory file
    copyFile(src + "/.directory", dest + "/.directory", moving );

    kdDebug() << "###" << dest.local8Bit() << endl;
    // copy files
    for (QStringList::ConstIterator it = filelist.begin(); it != filelist.end(); ++it) {
        QString file = (*it).mid((*it).findRev('/'), (*it).length());
        copyFile(src + file, dest + file, moving );
    }
    // process subdirs
    for (QStringList::ConstIterator it = dirlist.begin(); it != dirlist.end(); ++it) {
        QString file = (*it).mid((*it).findRev('/'), (*it).length());
        copyDir(src + file, dest + file, moving );
    }

    // unset hidden flag
    KConfig c(dest + "/.directory", false, false, "apps");
    c.setDesktopGroup();
    c.writeEntry("Hidden", false);
    c.sync();
}

// Return value: 0 - removed everything
//               1 - removed only local
//               2 - removed none
bool TreeView::deleteFile(const QString& deskfile, const bool move)
{
    // We search for the file in all prefixes and remove all writeable
    // ones. If we were not able to remove all (because of lack of permissons)
    // we set the "Hidden" flag in a writeable local file in a path returned
    // by localeLocal().
    bool hasLocal = false;
    bool hasGlobal = false;
    bool isLocal = true;

    // search the selected item in all resource dirs
    QStringList resdirs = KGlobal::dirs()->resourceDirs("apps");
    QString localFile = resdirs.first()+deskfile;

    for (QStringList::ConstIterator it = resdirs.begin(); it != resdirs.end(); ++it) {
        QFile f((*it) + deskfile);

        // continue if it does not exist in this resource dir
        if(f.exists())
        {
           if (isLocal)
              hasLocal = true;
           else
              hasGlobal = true;
        }

        isLocal = false;
    }

    if( KHotKeys::present()) // tell khotkeys this menu entry has been removed
        KHotKeys::menuEntryDeleted( deskfile );

    if (hasLocal)
    {
        ::unlink(QFile::encodeName(localFile).data());
    }

    if(hasGlobal) {
	KSimpleConfig c(localFile);
        c.setDesktopGroup();
        if (move)
        {
           c.writeEntry("Name", "empty");
           c.writeEntry("Hidden", true);
        }
        else
        {
           c.writeEntry("NoDisplay", true);
        }
        c.sync();
    }

    return true;
}

bool TreeView::deleteDir(const QString& d, const bool move)
{
    // We delete all .desktop files and then process with the subdirs.
    // Afterwards the .directory file gets deleted from all prefixes
    // and we try to rmdir the directory in all prefixes.
    // If we don't succed in deleting the directory from all prefixes
    // we add a .directory file with the "Hidden" flag set in a local
    // writeable dir return by locateLocal().
    bool allremoved = true;

    QString directory = d;

    // truncate "/.directory"
    int pos = directory.findRev("/.directory");
    if (pos > 0) directory.truncate(pos);

    kdDebug() << "deleteDir: " << directory.local8Bit() << endl;

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
#if 0
    QDir dir;
    QStringList dirs = KGlobal::dirs()->findDirs("apps", directory);
    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
        // remove all writeable files
        if(!dir.rmdir(*it))
            allremoved = false;
    }
#endif

    if(move) {
        KSimpleConfig c(locateLocal("apps", directory + "/.directory"));
        c.setDesktopGroup();
        c.writeEntry("Name", "empty");
        c.writeEntry("Hidden", true);
        c.sync();
    }
    return allremoved;
}

void TreeView::hideDir(const QString& d, const QString name, bool hide) {
	QString directory = d;
        KConfig c(directory + "/.directory", false, false, "apps");
	c.setDesktopGroup();
	c.writeEntry("Name", name);
	c.writeEntry("NoDisplay", hide);
	c.sync();
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

bool TreeView::acceptDrag(QDropEvent* event) const
{
    return (QString(event->format()).contains("text/plain"));
}

void TreeView::slotDropped (QDropEvent * e, QListViewItem *parent, QListViewItem*after)
{
    if(!e) return;

    // first move the item in the listview
    TreeItem *item = (TreeItem*)selectedItem();

    moveItem(item, parent, after);
    setOpen(parent, true);
    setSelected(item, true);

    // get source path from qdropevent
    QByteArray a = e->encodedData("text/plain");
    if (a.isEmpty()) return;
    QString src(a);

    bool isDir = src.find(".directory") > 0;

    kdDebug() << "src: " << src.local8Bit() << endl;

    // get source file
    int pos = src.findRev('/');
    if (isDir)
	pos = src.findRev('/', pos-1);
    QString srcfile;

    if (pos < 0)
	srcfile = src;
    else
	srcfile = src.mid(pos + 1, src.length());

    // get dest path
    QString dest;
    if (item->parent())
	dest = ((TreeItem*)item->parent())->file();

    // truncate file
    pos = dest.findRev('/');
    if (pos > 0) dest.truncate(pos);

    if(dest.isNull())
	dest = srcfile;
    else
	dest += '/' + srcfile;

    kdDebug() << "dest: " << dest.local8Bit() << endl;

    item->setFile(dest);
    if(src == dest) return;

    if(isDir) {
        copyDir(src, dest, true );
        deleteDir(src, true);
    }
    else {
        copyFile(src, dest, true );
        deleteFile(src, true);
    }
}

QDragObject *TreeView::dragObject()
{
    TreeItem *item = (TreeItem*)selectedItem();
    if(item == 0) return 0;

    QTextDrag *d = new QTextDrag(item->file(), (QWidget*)this);
    d->setPixmap(*item->pixmap(0));
    return d;
}

void TreeView::slotRMBPressed(QListViewItem*, const QPoint& p)
{
    TreeItem *item = (TreeItem*)selectedItem();
    if(item == 0) return;

    if(_rmb) _rmb->exec(p);
}

void TreeView::newsubmenu()
{
    KLineEditDlg dlg(i18n("Submenu name:"), QString::null, this);
    dlg.setCaption(i18n("New Submenu"));

    if (!dlg.exec()) return;
    QString dirname = dlg.text();

    TreeItem *item = (TreeItem*)selectedItem();

    QListViewItem* parent = 0;
    QListViewItem* after = 0;

    QString sfile;

    if(item && item->isDirectory()){
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

    if(sfile.find(".directory") > 0) {
        // truncate "blah/.directory"

        int pos = dir.findRev('/');
        int pos2 = dir.findRev('/', pos-1);

        if (pos2 >= 0)
            pos = pos2;

        if (pos > 0)
            dir.truncate(pos);
        else
            dir = QString::null;
    }
    else if (dir.find(".desktop")) {
        // truncate "blah.desktop"
        int pos = dir.findRev('/');

        if (pos > 0)
            dir.truncate(pos);
        else
            dir = QString::null;
    }
    if(!dir.isEmpty())
	dir += '/';

    dir += dirname + "/.directory";

    QFile f(locateLocal("apps", dir));
    if (f.exists()) {
    	KMessageBox::sorry(0, i18n("A file exists with that name. Please provide another name."), i18n("File Exists"));
	return;
    }

    TreeItem* newitem;

    if (!parent)
	newitem = new TreeItem(this, after, dir);
    else
	newitem = new TreeItem(parent, after, dir);

    newitem->setName(dirname);
    newitem->setPixmap(0, appIcon("package"));
    newitem->setExpandable(true);

    KConfig c(locateLocal("apps", dir));
    c.setDesktopGroup();
    c.writeEntry("Name", dirname);
    c.writeEntry("Icon", "package");
    c.sync();
    setSelected ( newitem, true);
    itemSelected( newitem);
}

void TreeView::newitem()
{
    KLineEditDlg dlg(i18n("Item name:"), QString::null, this);
    dlg.setCaption(i18n("New Item"));

    if (!dlg.exec()) return;
    QString filename = dlg.text();

    TreeItem *item = (TreeItem*)selectedItem();

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

    TreeItem* newitem;

    if (!parent)
	newitem = new TreeItem(this, after, dir);
    else
	newitem = new TreeItem(parent, after, dir);

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

void TreeView::cut()
{
    copy( true );
    del();
}

void TreeView::copy()
{
    copy( false );
}

void TreeView::copy( bool moving )
{
    TreeItem *item = (TreeItem*)selectedItem();

    // nil selected? -> nil to copy
    if (item == 0) return;

    // clean up old stuff
    cleanupClipboard();

    QString file = item->file();

    // is file a .directory or a .desktop file
    if(file.find(".directory") > 0) {
        _clipboard = file;

        // truncate path
        int pos = _clipboard.findRev('/');
        int pos2 = _clipboard.findRev('/', pos-1);
        if (pos2 >= 0)
            pos = pos2+1;
        else
            pos = 0;

        if (pos > 0)
            _clipboard = _clipboard.mid(pos, _clipboard.length());

        copyDir(file, QString(clipboard_prefix) + _clipboard, moving );
    }
    else if (file.find(".desktop")) {
        _clipboard = file;

        // truncate path
        int pos = _clipboard.findRev('/');
        if (pos >= 0)
            _clipboard = _clipboard.mid(pos+1, _clipboard.length());

        copyFile(file, QString(clipboard_prefix) + _clipboard, moving );
    }
    _ac->action("edit_paste")->setEnabled(true);
}

void TreeView::paste()
{
    TreeItem *item = (TreeItem*)selectedItem();

    // nil selected? -> nil to paste to
    if (item == 0) return;
    // is there content in the clipboard?
    if (_clipboard.isEmpty()) return;

    // get dest
    QString dest = item->file();

    // truncate ".directory"
    int pos = dest.findRev(".directory");
    if (pos > 0) dest.truncate(pos);

    // truncate '/'
    pos = dest.findRev('/');
    if (pos > 0) dest.truncate(pos);

    // truncate ".desktop" and file name
    pos = dest.findRev(".desktop");
    if (pos > 0) {
        dest.truncate(pos);

        // truncate file name
        pos = dest.findRev('/');
        if (pos < 0) pos = 0;
        dest.truncate(pos);
    }

    QString newname = _clipboard;
    QFile f(locate("apps", dest + '/' + newname));

    while (f.exists()) {
        pos = newname.findRev(".desktop");
        newname.insert(pos, " (Copy)");

        f.setName(locate("apps", dest + '/' + newname));
    }

    kdDebug() << "### clip: " << _clipboard.local8Bit() << " dest: " << dest.local8Bit() << " ###" << endl;

    // is _clipboard a .directory or a .desktop file
    if(_clipboard.find(".directory") > 0)// if cut&paste is done, assume it's moving too
        copyDir(QString(clipboard_prefix) + _clipboard, dest + '/' + newname, true );
    else if (_clipboard.find(".desktop"))
        copyFile(QString(clipboard_prefix) + _clipboard, dest + '/' + newname, true );

    // create the TreeItems:

    QListViewItem* parent = 0;

    if(item){
	if(item->isExpandable()) {
	    parent = item;
	    item = 0;
	}
	else
	    parent = item->parent();
    }

    if(parent)
        parent->setOpen(true);

    TreeItem* newitem;
    if (!parent)
	newitem = new TreeItem(this, item, "");
    else
	newitem = new TreeItem(parent, item, "");

    KDesktopFile df(locateLocal("apps", dest + '/' + newname));

    newitem->setName(findName(&df, false));
    if(!dest.isEmpty())
        newitem->setFile(dest + '/' + newname);
    else
        newitem->setFile(newname);
    newitem->setPixmap(0, appIcon(df.readIcon()));

    fillBranch(newitem->file(), newitem);
    setSelected ( newitem, true);
    itemSelected( newitem);
}

void TreeView::del()
{
    TreeItem *item = (TreeItem*)selectedItem();

    // nil selected? -> nil to delete
    if (item == 0) return;

    QString file = item->file();

    // is file a .directory or a .desktop file
    if(file.endsWith("/.directory"))
    {
        KDesktopFile df(item->file());
        hideDir(file.mid(0, file.find("/.directory")), findName(&df, false), true);
    }
    else if (file.find(".desktop"))
    {
        deleteFile(file);
    }
    else
    {
        return; // Should not happen.
    }

    if (_showHidden)
        item->setHidden(true);
    else
        delete item;

    _ac->action("edit_cut")->setEnabled(false);
    _ac->action("edit_copy")->setEnabled(false);
    _ac->action("delete")->setEnabled(false);

    // Select new current item
    setSelected( currentItem(), true );
    // Switch the UI to show that item
    itemSelected( selectedItem() );
}

void TreeView::undel()
{
    TreeItem *item = (TreeItem*)selectedItem();

    // nil selected? -> nil to delete
    if (!item || !item->isHidden()) return;

    KDesktopFile df(item->file());
    df.writeEntry("Name", item->name());
    df.deleteEntry("Hidden");
    df.deleteEntry("NoDisplay");
    df.sync();

    item->setHidden(false);

    _ac->action("edit_cut")->setEnabled(false);
    _ac->action("edit_copy")->setEnabled(false);
    _ac->action("delete")->setEnabled(false);

    // Select new current item
    setSelected( currentItem(), true );
    // Switch the UI to show that item
    itemSelected( selectedItem() );
}

void TreeView::cleanupClipboard() {
	cleanupClipboard(locateLocal("apps", ".kmenuedit_clipboard"));
}

void TreeView::cleanupClipboard(const QString path)
{
    QDir d(path);
    d.setFilter(QDir::Dirs);
    QStringList dirlist = d.entryList();

    if (!dirlist.isEmpty()) {
    	QStringList::ConstIterator it = dirlist.begin();
	int i = 0;
	do {
		if (i<2) {	// get rid of the "./" and  "../"
			++it;
			i++;
			continue;
		}
		cleanupClipboard(path + "/" + *it);
		i++;
		++it;
	} while (it != dirlist.end());
    }

    d.setFilter(QDir::Files | QDir::Hidden);
    QStringList filelist = d.entryList();
    if (!filelist.isEmpty()) {
 	QStringList::ConstIterator it = filelist.begin();
	QFile f;
	while (it != filelist.end()) {
		f.setName(path + "/" + *it);
		f.remove();
		++it;
	}
    }

    d.rmdir(path);
}
