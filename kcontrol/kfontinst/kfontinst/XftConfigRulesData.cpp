#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'XftConfigRules.ui'
**
** Created: Wed Oct 24 21:21:41 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "XftConfigRulesData.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qgroupbox.h>
#include <qheader.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include "XftConfigIncludesWidget.h"
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CXftConfigRulesData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CXftConfigRulesData::CXftConfigRulesData( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : KDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "Form1" );
    resize( 537, 416 ); 
    setCaption( tr2i18n( "XftConfig" ) );
    Form1Layout = new QGridLayout( this, 1, 1, 11, 6, "Form1Layout"); 

    itsCancelButton = new QPushButton( this, "itsCancelButton" );
    itsCancelButton->setText( tr2i18n( "&Cancel" ) );

    Form1Layout->addWidget( itsCancelButton, 1, 2 );

    itsOkButton = new QPushButton( this, "itsOkButton" );
    itsOkButton->setText( tr2i18n( "&OK" ) );

    Form1Layout->addWidget( itsOkButton, 1, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Form1Layout->addItem( spacer, 1, 0 );

    TabWidget2 = new QTabWidget( this, "TabWidget2" );

    itsRulesTab = new QWidget( TabWidget2, "itsRulesTab" );
    itsRulesTabLayout = new QGridLayout( itsRulesTab, 1, 1, 11, 6, "itsRulesTabLayout"); 

    GroupBox1 = new QGroupBox( itsRulesTab, "GroupBox1" );
    GroupBox1->setTitle( tr2i18n( "Entries:" ) );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 6 );
    GroupBox1->layout()->setMargin( 11 );
    GroupBox1Layout = new QGridLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( Qt::AlignTop );

    itsList = new QListView( GroupBox1, "itsList" );
    itsList->addColumn( tr2i18n( "Match:" ) );
    itsList->addColumn( tr2i18n( "Edit:" ) );
    itsList->setAllColumnsShowFocus( TRUE );

    GroupBox1Layout->addMultiCellWidget( itsList, 0, 0, 0, 3 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox1Layout->addItem( spacer_2, 1, 0 );

    itsRemoveButton = new QPushButton( GroupBox1, "itsRemoveButton" );
    itsRemoveButton->setEnabled( FALSE );
    itsRemoveButton->setText( tr2i18n( "&Remove" ) );

    GroupBox1Layout->addWidget( itsRemoveButton, 1, 3 );

    itsAddButton = new QPushButton( GroupBox1, "itsAddButton" );
    itsAddButton->setEnabled( FALSE );
    itsAddButton->setText( tr2i18n( "&Add..." ) );

    GroupBox1Layout->addWidget( itsAddButton, 1, 1 );

    itsEditButton = new QPushButton( GroupBox1, "itsEditButton" );
    itsEditButton->setEnabled( FALSE );
    itsEditButton->setText( tr2i18n( "&Edit..." ) );

    GroupBox1Layout->addWidget( itsEditButton, 1, 2 );

    itsRulesTabLayout->addWidget( GroupBox1, 0, 0 );
    TabWidget2->insertTab( itsRulesTab, tr2i18n( "R&ules" ) );

    itsIncludesTab = new QWidget( TabWidget2, "itsIncludesTab" );
    itsIncludesTabLayout = new QGridLayout( itsIncludesTab, 1, 1, 11, 6, "itsIncludesTabLayout"); 

    itsIncludes = new CXftConfigIncludesWidget( itsIncludesTab, "itsIncludes" );
    QWhatsThis::add( itsIncludes, tr2i18n( "List here any other Xft files that you want included from your Xft file.\n"
"\n"
"NOTE: Xft will complain if any files from this list are not found." ) );

    itsIncludesTabLayout->addWidget( itsIncludes, 0, 0 );

    itsIncludeIfs = new CXftConfigIncludesWidget( itsIncludesTab, "itsIncludeIfs" );
    QWhatsThis::add( itsIncludeIfs, tr2i18n( "List here any other Xft files that you want included from your Xft file.\n"
"\n"
"NOTE: Xft will *not* complain if any files from this list are not found." ) );

    itsIncludesTabLayout->addWidget( itsIncludeIfs, 1, 0 );
    TabWidget2->insertTab( itsIncludesTab, tr2i18n( "&Include Directives" ) );

    Form1Layout->addMultiCellWidget( TabWidget2, 0, 0, 0, 2 );

    // signals and slots connections
    connect( itsList, SIGNAL( currentChanged(QListViewItem*) ), this, SLOT( itemSelected(QListViewItem*) ) );
    connect( itsOkButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( itsCancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( itsAddButton, SIGNAL( clicked() ), this, SLOT( addButtonPressed() ) );
    connect( itsEditButton, SIGNAL( clicked() ), this, SLOT( editButtonPressed() ) );
    connect( itsRemoveButton, SIGNAL( clicked() ), this, SLOT( removeButtonPressed() ) );

    // tab order
    setTabOrder( itsList, itsAddButton );
    setTabOrder( itsAddButton, itsEditButton );
    setTabOrder( itsEditButton, itsRemoveButton );
    setTabOrder( itsRemoveButton, itsOkButton );
    setTabOrder( itsOkButton, itsCancelButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CXftConfigRulesData::~CXftConfigRulesData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CXftConfigRulesData::addButtonPressed()
{
    qWarning( "CXftConfigRulesData::addButtonPressed(): Not implemented yet!" );
}

void CXftConfigRulesData::itemSelected(QListViewItem *)
{
    qWarning( "CXftConfigRulesData::itemSelected(QListViewItem *): Not implemented yet!" );
}

void CXftConfigRulesData::editButtonPressed()
{
    qWarning( "CXftConfigRulesData::editButtonPressed(): Not implemented yet!" );
}

void CXftConfigRulesData::removeButtonPressed()
{
    qWarning( "CXftConfigRulesData::removeButtonPressed(): Not implemented yet!" );
}

#include "XftConfigRulesData.moc"
