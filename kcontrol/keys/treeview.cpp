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
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <unistd.h>

#include <qapplication.h>
#include <qdir.h>
#include <qimage.h>
#include <qstringlist.h>
#include <qcursor.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3Frame>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kservicegroup.h>

#include "treeview.h"
#include "treeview.moc"
#include "khotkeys.h"

AppTreeItem::AppTreeItem(Q3ListViewItem *parent, const QString& storageId)
    : KListViewItem(parent), m_init(false), m_storageId(storageId) {}

AppTreeItem::AppTreeItem(Q3ListViewItem *parent, Q3ListViewItem *after, const QString& storageId)
    : KListViewItem(parent, after), m_init(false), m_storageId(storageId) {}

AppTreeItem::AppTreeItem(Q3ListView *parent, const QString& storageId)
    : KListViewItem(parent), m_init(false), m_storageId(storageId) {}

AppTreeItem::AppTreeItem(Q3ListView *parent, Q3ListViewItem *after, const QString& storageId)
    : KListViewItem(parent, after), m_init(false), m_storageId(storageId) {}

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
    Q3ListViewItem::setOpen(o);
}

static QPixmap appIcon(const QString &iconName)
{
    QPixmap normal = SmallIcon( iconName );
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
    setFrameStyle(Q3Frame::WinPanel | Q3Frame::Sunken);
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

    connect(this, SIGNAL(clicked( Q3ListViewItem* )),
            SLOT(itemSelected( Q3ListViewItem* )));

    connect(this,SIGNAL(selectionChanged ( Q3ListViewItem * )),
            SLOT(itemSelected( Q3ListViewItem* )));
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

    Q3ListViewItem *after = 0;

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
                item = new AppTreeItem(this,  after, QString::null);
            else
                item = new AppTreeItem(parent, after, QString::null);

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
                item = new AppTreeItem(this, after, s->storageId());
            else
                item = new AppTreeItem(parent, after, s->storageId());

            item->setName(serviceCaption);
            item->setAccel(KHotKeys::getMenuEntryShortcut(s->storageId()));
            item->setPixmap(0, appIcon(s->icon()));

            after = item;
        }
    }
}

void AppTreeView::itemSelected(Q3ListViewItem *item)
{
    AppTreeItem *_item = static_cast<AppTreeItem*>(item);

    if(!item) return;

    emit entrySelected(_item->storageId(), _item->accel(), _item->isDirectory());
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
