#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'KfiMainWidget.ui'
**
** Created: Wed Oct 24 21:21:39 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "KfiMainWidgetData.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qpushbutton.h>
#include <qtabwidget.h>
#include "FontsWidget.h"
#include "SettingsWidget.h"
#include "XftConfigSettingsWidget.h"
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CKfiMainWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
CKfiMainWidgetData::CKfiMainWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CKfiMainWidgetData" );
    resize( 538, 359 ); 
    setCaption( tr2i18n( "Form1" ) );
    CKfiMainWidgetDataLayout = new QGridLayout( this, 1, 1, 5, 5, "CKfiMainWidgetDataLayout"); 

    itsTab = new QTabWidget( this, "itsTab" );
    itsTab->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, itsTab->sizePolicy().hasHeightForWidth() ) );

    itsFontsTab = new QWidget( itsTab, "itsFontsTab" );
    itsFontsTabLayout = new QGridLayout( itsFontsTab, 1, 1, 0, 0, "itsFontsTabLayout"); 

    itsFonts = new CFontsWidget( itsFontsTab, "itsFonts" );
    itsFonts->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, itsFonts->sizePolicy().hasHeightForWidth() ) );

    itsFontsTabLayout->addWidget( itsFonts, 0, 0 );
    itsTab->insertTab( itsFontsTab, tr2i18n( "F&onts" ) );

    itsAATab = new QWidget( itsTab, "itsAATab" );
    itsAATabLayout = new QGridLayout( itsAATab, 1, 1, 0, 0, "itsAATabLayout"); 

    itsAA = new CXftConfigSettingsWidget( itsAATab, "itsAA" );

    itsAATabLayout->addWidget( itsAA, 0, 0 );
    itsTab->insertTab( itsAATab, tr2i18n( "A&nti-Alias" ) );

    itsSettingsTab = new QWidget( itsTab, "itsSettingsTab" );
    itsSettingsTabLayout = new QGridLayout( itsSettingsTab, 1, 1, 0, 0, "itsSettingsTabLayout"); 

    itsSettings = new CSettingsWidget( itsSettingsTab, "itsSettings" );

    itsSettingsTabLayout->addWidget( itsSettings, 0, 0 );
    itsTab->insertTab( itsSettingsTab, tr2i18n( "&Settings" ) );

    CKfiMainWidgetDataLayout->addWidget( itsTab, 0, 0 );

    // signals and slots connections
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CKfiMainWidgetData::~CKfiMainWidgetData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CKfiMainWidgetData::tabChanged(QWidget *)
{
    qWarning( "CKfiMainWidgetData::tabChanged(QWidget *): Not implemented yet!" );
}

#include "KfiMainWidgetData.moc"
