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



#ifndef __DOCKWIDGET_H__
#define __DOCKWIDGET_H__


#include <qwidget.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qtabwidget.h>


class QPushButton;
class KDockWidget;
class KToolBoxManager;


class KDockContainer : public QTabWidget
{
  Q_OBJECT

  friend class KDockWidget;

public:

  KDockContainer(QWidget *parent=0, const char *name=0);

  void addWidget(QWidget *widget, QPixmap icon);
  void showWidget(QWidget *widget);


protected:

  void addDockWidget(KDockWidget *widget, QPixmap icon);
  void removeDockWidget(KDockWidget *widget);

  void dragChild(KDockWidget *child);


signals:

  void newModule(const QString &name);


public slots:

  void onHotSpot(int index);


private:

  KToolBoxManager    *_tbManager;
  bool               _dockSpot;

};


class KDockWidget : public QWidget
{
  Q_OBJECT

public:

  KDockWidget(QString caption, QPixmap icon, KDockContainer *parent=0, const char *name=0);
  ~KDockWidget();

  void undock();
  void dock();
  bool isDocked() { return _docked; };


protected:

  virtual QRect clientRect();
  static const int DOCKBAR_HEIGHT;

  void resizeEvent(QResizeEvent *event);
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent *event);


signals:

  void closeClicked();


private slots:

  void slotCloseClicked();
  

private:

  KDockContainer *_container;
  QPushButton    *_closeButton;
  bool           _docked;
  QPixmap        _icon;
};


#endif
