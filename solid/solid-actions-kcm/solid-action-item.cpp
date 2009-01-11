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
    icon = readKey(actionName, "Icon");
    name = readKey(actionName, "Name");
    exec = readKey(actionName, "Exec");
    predicate = readKey("DesktopEntryGroup", "X-KDE-Solid-Predicate");
}

SolidActionItem::~SolidActionItem()
{ 
    delete writeDesktop;
    delete desktopFile;
}

void SolidActionItem::setIcon(QString nameOfIcon)
{ 
    icon=nameOfIcon;
    desktopWrite()->actionGroup(actionName).writeEntry("Icon", icon, KConfigGroup::Normal);
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
      if(writeDesktop->hasActionGroup(keyGroup) && writeDesktop->actionGroup(actionName).hasKey(keyName))
      { readFile = writeDesktop; }
      else
      { readFile = desktopFile; }
      readGroup = readFile->actionGroup(keyGroup); 
    }
    return readGroup.readEntry(keyName);
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
