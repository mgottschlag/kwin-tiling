#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'FontListWidget.ui'
**
** Created: Sat May 12 01:51:27 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "FontListWidgetData.h"

#include <qgroupbox.h>
#include <qheader.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CFontListWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
CFontListWidgetData::CFontListWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CFontListWidgetData" );
    resize( 407, 155 ); 
    setCaption( i18n( "Form1" ) );
    CFontListWidgetDataLayout = new QGridLayout( this ); 
    CFontListWidgetDataLayout->setSpacing( 0 );
    CFontListWidgetDataLayout->setMargin( 0 );

    itsBox = new QGroupBox( this, "itsBox" );
    itsBox->setTitle( i18n( "Label" ) );
    itsBox->setColumnLayout(0, Qt::Vertical );
    itsBox->layout()->setSpacing( 0 );
    itsBox->layout()->setMargin( 0 );
    itsBoxLayout = new QGridLayout( itsBox->layout() );
    itsBoxLayout->setAlignment( Qt::AlignTop );
    itsBoxLayout->setSpacing( 6 );
    itsBoxLayout->setMargin( 11 );

    itsList = new QListView( itsBox, "itsList" );
    itsList->addColumn( i18n( "Directory/File" ) );
    itsList->addColumn( i18n( "Name" ) );
    itsList->setMinimumSize( QSize( 0, 24 ) );
    itsList->setSelectionMode( QListView::Extended );
    itsList->setAllColumnsShowFocus( TRUE );
    itsList->setShowSortIndicator( TRUE );

    itsBoxLayout->addMultiCellWidget( itsList, 0, 0, 0, 2 );

    itsButton2 = new QPushButton( itsBox, "itsButton2" );
    itsButton2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, itsButton2->sizePolicy().hasHeightForWidth() ) );
    itsButton2->setMinimumSize( QSize( 0, 0 ) );
    itsButton2->setMaximumSize( QSize( 32767, 32767 ) );
    itsButton2->setText( i18n( "Action" ) );
    QToolTip::add(  itsButton2, i18n( "Select File." ) );

    itsBoxLayout->addWidget( itsButton2, 1, 2 );

    itsButton1 = new QPushButton( itsBox, "itsButton1" );
    itsButton1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, itsButton1->sizePolicy().hasHeightForWidth() ) );
    itsButton1->setMinimumSize( QSize( 0, 0 ) );
    itsButton1->setMaximumSize( QSize( 32767, 32767 ) );
    itsButton1->setText( i18n( "Action" ) );
    QToolTip::add(  itsButton1, i18n( "Select File." ) );

    itsBoxLayout->addWidget( itsButton1, 1, 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    itsBoxLayout->addItem( spacer, 1, 1 );

    CFontListWidgetDataLayout->addWidget( itsBox, 0, 0 );

    // signals and slots connections
    connect( itsList, SIGNAL( selectionChanged() ), this, SLOT( selectionChanged() ) );
    connect( itsList, SIGNAL( currentChanged(QListViewItem*) ), this, SLOT( selectionChanged() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CFontListWidgetData::~CFontListWidgetData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CFontListWidgetData::selectionChanged()
{
    qWarning( "CFontListWidgetData::selectionChanged(): Not implemented yet!" );
}

