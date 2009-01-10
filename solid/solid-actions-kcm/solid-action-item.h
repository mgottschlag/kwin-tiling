/***************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <ben@eclipse.endoftheinternet.org> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#ifndef _SOLID_ACTION_ITEM_H_
#define _SOLID_ACTION_ITEM_H_

#include <QObject>

class QString;
class KIcon;
class KDesktopFile;
class KServiceAction;
class KUrl;
class KConfigGroup;

class SolidActionItem: public QObject
{
     Q_OBJECT

public:
     SolidActionItem(QString pathToDesktop, QString action, QObject *parent = 0);
     ~SolidActionItem();
     void setIconName(QString nameOfIcon);
     void setName(QString nameOfAction);
     void setExec(QString execUrl);
     void setPredicate(QString textOfPredicate);
     bool isUserSupplied();
     QString readKey(QString keyGroup, QString keyName);
     KDesktopFile * desktopWrite();

     QString desktopFilePath;
     QString writeDesktopPath;
     QString iconName;
     KIcon * icon;
     QString exec;
     QString name;
     QString predicate;
     QString actionName;

private:
     KDesktopFile * desktopFile;
     KDesktopFile * writeDesktop;

};

#endif
