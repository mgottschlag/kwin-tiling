#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'DisplaySettingsWidget.ui'
**
** Created: Wed Nov 21 00:35:18 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "DisplaySettingsWidgetData.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CDisplaySettingsWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
CDisplaySettingsWidgetData::CDisplaySettingsWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CDisplaySettingsWidgetData" );
    resize( 444, 234 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setCaption( tr2i18n( "Form5" ) );
    CDisplaySettingsWidgetDataLayout = new QGridLayout( this, 1, 1, 11, 6, "CDisplaySettingsWidgetDataLayout"); 

    ButtonGroup5_3 = new QButtonGroup( this, "ButtonGroup5_3" );
    ButtonGroup5_3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, ButtonGroup5_3->sizePolicy().hasHeightForWidth() ) );
    ButtonGroup5_3->setTitle( tr2i18n( "Mode:" ) );
    ButtonGroup5_3->setColumnLayout(0, Qt::Vertical );
    ButtonGroup5_3->layout()->setSpacing( 6 );
    ButtonGroup5_3->layout()->setMargin( 11 );
    ButtonGroup5_3Layout = new QGridLayout( ButtonGroup5_3->layout() );
    ButtonGroup5_3Layout->setAlignment( Qt::AlignTop );

    itsAdvanced = new QRadioButton( ButtonGroup5_3, "itsAdvanced" );
    itsAdvanced->setText( tr2i18n( "&Advanced" ) );
    QWhatsThis::add( itsAdvanced, tr2i18n( "In \"advanced\" mode the complete X11 fonts folder structure is displayed - allowing you to specify exactly where a font should be installed.\n"
"\n"
"This mode also allows you to install Speedo and bitmap fonts." ) );

    ButtonGroup5_3Layout->addWidget( itsAdvanced, 0, 0 );

    itsBasic = new QRadioButton( ButtonGroup5_3, "itsBasic" );
    itsBasic->setText( tr2i18n( "&Basic" ) );
    itsBasic->setChecked( TRUE );
    QWhatsThis::add( itsBasic, tr2i18n( "This mode only displays TrueType and Type1 fonts - and will hide the underlying folder structure." ) );

    ButtonGroup5_3Layout->addWidget( itsBasic, 0, 1 );

    CDisplaySettingsWidgetDataLayout->addMultiCellWidget( ButtonGroup5_3, 0, 0, 0, 2 );
    QSpacerItem* spacer = new QSpacerItem( 16, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    CDisplaySettingsWidgetDataLayout->addItem( spacer, 1, 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    CDisplaySettingsWidgetDataLayout->addItem( spacer_2, 5, 1 );

    itsCustomCheck = new QCheckBox( this, "itsCustomCheck" );
    itsCustomCheck->setText( tr2i18n( "&Custom preview string:" ) );

    CDisplaySettingsWidgetDataLayout->addMultiCellWidget( itsCustomCheck, 4, 4, 0, 1 );

    itsCustomText = new QLineEdit( this, "itsCustomText" );
    itsCustomText->setEnabled( FALSE );
    QWhatsThis::add( itsCustomText, tr2i18n( "Enter your custom preview string. This will be used in the font lists in the \"Fonts\" tab." ) );

    CDisplaySettingsWidgetDataLayout->addWidget( itsCustomText, 4, 2 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
    CDisplaySettingsWidgetDataLayout->addItem( spacer_3, 3, 1 );

    ButtonGroup2 = new QButtonGroup( this, "ButtonGroup2" );
    ButtonGroup2->setTitle( tr2i18n( "Font Lists:" ) );
    ButtonGroup2->setColumnLayout(0, Qt::Vertical );
    ButtonGroup2->layout()->setSpacing( 6 );
    ButtonGroup2->layout()->setMargin( 11 );
    ButtonGroup2Layout = new QGridLayout( ButtonGroup2->layout() );
    ButtonGroup2Layout->setAlignment( Qt::AlignTop );

    itsLeftAndRight = new QRadioButton( ButtonGroup2, "itsLeftAndRight" );
    itsLeftAndRight->setText( tr2i18n( "&Left and right" ) );

    ButtonGroup2Layout->addWidget( itsLeftAndRight, 0, 0 );

    itsTopAndBottom = new QRadioButton( ButtonGroup2, "itsTopAndBottom" );
    itsTopAndBottom->setText( tr2i18n( "&Top and bottom" ) );
    itsTopAndBottom->setChecked( TRUE );

    ButtonGroup2Layout->addWidget( itsTopAndBottom, 0, 1 );

    CDisplaySettingsWidgetDataLayout->addMultiCellWidget( ButtonGroup2, 2, 2, 0, 2 );

    // signals and slots connections
    connect( itsCustomCheck, SIGNAL( toggled(bool) ), itsCustomText, SLOT( setEnabled(bool) ) );
    connect( itsAdvanced, SIGNAL( toggled(bool) ), this, SLOT( advancedSelected(bool) ) );
    connect( itsTopAndBottom, SIGNAL( toggled(bool) ), this, SLOT( topAndBottomSelected(bool) ) );
    connect( itsCustomText, SIGNAL( textChanged(const QString&) ), this, SLOT( textChanged(const QString&) ) );
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
