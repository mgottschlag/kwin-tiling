#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'DisplaySettingsWidget.ui'
**
** Created: Tue Jul 31 21:00:14 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "DisplaySettingsWidgetData.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CDisplaySettingsWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
CDisplaySettingsWidgetData::CDisplaySettingsWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CDisplaySettingsWidgetData" );
    resize( 444, 234 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)5, sizePolicy().hasHeightForWidth() ) );
    setCaption( i18n( "Form5" ) );
    CDisplaySettingsWidgetDataLayout = new QGridLayout( this ); 
    CDisplaySettingsWidgetDataLayout->setSpacing( 6 );
    CDisplaySettingsWidgetDataLayout->setMargin( 11 );

    ButtonGroup5_3 = new QButtonGroup( this, "ButtonGroup5_3" );
    ButtonGroup5_3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, ButtonGroup5_3->sizePolicy().hasHeightForWidth() ) );
    ButtonGroup5_3->setTitle( i18n( "Mode:" ) );
    ButtonGroup5_3->setColumnLayout(0, Qt::Vertical );
    ButtonGroup5_3->layout()->setSpacing( 0 );
    ButtonGroup5_3->layout()->setMargin( 0 );
    ButtonGroup5_3Layout = new QGridLayout( ButtonGroup5_3->layout() );
    ButtonGroup5_3Layout->setAlignment( Qt::AlignTop );
    ButtonGroup5_3Layout->setSpacing( 6 );
    ButtonGroup5_3Layout->setMargin( 11 );

    itsAdvanced = new QRadioButton( ButtonGroup5_3, "itsAdvanced" );
    itsAdvanced->setText( i18n( "amp;Advanced" ) );
    QWhatsThis::add(  itsAdvanced, i18n( "In \"advanced\" mode the complete X11\nfonts folder structure is displayed - allowing\nyou to specify exactly where a font should\nbe installed.\n\nThis mode also allows you to install Speedo\nand bitmap fonts." ) );

    ButtonGroup5_3Layout->addWidget( itsAdvanced, 0, 0 );

    itsBasic = new QRadioButton( ButtonGroup5_3, "itsBasic" );
    itsBasic->setText( i18n( "&Basic" ) );
    itsBasic->setChecked( TRUE );
    QWhatsThis::add(  itsBasic, i18n( "This mode only displays TrueType and Type1\nfonts - and will hide the underlying folder\nstructure." ) );

    ButtonGroup5_3Layout->addWidget( itsBasic, 0, 1 );

    CDisplaySettingsWidgetDataLayout->addMultiCellWidget( ButtonGroup5_3, 0, 0, 0, 2 );
    QSpacerItem* spacer = new QSpacerItem( 16, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    CDisplaySettingsWidgetDataLayout->addItem( spacer, 1, 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    CDisplaySettingsWidgetDataLayout->addItem( spacer_2, 5, 1 );

    itsCustomCheck = new QCheckBox( this, "itsCustomCheck" );
    itsCustomCheck->setText( i18n( "&Custom preview string:" ) );

    CDisplaySettingsWidgetDataLayout->addMultiCellWidget( itsCustomCheck, 4, 4, 0, 1 );

    itsCustomText = new QLineEdit( this, "itsCustomText" );
    itsCustomText->setEnabled( FALSE );
    QWhatsThis::add(  itsCustomText, i18n( "Enter your custom preview string. This will be\nused in the font lists in the \"Fonts\" tab." ) );

    CDisplaySettingsWidgetDataLayout->addWidget( itsCustomText, 4, 2 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
    CDisplaySettingsWidgetDataLayout->addItem( spacer_3, 3, 1 );

    ButtonGroup2 = new QButtonGroup( this, "ButtonGroup2" );
    ButtonGroup2->setTitle( i18n( "Font Lists:" ) );
    ButtonGroup2->setColumnLayout(0, Qt::Vertical );
    ButtonGroup2->layout()->setSpacing( 0 );
    ButtonGroup2->layout()->setMargin( 0 );
    ButtonGroup2Layout = new QGridLayout( ButtonGroup2->layout() );
    ButtonGroup2Layout->setAlignment( Qt::AlignTop );
    ButtonGroup2Layout->setSpacing( 6 );
    ButtonGroup2Layout->setMargin( 11 );

    itsLeftAndRight = new QRadioButton( ButtonGroup2, "itsLeftAndRight" );
    itsLeftAndRight->setText( i18n( "&Left and right" ) );

    ButtonGroup2Layout->addWidget( itsLeftAndRight, 0, 0 );

    itsTopAndBottom = new QRadioButton( ButtonGroup2, "itsTopAndBottom" );
    itsTopAndBottom->setText( i18n( "&Top and bottom" ) );
    itsTopAndBottom->setChecked( TRUE );

    ButtonGroup2Layout->addWidget( itsTopAndBottom, 0, 1 );

    CDisplaySettingsWidgetDataLayout->addMultiCellWidget( ButtonGroup2, 2, 2, 0, 2 );

    // signals and slots connections
    connect( itsCustomCheck, SIGNAL( toggled(bool) ), itsCustomText, SLOT( setEnabled(bool) ) );
    connect( itsAdvanced, SIGNAL( toggled(bool) ), this, SLOT( advancedSelected(bool) ) );
    connect( itsTopAndBottom, SIGNAL( toggled(bool) ), this, SLOT( topAndBottomSelected(bool) ) );
    connect( itsCustomText, SIGNAL( textChanged(const QString&) ), this, SLOT( textChanged(const QString &) ) );
    connect( itsCustomCheck, SIGNAL( toggled(bool) ), this, SLOT( customStrChecked(bool) ) );

    // tab order
    setTabOrder( itsBasic, itsTopAndBottom );
    setTabOrder( itsTopAndBottom, itsCustomCheck );
    setTabOrder( itsCustomCheck, itsCustomText );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CDisplaySettingsWidgetData::~CDisplaySettingsWidgetData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CDisplaySettingsWidgetData::advancedSelected(bool)
{
    qWarning( "CDisplaySettingsWidgetData::advancedSelected(bool): Not implemented yet!" );
}

void CDisplaySettingsWidgetData::customStrChecked(bool)
{
    qWarning( "CDisplaySettingsWidgetData::customStrChecked(bool): Not implemented yet!" );
}

void CDisplaySettingsWidgetData::textChanged(const QString &)
{
    qWarning( "CDisplaySettingsWidgetData::textChanged(const QString &): Not implemented yet!" );
}

void CDisplaySettingsWidgetData::topAndBottomSelected(bool)
{
    qWarning( "CDisplaySettingsWidgetData::topAndBottomSelected(bool): Not implemented yet!" );
}

#include "DisplaySettingsWidgetData.moc"
