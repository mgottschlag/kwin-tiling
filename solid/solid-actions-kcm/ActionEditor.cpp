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

#include "ActionEditor.h"

#include <Solid/Predicate>

ActionEditor::ActionEditor(QWidget *parent) : KDialog(parent)
{
    topItem = 0;
    rootItem = new PredicateItem( Solid::Predicate(), 0 );
    rootModel = new PredicateModel( rootItem, this );
    // Prepare the dialog
    setInitialSize( QSize(600, 600) ); // Set a decent initial size
    setModal( true );
    // Set up the interface
    ui.setupUi( mainWidget() );
    ui.TvPredicateTree->setHeaderHidden( true );
    ui.TvPredicateTree->setModel( rootModel );
    ui.IbActionIcon->setIconSize( KIconLoader::SizeLarge );

    ui.CbDeviceType->addItems( actionData()->interfaceList() );

    // Connect up with everything needed -> slot names explain
    connect( ui.TvPredicateTree, SIGNAL(activated(QModelIndex)), this, SLOT(updateParameter()) );
    connect( ui.PbParameterSave, SIGNAL(clicked()), this, SLOT(saveParameter()) );
    connect( ui.PbParameterReset, SIGNAL(clicked()), this, SLOT(updateParameter()) );
    connect( ui.CbDeviceType, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePropertyList()) );
    connect( ui.CbParameterType, SIGNAL(currentIndexChanged(int)), this, SLOT(manageControlStatus()) );
}

ActionEditor::~ActionEditor()
{
    if( topItem ) {
        delete topItem;
    }
}

void ActionEditor::setPredicate( Solid::Predicate predicate )
{
    if( topItem ) {
        delete topItem;
    }
    topItem = new PredicateItem( Solid::Predicate(), 0 );
    rootItem = new PredicateItem( predicate, topItem );
    rootModel->setRootPredicate( rootItem->parent() );

    // Select the top item, not the hidden root
    QModelIndex topItem = rootModel->index( 0, 0, QModelIndex() );
    ui.TvPredicateTree->setCurrentIndex( topItem );
    updateParameter();
}

void ActionEditor::updateParameter()
{
    QModelIndex current = ui.TvPredicateTree->currentIndex();
    PredicateItem * currentItem = static_cast<PredicateItem*>( current.internalPointer() );

    ui.CbParameterType->setCurrentIndex( currentItem->itemType );
    updatePropertyList();
    ui.CbDeviceType->setCurrentIndex( actionData()->interfacePosition( currentItem->ifaceType ) );
    int valuePos = actionData()->propertyPosition( currentItem->ifaceType, currentItem->property );
    ui.CbValueName->setCurrentIndex( valuePos );
    ui.LeValueMatch->setText( currentItem->value.toString() );
    ui.CbValueMatch->setCurrentIndex( currentItem->compOperator );
}

void ActionEditor::saveParameter()
{
    QModelIndex current = ui.TvPredicateTree->currentIndex();
    PredicateItem * currentItem = static_cast<PredicateItem*>( current.internalPointer() );

    currentItem->setTypeByInt( ui.CbParameterType->currentIndex() );
    currentItem->ifaceType = actionData()->interfaceFromName( ui.CbDeviceType->currentText() );
    currentItem->property = actionData()->propertyInternal( currentItem->ifaceType, ui.CbValueName->currentText() );
    currentItem->value = QVariant( ui.LeValueMatch->text() );
    currentItem->setComparisonByInt( ui.CbValueMatch->currentIndex() );
}

QString ActionEditor::predicateString()
{
    return rootItem->predicate().toString();
}

void ActionEditor::updatePropertyList()
{
    Solid::DeviceInterface::Type currentType;
    currentType = actionData()->interfaceFromName( ui.CbDeviceType->currentText() );

    ui.CbValueName->clear();
    ui.CbValueName->addItems( actionData()->propertyList( currentType ) );
}

void ActionEditor::manageControlStatus()
{
    bool atomEnable = false;
    bool isEnable = false;

    switch( ui.CbParameterType->currentIndex() ) {
        case Solid::Predicate::PropertyCheck:
            atomEnable = true;
        case Solid::Predicate::InterfaceCheck:
            isEnable = true;
            break;
        default:
            break;
    }
    ui.CbDeviceType->setEnabled( isEnable );
    ui.CbValueName->setEnabled( atomEnable );
    ui.CbValueMatch->setEnabled( atomEnable );
    ui.LeValueMatch->setEnabled( atomEnable );
}

SolidActionData * ActionEditor::actionData()
{
    return SolidActionData::instance();
}

#include "ActionEditor.moc"
