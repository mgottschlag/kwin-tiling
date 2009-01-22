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

#include "solid-action-edit.h"
#include "solid-action-edit-predicate.h"

#include <KLocale>
#include <KMessageBox>
#include <KDebug>

#include <Solid/Predicate>

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

SolidActionEdit::SolidActionEdit(QWidget *parent) : KDialog(parent)
{
    // Set up the interface
    QWidget *editWidget = new QWidget(this);
    setMainWidget(editWidget);
    ui.setupUi(editWidget);
    setInitialSize(QSize(600, 400)); // Set a decent initial size
    ui.TwSolidRequirements->setHeaderLabel(""); // We don't need a header label

    // Instantiate the predicate edit dialog
    predicateUi = new SolidActionEditPredicate(this);

    // Connect up with everything needed -> slot names explain
    connect(ui.TwSolidRequirements, SIGNAL(itemSelectionChanged()), this, SLOT(updateButtonUsage()));
    connect(ui.PbAddRequirement, SIGNAL(clicked()), this, SLOT(addRequirement()));
    connect(ui.PbEditRequirement, SIGNAL(clicked()), this, SLOT(editRequirement()));
    connect(ui.PbRemoveRequirement, SIGNAL(clicked()), this, SLOT(deleteRequirement()));
    connect(predicateUi, SIGNAL(okClicked()), this, SLOT(updateRequirement()));
    connect(predicateUi, SIGNAL(cancelClicked()), this, SLOT(cancelEditRequirement()));
    updateButtonUsage();
}

SolidActionEdit::~SolidActionEdit()
{
}

void SolidActionEdit::fillPredicateTree(QString predicateText)
{
    ui.TwSolidRequirements->setColumnCount(1); // We only want to show the user friendly column
    ui.TwSolidRequirements->clear(); // We shouldn't show previously built results
    if (!Solid::Predicate::fromString(predicateText).isValid()) {
        hide();
        KMessageBox::error(this, i18n("It appears that the device conditions for this action are not valid. \nIf you previously used this utility to make changes, please revert them and file a bug"), i18n("Error parsing device conditions"));
        return;
    }
    QTreeWidgetItem *masterRoot = new QTreeWidgetItem(ui.TwSolidRequirements->invisibleRootItem()); // Get the root
    masterRoot->setText(5, "0"); // The Master Root must be a container
    setPredicateContainer(predicateText, masterRoot); // Import the data
    setPrettyNames(ui.TwSolidRequirements->invisibleRootItem()); // refresh the display
}

void SolidActionEdit::setPredicateContainer(QString predicate, QTreeWidgetItem *parent)
{
    int leftBracket;
    int rightBracket;
    int bracketBegin;
    int bracketEnd;
    QString parsedString;
    QStringList typeList;

    // Remove any "[" or "]" characters at beginning or end
    QString unParsed = predicate;
    if (unParsed.at(0) == '[') {
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
        if (parsedString.contains("AND") || parsedString.contains("OR")) { // If the grabbed text only has 1 "[" then it is a condition
            QTreeWidgetItem *newParent = new QTreeWidgetItem(parent);
            newParent->setText(5, "0"); // Set the type to be container ( 0 = container, 1 = condition )
            setPredicateContainer(parsedString, newParent);
        } else { // Otherwise it contains more containers
            setPredicateContainer(parsedString, parent);
        }
    }
    unParsed = unParsed.trimmed(); // Remove any lingering whitespace
    // We need to grab a condition if left behind
    typeList << "OR" << "AND";
    foreach(const QString &type, typeList) {
        if (unParsed == type) {
            parent->setText(4, type); // Store the type away in the container for later use
            unParsed.clear(); // Clear it so we don't try to process it later as a condition
        }
    }
    // If the unparsed text isn't empty and has no brackets then we must have a condition so import it
    if (unParsed.count("[") == 0 && !unParsed.isEmpty()) {
        setPredicateMultiItem(unParsed, parent);
    }
}

void SolidActionEdit::setPredicateMultiItem(QString predicate, QTreeWidgetItem *parent)
{
    QTreeWidgetItem *writeItem;
    QString unParsed = predicate;
    QStringList typeList;
    // The predicate could have multiple types in it separate them if they exist
    typeList << "OR" << "AND";
    foreach(const QString &type, typeList) {
        if (unParsed.contains(type)) { // Could be any contained be true
            unParsed.replace(type, "$"); // Replace with unified symbol for easier splitting later
            parent->setText(4, type); // Store the type away in the container for later use
        }
    }
    typeList.clear();
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
    foreach(const QString &type, typeList) {
        if (typeList.count() == 1) { // If the list of conditions only contains one, write into the parent
            writeItem = parent;
        } else { // Otherwise create a new item under the parent
            writeItem = new QTreeWidgetItem(parent);
            parent->setText(5, "0"); // Set the item to be a condition
        }
        setPredicateItem(type, writeItem);
    }
}

