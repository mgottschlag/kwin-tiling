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
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __treeview_h__
#define __treeview_h__

#include <qstring.h>
#include <klistview.h>

class QPopupMenu;
class KActionCollection;
class KDesktopFile;

class TreeItem : public QListViewItem
{

public:
    TreeItem(QListViewItem *parent, const QString& file);
    TreeItem(QListViewItem *parent, QListViewItem *after, const QString& file);
    TreeItem(QListView *parent, const QString& file);
    TreeItem(QListView *parent, QListViewItem* after, const QString& file);

    QString file() const;
    void setFile(const QString& file) { _file = file; }

    QString name() const { return _name; }
    void setName(const QString &name);
    
    bool isDirectory() const { return _directory; }
    void setDirectory(bool b);
    
    bool isHidden() const { return _hidden; }
    void setHidden(bool b);

    virtual void setOpen(bool o);

private:
    void update();

    QString _file;
    QString _name;
    bool _hidden : 1;
    bool _init : 1;
    bool _directory : 1;
};

class TreeView : public KListView
{
    friend class TreeItem;
    Q_OBJECT
public:
    TreeView(KActionCollection *ac, QWidget *parent=0, const char *name=0);
    ~TreeView();

    void setViewMode(bool showHidden);

public slots:
    void currentChanged();

signals:
    void entrySelected(const QString&, const QString &, bool);

protected slots:
    void itemSelected(QListViewItem *);
    void slotDropped(QDropEvent *, QListViewItem *, QListViewItem *);
    void slotRMBPressed(QListViewItem*, const QPoint&);

    void newsubmenu();
    void newitem();

    void cut();
    void copy();
    void paste();
    void del();
    void undel();

protected:
    void fill();
    void fillBranch(const QString& relPath, TreeItem* parent);
    QString findName(KDesktopFile *df, bool deleted);

    // moving = src will be removed later
    void copy( bool moving );

    void copyFile(const QString& src, const QString& dest, bool moving );
    void copyDir(const QString& src, const QString& dest, bool moving );

    bool deleteFile(const QString& deskfile, const bool move = false);
    bool deleteDir(const QString& dir, const bool move = false);
    void hideDir(const QString& d, const QString name, bool hide, QString icon);

    void cleanupClipboard();
    void cleanupClipboard(const QString path);

    QStringList fileList(const QString& relativePath);
    QStringList dirList(const QString& relativePath);

    bool acceptDrag(QDropEvent* event) const;
    QDragObject *dragObject();

private:
    KActionCollection *_ac;
    QPopupMenu        *_rmb;
    QString            _clipboard;
    bool               _showHidden;
};


#endif
