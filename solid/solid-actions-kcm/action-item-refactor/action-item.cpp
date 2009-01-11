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
    desktopWritePath = KDesktopFile::locateLocal(desktopMasterPath);
    actionName = action;
    // Create the desktop file
    desktopFileMaster = new KDesktopFile(desktopMasterPath);
    desktopFileWrite = new KDesktopFile(desktopWritePath);
    // We need to copy the config groups in first
    KConfigGroup writeDesktopGroup = desktopFileWrite->desktopGroup();
    KConfigGroup writeActionGroup = desktopFileWrite->actionGroup(actionName);
    KConfigGroup masterDesktopGroup = desktopFileMaster->desktopGroup();
    KConfigGroup masterActionGroup = desktopFileMaster->actionGroup(actionName);
    // Now we can fill the action groups list
    actionGroups.insert(ActionItem::GroupDesktop, &writeDesktopGroup);
    actionGroups.insert(ActionItem::GroupAction, &writeActionGroup);
    actionGroups.insert(ActionItem::GroupDesktop, &masterDesktopGroup);
    actionGroups.insert(ActionItem::GroupAction, &masterActionGroup);
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
      foreach( KConfigGroup * possibleGroup, actionGroups.values(keyGroup) )
      { if( possibleGroup ) { kDebug() << "possibleGroup is valid"; }
        if( possibleGroup->hasKey(keyName) )
        { foundGroup = possibleGroup;
          break;
        }
      }
      return foundGroup;
    }
    else if(actionType == ActionItem::DesktopWrite)
    {
      if(isUserSupplied())
      { countAccess = 1; }
      return actionGroups.values(keyGroup).at(countAccess);
    }
    return 0;
}

#include "action-item.moc"
