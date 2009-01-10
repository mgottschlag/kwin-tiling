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

#include "solid-actions.h"
#include "solid-action-item.h"

#include <KGenericFactory>
#include <KStandardDirs>
#include <KDebug>
#include <KAboutData>
#include <KDialog>
#include <KDesktopFile>
#include <KUrl>
#include <kdesktopfileactions.h>
#include <KIO/NetAccess>

#include <QComboBox>
#include <QPushButton>

#include <Solid/DeviceInterface>
#include <Solid/Predicate>

K_PLUGIN_FACTORY(SolidActionsFactory, registerPlugin<SolidActions>();)
K_EXPORT_PLUGIN(SolidActionsFactory( "kcmsolidactions", "kcm_solid_actions" ))

SolidActions::SolidActions( QWidget* parent, const QVariantList& )
        : KCModule( SolidActionsFactory::componentData(), parent )
{
    KAboutData* about = new KAboutData("Device Actions", 0, ki18n("Solid Device Actions Editor"), "1.0",
                                       ki18n("Solid Device Actions Control Panel Module"),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2009 Solid Device Actions team"));
    about->addAuthor(ki18n("Ben Cooksley"), ki18n( "Maintainer" ), "ben@eclipse.endoftheinternet.org");
    setAboutData( about );
    setButtons(KCModule::NoAdditionalButton);

    // Prepare main display dialog
    mainUi = new Ui_SolidActionsConfig();
    mainUi->setupUi(this);
    mainUi->LwActions->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(mainUi->PbEditAction, SIGNAL(clicked()), this, SLOT(editAction()));
    connect(mainUi->PbDeleteAction, SIGNAL(clicked()), this, SLOT(deleteAction()));
    connect(mainUi->PbDefaultAction, SIGNAL(clicked()), this, SLOT(setActionPreferred()));
    connect(mainUi->LwActions, SIGNAL(itemSelectionChanged()), this, SLOT(enableEditDelete()));
    connect(mainUi->CbDeviceType, SIGNAL(currentIndexChanged(int)), this, SLOT(fillActionsList()));

    // Prepare action edit dialog
    editDialog = new KDialog(this);
    editWidget = new QWidget(editDialog);
    editDialog->setMainWidget(editWidget);
    editUi = new Ui_SolidActionEdit();
    editUi->setupUi(editWidget);
    editDialog->setInitialSize(QSize(500, 200));

    editUi->IbActionIcon->setIconType(KIconLoader::NoGroup, KIconLoader::Any, false);

    connect(editDialog, SIGNAL(okClicked()), this, SLOT(acceptActionChanges()));

    // Prepare action add dialog
    addDialog = new KDialog(this);
    addWidget = new QWidget(addDialog);
    addDialog->setMainWidget(addWidget);
    addUi = new Ui_SolidActionAdd();
    addUi->setupUi(addWidget);
    addDialog->setInitialSize(QSize(300, 100));

    connect(mainUi->PbAddAction, SIGNAL(clicked()), addDialog, SLOT(show()));
    connect(addDialog, SIGNAL(okClicked()), this, SLOT(addAction()));
}


SolidActions::~SolidActions()
{
    clearActions();
    delete mainUi;
    delete editUi;
}

void SolidActions::load()
{ 
    QStringList deviceTypes;
    QList<Solid::DeviceInterface::Type> typeList;
    QStringList noDisplayDeviceTypes;

    // Fill the list of non displayed device types
    noDisplayDeviceTypes << "Processor" << "Unknown"; // can't be hotplugged or have actions
    noDisplayDeviceTypes << "AcAdapter" << "Battery"; // Handled by PowerDevil
    // Fill up the list of possible device types
    mainUi->CbDeviceType->clear();
    for( Solid::DeviceInterface::Type t = Solid::DeviceInterface::Unknown; t < Solid::DeviceInterface::Last; t = Solid::DeviceInterface::Type(t+1) )
    { QString deviceType = Solid::DeviceInterface::typeToString(t);
      if( deviceType == "" )
      { break; }
      if(!noDisplayDeviceTypes.contains(deviceType))
      { typeList.append(t); }
    }
    foreach (const Solid::DeviceInterface::Type &currentType, typeList)
    { deviceTypes << Solid::DeviceInterface::typeToString(currentType); }
    mainUi->CbDeviceType->insertItems(0, deviceTypes);
}

void SolidActions::defaults()
{
}

void SolidActions::save()
{
}

void SolidActions::addAction()
{
    QListWidgetItem * newAction = 0;

    QString enteredName = addUi->LeActionName->text();
    // Lets get a desktop file
    QString internalName = enteredName; 
    internalName.replace(" ", "-", Qt::CaseSensitive);
    QString filePath = KStandardDirs::locateLocal("data", 0);
    filePath = filePath + "solid/actions/" + internalName + ".desktop";
    kDebug() << filePath;
    KDesktopFile newDesktop(filePath);
    // Fill in an inital template
    newDesktop.actionGroup("open").writeEntry("Icon", "unknown");
    newDesktop.actionGroup("open").writeEntry("Exec", "");
    newDesktop.actionGroup("open").writeEntry("Name", enteredName);
    newDesktop.desktopGroup().writeEntry("Actions", "open;");
    newDesktop.desktopGroup().writeEntry("Type", "Service");
    newDesktop.desktopGroup().writeEntry("X-KDE-Solid-Predicate","[ IS " + mainUi->CbDeviceType->currentText() +" ]");
    newDesktop.desktopGroup().writeEntry("X-KDE-Solid-Action-Custom", "true");
    // Write the file contents
    newDesktop.sync();
    // Prepare to open the editDialog
    fillActionsList();
    foreach( SolidActionItem * newItem, actionsDb.values() )
    { 
      if( newItem->desktopFilePath == filePath )
      { newAction = actionsDb.key(newItem);
        break;
      }
    }
    mainUi->LwActions->setCurrentItem(newAction);
    editAction();
}

void SolidActions::editAction()
{ 
    SolidActionItem * selectedItem = selectedAction();
    // Set all the text appropriately
    editUi->IbActionIcon->setIcon(selectedItem->iconName);
    editUi->LeActionFriendlyName->setText(selectedItem->name);
    editUi->LeActionCommand->setPath(selectedItem->exec);
    editUi->LeSolidPredicate->setText(selectedItem->predicate);
    editDialog->setCaption("Editing action " +  selectedItem->name);
    // Display us!
    editDialog->show();
}

void SolidActions::deleteAction()
{
    KUrl desktopPath(QString(""));

    QListWidgetItem * item = selectedWidget();
    SolidActionItem * action = selectedAction();
    if(action->isUserSupplied())
    { KIO::NetAccess::del(KUrl(action->desktopFilePath), this); }
    mainUi->LwActions->removeItemWidget(item);
    actionsDb.remove(item);
    KIO::NetAccess::del(KUrl(action->writeDesktopPath), this);
    fillActionsList();
}

void SolidActions::setActionPreferred()
{
    bool preferAction = true;

    SolidActionItem * actionItem = selectedAction();
    if(actionItem->preferred)
    { preferAction = false; }
    foreach(SolidActionItem * deSelect, actionsDb.values())
    { deSelect->setPreferredAction(false); }
    actionItem->setPreferredAction(preferAction);
    fillActionsList();
}

QListWidgetItem * SolidActions::selectedWidget()
{ 
    foreach(QListWidgetItem* candidate, actionsDb.keys() )
    { if( candidate->isSelected() )
      { return candidate;
      }
    }
    return 0;
}

SolidActionItem * SolidActions::selectedAction()
{
    QListWidgetItem *action = selectedWidget();
    // Retrieve the Solid Action Item from the db
    SolidActionItem * actionItem = actionsDb.value(action);
    return actionItem;
}

void SolidActions::fillActionsList()
{
    QStringList allPossibleActions;
    QStringList validActions;
    QString preferredText;

    clearActions();
    // Prepare to search for possible actions
    allPossibleActions = KGlobal::dirs()->findAllResources("data", "solid/actions/");
    // filter down to the only valid actions
    foreach(const QString &possibleAction, allPossibleActions)
    { 
      KDesktopFile actionCfg(possibleAction);
      QString actionPredicate = actionCfg.desktopGroup().readEntry("X-KDE-Solid-Predicate");
    #warning Need to find correct way to filter using Solid::Predicate
      if( actionPredicate.contains(mainUi->CbDeviceType->currentText()) )
      { validActions.append(possibleAction);
      }
    }
    // Get service objects for those actions and add them to the display
    foreach (const QString &desktop, validActions) 
    {
       KUrl desktopFile(desktop);
       QString filePath = KStandardDirs::locate("data", "solid/actions/" + desktopFile.fileName());
       QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(filePath, true);
       foreach(const KServiceAction &deviceAction, services)
       {  
          preferredText=QString();
          SolidActionItem * actionItem = new SolidActionItem(filePath, deviceAction.name(), this);
          if( actionItem->preferred )
          { preferredText=" (Preferred)"; }
          QListWidgetItem * actionListItem = new QListWidgetItem(*actionItem->icon, actionItem->name + preferredText, 0, QListWidgetItem::Type);
          mainUi->LwActions->insertItem(mainUi->LwActions->count(), actionListItem);        
          actionsDb.insert(actionListItem, actionItem);
       }
    }
    toggleEditDelete(false);
}

void SolidActions::acceptActionChanges()
{  
    SolidActionItem *selectedItem = selectedAction();
    // apply the changes 
    if( editUi->IbActionIcon->icon() != selectedItem->iconName )
    { selectedItem->setIconName(editUi->IbActionIcon->icon()); }
    if( editUi->LeActionFriendlyName->text() != selectedItem->name )
    { selectedItem->setName(editUi->LeActionFriendlyName->text()); }
    if( editUi->LeActionCommand->text() != selectedItem->exec )
    { selectedItem->setExec(editUi->LeActionCommand->text()); }
    if( editUi->LeSolidPredicate->text() != selectedItem->predicate )
    { selectedItem->setPredicate(editUi->LeSolidPredicate->text()); }
    // Re-read the actions list to complete changes
    fillActionsList();
}

void SolidActions::toggleEditDelete(bool toggle)
{ 
    mainUi->PbEditAction->setEnabled(toggle);
    mainUi->PbDeleteAction->setEnabled(toggle);
    mainUi->PbDefaultAction->setEnabled(toggle);

    if(mainUi->LwActions->selectedItems().count() == 0)
    { 
      mainUi->PbDeleteAction->setText("No Action Selected");
      mainUi->PbDefaultAction->setText("No Action Selected");
      return;
    }
   
    KUrl writeDesktopFile(selectedAction()->writeDesktopPath); 
    if( selectedAction()->isUserSupplied() )
    { mainUi->PbDeleteAction->setText("Delete Action"); }
    else if( KIO::NetAccess::exists(writeDesktopFile, true, this) )
    { mainUi->PbDeleteAction->setText("Revert Modifications"); }
    else
    { mainUi->PbDeleteAction->setText("Cannot be deleted"); 
      mainUi->PbDeleteAction->setEnabled(false);
    }
    if( selectedAction()->preferred )
    { mainUi->PbDefaultAction->setText("Unprefer action"); }
    else
    { mainUi->PbDefaultAction->setText("Set Preferred Action"); }
}

void SolidActions::enableEditDelete()
{ toggleEditDelete(true);
}

void SolidActions::clearActions()
{ 
    foreach( SolidActionItem * deleteAction, actionsDb.values() )
    { delete deleteAction; }
    mainUi->LwActions->clear();
    actionsDb.clear();
}

#include "solid-actions.moc"
