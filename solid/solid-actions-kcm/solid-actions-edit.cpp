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

#include "solid-actions-edit.h"

#include <KLocale>
#include <KMessageBox>
#include <KDebug>

// Terms used in comments

// Condition = these are matched aganist a device
// Container = holds conditions, matching them up
// Requirement = either a condition or a container

// How the tree columns documentation
// 0 = user friendly i18n'ed text - NO others should be i18n'ed -> internal solid predicate data
// 1 = either "IS" or a device.value text
// 2 = either is device ( in the case of "IS" above ) or "==" / "!=" ( in the case of device.value )
// 3 = match text in case of device.value; not used for "IS" type
// 4 = used by containers to match conditions and sub-containers; either "AND" / "OR"
// 5 = The type of requirement; 0 for container, 1 for condition
// 6 = specifies if the requirement is new; text is blank for not; "NEW-ITEM" for when it is new

SolidActionsEdit::SolidActionsEdit(QWidget *parent) : KDialog(parent)
{
    // Set up the interface
    QWidget *editWidget = new QWidget(this);
    setMainWidget(editWidget);
    ui.setupUi(editWidget);
    setInitialSize(QSize(250, 400)); // Set a decent initial size
    ui.TwSolidRequirements->setHeaderLabel(""); // We don't need a header label

    // Instantiate the predicate edit dialog
    predicateDialog = new KDialog(this);
    QWidget *predicateWidget = new QWidget(predicateDialog);
    predicateDialog->setMainWidget(predicateWidget);
    predicateUi.setupUi(predicateWidget);
    predicateDialog->setInitialSize(QSize(150, 100)); // Set a decent size
   
    // Connect up with everything needed -> slot names explain
    connect(ui.TwSolidRequirements, SIGNAL(itemSelectionChanged()), this, SLOT(updateButtonUsage()));
    connect(ui.PbAddRequirement, SIGNAL(clicked()), this, SLOT(addRequirement()));
    connect(ui.PbEditRequirement, SIGNAL(clicked()), this, SLOT(editRequirement()));
    connect(ui.PbRemoveRequirement, SIGNAL(clicked()), this, SLOT(deleteRequirement()));
    connect(predicateDialog, SIGNAL(okClicked()), this, SLOT(updateRequirement()));
    connect(predicateDialog, SIGNAL(cancelClicked()), this, SLOT(cancelEditRequirement()));
    connect(predicateUi.CbRequirementType, SIGNAL(currentIndexChanged(int)), this, SLOT(editPredicateTypeToggle()));
    connect(predicateUi.CbRestrictionType, SIGNAL(currentIndexChanged(int)), this, SLOT(editPredicateTypeToggle()));
    updateButtonUsage();
}

SolidActionsEdit::~SolidActionsEdit()
{
}

void SolidActionsEdit::fillPredicateTree(QString predicateText)
{
    ui.TwSolidRequirements->setColumnCount(1); // We only want to show the user friendly column
    ui.TwSolidRequirements->clear(); // We shouldn't show previously built results
    QTreeWidgetItem *masterRoot = new QTreeWidgetItem(ui.TwSolidRequirements->invisibleRootItem()); // Get the root
    predicateContainer(predicateText, masterRoot); // Import the data
}

