/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 
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

#ifndef __indexwidget_h__
#define __indexwidget_h__

#include <qwidget.h>
#include <qlistview.h>

class ConfigModuleList;
class ConfigModule;
class QPushButton;

class IndexWidget : public QWidget
{  
  Q_OBJECT    
  
public:   
  IndexWidget(QWidget *parent, const char *name=0);	

  void fillIndex(ConfigModuleList *list);

public slots:
  void moduleChanged(ConfigModule *module);
  void makeVisible(ConfigModule *module);

protected slots:
  void currentChanged(QListViewItem *item);
  void treeButtonClicked();
  void iconButtonClicked();

signals:
  void moduleActivated(ConfigModule *module);

protected:
  void updateItem(QListViewItem *item, ConfigModule *module);
  void resizeEvent(QResizeEvent *);

private:
  QListViewItem *getGroupItem(QListViewItem *parent, const QStringList &groups);

  QListView     *_tree;
  QListViewItem *_root;
  QPushButton   *_treebtn, *_iconbtn;

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
  friend class IndexWidget;

public:
  IndexListItem(QListViewItem *parent, ConfigModule *module);
  IndexListItem(QListView *parent, ConfigModule *module);

  ConfigModule *getModule() { return _module; };


protected:
  ConfigModule *_module;

};

#endif
