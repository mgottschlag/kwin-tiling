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

#include "solid-action-item.h"

#include <QString>
#include <QList>

#include <kserviceaction.h>
#include <kdesktopfileactions.h>
#include <KDebug>
#include <KDesktopFile>
#include <KConfigGroup>
#include <KIcon>
    
SolidActionItem::SolidActionItem(QString pathToDesktop, QString action, QObject *parent)
{
    Q_UNUSED(parent);

    desktopFilePath = pathToDesktop;
    actionName = action;
    // Create the desktop file
    desktopFile = new KDesktopFile(desktopFilePath);
    writeDesktopPath = desktopFile->locateLocal(desktopFilePath);
    writeDesktop = new KDesktopFile(writeDesktopPath);
    // Set other variables
    iconName = readKey(actionName, "Icon");
    icon = new KIcon(iconName);
    name = readKey(actionName, "Name");
    exec = readKey(actionName, "Exec");
    predicate = readKey("DesktopEntryGroup", "X-KDE-Solid-Predicate");
    preferred = desktopWrite()->desktopGroup().hasKey("X-KDE-Solid-Action-Prefer");
}

SolidActionItem::~SolidActionItem()
{ 
    delete icon;
    delete writeDesktop;
    delete desktopFile;
}

void SolidActionItem::setIconName(QString nameOfIcon)
{ 
    iconName=nameOfIcon;
    desktopWrite()->actionGroup(actionName).writeEntry("Icon", iconName, KConfigGroup::Normal);
    delete icon;
    icon = new KIcon(nameOfIcon);
}

void SolidActionItem::setName(QString nameOfAction)
{ 
    name = nameOfAction;
    desktopWrite()->actionGroup(actionName).writeEntry("Name", name, KConfigGroup::Normal);
}

void SolidActionItem::setExec(QString execUrl)
{ 
    exec = execUrl;
    desktopWrite()->actionGroup(actionName).writeEntry("Exec", exec, KConfigGroup::Normal);
}

void SolidActionItem::setPredicate(QString textOfPredicate)
{ 
    predicate=textOfPredicate;
    desktopWrite()->desktopGroup().writeEntry("X-KDE-Solid-Predicate", predicate, KConfigGroup::Normal);
}

bool SolidActionItem::isUserSupplied()
{ 
    return desktopFile->desktopGroup().hasKey("X-KDE-Solid-Action-Custom");
}

QString SolidActionItem::readKey(QString keyGroup, QString keyName)
{
    KDesktopFile * readFile;
    KConfigGroup readGroup;
    // Is it a desktop entry?
    if( keyGroup == "DesktopEntryGroup" )
    { if( writeDesktop->desktopGroup().hasKey(keyName) )
      { readFile = writeDesktop; }
      else
      { readFile = desktopFile; }
      readGroup = readFile->desktopGroup();
    }
    else
    { 
      if(writeDesktop->hasActionGroup(keyGroup) && writeDesktop->actionGroup(keyGroup).hasKey(keyName))
      { readFile = writeDesktop; }
      else
      { readFile = desktopFile; }
      readGroup = readFile->actionGroup(keyGroup); 
    }
    return readGroup.readEntry(keyName);
}

void SolidActionItem::setPreferredAction(bool preferred)
{
    if(preferred)
    { desktopWrite()->desktopGroup().writeEntry("X-KDE-Solid-Action-Prefer", actionName, KConfigGroup::Normal); }
    else
    { desktopWrite()->desktopGroup().deleteEntry("X-KDE-Solid-Action-Prefer", KConfigGroup::Normal); }
}

KDesktopFile * SolidActionItem::desktopWrite()
{
    if( isUserSupplied() )
    { return desktopFile; }
    else
    { return writeDesktop; }
    return 0;
}

#include "solid-action-item.moc"