void SolidActionsEdit::predicateContainer(QString predicate, QTreeWidgetItem *parent)
{
    int leftBracket;
    int rightBracket;
    int bracketBegin;
    int bracketEnd;
    QString parsedString;

    // Remove any "[" or "]" characters at beginning or end
    QString unParsed = predicate;
    if (unParsed[0] == '[') {
        unParsed.remove(0, 1);
    }
    if (unParsed.at(unParsed.count() - 1) == ']') {
        unParsed.chop(1);
    }
    // Check to see if we have a container or a condition
    while (unParsed.count("[") != 0)  // We have a container
    { // Clear the variables for reuse
        leftBracket = 0;
        rightBracket = 0;
        bracketEnd = 0;
        bracketBegin = 0;
        for (int textChecked = 0; leftBracket != rightBracket || leftBracket == 0; textChecked = textChecked + 1) { // Continue counting across the predicate until we have a count of right + left brackets that matches and != 0
            // Must match otherwise we will break conditions apart
            if (unParsed[textChecked] == '[') {
                leftBracket = leftBracket + 1;
            }
            if (bracketBegin == 0 && unParsed.at(textChecked) == '[') { // Have we found our first [? we grab from here
                bracketBegin = textChecked;
            }
            if (unParsed[textChecked] == ']') {
                rightBracket = rightBracket + 1;
            }
            bracketEnd = textChecked + 1; // Export the last value checked
        }
        parsedString = unParsed.mid(bracketBegin, bracketEnd - bracketBegin); // Grab the text between the two brackets
        unParsed.remove(bracketBegin, bracketEnd - bracketBegin);
        if (parsedString.count("[") >= 1) { // If the grabbed text only has 1 "[" then it is a condition
            predicateContainer(parsedString, parent);
        } else { // Otherwise it contains more containers
            QTreeWidgetItem *newParent = new QTreeWidgetItem(parent);
            newParent->setText(5, "0"); // Set the type to be container ( 0 = container, 1 = condition )
            predicateContainer(parsedString, newParent);
        }
    }
    unParsed = unParsed.trimmed(); // Remove any lingering whitespace
    // We need to grab a condition if left behind
    if (unParsed == "OR") { // Can either be any inside the container
        parent->setText(4, unParsed); // Save it
        parent->setText(0, i18n("Any of the conditions below this must match")); // Set the friendly text
        unParsed.clear(); // Clear it so we don't try to process it later as a condition
    }
    if (unParsed == "AND") { // Or all inside the container ( note: only friendly text changed from above )
        parent->setText(4, unParsed);
        parent->setText(0, i18n("All of the conditions below this must match"));
        unParsed.clear();
    }
    // If the unparsed text isn't empty and has no brackets then we must have a condition so import it
    if (unParsed.count("[") == 0 && !unParsed.isEmpty()) {
        predicateItem(unParsed, parent);
    }
}

void SolidActionsEdit::predicateItem(QString predicate, QTreeWidgetItem *parent)
{
    QTreeWidgetItem * newItem = new QTreeWidgetItem(parent);
    QTreeWidgetItem * writeItem;
    QString unParsed = predicate;
    QStringList typeList;
    // The predicate could have multiple types in it separate them if they exist
    if (unParsed.contains("OR")) { // Could be any contained be true
        unParsed.replace("OR", "$"); // Replace with unified symbol for easier splitting later
        newItem->setText(4, "OR"); // Store the type away in the container for later use
        newItem->setText(0, i18n("Any of the conditions below this must match")); // Set friendly text
    }
    if (unParsed.contains("AND")) { // Or all contained must be true
        unParsed.replace("AND", "$"); // Replace with unified symbol for easier splitting later
        newItem->setText(0, i18n("All of the conditions below this must match")); // Set friendly text
        newItem->setText(4, "AND"); // Store the type away in the container for later use
    }
    // If it contains the unified symbol then we need to split it
    if (unParsed.contains("$")) {
        while (unParsed.count("$") > 0) {
            QString typeContent = unParsed.mid(unParsed.lastIndexOf("$") - 1); // Read all text from the end to the first split
            unParsed.remove(typeContent); // Remove that text
            typeList.prepend(typeContent.remove('$')); // Add it on to the beginning so that the reversal is reversed
        }
        if (!unParsed.isEmpty()) {
            typeList.prepend(unParsed); // Add any remaining condition to the beginning as well ( covers last condition )
        }
    }
    // If the predicate doesn't have a splitter then just append the entire string
    else {
        typeList << unParsed;
    }
    unParsed = unParsed.trimmed(); // Sterilise whitespace for later
    // Parse the list of conditions here
    foreach(const QString &type, typeList) {
        QString typeFull = type.trimmed(); // Clean off whitespace first
        if (typeList.count() == 1) { // If the list of conditions only contains one, write into the parent
            writeItem = newItem;
        } else { // Otherwise create a new item under the parent
            writeItem = new QTreeWidgetItem(newItem);
            newItem->setText(5, "0"); // Set the item to be a condition
        }
        // Parse the individual segments into different lines
        for (int linesSeperated = 0; typeFull.count() > 0; linesSeperated = linesSeperated + 1) {
            QString writeText = typeFull.mid(0, typeFull.indexOf(" ")); // Grab all the text up to the first space
            typeFull.remove(writeText); // Remove it from the condition
            writeItem->setText(linesSeperated + 1, writeText); // Save it to the appropriate column
            typeFull = typeFull.trimmed(); // Clean off whitespace
        }
        writeItem->setText(5, "1"); // Set it to be a condition
        // Conditions can only be two types -> require different friendly text
        if (writeItem->text(1) == "IS") { // First is "IS <DeviceType>"
            writeItem->setText(0, i18n("The device must be of the type %1", writeItem->text(2)));
        } else if (writeItem->text(2) == "==") { // Another is "<DeviceType>.<DeviceProperty> == <value>
            writeItem->setText(0, i18n("The devices property %1 must equal %2", writeItem->text(1), writeItem->text(3)));
        } else if (writeItem->text(2) == "!=") { // Another is "<DeviceType>.<DeviceProperty> != <value>
            writeItem->setText(0, i18n("The devices property %1 must not equal %2", writeItem->text(1), writeItem->text(3)));
        }
    }
}

