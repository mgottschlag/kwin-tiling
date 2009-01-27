/***************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <ben@eclipse.endoftheinternet.org> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#include "solid-action-edit-predicate.h"

#include <QTreeWidgetItem>

#include <KStandardDirs>
#include <KDesktopFile>
#include <KConfigGroup>
#include <kserviceaction.h>
#include <kdesktopfileactions.h>
#include <KDebug>

SolidActionEditPredicate::SolidActionEditPredicate(QWidget *parent) : KDialog(parent)
{
    QStringList allPossibleDevices;

    QWidget *predicateWidget = new QWidget(this);
    setMainWidget(predicateWidget);
    ui.setupUi(predicateWidget);
    setInitialSize(QSize(150, 100)); // Set a decent size
    setCaption(i18n("Editing requirement"));

    // Fill the lists of possible device types / device values
    allPossibleDevices = KGlobal::dirs()->findAllResources("data", "solid/devices/");
    // Get service objects for those actions and add them to the display
    foreach(const QString &desktop, allPossibleDevices) {
        KDesktopFile deviceFile(desktop);
        KConfigGroup deviceType = deviceFile.desktopGroup(); // Retrieve the configuration group where the user friendly name is
        deviceTypes.insert(deviceType.readEntry("X-KDE-Solid-Actions-Type"), deviceType.readEntry("Name")); // Lets read the user friendly name
        QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(desktop, true); // Get the list of contained services
        foreach(const KServiceAction &deviceValue, services) { // We want every single action
            deviceValuesTypeMap.insert(deviceType.readEntry("X-KDE-Solid-Actions-Type"), deviceValue.name()); // Add to the type - actions map
            QString typeName = deviceType.readEntry("X-KDE-Solid-Actions-Type") + QChar('.') + deviceValue.name(); // prepare the full internal name
            deviceValues.insert(typeName, deviceValue.text()); // Add to the full internal name - user friendly name map
        }
    }
    ui.CbRestrictionDeviceType->addItems(deviceTypes.values()); // Add the device types to the list

    connect(ui.CbRequirementType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInterface()));
    connect(ui.CbRestrictionType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInterface()));
    connect(ui.CbRestrictionDeviceType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateValuesList()));
}

SolidActionEditPredicate::~SolidActionEditPredicate()
{
}

void SolidActionEditPredicate::prepareShow(QTreeWidgetItem *editItem)
{
    int currentIndexValue = 0;

    // We need to set the type first
    if (editItem->text(5) == "0") { // The type is either "0" ->  Container
        ui.CbRequirementType->setCurrentIndex(0);
    } else if (editItem->text(5) == "1") { // Or "1" -> Requirement
        ui.CbRequirementType->setCurrentIndex(1);
    }
    // If the type is "Container" then we need to import the settings
    if (ui.CbRequirementType->currentIndex() == 0) { // The type can be either all must match or any must match - check
        if (editItem->text(4) == "AND") {
            currentIndexValue = 0;
        } else if (editItem->text(4) == "OR") {
            currentIndexValue = 1;
        }
        ui.CbContainerLogic->setCurrentIndex(currentIndexValue); // Set the type - whatever was set above
    }
    // Otherwise the type is Requirement, so import the settings for it
    else if (ui.CbRequirementType->currentIndex() == 1) { // There are two types of requirement, they require different import methods: "IS <DeviceType>"
        if (editItem->text(1) == "IS") {
            ui.CbRestrictionType->setCurrentIndex(0); // Set the type appropriately
            ui.CbRestrictionDeviceType->setCurrentIndex(deviceTypes.keys().indexOf(editItem->text(2))); // Import <DeviceType>
        }
        // Second type of requirement "<DeviceType>.<DeviceVariable> <equals/doesn't equal> <compare data>
        else {
            ui.CbRestrictionType->setCurrentIndex(1); // Set the type appropriately
            ui.LeRestrictionRequirement->setText(editItem->text(3)); // Set the requirement value match
            QStringList deviceRequirement = editItem->text(1).split(QChar('.')); // Split the value from device type
            ui.CbRestrictionDeviceType->setCurrentIndex(deviceTypes.keys().indexOf(deviceRequirement.at(0))); // Import <DeviceType>
            updateValuesList(); // The action list should be refreshed now so it is ready for setting
            QStringList deviceReqList = deviceValuesTypeMap.values(deviceRequirement.at(0));
            ui.CbRestrictionDeviceValue->setCurrentIndex(deviceReqList.indexOf(deviceRequirement.at(1))); // Import <Value>
            // If the comparer is "equals" then we should set it as so
            if (editItem->text(2) == "==") {
                ui.CbRestrictionRequirement->setCurrentIndex(0);
            }
            // Otherwise is must be "doesn't equal"
            else {
                ui.CbRestrictionRequirement->setCurrentIndex(1);
            }
        }
    }
    // Check if it is a new item -> if so allow the type to be selected
    if (editItem->text(6) == "NEW-ITEM") {
        ui.FrRequirementType->show();
    } else {
        ui.FrRequirementType->hide();
    }
    updateInterface(); // Make sure the user can only see what they should be able to
}

void SolidActionEditPredicate::finishShow(QTreeWidgetItem *updating)
{
    updating->setText(5, QString::number(ui.CbRequirementType->currentIndex())); // Set the type appropriately
    updating->setText(6, ""); // It is no longer a new item
    if (ui.CbContainerLogic->currentIndex() == 0) {
        updating->setText(4, "AND"); // the container is of the all must match type
    } else if (ui.CbContainerLogic->currentIndex() == 1) {
        updating->setText(4, "OR"); // the container is of the any must match type
    }
    if (ui.CbRestrictionType->currentIndex() == 0) { // The condition sub-type could be "Device is of type"
        updating->setText(1, "IS"); // Set it to be so
        updating->setText(2, deviceTypes.key(ui.CbRestrictionDeviceType->currentText())); // and import the device type
    } else { // otherwise the container sub-type must be device.value <logic> match
        QString deviceInternalName = deviceTypes.key(ui.CbRestrictionDeviceType->currentText());
        foreach(const QString &deviceName, deviceValuesTypeMap.values(deviceInternalName)) {
            QString fullInternalName = deviceInternalName + QChar('.') + deviceName; // Prepare the full internal name
            if (deviceValues.value(fullInternalName) == ui.CbRestrictionDeviceValue->currentText()) { // Does the value name match the selected
                updating->setText(1, fullInternalName);
            } // Set the value name
        }
        updating->setText(3, ui.LeRestrictionRequirement->text());  // Read the match information
        if (ui.CbRestrictionRequirement->currentIndex() == 0) { // Is the <logic> equals
            updating->setText(2, "==");
        } else if (ui.CbRestrictionRequirement->currentIndex() == 1) { // Or is it not equals
            updating->setText(2, "!=");
        }
    }
}

void SolidActionEditPredicate::updateInterface()
{
    bool currentRequireType;
    if (ui.CbRequirementType->currentIndex() == 0) { // The requirement type should be a container
        currentRequireType = true;
    } else {
        currentRequireType = false;
    }
    ui.FrTypeContainer->setShown(currentRequireType); // We show this one if its a container
    ui.FrTypeRestriction->setShown(!currentRequireType); // Otherwise we show this one, for restriction
    bool currentRestrictType = ui.CbRestrictionType->currentIndex(); // Get the current condition sub-type
    ui.CbRestrictionRequirement->setShown(currentRestrictType); // These are only shown if the condition sub-type is value = text
    ui.LeRestrictionRequirement->setShown(currentRestrictType); // Ditto
    ui.LblRestrictionDeviceValue->setShown(currentRestrictType); // Ditto
    ui.CbRestrictionDeviceValue->setShown(currentRestrictType); // Ditto
}

void SolidActionEditPredicate::updateValuesList()
{   // We need to refill the list of available values
    QStringList deviceValuesList;
    ui.CbRestrictionDeviceValue->clear(); // No need to keep previous values
    QString deviceInternalName = deviceTypes.key(ui.CbRestrictionDeviceType->currentText()); // Retrieve the device internal name
    foreach( const QString &deviceName, deviceValuesTypeMap.values(deviceInternalName)) { // Get all values for this device name
        QString fullInternalName = deviceInternalName + QChar('.') + deviceName; // Create the full internal name
        deviceValuesList << deviceValues.value(fullInternalName); // Add the user friendly name to the list of values
    }
    qSort(deviceValuesList.begin(), deviceValuesList.end()); // Sort it so the correct one is selected later
    ui.CbRestrictionDeviceValue->addItems(deviceValuesList); // Add the items to the combobox
}

#include "solid-action-edit-predicate.moc"
