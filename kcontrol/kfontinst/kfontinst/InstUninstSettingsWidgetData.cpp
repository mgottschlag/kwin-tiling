#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file 'InstUninstSettingsWidget.ui'
**
** Created: Tue Jul 31 21:01:28 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "InstUninstSettingsWidgetData.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a CInstUninstSettingsWidgetData which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
CInstUninstSettingsWidgetData::CInstUninstSettingsWidgetData( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CInstUninstSettingsWidgetData" );
    resize( 315, 187 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)5, sizePolicy().hasHeightForWidth() ) );
    setCaption( i18n( "Form2" ) );
    CInstUninstSettingsWidgetDataLayout = new QGridLayout( this ); 
    CInstUninstSettingsWidgetDataLayout->setSpacing( 6 );
    CInstUninstSettingsWidgetDataLayout->setMargin( 11 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    CInstUninstSettingsWidgetDataLayout->addItem( spacer, 3, 1 );

    GroupBox5 = new QGroupBox( this, "GroupBox5" );
    GroupBox5->setTitle( i18n( "Upon Install:" ) );
    GroupBox5->setColumnLayout(0, Qt::Vertical );
    GroupBox5->layout()->setSpacing( 0 );
    GroupBox5->layout()->setMargin( 0 );
    GroupBox5Layout = new QGridLayout( GroupBox5->layout() );
    GroupBox5Layout->setAlignment( Qt::AlignTop );
    GroupBox5Layout->setSpacing( 6 );
    GroupBox5Layout->setMargin( 11 );

    itsFixTtfPsNamesUponInstall = new QCheckBox( GroupBox5, "itsFixTtfPsNamesUponInstall" );
    itsFixTtfPsNamesUponInstall->setText( i18n( "Fix &TrueType Postscript names table" ) );
    QWhatsThis::add(  itsFixTtfPsNamesUponInstall, i18n( "Some TrueType fonts have incorrect\nPostscript names for certain characters.\n\nFor example, \"Euro\" is sometimes listed\nas \"uni20ac\". This would affect any\nPostscript output from applications (and\nmost produce Postscript when printing),\nas it would list the use of the \"Euro\"\ncharacter -but when this Postscript\noutput is sent to the printer, or viewed,\nthe \"Euro\" symbol would not be found.\n\nSelecting this option will cause the installer\nto automatically correct any broken fonts." ) );

    GroupBox5Layout->addWidget( itsFixTtfPsNamesUponInstall, 0, 0 );

    CInstUninstSettingsWidgetDataLayout->addMultiCellWidget( GroupBox5, 0, 0, 0, 1 );

    ButtonGroup1 = new QButtonGroup( this, "ButtonGroup1" );
    ButtonGroup1->setTitle( i18n( "To Uninstall:" ) );
    ButtonGroup1->setColumnLayout(0, Qt::Vertical );
    ButtonGroup1->layout()->setSpacing( 0 );
    ButtonGroup1->layout()->setMargin( 0 );
    ButtonGroup1Layout = new QGridLayout( ButtonGroup1->layout() );
    ButtonGroup1Layout->setAlignment( Qt::AlignTop );
    ButtonGroup1Layout->setSpacing( 6 );
    ButtonGroup1Layout->setMargin( 11 );

    itsDeleteRadio = new QRadioButton( ButtonGroup1, "itsDeleteRadio" );
    itsDeleteRadio->setText( i18n( "De&lete" ) );
    QWhatsThis::add(  itsDeleteRadio, i18n( "If this is selected, then any fonts you choose\nto uninstall will be deleted." ) );

    ButtonGroup1Layout->addWidget( itsDeleteRadio, 0, 0 );

    itsUninstallDirButton = new QPushButton( ButtonGroup1, "itsUninstallDirButton" );
    itsUninstallDirButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, itsUninstallDirButton->sizePolicy().hasHeightForWidth() ) );
    itsUninstallDirButton->setMinimumSize( QSize( 22, 22 ) );
    itsUninstallDirButton->setMaximumSize( QSize( 22, 22 ) );
    itsUninstallDirButton->setText( i18n( "." ) );
    QToolTip::add(  itsUninstallDirButton, i18n( "Change Folder." ) );

    ButtonGroup1Layout->addWidget( itsUninstallDirButton, 0, 3 );

    itsMoveRadio = new QRadioButton( ButtonGroup1, "itsMoveRadio" );
    itsMoveRadio->setText( i18n( "M&ove to:" ) );
    itsMoveRadio->setChecked( TRUE );
    QWhatsThis::add(  itsMoveRadio, i18n( "If this is selected, then any fonts you choose\nto uninstall will be moved to the desired folder." ) );

    ButtonGroup1Layout->addWidget( itsMoveRadio, 0, 1 );

    itsUninstallDirText = new QLabel( ButtonGroup1, "itsUninstallDirText" );
    itsUninstallDirText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, itsUninstallDirText->sizePolicy().hasHeightForWidth() ) );
    itsUninstallDirText->setFrameShape( QLabel::Panel );
    itsUninstallDirText->setFrameShadow( QLabel::Sunken );
    itsUninstallDirText->setText( i18n( "TextLabel4" ) );
    itsUninstallDirText->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );

    ButtonGroup1Layout->addWidget( itsUninstallDirText, 0, 2 );

    CInstUninstSettingsWidgetDataLayout->addMultiCellWidget( ButtonGroup1, 2, 2, 0, 1 );
    QSpacerItem* spacer_2 = new QSpacerItem( 16, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    CInstUninstSettingsWidgetDataLayout->addItem( spacer_2, 1, 0 );

    // signals and slots connections
    connect( itsMoveRadio, SIGNAL( toggled(bool) ), itsUninstallDirButton, SLOT( setEnabled(bool) ) );
    connect( itsFixTtfPsNamesUponInstall, SIGNAL( toggled(bool) ), this, SLOT( fixTtfNamesSelected(bool) ) );
    connect( itsMoveRadio, SIGNAL( toggled(bool) ), this, SLOT( moveToSelected(bool) ) );
    connect( itsUninstallDirButton, SIGNAL( clicked() ), this, SLOT( uninstallDirButtonPressed() ) );

    // tab order
    setTabOrder( itsFixTtfPsNamesUponInstall, itsMoveRadio );
    setTabOrder( itsMoveRadio, itsUninstallDirButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CInstUninstSettingsWidgetData::~CInstUninstSettingsWidgetData()
{
    // no need to delete child widgets, Qt does it all for us
}

void CInstUninstSettingsWidgetData::fixTtfNamesSelected(bool)
{
    qWarning( "CInstUninstSettingsWidgetData::fixTtfNamesSelected(bool): Not implemented yet!" );
}

void CInstUninstSettingsWidgetData::moveToSelected(bool)
{
    qWarning( "CInstUninstSettingsWidgetData::moveToSelected(bool): Not implemented yet!" );
}

void CInstUninstSettingsWidgetData::processAfmsSelected(bool)
{
    qWarning( "CInstUninstSettingsWidgetData::processAfmsSelected(bool): Not implemented yet!" );
}

void CInstUninstSettingsWidgetData::uninstallDirButtonPressed()
{
    qWarning( "CInstUninstSettingsWidgetData::uninstallDirButtonPressed(): Not implemented yet!" );
}

