#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'XftConfigRules.ui'
**
** Created: Tue Sep 18 12:04:33 2001
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
    setCaption( QT_KDE_I18N( "XftConfig", "" ) );
    Form1Layout = new QGridLayout( this ); 
    Form1Layout->setSpacing( 6 );
    Form1Layout->setMargin( 11 );

    itsCancelButton = new QPushButton( this, "itsCancelButton" );
    itsCancelButton->setText( QT_KDE_I18N( "&Cancel", "" ) );

    Form1Layout->addWidget( itsCancelButton, 1, 2 );

    itsOkButton = new QPushButton( this, "itsOkButton" );
    itsOkButton->setText( QT_KDE_I18N( "&OK", "" ) );

    Form1Layout->addWidget( itsOkButton, 1, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Form1Layout->addItem( spacer, 1, 0 );

    TabWidget2 = new QTabWidget( this, "TabWidget2" );

    itsRulesTab = new QWidget( TabWidget2, "itsRulesTab" );
    itsRulesTabLayout = new QGridLayout( itsRulesTab ); 
    itsRulesTabLayout->setSpacing( 6 );
    itsRulesTabLayout->setMargin( 11 );

    GroupBox1 = new QGroupBox( itsRulesTab, "GroupBox1" );
    GroupBox1->setTitle( QT_KDE_I18N( "Entries:", "" ) );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 0 );
    GroupBox1->layout()->setMargin( 0 );
    GroupBox1Layout = new QGridLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( Qt::AlignTop );
    GroupBox1Layout->setSpacing( 6 );
    GroupBox1Layout->setMargin( 11 );

    itsList = new QListView( GroupBox1, "itsList" );
    itsList->addColumn( QT_KDE_I18N( "Match:", "" ) );
    itsList->addColumn( QT_KDE_I18N( "Edit:", "" ) );
    itsList->setAllColumnsShowFocus( TRUE );

    GroupBox1Layout->addMultiCellWidget( itsList, 0, 0, 0, 3 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox1Layout->addItem( spacer_2, 1, 0 );

    itsRemoveButton = new QPushButton( GroupBox1, "itsRemoveButton" );
    itsRemoveButton->setEnabled( FALSE );
    itsRemoveButton->setText( QT_KDE_I18N( "&Remove", "" ) );

    GroupBox1Layout->addWidget( itsRemoveButton, 1, 3 );

    itsAddButton = new QPushButton( GroupBox1, "itsAddButton" );
    itsAddButton->setEnabled( FALSE );
    itsAddButton->setText( QT_KDE_I18N( "&Add...", "" ) );

    GroupBox1Layout->addWidget( itsAddButton, 1, 1 );

    itsEditButton = new QPushButton( GroupBox1, "itsEditButton" );
    itsEditButton->setEnabled( FALSE );
    itsEditButton->setText( QT_KDE_I18N( "&Edit...", "" ) );

    GroupBox1Layout->addWidget( itsEditButton, 1, 2 );

    itsRulesTabLayout->addWidget( GroupBox1, 0, 0 );
    TabWidget2->insertTab( itsRulesTab, QT_KDE_I18N( "R&ules", "" ) );

    itsIncludesTab = new QWidget( TabWidget2, "itsIncludesTab" );
    itsIncludesTabLayout = new QGridLayout( itsIncludesTab ); 
    itsIncludesTabLayout->setSpacing( 6 );
    itsIncludesTabLayout->setMargin( 11 );

    itsIncludes = new CXftConfigIncludesWidget( itsIncludesTab, "itsIncludes" );
    QWhatsThis::add( itsIncludes, QT_KDE_I18N( "List here any other Xft files that you want included from your Xft file.\n"
"\n"
"NOTE: Xft will complain if any files from this list are not found.", "" ) );

    itsIncludesTabLayout->addWidget( itsIncludes, 0, 0 );

    itsIncludeIfs = new CXftConfigIncludesWidget( itsIncludesTab, "itsIncludeIfs" );
    QWhatsThis::add( itsIncludeIfs, QT_KDE_I18N( "List here any other Xft files that you want included from your Xft file.\n"
"\n"
"NOTE: Xft will *not* complain if any files from this list are not found.", "" ) );

    itsIncludesTabLayout->addWidget( itsIncludeIfs, 1, 0 );
    TabWidget2->insertTab( itsIncludesTab, QT_KDE_I18N( "&Include Directives", "" ) );

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