void SolidActionEdit::setPredicateItem(QString predicate, QTreeWidgetItem *parent)
{
    QString typeFull = predicate.trimmed(); // Clean off whitespace first
    // Parse the individual segments into different lines
    for (int linesSeperated = 0; typeFull.count() > 0; linesSeperated = linesSeperated + 1) {
        QString writeText = typeFull.mid(0, typeFull.indexOf(" ")); // Grab all the text up to the first space
        typeFull.remove(writeText); // Remove it from the condition
        parent->setText(linesSeperated + 1, writeText); // Save it to the appropriate column
        typeFull = typeFull.trimmed(); // Clean off whitespace
    }
    parent->setText(5, "1"); // Set it to be a condition
}

void SolidActionEdit::setPrettyNames(QTreeWidgetItem *parent)
{
    QTreeWidgetItem *child;
    for (int count = 0; count < parent->childCount(); count = count + 1) {
        child = parent->child(count);
        if (child->childCount() > 0) {  // Does this child have any children itself?
            setPrettyNames(child);
        } // We need to set their names as well
        if (child->text(5) == "0") {  // Is it a container?
            if (child->text(4) == "OR") {  // There are 2 types: "OR" ( Any can match )
                child->setText(0, i18n("Any of the contained conditions must match"));
            } else if (child->text(4) == "AND") { // Alternately there is "AND"
                child->setText(0, i18n("All of the contained conditions must match"));
            }
        } else if (child->text(5) == "1") { // Then it might be Condition
            if (child->text(1) == "IS") { // First it could "IS <DeviceType>"
                child->setText(0, i18n("The device must be of the type %1", child->text(2)));
            } else if (child->text(2) == "==") { // maybe "<DeviceType>.<DeviceProperty> == <value>?
                child->setText(0, i18n("The devices property %1 must equal %2", child->text(1), child->text(3)));
            } else if (child->text(2) == "!=") { // Also might be "<DeviceType>.<DeviceProperty> != <value>
                child->setText(0, i18n("The devices property %1 must not equal %2", child->text(1), child->text(3)));
            }
        }
    }
}

QString SolidActionEdit::predicate()
{
    return readPredicate(ui.TwSolidRequirements->invisibleRootItem()->child(0));
}

QString SolidActionEdit::readPredicate(QTreeWidgetItem *parent)
{
    QString returnText;
    QString bracketSpacer;
    QList<QTreeWidgetItem *> childrenList = parent->takeChildren(); // Grab the children of the parent
    foreach(QTreeWidgetItem * item, childrenList) {
        if (item->childCount() > 0) { // If the child count of this child isn't zero then we need to get their text
            returnText = returnText + readPredicate(item);
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
    parent->addChildren(childrenList);
    return QString('[' + bracketSpacer + returnText + bracketSpacer + ']'); // Add on brackets with spacer
}

void SolidActionEdit::addRequirement()
{
    QTreeWidgetItem * newItemParent = ui.TwSolidRequirements->currentItem(); // Get the currently selected item
    QTreeWidgetItem * newItem = new QTreeWidgetItem(newItemParent); // Create a new item on the current item
    newItem->setText(6, "NEW-ITEM"); // The new requirement must have "NEW-ITEM" so its deleted if user cancels
    newItem->setText(5, "0");  // Set some safe defaults
    newItem->setText(4, "AND");  // Ditto
    ui.TwSolidRequirements->setCurrentItem(newItem);
    editRequirement(); // Open the editing interface to set the data
}

void SolidActionEdit::cancelEditRequirement()
{
    QTreeWidgetItem * item = ui.TwSolidRequirements->currentItem(); // Retrieve current item
    if (item->text(6) == "NEW-ITEM") { // If the item is newly created and the user canceled, destroy it
        deleteRequirement();
    }
}

void SolidActionEdit::editRequirement()
{
    predicateUi->prepareShow(ui.TwSolidRequirements->currentItem()); // Fill display
    predicateUi->show();
}

void SolidActionEdit::deleteRequirement()
{
    QTreeWidgetItem * toDelete = ui.TwSolidRequirements->currentItem();
    toDelete->parent()->removeChild(toDelete);
}

void SolidActionEdit::updateButtonUsage()
{
    QTreeWidgetItem * selected = ui.TwSolidRequirements->currentItem();
    if (selected) { // The item will become unselected during rebuild, we must check
        bool editAddActive = selected->text(5).toInt(0, 8); // Retrieve the type
        ui.PbAddRequirement->setEnabled(!editAddActive); // You can't add a requirement to a condition
        ui.PbEditRequirement->setEnabled(true); // Enable in case it was disabled when currentItem() was null
        ui.PbRemoveRequirement->setEnabled(true); // Ditto
        if (selected == ui.TwSolidRequirements->invisibleRootItem()->child(0)) {
            ui.PbRemoveRequirement->setEnabled(false); // We can't allow the root item to be removed
        }
    } else { // If null, close off access to these, they need a valid requirement
        ui.PbAddRequirement->setEnabled(false);
        ui.PbEditRequirement->setEnabled(false);
        ui.PbRemoveRequirement->setEnabled(false);
    }
}

void SolidActionEdit::updateRequirement()
{
    predicateUi->finishShow(ui.TwSolidRequirements->currentItem()); // ensure the changes are written
    setPrettyNames(ui.TwSolidRequirements->invisibleRootItem()); // refresh the display
}

#include "solid-action-edit.moc"
