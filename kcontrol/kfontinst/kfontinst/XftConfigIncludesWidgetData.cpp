#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'XftConfigIncludesWidget.ui'
**
** Created: Wed Jul 4 01:45:44 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "XftConfigIncludesWidgetData.h"

#include <qgroupbox.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CXftConfigIncludesWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
CXftConfigIncludesWidgetData::CXftConfigIncludesWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CXftConfigIncludesWidgetData" );
    resize( 540, 218 ); 
    setCaption( i18n( "Form1" ) );
    CXftConfigIncludesWidgetDataLayout = new QGridLayout( this ); 
    CXftConfigIncludesWidgetDataLayout->setSpacing( 0 );
    CXftConfigIncludesWidgetDataLayout->setMargin( 0 );

    itsGroupBox = new QGroupBox( this, "itsGroupBox" );
    itsGroupBox->setTitle( i18n( "\"Include\":" ) );
    itsGroupBox->setColumnLayout(0, Qt::Vertical );
    itsGroupBox->layout()->setSpacing( 0 );
    itsGroupBox->layout()->setMargin( 0 );
    itsGroupBoxLayout = new QGridLayout( itsGroupBox->layout() );
    itsGroupBoxLayout->setAlignment( Qt::AlignTop );
    itsGroupBoxLayout->setSpacing( 6 );
    itsGroupBoxLayout->setMargin( 11 );

    itsList = new QListBox( itsGroupBox, "itsList" );

    itsGroupBoxLayout->addMultiCellWidget( itsList, 0, 0, 0, 3 );

    itsRemoveButton = new QPushButton( itsGroupBox, "itsRemoveButton" );
    itsRemoveButton->setEnabled( FALSE );
    itsRemoveButton->setText( i18n( "Remove" ) );

    itsGroupBoxLayout->addWidget( itsRemoveButton, 1, 3 );

    itsEditButton = new QPushButton( itsGroupBox, "itsEditButton" );
    itsEditButton->setEnabled( FALSE );
    itsEditButton->setText( i18n( "Edit..." ) );

    itsGroupBoxLayout->addWidget( itsEditButton, 1, 2 );

    itsAddButton = new QPushButton( itsGroupBox, "itsAddButton" );
    itsAddButton->setEnabled( TRUE );
    itsAddButton->setText( i18n( "Add..." ) );

    itsGroupBoxLayout->addWidget( itsAddButton, 1, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    itsGroupBoxLayout->addItem( spacer, 1, 0 );

    CXftConfigIncludesWidgetDataLayout->addWidget( itsGroupBox, 0, 0 );

    // signals and slots connections
    connect( itsList, SIGNAL( selectionChanged(QListBoxItem*) ), this, SLOT( itemSelected(QListBoxItem *) ) );
    connect( itsAddButton, SIGNAL( clicked() ), this, SLOT( addPressed() ) );
    connect( itsRemoveButton, SIGNAL( clicked() ), this, SLOT( removePressed() ) );
    connect( itsEditButton, SIGNAL( clicked() ), this, SLOT( editPressed() ) );

    // tab order
    setTabOrder( itsList, itsAddButton );
    setTabOrder( itsAddButton, itsEditButton );
    setTabOrder( itsEditButton, itsRemoveButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CXftConfigIncludesWidgetData::~CXftConfigIncludesWidgetData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CXftConfigIncludesWidgetData::addPressed()
{
    qWarning( "CXftConfigIncludesWidgetData::addPressed(): Not implemented yet!" );
}

void CXftConfigIncludesWidgetData::itemSelected(QListBoxItem *)
{
    qWarning( "CXftConfigIncludesWidgetData::itemSelected(QListBoxItem *): Not implemented yet!" );
}

void CXftConfigIncludesWidgetData::editPressed()
{
    qWarning( "CXftConfigIncludesWidgetData::editPressed(): Not implemented yet!" );
}

void CXftConfigIncludesWidgetData::removePressed()
{
    qWarning( "CXftConfigIncludesWidgetData::removePressed(): Not implemented yet!" );
}

#include "XftConfigIncludesWidgetData.moc"