QString SolidActionsEdit::predicate()
{
    return predicateRetrieve(ui.TwSolidRequirements->invisibleRootItem()->child(0));
}

QString SolidActionsEdit::predicateRetrieve(QTreeWidgetItem *parent)
{
    QString returnText;
    QString bracketSpacer;
    QList<QTreeWidgetItem *> childrenList = parent->takeChildren(); // Grab the children of the parent
    foreach(QTreeWidgetItem * item, childrenList) {
        if (item->childCount() > 0) { // If the child count of this child isn't zero then we need to get their text
            returnText = returnText + predicateRetrieve(item);
        } else {
            QString appendText = item->text(1) + ' ' + item->text(2) + ' ' + item->text(3); // Read in + format data
            returnText.append(appendText.trimmed()); // Append the formatted data
        }
        if (childrenList.last() != item) { // If we don't have the last then we need to add the comparer
            returnText.append(' ' + parent->text(4) + ' ');
        }
    }
    if (returnText.at(0) == '[') { // If the first letter is a bracket, there is no seperating space
        bracketSpacer = "";
    } else { // If there isn't a bracket, there is a seperating space
        bracketSpacer = " ";
    }
    return QString('[' + bracketSpacer + returnText + bracketSpacer + ']'); // Add on brackets with spacer
}

void SolidActionsEdit::addRequirement()
{
    QTreeWidgetItem * newItemParent = ui.TwSolidRequirements->currentItem(); // Get the currently selected item
    QTreeWidgetItem * newItem = new QTreeWidgetItem(newItemParent); // Create a new item on the current item
    newItem->setText(6, "NEW-ITEM"); // The new requirement must have "NEW-ITEM" so its deleted if user cancels
    newItem->setText(5, "0");  // Set some safe defaults
    newItem->setText(4, "AND");  // Ditto
    ui.TwSolidRequirements->setCurrentItem(newItem);
    editRequirement(); // Open the editing interface to set the data
}

void SolidActionsEdit::cancelEditRequirement()
{
    QTreeWidgetItem * item = ui.TwSolidRequirements->currentItem(); // Retrieve current item
    if (item->text(6) == "NEW-ITEM") { // If the item is newly created and the user canceled, destroy it
        deleteRequirement();
    }
}

void SolidActionsEdit::editRequirement()
{
    QTreeWidgetItem * editItem = ui.TwSolidRequirements->currentItem(); // Retrieve current item
    int currentIndexValue = 0;

    // We need to set the type first
    if (editItem->text(5) == "0") { // The type is either "0" ->  Container
        predicateUi.CbRequirementType->setCurrentIndex(0);
    } else if (editItem->text(5) == "1") { // Or "1" -> Requirement
        predicateUi.CbRequirementType->setCurrentIndex(1);
    }
    // If the type is "Container" then we need to import the settings
    if (predicateUi.CbRequirementType->currentIndex() == 0) { // The type can be either all must match or any must match - check
        if (editItem->text(4) == "AND") {
            currentIndexValue = 0;
        } else if (editItem->text(4) == "OR") {
            currentIndexValue = 1;
        }
        predicateUi.CbContainerLogic->setCurrentIndex(currentIndexValue); // Set the type - whatever was set above
    }
    // Otherwise the type is Requirement, so import the settings for it
    else if (predicateUi.CbRequirementType->currentIndex() == 1) { // There are two types of requirement, they require different import methods: "IS <DeviceType>"
        if (editItem->text(1) == "IS") {
            predicateUi.CbRestrictionType->setCurrentIndex(0); // Set the type appropriately
            predicateUi.LeRestrictionValue->setText(editItem->text(2)); // Import <DeviceType>
        }
        // Second type of requirement "<DeviceType>.<DeviceVariable> <equals/doesn't equal> <compare data>
        else {
            predicateUi.CbRestrictionType->setCurrentIndex(1); // Set the type appropriately
            predicateUi.LeRestrictionValue->setText(editItem->text(1)); // Import <DeviceType>.<DeviceVariable>
            predicateUi.LeRestrictionRequirement->setText(editItem->text(3)); // Import <compare data>
            // If the comparer is "equals" then we should set it as so
            if (editItem->text(2) == "==") {
                predicateUi.CbRestrictionRequirement->setCurrentIndex(0);
            }
            // Otherwise is must be "doesn't equal"
            else {
                predicateUi.CbRestrictionRequirement->setCurrentIndex(1);
            }
        }
    }
    // Check if it is a new item -> if so allow the type to be selected
    if (editItem->text(6) == "NEW-ITEM") {
        predicateUi.FrRequirementType->show();
    } else {
        predicateUi.FrRequirementType->hide();
    }
    editPredicateTypeToggle();
    predicateDialog->show();
}

