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

class TreeItem : public QListViewItem
{

public:
    TreeItem(QListViewItem *parent, const QString& file);
    TreeItem(QListViewItem *parent, QListViewItem *after, const QString& file);
    TreeItem(QListView *parent, const QString& file);
    TreeItem(QListView *parent, QListViewItem* after, const QString& file);

    QString file() const { return _file; };
    void setFile(const QString& file) { _file = file; }

private:
    QString _file;
};

class TreeView : public KListView
{
    Q_OBJECT

public:
    TreeView(KActionCollection *ac,QWidget *parent=0, const char *name=0);
    ~TreeView();

public slots:
    void currentChanged();

signals:
    void entrySelected(const QString&);

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
    void hide();
    void unhide();

protected:
    void fill();
    void fillBranch(const QString& relPath, TreeItem* parent);

    // moving = src will be removed later
    void copy( bool moving );

    void copyFile(const QString& src, const QString& dest, bool moving );
    void copyDir(const QString& src, const QString& dest, bool moving );

    int deleteFile(const QString& deskfile, const bool move = false);
    bool deleteDir(const QString& dir, const bool move = false);
    void hideFile(const QString& deskfile, bool hide);
    void hideDir(const QString& d, const QString name, bool hide, QString icon);
    
    void dohide(bool _hide);


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
};

inline void TreeView::hide() {
	dohide(true);
}

inline void TreeView::unhide() {
	dohide(false);
}

#endif
