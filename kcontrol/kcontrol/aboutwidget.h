/*
  Copyright (c) 2000,2001 Matthias Elter <elter@kde.org>
 
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

#ifndef __aboutwidget_h__
#define __aboutwidget_h__

#include <qwidget.h>
#include <qpixmap.h>
#include <qlistview.h>

class AboutWidget : public QWidget
{  
  Q_OBJECT    
  
public:   
  AboutWidget(QWidget *parent, const char *name=0, QListViewItem* category=0);	
  
protected:
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);

private:
    QPixmap _part1, _part2, _part3;
    QPixmap _buffer;
    bool    _moduleList;
    QListViewItem* _category;
};

#endif