void SolidActionsEdit::deleteRequirement()
{
    QTreeWidgetItem * toDelete = ui.TwSolidRequirements->currentItem();
    toDelete->parent()->removeChild(toDelete);
}

void SolidActionsEdit::updateButtonUsage()
{
    QTreeWidgetItem * selected = ui.TwSolidRequirements->currentItem();
    if (selected) { // The item will become unselected during rebuild, we must check
        bool editAddActive = selected->text(5).toInt(0, 8); // Retrieve the type
        ui.PbAddRequirement->setEnabled(!editAddActive); // You can't add a requirement to condition
        ui.PbEditRequirement->setEnabled(true); // Enable in case it was disabled when currentItem() was null
        ui.PbRemoveRequirement->setEnabled(true); // Ditto
        if (selected == ui.TwSolidRequirements->invisibleRootItem()->child(0)) {
            ui.PbRemoveRequirement->setEnabled(false); // We can't allow the root item to be removed
        }
    } else { // If null, we should close off access to these, they need a valid requirement
        ui.PbAddRequirement->setEnabled(false);
        ui.PbEditRequirement->setEnabled(false);
        ui.PbRemoveRequirement->setEnabled(false);
    }
}

void SolidActionsEdit::editPredicateTypeToggle()
{
    bool currentRequireType;
    if (predicateUi.CbRequirementType->currentIndex() == 0) { // The requirement type should be a container
        currentRequireType = true; 
    } else {
        currentRequireType = false;
    }
    predicateUi.FrTypeContainer->setShown(currentRequireType); 
    predicateUi.FrTypeRestriction->setShown(!currentRequireType);
    bool currentRestrictType = predicateUi.CbRestrictionType->currentIndex(); // Get the current condition sub-type
    predicateUi.CbRestrictionRequirement->setShown(currentRestrictType);
    predicateUi.LeRestrictionRequirement->setShown(currentRestrictType);
}

void SolidActionsEdit::updateRequirement()
{
    QTreeWidgetItem * updating = ui.TwSolidRequirements->currentItem();
    updating->setText(5, QString::number(predicateUi.CbRequirementType->currentIndex())); // Set the type appropriately
    updating->setText(6, ""); // It is no longer a new item
    if (predicateUi.CbContainerLogic->currentIndex() == 0) {
        updating->setText(4, "AND"); // the container is of the all must match type
    } else if (predicateUi.CbContainerLogic->currentIndex() == 1) {
        updating->setText(4, "OR"); // the container is of the any must match type
    }
    if (predicateUi.CbRestrictionType->currentIndex() == 0) { // The condition sub-type could be "Device is of type"
        updating->setText(1, "IS"); // Set it to be so
        updating->setText(2, predicateUi.LeRestrictionValue->text()); // and import the device type
    } else { // otherwise the container sub-type must be device.value <logic> match 
        updating->setText(1, predicateUi.LeRestrictionValue->text()); // Read the device.value information
        updating->setText(3, predicateUi.LeRestrictionRequirement->text()); // Read the match information
        if (predicateUi.CbRestrictionRequirement->currentIndex() == 0) { // Is the <logic> equals
            updating->setText(2, "==");
        } else if (predicateUi.CbRestrictionRequirement->currentIndex() == 1) { // Or is it not equals
            updating->setText(2, "!=");
        }
    }
    fillPredicateTree(predicate()); // refresh the display
}

#include "solid-actions-edit.moc"
