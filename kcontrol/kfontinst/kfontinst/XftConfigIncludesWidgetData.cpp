#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'XftConfigIncludesWidget.ui'
**
** Created: Wed Nov 21 00:35:22 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "XftConfigIncludesWidgetData.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CXftConfigIncludesWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
CXftConfigIncludesWidgetData::CXftConfigIncludesWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CXftConfigIncludesWidgetData" );
    resize( 540, 218 ); 
    setCaption( tr2i18n( "Form1" ) );
    CXftConfigIncludesWidgetDataLayout = new QGridLayout( this, 1, 1, 0, 0, "CXftConfigIncludesWidgetDataLayout"); 

    itsGroupBox = new QGroupBox( this, "itsGroupBox" );
    itsGroupBox->setTitle( tr2i18n( "\"Include\":" ) );
    itsGroupBox->setColumnLayout(0, Qt::Vertical );
    itsGroupBox->layout()->setSpacing( 6 );
    itsGroupBox->layout()->setMargin( 11 );
    itsGroupBoxLayout = new QGridLayout( itsGroupBox->layout() );
    itsGroupBoxLayout->setAlignment( Qt::AlignTop );

    itsList = new QListBox( itsGroupBox, "itsList" );
    itsList->setFrameShape( QListBox::StyledPanel );
    itsList->setFrameShadow( QListBox::Sunken );

    itsGroupBoxLayout->addMultiCellWidget( itsList, 0, 0, 0, 3 );

    itsRemoveButton = new QPushButton( itsGroupBox, "itsRemoveButton" );
    itsRemoveButton->setEnabled( FALSE );
    itsRemoveButton->setText( tr2i18n( "Remove" ) );

    itsGroupBoxLayout->addWidget( itsRemoveButton, 1, 3 );

    itsEditButton = new QPushButton( itsGroupBox, "itsEditButton" );
    itsEditButton->setEnabled( FALSE );
    itsEditButton->setText( tr2i18n( "Edit..." ) );

    itsGroupBoxLayout->addWidget( itsEditButton, 1, 2 );

    itsAddButton = new QPushButton( itsGroupBox, "itsAddButton" );
    itsAddButton->setEnabled( TRUE );
    itsAddButton->setText( tr2i18n( "Add..." ) );

    itsGroupBoxLayout->addWidget( itsAddButton, 1, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    itsGroupBoxLayout->addItem( spacer, 1, 0 );

    CXftConfigIncludesWidgetDataLayout->addWidget( itsGroupBox, 0, 0 );

    // signals and slots connections
    connect( itsList, SIGNAL( selectionChanged(QListBoxItem*) ), this, SLOT( itemSelected(QListBoxItem*) ) );
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

void CXftConfigIncludesWidgetData::editPressed()
{
    qWarning( "CXftConfigIncludesWidgetData::editPressed(): Not implemented yet!" );
}

void CXftConfigIncludesWidgetData::itemSelected(QListBoxItem *)
{
    qWarning( "CXftConfigIncludesWidgetData::itemSelected(QListBoxItem *): Not implemented yet!" );
}

void CXftConfigIncludesWidgetData::removePressed()
{
    qWarning( "CXftConfigIncludesWidgetData::removePressed(): Not implemented yet!" );
}

#include "XftConfigIncludesWidgetData.moc"
