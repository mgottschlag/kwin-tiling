#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'KfiMainWidget.ui'
**
** Created: Mon Sep 17 00:38:07 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "KfiMainWidgetData.h"

#include <qpushbutton.h>
#include <qtabwidget.h>
#include "AboutWidget.h"
#include "FontsWidget.h"
#include "SettingsWidget.h"
#include "XftConfigSettingsWidget.h"
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CKfiMainWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
CKfiMainWidgetData::CKfiMainWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CKfiMainWidgetData" );
    resize( 540, 480 ); 
    setCaption( i18n( "Form1" ) );
    CKfiMainWidgetDataLayout = new QGridLayout( this ); 
    CKfiMainWidgetDataLayout->setSpacing( 5 );
    CKfiMainWidgetDataLayout->setMargin( 5 );

    itsTab = new QTabWidget( this, "itsTab" );
    itsTab->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, itsTab->sizePolicy().hasHeightForWidth() ) );

    itsFontsTab = new QWidget( itsTab, "itsFontsTab" );
    itsFontsTabLayout = new QGridLayout( itsFontsTab ); 
    itsFontsTabLayout->setSpacing( 0 );
    itsFontsTabLayout->setMargin( 0 );

    itsFonts = new CFontsWidget( itsFontsTab, "itsFonts" );
    itsFonts->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, itsFonts->sizePolicy().hasHeightForWidth() ) );

    itsFontsTabLayout->addWidget( itsFonts, 0, 0 );
    itsTab->insertTab( itsFontsTab, i18n( "F&onts" ) );

    itsAATab = new QWidget( itsTab, "itsAATab" );
    itsAATabLayout = new QGridLayout( itsAATab ); 
    itsAATabLayout->setSpacing( 0 );
    itsAATabLayout->setMargin( 0 );

    itsAA = new CXftConfigSettingsWidget( itsAATab, "itsAA" );

    itsAATabLayout->addWidget( itsAA, 0, 0 );
    itsTab->insertTab( itsAATab, i18n( "A&nti-Alias" ) );

    itsSettingsTab = new QWidget( itsTab, "itsSettingsTab" );
    itsSettingsTabLayout = new QGridLayout( itsSettingsTab ); 
    itsSettingsTabLayout->setSpacing( 0 );
    itsSettingsTabLayout->setMargin( 0 );

    itsSettings = new CSettingsWidget( itsSettingsTab, "itsSettings" );

    itsSettingsTabLayout->addWidget( itsSettings, 0, 0 );
    itsTab->insertTab( itsSettingsTab, i18n( "&Settings" ) );

    itsAboutTab = new QWidget( itsTab, "itsAboutTab" );
    itsAboutTabLayout = new QGridLayout( itsAboutTab ); 
    itsAboutTabLayout->setSpacing( 0 );
    itsAboutTabLayout->setMargin( 0 );

    CAboutWidget1 = new CAboutWidget( itsAboutTab, "CAboutWidget1" );

    itsAboutTabLayout->addWidget( CAboutWidget1, 0, 0 );
    itsTab->insertTab( itsAboutTab, i18n( "A&bout" ) );

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
