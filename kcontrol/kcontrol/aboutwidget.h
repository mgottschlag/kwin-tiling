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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 
*/                                                                            

#ifndef __aboutwidget_h__
#define __aboutwidget_h__

#include <qwidget.h>
#include <qlistview.h>
#include <qhbox.h>

class KCModuleInfo;
class QPixmap;
class KPixmap;
class ConfigModule;
class KHTMLPart;
class KURL;

class AboutWidget : public QHBox
{  
  Q_OBJECT    
  
public:   
  AboutWidget(QWidget *parent, const char *name=0, QListViewItem* category=0, const QString &caption=QString::null);

    /**
     * Set a new category without creating a new AboutWidget if there is
     * one visible already (reduces flicker)
     */
    void setCategory( QListViewItem* category, const QString& caption);

signals:
    void moduleSelected(ConfigModule *);

private slots:
    void slotModuleLinkClicked( const KURL& );

private:
    /**
     * Update the pixmap to be shown. Called from resizeEvent and from
     * setCategory.
     */
    void updatePixmap();

    bool    _moduleList;
    QListViewItem* _category;
    QString _caption;
    KHTMLPart *_viewer;
    QMap<QString,ConfigModule*> _moduleMap;
};

#endif
