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

#include "action-item.h"

#include <QString>
#include <QMap>

#include <kdesktopfileactions.h>
#include <KDebug>
#include <KDesktopFile>
#include <KConfigGroup>
    
ActionItem::ActionItem(QString pathToDesktop, QString action, QObject *parent)
{
    Q_UNUSED(parent);

    desktopMasterPath = pathToDesktop;
    actionName = action;
    // Create the desktop file
    desktopFileMaster = new KDesktopFile(desktopMasterPath);
    desktopWritePath = desktopFileMaster->locateLocal(desktopMasterPath);
    desktopFileWrite = new KDesktopFile(desktopWritePath);
    // Now we can fill the action groups list
    actionGroups.insert(ActionItem::GroupDesktop, desktopFileWrite->desktopGroup());
    actionGroups.insert(ActionItem::GroupAction, desktopFileWrite->actionGroup(actionName));
    actionGroups.insert(ActionItem::GroupDesktop, desktopFileMaster->desktopGroup());
    actionGroups.insert(ActionItem::GroupAction, desktopFileMaster->actionGroup(actionName));
}

ActionItem::~ActionItem()
{ 
    delete desktopFileWrite;
    delete desktopFileMaster;
}

/// Public functions below

bool ActionItem::isUserSupplied()
{
    return hasKey(ActionItem::GroupDesktop, "X-KDE-Solid-Action-Custom");
}

QString ActionItem::readKey(GroupType keyGroup, QString keyName)
{
    return configItem(ActionItem::DesktopRead, keyGroup, keyName)->readEntry(keyName);
}

void ActionItem::setKey(GroupType keyGroup, QString keyName, QString keyContents)
{
    configItem(ActionItem::DesktopWrite, keyGroup)->writeEntry(keyName, keyContents);
}

bool ActionItem::hasKey(GroupType keyGroup, QString keyName)
{
    return configItem(ActionItem::DesktopRead, keyGroup, keyName)->hasKey(keyName);
}

QString ActionItem::icon()
{
    return readKey(ActionItem::GroupAction, "Icon");
}

QString ActionItem::exec()
{
    return readKey(ActionItem::GroupAction, "Exec");
}

QString ActionItem::name()
{
    return readKey(ActionItem::GroupAction, "Name");
}

void ActionItem::setIcon(QString nameOfIcon)
{
    setKey(ActionItem::GroupAction, "Icon", nameOfIcon);
}

void ActionItem::setName(QString nameOfAction)
{
    setKey(ActionItem::GroupAction, "Name", nameOfAction);
}

void ActionItem::setExec(QString execUrl)
{
    setKey(ActionItem::GroupAction, "Exec", execUrl);
}

/// Private functions below

KConfigGroup * ActionItem::configItem(DesktopAction actionType, GroupType keyGroup, QString keyName)
{
    int countAccess = 0;
    KConfigGroup * foundGroup = 0;

    if(actionType == ActionItem::DesktopRead)
    {
      foreach( KConfigGroup possibleGroup, actionGroups.values(keyGroup) )
      { if(possibleGroup.isValid()) { kDebug() << "possibleGroup valid"; }
        if( possibleGroup.hasKey(keyName) )
        { foundGroup = &possibleGroup;
          break;
        }
      }
    }
    else if(actionType == ActionItem::DesktopWrite)
    {
      if(isUserSupplied())
      { countAccess = 1; }
      foundGroup = &(actionGroups.values(keyGroup)[countAccess]);
    }
    return foundGroup;
}

#include "action-item.moc"
