/*

  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
*/                                                                            



#ifndef __INDEX_H__
#define __INDEX_H__


#include <qlistview.h>
#include <qwidget.h>
#include <qobject.h>
#include <qstringlist.h>

#include "modules.h"


class IndexPane : public QWidget
{
  Q_OBJECT

public:

  IndexPane(QWidget *parent=0, const char *name=0);

  void fillIndex(ConfigModuleList &list);


public slots:

  void moduleChanged(ConfigModule *module);
  void makeVisible(ConfigModule *module);
 

protected:

  void resizeEvent(QResizeEvent *event);
  void updateItem(QListViewItem *item, ConfigModule *module);


protected slots:
    
  void doubleClicked(QListViewItem *item);
 

signals:

  void moduleDoubleClicked(ConfigModule *module);
 

private:

  QListViewItem *getGroupItem(QListViewItem *parent, const QStringList &groups);

  QListView *_tree;
  QListViewItem *localMachine, *localUser;

};


class IndexItem : public QListViewItem
{
public:

  IndexItem(QListViewItem *parent)
    : QListViewItem(parent) {};
  IndexItem(QListView *parent)
    : QListViewItem(parent) {};

  void setTag(QString tag) { _tag = tag; };
  QString tag() const { return _tag; };


private:
  
  QString _tag;

};


class IndexListItem : public IndexItem
{
  friend class IndexPane;

public:

  IndexListItem(QListViewItem *parent, ConfigModule *module);
  IndexListItem(QListView *parent, ConfigModule *module);

  ConfigModule *getModule() { return _module; };


protected:

  void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);

  ConfigModule *_module;

};


#endif
