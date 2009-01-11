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

#ifndef _ACTION_ITEM_H_
#define _ACTION_ITEM_H_

#include <QObject>
#include <QMap>

class QString;

class KDesktopFile;
class KConfigGroup;

class ActionItem: public QObject
{
     Q_OBJECT

public:
     ActionItem(QString pathToDesktop, QString action, QObject *parent = 0);
     ~ActionItem();

     enum GroupType { GroupDesktop = 0, GroupAction = 1 };

     bool isUserSupplied();
     QString readKey(GroupType keyGroup, QString keyName);
     void setKey(GroupType keyGroup, QString keyName, QString keyContents);
     bool hasKey(GroupType keyGroup, QString keyName);

     QString icon();
     QString exec();
     QString name();
     void setIcon(QString nameOfIcon);
     void setName(QString nameOfAction);
     void setExec(QString execUrl);

     QString desktopMasterPath;
     QString desktopWritePath;
     QString actionName;

private:
     enum DesktopAction { DesktopRead = 0, DesktopWrite = 1 };

     KConfigGroup * configItem(DesktopAction actionType, GroupType keyGroup, QString keyName = QString()); 

     KDesktopFile * desktopFileMaster;
     KDesktopFile * desktopFileWrite;
     QMap<GroupType, KConfigGroup*> actionGroups;

};

#endif
