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

SolidActionEditPredicate::SolidActionEditPredicate(QWidget *parent) : KDialog(parent)
{
    QWidget *predicateWidget = new QWidget(this);
    setMainWidget(predicateWidget);
    ui.setupUi(predicateWidget);
    setInitialSize(QSize(150, 100)); // Set a decent size
    setCaption(i18n("Editing requirement"));

    connect(ui.CbRequirementType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInterface()));
    connect(ui.CbRestrictionType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInterface()));
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
            ui.LeRestrictionValue->setText(editItem->text(2)); // Import <DeviceType>
        }
        // Second type of requirement "<DeviceType>.<DeviceVariable> <equals/doesn't equal> <compare data>
        else {
            ui.CbRestrictionType->setCurrentIndex(1); // Set the type appropriately
            ui.LeRestrictionValue->setText(editItem->text(1)); // Import <DeviceType>.<DeviceVariable>
            ui.LeRestrictionRequirement->setText(editItem->text(3)); // Import <compare data>
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
    updateInterface();
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
        updating->setText(2, ui.LeRestrictionValue->text()); // and import the device type
    } else { // otherwise the container sub-type must be device.value <logic> match
        updating->setText(1, ui.LeRestrictionValue->text()); // Read the device.value information
        updating->setText(3, ui.LeRestrictionRequirement->text()); // Read the match information
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
    ui.FrTypeContainer->setShown(currentRequireType);
    ui.FrTypeRestriction->setShown(!currentRequireType);
    bool currentRestrictType = ui.CbRestrictionType->currentIndex(); // Get the current condition sub-type
    ui.CbRestrictionRequirement->setShown(currentRestrictType);
    ui.LeRestrictionRequirement->setShown(currentRestrictType);
}

#include "solid-action-edit-predicate.moc"
